#!/usr/bin/perl

$help_text = 
"
The LibModSynth packaging script.  http://libmodsynth.sourceforge.net

This script is for creating a monolithic debian package for any number of individual plugins defined in the \@plugins array in this script. This script can be used as-is for packaging the LMS Suite for a distro.  If you are a developer using LibModSynth, you should edit the values in this script before running it.

usage:
perl build-all.pl ([-y] && ([--ubuntu] || [--debian])) || [--help] 

-y :  Answer yes to all questions, and do not prompt.  However, this will generate generic maintainer info if you haven't run it without the -y switch before.  You must specify an operating system with this switch.

--debian :  Build for the Debian or Ubuntu based distros.  Switches for differnt versions may be added later if dependencies change.  Must be used with -y switch.  If you are not using -y, the script will prompt you for an OS later.

--generic:  Don't create a package, just compile and create a generic installer script.  Use this to install the suite on a machine that doesn't use a supported packaging format.  You must install the following dependencies first:

liblo-dev, dssi-dev, ladspa-sdk, libasound2-dev, qjackctl, libjack-dev | libjack-jackd2-dev, libsndfile1-dev, libsamplerate0-dev, libsm-dev, liblscp-dev, libmad0-dev

Once --install has run, you must run an additional command

If you would like an alternate operating system added, and are willing to track down the dependency package names(and possibly help with the commands for how to create that package type), then please email them to jhubbard651-at-users.sf.net to have them added.

--help:  show this help information

";

