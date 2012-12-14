/* -*- c-basic-offset: 4 -*-  vi:set ts=8 sts=4 sw=4: */

/* synth_qt_gui.cpp

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

*/

#include "synth_qt_gui.h"

#include <QApplication>
#include <QPushButton>
#include <QTimer>
#include <iostream>
#include <unistd.h>

#include <QGroupBox>
#include <QPixmap>

#include <stdlib.h>

#include "synth.h"
#include "meta.h"

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/SM/SMlib.h>

static int handle_x11_error(Display *dpy, XErrorEvent *err)
{
    char errstr[256];
    XGetErrorText(dpy, err->error_code, errstr, 256);
    if (err->error_code != BadWindow) {
	std::cerr << "synth_qt_gui: X Error: "
		  << errstr << " " << err->error_code
		  << "\nin major opcode:  " << err->request_code << std::endl;
    }
    return 0;
}
#endif

using std::endl;

lo_server osc_server = 0;

static QTextStream rayv_cerr(stderr);


rayv_gui::rayv_gui(const char * host, const char * port,
		   QByteArray controlPath, QByteArray midiPath, QByteArray programPath,
		   QByteArray exitingPath, QWidget *w) :
    QFrame(w),
    m_controlPath(controlPath),
    m_midiPath(midiPath),
    m_programPath(programPath),
    m_exitingPath(exitingPath),
    m_suppressHostUpdate(true),
    m_hostRequestedQuit(false),
    m_ready(false)
{    
    m_host = lo_address_new(host, port);
        
    /*Set the CSS style that will "cascade" on the other controls.  Other control's styles can be overridden by running their own setStyleSheet method*/
    this->setStyleSheet("QPushButton {background-color: black; border-style: outset; border-width: 2px; border-radius: 10px;border-color: white;font: bold 14px; min-width: 10em; padding: 6px; color:white;}  QAbstractItemView {outline: none;} QComboBox{background-color:black; color:white; border:solid 1px white;} QComboBox:editable {background-color:black; color:white;} QDial{background-color:rgb(152, 152, 152);} QFrame{background-color:rgb(0,0,0);} QGroupBox {color: white; border: 2px solid gray;  border-radius: 10px;  margin-top: 1ex; } QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 3px;} QMessageBox{color:white;background-color:black;}");
    
    QStringList f_osc_types = QStringList() << "Plain Saw" << "SuperbSaw" << "Viral Saw" << "Off";
        
    QStringList f_lfo_types = QStringList() << "Off" << "Sine" << "Triangle";
        
    QString f_default_presets = QString("empty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\n");
        
    LMS_style_info * f_info = new LMS_style_info(64);
    f_info->LMS_set_label_style("QLabel{background-color: white; border: 1px solid black;  border-radius: 6px;}", 60);
    f_info->LMS_set_value_style(QString("color : white; background-color: rgba(0,0,0,0);"), 64);
        
    m_tab_widget = new QTabWidget(this);
    
    m_window_layout = new QVBoxLayout();
    this->setLayout(m_window_layout);
    m_window_layout->addWidget(m_tab_widget);
    
    m_osc_tab = new QWidget();
    m_tab_widget->addTab(m_osc_tab, QString("Oscillators"));
    m_poly_fx_tab = new QWidget();    
    m_tab_widget->addTab(m_poly_fx_tab, QString("PolyFX"));

    m_oscillator_layout = new LMS_main_layout(m_osc_tab);
    
    m_program = new LMS_preset_manager(QString(WAYV_PLUGIN_NAME), f_default_presets, WAYV_PROGRAM_CHANGE, f_info, this);
        
    connect(m_program->m_program, SIGNAL(currentIndexChanged(int)), this, SLOT(programChanged(int)));
    connect(m_program->m_prog_save, SIGNAL(pressed()), this, SLOT(programSaved()));
    
    m_oscillator_layout->lms_add_widget(m_program->lms_group_box);
    
    QLabel * f_logo_label = new QLabel("", this);    
    f_logo_label->setTextFormat(Qt::RichText);
    /*This string is a base64 encoded .png image I created using Gimp for the logo.  To get the base64 encoded string,
     run it through png_to_base64.pl in the packaging folder*/
    
    /*
    QString f_logo_text = "<html><img src=\"data:image/png;base64,iVBORw0KGgoAAAANsnip==\"/></html>";
    */
    QString f_logo_text = "Way-V(real logo coming soon)";
    f_logo_label->setText(f_logo_text);
    f_logo_label->setMinimumSize(90, 30);   
    f_logo_label->show();
    m_oscillator_layout->lms_add_widget(f_logo_label);
    
        
    m_oscillator_layout->lms_add_layout();
    
    m_osc1 = new LMS_oscillator_widget(f_info, this, QString("Oscillator 1") , WAYV_OSC1_PITCH, WAYV_OSC1_TUNE, WAYV_OSC1_VOLUME, WAYV_OSC1_TYPE, f_osc_types);
    
    m_oscillator_layout->lms_add_widget(m_osc1->lms_groupbox->lms_groupbox);
    
    connect(m_osc1->lms_pitch_knob->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(osc1PitchChanged(int)));
    connect(m_osc1->lms_fine_knob->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(osc1TuneChanged(int)));        
    connect(m_osc1->lms_vol_knob->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(osc1VolumeChanged(int)));        
    connect(m_osc1->lms_osc_type_box->lms_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(osc1TypeChanged(int)));

    m_adsr_amp1 = new LMS_adsr_widget(this, f_info, TRUE, WAYV_ATTACK1, WAYV_DECAY1, WAYV_SUSTAIN1, WAYV_RELEASE1, QString("ADSR Osc1"));    
    m_oscillator_layout->lms_add_widget(m_adsr_amp1->lms_groupbox_adsr->lms_groupbox);
    
    connect(m_adsr_amp1->lms_attack->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(attack1Changed(int)));    
    connect(m_adsr_amp1->lms_decay->lms_knob,   SIGNAL(valueChanged(int)), this, SLOT(decay1Changed(int)));        
    connect(m_adsr_amp1->lms_sustain->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(sustain1Changed(int)));        
    connect(m_adsr_amp1->lms_release->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(release1Changed(int)));    
    
    
    m_oscillator_layout->lms_add_layout();    
    
    m_osc2 = new LMS_oscillator_widget(f_info, this, QString("Oscillator 2"), WAYV_OSC2_PITCH, WAYV_OSC2_TUNE, WAYV_OSC2_VOLUME, WAYV_OSC2_TYPE, f_osc_types);    
    
    m_oscillator_layout->lms_add_widget(m_osc2->lms_groupbox->lms_groupbox);
    connect(m_osc2->lms_pitch_knob->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(osc2PitchChanged(int)));
    connect(m_osc2->lms_fine_knob->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(osc2TuneChanged(int)));
    connect(m_osc2->lms_vol_knob->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(osc2VolumeChanged(int)));    
    connect(m_osc2->lms_osc_type_box->lms_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(osc2TypeChanged(int)));
        
    m_adsr_amp2 = new LMS_adsr_widget(this, f_info, TRUE, WAYV_ATTACK2, WAYV_DECAY2, WAYV_SUSTAIN2, WAYV_RELEASE2, QString("ADSR Osc2"));    
    m_oscillator_layout->lms_add_widget(m_adsr_amp2->lms_groupbox_adsr->lms_groupbox);
    
    connect(m_adsr_amp2->lms_attack->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(attack2Changed(int)));    
    connect(m_adsr_amp2->lms_decay->lms_knob,   SIGNAL(valueChanged(int)), this, SLOT(decay2Changed(int)));        
    connect(m_adsr_amp2->lms_sustain->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(sustain2Changed(int)));        
    connect(m_adsr_amp2->lms_release->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(release2Changed(int)));    
    
    m_oscillator_layout->lms_add_layout();
    
    m_master = new LMS_master_widget(this, f_info, WAYV_MASTER_VOLUME, WAYV_MASTER_UNISON_VOICES, 
            WAYV_MASTER_UNISON_SPREAD, WAYV_MASTER_GLIDE, WAYV_MASTER_PITCHBEND_AMT, QString("Master"));
    m_oscillator_layout->lms_add_widget(m_master->lms_groupbox->lms_groupbox);    
    
    connect(m_master->lms_master_volume->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(masterVolumeChanged(int)));    
    connect(m_master->lms_master_unison_voices->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT( masterUnisonVoicesChanged(int)));    
    connect(m_master->lms_master_unison_spread->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(masterUnisonSpreadChanged(int)));
    connect(m_master->lms_master_glide->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(masterGlideChanged(int)));    
    connect(m_master->lms_master_pitchbend_amt->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(masterPitchbendAmtChanged(int)));
    
    m_adsr_amp_main = new LMS_adsr_widget(this, f_info, TRUE, WAYV_ATTACK, WAYV_DECAY, WAYV_SUSTAIN, WAYV_RELEASE, QString("ADSR Master"));    
    m_oscillator_layout->lms_add_widget(m_adsr_amp_main->lms_groupbox_adsr->lms_groupbox);
    
    connect(m_adsr_amp_main->lms_attack->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(attackMainChanged(int)));    
    connect(m_adsr_amp_main->lms_decay->lms_knob,   SIGNAL(valueChanged(int)), this, SLOT(decayMainChanged(int)));        
    connect(m_adsr_amp_main->lms_sustain->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(sustainMainChanged(int)));        
    connect(m_adsr_amp_main->lms_release->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(releaseMainChanged(int)));    
    
    m_main_layout = new LMS_main_layout(m_poly_fx_tab);
        
    //From Modulex

    m_fx0 = new LMS_multieffect(this, QString("FX1"), f_info, EUPHORIA_FX0_KNOB0, EUPHORIA_FX0_KNOB1, EUPHORIA_FX0_KNOB2, EUPHORIA_FX0_COMBOBOX);
    connect(m_fx0->lms_knob1->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx0knob0Changed(int)));
    connect(m_fx0->lms_knob2->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx0knob1Changed(int)));
    connect(m_fx0->lms_knob3->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx0knob2Changed(int)));
    connect(m_fx0->lms_combobox->lms_combobox,  SIGNAL(currentIndexChanged(int)), this, SLOT(fx0comboboxChanged(int)));

    m_main_layout->lms_add_widget(m_fx0->lms_groupbox->lms_groupbox);

    m_fx1 = new LMS_multieffect(this, QString("FX2"), f_info, EUPHORIA_FX1_KNOB0, EUPHORIA_FX1_KNOB1, EUPHORIA_FX1_KNOB2, LMS_FX1_COMBOBOX);
    connect(m_fx1->lms_knob1->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx1knob0Changed(int)));
    connect(m_fx1->lms_knob2->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx1knob1Changed(int)));
    connect(m_fx1->lms_knob3->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx1knob2Changed(int)));
    connect(m_fx1->lms_combobox->lms_combobox,  SIGNAL(currentIndexChanged(int)), this, SLOT(fx1comboboxChanged(int)));

    m_main_layout->lms_add_widget(m_fx1->lms_groupbox->lms_groupbox);

    m_main_layout->lms_add_layout();    

    m_fx2 = new LMS_multieffect(this, QString("FX3"), f_info, LMS_FX2_KNOB0, LMS_FX2_KNOB1, LMS_FX2_KNOB2, LMS_FX2_COMBOBOX);
    connect(m_fx2->lms_knob1->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx2knob0Changed(int)));
    connect(m_fx2->lms_knob2->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx2knob1Changed(int)));
    connect(m_fx2->lms_knob3->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx2knob2Changed(int)));
    connect(m_fx2->lms_combobox->lms_combobox,  SIGNAL(currentIndexChanged(int)), this, SLOT(fx2comboboxChanged(int)));

    m_main_layout->lms_add_widget(m_fx2->lms_groupbox->lms_groupbox);

    m_fx3 = new LMS_multieffect(this, QString("FX4"), f_info, LMS_FX3_KNOB0, LMS_FX3_KNOB1, LMS_FX3_KNOB2, LMS_FX3_COMBOBOX);
    connect(m_fx3->lms_knob1->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx3knob0Changed(int)));
    connect(m_fx3->lms_knob2->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx3knob1Changed(int)));
    connect(m_fx3->lms_knob3->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx3knob2Changed(int)));
    connect(m_fx3->lms_combobox->lms_combobox,  SIGNAL(currentIndexChanged(int)), this, SLOT(fx3comboboxChanged(int)));

    m_main_layout->lms_add_widget(m_fx3->lms_groupbox->lms_groupbox);

    m_main_layout->lms_add_layout();  


    //New mod matrix

    QList <LMS_mod_matrix_column*> f_mod_matrix_columns;

    f_mod_matrix_columns << new LMS_mod_matrix_column(spinbox, QString("FX1\nCtrl1"), -100, 100, 0); 
    f_mod_matrix_columns << new LMS_mod_matrix_column(spinbox, QString("FX1\nCtrl2"), -100, 100, 0); 
    f_mod_matrix_columns << new LMS_mod_matrix_column(spinbox, QString("FX1\nCtrl3"), -100, 100, 0); 

    f_mod_matrix_columns << new LMS_mod_matrix_column(spinbox, QString("FX2\nCtrl1"), -100, 100, 0); 
    f_mod_matrix_columns << new LMS_mod_matrix_column(spinbox, QString("FX2\nCtrl2"), -100, 100, 0); 
    f_mod_matrix_columns << new LMS_mod_matrix_column(spinbox, QString("FX2\nCtrl3"), -100, 100, 0);  

    f_mod_matrix_columns << new LMS_mod_matrix_column(spinbox, QString("FX3\nCtrl1"), -100, 100, 0);  
    f_mod_matrix_columns << new LMS_mod_matrix_column(spinbox, QString("FX3\nCtrl2"), -100, 100, 0);  
    f_mod_matrix_columns << new LMS_mod_matrix_column(spinbox, QString("FX3\nCtrl3"), -100, 100, 0);  

    f_mod_matrix_columns << new LMS_mod_matrix_column(spinbox, QString("FX4\nCtrl1"), -100, 100, 0);  
    f_mod_matrix_columns << new LMS_mod_matrix_column(spinbox, QString("FX4\nCtrl2"), -100, 100, 0);  
    f_mod_matrix_columns << new LMS_mod_matrix_column(spinbox, QString("FX4\nCtrl3"), -100, 100, 0);  

    m_polyfx_mod_matrix[0] = new LMS_mod_matrix(this, EUPHORIA_MODULATOR_COUNT, f_mod_matrix_columns, LMS_PFXMATRIX_FIRST_PORT, f_info);

    m_polyfx_mod_matrix[0]->lms_mod_matrix->setVerticalHeaderLabels(QStringList() << QString("ADSR Amp") << QString("ADSR 2") << QString("Ramp Env") << QString("LFO"));

    m_main_layout->lms_add_widget(m_polyfx_mod_matrix[0]->lms_mod_matrix);

    m_polyfx_mod_matrix[0]->lms_mod_matrix->resizeColumnsToContents();

    m_main_layout->lms_add_layout();  

    //Connect all ports from PolyFX mod matrix

    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[0]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst0src0ctrl0Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[1]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst0src0ctrl1Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[2]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst0src0ctrl2Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[0]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst0src1ctrl0Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[1]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst0src1ctrl1Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[2]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst0src1ctrl2Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[0]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst0src2ctrl0Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[1]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst0src2ctrl1Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[2]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst0src2ctrl2Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[0]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst0src3ctrl0Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[1]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst0src3ctrl1Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[2]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst0src3ctrl2Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[3]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst1src0ctrl0Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[4]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst1src0ctrl1Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[5]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst1src0ctrl2Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[3]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst1src1ctrl0Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[4]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst1src1ctrl1Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[5]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst1src1ctrl2Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[3]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst1src2ctrl0Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[4]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst1src2ctrl1Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[5]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst1src2ctrl2Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[3]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst1src3ctrl0Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[4]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst1src3ctrl1Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[5]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst1src3ctrl2Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[6]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst2src0ctrl0Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[7]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst2src0ctrl1Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[8]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst2src0ctrl2Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[6]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst2src1ctrl0Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[7]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst2src1ctrl1Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[8]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst2src1ctrl2Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[6]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst2src2ctrl0Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[7]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst2src2ctrl1Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[8]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst2src2ctrl2Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[6]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst2src3ctrl0Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[7]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst2src3ctrl1Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[8]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst2src3ctrl2Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[9]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst3src0ctrl0Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[10]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst3src0ctrl1Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[11]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst3src0ctrl2Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[9]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst3src1ctrl0Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[10]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst3src1ctrl1Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[11]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst3src1ctrl2Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[9]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst3src2ctrl0Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[10]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst3src2ctrl1Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[11]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst3src2ctrl2Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[9]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst3src3ctrl0Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[10]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst3src3ctrl1Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[11]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst3src3ctrl2Changed(int)));

    //End new mod matrix



    //End from Modulex

    m_adsr_amp = new LMS_adsr_widget(this, f_info, TRUE, EUPHORIA_ATTACK, EUPHORIA_DECAY, EUPHORIA_SUSTAIN, EUPHORIA_RELEASE, QString("ADSR Amp"));

    m_adsr_amp->lms_release->lms_knob->setMinimum(5);  //overriding the default for this, because we want a low minimum default that won't click

    m_main_layout->lms_add_widget(m_adsr_amp->lms_groupbox_adsr->lms_groupbox);

    connect(m_adsr_amp->lms_attack->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(attackChanged(int)));    
    connect(m_adsr_amp->lms_decay->lms_knob,   SIGNAL(valueChanged(int)), this, SLOT(decayChanged(int)));        
    connect(m_adsr_amp->lms_sustain->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(sustainChanged(int)));        
    connect(m_adsr_amp->lms_release->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(releaseChanged(int)));    

    m_groupbox_noise = new LMS_group_box(this, QString("Noise"), f_info);
    m_main_layout->lms_add_widget(m_groupbox_noise->lms_groupbox);

    m_noise_amp = new LMS_knob_regular(QString("Vol"), -60, 0, 1, 30, QString(""), m_groupbox_noise->lms_groupbox, f_info, lms_kc_integer, EUPHORIA_NOISE_AMP);
    m_groupbox_noise->lms_add_h(m_noise_amp);
    connect(m_noise_amp->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(noiseAmpChanged(int)));

    m_noise_type = new LMS_combobox(QString("Type"), this, QStringList() << QString("Off") << QString("White") << QString("Pink"), LMS_NOISE_TYPE, f_info);
    m_groupbox_noise->lms_add_h(m_noise_type);
    connect(m_noise_type->lms_combobox,  SIGNAL(currentIndexChanged(int)), this, SLOT(noise_typeChanged(int)));

    m_adsr_filter = new LMS_adsr_widget(this, f_info, FALSE, EUPHORIA_FILTER_ATTACK, EUPHORIA_FILTER_DECAY, EUPHORIA_FILTER_SUSTAIN, EUPHORIA_FILTER_RELEASE, QString("ADSR 2"));

    m_main_layout->lms_add_widget(m_adsr_filter->lms_groupbox_adsr->lms_groupbox);

    connect(m_adsr_filter->lms_attack->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(filterAttackChanged(int)));
    connect(m_adsr_filter->lms_decay->lms_knob,   SIGNAL(valueChanged(int)), this, SLOT(filterDecayChanged(int))); 
    connect(m_adsr_filter->lms_sustain->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(filterSustainChanged(int)));
    connect(m_adsr_filter->lms_release->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(filterReleaseChanged(int)));

    m_pitch_env = new LMS_ramp_env(this, f_info, EUPHORIA_PITCH_ENV_TIME, -1, -1, FALSE, QString("Ramp Env"), FALSE);
    m_main_layout->lms_add_widget(m_pitch_env->lms_groupbox->lms_groupbox);

    connect(m_pitch_env->lms_time_knob->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(pitchEnvTimeChanged(int)));

    m_lfo = new LMS_lfo_widget(this, f_info, EUPHORIA_LFO_FREQ, EUPHORIA_LFO_TYPE, f_lfo_types, QString("LFO"));
    m_main_layout->lms_add_widget(m_lfo->lms_groupbox->lms_groupbox);

    //Overriding the default so we can have a faster LFO
    m_lfo->lms_freq_knob->lms_knob->setMaximum(1600);

    connect(m_lfo->lms_freq_knob->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(LFOfreqChanged(int)));
    connect(m_lfo->lms_type_combobox->lms_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(LFOtypeChanged(int)));

    
    
    
    
    
    /*Add the knobs to the preset module*/
    
    m_program->lms_add_control(m_adsr_amp_main->lms_attack);
    m_program->lms_add_control(m_adsr_amp_main->lms_decay);
    m_program->lms_add_control(m_adsr_amp_main->lms_sustain);
    m_program->lms_add_control(m_adsr_amp_main->lms_release);    
    m_program->lms_add_control(m_adsr_amp->lms_attack);
    m_program->lms_add_control(m_adsr_amp->lms_decay);
    m_program->lms_add_control(m_adsr_amp->lms_sustain);
    m_program->lms_add_control(m_adsr_amp->lms_release);
    m_program->lms_add_control(m_adsr_filter->lms_attack);
    m_program->lms_add_control(m_adsr_filter->lms_decay);
    m_program->lms_add_control(m_adsr_filter->lms_sustain);
    m_program->lms_add_control(m_adsr_filter->lms_release);
    m_program->lms_add_control(m_adsr_amp1->lms_attack);
    m_program->lms_add_control(m_adsr_amp1->lms_decay);
    m_program->lms_add_control(m_adsr_amp1->lms_sustain);
    m_program->lms_add_control(m_adsr_amp1->lms_release);
    m_program->lms_add_control(m_adsr_amp2->lms_attack);
    m_program->lms_add_control(m_adsr_amp2->lms_decay);
    m_program->lms_add_control(m_adsr_amp2->lms_sustain);
    m_program->lms_add_control(m_adsr_amp2->lms_release);
    m_program->lms_add_control(m_lfo->lms_freq_knob);
    m_program->lms_add_control(m_lfo->lms_type_combobox);
    
    m_program->lms_add_control(m_noise_amp);
    m_program->lms_add_control(m_noise_type);
    m_program->lms_add_control(m_pitch_env->lms_time_knob);
    m_program->lms_add_control(m_osc1->lms_osc_type_box);
    m_program->lms_add_control(m_osc1->lms_pitch_knob);
    m_program->lms_add_control(m_osc1->lms_fine_knob);
    m_program->lms_add_control(m_osc1->lms_vol_knob);    
    m_program->lms_add_control(m_osc2->lms_osc_type_box);
    m_program->lms_add_control(m_osc2->lms_pitch_knob);
    m_program->lms_add_control(m_osc2->lms_fine_knob);
    m_program->lms_add_control(m_osc2->lms_vol_knob);    
    m_program->lms_add_control(m_master->lms_master_volume);
    m_program->lms_add_control(m_master->lms_master_unison_voices);
    m_program->lms_add_control(m_master->lms_master_unison_spread);
    m_program->lms_add_control(m_master->lms_master_glide);
    m_program->lms_add_control(m_master->lms_master_pitchbend_amt);
    
    m_program->lms_add_control(m_fx0->lms_knob1);
    m_program->lms_add_control(m_fx0->lms_knob2);
    m_program->lms_add_control(m_fx0->lms_knob3);
    m_program->lms_add_control(m_fx0->lms_combobox);
    m_program->lms_add_control(m_fx1->lms_knob1);
    m_program->lms_add_control(m_fx1->lms_knob2);
    m_program->lms_add_control(m_fx1->lms_knob3);
    m_program->lms_add_control(m_fx1->lms_combobox);
    m_program->lms_add_control(m_fx2->lms_knob1);
    m_program->lms_add_control(m_fx2->lms_knob2);
    m_program->lms_add_control(m_fx2->lms_knob3);
    m_program->lms_add_control(m_fx2->lms_combobox);
    m_program->lms_add_control(m_fx3->lms_knob1);
    m_program->lms_add_control(m_fx3->lms_knob2);
    m_program->lms_add_control(m_fx3->lms_knob3);
    m_program->lms_add_control(m_fx3->lms_combobox);
        
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[0]->controls[0]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[1]->controls[0]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[2]->controls[0]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[0]->controls[1]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[1]->controls[1]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[2]->controls[1]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[0]->controls[2]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[1]->controls[2]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[2]->controls[2]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[0]->controls[3]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[1]->controls[3]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[2]->controls[3]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[3]->controls[0]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[4]->controls[0]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[5]->controls[0]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[3]->controls[1]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[4]->controls[1]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[5]->controls[1]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[3]->controls[2]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[4]->controls[2]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[5]->controls[2]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[3]->controls[3]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[4]->controls[3]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[5]->controls[3]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[6]->controls[0]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[7]->controls[0]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[8]->controls[0]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[6]->controls[1]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[7]->controls[1]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[8]->controls[1]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[6]->controls[2]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[7]->controls[2]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[8]->controls[2]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[6]->controls[3]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[7]->controls[3]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[8]->controls[3]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[9]->controls[0]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[10]->controls[0]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[11]->controls[0]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[9]->controls[1]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[10]->controls[1]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[11]->controls[1]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[9]->controls[2]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[10]->controls[2]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[11]->controls[2]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[9]->controls[3]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[10]->controls[3]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[11]->controls[3]);

       
    
    QTimer *myTimer = new QTimer(this);
    connect(myTimer, SIGNAL(timeout()), this, SLOT(oscRecv()));
    myTimer->setSingleShot(false);
    myTimer->start(0);
    
    m_suppressHostUpdate = false;
}





