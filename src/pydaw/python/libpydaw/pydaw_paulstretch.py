#!/usr/bin/env python3
#
# Paul's Extreme Sound Stretch (Paulstretch) - Python version
# originally by Nasca Octavian PAUL, Targu Mures, Romania
#
# Forked to allow better integration with PyDAW, fixes, improvements, etc...
"""
This file is part of the PyDAW project, Copyright PyDAW Team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
"""

import sys, os, subprocess
import numpy
import scipy.io.wavfile
import wave
from optparse import OptionParser
try:
    from libpydaw.pydaw_util import *
except ImportError:
    from pydaw_util import *

plot_onsets=False
if plot_onsets:
    import matplotlib.pyplot as plt


def load_wav(filename):
    try:
        wavedata = scipy.io.wavfile.read(filename)
        samplerate = int(wavedata[0])
        smp = normalize(wavedata[1]) # * (1.0 / 32768.0)
        smp = smp.transpose()
        if len(smp.shape) == 1: #convert to stereo
            smp = tile(smp,(2, 1))
        return (samplerate, smp)
    except:
        print("Error loading wav: {}".format(filename))
        return None


def optimize_windowsize(n):
    orig_n = n
    while True:
        n=orig_n
        while (n % 2) == 0:
            n /= 2
        while (n % 3) == 0:
            n /= 3
        while (n % 5) == 0:
            n /= 5
        if n < 2:
            break
        orig_n += 1
    return orig_n

def normalize(output):
    f_max = output.max()
    f_min = abs(output.min())
    if f_min > f_max:
        f_max = f_min
    if f_max > 0.5:
        f_normalize = 0.5 / f_max
        return output * f_normalize
    else:
        return output

