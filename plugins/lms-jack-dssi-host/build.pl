
#!/usr/bin/perl

require "../../plugins/build-lib.pl";

#change $plugin_name and $clean when you fork a LibModSynth plugin
$plugin_name = "jack-dssi-host";
$clean = "sudo rm -R $plugin_path/jack-dssi-host*";

run_script();
