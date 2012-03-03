#!/usr/bin/perl

#This script provides functions for the build.pl scripts in each plugin directory.  This prevents the need 
#to duplicate changes to each build.pl script when making changes.

#This script should not be invoked by itself, it is merely a library for the other scripts.

#TODO:  An option to cleanly fork a plugin that will autogenerate the required changes

$help_text = "
The LibModSynth build helper script.  http://libmodsynth.sourceforge.net

Usage:

perl build.pl [args] [compile options]

args:
--full-build 	:  A clean build, rebuilding all autotools files, does not install.
--quick-build 	:  A quick build, does not install.
--install	:  Install using make install
--debug		:  Compile, install and run standalone from the terminal.  Use this to test changes to your code.
--deb 		:  Compile and package the plugin into a .deb file
--ubuntu-deps	:  Install all Ubuntu dependencies
--fork 		:  Fork the current plugin into a new plugin, with updated meta-data and Makfile.
--git-add	:  Adds the appropriate files to a git repository for a forked plugin

compile options:

--native  :  Compile using -march=native .  This optimizes for the machine being compiled on, but the binaries will not be usable on a different machine.  This can give you greater performance if compiling your own plugins, but may introduce bugs.

--sse3    :  Compile for SSE, SSE2 and SSE3.  This is the default option, requires a later Pentium4, Athlon64 or newer machine.

--user-cflags [CFLAGS]  :  Specify your own additional CFLAGS

--no-opt  :  Compile with no optimizations.  Not recommended unless compiling for a non-x86/x64 architecture.

There should be one of these scripts in each plugin directory.

";


$plugin_path = "/usr/local/lib/dssi";
$jack_host = "../jack-dssi-host/jack-dssi-host";
$sleep = "sleep 6";

$makefile = "Makefile";

$deps_ubuntu = "sudo apt-get install liblo-dev dssi-dev ladspa-sdk libasound2-dev g++ qjackctl qt4-designer libjack-jackd2-dev libsndfile1-dev libsamplerate0-dev libtool autoconf libsm-dev uuid-dev cmake liblscp-dev checkinstall libmad0-dev ; sudo usermod -g audio \$USER";

#TODO:  Check for dependencies when running the other arguments, place a file when installed
#TODO:  Place a file in the plugin directory once the first build has been run
sub run_script
{
	if($ARGV[0] eq "--full-build")
	{
		notify_wait();
		first_build();
		notify_done();
	}
	elsif($ARGV[0] eq "--debug")
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
		exec("$jack_host $plugin_path/$plugin_name");
	}
	elsif($ARGV[0] eq "--quick-build")
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
	elsif($ARGV[0] eq "--deb")
	{

		deb_package();
		notify_done();
	}
	elsif($ARGV[0] eq "--install")
	{
		`sudo make install`;
	}
	elsif($ARGV[0] eq "--ubuntu-deps")
	{
		install_deps_ubuntu();
	}
	elsif($ARGV[0] eq "--fork")
	{
		fork_plugin();
	}
	elsif($ARGV[0] eq "--git-add")
	{
		system("git add src/dssi.h src/libmodsynth.h src/Makefile.am src/synth.c src/synth.h src/synth_qt_gui.cpp src/meta.h src/synth_qt_gui.h build.pl Makefile.am configure.ac");
	}
	else
	{
		print $help_text . 'Invalid argument:  $ARGV[0] eq "' . $ARGV[0] . "\"\n\n";
	}

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
`$clean`;
`make clean`;
`$sleep`;
}

sub build
{
#TODO:  test -ffast-math CFLAG
#TODO:  Remove the extra cflags from Makefile.am in ray-v

if($ARGV[1] eq "--native")
{
$make_result = `make CFLAGS+="-O3 -pipe -march=native -mtune=native"`;
}
elsif($ARGV[1] eq "--user-cflags")
{
$user_flags = $ARGV[2];
$make_result = `make CFLAGS+="$user_flags"`;
}
elsif($ARGV[1] eq "--no-opt")
{
$make_result = `make`;
}
else
{
$make_result = `make CFLAGS+="-O3 -msse -msse2 -msse3 -mmmx -pipe"`;
}

#TODO:  Check make result
#TODO:  Properly parse the args at the beginning of the script instead of relying on index

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

#Adding 2 random numbers is more random than 1
my $uuid = int(rand($range)) + int(rand($range)) + $minimum;

chomp($short_name);
chomp($long_name);
chomp($email);
chomp($name);

$short_name =~ s/\s+/_/g;
$short_name =~ s/-/_/g;

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

`cp -R . ../$short_name`;

$meta_dot_h = "
/* This file contains global identifier information for the plugin. */

#ifndef META_H
#define	META_H

#ifdef	__cplusplus
extern \"C\" {
#endif

/*These should be customized for each developer and plugin*/
#define LMS_PLUGIN_NAME \"$short_name\"
#define LMS_PLUGIN_LONG_NAME \"$long_name (Powered by LibModSynth)\";  //Please keep the (Powered by LibModSynth) tag, it helps further the goal of promoting Linux DSSI plugins
#define LMS_PLUGIN_DEV \"$name <$email>\";
#define LMS_PLUGIN_UUID $uuid


#ifdef	__cplusplus
}
#endif

#endif	/* META_H */
";

$makefile_dot_am = "
## Process this file with automake to produce Makefile.in

plugindir = \$(libdir)/dssi

if BUILD_SAMPLER
plugin_LTLIBRARIES = $short_name.la
else
plugin_LTLIBRARIES = $short_name.la
endif

$short_name" . "_la_SOURCES = \\
        synth.c \\
	dssi.h



$short_name" . "_la_CFLAGS = -I\$(top_srcdir)/dssi \$(AM_CFLAGS) \$(ALSA_CFLAGS)

$short_name" . "_la_LDFLAGS = -module -avoid-version
if DARWIN
$short_name" . "_la_LIBADD = -lm -lmx
else
$short_name" . "_la_LIBADD = -lm
endif


if HAVE_LIBLO
if HAVE_QT
lms_ui_PROGRAMS = LMS_qt
else
lms_ui_PROGRAMS =
endif
else
lms_ui_PROGRAMS =
endif

lms_uidir = \$(libdir)/dssi/$short_name

LMS_MOC = synth_qt_gui.moc.cpp

LMS_qt_SOURCES = \\
	synth_qt_gui.cpp \\
	synth_qt_gui.h

nodist_LMS_qt_SOURCES = \$(LMS_MOC)

LMS_qt_CXXFLAGS = \$(AM_CXXFLAGS) \$(QT_CFLAGS) \$(LIBLO_CFLAGS)
LMS_qt_LDADD = \$(AM_LDFLAGS) \$(QT_LIBS) \$(LIBLO_LIBS)



CLEANFILES = \$(BUILT_SOURCES)

# create symlinks for each plugin to jack-dssi-host
#install-exec-hook:

";

$build_dot_pl = "
#!/usr/bin/perl

require \"../build-lib.pl\";

#change \$plugin_name and \$clean when you fork a LibModSynth plugin
\$plugin_name = \"$short_name.so\";
\$clean = \"sudo rm -R \$plugin_path/$short_name*\";

run_script();
";

`rm ../$short_name/src/Makefile.am`;
open (MYFILE, ">>../$short_name/src/Makefile.am");
print MYFILE "$makefile_dot_am";
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
perl build.pl -f

";

}

