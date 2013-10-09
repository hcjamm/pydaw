#!/usr/bin/env python3

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

import os, sys, time, ctypes
from PyQt4 import QtGui, QtCore
try:
    from libpydaw import pydaw_util, portaudio, portmidi
except ImportError:
    import pydaw_util, portaudio, portmidi

class pydaw_device_dialog:
    def __init__(self, a_home_folder, a_is_running=False):
        self.is_running = a_is_running
        self.device_name = None
        self.home_folder = str(a_home_folder)
        if not os.path.isdir(self.home_folder):
            os.mkdir(self.home_folder)
        print((self.home_folder))
        self.device_file = "%s/device.txt" % (self.home_folder,)
        self.sample_rates = ["44100", "48000", "88200", "96000"]
        self.buffer_sizes = ["32", "64", "128", "256", "512", "1024", "2048"]
        self.val_dict = {}
        if os.path.isfile(self.device_file):
            f_file_text = pydaw_util.pydaw_read_file_text(self.device_file)
            for f_line in f_file_text.split("\n"):
                if f_line.strip() == "\\":
                    break
                if f_line.strip() != "":
                    f_line_arr = f_line.split("|", 1)
                    self.val_dict[f_line_arr[0].strip()] = f_line_arr[1].strip()

    def show_device_dialog(self, a_msg=None, a_notify=False):
        f_stylesheet_file = "%s/lib/%s/themes/default/style.txt" % (pydaw_util.global_pydaw_install_prefix,  \
        pydaw_util.global_pydaw_version_string)
        f_stylesheet = pydaw_util.pydaw_read_file_text(f_stylesheet_file)
        f_stylesheet = pydaw_util.pydaw_escape_stylesheet(f_stylesheet, f_stylesheet_file)
        if self.is_running:
            f_window = QtGui.QDialog()
        else:
            f_window = QtGui.QWidget()
        f_window.setWindowFlags(QtCore.Qt.WindowStaysOnTopHint)
        f_window.setStyleSheet(f_stylesheet)
        f_window.setWindowTitle("Hardware Settings...")
        f_window_layout = QtGui.QGridLayout(f_window)
        f_window_layout.addWidget(QtGui.QLabel("Audio Device:"), 0, 0)
        f_device_name_combobox = QtGui.QComboBox()
        f_device_name_combobox.setMinimumWidth(390)
        f_window_layout.addWidget(f_device_name_combobox, 0, 1)
        f_window_layout.addWidget(QtGui.QLabel("Sample Rate:"), 1, 0)
        f_samplerate_combobox = QtGui.QComboBox()
        f_samplerate_combobox.addItems(self.sample_rates)
        f_window_layout.addWidget(f_samplerate_combobox, 1, 1)
        f_buffer_size_combobox = QtGui.QComboBox()
        f_buffer_size_combobox.addItems(self.buffer_sizes)
        f_buffer_size_combobox.setCurrentIndex(4)
        f_window_layout.addWidget(QtGui.QLabel("Buffer Size:"), 2, 0)
        f_window_layout.addWidget(f_buffer_size_combobox, 2, 1)
        f_latency_label = QtGui.QLabel("")
        f_window_layout.addWidget(f_latency_label, 2, 2)
        f_window_layout.addWidget(QtGui.QLabel("Worker Threads:"), 3, 0)
        f_worker_threads_combobox = QtGui.QComboBox()
        f_worker_threads_combobox.addItems(["Auto", "1", "2", "3", "4", "5", "6", "7", "8"])
        f_worker_threads_combobox.setToolTip("%s\n%s\n\n%s\n%s\n\n%s\n%s" %
        ("This control sets the number of worker threads for processing plugins and effects.",
         "Setting to 1 can result in the best latency.",
         "If your projects require more CPU power than one CPU core can provide, for best latency it is ",
         "recommended that you only add the required number of cores, and not more than one thread per CPU core.",
         "Auto attempts to pick a sane number of worker threads automatically based on your CPU,",
         "if you're not sure how to use this setting, you should leave it on 'Auto'."))
        f_window_layout.addWidget(f_worker_threads_combobox, 3, 1)
        f_thread_affinity_checkbox = QtGui.QCheckBox("Lock worker threads to own core?")
        f_thread_affinity_checkbox.setToolTip(
        "This may give better performance with fewer Xruns, but may perform badly on certain configurations.")
        f_window_layout.addWidget(f_thread_affinity_checkbox, 4, 1)
        f_window_layout.addWidget(QtGui.QLabel("MIDI In Device:"), 7, 0)
        f_midi_in_device_combobox = QtGui.QComboBox()
        f_midi_in_device_combobox.addItem("None")
        f_window_layout.addWidget(f_midi_in_device_combobox, 7, 1)
        f_ok_cancel_layout = QtGui.QHBoxLayout()
        f_window_layout.addLayout(f_ok_cancel_layout, 10, 1)
        f_ok_button = QtGui.QPushButton("OK")
        f_ok_cancel_layout.addWidget(f_ok_button)
        f_cancel_button = QtGui.QPushButton("Cancel")
        f_ok_cancel_layout.addWidget(f_cancel_button)

        f_portaudio_so_path = "%s/libportaudio.so" % (os.path.dirname(os.path.abspath(__file__)),)
        ctypes.cdll.LoadLibrary(f_portaudio_so_path)
        f_pyaudio = ctypes.CDLL(f_portaudio_so_path)
        f_pyaudio.Pa_GetDeviceInfo.restype = ctypes.POINTER(portaudio.PaDeviceInfo)
        f_pyaudio.Pa_GetDeviceInfo.argstype = [ctypes.c_int]
        f_pyaudio.Pa_GetHostApiInfo.restype = ctypes.POINTER(portaudio.PaHostApiInfo)
        f_pyaudio.Pa_GetHostApiInfo.argstype = [ctypes.c_int]
        f_pyaudio.Pa_IsFormatSupported.argstype = [ctypes.POINTER(portaudio.PaStreamParameters),
                                                   ctypes.POINTER(portaudio.PaStreamParameters), ctypes.c_double]
        f_pyaudio.Pa_Initialize()
        f_count = f_pyaudio.Pa_GetHostApiCount()

        f_count = f_pyaudio.Pa_GetDeviceCount()
        print(("f_count == %s" % (f_count,)))

        f_result_dict = {}
        f_name_to_index = {}
        f_audio_device_names = []

        for i in range(f_count):
            f_dev = f_pyaudio.Pa_GetDeviceInfo(i)
            print(("\nDevice Index: %s" % (i,)))
            f_dev_name = f_dev.contents.name.decode("utf-8")
            print(("Name : %s" % (f_dev_name,)))
            f_name_to_index[f_dev_name] = i
            f_result_dict[f_dev_name] = f_dev.contents
            f_audio_device_names.append(f_dev_name)

        ctypes.cdll.LoadLibrary("libportmidi.so")
        pypm = ctypes.CDLL("libportmidi.so")
        pypm.Pm_GetDeviceInfo.restype = ctypes.POINTER(portmidi.PmDeviceInfo)
        pypm.Pm_Initialize()

        print("\n")
        for loop in range(pypm.Pm_CountDevices()):
            f_midi_device = pypm.Pm_GetDeviceInfo(loop)
            f_midi_device_name = f_midi_device.contents.name.decode("utf-8")
            print(("DeviceID: %s Name: '%s' Input?: %s Output?: %s Opened: %s " %
            (loop, f_midi_device_name, f_midi_device.contents.input, f_midi_device.contents.output,
             f_midi_device.contents.opened)))
            if f_midi_device.contents.input == 1:
                f_midi_in_device_combobox.addItem(f_midi_device_name)

        def latency_changed(a_self=None, a_val=None):
            f_sample_rate = float(str(f_samplerate_combobox.currentText()))
            f_buffer_size = float(str(f_buffer_size_combobox.currentText()))
            f_latency = (f_buffer_size / f_sample_rate) * 1000.0
            f_latency_label.setText(str(round(f_latency, 1)) + " ms")

        f_samplerate_combobox.currentIndexChanged.connect(latency_changed)
        f_buffer_size_combobox.currentIndexChanged.connect(latency_changed)

        def combobox_changed(a_self=None, a_val=None):
            self.device_name = str(f_device_name_combobox.currentText())
            f_samplerate = str(int(f_result_dict[self.device_name].defaultSampleRate))
            if f_samplerate in self.sample_rates:
                f_samplerate_combobox.setCurrentIndex(f_samplerate_combobox.findText(f_samplerate))

        def on_ok(a_self=None):
            f_buffer_size = int(str(f_buffer_size_combobox.currentText()))
            f_samplerate = int(str(f_samplerate_combobox.currentText()))
            f_worker_threads = f_worker_threads_combobox.currentIndex()
            f_midi_in_device = str(f_midi_in_device_combobox.currentText())
            if f_thread_affinity_checkbox.isChecked():
                f_thread_affinity = 1
            else:
                f_thread_affinity = 0
            try:
                #This doesn't work if the device is open already, so skip the test, and if it fails the
                #user will be prompted again next time PyDAW starts
                if not self.is_running or "name" not in self.val_dict or self.val_dict["name"] != self.device_name:
                    f_output = portaudio.PaStreamParameters(f_name_to_index[self.device_name], 2, portaudio.paInt16,
                                                            float(f_buffer_size)/float(f_samplerate), None)
                    f_supported = f_pyaudio.Pa_IsFormatSupported(0, ctypes.byref(f_output), f_samplerate)
                    if not f_supported:
                        raise Exception()
                f_file = open(self.device_file, "w")
                f_file.write("name|%s\n" % (self.device_name,))
                f_file.write("bufferSize|%s\n" % (f_buffer_size,))
                f_file.write("sampleRate|%s\n" % (f_samplerate,))
                f_file.write("threads|%s\n" % (f_worker_threads,))
                f_file.write("threadAffinity|%s\n" % (f_thread_affinity,))
                f_file.write("midiInDevice|%s\n" % (f_midi_in_device,))

                f_file.write("\\")
                f_file.close()
                f_pyaudio.Pa_Terminate()
                pypm.Pm_Terminate()

                if a_notify:
                    QtGui.QMessageBox.warning(f_window, "Settings changed",
                      "Hardware setttings have been changed, and will be applied next time you start PyDAW.")
                else:
                    time.sleep(0.2)
                f_window.close()
            except Exception as ex:
                QtGui.QMessageBox.warning(f_window, "Error", "Couldn't open audio device\n\n%s\n\n%s" % (ex,
                        "This may (or may not) be because the device already open by PyDAW or another application."))

        def on_cancel(a_self=None):
            f_window.close()

        f_ok_button.pressed.connect(on_ok)
        f_cancel_button.pressed.connect(on_cancel)

        f_device_name_combobox.currentIndexChanged.connect(combobox_changed)
        f_audio_device_names.sort()
        f_device_name_combobox.addItems(f_audio_device_names)

        if "name" in self.val_dict and self.val_dict["name"] in f_result_dict:
            f_device_name_combobox.setCurrentIndex(f_device_name_combobox.findText(self.val_dict["name"]))

        if "bufferSize" in self.val_dict and self.val_dict["bufferSize"] in self.buffer_sizes:
            f_buffer_size_combobox.setCurrentIndex(f_buffer_size_combobox.findText(self.val_dict["bufferSize"]))

        if "sampleRate" in self.val_dict and self.val_dict["sampleRate"] in self.sample_rates:
            f_samplerate_combobox.setCurrentIndex(f_samplerate_combobox.findText(self.val_dict["sampleRate"]))

        if "threads" in self.val_dict:
            f_worker_threads_combobox.setCurrentIndex(int(self.val_dict["threads"]))

        if "threadAffinity" in self.val_dict:
            if int(self.val_dict["threadAffinity"]) == 1:
                f_thread_affinity_checkbox.setChecked(True)

        if "midiInDevice" in self.val_dict:
            f_midi_in_device_combobox.setCurrentIndex(f_midi_in_device_combobox.findText(self.val_dict["midiInDevice"]))

        if a_msg is not None:
            QtGui.QMessageBox.warning(f_window, "Error", a_msg)

        f_screen = QtGui.QDesktopWidget().screenGeometry()
        f_size = f_window.geometry()
        f_hpos = ( f_screen.width() - f_size.width() ) / 2
        f_vpos = ( f_screen.height() - f_size.height() ) / 2
        f_window.move(f_hpos, f_vpos)
        latency_changed()
        if self.is_running:
            f_window.exec_()
        else:
            f_window.show()

if __name__ == "__main__":
    def _pydaw_portaudio_standalone():
        app = QtGui.QApplication(sys.argv)
        if len(sys.argv) == 2:
            f_msg = sys.argv[1]
        else:
            f_msg = None

        f_pydaw_device_dialog = pydaw_device_dialog(pydaw_util.global_pydaw_home)
        f_pydaw_device_dialog.show_device_dialog(f_msg)
        sys.exit(app.exec_())

    _pydaw_portaudio_standalone()
