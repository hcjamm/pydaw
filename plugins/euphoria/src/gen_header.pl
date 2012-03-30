#!/usr/bin/perl

$max_samples = 32;

$defines = "";
$struct = "";
$qdials_h = "//Mod matrix knobs\n";
$qdials_cpp = "//Mod matrix knobs\n";
$ladspa_descriptor = "";
$connect_port = "";
$midi_cc = "";  #else if (port == Sampler_BASE_PITCH) return DSSI_CC(13);

for($i = 1; $i <= $max_samples; $i++)
{
	$i2 = 7 + ($i * 15);
	$defines .= "#define LMS_SMPL_NOTE_$i = " . $i2 . "\n"; $i2++;
	$struct .= "LADSPA_Data smpl_note_$i;\n";

	$defines .= "#define LMS_SMPL_OCT_$i = " . $i2 . "\n"; $i2++;
	$struct .= "LADSPA_Data smpl_oct_$i;\n";

	$defines .= "#define LMS_SMPL_LNOTE_$i = " . $i2 . "\n"; $i2++;
	$struct .= "LADSPA_Data smpl_lnote_$i;\n";

	$defines .= "#define LMS_SMPL_HNOTE_$i = " . $i2 . "\n"; $i2++;
	$struct .= "LADSPA_Data smpl_hnote_$i;\n";

	$defines .= "#define LMS_SMPL_LVEL_$i = " . $i2 . "\n"; $i2++;
	$struct .= "LADSPA_Data smpl_lvel_$i;\n";

	$defines .= "#define LMS_SMPL_HVEL_$i = " . $i2 . "\n"; $i2++;
	$struct .= "LADSPA_Data smpl_hvel_$i;\n";

	$defines .= "#define LMS_SMPL_VOL_$i = " . $i2 . "\n"; $i2++;
	$struct .= "LADSPA_Data smpl_vol_$i;\n";

	$defines .= "#define LMS_SMPL_FXGRP_$i = " . $i2 . "\n"; $i2++;
	$struct .= "LADSPA_Data smpl_fxgrp_$i;\n";

	$defines .= "#define LMS_SMPL_MODE_$i = " . $i2 . "\n"; $i2++;
	$struct .= "LADSPA_Data smpl_mode_$i;\n";

	$defines .= "#define LMS_SMPL_LSEC_$i = " . $i2 . "\n"; $i2++;
	$struct .= "LADSPA_Data smpl_lsec_$i;\n";

	$defines .= "#define LMS_SMPL_LSAMP_$i = " . $i2 . "\n"; $i2++;
	$struct .= "LADSPA_Data smpl_lsamp_$i;\n";

	$defines .= "#define LMS_SMPL_START_$i = " . $i2 . "\n"; $i2++;
	$struct .= "LADSPA_Data smpl_start_$i;\n";

	$defines .= "#define LMS_SMPL_END_$i = " . $i2 . "\n"; $i2++;
	$struct .= "LADSPA_Data smpl_end_$i;\n";

	$defines .= "#define LMS_SMPL_LSTART_$i = " . $i2 . "\n"; $i2++;
	$struct .= "LADSPA_Data smpl_lstart_$i;\n";

	$defines .= "#define LMS_SMPL_LEND_$i = " . $i2 . "\n"; $i2++;
	$struct .= "LADSPA_Data smpl_lend_$i;\n";
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

print $defines;
print $struct;
print $qdials_h;
print $qdials_cpp;
print $ladspa_descriptor;

#0 == port define(LMS_SAMPL_XXXX), 1 == description
sub get_ladspa_desc
{
return "
	port_descriptors[$_[0]] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
	port_names[$_[0]] = \"$_[1]\";
	port_range_hints[$_[0]].HintDescriptor =
	    LADSPA_HINT_DEFAULT_MINIMUM | LADSPA_HINT_LOGARITHMIC |
	    LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
	port_range_hints[$_[0]].LowerBound = $_[0]_MIN;
	port_range_hints[$_[0]].UpperBound = $_[0]_MAX;
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

groupBox_3->setTitle(QApplication::translate("Frame", "FX2", 0, QApplication::UnicodeUTF8));

"
);
}