void rayv_gui::pfxmatrix_Changed(int a_port, int a_fx_group, int a_dst, int a_ctrl, int a_src)
{
    //m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[a_control_index]->lms_value_changed(0);    
#ifndef LMS_DEBUG_STANDALONE
    if (!m_suppressHostUpdate) {        
	lo_send(m_host, m_controlPath, "if",
                a_port,
                //(float)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[a_control_index]->lms_get_value())
                (float)(m_polyfx_mod_matrix[a_fx_group]->lms_mm_columns[((a_dst * EUPHORIA_CONTROLS_PER_MOD_EFFECT) + a_ctrl)]->controls[a_src]->lms_get_value())
                );
    }
#endif
}





void rayv_gui::fx0knob0Changed(int value){ lms_value_changed(value, m_fx0->lms_knob1); }
void rayv_gui::fx0knob1Changed(int value){ lms_value_changed(value, m_fx0->lms_knob2); }
void rayv_gui::fx0knob2Changed(int value){ lms_value_changed(value, m_fx0->lms_knob3); }
void rayv_gui::fx0comboboxChanged(int value){ lms_value_changed(value, m_fx0->lms_combobox); m_fx0->lms_combobox_changed(); }

void rayv_gui::fx1knob0Changed(int value){ lms_value_changed(value, m_fx1->lms_knob1); }
void rayv_gui::fx1knob1Changed(int value){ lms_value_changed(value, m_fx1->lms_knob2); }
void rayv_gui::fx1knob2Changed(int value){ lms_value_changed(value, m_fx1->lms_knob3); }
void rayv_gui::fx1comboboxChanged(int value){ lms_value_changed(value, m_fx1->lms_combobox); m_fx1->lms_combobox_changed(); }

