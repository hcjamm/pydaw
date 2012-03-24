#!/usr/bin/perl

require "../build-lib.pl";

#change $plugin_name and $clean when you fork a LibModSynth plugin
$plugin_name = "lms_comb.so";
$clean = "sudo rm -R $plugin_path/lms_comb*";

run_script();