#per-distro dependencies.  A special thanks to Glen MacArthur from AV Linux (http://www.bandshed.net/AVLinux.html) 
#for helping me with dependencies that work in Ubuntu and Debian
$debian_deps = "liblo-dev, dssi-dev, ladspa-sdk, libasound2-dev, qjackctl, libsndfile1-dev, libsamplerate0-dev";

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
		elsif($ARGV[1] eq "--generic")
		{
			$depends = $debian_deps; $os = "install"; $package_type = "install";
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

#This is only for printing out extra debug information when running this script.  0 == no debugging, 1 == debugging
$debug_mode = 0;

#Change this if you are compiling your own suite of plugins:
$short_name = "lms_suite";

$deb_name = replace_underscore_with_dash($short_name);

build_all_debug("\$deb_name == $deb_name");

#These are to replace the original packages that the LMS Suite came in.  You can remove these from your own plugins.
$replaces = "ray_v, ray-v, lms_comb, lms-comb, lms_distortion, lms-distortion, lms_delay, lms-delay, lms_filter, lms-filter, euphoria";

#You can probably leave this empty, otherwise you should probably know if any packages conflict
$conflicts = "";

#This is a standard description for the package, change this if packaging your own plugins
$description = "The LMS Suite is a collection of DSSI plugins written using LibModSynth.";

#add any new plugins here, or remove all of them if you are not going to redistribute the official LMS plugins.
#Please take care not to create package conflicts for people with the LMS Suite installed.
@plugins = (
'ray_v',
'lms_comb',
'lms_delay',
'lms_distortion',
'lms_filter',
'lms_eq5',
'euphoria'
);
#Unfinished plugins not yet in the official package:
#lms_dynamics,
#lms_reverb

#Plugins in this array will be symlinked to /usr/lib/ladspa, allowing them to be used by
#hosts that support LADSPA effects but not DSSI.
@ladspa_plugins = (
'lms_comb',
'lms_delay',
'lms_distortion',
'lms_filter',
'lms_eq5'
);

#This is the notes that your package manager will show when browsing your package.  Change this if packaging your own plugins.
$notes = " LibModSynth is a set of developer tools designed to make it fast and easy to develop high quality DSSI plugins.
 The suite currently includes: ";

$first = 1;
foreach $val (@plugins)
{
if($first)
{
$first = 0;
}
else
{
$notes .= ", "
}
$notes .= "$val";
}
#dpkg-deb crashes if EOF happens at the end of a description line
$notes .= ".\n";

#End variables, begin the actual script:


if($prompt)
{
ack_label:
	print "
This will build all of the plugins and package them.  You must edit the list of plugins in this script to include any new ones, or exclude any old ones.  Please take care when packaging the existing LMS plugins not to create package conflicts.  
If you wish only to build a single plugin, use the build.pl scripts in that plugin's directory.  If you wish to package your own collection of LMS-derived plugins, you should edit the \@plugins array in this script to include only your plugins.

Proceed?  (y/[n]): ";

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

os_choice_label:
	print "
Please select the operating system that you are packaging for:
1. Ubuntu/Debian/AVLinux
2. Generic (use this if your OS isn't a variant of one of the above)

Enter choice [1-2]: ";

$os_choice = <STDIN>;
chomp($os_choice);

if($os_choice eq "1")
{
	$depends = $debian_deps; $os = "debian"; $package_type = "deb";
}
elsif($os_choice eq "2")
{
	$depends = $debian_deps; $os = "install"; $package_type = "install";
}
else
{
	print "\n\nInvalid OS choice: $os_choice .  Please select the number of the OS(1, 2, etc...)\n";
	goto os_choice_label;
}

}
require 'build-lib.pl';
if($os ne "install")
{
	#Attempt to install dependencies first
	check_deps();
}

#Here are the directories used for the install, you can modify them if needed.
$base_dir = "$short_name";
$package_dir = "$base_dir/$os";

#Check to see that our system directories are where we think they are.  If not, exit gracefully and ask the user to report it.
if($os eq "install")
{
	if(-e "/usr/bin")
	{
		$bin_dir = "$package_dir/usr/bin";
	}
	else
	{
		print "

Unable to find /usr/bin, the script doesn't know where to place the binary files.
Please report a bug on the LibModSynth sourceforge.net page, and include the name and version of the OS you are running.

";
		exit;
	}

	if(-e "/usr/lib")
	{
		$plugin_dir = "$package_dir/usr/lib/dssi";
	}
	else
	{
		print "

Unable to find /usr/lib, the script doesn't know where to place the plugin files.
Please report a bug on the LibModSynth sourceforge.net page, and include the name and version of the OS you are running.

";
		exit;
	}

	if(-e "/usr/share/pixmaps")
	{
		$icon_dir = "$package_dir/usr/share/pixmaps";
	}
	else
	{
		print "

Unable to find /usr/share/pixmaps, the script doesn't know where to place the icon files.
Please report a bug on the LibModSynth sourceforge.net page, and include the name and version of the OS you are running.

";
		exit;
	}

	if(-e "/usr/share/applications")
	{
		$desktop_dir = "$package_dir/usr/share/applications";
	}
	else
	{
		print "

Unable to find /usr/share/applications, the script doesn't know where to place the .desktop files.
Please report a bug on the LibModSynth sourceforge.net page, and include the name and version of the OS you are running.

";
		exit;
	}
}
else
{
	$bin_dir = "$package_dir/usr/bin";
	$plugin_dir = "$package_dir/usr/lib/dssi";
	$doc_dir = "$package_dir/usr/share/doc/$short_name";
}

if($os ne "install")
{
	$debian_dir = "$package_dir/DEBIAN";
	#At some point, this script may include switches for different distros, which will automatically set these as appropriate.
	#The current values below are valid for Ubuntu, and likely most Debian variants
	$icon_dir = "$package_dir/usr/share/pixmaps";
	$desktop_dir = "$package_dir/usr/share/applications";
}

#print the folder names if debugging enabled
build_all_debug(
"\$base_dir == $base_dir
\$package_dir == $package_dir
\$debian_dir == $debian_dir
\$icon_dir == $icon_dir
\$desktop_dir == $desktop_dir
\$plugin_dir == $plugin_dir
\$bin_dir == $bin_dir
\$doc_dir == $doc_dir");

#Create a clean folder for the plugins to go in
`rm -Rf $package_dir`;
`mkdir -p $package_dir`;
`mkdir -p $plugin_dir`;
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
#copy the .so, .la and LMS_qt files to the directory we created
`mkdir $plugin_dir/$val`;
print "Compiling $val\n";
system("cd $val ; perl build.pl --full-build");

print "Copying files\n";
system("cp $val/src/*_qt $plugin_dir/$val/");
system("cp $val/src/.libs/$val.so $plugin_dir/$val.so");
system("cp $val/src/.libs/$val.la $plugin_dir/$val.la");

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
Categories=AudioVideo;Audio;AudioEditing;";

open (MYFILE, ">>$desktop_dir/$val.desktop");
print MYFILE "$desktop_text";
close (MYFILE);

}

build_all_debug("Building jack-dssi-host");

`cd ../tools/jack-dssi-host ;  perl build.pl --build-jack-host ; cp jack-dssi-host ../../plugins/$bin_dir/lms-jack-dssi-host`;

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

$arch = `uname -i`;
chomp($arch);

if($arch eq "x86_64")
{
$arch = "amd64"; #i386 will already be correct, so there is no elsif for it
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
		open (MYFILE, ">>$short_name-version.txt");
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
"This package was created automatically with the LibModSynth 
built-in packaging script:  build-all.pl, by the following:

$maintainer

LibModSynth is licensed under the GNU GPL version 3.  If this package 
is a derivative work, it must be licensed under a compatible license,
with the full source code available for download.
If not, please send a message to jhubbard651\@users.sf.net.

See /usr/share/common-licenses/GPL-3

Check out the LibModSynth project and view it's source code at:

http://libmodsynth.sourceforge.net/

LibModSynth is Copyright (c) 2012 Jeff Hubbard
Any derivative works are the copyright of their respective owners
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

if($prompt)
{
	package_label:
	print "
The plugins have been compiled and built.  Would you like to package them now?  If you need to modify the files in $debian_dir first, you should choose 'n', and modify them manually.  Once you've done this, you can build the packages with the following commands:

	cd $base_dir
	dpkg-deb --build $os

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
}

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
