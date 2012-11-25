#!/usr/bin/perl

#This script should not be invoked by itself, it is a library for each plugin's build.pl script

#usage:
#require "../build-lib.pl"
#(call subs)

$help_text = "
The LibModSynth build helper script.  http://libmodsynth.sourceforge.net

Usage:

perl build.pl [args]

args:
--full-build 	:  A clean build, rebuilding all autotools files, does not install

--build-debug	:  A clean build, with debug symbols.

--debug		:  Compile with debug symbols and run, using console output.

--gdb		:  Compile with debug symbols and open in gdb

--valgrind	:  Compile with debug symbols and run with valgrind

--coredump	:  Read the most recent core dump for the current plugin directory in GDB(requires that you ran the plugin with --debug and experienced a SEGFAULT)

--run		:  Compile without debug symbols and with optimizations, and then run.

--deb 		:  Compile and package the plugin into a .deb file (this uses checkinstall, you should use the build-all.pl script instead, using the LibModSynth native packaging system)

--deps		:  Install all dependencies.  Some operating systems are not yet supported

--fork 		:  Fork the current plugin into a new plugin, with updated meta-data and Makefile

--git-add	:  Adds the appropriate files to a git repository for a forked plugin using 'git add [files]'.  Use this to avoid adding unnecessary GNU autotools files to a git repository.

--scm-add-show	:  Display a list of files that should be added to an alternative source code management system such as SVN, Mercurial, CVS, etc...  This only displays the files you should be adding, it does not add them for you.

--build-jack-host  :  This is for compiling the jack-dssi-host.  The user should never have to run this, the script will run it automatically when needed.

--build-jack-host-debug  :  This is for compiling the jack-dssi-host with debug symbols.

Note that you can also debug using the included debugger project and GDB, using the IDE of your choice.  See doc/instructions.txt for details.  GDB is more suitable for using breakpoints to step through code, whereas the console output is more suitable for getting a glimpse of what's going on in your plugin while you are using it in jack-dssi-host.

There should be one build.pl script in each plugin directory.

";


$plugin_path = "/usr/local/lib/dssi";
$jack_host_dir = "../lms-jack-dssi-host";
$jack_host = "$jack_host_dir/jack-dssi-host";
$debug_dir = "../bin";

unless(-e $debug_dir)
{
`mkdir $debug_dir`;
}

$debug_flags = ' CFLAGS+=" -O0 -g -gdwarf-3 " ';
$release_flags = ' CFLAGS+=" -O3 " ';

$current_dir = get_current_dir();
$dssi_path = `cd $debug_dir ; pwd`;
chomp($dssi_path);

$sleep = "sleep 1";

$makefile = "Makefile";

$deps_ubuntu = "sudo apt-get install -y liblo-dev dssi-dev ladspa-sdk libasound2-dev g++ libqt4-dev libjack-jackd2-dev libsndfile1-dev libtool autoconf libsm-dev gdb debhelper dh-make build-essential automake autoconf libtool qjackctl alsa-utils python-liblo python-qt4 git libsdl1.2-dev ffado-mixer-qt4 ffado-tools ffado-dbus-server";

$deps_debian = "sudo apt-get install -y liblo-dev dssi-dev ladspa-sdk libasound2-dev libqt4-dev libjack-dev libsndfile1-dev libsm-dev automake autoconf libtool qjackctl alsa-utils python-liblo python-qt4 git libsdl1.2-dev ffado-mixer-qt4 ffado-tools ffado-dbus-server";

#This isn't currently in use, due to the potential for conflicts it may cause.
$audio_group = "sudo usermod -g audio \$USER";