void rayv_gui::fx2knob0Changed(int value){ lms_value_changed(value, m_fx2->lms_knob1); }
void rayv_gui::fx2knob1Changed(int value){ lms_value_changed(value, m_fx2->lms_knob2); }
void rayv_gui::fx2knob2Changed(int value){ lms_value_changed(value, m_fx2->lms_knob3); }
void rayv_gui::fx2comboboxChanged(int value){ lms_value_changed(value, m_fx2->lms_combobox); m_fx2->lms_combobox_changed(); }

void rayv_gui::fx3knob0Changed(int value){ lms_value_changed(value, m_fx3->lms_knob1); }
void rayv_gui::fx3knob1Changed(int value){ lms_value_changed(value, m_fx3->lms_knob2); }
void rayv_gui::fx3knob2Changed(int value){ lms_value_changed(value, m_fx3->lms_knob3); }
void rayv_gui::fx3comboboxChanged(int value){ lms_value_changed(value, m_fx3->lms_combobox); m_fx3->lms_combobox_changed(); }




void rayv_gui::pfxmatrix_grp0dst0src0ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST0SRC0CTRL0, 0, 0, 0, 0);}
void rayv_gui::pfxmatrix_grp0dst0src0ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST0SRC0CTRL1, 0, 0, 1, 0);}
void rayv_gui::pfxmatrix_grp0dst0src0ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST0SRC0CTRL2, 0, 0, 2, 0);}
void rayv_gui::pfxmatrix_grp0dst0src1ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST0SRC1CTRL0, 0, 0, 0, 1);}
void rayv_gui::pfxmatrix_grp0dst0src1ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST0SRC1CTRL1, 0, 0, 1, 1);}
void rayv_gui::pfxmatrix_grp0dst0src1ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST0SRC1CTRL2, 0, 0, 2, 1);}
void rayv_gui::pfxmatrix_grp0dst0src2ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST0SRC2CTRL0, 0, 0, 0, 2);}
void rayv_gui::pfxmatrix_grp0dst0src2ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST0SRC2CTRL1, 0, 0, 1, 2);}
void rayv_gui::pfxmatrix_grp0dst0src2ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST0SRC2CTRL2, 0, 0, 2, 2);}
void rayv_gui::pfxmatrix_grp0dst0src3ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST0SRC3CTRL0, 0, 0, 0, 3);}
void rayv_gui::pfxmatrix_grp0dst0src3ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST0SRC3CTRL1, 0, 0, 1, 3);}
void rayv_gui::pfxmatrix_grp0dst0src3ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST0SRC3CTRL2, 0, 0, 2, 3);}
void rayv_gui::pfxmatrix_grp0dst1src0ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST1SRC0CTRL0, 0, 1, 0, 0);}
void rayv_gui::pfxmatrix_grp0dst1src0ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST1SRC0CTRL1, 0, 1, 1, 0);}
void rayv_gui::pfxmatrix_grp0dst1src0ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST1SRC0CTRL2, 0, 1, 2, 0);}
void rayv_gui::pfxmatrix_grp0dst1src1ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST1SRC1CTRL0, 0, 1, 0, 1);}
void rayv_gui::pfxmatrix_grp0dst1src1ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST1SRC1CTRL1, 0, 1, 1, 1);}
void rayv_gui::pfxmatrix_grp0dst1src1ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST1SRC1CTRL2, 0, 1, 2, 1);}
void rayv_gui::pfxmatrix_grp0dst1src2ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST1SRC2CTRL0, 0, 1, 0, 2);}
void rayv_gui::pfxmatrix_grp0dst1src2ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST1SRC2CTRL1, 0, 1, 1, 2);}
void rayv_gui::pfxmatrix_grp0dst1src2ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST1SRC2CTRL2, 0, 1, 2, 2);}
void rayv_gui::pfxmatrix_grp0dst1src3ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST1SRC3CTRL0, 0, 1, 0, 3);}
void rayv_gui::pfxmatrix_grp0dst1src3ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST1SRC3CTRL1, 0, 1, 1, 3);}
void rayv_gui::pfxmatrix_grp0dst1src3ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST1SRC3CTRL2, 0, 1, 2, 3);}
void rayv_gui::pfxmatrix_grp0dst2src0ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST2SRC0CTRL0, 0, 2, 0, 0);}
void rayv_gui::pfxmatrix_grp0dst2src0ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST2SRC0CTRL1, 0, 2, 1, 0);}
void rayv_gui::pfxmatrix_grp0dst2src0ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST2SRC0CTRL2, 0, 2, 2, 0);}
void rayv_gui::pfxmatrix_grp0dst2src1ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST2SRC1CTRL0, 0, 2, 0, 1);}
void rayv_gui::pfxmatrix_grp0dst2src1ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST2SRC1CTRL1, 0, 2, 1, 1);}
void rayv_gui::pfxmatrix_grp0dst2src1ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST2SRC1CTRL2, 0, 2, 2, 1);}
void rayv_gui::pfxmatrix_grp0dst2src2ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST2SRC2CTRL0, 0, 2, 0, 2);}
void rayv_gui::pfxmatrix_grp0dst2src2ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST2SRC2CTRL1, 0, 2, 1, 2);}
void rayv_gui::pfxmatrix_grp0dst2src2ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST2SRC2CTRL2, 0, 2, 2, 2);}
void rayv_gui::pfxmatrix_grp0dst2src3ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST2SRC3CTRL0, 0, 2, 0, 3);}
void rayv_gui::pfxmatrix_grp0dst2src3ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST2SRC3CTRL1, 0, 2, 1, 3);}
void rayv_gui::pfxmatrix_grp0dst2src3ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST2SRC3CTRL2, 0, 2, 2, 3);}
void rayv_gui::pfxmatrix_grp0dst3src0ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST3SRC0CTRL0, 0, 3, 0, 0);}
void rayv_gui::pfxmatrix_grp0dst3src0ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST3SRC0CTRL1, 0, 3, 1, 0);}
void rayv_gui::pfxmatrix_grp0dst3src0ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST3SRC0CTRL2, 0, 3, 2, 0);}
void rayv_gui::pfxmatrix_grp0dst3src1ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST3SRC1CTRL0, 0, 3, 0, 1);}
void rayv_gui::pfxmatrix_grp0dst3src1ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST3SRC1CTRL1, 0, 3, 1, 1);}
void rayv_gui::pfxmatrix_grp0dst3src1ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST3SRC1CTRL2, 0, 3, 2, 1);}
void rayv_gui::pfxmatrix_grp0dst3src2ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST3SRC2CTRL0, 0, 3, 0, 2);}
void rayv_gui::pfxmatrix_grp0dst3src2ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST3SRC2CTRL1, 0, 3, 1, 2);}
void rayv_gui::pfxmatrix_grp0dst3src2ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST3SRC2CTRL2, 0, 3, 2, 2);}
void rayv_gui::pfxmatrix_grp0dst3src3ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST3SRC3CTRL0, 0, 3, 0, 3);}
void rayv_gui::pfxmatrix_grp0dst3src3ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST3SRC3CTRL1, 0, 3, 1, 3);}
void rayv_gui::pfxmatrix_grp0dst3src3ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST3SRC3CTRL2, 0, 3, 2, 3);}









