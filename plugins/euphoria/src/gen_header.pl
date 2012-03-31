#!/usr/bin/perl

$max_samples = 32;

$defines = "";
$struct = "";
$qdials_h = "//Mod matrix knobs\n";
$qdials_cpp = "//Mod matrix knobs\n";
$ladspa_descriptor = "";
$connect_port = "\n\n/*Begin auto-generated ports*/\n";
$midi_cc = "";  #else if (port == Sampler_BASE_PITCH) return DSSI_CC(13);

$lh_min = "LADSPA_HINT_DEFAULT_MINIMUM |";
$lh_max = "LADSPA_HINT_DEFAULT_MAXIMUM |";
$lh_mid = "LADSPA_HINT_DEFAULT_MIDDLE |";

$port_define = $_[0];
$min = $_[1];
$max = $_[2];
$hint = $_[3];
$desc = $_[4];

for($i = 1; $i <= $max_samples; $i++)
{
	$i2 = 7 + ($i * 15);

	$define = "LMS_SMPL_NOTE_$i";
	$ladspa = "smpl_note_$i";
	$defines .= "#define $define = $i2\n";  $i2++;
	$struct .= "LADSPA_Data * $ladspa;\n";
	$connect_port .= "case $define: plugin->$ladspa = data; break;\n";
	$ladspa_descriptor .= get_ladspa_desc($define, 0, 120, $lh_mid, "Sample Note $i");

	$define = "LMS_SMPL_OCT_$i";
	$ladspa = "smpl_oct_$i";
	$defines .= "#define $define = $i2\n";  $i2++;
	$struct .= "LADSPA_Data * $ladspa;\n";
	$connect_port .= "case $define: plugin->$ladspa = data; break;\n";
	$ladspa_descriptor .= get_ladspa_desc($define, -2, 8, $lh_mid, "Sample Octave $i");

	$define = "LMS_SMPL_LNOTE_$i";
	$ladspa = "smpl_lnote_$i";
	$defines .= "#define $define = $i2\n";  $i2++;
	$struct .= "LADSPA_Data * $ladspa;\n";
	$connect_port .= "case $define: plugin->$ladspa = data; break;\n";
	$ladspa_descriptor .= get_ladspa_desc($define, 0, 120, $lh_min, "Sample Low Note $i");

	$define = "LMS_SMPL_HNOTE_$i";
	$ladspa = "smpl_hnote_$i";
	$defines .= "#define $define = $i2\n";  $i2++;
	$struct .= "LADSPA_Data * $ladspa;\n";
	$connect_port .= "case $define: plugin->$ladspa = data; break;\n";
	$ladspa_descriptor .= get_ladspa_desc($define, 0, 120, $lh_max, "Sample High Note $i");

	$define = "LMS_SMPL_LVEL_$i";
	$ladspa = "smpl_lvel_$i";
	$defines .= "#define $define = $i2\n";  $i2++;
	$struct .= "LADSPA_Data * $ladspa;\n";
	$connect_port .= "case $define: plugin->$ladspa = data; break;\n";
	$ladspa_descriptor .= get_ladspa_desc($define, 1, 127, $lh_min, "Sample Low Velocity $i");

	$define = "LMS_SMPL_HVEL_$i";
	$ladspa = "smpl_hvel_$i";
	$defines .= "#define $define = $i2\n";  $i2++;
	$struct .= "LADSPA_Data * $ladspa;\n";
	$connect_port .= "case $define: plugin->$ladspa = data; break;\n";
	$ladspa_descriptor .= get_ladspa_desc($define, 1, 127, $lh_max, "Sample High Velocity $i");

	$define = "LMS_SMPL_VOL_$i";
	$ladspa = "smpl_vol_$i";
	$defines .= "#define $define = $i2\n";  $i2++;
	$struct .= "LADSPA_Data * $ladspa;\n";
	$connect_port .= "case $define: plugin->$ladspa = data; break;\n";
	$ladspa_descriptor .= get_ladspa_desc($define, -50, 36, "LADSPA_HINT_DEFAULT_HIGH | ", "Sample Volume $i");

	$define = "LMS_SMPL_FXGRP_$i";
	$ladspa = "smpl_fxgrp_$i";
	$defines .= "#define $define = $i2\n";  $i2++;
	$struct .= "LADSPA_Data * $ladspa;\n";
	$connect_port .= "case $define: plugin->$ladspa = data; break;\n";
	$ladspa_descriptor .= get_ladspa_desc($define, 0, 4, $lh_min, "Sample FX Group $i");

	$define = "LMS_SMPL_MODE_$i";
	$ladspa = "smpl_mode_$i";
	$defines .= "#define $define = $i2\n";  $i2++;
	$struct .= "LADSPA_Data * $ladspa;\n";
	$connect_port .= "case $define: plugin->$ladspa = data; break;\n";
	$ladspa_descriptor .= get_ladspa_desc($define, 0, 2, $lh_min, "Sample Mode $i");

	$define = "LMS_SMPL_LSEC_$i";
	$ladspa = "smpl_lsec_$i";
	$defines .= "#define $define = $i2\n";  $i2++;
	$struct .= "LADSPA_Data * $ladspa;\n";
	$connect_port .= "case $define: plugin->$ladspa = data; break;\n";
	$ladspa_descriptor .= get_ladspa_desc($define, 0, 1000, $lh_min, "Sample Length Seconds $i");


	$define = "LMS_SMPL_LSAMP_$i";
	$ladspa = "smpl_lsamp_$i";
	$defines .= "#define $define = $i2\n";  $i2++;
	$struct .= "LADSPA_Data * $ladspa;\n";
	$connect_port .= "case $define: plugin->$ladspa = data; break;\n";
	$ladspa_descriptor .= get_ladspa_desc($define, 0, 1000, $lh_mid, "Sample Lenth Samples $i");

	$define = "LMS_SMPL_START_$i";
	$ladspa = "smpl_start_$i";
	$defines .= "#define $define = $i2\n";  $i2++;
	$struct .= "LADSPA_Data * $ladspa;\n";
	$connect_port .= "case $define: plugin->$ladspa = data; break;\n";
	$ladspa_descriptor .= get_ladspa_desc($define, 0, "Sampler_FRAMES_MAX", $lh_min, "Sample Start $i");


	$define = "LMS_SMPL_END_$i";
	$ladspa = "smpl_end_$i";
	$defines .= "#define $define = $i2\n";  $i2++;
	$struct .= "LADSPA_Data * $ladspa;\n";
	$connect_port .= "case $define: plugin->$ladspa = data; break;\n";
	$ladspa_descriptor .= get_ladspa_desc($define, 0, "Sampler_FRAMES_MAX", $lh_max, "Sample End $i");

	$define = "LMS_SMPL_LSTART_$i";
	$ladspa = "smpl_lstart_$i";
	$defines .= "#define $define = $i2\n";  $i2++;
	$struct .= "LADSPA_Data * $ladspa;\n";
	$connect_port .= "case $define: plugin->$ladspa = data; break;\n";
	$ladspa_descriptor .= get_ladspa_desc($define, 0, "Sampler_FRAMES_MAX", $lh_min, "Sample Loop Start $i");

	$define = "LMS_SMPL_LEND_$i";
	$ladspa = "smpl_lend_$i";
	$defines .= "#define $define = $i2\n";  $i2++;
	$struct .= "LADSPA_Data * $ladspa;\n";
	$connect_port .= "case $define: plugin->$ladspa = data; break;\n";
	$ladspa_descriptor .= get_ladspa_desc($define, 0, "Sampler_FRAMES_MAX", $lh_max, "Sample Loop End $i");
}