#TODO:  Check for dependencies when running the other arguments, place a file when installed
#TODO:  Place a file in the plugin directory once the first build has been run
sub run_script
{
	if($ARGV[0] eq "--build")
	{
		check_deps();
		notify_wait();
		first_build($release_flags);
		notify_done();
	}
	if($ARGV[0] eq "--build-debug")
	{
		check_deps();
		notify_wait();
		first_build($debug_flags);
		notify_done();
	}
	elsif($ARGV[0] eq "--coredump")
	{
		system("gdb lms-jack-dssi-host core");
	}
	elsif($ARGV[0] eq "--debug" or $ARGV[0] eq "--gdb" or $ARGV[0] eq "--valgrind" or $ARGV[0] eq "--run")
	{
		check_deps();
		notify_wait();
		$local_jack_host = "./lms-jack-dssi-host";
		if(! -e $local_jack_host)
		{
			system("cd $jack_host_dir ; perl build.pl --build-jack-host-debug ");
			system("cp -f $jack_host $local_jack_host");
		}
		

		system("make clean");
		if($ARGV[0] eq "--run")
		{
			build($release_flags);
		}
		else
		{
			build($debug_flags);
		}
		
		`rm -Rf ../bin/*`; #Cleanup the bin directory
		`rm -Rf ./core`; #Delete any core dumps from previous sessions, gdb seems to be acting up when the previous core dump isn't deleted
		`make PREFIX=/usr DESTDIR=$dssi_path install`;
		if($ARGV[0] eq "--debug" or $ARGV[0] eq "--run"){
			exec("ulimit -c unlimited; export DSSI_PATH=\"$dssi_path/usr/lib/dssi\" ; $local_jack_host $current_dir.so");
		} elsif($ARGV[0] eq "--gdb") {
			print("\n\n\n****Type 'run $current_dir.so' at the (gdb) prompt***\n\n\n");
			system("ulimit -c unlimited; export DSSI_PATH=\"$dssi_path/usr/lib/dssi\" ; gdb $local_jack_host");
		}
		elsif($ARGV[0] eq "--valgrind") {
			system("ulimit -c unlimited; export DSSI_PATH=\"$dssi_path/usr/lib/dssi\" ; valgrind $local_jack_host $current_dir.so");
		}
			
	}
	elsif($ARGV[0] eq "--build-jack-host")
	{
		check_deps();
		notify_wait();

		build_jack_host($release_flags);

		notify_done();
	}
	elsif($ARGV[0] eq "--build-jack-host-debug")
	{
		check_deps();
		notify_wait();

		build_jack_host($debug_flags);

		notify_done();
	}
	elsif($ARGV[0] eq "--deb")
	{
		build_package("debian");
		notify_done();
	}
	elsif($ARGV[0] eq "--deps")
	{
		install_deps();
	}
	elsif($ARGV[0] eq "--fork")
	{
		fork_plugin();
	}
	elsif($ARGV[0] eq "--git-add")
	{
		system("git add src/dssi.h src/libmodsynth.h Makefile src/synth.c src/synth.h src/synth_qt_gui.cpp src/meta.h src/synth_qt_gui.h build.pl");
	}
	elsif($ARGV[0] eq "--scm-add-show")
	{
		print "\n\n add src/dssi.h src/libmodsynth.h Makefile src/synth.c src/synth.h src/synth_qt_gui.cpp src/meta.h src/synth_qt_gui.h build.pl\n\n";
	}
	else
	{
		print $help_text . 'Invalid argument:  $ARGV[0] eq "' . $ARGV[0] . "\"\n\n";
	}

}
sub notify_wait
{
print "\nPlease wait, this could take a few minutes...\n\n";
}

sub notify_done
{
print "\n\nFinished.\n\n";
}

sub first_build
{
check_deps();
clean();
build($_[0]);
}

sub clean
{
#`$clean`;
`make clean`;
`$sleep`;
}

#The first argument passed in is any additional CFLAGS
sub build
{
#system("moc -o src/synth_qt_gui.moc.cpp src/synth_qt_gui.h");
$make = 'make ';


if(defined $_[0])
{
	$make .= $_[0];
}

$make_result = system($make);

if($make_result)
{
	print "\n\nError, \$make_result == $make_result
Cannot compile, aborting script, please check your code for errors\n\n";
	exit;
}

}


sub build_jack_host
{
system("make clean");

if(defined $_[0])
{
	$make = "make " . $_[0];
	system("$make");
}
else
{
	system("make");
}

}

#This isn't a real check, it only tests to see if the script attempted to install the dependencies.  Valid on Ubuntu only.
sub check_deps
{
	#check both directories, because this can be invoked from the plugin's directory or the base directory
	unless((-e "../deps_installed.txt") || (-e "deps_installed.txt"))
	{
		install_deps();
	}
}