void rayv_gui::lms_set_value(float val, LMS_control * a_ctrl )
{    
    m_suppressHostUpdate = true;
    a_ctrl->lms_set_value(int(val));
    m_suppressHostUpdate = false;     
}

void rayv_gui::setAttackMain(float a_value){ lms_set_value(a_value, m_adsr_amp_main->lms_attack);}
void rayv_gui::setDecayMain(float a_value){ lms_set_value(a_value, m_adsr_amp_main->lms_decay); }
void rayv_gui::setSustainMain(float a_value){lms_set_value(a_value, m_adsr_amp_main->lms_sustain);}
void rayv_gui::setReleaseMain(float a_value){lms_set_value(a_value, m_adsr_amp_main->lms_release);}

void rayv_gui::setAttack1(float a_value){ lms_set_value(a_value, m_adsr_amp1->lms_attack);}
void rayv_gui::setDecay1(float a_value){ lms_set_value(a_value, m_adsr_amp1->lms_decay); }
void rayv_gui::setSustain1(float a_value){lms_set_value(a_value, m_adsr_amp1->lms_sustain);}
void rayv_gui::setRelease1(float a_value){lms_set_value(a_value, m_adsr_amp1->lms_release);}

void rayv_gui::setAttack2(float a_value){ lms_set_value(a_value, m_adsr_amp2->lms_attack);}
void rayv_gui::setDecay2(float a_value){ lms_set_value(a_value, m_adsr_amp2->lms_decay); }
void rayv_gui::setSustain2(float a_value){lms_set_value(a_value, m_adsr_amp2->lms_sustain);}
void rayv_gui::setRelease2(float a_value){lms_set_value(a_value, m_adsr_amp2->lms_release);}

void rayv_gui::setOsc1Type(float a_value){lms_set_value(a_value, m_osc1->lms_osc_type_box);}
void rayv_gui::setOsc1Pitch(float a_value){lms_set_value(a_value, m_osc1->lms_pitch_knob);}
void rayv_gui::setOsc1Tune(float a_value){lms_set_value(a_value, m_osc1->lms_fine_knob);}
void rayv_gui::setOsc1Volume(float a_value){lms_set_value(a_value, m_osc1->lms_vol_knob);}
void rayv_gui::setOsc2Type(float a_value){lms_set_value(a_value, m_osc2->lms_osc_type_box);}
void rayv_gui::setOsc2Pitch(float a_value){lms_set_value(a_value, m_osc2->lms_pitch_knob);}
void rayv_gui::setOsc2Tune(float a_value){lms_set_value(a_value, m_osc2->lms_fine_knob);}
void rayv_gui::setOsc2Volume(float a_value){lms_set_value(a_value, m_osc2->lms_vol_knob);}
void rayv_gui::setProgram(float a_value){lms_set_value(a_value, m_program);}







void rayv_gui::setFX0knob0(float val){ lms_set_value(val, m_fx0->lms_knob1); }
void rayv_gui::setFX0knob1(float val){ lms_set_value(val, m_fx0->lms_knob2); }
void rayv_gui::setFX0knob2(float val){ lms_set_value(val, m_fx0->lms_knob3); }
void rayv_gui::setFX0combobox(float val){ lms_set_value(val, m_fx0->lms_combobox); }

void rayv_gui::setFX1knob0(float val){ lms_set_value(val, m_fx1->lms_knob1); }
void rayv_gui::setFX1knob1(float val){ lms_set_value(val, m_fx1->lms_knob2); }
void rayv_gui::setFX1knob2(float val){ lms_set_value(val, m_fx1->lms_knob3); }
void rayv_gui::setFX1combobox(float val){ lms_set_value(val, m_fx1->lms_combobox); }

void rayv_gui::setFX2knob0(float val){ lms_set_value(val, m_fx2->lms_knob1); }
void rayv_gui::setFX2knob1(float val){ lms_set_value(val, m_fx2->lms_knob2); }
void rayv_gui::setFX2knob2(float val){ lms_set_value(val, m_fx2->lms_knob3); }
void rayv_gui::setFX2combobox(float val){ lms_set_value(val, m_fx2->lms_combobox); }

