#!/usr/bin/perl

$help_text = 
"
This script is for creating a monolithic debian package for PyDAW

usage:
perl deb.pl 

--help:  show this help information

";

$short_name = "pydaw3";

$arch = `dpkg --print-architecture`;
chomp($arch);

require 'build-lib.pl';

#Attempt to install dependencies first
check_deps();

#Create a clean folder for the plugins to go in
`rm -Rf pydaw-build/debian/usr`;
`mkdir pydaw-build/debian/usr`;

if( system("make clean && make && make strip && make DESTDIR=\$(pwd)/pydaw-build/debian install") != 0)
{
	exit 1;
}


#Delete gedit backup files, core dumps and Python compile-cache files
system('find ./pydaw-build/debian -type f -name *~  -exec rm -f {} \\;');
system('find ./pydaw-build/debian -type f -name *.pyc  -exec rm -f {} \\;');
system('find ./pydaw-build/debian -type f -name core  -exec rm -f {} \\;');

if(-e "$short_name-version.txt")
{
	open FILE, "$short_name-version.txt"; # or `rm maintainer.txt` && goto maintainer_label; #die "Couldn't open file: $!"; 
	$version = join("", <FILE>); 
	close FILE;
	chomp($version);
}
else
{
	#Creates a version number like what Ubuntu does, for example:  12.03 or 13.11.
	$version = `date +"%y.%m"`;
	chomp($version);
	$version .= "-1";
}


print 
"Please enter the version number of this release.  
The format should be something like:  1.1.3-1 or 12.04-1
Hit enter to accept the auto-generated default version number:  $version
[version number]:";
$version_answer = <STDIN>;
chomp($version_answer);

if($version_answer ne ""){
	$version = $version_answer;
	open (MYFILE, ">$short_name-version.txt");
	print MYFILE "$version";
	close (MYFILE);
}	

$size = `du -s pydaw-build/debian/usr`;
$size = (split(" ", $size))[0];
chomp($size);

#libsdl1.2-dev, ffado-mixer-qt4, ffado-tools, ffado-dbus-server,

$debian_control = "
Package: $short_name
Priority: extra
Section: sound
Installed-Size: $size
Maintainer: PyDAW Team <pydawteam\@users.sf.net>
Architecture: $arch
Version: $version
Depends: liblo-dev, libsndfile1-dev, python-pypm, libportmidi-dev, python-pyaudio, portaudio19-dev, python-liblo, python-qt4, audacity, python2.7, libmad0-dev, python-scipy, python-numpy, libsamplerate0-dev, libfftw3-dev
Provides: $short_name
Conflicts: 
Replaces: 
Description: A digital audio workstation with a full suite of instrument and effects plugins.
 PyDAW is a full featured audio and MIDI sequencer with a suite of high quality instrument and effects plugins.
";

open (MYFILE, ">pydaw-build/debian/DEBIAN/control");
print MYFILE "$debian_control";
close (MYFILE); 

`chmod 755 pydaw-build/debian/DEBIAN/control`;
`chmod 755 "pydaw-build/debian/DEBIAN/postinst"`;

#Create the DEBIAN/md5sums file
`cd pydaw-build/debian; find . -type f ! -regex '.*\.hg.*' ! -regex '.*?debian-binary.*' ! -regex '.*?DEBIAN.*' -printf '%P ' | xargs md5sum > DEBIAN/md5sums`;

system("chmod -R 755 pydaw-build/debian/usr ; chmod 644 pydaw-build/debian/DEBIAN/md5sums");

$build_suffix = "";

if(-e "build-suffix.txt")
{
	open FILE, "build-suffix.txt";
	$build_suffix = join("", <FILE>); 
	close FILE;
	chomp($build_suffix);
}
else
{

	print "\nYou may enter an optional build suffix.  Usually this will be the operating system you are compiling for on this machine, for example: ubuntu12.10 \n\n";
	print "\nPlease enter a build suffix, or hit 'enter' to leave blank:\n";
	$build_suffix = <STDIN>;
	chomp($build_suffix);
	if($build_suffix ne "")
	{
		$build_suffix = "-" . $build_suffix;
	}

	open (MYFILE, ">>build-suffix.txt");
	print MYFILE "$build_suffix";
	close (MYFILE); 

}

$package_name = "$short_name-$version-$arch$build_suffix.deb";

`cd pydaw-build ; fakeroot dpkg-deb --build debian ; rm $package_name ; mv debian.deb $package_name`;


print "\n\nComplete.  Your package is now located at:\n pydaw-build/$package_name\n\nInstall now?  y/[n]: ";
$install_answer = <STDIN>;
chomp($install_answer);

if($install_answer eq "y"){
	system("sudo dpkg -i pydaw-build/$package_name");
}

$pydaw_os_home = "pydaw-build/pydaw_os/edit/root";

if(-d $pydaw_os_home){
	system("sudo cp pydaw-build/$package_name $pydaw_os_home/");
}
else
{
	print "\n\nThere is no PyDAW-OS directory, not copying package to /root\n\n\n";
}

#This is because debian packages will only accept dashes in package names
sub replace_underscore_with_dash
{
	$result = $_[0];
	$result =~ s/_/-/g;
	return $result;
}
