#!/usr/bin/perl

#TODO:  An option to cleanly fork a plugin that will autogenerate the required changes
$help_text = "
The LibModSynth build helper script.  http://libmodsynth.sourceforge.net

Usage:

perl build.pl [-f (first build)] || [-b (build)] || [-i (install)] || [-s (run standalone)] || 
[-d (build release .deb packages)]  || [-u (install Ubuntu dependencies)]

-f :  A clean build, rebuilding all autotools files, does not install.
-b :  A quick build, does not install.
-i :  Install using make install
-s :  Compile, install and run standalone from the terminal.  Use this to test changes to your code.
-d :  Compile and package the plugin into a .deb file
-u :  Install all Ubuntu dependencies

Only the first argument will be used.  There should be one of these scripts in each plugin directory.

";

 #change $plugin_name when you fork a LibModSynth plugin
$plugin_name = "ray_v.so";
$plugin_path = "/usr/local/lib/dssi/";

$jack_host = "../jack-dssi-host/jack-dssi-host";
$sleep = "sleep 10";

$makefile = "Makefile";

$deps_ubuntu = "sudo apt-get install liblo-dev dssi-dev ladspa-sdk libasound2-dev g++ qjackctl qt4-designer libjack-jackd2-dev libsndfile1-dev libsamplerate0-dev libtool autoconf openjdk-7-jre libsm-dev uuid-dev cmake liblscp-dev checkinstall libmad0-dev ; sudo usermod -g audio \$USER";

#TODO:  Check for dependencies when running the other arguments, place a file when installed
#TODO:  Place a file in the plugin directory once the first build has been run

if($ARGV[0] eq "-f")
{
	notify_wait();
	first_build();
	notify_done();
}
elsif($ARGV[0] eq "-s")
{
	notify_wait();
	unless(-e $jack_host)
	{
		`cd ../jack-dssi-host ; sh ./autotools_script.sh`;
	}

	if(-e $makefile)
	{
		clean();
		build();
	}
	else
	{
		first_build();
	}
	make_install();
	exec("$jack_host $plugin_path$plugin_name");
}
elsif($ARGV[0] eq "-b")
{
	notify_wait();
	if(-e $makefile)
	{
		clean();
		build();
	}
	else
	{
		first_build();
	}
	notify_done();
}
elsif($ARGV[0] eq "-d")
{

	deb_package();
	notify_done();
}
elsif($ARGV[0] eq "-i")
{
	`sudo make install`;
}
elsif($ARGV[0] eq "-u")
{
	install_deps_ubuntu();
}
else
{
	print $help_text . 'Invalid argument:  $ARGV[0] eq "' . $ARGV[0] . "\"\n\n";
}

sub notify_wait
{
print "\nPlease wait, this could take a few minutes...\n";
}

sub notify_done
{
print "\n\nFinished.\n\n";
}

sub first_build
{
check_deps();
clean();
`aclocal`;
`$sleep`;
`libtoolize --force --copy`;
`$sleep`;
`autoheader`;
`$sleep`;
`automake --add-missing --foreign`;
`$sleep`;
`autoconf`;
`$sleep`;
`moc ./src/synth_qt_gui.h -o ./src/synth_qt_gui.moc.cpp`;
`$sleep`;
`./configure`;
`$sleep`;
build();
}

sub clean
{
`sudo rm -R /usr/local/lib/dssi/*`;
`make clean`;
`$sleep`;
}

sub build
{
$make_result = `make`;
#TODO:  Check make result
`$sleep`;
}

sub make_install
{
$install_result = system("sudo make install");
#TODO:  Check install result
}

sub deb_package
{
first_build();
`sudo checkinstall --type=debian --install=no`;
}

#This isn't a real check, it only tests to see if the script attempted to install the dependencies
sub check_deps
{
	unless(-e "../deps_installed.txt")
	{
		install_deps_ubuntu();
	}
}

sub install_deps_ubuntu
{
$check_ubuntu = `uname -v`;
if($check_ubuntu =~ m/ubuntu/i)
{
	`$deps_ubuntu`;
}
else
{	
print("Did not detect a Ubuntu system, dependencies not installed.
If you are running a non-Ubuntu system, please ensure that you have the 
equivalent dependencies on your system:

$deps_ubuntu
");
}
`echo 'This file is created when build.pl attempts to install the dependencies' > ../deps_installed.txt`;
}