void rayv_gui::setFX3knob0(float val){ lms_set_value(val, m_fx3->lms_knob1); }
void rayv_gui::setFX3knob1(float val){ lms_set_value(val, m_fx3->lms_knob2); }
void rayv_gui::setFX3knob2(float val){ lms_set_value(val, m_fx3->lms_knob3); }
void rayv_gui::setFX3combobox(float val){ lms_set_value(val, m_fx3->lms_combobox); }




void rayv_gui::setAttack(float a_value){ lms_set_value(a_value, m_adsr_amp->lms_attack);}
void rayv_gui::setDecay(float a_value){ lms_set_value(a_value, m_adsr_amp->lms_decay); }
void rayv_gui::setSustain(float a_value){lms_set_value(a_value, m_adsr_amp->lms_sustain);}
void rayv_gui::setRelease(float a_value){lms_set_value(a_value, m_adsr_amp->lms_release);}
void rayv_gui::setFilterAttack (float a_value){lms_set_value(a_value, m_adsr_filter->lms_attack);}
void rayv_gui::setFilterDecay  (float a_value){lms_set_value(a_value, m_adsr_filter->lms_decay);}
void rayv_gui::setFilterSustain(float a_value){lms_set_value(a_value, m_adsr_filter->lms_sustain);}
void rayv_gui::setFilterRelease(float a_value){lms_set_value(a_value, m_adsr_filter->lms_release);}
void rayv_gui::setNoiseAmp(float a_value){lms_set_value(a_value, m_noise_amp);}
void rayv_gui::setNoiseType(float a_value){lms_set_value(a_value, m_noise_type);}
void rayv_gui::setMasterVolume(float a_value){lms_set_value(a_value, m_master->lms_master_volume);}
void rayv_gui::setMasterUnisonVoices(float a_value){lms_set_value(a_value, m_master->lms_master_unison_voices);}
void rayv_gui::setMasterUnisonSpread(float a_value){lms_set_value(a_value, m_master->lms_master_unison_spread);}
void rayv_gui::setMasterGlide(float a_value){lms_set_value(a_value, m_master->lms_master_glide);}
void rayv_gui::setMasterPitchbendAmt(float a_value){lms_set_value(a_value, m_master->lms_master_pitchbend_amt);}
void rayv_gui::setPitchEnvTime(float a_value){lms_set_value(a_value, m_pitch_env->lms_time_knob);}
void rayv_gui::setLFOfreq(float a_value){lms_set_value(a_value, m_lfo->lms_freq_knob);}
void rayv_gui::setLFOtype(float a_value){lms_set_value(a_value, m_lfo->lms_type_combobox);}







void rayv_gui::lms_value_changed(int a_value, LMS_control * a_ctrl)
{    
    a_ctrl->lms_value_changed(a_value);

    if (!m_suppressHostUpdate) {
        lo_send(m_host, m_controlPath, "if", (a_ctrl->lms_port), float(a_value));
    }    
}

void rayv_gui::attackMainChanged(int a_value){lms_value_changed(a_value, m_adsr_amp_main->lms_attack);}
void rayv_gui::decayMainChanged(int a_value){lms_value_changed(a_value, m_adsr_amp_main->lms_decay);}
void rayv_gui::sustainMainChanged(int a_value){lms_value_changed(a_value, m_adsr_amp_main->lms_sustain);}
void rayv_gui::releaseMainChanged(int a_value){lms_value_changed(a_value, m_adsr_amp_main->lms_release);}

void rayv_gui::attack1Changed(int a_value){lms_value_changed(a_value, m_adsr_amp1->lms_attack);}
void rayv_gui::decay1Changed(int a_value){lms_value_changed(a_value, m_adsr_amp1->lms_decay);}
void rayv_gui::sustain1Changed(int a_value){lms_value_changed(a_value, m_adsr_amp1->lms_sustain);}
void rayv_gui::release1Changed(int a_value){lms_value_changed(a_value, m_adsr_amp1->lms_release);}

void rayv_gui::attack2Changed(int a_value){lms_value_changed(a_value, m_adsr_amp2->lms_attack);}
void rayv_gui::decay2Changed(int a_value){lms_value_changed(a_value, m_adsr_amp2->lms_decay);}
void rayv_gui::sustain2Changed(int a_value){lms_value_changed(a_value, m_adsr_amp2->lms_sustain);}
void rayv_gui::release2Changed(int a_value){lms_value_changed(a_value, m_adsr_amp2->lms_release);}

void rayv_gui::osc1TypeChanged(int a_value){lms_value_changed(a_value, m_osc1->lms_osc_type_box);}
void rayv_gui::osc1PitchChanged(int a_value){lms_value_changed(a_value, m_osc1->lms_pitch_knob);}
void rayv_gui::osc1TuneChanged(int a_value){lms_value_changed(a_value, m_osc1->lms_fine_knob);}
void rayv_gui::osc1VolumeChanged(int a_value){lms_value_changed(a_value, m_osc1->lms_vol_knob);}
void rayv_gui::osc2TypeChanged(int a_value){lms_value_changed(a_value, m_osc2->lms_osc_type_box);}
void rayv_gui::osc2PitchChanged(int a_value){lms_value_changed(a_value, m_osc2->lms_pitch_knob);}
void rayv_gui::osc2TuneChanged(int a_value){lms_value_changed(a_value, m_osc2->lms_fine_knob);}
void rayv_gui::osc2VolumeChanged(int a_value){lms_value_changed(a_value, m_osc2->lms_vol_knob);}
void rayv_gui::programChanged(int a_value){lms_value_changed(a_value, m_program);}
void rayv_gui::programSaved(){ m_program->programSaved(); }






void rayv_gui::attackChanged(int a_value){lms_value_changed(a_value, m_adsr_amp->lms_attack);}
void rayv_gui::decayChanged(int a_value){lms_value_changed(a_value, m_adsr_amp->lms_decay);}
void rayv_gui::sustainChanged(int a_value){lms_value_changed(a_value, m_adsr_amp->lms_sustain);}
void rayv_gui::releaseChanged(int a_value){lms_value_changed(a_value, m_adsr_amp->lms_release);}
void rayv_gui::filterAttackChanged(int a_value){lms_value_changed(a_value, m_adsr_filter->lms_attack);}
void rayv_gui::filterDecayChanged(int a_value){lms_value_changed(a_value, m_adsr_filter->lms_decay);}
void rayv_gui::filterSustainChanged(int a_value){lms_value_changed(a_value, m_adsr_filter->lms_sustain);}
void rayv_gui::filterReleaseChanged(int a_value){lms_value_changed(a_value, m_adsr_filter->lms_release);}
void rayv_gui::noiseAmpChanged(int a_value){lms_value_changed(a_value, m_noise_amp);}
void rayv_gui::noise_typeChanged(int a_value){lms_value_changed(a_value, m_noise_type);}
void rayv_gui::masterVolumeChanged(int a_value){lms_value_changed(a_value, m_master->lms_master_volume);}
void rayv_gui::masterUnisonVoicesChanged(int a_value){lms_value_changed(a_value, m_master->lms_master_unison_voices);}
void rayv_gui::masterUnisonSpreadChanged(int a_value){lms_value_changed(a_value, m_master->lms_master_unison_spread);}
void rayv_gui::masterGlideChanged(int a_value){lms_value_changed(a_value, m_master->lms_master_glide);}
void rayv_gui::masterPitchbendAmtChanged(int a_value){lms_value_changed(a_value, m_master->lms_master_pitchbend_amt);}
void rayv_gui::pitchEnvTimeChanged(int a_value){lms_value_changed(a_value, m_pitch_env->lms_time_knob);}
void rayv_gui::LFOfreqChanged(int a_value){lms_value_changed(a_value, m_lfo->lms_freq_knob);}
void rayv_gui::LFOtypeChanged(int a_value){lms_value_changed(a_value, m_lfo->lms_type_combobox);}







void rayv_gui::v_print_port_name_to_cerr(int a_port)
{
#ifdef LMS_DEBUG_MODE_QT
    switch (a_port) {
    case WAYV_ATTACK: rayv_cerr << "LMS_ATTACK"; break;
    case WAYV_DECAY: rayv_cerr << "LMS_DECAY"; break;
    case WAYV_SUSTAIN: rayv_cerr << "LMS_SUSTAIN"; break;
    case WAYV_RELEASE: rayv_cerr << "LMS_RELEASE"; break;
    case WAYV_TIMBRE: rayv_cerr << "LMS_TIMBRE"; break;
    case WAYV_RES: rayv_cerr << "LMS_RES"; break;        
    case WAYV_DIST: rayv_cerr << "LMS_DIST"; break;
    case WAYV_FILTER_ATTACK: rayv_cerr << "LMS_FILTER_ATTACK"; break;
    case WAYV_FILTER_DECAY: rayv_cerr << "LMS_FILTER_DECAY"; break;
    case WAYV_FILTER_SUSTAIN: rayv_cerr << "LMS_FILTER_SUSTAIN"; break;
    case WAYV_FILTER_RELEASE: rayv_cerr << "LMS_FILTER_RELEASE"; break;
    case WAYV_NOISE_AMP: rayv_cerr << "LMS_NOISE_AMP"; break;    
    case WAYV_DIST_WET: rayv_cerr << "LMS_DIST_WET"; break;            
    case WAYV_FILTER_ENV_AMT: rayv_cerr << "LMS_FILTER_ENV_AMT"; break;    
    case WAYV_OSC1_TYPE: rayv_cerr << "LMS_OSC1_TYPE"; break;            
    case WAYV_OSC1_PITCH: rayv_cerr << "LMS_OSC1_PITCH"; break;    
    case WAYV_OSC1_TUNE: rayv_cerr << "LMS_OSC1_TUNE"; break;    
    case WAYV_OSC1_VOLUME: rayv_cerr << "LMS_OSC1_VOLUME"; break;        
    case WAYV_OSC2_TYPE: rayv_cerr << "LMS_OSC2_TYPE"; break;            
    case WAYV_OSC2_PITCH: rayv_cerr << "LMS_OSC2_PITCH"; break;    
    case WAYV_OSC2_TUNE: rayv_cerr << "LMS_OSC2_TUNE";  break;    
    case WAYV_OSC2_VOLUME: rayv_cerr << "LMS_OSC2_VOLUME"; break;        
    case WAYV_MASTER_VOLUME: rayv_cerr << "LMS_MASTER_VOLUME"; break;
    case WAYV_MASTER_UNISON_VOICES: rayv_cerr << "LMS_MASTER_UNISON_VOICES"; break;
    case WAYV_MASTER_UNISON_SPREAD: rayv_cerr << "LMS_MASTER_UNISON_SPREAD"; break;
    case WAYV_MASTER_GLIDE: rayv_cerr << "LMS_MASTER_GLIDE"; break;
    case WAYV_MASTER_PITCHBEND_AMT: rayv_cerr << "LMS_MASTER_PITCHBEND_AMT"; break;
    case WAYV_PITCH_ENV_AMT: rayv_cerr << "LMS_PITCH_ENV_AMT "; break;
    case WAYV_PITCH_ENV_TIME: rayv_cerr << "LMS_PITCH_ENV_TIME ";  break;        
    case WAYV_PROGRAM_CHANGE: rayv_cerr << "LMS_PROGRAM_CHANGE "; break;
    default: rayv_cerr << "Warning: received request to set nonexistent port " << a_port ; break;
    }
#endif
}