#for each row header
for($i4 = 0; $i4 < 5; $i4++)
{
	$i3 = 0;  #Column index
	#for each FX group
	for($i = 1; $i <= 4; $i++)
	{	
		#for each FX knob
		for($i2 = 1; $i2 <= 3; $i2++)
		{
#m_fx_knob_4_2 = new QDial(groupBox_5);
#m_fx_knob_4_2->setObjectName(QString::fromUtf8("m_fx_knob_4_2"));
#m_fx_knob_4_2->setMinimumSize(QSize(48, 48));
#m_fx_knob_4_2->setMaximumSize(QSize(48, 48));
			$qdial = "m_mmtrx$i4" . "_$i" . "_$i2";
			$qdials_h .= "QDial * $qdial;\n";
			#TODO: Refactor tableWidget_2 and rename it here
			$qdials_cpp .= "$qdial = new QDial(0, 100, 1, 0);\ntableWidget_2->setCellWidget($i4, $i3, $qdial);\n";
			$i3++;
		}
	}
}

print "

$defines

";

print "

$struct

";

print "

$qdials_h

";

print "

$qdials_cpp

";

print "

$connect_port


";

print "

$ladspa_descriptor

";

#0 == port define(LMS_SAMPL_XXXX), 1 == description
sub get_ladspa_desc
{
$port_define = $_[0];
$min = $_[1];
$max = $_[2];
$hint = $_[3];
$desc = $_[4];
return "
	port_descriptors[$port_define] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[$port_define] = \"$desc\";
	port_range_hints[$port_define].HintDescriptor =
	    $hint
	    LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[$port_define].LowerBound = $min;
	port_range_hints[$port_define].UpperBound = $max;
";
}

