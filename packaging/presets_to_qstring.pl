#!/usr/bin/perl

#Script to read a preset file into a C header file
#usage:
#perl presets_to_qstring.pl ~/dssi/(name_of_synth.tsv) > qt_synth_gui_presets.h

local $/ = undef;
open FILE, @argv[0] or die "Couldn't open file: $!";
binmode FILE;
$string = <FILE>;
close FILE;

#Replace tabs and newlines with their escaped versions
$string =~ s/\n/\\n/g;
$string =~ s/\t/\\t/g;

printf "#include <QString>\n\nQString q_preset_string = QString(\"" 
. $string . 
"\");";


