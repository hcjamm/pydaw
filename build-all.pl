#!/usr/bin/perl

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

require 'build-all.pl';

#add any new plugins here
@plugins = 
(
'ray_v',
'lms_comb',
'lms_delay',
'lms_distortion',
'lms_filter'
);
#lms_dynamics,
#lms_reverb

$base_dir = "../lms_suite";
$plugin_dir = "$base_dir/deb/usr/lib/dssi";
$debian_dir = "$base_dir/deb/debian";

#Create a clean folder for the plugins to go in
`rm -Rf $base_dir`;
`mkdir -p $plugin_dir`;
`mkdir -p $debian_dir`;

foreach $val(@plugins)
{
#copy the .so, .la and LMS_qt files to the directory we created
`mkdir $plugin_dir/$val`;
print "Compiling $val\n";
`cd $val ; perl build.pl --full-build`;

print "Copying files\n";
system("cp $val/src/LMS_qt $plugin_dir/$val/LMS_qt");
system("cp $val/src/.libs/$val.so $plugin_dir/$val.so");
system("cp $val/src/.libs/$val.la $plugin_dir/$val.la");
}

`cp -r . $base_dir/libmodsynth-git`;

maintainer_label:

if(-e "maintainer.txt")
{
	open FILE, "maintainer.txt" or `rm maintainer.txt` && goto maintainer_label;#die "Couldn't open file: $!"; 
	$maintainer = join("", <FILE>); 
	close FILE;
	chomp($maintainer);
}
else
{
	print "\nThe following questions are required for the package maintainer meta-data.\n\n";
	print "\nPlease enter your email:\n";
	my $email = <STDIN>;
	print "\nPlease enter your first and last name:\n";
	my $name = <STDIN>;

	chomp($email);
	chomp($name);

	$maintainer = $name . " <$email>";

	open (MYFILE, ">>../maintainer.txt");
	print MYFILE "$maintainer";
	close (MYFILE); 
}

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
goto package_label;
}

`cd $debian_dir ; dpkg-deb --build debian ; mv debian.deb lms_suite`;
