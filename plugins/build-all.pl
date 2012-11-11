#!/usr/bin/perl

$help_text = 
"
The PyDAW interactive packaging script.  https://github.com/j3ffhubb/audiocode

This script is for creating a monolithic debian package for any number of individual plugins defined in the \@plugins array in this script(including PyDAW itself).

usage:
perl build-all.pl 

--help:  show this help information

";

$debian_deps = "liblo-dev, dssi-dev, ladspa-sdk, libasound2-dev, libqt4-dev, libsndfile1-dev, libsamplerate0-dev, libsm-dev, qjackctl, alsa-utils, python-liblo, python-qt4";

$prompt = 1;

if(defined $ARGV[0])
{
	if($ARGV[0] eq "-y")
	{
		$prompt = 0;

		if($ARGV[1] eq "--debian")
		{
			$depends = $debian_deps; $os = "debian"; $package_type = "deb";
		}
		else
		{
			print $help_text;
			print "Invalid operating system argument: \"" . $ARGV[$i] . "\"\n\n";
			exit 1;
		}
	}
	else
	{
		print $help_text;
		exit;
	}
}

$debug_build = 0;

#This is only for printing out extra debug information when running this script.  0 == no debugging, 1 == debugging
$debug_mode = 0;

$short_name = "pydaw";

$deb_name = replace_underscore_with_dash($short_name);

build_all_debug("\$deb_name == $deb_name");

$replaces = "";

#You can probably leave this empty, otherwise you should probably know if any packages conflict
$conflicts = "";

#This is a standard description for the package, change this if packaging your own plugins
$description = "PyDAW is a digital audio workstation with robust MIDI capabilities and a full suite of instrument and effects plugins";

#add any new plugins here, or remove all of them if you are not going to redistribute the official LMS plugins.
#Please take care not to create package conflicts for people with the LMS Suite installed.
@plugins = (
'ray_v',
'euphoria',
'lms_modulex',
'pydaw'
);

#Plugins in this array will be symlinked to /usr/lib/ladspa, allowing them to be used by
#hosts that support LADSPA effects but not DSSI.
@ladspa_plugins = (
'lms_modulex'
);

#This is the notes that your package manager will show when browsing your package.  Change this if packaging your own plugins.
$notes = " PyDAW is a Digital Audio Workstation with a UI written in Python and PyQt4, and a high performance back-end written in C.  It comes with a flexible modular sampler called Euphoria, and a retro analog style synthesizer called Ray-V, as well as numerous built-in effects. ";

#dpkg-deb crashes if EOF happens at the end of a description line
$notes .= "\n";

#End variables, begin the actual script:


if($prompt)
{
#os_choice_label:
#	print "
#Please select the operating system that you are packaging for:
#1. Ubuntu/Debian
#(others not yet supported)
#Enter choice [1-1]: ";

#$os_choice = <STDIN>;
#chomp($os_choice);

$os_choice = "1";

if($os_choice eq "1")
{
	$depends = $debian_deps; $os = "debian"; $package_type = "deb";
}
else
{
	print "\n\nInvalid OS choice: $os_choice .  Please select the number of the OS(1, 2, etc...)\n";
	goto os_choice_label;
}

build_debug_label:
	print "
Build plugins with debug symbols?(makes them much slower, but allows for debugging):
1. Yes
2. No
Enter choice [1-2]: ";

$debug_choice = <STDIN>;
chomp($debug_choice);

if($debug_choice eq "1")
{
	$debug_build = 1;
}
else
{
#Do nothing
}

}


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

#Here are the directories used for the install, you can modify them if needed.
$base_dir = "$short_name-build";
$package_dir = "$base_dir/$os";

$bin_dir = "$package_dir/usr/bin";
#$plugin_dir = "$package_dir/usr/lib/dssi";
$doc_dir = "$package_dir/usr/share/doc/$short_name";
$debian_dir = "$package_dir/DEBIAN";
#At some point, this script may include switches for different distros, which will automatically set these as appropriate.
#The current values below are valid for Ubuntu, and likely most Debian variants
$icon_dir = "$package_dir/usr/share/pixmaps";
$desktop_dir = "$package_dir/usr/share/applications";


#Create a clean folder for the plugins to go in
`rm -Rf $package_dir`;
`mkdir -p $package_dir`;
#`mkdir -p $plugin_dir`;
`mkdir -p $bin_dir`;
`mkdir -p $doc_dir`;
`mkdir -p $icon_dir`;
`mkdir -p $desktop_dir`;

if($os ne "install")
{
	`mkdir -p $debian_dir`;
}

foreach $val(@plugins)
{
#copy the .so and XXX_qt files to the directory we created
print "Compiling $val\n";
if($debug_build)
{
	system("cd $val ; make clean; make CFLAGS+=\" -O0 -g -gdwarf-3 \"");
}
else
{
	system("cd $val ; make clean; make CFLAGS+=\" -O3 \"");
}

print "Copying files\n";
system("cd $val ; make PREFIX=/usr DESTDIR=../$package_dir install");

#Plugins with their own icon can use a file called icon.png in the base directory instead of the LMS icon
if(-e "$val/icon.png")
{
	`cp $val/icon.png $icon_dir/$val.png`;
	$icon_name = "$val.png";
}
else
{
	`cp ../img/libmodsynth.png $icon_dir/libmodsynth.png`;	
	$icon_name = "libmodsynth.png";
}

#TODO:  Copy a .desktop file to the desktop_dir

$desktop_text = 
"[Desktop Entry]
Name=$val
Comment=$description
Exec=lms-jack-dssi-host \"$val.so\"
Icon=$icon_name
Terminal=false
Type=Application
Categories=Audio;AudioEditing;";

open (MYFILE, ">>$desktop_dir/$val.desktop");
print MYFILE "$desktop_text";
close (MYFILE);

}

