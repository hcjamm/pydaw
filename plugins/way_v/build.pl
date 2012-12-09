
#!/usr/bin/perl

require "../build-lib.pl";

#change $plugin_name and $clean when you fork a LibModSynth plugin
$plugin_name = "way_v.so";
$clean = "sudo rm -R $plugin_path/way_v*";

run_script();