def paulstretch(file_path, stretch, windowsize_seconds, onset_level, outfilename, a_start_pitch,
                a_end_pitch, a_in_file, a_delete=False):
    f_tuple = load_wav(file_path)
    if f_tuple is None:
        print("Error loading wav file, returned None")
        return
    else:
        samplerate, smp = f_tuple

    if plot_onsets:
        onsets = []

    nchannels = smp.shape[0]

    outfile=wave.open(outfilename,"wb")
    outfile.setsampwidth(2)
    outfile.setframerate(samplerate)
    outfile.setnchannels(nchannels)

    #make sure that windowsize is even and larger than 16
    windowsize=int(windowsize_seconds*samplerate)
    if windowsize < 16:
        windowsize = 16
    windowsize=optimize_windowsize(windowsize)
    windowsize=int(windowsize / 2) * 2
    half_windowsize=int(windowsize/2)

    #correct the end of the smp
    nsamples = smp.shape[1]
    end_size = int(samplerate*0.05)
    if end_size < 16:
        end_size = 16

    smp[:,nsamples-end_size:nsamples] *= numpy.linspace(1,0,end_size)

    #compute the displacement inside the input file
    start_pos=0.0
    displace_pos=windowsize*0.5

    #create Hann window
    window = 0.5 - numpy.cos(numpy.arange(windowsize,dtype='double') * \
        2.0 * numpy.pi / (windowsize - 1)) * 0.5

    old_windowed_buf = numpy.zeros((2,windowsize))
    hinv_sqrt2 = (1 + numpy.sqrt(0.5)) * 0.5
    hinv_buf = 2.0 * (hinv_sqrt2 - (1.0 - hinv_sqrt2) * \
        numpy.cos(numpy.arange(half_windowsize, dtype='double') \
        * 2.0 * numpy.pi / half_windowsize)) / hinv_sqrt2

    freqs = numpy.zeros((2, half_windowsize + 1))
    old_freqs = freqs

    num_bins_scaled_freq = 32
    freqs_scaled = numpy.zeros(num_bins_scaled_freq)
    old_freqs_scaled = freqs_scaled

    displace_tick=0.0
    displace_tick_increase = 1.0 / stretch
    if displace_tick_increase>1.0:
        displace_tick_increase=1.0
    extra_onset_time_credit=0.0
    get_next_buf=True
    while True:
        if get_next_buf:
            old_freqs = freqs
            old_freqs_scaled = freqs_scaled

            #get the windowed buffer
            istart_pos = int(numpy.floor(start_pos))
            buf = smp[:,istart_pos:istart_pos + windowsize]
            if buf.shape[1]<windowsize:
                buf = numpy.append(buf, numpy.zeros((2, windowsize - buf.shape[1])), 1)
            buf = buf * window

            #get the amplitudes of the frequency components and discard the phases
            freqs = numpy.abs(numpy.fft.rfft(buf))

            #scale down the spectrum to detect onsets
            freqs_len = freqs.shape[1]
            if num_bins_scaled_freq < freqs_len:
                freqs_len_div = freqs_len // num_bins_scaled_freq
                new_freqs_len = freqs_len_div * num_bins_scaled_freq
                freqs_scaled = numpy.mean(numpy.mean(freqs, 0)[:new_freqs_len].reshape(
                [num_bins_scaled_freq,freqs_len_div]), 1)
            else:
                freqs_scaled = numpy.zeros(num_bins_scaled_freq)

            #process onsets
            m = 2.0 * numpy.mean(freqs_scaled - old_freqs_scaled) / \
                (numpy.mean(numpy.abs(old_freqs_scaled)) + 1e-3)
            if m < 0.0:
                m = 0.0
            if m > 1.0:
                m = 1.0
            if plot_onsets:
                onsets.append(m)
            if m > onset_level:
                displace_tick = 1.0
                extra_onset_time_credit += 1.0

        cfreqs = (freqs * displace_tick) + (old_freqs * (1.0 - displace_tick))

        #randomize the phases by multiplication with a random complex number with modulus=1
        ph = numpy.random.random(size=(nchannels, cfreqs.shape[1])) * (2. * numpy.pi) * 1j
        cfreqs = cfreqs * numpy.exp(ph)

        #do the inverse FFT
        buf = numpy.fft.irfft(cfreqs)

        #window again the output buffer
        buf *= window

        #overlap-add the output
        output = buf[:,0:half_windowsize] + old_windowed_buf[:,half_windowsize:windowsize]
        old_windowed_buf=buf

        #remove the resulted amplitude modulation
        output *= hinv_buf

        output = normalize(output)

        outfile.writeframes(numpy.int16(output.ravel(1) * 30000.0).tostring()) #32767.0

        if get_next_buf:
            start_pos += displace_pos

        get_next_buf=False

        if start_pos >= nsamples:
            #print("100 %")
            break
        #print("%d %% \r" % int(100.0*start_pos/nsamples),)
        sys.stdout.flush()

        if extra_onset_time_credit <= 0.0:
            displace_tick += displace_tick_increase
        else:
            #this must be less than displace_tick_increase
            credit_get = 0.5 * displace_tick_increase
            extra_onset_time_credit -= credit_get
            if extra_onset_time_credit < 0:
                extra_onset_time_credit = 0
            displace_tick += displace_tick_increase-credit_get

        if displace_tick >= 1.0:
            displace_tick = displace_tick % 1.0
            get_next_buf=True

    outfile.close()

    if a_delete:
        print("Deleting source file {}".format(a_in_file))
        os.remove(a_in_file)

    if a_start_pitch is not None:
        print("Pitch shifting file")
        f_dest_path = outfilename
        f_src_path = outfilename.replace(".wav", "-OLD.wav")
        print((f_src_path, "\n", f_dest_path))
        os.rename(f_dest_path, f_src_path)
        if a_end_pitch is not None:
            f_cmd = ["{}/lib/{}/sbsms/bin/sbsms".format(
                        global_pydaw_install_prefix, global_pydaw_version_string),
                     f_src_path, f_dest_path, "1.0", "1.0", str(a_start_pitch), str(a_end_pitch)]
        else:
            f_cmd = ["rubberband", "-p", str(a_start_pitch), "-R",
                     "--pitch-hq", f_src_path, f_dest_path]
        print("Running {}".format(" ".join(f_cmd)))
        f_proc = subprocess.Popen(f_cmd)
        f_proc.wait()
        os.remove(f_src_path)
    if plot_onsets:
        plt.plot(onsets)
        plt.show()

########################################
print("Paul's Extreme Sound Stretch (Paulstretch) - Python version 20110223")
print("new method: using onsets information")
print("by Nasca Octavian PAUL, Targu Mures, Romania\n")
parser = OptionParser(usage="usage: %prog [options] input_wav output_wav")
parser.add_option("-s", "--stretch", dest="stretch", help="stretch amount (1.0 = no stretch)",
                  type="float",default=8.0)
parser.add_option("-w", "--window_size", dest="window_size", help="window size (seconds)",
                  type="float", default=0.25)
parser.add_option("-t", "--onset", dest="onset", help="onset sensitivity (0.0=max, 1.0=min)",
                  type="float", default=10.0)
parser.add_option("-p", "--start-pitch", dest="start_pitch",
                  help="start pitch (36.0=max, -36.0=min)",
                  type="float", default=None)
parser.add_option("-e", "--end-pitch", dest="end_pitch", help="end pitch (36.0=max, -36.0=min)",
                  type="float", default=None)
parser.add_option("-d", "--delete", dest="delete", help="Delete source file after stretching",
                  action="store_true", default=False)
(options, args) = parser.parse_args()

if (len(args) < 2) or (options.stretch <= 0.0) or (options.window_size <= 0.001):
    print("Error in command line parameters. Run this program with --help for help.")
    sys.exit(1)

print("stretch amount = {}".format(options.stretch))
print("window size = {} seconds".format(options.window_size))
print("onset sensitivity = {}".format(options.onset))

paulstretch(args[0], numpy.double(options.stretch), numpy.double(options.window_size),
            numpy.double(options.onset),
            args[1], options.start_pitch, options.end_pitch, args[0], options.delete)