build_all_debug("Building jack-dssi-host");

if($debug_build)
{
	system("cd lms-jack-dssi-host ; make clean ; perl build.pl --build-jack-host ; cp jack-dssi-host ../$bin_dir/lms-jack-dssi-host");
}
else
{
	system("cd lms-jack-dssi-host ; make clean ;  perl build.pl --build-jack-host-debug ; cp jack-dssi-host ../$bin_dir/lms-jack-dssi-host");
}

if($os eq "install")
{
	print "

Complete.  You can now install $short_name by running the following command as root:

cp -R \"$package_dir/*\" /

";
	exit;
}

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
	if($prompt)
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
	else
	{
		$maintainer = "No Maintainer <nobody\@maintainer.org>";
	}
}

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

if($prompt)
{
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

	
}


$size = `du -s $package_dir/usr`;
$size = (split(" ", $size))[0];
chomp($size);

$debian_control = "
Package: $deb_name
Priority: extra
Section: sound
Installed-Size: $size
Maintainer: $maintainer
Architecture: $arch
Version: $version
Depends: $depends
Provides: $deb_name
Conflicts: $conflicts
Replaces: $replaces
Description: $description
$notes";

open (MYFILE, ">>$debian_dir/control");
print MYFILE "$debian_control";
close (MYFILE); 

#Create the DEBIAN/conffiles file.  TODO:  look into deprecating this, it probably won't ever be needed
open (MYFILE, ">>$debian_dir/conffiles");
print MYFILE "";   #TODO:  Find out what this file is for, and when/if something should ever go in it
close (MYFILE);

#Create the DEBIAN/postinst script
$postinst = 
"#!/usr/bin/perl
#This removes locally compiled/installed copys of the plugins, to avoid conflicts in DSSI hosts

";

foreach $val(@plugins)
{
	#Remove the local versions of the plugin to avoid conflict
	$postinst .= "`rm -Rf /usr/local/lib/dssi/$val*`;\n";
}

foreach $val(@ladspa_plugins)
{
#symlink the effects to the LADSPA plugin directory so that they can be used by LADSPA hosts
$postinst .= 
"`cd /usr/lib/ladspa ; rm -Rf $val.so $val.la $val`;
`ln -s /usr/lib/dssi/$val /usr/lib/ladspa/$val`;
`ln -s /usr/lib/dssi/$val.so /usr/lib/ladspa/$val.so`;
`ln -s /usr/lib/dssi/$val.la /usr/lib/ladspa/$val.la`;\n";
}

$postinst .= "exit 0;";

open (MYFILE, ">>$debian_dir/postinst");
print MYFILE "$postinst";
close (MYFILE); 

`chmod 555 "$debian_dir/postinst"`;

#Create the DEBIAN/md5sums file
`cd $package_dir; find . -type f ! -regex '.*\.hg.*' ! -regex '.*?debian-binary.*' ! -regex '.*?DEBIAN.*' -printf '%P ' | xargs md5sum > DEBIAN/md5sums`;

#Create the copyright file.  TODO: add an option to specify if this is re-packaged, or a derivative work

$copyright_file = 
"This package was created automatically with the PyDAW 
built-in packaging script:  build-all.pl, by the following:

$maintainer

PyDAW is licensed under the GNU GPL version 3.

See /usr/share/common-licenses/GPL-3
";

open (MYFILE, ">>$doc_dir/copyright");
#open (MYFILE, ">>$debian_dir/copyright");  #Debian says put it here, but I haven't found a single other package that does in Ubuntu, so I'm going to do what everybody else is doing
print MYFILE "$copyright_file";
close (MYFILE);

#Copy any documentation to the $doc_dir
if(system("cp -Rf ../doc/* $doc_dir/"))
{
	print "Errors encountered while copying to $doc_dir";
}
else
{
	`rm -Rf $doc_dir/*~`;
}

#Not doing this for now, although Ubuntu 12.10 seems to want it.  Changing ownership to root/root didn't work unless sudo'ed...
#I would expect the package manager to do this on it's own, whether during package creation or afterwards...
#system("sudo chown -R root $package_dir ; sudo chgrp -R root $package_dir");
system("chmod -R 755 $package_dir");

$package_name = "$short_name-$version-$arch.$package_type";

`cd $base_dir ; dpkg-deb --build $os ; rm $package_name ; mv $os.deb $package_name`;

print "\n\nComplete.  Your package is now located at:\n $base_dir/$package_name\n\n";

sub build_all_debug
{
	if($debug_mode)
	{
		print "\nDebug Info:\n" . $_[0] . "\n";
	}
}

#This is because debian packages will only accept dashes in package names
sub replace_underscore_with_dash
{
	$result = $_[0];

	$result =~ s/_/-/g;

	return $result;
}