#0 == fx group number, 1 == fx number
#returns array:  [0] == header declarations, [1] == cpp instantiations, [2] == retranslate function
#TODO:  Place in separate layouts, and write a function that switches the layout that each one is in.
sub get_fx_groupbox
{
return ("
    QGroupBox *groupBox_3;
    QVBoxLayout *verticalLayout_3;
    QGridLayout *gridLayout_4;
    QDial *m_fx_knob_2_3;
    QLabel *m_fx_label_2_2;
    QLabel *label_11;
    QDial *m_fx_knob_2_2;
    QComboBox *comboBox_3;
    QLabel *m_fx_label_2_1;
    QLabel *m_fx_label_2_3;
    QDial *m_fx_knob_2_1;
", "

        groupBox_3 = new QGroupBox(tab_2);
        groupBox_3->setObjectName(QString::fromUtf8(\"groupBox_3\"));
        groupBox_3->setStyleSheet(QString::fromUtf8(\"QGroupBox{border-color: rgb(0, 0, 127); background-color: white;};\"));
        verticalLayout_3 = new QVBoxLayout(groupBox_3);
        verticalLayout_3->setObjectName(QString::fromUtf8(\"verticalLayout_3\"));
        gridLayout_4 = new QGridLayout();
        gridLayout_4->setObjectName(QString::fromUtf8(\"gridLayout_4\"));
        m_fx_knob_2_3 = new QDial(groupBox_3);
        m_fx_knob_2_3->setObjectName(QString::fromUtf8(\"m_fx_knob_2_3\"));
        m_fx_knob_2_3->setMinimumSize(QSize(48, 48));
        m_fx_knob_2_3->setMaximumSize(QSize(48, 48));

        gridLayout_4->addWidget(m_fx_knob_2_3, 1, 2, 1, 1);

        m_fx_label_2_2 = new QLabel(groupBox_3);
        m_fx_label_2_2->setObjectName(QString::fromUtf8(\"m_fx_label_2_2\"));

        gridLayout_4->addWidget(m_fx_label_2_2, 0, 1, 1, 1);

        label_11 = new QLabel(groupBox_3);
        label_11->setObjectName(QString::fromUtf8(\"label_11\"));

        gridLayout_4->addWidget(label_11, 0, 3, 1, 1);

        m_fx_knob_2_2 = new QDial(groupBox_3);
        m_fx_knob_2_2->setObjectName(QString::fromUtf8(\"m_fx_knob_2_2\"));
        m_fx_knob_2_2->setMinimumSize(QSize(48, 48));
        m_fx_knob_2_2->setMaximumSize(QSize(48, 48));

        gridLayout_4->addWidget(m_fx_knob_2_2, 1, 1, 1, 1);

        comboBox_3 = new QComboBox(groupBox_3);
        comboBox_3->setObjectName(QString::fromUtf8(\"comboBox_3\"));
        comboBox_3->setMinimumSize(QSize(60, 0));
        comboBox_3->setMaximumSize(QSize(60, 16777215));

        gridLayout_4->addWidget(comboBox_3, 1, 3, 1, 1);

        m_fx_label_2_1 = new QLabel(groupBox_3);
        m_fx_label_2_1->setObjectName(QString::fromUtf8(\"m_fx_label_2_1\"));

        gridLayout_4->addWidget(m_fx_label_2_1, 0, 0, 1, 1);

        m_fx_label_2_3 = new QLabel(groupBox_3);
        m_fx_label_2_3->setObjectName(QString::fromUtf8(\"m_fx_label_2_3\"));

        gridLayout_4->addWidget(m_fx_label_2_3, 0, 2, 1, 1);

        m_fx_knob_2_1 = new QDial(groupBox_3);
        m_fx_knob_2_1->setObjectName(QString::fromUtf8(\"m_fx_knob_2_1\"));
        m_fx_knob_2_1->setMinimumSize(QSize(48, 48));
        m_fx_knob_2_1->setMaximumSize(QSize(48, 48));
        m_fx_knob_2_1->setStyleSheet(QString::fromUtf8(\"\"));

        gridLayout_4->addWidget(m_fx_knob_2_1, 1, 0, 1, 1);

        verticalLayout_3->addLayout(gridLayout_4);

", "

groupBox_3->setTitle(QApplication::translate(\"Frame\", \"FX2\", 0, QApplication::UnicodeUTF8));

"
);
}
