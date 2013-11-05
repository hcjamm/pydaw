sudo apt-get install -y libasound2-dev \
libportmidi-dev liblo-dev g++ libsndfile1-dev libtool gdb \
debhelper dh-make build-essential automake autoconf \
python3-pyqt4 python3 squashfs-tools genisoimage \
python3-scipy python3-numpy libsamplerate0-dev \
libfftw3-dev gcc python3-dev libsbsms-dev libcpufreq-dev \
ffmpeg lame 
echo
echo
echo
echo "Ubuntu 12.10, 13.04 and 13.10 users:"
echo "It is highly recommended that you install gcc-4.6 with this command:"
echo
echo "sudo apt-get install gcc-4.6"
echo
echo "gcc-4.7 and gcc-4.8 had a number of regressions that can cause weird bugs in "

echo  "PyDAW.  To the best of my knowledge, these will all be fixed in the gcc"
echo " shipping with Ubuntu 14.04"
echo