void rayv_gui::v_set_control(int a_port, float a_value)
{

#ifdef LMS_DEBUG_MODE_QT    
    rayv_cerr << "v_set_control called.  ";  
    v_print_port_name_to_cerr(a_port);
    rayv_cerr << "  value: " << a_value << endl;
#endif
        
    switch (a_port) {
        case WAYV_ATTACK: setAttackMain(a_value); break;
        case WAYV_DECAY: setDecayMain(a_value); break;
        case WAYV_SUSTAIN: setSustainMain(a_value); break;
        case WAYV_RELEASE: setReleaseMain(a_value); break;
        
        case WAYV_ATTACK1: setAttack1(a_value); break;
        case WAYV_DECAY1: setDecay1(a_value); break;
        case WAYV_SUSTAIN1: setSustain1(a_value); break;
        case WAYV_RELEASE1: setRelease1(a_value); break;
        
        case WAYV_ATTACK2: setAttack2(a_value); break;
        case WAYV_DECAY2: setDecay2(a_value); break;
        case WAYV_SUSTAIN2: setSustain2(a_value); break;
        case WAYV_RELEASE2: setRelease2(a_value); break;
        
        case WAYV_NOISE_AMP: setNoiseAmp(a_value); break;
        case WAYV_OSC1_TYPE: setOsc1Type(a_value); break;            
        case WAYV_OSC1_PITCH: setOsc1Pitch(a_value);  break;    
        case WAYV_OSC1_TUNE: setOsc1Tune(a_value); break;    
        case WAYV_OSC1_VOLUME: setOsc1Volume(a_value); break;        
        case WAYV_OSC2_TYPE: setOsc2Type(a_value); break;            
        case WAYV_OSC2_PITCH: setOsc2Pitch(a_value); break;    
        case WAYV_OSC2_TUNE: setOsc2Tune(a_value); break;    
        case WAYV_OSC2_VOLUME: setOsc2Volume(a_value); break;        
        case WAYV_MASTER_VOLUME: setMasterVolume(a_value); break;    
        case WAYV_MASTER_UNISON_VOICES: setMasterUnisonVoices(a_value); break;
        case WAYV_MASTER_UNISON_SPREAD: setMasterUnisonSpread(a_value); break;
        case WAYV_MASTER_GLIDE: setMasterGlide(a_value); break;
        case WAYV_MASTER_PITCHBEND_AMT: setMasterPitchbendAmt(a_value); break;
        case WAYV_PROGRAM_CHANGE: break; //This screws up host recall //setProgram(a_value); break;   
        
        case EUPHORIA_ATTACK: setAttack(a_value); break;
        case EUPHORIA_DECAY: setDecay(a_value); break;
        case EUPHORIA_SUSTAIN: setSustain(a_value); break;
        case EUPHORIA_RELEASE: setRelease(a_value); break;
        case EUPHORIA_FILTER_ATTACK: setFilterAttack(a_value); break;
        case EUPHORIA_FILTER_DECAY: setFilterDecay(a_value); break;
        case EUPHORIA_FILTER_SUSTAIN: setFilterSustain(a_value); break;
        case EUPHORIA_FILTER_RELEASE: setFilterRelease(a_value); break;
        case EUPHORIA_NOISE_AMP: setNoiseAmp(a_value); break;        
        case LMS_NOISE_TYPE: setNoiseType(a_value); break;       
        case EUPHORIA_PITCH_ENV_TIME: setPitchEnvTime(a_value); break;                
        case EUPHORIA_LFO_FREQ: setLFOfreq(a_value); break;            
        case EUPHORIA_LFO_TYPE:  setLFOtype(a_value);  break;
        //From Modulex            
        case EUPHORIA_FX0_KNOB0:	setFX0knob0(a_value); break;
        case EUPHORIA_FX0_KNOB1:	setFX0knob1(a_value); break;        
        case EUPHORIA_FX0_KNOB2:	setFX0knob2(a_value); break;        
        case EUPHORIA_FX0_COMBOBOX: setFX0combobox(a_value); break;

        case EUPHORIA_FX1_KNOB0:	setFX1knob0(a_value); break;
        case EUPHORIA_FX1_KNOB1:	setFX1knob1(a_value); break;        
        case EUPHORIA_FX1_KNOB2:	setFX1knob2(a_value); break;        
        case LMS_FX1_COMBOBOX: setFX1combobox(a_value); break;

        case LMS_FX2_KNOB0:	setFX2knob0(a_value); break;
        case LMS_FX2_KNOB1:	setFX2knob1(a_value); break;        
        case LMS_FX2_KNOB2:	setFX2knob2(a_value); break;        
        case LMS_FX2_COMBOBOX: setFX2combobox(a_value); break;

        case LMS_FX3_KNOB0:	setFX3knob0(a_value); break;
        case LMS_FX3_KNOB1:	setFX3knob1(a_value); break;        
        case LMS_FX3_KNOB2:	setFX3knob2(a_value); break;        
        case LMS_FX3_COMBOBOX: setFX3combobox(a_value); break;
        //End from Modulex            
        //From PolyFX mod matrix
        case LMS_PFXMATRIX_GRP0DST0SRC0CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[0]->controls[0]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST0SRC0CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[1]->controls[0]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST0SRC0CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[2]->controls[0]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST0SRC1CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[0]->controls[1]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST0SRC1CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[1]->controls[1]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST0SRC1CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[2]->controls[1]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST0SRC2CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[0]->controls[2]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST0SRC2CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[1]->controls[2]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST0SRC2CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[2]->controls[2]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST0SRC3CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[0]->controls[3]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST0SRC3CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[1]->controls[3]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST0SRC3CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[2]->controls[3]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST1SRC0CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[3]->controls[0]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST1SRC0CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[4]->controls[0]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST1SRC0CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[5]->controls[0]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST1SRC1CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[3]->controls[1]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST1SRC1CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[4]->controls[1]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST1SRC1CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[5]->controls[1]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST1SRC2CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[3]->controls[2]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST1SRC2CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[4]->controls[2]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST1SRC2CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[5]->controls[2]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST1SRC3CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[3]->controls[3]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST1SRC3CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[4]->controls[3]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST1SRC3CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[5]->controls[3]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST2SRC0CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[6]->controls[0]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST2SRC0CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[7]->controls[0]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST2SRC0CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[8]->controls[0]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST2SRC1CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[6]->controls[1]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST2SRC1CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[7]->controls[1]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST2SRC1CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[8]->controls[1]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST2SRC2CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[6]->controls[2]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST2SRC2CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[7]->controls[2]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST2SRC2CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[8]->controls[2]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST2SRC3CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[6]->controls[3]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST2SRC3CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[7]->controls[3]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST2SRC3CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[8]->controls[3]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST3SRC0CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[9]->controls[0]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST3SRC0CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[10]->controls[0]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST3SRC0CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[11]->controls[0]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST3SRC1CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[9]->controls[1]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST3SRC1CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[10]->controls[1]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST3SRC1CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[11]->controls[1]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST3SRC2CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[9]->controls[2]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST3SRC2CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[10]->controls[2]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST3SRC2CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[11]->controls[2]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST3SRC3CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[9]->controls[3]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST3SRC3CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[10]->controls[3]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST3SRC3CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[11]->controls[3]->lms_get_widget()))->setValue(a_value); break;

    }
    
}

