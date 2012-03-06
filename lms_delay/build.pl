
#!/usr/bin/perl

require "../build-lib.pl";

#change $plugin_name and $clean when you fork a LibModSynth plugin
$plugin_name = "lms_delay.so";
$clean = "sudo rm -R $plugin_path/lms_delay*";

run_script();
