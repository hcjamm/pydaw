
#!/usr/bin/perl

require "../build-lib.pl";

#change $plugin_name and $clean when you fork a LibModSynth plugin
$plugin_name = "lms_eq5.so";
$clean = "sudo rm -R $plugin_path/lms_eq5*";

run_script();
