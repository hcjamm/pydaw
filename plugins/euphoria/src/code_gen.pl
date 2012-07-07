#!/usr/bin/perl

$sample_count = 32;
$class_name = "SamplerGUI";
$blurb = ".  Created by src/code_gen.pl ";

$mod_matrix_knobs_per_control = 3;
$mod_matrix_control_count = 4;
$mod_matrix_mod_sources = 4;
$fx_group_count = 1;

$mod_matrix_first_control_port = 35;

$slots = "";
$slot_defs = "";
$connections = "";

$ports = "";
$port_descriptors = "";
$connect_ports = "";

for($i = 0; $i < $sample_count; $i++)
{
my $slot = "sample_pitch$i" . "Changed(int)";
my $slot_def = "sample_pitch$i" . "Changed(int a_value)";
my $function = "sample_pitchChanged($i);";
$slots .= "\tvoid $slot;\n";
$slot_defs .= "void $class_name" . "::" . "$slot_def" . "{$function}\n";
$connections .= "\t\t\tconnect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[$i]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT($slot));\n";
$connections .= "\t\t\tconnect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[$i]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT($slot));\n";
}

for($i = 0; $i < $sample_count; $i++)
{
my $slot = "sample_lnote$i" . "Changed(int)";
my $slot_def = "sample_lnote$i" . "Changed(int a_value)";
my $function = "sample_lnoteChanged($i);";
$slots .= "\tvoid $slot;\n";
$slot_defs .= "void $class_name" . "::" . "$slot_def" . "{$function}\n";
$connections .= "\t\t\tconnect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[$i]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT($slot));\n";
$connections .= "\t\t\tconnect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[$i]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT($slot));\n";
}

for($i = 0; $i < $sample_count; $i++)
{
my $slot = "sample_hnote$i" . "Changed(int)";
my $slot_def = "sample_hnote$i" . "Changed(int a_value)";
my $function = "sample_hnoteChanged($i);";
$slots .= "\tvoid $slot;\n";
$slot_defs .= "void $class_name" . "::" . "$slot_def" . "{$function}\n";
$connections .= "\t\t\tconnect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[$i]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT($slot));\n";
$connections .= "\t\t\tconnect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[$i]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT($slot));\n";
}

for($i = 0; $i < $sample_count; $i++)
{
my $slot = "sample_vol$i" . "Changed(int)";
my $slot_def = "sample_vol$i" . "Changed(int a_value)";
my $function = "sample_volChanged($i);";
$slots .= "\tvoid $slot;\n";
$slot_defs .= "void $class_name" . "::" . "$slot_def" . "{$function}\n";
$connections .= "\t\t\tconnect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[$i]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT($slot));\n";
}

for($i = 0; $i < $sample_count; $i++)
{
my $slot = "sample_vel_sens$i" . "Changed(int)";
my $slot_def = "sample_vel_sens$i" . "Changed(int a_value)";
my $function = "sample_vel_sensChanged($i);";
$slots .= "\tvoid $slot;\n";
$slot_defs .= "void $class_name" . "::" . "$slot_def" . "{$function}\n";
$connections .= "\t\t\tconnect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_SENS_INDEX]->controls[$i]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT($slot));\n";
}

for($i = 0; $i < $sample_count; $i++)
{
my $slot = "sample_vel_low$i" . "Changed(int)";
my $slot_def = "sample_vel_low$i" . "Changed(int a_value)";
my $function = "sample_vel_lowChanged($i);";
$slots .= "\tvoid $slot;\n";
$slot_defs .= "void $class_name" . "::" . "$slot_def" . "{$function}\n";
$connections .= "\t\t\tconnect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_LOW_INDEX]->controls[$i]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT($slot));\n";
}

for($i = 0; $i < $sample_count; $i++)
{
my $slot = "sample_vel_high$i" . "Changed(int)";
my $slot_def = "sample_vel_high$i" . "Changed(int a_value)";
my $function = "sample_vel_highChanged($i);";
$slots .= "\tvoid $slot;\n";
$slot_defs .= "void $class_name" . "::" . "$slot_def" . "{$function}\n";
$connections .= "\t\t\tconnect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[$i]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT($slot));\n";
}




$mod_matrix_knobs_per_control = 3;
$mod_matrix_control_count = 4;
$mod_matrix_mod_sources = 4;

$current_control_port = $mod_matrix_first_control_port;
for($i4 = 0; $i4 < $fx_group_count; $i4++)
{
for($i2 = 0; $i2 < $mod_matrix_control_count; $i2++)
{
for($i = 0; $i < $mod_matrix_mod_sources; $i++)
{
for($i3 = 0; $i3 < $mod_matrix_knobs_per_control; $i3++)
{

my $port_name = "LMS_PFXMATRIX_GRP$i4". "DST$i2" . "SRC$i" . "CTRL$i3";
my $port = "$port_name  $current_control_port";
$current_control_port++;

$ports .= "#define $port\n";

$port_descriptors .= 
"	port_descriptors[$port_name] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL; port_names[$port_name] = \"$port_name\";
	port_range_hints[$port_name].HintDescriptor = LADSPA_HINT_DEFAULT_MIDDLE | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[$port_name].LowerBound =  -100; port_range_hints[$port_name].UpperBound =  100;\n\n";

my $lc_name = "pfxmatrix_grp$i4" . "dst$i2" . "src$i" . "ctrl$i3";
my $slot = $lc_name . "Changed(int)";
my $slot_def = $lc_name . "Changed(int a_value)";
my $function = "pfxmatrix_Changed($i4, $i, $i2, $i3);";
$slots .= "\tvoid $slot;\n";
$slot_defs .= "void $class_name" . "::" . "$slot_def" . "{$function}\n";
$connections .= "\t\t\tconnect((QDial*)(m_polyfx_mod_matrix[$i4]->lms_mm_columns[" . (($i2 * $mod_matrix_knobs_per_control) + $i3) . "]->controls[$i]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT($slot));\n";

$connect_ports .= "case $port_name: plugin->polyfx_mod_matrix[$i4][$i2][$i][$i3] = data; break;\n";

}
}
}
}

print "
/*synth_qt_gui.h Autogenerated slots$blurb*/

$slots
/*End synth_qt_gui.h Autogenerated slots$blurb*/

/*synth_qt_gui.cpp Autogenerated connections$blurb*/

$connections
/*End synth_qt_gui.cpp Autogenerated connections$blurb*/

/*synth_qt_gui.cpp Autogenerated slots$blurb*/

$slot_defs
/*End synth_qt_gui.cpp Autogenerated slots$blurb*/

/*Ports*/
$ports

/*Port descriptors*/
$port_descriptors

/*Port connections*/
$connect_ports

";

