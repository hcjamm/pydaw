#!/usr/bin/perl

#This script is for autogenerating all of the required code for a plugin based on asking
#the user a series of questions.  This only generates the GUI and all of the required framework
#for the GUI to talk to the plugin, but it does not create any of the audio bits.

#Eventually, this script will form the basis for visual design tools that can create everything,
#and then export to C/C++ to be compiled or modified further in an IDE.

create_plugin();

sub create_plugin
{
print 
"
new-plugin.pl :  DSSI plugin auto-generator.  http://libmodsynth.sourceforge.net

This script will ask you a series of questions that will be used to automatically generate C and C++ code for a new DSSI plugin.

The script will only generate the GUI and all of your internal variables, it will not create the audio portion of the plugin.  You can use the LibModSynth C library of audio modules to create the internals of the plugin.

Press enter to start.
";

$dummy = <STDIN>;

print "
Please enter a short name for the plugin.  All letters will be made lowercase, and spaces and hyphens replaced with underscores.
";

$short_name = <STDIN>;
chomp($short_name);
$short_name = lc($short_name);

instrument_or_effect:

print "
Enter 'I' if you are creating an instrument, or 'E' if you are creating an effect.
";

$instrument_or_effect = <STDIN>;
$instrument_or_effect = lc($instrument_or_effect);
chomp($instrument_or_effect);

if(($instrument_or_effect ne "i") && ($instrument_or_effect ne "e"))
{
	goto instrument_or_effect;
}

audio_ins:

print "
How many audio inputs should the plugin have?  Enter a number between 0 and 8.
";

$audio_ins = <STDIN>;
chomp($audio_ins);

if (($audio_ins =~ /[0-8]/ ) && (length($audio_ins) == 1))
{
#...
}
else
{
	goto audio_ins;
}


audio_outs:

print "
How many audio outputs should the plugin have?  Enter a number between 1 and 8.
";

$audio_outs = <STDIN>;
chomp($audio_outs);

if (($audio_outs =~ /[1-8]/ ) && (length($audio_outs) == 1))
{
#...
}
else
{
	goto audio_outs;
}


#creating the GUI

new_groupbox:

print "
Groupboxes contain knobs on the plugin GUI.  Enter 'G' to create a new groupbox, or 'Q' to exit and create your plugin.
";

$groupbox_answer = <STDIN>;
chomp($groupbox_answer);
$groupbox_answer = lc($groupbox_answer);

if($groupbox_answer eq "G")
{}
elsif($groupbox_answer eq "Q")
{}
else
{
	goto new_groupbox;
}

}

sub get_groupbox
{

}

sub get_knob
{

}

sub create_plugin
{

}