void rayv_gui::v_control_changed(int a_port, int a_value, bool a_suppress_host_update)
{
    
#ifdef LMS_DEBUG_MODE_QT    
    rayv_cerr << "v_control_changed called.  ";  
    v_print_port_name_to_cerr(a_port);
    rayv_cerr << "  value: " << a_value << endl;
#endif
    
    if(a_suppress_host_update)
        m_suppressHostUpdate = true;      
    
    switch (a_port) {
    case WAYV_ATTACK: attackMainChanged(a_value); break;
    case WAYV_DECAY: decayMainChanged(a_value); break;
    case WAYV_SUSTAIN: sustainMainChanged(a_value); break;
    case WAYV_RELEASE: releaseMainChanged(a_value); break;
    
    case WAYV_ATTACK1: attack1Changed(a_value); break;
    case WAYV_DECAY1: decay1Changed(a_value); break;
    case WAYV_SUSTAIN1: sustain1Changed(a_value); break;
    case WAYV_RELEASE1: release1Changed(a_value); break;
    
    case WAYV_ATTACK2: attack2Changed(a_value); break;
    case WAYV_DECAY2: decay2Changed(a_value); break;
    case WAYV_SUSTAIN2: sustain2Changed(a_value); break;
    case WAYV_RELEASE2: release2Changed(a_value); break;
    
    case WAYV_NOISE_AMP: noiseAmpChanged(a_value); break;    
    case WAYV_OSC1_TYPE: osc1TypeChanged(a_value);  break;            
    case WAYV_OSC1_PITCH:  osc1PitchChanged(a_value);  break;    
    case WAYV_OSC1_TUNE: osc1TuneChanged(a_value); break;    
    case WAYV_OSC1_VOLUME: osc1VolumeChanged(a_value); break;
    case WAYV_OSC2_TYPE: osc2TypeChanged(a_value); break;            
    case WAYV_OSC2_PITCH: osc2PitchChanged(a_value); break;    
    case WAYV_OSC2_TUNE: osc2TuneChanged(a_value); break;    
    case WAYV_OSC2_VOLUME: osc2VolumeChanged(a_value); break;
    case WAYV_MASTER_VOLUME: masterVolumeChanged(a_value); break;
    case WAYV_MASTER_UNISON_VOICES: masterUnisonVoicesChanged(a_value); break;
    case WAYV_MASTER_UNISON_SPREAD: masterUnisonSpreadChanged(a_value); break;
    case WAYV_MASTER_GLIDE: masterGlideChanged(a_value); break;
    case WAYV_MASTER_PITCHBEND_AMT: masterPitchbendAmtChanged(a_value); break;
    case WAYV_PROGRAM_CHANGE: break; //ignoring this one, there is no reason to set it //programChanged(a_value);  break;
    
    
    
    case EUPHORIA_ATTACK: attackChanged(a_value); break;
    case EUPHORIA_DECAY: decayChanged(a_value); break;
    case EUPHORIA_SUSTAIN: sustainChanged(a_value); break;
    case EUPHORIA_RELEASE: releaseChanged(a_value); break;
    case EUPHORIA_FILTER_ATTACK: filterAttackChanged(a_value); break;
    case EUPHORIA_FILTER_DECAY: filterDecayChanged(a_value); break;
    case EUPHORIA_FILTER_SUSTAIN: filterSustainChanged(a_value); break;
    case EUPHORIA_FILTER_RELEASE: filterReleaseChanged(a_value); break;
    case EUPHORIA_NOISE_AMP: noiseAmpChanged(a_value); break;      
    case LMS_NOISE_TYPE: noise_typeChanged(a_value); break;             
    case EUPHORIA_PITCH_ENV_TIME: pitchEnvTimeChanged(a_value); break;
    case EUPHORIA_LFO_FREQ: LFOfreqChanged(a_value); break;
    case EUPHORIA_LFO_TYPE: LFOtypeChanged(a_value); break;
    //From Modulex            
    case EUPHORIA_FX0_KNOB0:	fx0knob0Changed(a_value); break;
    case EUPHORIA_FX0_KNOB1:	fx0knob1Changed(a_value); break;
    case EUPHORIA_FX0_KNOB2:	fx0knob2Changed(a_value); break;  
    case EUPHORIA_FX0_COMBOBOX:  fx0comboboxChanged(a_value); break;

    case EUPHORIA_FX1_KNOB0:	fx1knob0Changed(a_value); break;
    case EUPHORIA_FX1_KNOB1:	fx1knob1Changed(a_value); break;
    case EUPHORIA_FX1_KNOB2:	fx1knob2Changed(a_value); break;  
    case LMS_FX1_COMBOBOX:  fx1comboboxChanged(a_value); break;

    case LMS_FX2_KNOB0:	fx2knob0Changed(a_value); break;
    case LMS_FX2_KNOB1:	fx2knob1Changed(a_value); break;
    case LMS_FX2_KNOB2:	fx2knob2Changed(a_value); break;  
    case LMS_FX2_COMBOBOX:  fx2comboboxChanged(a_value); break;

    case LMS_FX3_KNOB0:	fx3knob0Changed(a_value); break;
    case LMS_FX3_KNOB1:	fx3knob1Changed(a_value); break;
    case LMS_FX3_KNOB2:	fx3knob2Changed(a_value); break;  
    case LMS_FX3_COMBOBOX:  fx3comboboxChanged(a_value); break;
    //End from Modulex
    //From PolyFX mod matrix
    case LMS_PFXMATRIX_GRP0DST0SRC0CTRL0:  pfxmatrix_grp0dst0src0ctrl0Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST0SRC0CTRL1:  pfxmatrix_grp0dst0src0ctrl1Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST0SRC0CTRL2:  pfxmatrix_grp0dst0src0ctrl2Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST0SRC1CTRL0:  pfxmatrix_grp0dst0src1ctrl0Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST0SRC1CTRL1:  pfxmatrix_grp0dst0src1ctrl1Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST0SRC1CTRL2:  pfxmatrix_grp0dst0src1ctrl2Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST0SRC2CTRL0:  pfxmatrix_grp0dst0src2ctrl0Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST0SRC2CTRL1:  pfxmatrix_grp0dst0src2ctrl1Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST0SRC2CTRL2:  pfxmatrix_grp0dst0src2ctrl2Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST0SRC3CTRL0:  pfxmatrix_grp0dst0src3ctrl0Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST0SRC3CTRL1:  pfxmatrix_grp0dst0src3ctrl1Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST0SRC3CTRL2:  pfxmatrix_grp0dst0src3ctrl2Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST1SRC0CTRL0:  pfxmatrix_grp0dst1src0ctrl0Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST1SRC0CTRL1:  pfxmatrix_grp0dst1src0ctrl1Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST1SRC0CTRL2:  pfxmatrix_grp0dst1src0ctrl2Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST1SRC1CTRL0:  pfxmatrix_grp0dst1src1ctrl0Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST1SRC1CTRL1:  pfxmatrix_grp0dst1src1ctrl1Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST1SRC1CTRL2:  pfxmatrix_grp0dst1src1ctrl2Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST1SRC2CTRL0:  pfxmatrix_grp0dst1src2ctrl0Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST1SRC2CTRL1:  pfxmatrix_grp0dst1src2ctrl1Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST1SRC2CTRL2:  pfxmatrix_grp0dst1src2ctrl2Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST1SRC3CTRL0:  pfxmatrix_grp0dst1src3ctrl0Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST1SRC3CTRL1:  pfxmatrix_grp0dst1src3ctrl1Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST1SRC3CTRL2:  pfxmatrix_grp0dst1src3ctrl2Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST2SRC0CTRL0:  pfxmatrix_grp0dst2src0ctrl0Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST2SRC0CTRL1:  pfxmatrix_grp0dst2src0ctrl1Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST2SRC0CTRL2:  pfxmatrix_grp0dst2src0ctrl2Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST2SRC1CTRL0:  pfxmatrix_grp0dst2src1ctrl0Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST2SRC1CTRL1:  pfxmatrix_grp0dst2src1ctrl1Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST2SRC1CTRL2:  pfxmatrix_grp0dst2src1ctrl2Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST2SRC2CTRL0:  pfxmatrix_grp0dst2src2ctrl0Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST2SRC2CTRL1:  pfxmatrix_grp0dst2src2ctrl1Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST2SRC2CTRL2:  pfxmatrix_grp0dst2src2ctrl2Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST2SRC3CTRL0:  pfxmatrix_grp0dst2src3ctrl0Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST2SRC3CTRL1:  pfxmatrix_grp0dst2src3ctrl1Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST2SRC3CTRL2:  pfxmatrix_grp0dst2src3ctrl2Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST3SRC0CTRL0:  pfxmatrix_grp0dst3src0ctrl0Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST3SRC0CTRL1:  pfxmatrix_grp0dst3src0ctrl1Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST3SRC0CTRL2:  pfxmatrix_grp0dst3src0ctrl2Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST3SRC1CTRL0:  pfxmatrix_grp0dst3src1ctrl0Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST3SRC1CTRL1:  pfxmatrix_grp0dst3src1ctrl1Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST3SRC1CTRL2:  pfxmatrix_grp0dst3src1ctrl2Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST3SRC2CTRL0:  pfxmatrix_grp0dst3src2ctrl0Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST3SRC2CTRL1:  pfxmatrix_grp0dst3src2ctrl1Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST3SRC2CTRL2:  pfxmatrix_grp0dst3src2ctrl2Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST3SRC3CTRL0:  pfxmatrix_grp0dst3src3ctrl0Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST3SRC3CTRL1:  pfxmatrix_grp0dst3src3ctrl1Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST3SRC3CTRL2:  pfxmatrix_grp0dst3src3ctrl2Changed(a_value); break;

    
    
    
    default:
#ifdef LMS_DEBUG_MODE_QT
	rayv_cerr << "Warning: received request to set nonexistent port " << a_port << endl;
#endif
        break;
    }
    
    if(a_suppress_host_update)
        m_suppressHostUpdate = false;
    
}

/*TODO:  For the forseeable future, this will only be used for getting the values to write back to 
 the presets.tsv file;  It should probably return a string that can be re-interpreted into other values for
 complex controls that could have multiple ints, or string values, etc...*/
