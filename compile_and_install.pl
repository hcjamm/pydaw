#!/usr/bin/perl

@plugins = (
"ray_v", "lms_delay", "lms_distortion",
"lms_filter", "lms_comb", "lms_eq5", "euphoria"
);

foreach $val(@plugins)
{
	system("cd plugins/$val ; perl build.pl --full-build ; perl build.pl --install");
	system("rm -Rf /usr/lib/dssi/$val*");
}

print "
Complete.  The plugins have been placed in:
	/usr/local/lib/dssi
";