sub install_deps
{
$supported_os = 0;
$check_os = `uname -v`;
if($check_os =~ m/ubuntu/i)
{
	$supported_os = 1;
	$deps_os = "ubuntu";
}
elsif(($check_os =~ m/debian/i) || ($check_os =~ m/avl/i))
{
	$supported_os = 1;
	$deps_os = "debian";
}
else
{	
print("Did not detect a Ubuntu or Debian system, dependencies not installed.
If you are running a non-Ubuntu system, please ensure that you have the 
equivalent dependencies on your system:

$deps_ubuntu

Press enter to continue, or 'q' to exit.
");
$quit_answer = <STDIN>;
chomp($quit_answer);
$quit_answer = lc($quit_answer);

if($quit_answer eq "q")
{
	exit;
}

}

if($supported_os)
{
	ask_deps:
	print(
"


Dependencies may need to be installed.

Enter 'y' to attempt to install them automatically, 'n' to continue without installing, or 'q' to quit 

If you select 'y' or 'n', you will not be prompted again to install dependencies.  You can attempt to install the dependencies later with this command:

perl build.pl --deps

Also, you may want to consider adding yourself to the audio group with this command:

$audio_group

Attempt to install dependencies? [n]:\n");

	$install_answer = <STDIN>;
	chomp($install_answer);
	$install_answer = lc($install_answer);

	if(($install_answer eq "y"))
	{
		if($deps_os eq "ubuntu"){		
			$deps_result = system("$deps_ubuntu");
		}
		elsif($deps_os eq "debian"){
			$deps_result = system("$deps_debian");
		}

		if($deps_result)
		{
			print("\nInstalling dependencies returned $deps_result.  The required dependencies may not have installed correctly.  If you think this is a bug, please report it.\n\n");
		}
	}
	elsif(($install_answer eq "n") || ($install_answer eq ""))
	{}
	elsif($install_answer eq "q")
	{
		exit;
	}
	else
	{
		goto ask_deps;
	}
}

if(get_current_dir() eq "plugins")
{
`echo 'This file is created when build.pl attempts to install the dependencies' > deps_installed.txt`;
}
else
{
`echo 'This file is created when build.pl attempts to install the dependencies' > ../deps_installed.txt`;
}

}

#Cleanly fork a plugin into a new plugin, complete with updated Makefiles and DSSI meta-data
sub fork_plugin
{
get_values:
print "\nPlease enter a short name for the plugin.  Spaces and dashes will be replaced with underscores, and letters will be lower-case:\n";
my $short_name = <STDIN>;
print "\nPlease enter a long name for the plugin.  Use proper capitalization and spacing:\n";
my $long_name = <STDIN>;
print "The next 2 questions are for the maintainer meta-data.  Just make something up if you don't want to put your real information into the plugins meta-data.\n";
print "\nPlease enter your email:\n";
my $email = <STDIN>;
print "\nPlease enter your first and last name:\n";
my $name = <STDIN>;

my $range = 687651;
my $minimum = 12740;

#Adding 2 random numbers is more random than 1, digital random numbers aren't completely random.
my $uuid = int(rand($range)) + int(rand($range)) + $minimum;

chomp($short_name);
chomp($long_name);
chomp($email);
chomp($name);

$short_name =~ s/\s+/_/g;
$short_name =~ s/-/_/g;

$gui_name = uc($short_name) . "_qt";

$short_name = lc($short_name);

print "\n\n\$short_name == $short_name\n\$long_name == $long_name\n\$email == $email\n\$name == $name\n\$uuid == $uuid\n\n";

yes_or_no:
print "Enter y to accept, n to re-enter, or q to quit [y]:\n";

my $answer = <STDIN>;
chomp($answer);
$answer = lc($answer);

if(($answer eq "y") || ($answer eq ""))
{

}
elsif($answer eq "n")
{
goto get_values;
}
elsif($answer eq "q")
{
exit;
}
else
{
goto yes_or_no;
}
#delete any files that will clutter up the new project
`make clean`;
`cp -R . ../$short_name`;

$meta_dot_h = "
/* This file contains global identifier information for the plugin. */

#ifndef META_H
#define	META_H

#ifdef	__cplusplus
extern \"C\" {
#endif

/*These should be customized for each developer and plugin*/
#define LMS_PLUGIN_NAME \"" . uc($short_name) . "\"
#define LMS_PLUGIN_LONG_NAME \"$long_name\";
#define LMS_PLUGIN_DEV \"$name <$email>\";
#define LMS_PLUGIN_UUID $uuid

#ifdef	__cplusplus
}
#endif

#endif	/* META_H */
";

$makefile_text = "
#!/usr/bin/make -f

CC  ?= gcc
CXX ?= g++
MOC ?= moc

PREFIX ?= /usr/local

BASE_FLAGS     = -ffast-math -fPIC -msse -msse2 -msse3 -mfpmath=sse -Wall -Isrc -I.
BUILD_CFLAGS   = \$(BASE_FLAGS) \$(CFLAGS)
BUILD_CXXFLAGS = \$(BASE_FLAGS) \$(shell pkg-config --cflags liblo QtCore QtGui x11 sm sndfile) \$(CXXFLAGS)
LINK_CFLAGS    = -shared -lm \$(LDFLAGS) \$(shell pkg-config --libs liblo alsa sndfile samplerate)
LINK_CXXFLAGS  = -lm -pthread \$(shell pkg-config --libs liblo QtCore QtGui x11 sm sndfile) \$(LDFLAGS)

C_OBJS   = src/synth.o
CXX_OBJS = src/synth_qt_gui.o src/moc_synth_qt_gui.o

# --------------------------------------------------------------

all: $short_name.so $gui_name

$short_name.so: \$(C_OBJS)
	\$(CC) \$(C_OBJS) \$(LINK_CFLAGS) -o \$@

$gui_name: \$(CXX_OBJS)
	\$(CXX) \$(CXX_OBJS) \$(LINK_CXXFLAGS) -o \$@

# --------------------------------------------------------------

.c.o:
	\$(CC) -c \$< \$(BUILD_CFLAGS) -o \$@

.cpp.o:
	\$(CXX) -c \$< \$(BUILD_CXXFLAGS) -o \$@

src/moc_synth_qt_gui.cpp: src/synth_qt_gui.h
	\$(MOC) \$< -o \$@

# --------------------------------------------------------------

install:
	install -d \$(DESTDIR)\$(PREFIX)/lib/dssi
	install -d \$(DESTDIR)\$(PREFIX)/lib/dssi/$short_name
	install -m 644 $short_name.so \$(DESTDIR)\$(PREFIX)/lib/dssi
	install -m 755 $gui_name \$(DESTDIR)\$(PREFIX)/lib/dssi/$short_name

clean:
	rm -f src/*.o src/moc_*.cpp *.so $gui_name
";

$build_dot_pl = "
#!/usr/bin/perl

require \"../build-lib.pl\";

#change \$plugin_name and \$clean when you fork a LibModSynth plugin
\$plugin_name = \"$short_name.so\";
\$clean = \"sudo rm -R \$plugin_path/$short_name*\";

run_script();
";

`rm ../$short_name/Makefile`;
open (MYFILE, ">>../$short_name/Makefile");
print MYFILE "$makefile_text";
close (MYFILE); 

`rm ../$short_name/src/meta.h`;
open (MYFILE, ">>../$short_name/src/meta.h");
print MYFILE "$meta_dot_h";
close (MYFILE);

`rm ../$short_name/build.pl`;
open (MYFILE, ">>../$short_name/build.pl");
print MYFILE "$build_dot_pl";
close (MYFILE);

print "

The plugin has been forked.  You should now rebuild it with the following commands:

cd ../$short_name
perl build.pl --full-build

You should delete any IDE-specific project folders before opening it in an IDE, as it could interfere with the original project.
";
}

#Return the current array.  For example:
#$value = get_current_dir();  #$value is now "ray_v" if invoked from "libmodsynth-git/ray_v"
sub get_current_dir
{
	my $result = `pwd`;
	chomp($result);
	my @split_arr = (split("/", $result));

	#The end result is that the last value in the array is the result
	foreach my $val (@split_arr)
	{
		$result = $val;
	}

	return $result;
}

