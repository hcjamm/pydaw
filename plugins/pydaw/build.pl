
#!/usr/bin/perl

require "../build-lib.pl";

#change $plugin_name and $clean when you fork a LibModSynth plugin
$plugin_name = "pydaw.so";
$clean = "sudo rm -R $plugin_path/pydaw*";

run_script();
