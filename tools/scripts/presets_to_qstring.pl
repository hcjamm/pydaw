#!/usr/bin/perl

#Script to convert a preset file into a C header file, for packaging a default set of presets into a plugin
#usage:
#perl presets_to_qstring.pl ~/dssi/(name_of_synth.tsv) > q_preset_file.h

local $/ = undef;
open FILE, $ARGV[0] or die "Couldn't open file: " .  $ARGV[0];
binmode FILE;
$string = <FILE>;
close FILE;

#Replace tabs and newlines with their escaped versions
$string =~ s/\n/\\n/g;
$string =~ s/\t/\\t/g;

printf "#ifndef Q_PRESET_FILE_H\n#define	Q_PRESET_FILE_H\n" . 
"#include <QString>\n\nQString q_preset_string = QString(\"" .
$string . 
"\");\n#endif";


