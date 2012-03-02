#!/usr/bin/perl

require "../build-lib.pl";

#change $plugin_name and $clean when you fork a LibModSynth plugin
$plugin_name = "ray_v.so";
$clean = "sudo rm -R $plugin_path/ray_v*";

run_script();
