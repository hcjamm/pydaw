#!/usr/bin/perl

#This script generates much of the more tedious code for Euphoria.  DSSI plugins aren't very dynamic, you must define
#your I/O ports, etc... at compile time, and this plugin contains approximately 500 ports with $max_samples set to 32, with each port requiring
#various additional code throughout the plugin.  By running this and piping it to a text file, you can generate all of 
#the required code.

$max_samples = 32;
$script_name = "gen_header.pl";

$defines = "\n\n/*defines generated automatically by $script_name*/\n";
$ladspa_descriptor = "\n\n/*LADSPA descriptors generated automatically by $script_name*/\n";
$connect_port = "\n\n/*Port mappings generated automatically by $script_name*/\n";
$slot_defs = "\n\n/*Slot definitions generated automatically by $script_name*/\n";
$signal_defs = "\n\n/*Signal definitions generated automatically by $script_name*/\n";
$slots = "\n\n/*Slots generated automatically by $script_name*/\n\n";
$signals = "\n\n/*Signals generated automatically by $script_name*/\n\n";
$qt_controls = "\n\n/*Controls generated automatically by $script_name*/\n";
$qt_controls_init = "";
$gui_control_handlers = "\n\n/*Control handlers generated automatically by $script_name*/\n";

$lh_min = "LADSPA_HINT_DEFAULT_MINIMUM |";
$lh_max = "LADSPA_HINT_DEFAULT_MAXIMUM |";
$lh_mid = "LADSPA_HINT_DEFAULT_MIDDLE |";

$spinbox = 1; $cb_notes = 2; $cb_fxgrp = 3; $cb_modes = 4; $text = 5;

for($i = 0; $i < $max_samples; $i++)
{
	$i2 = 7 + ($i * 15);

	get_per_sample($i, $i2, "Note", 0, 120, $lh_mid, $cb_notes); $i2++;
	get_per_sample($i, $i2, "Oct", -2, 8, $lh_mid, $spinbox); $i2++;
	get_per_sample($i, $i2, "Lnote", 0, 120, $lh_min, $spinbox); $i2++;
	get_per_sample($i, $i2, "Hnote", 0, 120, $lh_max, $spinbox); $i2++;
	get_per_sample($i, $i2, "Lvel", 1, 127, $lh_min, $spinbox); $i2++;
	get_per_sample($i, $i2, "Hvel", 1, 127, $lh_max, $spinbox); $i2++;
	get_per_sample($i, $i2, "Vol", -50, 36, $lh_mid, $spinbox); $i2++;
	get_per_sample($i, $i2, "Fxgrp", 0, 4, $lh_min, $cb_fxgrp); $i2++;
	get_per_sample($i, $i2, "Mode", 0, 2, $lh_min, $cb_mode); $i2++;
	get_per_sample($i, $i2, "Lsec", 0, 1000, $lh_min, $text); $i2++;
	get_per_sample($i, $i2, "Lsamp", 0, "Sampler_FRAMES_MAX", $lh_min, $text); $i2++;
	get_per_sample($i, $i2, "Start", 0, "Sampler_FRAMES_MAX", $lh_min, $text); $i2++;
	get_per_sample($i, $i2, "End", 0, "Sampler_FRAMES_MAX", $lh_max, $text); $i2++;
	get_per_sample($i, $i2, "Lstart", 0, "Sampler_FRAMES_MAX", $lh_min, $text); $i2++;
	get_per_sample($i, $i2, "Lend", 0, "Sampler_FRAMES_MAX", $lh_max, $text); $i2++;

}

print "\n\n$defines\n\n";
print "\n\n$ladspa_descriptor\n\n";
print "\n\n$connect_port\n\n";
print "\n\n$slot_defs\n\n";
print "\n\n$signal_defs\n\n";
print "\n\n$slots\n\n";
print "\n\n$signals\n\n";
print "\n\n$qt_controls\n\n";
print "\n\n$gui_control_handlers\n\n";

#0 == port define(LMS_SAMPL_XXXX), 1 == description
sub get_ladspa_desc
{
	$port_define = $_[0];
	$min = $_[1];
	$max = $_[2];
	$hint = $_[3];
	$desc = $_[4];
	return "port_descriptors[$port_define] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[$port_define] = \"$desc\";	port_range_hints[$port_define].HintDescriptor = $hint LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE; port_range_hints[$port_define].LowerBound = $min;	 port_range_hints[$port_define].UpperBound = $max;\n";
}

sub get_per_sample
{
	my $i = $_[0];
	my $i2 = $_[1];
	my $ctrl_name_uc = uc($_[2]);
	my $ctrl_name_lc = lc($_[2]);
	my $ctrl_name = $_[2];
	my $lmin = $_[3];
	my $lmax = $_[4];
	my $lhint = $_[5];
	my $define = "LMS_SMPL_$ctrl_name_uc" . "_$i";
	my $signal = "smpl$ctrl_name" . "$i" . "Changed";
	my $slot = "setSmpl$ctrl_name" . "$i";

	if($_[6] == 1) { $control = "QSpinBox"; $set_func = "setValue"; $widget_name = "m_sampl_$ctrl_name_lc" . "$i"; $widget_signal = "valueChanged(int)";}
	elsif($_[6] == 2) { $control = "QComboBox"; $set_func = "setCurrentIndex"; $widget_name = "m_sampl_$ctrl_name_lc" . "$i"; $widget_signal = "currentIndexChanged(int)";
	$qt_extra = "$widget_name->insertItems(0, f_notes_list);\n"}
	elsif($_[6] == 3) { $control = "QComboBox"; $set_func = "setCurrentIndex"; $widget_name = "m_sampl_$ctrl_name_lc" . "$i"; $widget_signal = "currentIndexChanged(int)";
	$qt_extra = "$widget_name->insertItems(0, f_fx_grp_list);\n"}
	elsif($_[6] == 4) { $control = "QComboBox"; $set_func = "setCurrentIndex"; $widget_name = "m_sampl_$ctrl_name_lc" . "$i"; $widget_signal = "currentIndexChanged(int)";
	$qt_extra = "$widget_name->insertItems(0, f_mode_list);\n"}
	elsif($_[6] == 5) { $control = "text"; $set_func = "text"; $widget_name = "m_sampl_$ctrl_name_lc" . "$i"; $widget_signal = "";}

	$defines .= "#define $define = $i2\n";
	$connect_port .= "case $define: plugin->smpl_notes[$i] = data; break;\n";
	$ladspa_descriptor .= get_ladspa_desc($define, 0, 120, $lh_mid, "Sample $ctrl_name $i");
	$gui_control_handlers .= "case $define: gui->$slot(value); break;\n";
	$slots .= "void SamplerGUI::$slot(int a_value){m_suppressHostUpdate = true; $control->$set_func(a_value); m_suppressHostUpdate = false;}\n";
	$signals .= "void SamplerGUI::$signal(int a_value){if(!m_suppressHostUpdate){float v = (float)a_value; lo_send(m_host, m_controlPath, \"if\", $define, v);}}\n";
	$slot_defs .= "$slot(int);\n";
	$signal_defs .= "$signal(int);\n";
	$qt_controls .= "$control * $widget_name;\n"; 
	$qt_controls_init .= "connect($widget_name, SIGNAL($widget_signal), this, SLOT($slot); m_sample_table->setCellWidget($i, 0, $widget_name);";
}