int rayv_gui::i_get_control(int a_port)
{
        /*Add the controls you created to the control handler*/
    
    switch (a_port) {
    case WAYV_ATTACK: return  m_adsr_amp_main->lms_attack->lms_get_value();
    case WAYV_DECAY:  return m_adsr_amp_main->lms_decay->lms_get_value();
    case WAYV_SUSTAIN: return m_adsr_amp_main->lms_sustain->lms_get_value();
    case WAYV_RELEASE: return m_adsr_amp_main->lms_release->lms_get_value();
    case WAYV_NOISE_AMP: return m_noise_amp->lms_get_value();
    case WAYV_OSC1_TYPE: return m_osc1->lms_osc_type_box->lms_get_value();
    case WAYV_OSC1_PITCH: return m_osc1->lms_pitch_knob->lms_get_value();
    case WAYV_OSC1_TUNE: return  m_osc1->lms_fine_knob->lms_get_value();
    case WAYV_OSC1_VOLUME: return m_osc1->lms_vol_knob->lms_get_value();
    case WAYV_OSC2_TYPE:  return m_osc2->lms_osc_type_box->lms_get_value();
    case WAYV_OSC2_PITCH: return m_osc2->lms_pitch_knob->lms_get_value();
    case WAYV_OSC2_TUNE: return m_osc2->lms_fine_knob->lms_get_value();
    case WAYV_OSC2_VOLUME: return m_osc2->lms_vol_knob->lms_get_value();
    case WAYV_MASTER_VOLUME: return m_master->lms_master_volume->lms_get_value();
    case WAYV_MASTER_UNISON_VOICES: return m_master->lms_master_unison_voices->lms_get_value();
    case WAYV_MASTER_UNISON_SPREAD: return m_master->lms_master_unison_spread->lms_get_value();
    case WAYV_MASTER_GLIDE: return m_master->lms_master_glide->lms_get_value();
    case WAYV_MASTER_PITCHBEND_AMT: return m_master->lms_master_pitchbend_amt->lms_get_value();
    //case LMS_PROGRAM_CHANGE:
        //return m_program->currentIndex();
    default:
#ifdef LMS_DEBUG_MODE_QT
	rayv_cerr << "Warning: received request to get nonexistent port " << a_port << endl;
#endif
        return 0;
        break;
    }    
}

void rayv_gui::oscRecv()
{
    if (osc_server) {
	lo_server_recv_noblock(osc_server, 1);
    }
}

void rayv_gui::aboutToQuit()
{
    if (!m_hostRequestedQuit) lo_send(m_host, m_exitingPath, "");
}

rayv_gui::~rayv_gui()
{
    lo_address_free(m_host);
}


void rayv_osc_error(int num, const char *msg, const char *path)
{
#ifdef LMS_DEBUG_MODE_QT
    rayv_cerr << "Error: liblo server error " << num
	 << " in path \"" << (path ? path : "(null)")
	 << "\": " << msg << endl;
#endif
}

int rayv_debug_handler(const char *path, const char *types, lo_arg **argv,
	      int argc, void *data, void *user_data)
{
    int i;
#ifdef LMS_DEBUG_MODE_QT
      rayv_cerr << "Warning: unhandled OSC message in GUI:" << endl;
#endif
    

    for (i = 0; i < argc; ++i) {
#ifdef LMS_DEBUG_MODE_QT
	rayv_cerr << "arg " << i << ": type '" << types[i] << "': ";
#endif
        lo_arg_pp((lo_type)types[i], argv[i]);
#ifdef LMS_DEBUG_MODE_QT
	rayv_cerr << endl;
#endif
    }
#ifdef LMS_DEBUG_MODE_QT
    rayv_cerr << "(path is <" << path << ">)" << endl;
#endif
    return 1;
}

int rayv_program_handler(const char *path, const char *types, lo_arg **argv,
	       int argc, void *data, void *user_data)
{
    rayv_gui *gui = static_cast<rayv_gui *>(user_data);
    
    //int bank = 0;
    int program = 0;

    /*
    if (argc < 2) {
        //GDB_MESSAGE(GDB_OSC, " error: too few arguments to osc_program_handler\n");
        return 1;
    }

    
    bank = argv[0]->i;
    program = argv[1]->i;

    if (bank || program < 0 || program >= 128) {        
        return 0;
    }
    */
#ifdef LMS_DEBUG_MODE_QT
    rayv_cerr << "Bank:  " << bank << ", Program:  " << program << endl;
#endif
    gui->setProgram(program);

    return 0;
}

int rayv_configure_handler(const char *path, const char *types, lo_arg **argv,
		  int argc, void *data, void *user_data)
{
    //rayv_gui *gui = static_cast<rayv_gui *>(user_data);
    const char *key = (const char *)&argv[0]->s;
    const char *value = (const char *)&argv[1]->s;

    rayv_cerr << "GUI configure_handler:  Key:  " << QString::fromLocal8Bit(key) << " , Value:" << QString::fromLocal8Bit(value);
    
    return 0;
}

int rayv_rate_handler(const char *path, const char *types, lo_arg **argv,
	     int argc, void *data, void *user_data)
{
    return 0; /* ignore it */
}

int rayv_show_handler(const char *path, const char *types, lo_arg **argv,
	     int argc, void *data, void *user_data)
{
    rayv_gui *gui = static_cast<rayv_gui *>(user_data);
    while (!gui->ready()) sleep(1);
    if (gui->isVisible()) gui->raise();
    else gui->show();
    return 0;
}

int rayv_hide_handler(const char *path, const char *types, lo_arg **argv,
	     int argc, void *data, void *user_data)
{
    rayv_gui *gui = static_cast<rayv_gui *>(user_data);
    gui->hide();
    return 0;
}

int rayv_quit_handler(const char *path, const char *types, lo_arg **argv,
	     int argc, void *data, void *user_data)
{
    rayv_gui *gui = static_cast<rayv_gui *>(user_data);
    gui->setHostRequestedQuit(true);
    qApp->quit();
    return 0;
}

int rayv_control_handler(const char *path, const char *types, lo_arg **argv,
		int argc, void *data, void *user_data)
{
    rayv_gui *gui = static_cast<rayv_gui *>(user_data);

    if (argc < 2) {
        
#ifdef LMS_DEBUG_MODE_QT
	rayv_cerr << "Error: too few arguments to control_handler" << endl;
#endif
	return 1;
    }

    const int port = argv[0]->i;
    const float value = argv[1]->f;

#ifdef LMS_DEBUG_MODE_QT
    rayv_cerr << "control_handler called.  port:  " << port << " , value " << value << endl;
#endif

    gui->v_set_control(port, value);  
     
    //gui->v_control_changed(port, value, true);

    return 0;
}

int main(int argc, char **argv)
{
    for(int i = 0; i < argc; i++)
    {
        rayv_cerr << "argv[" << i << "] == " << argv[i] << "\n";
    }

    rayv_cerr << "Qt GUI main() called..." << endl;

    
    QApplication application(argc, argv);

    if (application.argc() < 5)
    {

	rayv_cerr << "usage: "
	     << application.argv()[0] 
	     << " <osc url>"
	     << " <plugin dllname>"
	     << " <plugin label>"
	     << " <user-friendly id>"
	     << endl;

	return 2;
    }

#ifdef Q_WS_X11
    XSetErrorHandler(handle_x11_error);
#endif

    char *url = application.argv()[1];

    char *host = lo_url_get_hostname(url);
    char *port = lo_url_get_port(url);
    char *path = lo_url_get_path(url);
    
    rayv_cerr << "host: " << host << " port: " << port << " path: " << path << "\n";
        
    rayv_cerr << QString("argc==") << QString::number(argc) << QString("\n");
    

    rayv_gui gui(host, port,
		 QByteArray(path) + "/control",
		 QByteArray(path) + "/midi",
		 QByteArray(path) + "/program",
		 QByteArray(path) + "/exiting",
		 0);
 
    QByteArray myControlPath = QByteArray(path) + "/control";
    QByteArray myProgramPath = QByteArray(path) + "/program";
    QByteArray myConfigurePath = QByteArray(path) + "/configure";
    QByteArray myRatePath = QByteArray(path) + "/sample-rate";
    QByteArray myShowPath = QByteArray(path) + "/show";
    QByteArray myHidePath = QByteArray(path) + "/hide";
    QByteArray myQuitPath = QByteArray(path) + "/quit";

    rayv_cerr << "Adding lo server methods" << endl;

    osc_server = lo_server_new(NULL, rayv_osc_error);
    lo_server_add_method(osc_server, myControlPath, "if", rayv_control_handler, &gui);
    lo_server_add_method(osc_server, myProgramPath, "ii", rayv_program_handler, &gui);
    lo_server_add_method(osc_server, myConfigurePath, "ss", rayv_configure_handler, &gui);
    lo_server_add_method(osc_server, myRatePath, "i", rayv_rate_handler, &gui);
    lo_server_add_method(osc_server, myShowPath, "", rayv_show_handler, &gui);
    lo_server_add_method(osc_server, myHidePath, "", rayv_hide_handler, &gui);
    lo_server_add_method(osc_server, myQuitPath, "", rayv_quit_handler, &gui);
    lo_server_add_method(osc_server, NULL, NULL, rayv_debug_handler, &gui);

    lo_address hostaddr = lo_address_new(host, port);
    lo_send(hostaddr,
	    QByteArray(path) + "/update",
	    "s",
	    (QByteArray(lo_server_get_url(osc_server))+QByteArray(path+1)).data());

    QObject::connect(&application, SIGNAL(aboutToQuit()), &gui, SLOT(aboutToQuit()));

    gui.setWindowTitle(QString("PyDAW - Way-V - ") + application.argv()[3]);
    gui.setReady(true);
    
    rayv_cerr << "Starting GUI now..." << endl;
    
    return application.exec();
}

