#!/usr/bin/perl

use Switch;

$jack_host = "../jack-dssi-host/jack-dssi-host";

#TODO:  An option to install Ubuntu dependencies
#TODO:  An option to cleanly fork a plugin that will autogenerate the required changes
$helptext = "Usage:\nperl build.pl [-f (first build)][-b (build)][-s (run standalone)][-d (build release .deb packages)]\n\n";

 #change $plugin_name when you fork a LibModSynth plugin
$plugin_name = "ray_v.so";
$plugin_path = "/usr/local/lib/dssi/";

$quick_build = "sh ./quick_build.sh";
$full_build = "sh ./autotools_script.sh";

if($ARGV[0] eq "-f")
{
	print "\nPlease wait, this could take a few minutes...\n";
	print `$full_build`;
}
elsif($ARGV[0] eq "-s")
{
	print "\nPlease wait, this could take a few minutes...\n";
	unless(-e $jack_host)
	{
		print `cd ../jack-dssi-host ; sh ./autotools_script.sh`;
	}
	print `$quick_build`;
	print `$jack_host $plugin_path$plugin_name`;
}
elsif($ARGV[0] eq "-b")
{
	print "\nPlease wait, this could take a few minutes...\n";
	print `$quick_build`;
}
elsif($ARGV[0] eq "-d")
{
	print "\nPlease wait, this could take a few minutes...\n";
	print `$full_build`;
	print `sudo checkinstall --install=no --type=debian`;
}
else
{
	print $help_text . '$ARGV[0] eq ' . $ARV[0] . "\n\n";
}



