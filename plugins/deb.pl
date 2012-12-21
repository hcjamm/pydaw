#!/usr/bin/perl

$help_text = 
"
The PyDAW interactive packaging script.  https://github.com/j3ffhubb/audiocode

This script is for creating a monolithic debian package for any number of individual plugins defined in the \@plugins array in this script(including PyDAW itself).

usage:
perl build-all.pl 

--help:  show this help information

";

$short_name = "pydaw";

$arch = `uname -i`;
chomp($arch);

if($arch eq "x86_64")
{
$arch = "amd64"; #i386 will already be correct, so there is no elsif for it
}
elsif($arch eq "unknown")
{
	print "Unknown architecture, please enter the name of the architecture.  Some standard ones are i386, i686 and AMD64.  
If you're not sure, press CTRL+C to exit the script, and confirm the architecture you're compiling for, otherwise your packages won't work
arch:\n";
	$arch = <STDIN>;
	chomp($arch);	
}

require 'build-lib.pl';

#Attempt to install dependencies first
check_deps();

#Create a clean folder for the plugins to go in
`rm -Rf pydaw-build/debian/usr`;
`mkdir pydaw-build/debian/usr`;
system("make clean && make && make DESTDIR=\$(pwd)/pydaw-build/debian install");

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

$debian_control = "
Package: pydaw
Priority: extra
Section: sound
Installed-Size: $size
Maintainer: Jeff Hubbard <jhubbard651\@users.sf.net>
Architecture: amd64
Version: $version
Depends: liblo-dev, dssi-dev, ladspa-sdk, libasound2-dev, libqt4-dev, libsndfile1-dev, libsm-dev, qjackctl, alsa-utils, python-liblo, python-qt4, git, libsdl1.2-dev, ffado-mixer-qt4, ffado-tools, ffado-dbus-server, audacity
Provides: pydaw
Conflicts: 
Replaces: 
Description: PyDAW is a digital audio workstation with robust MIDI capabilities and a full suite of instrument and effects plugins.
 It comes with a flexible modular sampler, a modular wavetable synthesizer, and a retro analog style synthesizer called Ray-V, as well as numerous built-in effects.
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

$package_name = "pydaw-$version-$arch$build_suffix.deb";

`cd pydaw-build ; fakeroot dpkg-deb --build debian ; rm $package_name ; mv debian.deb $package_name`;


print "\n\nComplete.  Your package is now located at:\n pydaw-build/$package_name\n\nInstall now?  y/[n]: ";
$install_answer = <STDIN>;
chomp($install_answer);

if($install_answer eq "y"){
	system("sudo dpkg -i pydaw-build/$package_name");
}

#This is because debian packages will only accept dashes in package names
sub replace_underscore_with_dash
{
	$result = $_[0];
	$result =~ s/_/-/g;
	return $result;
}
