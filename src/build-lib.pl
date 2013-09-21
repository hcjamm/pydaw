
$deps_ubuntu = "sudo apt-get install -y python-pyaudio portaudio19-dev liblo-dev g++ libsndfile1-dev libtool gdb debhelper dh-make build-essential automake autoconf python-liblo python-qt4 python2.7 squashfs-tools genisoimage libmad0-dev python-scipy python-numpy libsamplerate0-dev vamp-plugin-sdk ladspa-sdk libfftw3-dev gcc";

#libjack-jackd2-dev qjackctl libsdl1.2-dev ffado-mixer-qt4 ffado-tools ffado-dbus-server

$deps_debian = "sudo apt-get install -y python-pyaudio portaudio19-dev liblo-dev libasound2-dev libsndfile1-dev automake autoconf libtool python-liblo python-qt4 libmad0-dev python-scipy python-numpy libsamplerate0-dev vamp-plugin-sdk ladspa-sdk libfftw3-dev gcc";

#libsdl1.2-dev ffado-mixer-qt4 ffado-tools ffado-dbus-server

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
		print("
Did not detect a Ubuntu or Debian system, dependencies not installed.
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
		print("
Dependencies may need to be installed.

Enter 'y' to attempt to install them automatically, 'n' to continue without installing, or 'q' to quit 

If you select 'y' or 'n', you will not be prompted again to install dependencies.  You can attempt to install the dependencies later with this command:

perl build.pl --deps

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
			else
			{
				#install libSBSMS to /usr/local so that we can build against it
				system("cd pydaw_sbsms && ./configure && make && sudo make install");
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

