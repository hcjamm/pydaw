#!/usr/bin/perl

#This script is for creating a monolithic debian package for any number of individual plugins defined in the @plugins array below
#This script can be used as-is for packaging the LMS Suite for a distro.  If you are a developer using LibModSynth, you should edit
#the below values.
#usage:
#perl build-all.pl

#Change this if you are compiling your own suite of plugins:
$short_name = "lms_suite";

#The exact dependencies could vary with each distro, and some of these aren't used yet, but are intended for future use
$depends = "liblo-dev,dssi-dev,ladspa-sdk,libasound2-dev,qjackctl,libjack-jackd2-dev,libsndfile1-dev,libsamplerate0-dev,libsm-dev,liblscp-dev,libmad0-dev";

#These are to replace the original packages that the LMS Suite came in.  You can remove these from your own plugins.
$replaces = "ray_v,ray-v,lms_comb,lms-comb,lms_distortion,lms-distortion,lms_delay,lms-delay,lms_filter,lms-filter";

#You can probably leave this empty, otherwise you should probably know if any packages conflict
$conflicts = "";

#This is a standard description for the package, change this if packaging your own plugins
$description = "The LMS Suite is a collection of DSSI plugins written using LibModSynth."

#add any new plugins here, or remove all of them if you are not going to redistribute the official LMS plugins.
#Please take care not to create package conflicts for people with the LMS Suite installed.
@plugins = 
(
'ray_v',
'lms_comb',
'lms_delay',
'lms_distortion',
'lms_filter'
);
#Unfinished plugins not yet in the official package:
#lms_dynamics,
#lms_reverb

#This is the notes that your package manager will show when browsing your package.  Change this if packaging your own plugins.
$notes = " LibModSynth is a set of developer tools designed to make it fast and easy to develop high quality DSSI plugins.
 The suite currently includes:
";

foreach $val (@plugins)
{
$notes .= " $val\n";
}


#End variables, begin the actual script:

ack_label:
print "
This will build all of the plugins and package them.  You must edit the list of plugins in this script to include any new ones, or exclude any old ones.  Please take care when packaging the existing LMS plugins not to create package conflicts.  If you wish only to build a single plugin, use the build.pl scripts in that plugin's directory.  If you wish to package your own collection of LMS-derived plugins, you should edit the \@plugins array in this script to include only your plugins.

Proceed?  (y/[n])
";

$ack = <STDIN>;

chomp($ack);
$ack = lc($ack);

if($ack eq 'y')
{
#do nothing
}
elsif(($ack eq 'n') || ($ack eq ''))
{
exit;
}
else
{
goto ack_label;
}

require 'build-lib.pl';


#Here are the directories used for the install, you can modify them if needed.
$base_dir = "../$short_name";
$package_dir = "$base_dir/deb";
$debian_dir = "$package_dir/debian";
#At some point, this script may include switches for different distros, which will automatically set these as appropriate.
#The current values below are valid for Ubuntu, and likely most Debian variants
$icon_dir = "$package_dir/usr/share/pixmaps";
$desktop_dir = "$package_dir/usr/share/applications";
$plugin_dir = "$package_dir/usr/lib/dssi";


#Create a clean folder for the plugins to go in
`rm -Rf $base_dir`;
`mkdir -p $plugin_dir`;
`mkdir -p $debian_dir`;
`mkdir -p $package_dir`;
`mkdir -p $icon_dir`;
`mkdir -p $desktop_dir`;

foreach $val(@plugins)
{
#copy the .so, .la and LMS_qt files to the directory we created
`mkdir $package_dir/$plugin_dir/$val`;
print "Compiling $val\n";
`cd $val ; perl build.pl --full-build`;

print "Copying files\n";
system("cp $val/src/LMS_qt $plugin_dir/$val/LMS_qt");
system("cp $val/src/.libs/$val.so $plugin_dir/$val.so");
system("cp $val/src/.libs/$val.la $plugin_dir/$val.la");

#This currently just overwrites the same file at every iteration, but I'm leaving it here because it will eventually support custom icons
`cp packaging/libmodsynth.png $icon_dir/libmodsynth.png`;

#TODO:  Copy a .desktop file to the desktop_dir

$desktop_text = 
"[Desktop Entry]
Name=$val
Comment=$description
Exec=jack-dssi-host \"$val.so\"
Icon=libmodsynth.png
Terminal=false
Type=Application
Categories=AudioVideo;Audio;";

open (MYFILE, ">>$desktop_dir");
print MYFILE "$desktop_text";
close (MYFILE); 
}

#TODO:  Compile and install jack-dssi-host

`cp -r . $base_dir/libmodsynth-git`;

maintainer_label:

if(-e "maintainer.txt")
{
	open FILE, "maintainer.txt"; # or `rm maintainer.txt` && goto maintainer_label; #die "Couldn't open file: $!"; 
	$maintainer = join("", <FILE>); 
	close FILE;
	chomp($maintainer);
}
else
{
	print "\nThe following questions are required for the package maintainer meta-data.\n\n";
	print "\nPlease enter your first and last name:\n";
	my $name = <STDIN>;
	print "\nPlease enter your email:\n";
	my $email = <STDIN>;

	chomp($email);
	chomp($name);

	$maintainer = "$name <$email>";

	open (MYFILE, ">>maintainer.txt");
	print MYFILE "$maintainer";
	close (MYFILE); 
}

$arch = `uname -i`;
chomp($arch);

if($arch eq "x86_64")
{
$arch = "amd64"; #i386 will already be correct, so there is no elsif for it
}

#$version = "1.0.0-1";
#print "Please enter the version number of this release.  
#The format should be something like:  1.1.3 or 3.5.0\n[version number]:";
#$version = <STDIN>;
#chomp($version);

#Creates a version number like what Ubuntu does, for example:  12.03 or 13.11.  The script may eventually offer a choice of how to name it.
$version = `date +"%y.%m"`;
$version .= "-1";
chomp($version);

$size = `du -s $package_dir/usr`;
$size = split(" ", $size)[0];
chomp($size);

$debian_control = "
Package: $short_name
Priority: extra
Section: sound
Installed-Size: $size
Maintainer: $maintainer
Architecture: $arch
Version: $version
Depends: $depends
Provides: $short_name
Conflicts: $conflicts
Replaces: $replaces
Description: $description
$notes";

open (MYFILE, ">>$debian_dir/control");
print MYFILE "$debian_control";
close (MYFILE); 

open (MYFILE, ">>$debian_dir/conffiles");
print MYFILE "";   #TODO:  Find out what this file is for, and when/if something should ever go in it
close (MYFILE); 

package_label:
print "
The plugins have been compiled and built.  Would you like to package them now?  If you need to modify the files in $debian_dir first, you should choose 'n', and modify them manually.  Once you've done this, you can build the packages with the following commands:

cd $debian_dir
dpkg-deb --build debian

Build the packages now?  ([y]/n)
";

$ack = <STDIN>;

chomp($ack);
$ack = lc($ack);

if(($ack eq 'y') || ($ack eq ''))
{
#do nothing
}
elsif($ack eq 'n')
{
exit;
}
else
{
goto package_label;
}

`cd $package_dir ; dpkg-deb --build debian ; mv debian.deb ../$short_name-$version-$arch.deb`;

print "\n\nComplete.  Your package is now located at ../$short_name-$version-$arch.deb\n\n";
