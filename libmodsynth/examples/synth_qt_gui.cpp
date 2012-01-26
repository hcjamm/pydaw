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
#include <QTextStream>
#include <QTimer>
#include <iostream>
#include <unistd.h>

#include <qt4/QtGui/qgroupbox.h>
#include <qt4/QtGui/qlayout.h>
#include <qt4/QtGui/qlabel.h>
#include <qt4/QtGui/qgridlayout.h>
#include <QFormLayout>
#include <qt4/QtGui/qboxlayout.h>
#include <QGroupBox>
#include <qt4/QtGui/qdial.h>
#include <QPixmap>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QMessageBox>

#include <stdlib.h>
#include "libmodsynth/lib/amp.h"
#include "libmodsynth/lib/pitch_core.h"

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


//Comment this out when you compile a stable release version on the plugin, you don't want it printing debug output unless you're debugging
#define LMS_DEBUG_MODE_QT

/*This is used for things like naming the preset file, etc...*/
#define LMS_PLUGIN_NAME "ray-v"
//#define PROGRAM_XML_FILE  QDir::homePath() + "/dssi/" + PLUGIN_NAME + "-presets.xml"


using std::endl;

/*GUI Step 6:  Define ports for each control that will send messages to the plugin, such as dials/knobs or faders.*/

//TODO:  put something at zero here, and update the below iterator
#define LTS_PORT_ATTACK  1
#define LTS_PORT_DECAY   2
#define LTS_PORT_SUSTAIN 3
#define LTS_PORT_RELEASE 4
#define LTS_PORT_TIMBRE  5
#define LTS_PORT_RES  6
#define LTS_PORT_DIST  7
#define LTS_PORT_FILTER_ATTACK  8
#define LTS_PORT_FILTER_DECAY   9
#define LTS_PORT_FILTER_SUSTAIN 10
#define LTS_PORT_FILTER_RELEASE 11
#define LTS_PORT_NOISE_AMP 12
#define LTS_PORT_FILTER_ENV_AMT 13
#define LTS_PORT_DIST_WET 14
#define LTS_PORT_OSC1_TYPE 15
#define LTS_PORT_OSC1_PITCH 16
#define LTS_PORT_OSC1_TUNE 17
#define LTS_PORT_OSC1_VOLUME 18
#define LTS_PORT_OSC2_TYPE 19
#define LTS_PORT_OSC2_PITCH 20
#define LTS_PORT_OSC2_TUNE 21
#define LTS_PORT_OSC2_VOLUME 22
#define LTS_PORT_MASTER_VOLUME 23

#define LTS_PORT_MASTER_UNISON_VOICES 24
#define LTS_PORT_MASTER_UNISON_SPREAD 25
#define LTS_PORT_MASTER_GLIDE 26
#define LTS_PORT_MASTER_PITCHBEND_AMT 27
#define LTS_PORT_MAX 28  //This corresponds to the highest number, you must update this when adding or removing controls


lo_server osc_server = 0;

static QTextStream cerr(stderr);

SynthGUI::SynthGUI(const char * host, const char * port,
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
    
    /*Set the CSS style that will "cascade" on the other controls.*/
    this->setStyleSheet("QGroupBox {background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #E0E0E0, stop: 1 #FFFFFF); border: 2px solid gray;  border-radius: 10px;  margin-top: 1ex; } QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 3px; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #FFOECE, stop: 1 #FFFFFF); }");

    
    
    QString _osc_types [] = {"Saw", "Square", "Triangle", "Sine", "Off"};
    int _osc_types_count = 5;
    
    
    /*Getting the program list from the presets file*/
    
    QString f_programs_list [128];
    
    QString f_preset_path = QDir::homePath() + "/dssi";
    
    QDir* f_dir = new QDir(QDir::homePath());
    
    if(!f_dir->exists(f_preset_path))
        f_dir->mkpath(f_preset_path);
    
    QFile* f_file = new QFile(f_preset_path + "/" + LMS_PLUGIN_NAME + "-presets.tsv");
    
    if(!f_file->open(QIODevice::ReadOnly)) 
    {
        cerr << "Failed to open preset file:  " << f_preset_path << "/" << LMS_PLUGIN_NAME << "-presets.tsv" << "\n";
        
        for(int i = 0; i < 128; i++) 
        {
            f_programs_list[i] = "empty";
            presets_tab_delimited[i] = "empty";
        }
        
    }
    else
    {
        QTextStream * in = new QTextStream(f_file);

        for(int i = 0; i < 128; i++) 
        {
            if(in->atEnd())
            {
                f_programs_list[i] = "empty";
                presets_tab_delimited[i] = "empty";
            }
            else
            {
                QString line = in->readLine();    

                QStringList fields = line.split("\t");    

                f_programs_list[i] = fields.at(0);

                presets_tab_delimited[i] = line;
            }

        }

    }
    
    f_file->close();
    
    /*End getting the program list from the presets file*/
    
    //QGridLayout *layout = new QGridLayout(this);
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    QHBoxLayout *layout_row0 = new QHBoxLayout();
    
    layout->addLayout(layout_row0);
        
    QGroupBox * f_gb_program = newGroupBox("Program", this);    
    QHBoxLayout *layout_program = new QHBoxLayout();
    
    QLabel *m_prog_label = new QLabel(this);
    m_prog_label->setText("Program");
    layout_program->addWidget(m_prog_label, -1, Qt::AlignLeft);
    
    
    m_program = get_combobox( f_programs_list, 128, this);
    
    m_program->setEditable(TRUE);
    connect(m_program, SIGNAL(currentIndexChanged(int)), this, SLOT(programChanged(int)));        
    layout_program->addWidget(m_program, -1, Qt::AlignLeft);
    
    m_prog_save = new QPushButton(this);
    m_prog_save->setText("Save");
    connect(m_prog_save, SIGNAL(pressed()), this, SLOT(programSaved()));
    layout_program->addWidget(m_prog_save);
    
    //TODO:  Run a load-default-bank method
    f_gb_program->setLayout(layout_program);
    layout_row0->addWidget(f_gb_program, -1, Qt::AlignLeft);
    
    QPushButton *testButton = new QPushButton("Test Note", this);
    connect(testButton, SIGNAL(pressed()), this, SLOT(test_press()));
    connect(testButton, SIGNAL(released()), this, SLOT(test_release()));    
        
    layout_row0->addWidget(testButton, -1, Qt::AlignRight);
    
    
    int f_row = 0;
    int f_column = 0;
    
    int f_gb_layout_row = 0;
    int f_gb_layout_column = 0;
    /*GUI Step 4:  Lay out the controls you declared in the first step*/
    
    
    QHBoxLayout *layout_row1 = new QHBoxLayout();
    
    /*The oscillator1 GroupBox*/
    QGroupBox * f_gb_osc1 = newGroupBox("Osc1", this);
    QGridLayout *f_gb_osc1_layout = new QGridLayout(f_gb_osc1);
    
    m_osc1_pitch = get_knob(minus12_to_12);
    m_osc1_pitchLabel = newQLabel(this);
    add_widget(f_gb_osc1_layout, f_gb_layout_column, f_gb_layout_row, "Pitch", m_osc1_pitch, m_osc1_pitchLabel);
    connect(m_osc1_pitch, SIGNAL(valueChanged(int)), this, SLOT(osc1PitchChanged(int)));
    osc1PitchChanged(m_osc1_pitch->value());
    
    f_gb_layout_column++;
    
    m_osc1_tune = get_knob(minus1_to_1);
    m_osc1_tuneLabel = newQLabel(this);
    add_widget(f_gb_osc1_layout, f_gb_layout_column, f_gb_layout_row, "Tune", m_osc1_tune, m_osc1_tuneLabel);
    connect(m_osc1_tune, SIGNAL(valueChanged(int)), this, SLOT(osc1TuneChanged(int)));
    osc1TuneChanged(m_osc1_tune->value());
    
    f_gb_layout_column++;
    
    m_osc1_volume = get_knob(decibels_0, -6);
    m_osc1_volumeLabel = newQLabel(this);
    add_widget(f_gb_osc1_layout, f_gb_layout_column, f_gb_layout_row, "Vol", m_osc1_volume, m_osc1_volumeLabel);
    connect(m_osc1_volume, SIGNAL(valueChanged(int)), this, SLOT(osc1VolumeChanged(int)));
    osc1VolumeChanged(m_osc1_volume->value());
    
    f_gb_layout_column++;
    
    m_osc1_type = get_combobox(_osc_types, _osc_types_count , this);     
    add_widget_no_label(f_gb_osc1_layout, f_gb_layout_column, f_gb_layout_row, "Type", m_osc1_type);
    connect(m_osc1_type, SIGNAL(currentIndexChanged(int)), this, SLOT(osc1TypeChanged(int)));
    osc1TypeChanged(m_osc1_type->currentIndex());
    
    layout_row1->addWidget(f_gb_osc1, -1, Qt::AlignLeft);
    f_gb_layout_row = 0;
    f_gb_layout_column = 0;
    f_column++;
    
    /*The amplitude ADSR GroupBox*/
    QGroupBox * f_gb_adsr = newGroupBox("ADSR Amp", this);
    QGridLayout *f_gb_adsr_layout = new QGridLayout(f_gb_adsr);
    
    
    
    m_attack = get_knob(zero_to_one);
    m_attackLabel = newQLabel(this);
    add_widget(f_gb_adsr_layout, f_gb_layout_column, f_gb_layout_row, "Attack",m_attack, m_attackLabel);
    connect(m_attack,   SIGNAL(valueChanged(int)), this, SLOT(attackChanged(int)));
    attackChanged  (m_attack  ->value());
    
    f_gb_layout_column++;
        
    m_decay   =  get_knob(zero_to_one); //newQDial(  1, 100,  1,  25); // s * 100
    m_decayLabel   = newQLabel(this);
    add_widget(f_gb_adsr_layout, f_gb_layout_column, f_gb_layout_row, "Decay",m_decay, m_decayLabel);
    connect(m_decay,   SIGNAL(valueChanged(int)), this, SLOT(decayChanged(int)));
    decayChanged  (m_decay  ->value());
    
    f_gb_layout_column++;
    
    m_sustain =  get_knob(decibels_0); // newQDial(  0, 100,  1,  75); // %
    m_sustainLabel = newQLabel(this);
    add_widget(f_gb_adsr_layout, f_gb_layout_column, f_gb_layout_row, "Sustain", m_sustain, m_sustainLabel);    
    connect(m_sustain, SIGNAL(valueChanged(int)), this, SLOT(sustainChanged(int)));
    sustainChanged(m_sustain->value());
    
    f_gb_layout_column++;
    
    m_release = get_knob(zero_to_four); //newQDial(  1, 400, 10, 200); // s * 100
    m_releaseLabel = newQLabel(this);
    add_widget(f_gb_adsr_layout, f_gb_layout_column, f_gb_layout_row, "Release", m_release, m_releaseLabel);
    connect(m_release, SIGNAL(valueChanged(int)), this, SLOT(releaseChanged(int)));
    releaseChanged(m_release->value());
    
    layout_row1->addWidget(f_gb_adsr, -1, Qt::AlignLeft);
    f_column++;
    f_gb_layout_row = 0;
    f_gb_layout_column = 0;
        
    
    /*The Distortion GroupBox*/
    QGroupBox * f_gb_dist = newGroupBox("Distortion", this);    
    QGridLayout *f_gb_dist_layout = new QGridLayout(f_gb_dist);
    
    
    m_dist  = newQDial(  -6, 36,  1,  -6); 
    m_distLabel  = new QLabel(this);
    add_widget(f_gb_dist_layout, f_gb_layout_column, f_gb_layout_row, "Gain", m_dist, m_distLabel);
    connect(m_dist,  SIGNAL(valueChanged(int)), this, SLOT(distChanged(int)));
    distChanged (m_dist ->value());
    
    f_gb_layout_column++;
    
    m_dist_wet  = get_knob(zero_to_one);    
    add_widget_no_label(f_gb_dist_layout, f_gb_layout_column, f_gb_layout_row, "Wet", m_dist_wet);
    connect(m_dist_wet,  SIGNAL(valueChanged(int)), this, SLOT(distWetChanged(int)));
    distWetChanged (m_dist_wet ->value());
    
    layout_row1->addWidget(f_gb_dist, -1, Qt::AlignLeft);
    //layout->addWidget(_gb_dist, _row, _column, Qt::AlignCenter);    
    f_column++;
    f_gb_layout_row = 0;
    f_gb_layout_column = 0;
    
    
    
    /*The Noise Amp GroupBox*/
    QGroupBox * f_gb_noise_amp = newGroupBox("Noise", this);    
    QGridLayout *f_gb_noise_amp_layout = new QGridLayout(f_gb_noise_amp);
    
    
    m_noise_amp  = get_knob(decibels_0);
    m_noise_ampLabel  = new QLabel(this);
    add_widget(f_gb_noise_amp_layout, f_column, f_row, "Vol", m_noise_amp, m_noise_ampLabel);
    connect(m_noise_amp,  SIGNAL(valueChanged(int)), this, SLOT(noiseAmpChanged(int)));
    noiseAmpChanged (m_dist ->value());
    
    layout_row1->addWidget(f_gb_noise_amp, -1, Qt::AlignLeft);
    
    /*Start a new row*/
    f_row++;
    f_column = 0;
    
    
    layout->addLayout(layout_row1, -1);    
    
    QHBoxLayout *layout_row2 = new QHBoxLayout();
    
    
    /*The oscillator2 GroupBox*/
    QGroupBox * f_gb_osc2 = newGroupBox("Osc2", this);
    QGridLayout *f_gb_osc2_layout = new QGridLayout(f_gb_osc2);
    
    m_osc2_pitch = get_knob(minus12_to_12);
    m_osc2_pitchLabel = newQLabel(this);
    add_widget(f_gb_osc2_layout, f_gb_layout_column, f_gb_layout_row, "Pitch", m_osc2_pitch, m_osc2_pitchLabel);
    connect(m_osc2_pitch, SIGNAL(valueChanged(int)), this, SLOT(osc2PitchChanged(int)));
    osc2PitchChanged(m_osc2_pitch->value());
            
    f_gb_layout_column++;
    
    m_osc2_tune = get_knob(minus1_to_1);
    m_osc2_tuneLabel = newQLabel(this);
    add_widget(f_gb_osc2_layout, f_gb_layout_column, f_gb_layout_row, "Tune", m_osc2_tune, m_osc2_tuneLabel);
    connect(m_osc2_tune, SIGNAL(valueChanged(int)), this, SLOT(osc2TuneChanged(int)));
    osc2TuneChanged(m_osc2_tune->value());
    
    f_gb_layout_column++;
    
    m_osc2_volume = get_knob(decibels_0, -60);
    m_osc2_volumeLabel = newQLabel(this);
    add_widget(f_gb_osc2_layout, f_gb_layout_column, f_gb_layout_row, "Vol", m_osc2_volume, m_osc2_volumeLabel);
    connect(m_osc2_volume, SIGNAL(valueChanged(int)), this, SLOT(osc2VolumeChanged(int)));
    osc2VolumeChanged(m_osc2_volume->value());
    
    f_gb_layout_column++;
    
    m_osc2_type = get_combobox(_osc_types, _osc_types_count , this);     
    add_widget_no_label(f_gb_osc2_layout, f_gb_layout_column, f_gb_layout_row, "Type", m_osc2_type);
    connect(m_osc2_type, SIGNAL(currentIndexChanged(int)), this, SLOT(osc2TypeChanged(int)));
    osc2TypeChanged(m_osc2_type->currentIndex());
    
    layout_row2->addWidget(f_gb_osc2, -1, Qt::AlignLeft);
    f_gb_layout_row = 0;
    f_gb_layout_column = 0;
    f_column++;
    
    /*The filter ADSR GroupBox*/
    QGroupBox * f_gb_adsr_f = newGroupBox("ADSR Filter", this);
    QGridLayout *f_gb_adsr_f_layout = new QGridLayout(f_gb_adsr_f);
    
    f_gb_layout_row = 0;
    f_gb_layout_column = 0;
    
    m_filter_attack = get_knob(zero_to_one);
    m_filter_attackLabel = newQLabel(this);
    add_widget(f_gb_adsr_f_layout, f_gb_layout_column, f_gb_layout_row, "Attack",m_filter_attack, m_filter_attackLabel);
    connect(m_filter_attack,   SIGNAL(valueChanged(int)), this, SLOT(filterAttackChanged(int)));
    filterAttackChanged  (m_filter_attack  ->value());
    
    f_gb_layout_column++;
        
    m_filter_decay   =  get_knob(zero_to_one); //newQDial(  1, 100,  1,  25); // s * 100
    m_filter_decayLabel   = newQLabel(this);
    add_widget(f_gb_adsr_f_layout, f_gb_layout_column, f_gb_layout_row, "Decay",m_filter_decay, m_filter_decayLabel);
    connect(m_filter_decay,   SIGNAL(valueChanged(int)), this, SLOT(filterDecayChanged(int)));
    filterDecayChanged  (m_filter_decay ->value());
    
    f_gb_layout_column++;
    
    m_filter_sustain =  get_knob(zero_to_one); // newQDial(  0, 100,  1,  75); // %
    m_filter_sustainLabel = newQLabel(this);
    add_widget(f_gb_adsr_f_layout, f_gb_layout_column, f_gb_layout_row, "Sustain", m_filter_sustain, m_filter_sustainLabel);
    connect(m_filter_sustain, SIGNAL(valueChanged(int)), this, SLOT(filterSustainChanged(int)));
    filterSustainChanged(m_filter_sustain->value());
    
    f_gb_layout_column++;
    
    m_filter_release = get_knob(zero_to_four); //newQDial(  1, 400, 10, 200); // s * 100
    m_filter_releaseLabel = newQLabel(this);
    add_widget(f_gb_adsr_f_layout, f_gb_layout_column, f_gb_layout_row, "Release", m_filter_release, m_filter_releaseLabel);
    connect(m_filter_release, SIGNAL(valueChanged(int)), this, SLOT(filterReleaseChanged(int)));    
    filterReleaseChanged(m_filter_release->value());
        
    layout_row2->addWidget(f_gb_adsr_f, -1, Qt::AlignLeft);
    f_gb_layout_row = 0;
    f_gb_layout_column = 0;
    f_column++;
    
    
    /*The Filter GroupBox*/
    QGroupBox * _gb_filter = newGroupBox("LP Filter", this); 
    QGridLayout *_gb_filter_layout = new QGridLayout(_gb_filter);
    
    m_timbre  =  get_knob(pitch);  //newQDial(  39, 136,  1,  82); // s * 100
    m_timbreLabel  = newQLabel(this);
    add_widget(_gb_filter_layout, f_gb_layout_column, f_gb_layout_row, "Cutoff",m_timbre, m_timbreLabel);
    connect(m_timbre,  SIGNAL(valueChanged(int)), this, SLOT(timbreChanged(int)));
    timbreChanged (m_timbre ->value());
    
    f_gb_layout_column++;
    
    m_res  =  get_knob(decibels_0); 
    m_resLabel  = newQLabel(this);
    add_widget(_gb_filter_layout, f_gb_layout_column, f_gb_layout_row, "Res", m_res, m_resLabel);
    connect(m_res,  SIGNAL(valueChanged(int)), this, SLOT(resChanged(int)));
    resChanged (m_res ->value());
    
    f_gb_layout_column++;
    
    m_filter_env_amt  =  get_knob(minus36_to_36); 
    m_filter_env_amtLabel  = newQLabel(this);
    add_widget(_gb_filter_layout, f_gb_layout_column, f_gb_layout_row, "Env", m_filter_env_amt, m_filter_env_amtLabel);
    connect(m_filter_env_amt,  SIGNAL(valueChanged(int)), this, SLOT(filterEnvAmtChanged(int)));
    filterEnvAmtChanged (m_filter_env_amt ->value());
    
    layout_row2->addWidget(_gb_filter, -1, Qt::AlignLeft);
    f_column++;
    f_gb_layout_row = 0;
    f_gb_layout_column = 0;
    
    layout->addLayout(layout_row2, -1);
    
    QHBoxLayout *layout_row3 = new QHBoxLayout();
    
    /*The Master Volume GroupBox*/
    QGroupBox * f_gb_master_vol = newGroupBox("Master", this); 
    QGridLayout *f_gb_master_vol_layout = new QGridLayout(f_gb_master_vol);
    
    m_master_volume  =  get_knob(decibels_plus_12);
    m_master_volumeLabel  = newQLabel(this);
    add_widget(f_gb_master_vol_layout, f_gb_layout_column, f_gb_layout_row, "Volume",m_master_volume, m_master_volumeLabel);
    connect(m_master_volume,  SIGNAL(valueChanged(int)), this, SLOT(masterVolumeChanged(int)));
    masterVolumeChanged (m_master_volume ->value());
    
    f_gb_layout_column++;
    
    
    
    m_master_unison_voices  =  newQDial(1, 7, 1, 1);
    m_master_unison_voicesLabel  = newQLabel(this);
    add_widget(f_gb_master_vol_layout, f_gb_layout_column, f_gb_layout_row, "Unison",m_master_unison_voices, m_master_unison_voicesLabel);
    connect(m_master_unison_voices,  SIGNAL(valueChanged(int)), this, SLOT( masterUnisonVoicesChanged(int)));
    masterUnisonVoicesChanged (m_master_unison_voices ->value());
    
    f_gb_layout_column++;
    
    m_master_unison_spread  =  get_knob(zero_to_one);
    m_master_unison_spreadLabel  = newQLabel(this);
    add_widget(f_gb_master_vol_layout, f_gb_layout_column, f_gb_layout_row, "Spread",m_master_unison_spread, m_master_unison_spreadLabel);
    connect(m_master_unison_spread,  SIGNAL(valueChanged(int)), this, SLOT(masterUnisonSpreadChanged(int)));
    masterUnisonSpreadChanged (m_master_unison_spread ->value());
    
    f_gb_layout_column++;
    
    m_master_glide  =  get_knob(zero_to_one);
    m_master_glideLabel  = newQLabel(this);
    add_widget(f_gb_master_vol_layout, f_gb_layout_column, f_gb_layout_row, "Glide",m_master_glide, m_master_glideLabel);
    connect(m_master_glide,  SIGNAL(valueChanged(int)), this, SLOT(masterGlideChanged(int)));
    masterGlideChanged (m_master_glide ->value());
    
    f_gb_layout_column++;
    
    m_master_pitchbend_amt  =  newQDial(1, 36, 1, 2);
    m_master_pitchbend_amtLabel  = newQLabel(this);
    add_widget(f_gb_master_vol_layout, f_gb_layout_column, f_gb_layout_row, "Pitchbend",m_master_pitchbend_amt, m_master_pitchbend_amtLabel);
    connect(m_master_pitchbend_amt,  SIGNAL(valueChanged(int)), this, SLOT(masterPitchbendAmtChanged(int)));
    masterPitchbendAmtChanged (m_master_pitchbend_amt ->value());
    
    f_gb_layout_column++;
    
    
    layout_row3->addWidget(f_gb_master_vol, -1, Qt::AlignLeft);
    f_column++;
    f_gb_layout_row = 0;
    f_gb_layout_column = 0;
    
    QLabel * f_logo_label = new QLabel("", this);
    f_logo_label->setPixmap(QPixmap(QString::fromUtf8("/usr/local/lib/dssi/synth/ray-v-logo.png")));
    f_logo_label->setMinimumSize(90, 30);   
    f_logo_label->show();
    layout_row3->addWidget(f_logo_label, -1, Qt::AlignRight);
        
        
    /*IMPORTANT:  Adding the last row, don't delete this*/
    
    layout->addLayout(layout_row3, -1);
        
    /*End test button code, DO NOT remove the code below this*/

    programChanged(0);   //Changing the preset to the first one
    
    QTimer *myTimer = new QTimer(this);
    connect(myTimer, SIGNAL(timeout()), this, SLOT(oscRecv()));
    myTimer->setSingleShot(false);
    myTimer->start(0);
    
    m_suppressHostUpdate = false;
}



void SynthGUI::add_widget(QGridLayout * a_layout, int a_position_x, int a_position_y, QString a_label_text,  QWidget * a_widget,
    QLabel * _label)
{   
    QLabel * f_knob_title = new QLabel(a_label_text,  this);
    f_knob_title->setMinimumWidth(60);  //TODO:  make this a constant
    f_knob_title->setAlignment(Qt::AlignCenter);
    f_knob_title->setStyleSheet("background-color: white; border: 1px solid black;  border-radius: 6px;");  //TODO:  make this a constant string for all knobs
    
    a_layout->addWidget(f_knob_title, a_position_y, a_position_x, Qt::AlignCenter);    
    a_layout->addWidget(a_widget,  (a_position_y + 1), a_position_x);
    a_layout->addWidget(_label,  (a_position_y + 2), a_position_x, Qt::AlignCenter);     
}

void SynthGUI::add_widget_no_label(QGridLayout * a_layout, int a_position_x, int a_position_y, QString a_label_text, QWidget * a_widget)
{
    QLabel * f_knob_title = new QLabel(a_label_text,  this);
    f_knob_title->setMinimumWidth(60);  //TODO:  make this a constant
    f_knob_title->setAlignment(Qt::AlignCenter);
    f_knob_title->setStyleSheet("background-color: white; border: 1px solid black;  border-radius: 6px;");    //TODO:  make this a constant string for all knobs
    
    a_layout->addWidget(f_knob_title, a_position_y, a_position_x, Qt::AlignCenter);    
    a_layout->addWidget(a_widget,  (a_position_y + 1), a_position_x);    
}

QGroupBox * SynthGUI::newGroupBox(QString a_title, QWidget * a_parent)
{
    QGroupBox * f_result = new QGroupBox(a_parent);
    
    f_result->setTitle(a_title);
    f_result->setAlignment(Qt::AlignHCenter);
    return f_result;
}

QLabel * SynthGUI::newQLabel(QWidget * a_parent)
{
    QLabel * f_result = new QLabel(a_parent);
    //_result->setStyleSheet("background-color: white; border: 2px solid black;  border-radius: 6px;");
    return f_result;
}

QDial * SynthGUI::get_knob(_knob_type a_ktype, int a_default_value)
{
    int f_min, f_max, f_step, f_value;
    
    switch(a_ktype)
    {
        case decibels_0:
                f_min = -60; f_max = 0; f_step = 1; f_value = -6; 
                break;
        case decibels_plus_12:
            f_min = -60; f_max = 12; f_step = 1; f_value = -6;            
            break;
        case decibels_plus_24:
            f_min = -60; f_max = 24; f_step = 1; f_value = -6;            
            break;
        case decibels_plus_6:            
            f_min = -60; f_max = 6; f_step = 1; f_value = -6;            
            break;
        case pitch:
            f_min = 20; f_max = 124; f_step = 1; f_value = 105;            
            break;
        case zero_to_four:
            f_min = 1; f_max = 400; f_step = 4; f_value = 75;            
            break;
        case zero_to_one:
            f_min = 1; f_max = 100; f_step = 1; f_value = 15;            
            break;
        case zero_to_two:
            f_min = 1; f_max = 200; f_step = 2; f_value = 25;            
            break;
        case minus1_to_1:
            f_min = -100; f_max = 100; f_step = 1; f_value = 0;            
            break;
        case minus12_to_12:
            f_min = -12; f_max = 12; f_step = 1; f_value = 0;            
            break;
        case minus24_to_24:
            f_min = -24; f_max = 24; f_step = 1; f_value = 0;            
            break;
        case minus36_to_36:
            f_min = -36; f_max = 36; f_step = 1; f_value = 0;            
            break;
    }
    
    if(a_default_value != 333)  //This makes the assumption that we will never pick 333 as a default value
    {
        f_value = a_default_value;
    }
    
     return newQDial(f_min, f_max, f_step, f_value);
    
}

QCheckBox * SynthGUI::get_checkbox(std::string a_text)
{
    QCheckBox * f_checkbox = new QCheckBox(this);
    
    f_checkbox->setText(QString::fromStdString(a_text));
    
    //TODO:  add a skin to make it look like a toggle-switch
        
    return f_checkbox;
}


QDial * SynthGUI::newQDial( int minValue, int maxValue, int pageStep, int value )
{
    QDial *dial = new QDial( this );
    dial->setMinimum( minValue );
    dial->setMaximum( maxValue );
    dial->setPageStep( pageStep );
    dial->setValue( value );
    dial->setNotchesVisible(false); 
    //TODO:  Make this a constant value
    dial->setMaximumHeight(66);
    dial->setMaximumWidth(66);
    dial->setMinimumHeight(66);
    dial->setMinimumWidth(66);
    
    //dial->setFocusPolicy(Qt::NoFocus);
    
    return dial;
}

QComboBox * SynthGUI::get_combobox(QString a_choices [], int a_count,  QWidget * a_parent)
{
    QComboBox * f_result = new QComboBox(a_parent);
    QStringList f_items;
    
    for(int i = 0; i < a_count; i++)
    {
        f_items << a_choices[i];
    }
    
    f_result->addItems(f_items);
    
    return f_result;
}

/*GUI Step 5:  Implement the event handlers from step 2.*/
void SynthGUI::setAttack(float sec)
{
    m_suppressHostUpdate = true;
    m_attack->setValue(int(sec * 100));
    m_suppressHostUpdate = false;
}

void SynthGUI::setDecay(float sec)
{
    m_suppressHostUpdate = true;
    m_decay->setValue(int(sec * 100));
    m_suppressHostUpdate = false;
}

void SynthGUI::setSustain(float val)
{
    m_suppressHostUpdate = true;
    m_sustain->setValue(int(val));
    m_suppressHostUpdate = false;
}

void SynthGUI::setRelease(float sec)
{
    m_suppressHostUpdate = true;
    m_release->setValue(int(sec * 100));
    m_suppressHostUpdate = false;
}

void SynthGUI::setTimbre(float val)
{
    m_suppressHostUpdate = true;
    m_timbre->setValue(int(val));  // * 100));
    m_suppressHostUpdate = false;
}

void SynthGUI::setRes(float val)
{
    m_suppressHostUpdate = true;
    m_res->setValue(int(val));  // * 100));
    m_suppressHostUpdate = false;
}

void SynthGUI::setDist(float val)
{
    m_suppressHostUpdate = true;
    m_dist->setValue(int(val));  // * 100));
    m_suppressHostUpdate = false;
}

void SynthGUI::setFilterAttack (float sec)
{    
    m_suppressHostUpdate = true;
    m_filter_attack->setValue(int(sec * 100));
    m_suppressHostUpdate = false;
}

void SynthGUI::setFilterDecay  (float sec)
{
    m_suppressHostUpdate = true;
    m_filter_decay->setValue(int(sec * 100));
    m_suppressHostUpdate = false;
}

void SynthGUI::setFilterSustain(float val)
{
    m_suppressHostUpdate = true;
    m_filter_sustain->setValue(int(val * 100));
    m_suppressHostUpdate = false;
}

void SynthGUI::setFilterRelease(float sec)
{
    m_suppressHostUpdate = true;
    m_filter_release->setValue(int(sec * 100));
    m_suppressHostUpdate = false;
}

void SynthGUI::setNoiseAmp(float val)
{
    m_suppressHostUpdate = true;
    m_noise_amp->setValue(int(val));
    m_suppressHostUpdate = false;
}

void SynthGUI::setFilterEnvAmt(float val)
{
    m_suppressHostUpdate = true;
    m_filter_env_amt->setValue(int(val));
    m_suppressHostUpdate = false;
}

void SynthGUI::setDistWet(float val)
{
    m_suppressHostUpdate = true;
    m_dist_wet->setValue(int(val * 100));
    m_suppressHostUpdate = false;
}

void SynthGUI::setOsc1Type(float val)
{
    m_suppressHostUpdate = true;
    m_osc1_type->setCurrentIndex(int(val));
    m_suppressHostUpdate = false;
}

void SynthGUI::setOsc1Pitch(float val)
{
    m_suppressHostUpdate = true;
    m_osc1_pitch->setValue(int(val));
    m_suppressHostUpdate = false;
}

void SynthGUI::setOsc1Tune(float val)
{
    m_suppressHostUpdate = true;
    m_osc1_tune->setValue(int(val * 100));
    m_suppressHostUpdate = false;
}

void SynthGUI::setOsc1Volume(float val)
{
    m_suppressHostUpdate = true;
    m_osc1_volume->setValue(int(val));
    m_suppressHostUpdate = false;
}

void SynthGUI::setOsc2Type(float val)
{
    m_suppressHostUpdate = true;
    m_osc2_type->setCurrentIndex(int(val));
    m_suppressHostUpdate = false;
}

void SynthGUI::setOsc2Pitch(float val)
{
    m_suppressHostUpdate = true;
    m_osc2_pitch->setValue(int(val));
    m_suppressHostUpdate = false;
}

void SynthGUI::setOsc2Tune(float val)
{
    m_suppressHostUpdate = true;
    m_osc2_tune->setValue(int(val * 100));
    m_suppressHostUpdate = false;
}

void SynthGUI::setOsc2Volume(float val)
{
    m_suppressHostUpdate = true;
    m_osc2_volume->setValue(int(val));
    m_suppressHostUpdate = false;
}

void SynthGUI::setMasterVolume(float val)
{
    m_suppressHostUpdate = true;
    m_master_volume->setValue(int(val));
    m_suppressHostUpdate = false;
}

/*Begin new stuff*/

void SynthGUI::setMasterUnisonVoices(float val)
{
    m_suppressHostUpdate = true;
    m_master_unison_voices->setValue(int(val));
    m_suppressHostUpdate = false;
}

void SynthGUI::setMasterUnisonSpread(float val)
{
    m_suppressHostUpdate = true;
    m_master_unison_spread->setValue(int(val));
    m_suppressHostUpdate = false; 
}

void SynthGUI::setMasterGlide(float val)
{
    m_suppressHostUpdate = true;
    m_master_glide->setValue(int(val));
    m_suppressHostUpdate = false;
}

void SynthGUI::setMasterPitchbendAmt(float val)
{
    m_suppressHostUpdate = true;
    m_master_pitchbend_amt->setValue(int(val));
    m_suppressHostUpdate = false;
}

/*
void SynthGUI::setBank(int val)
{
    cerr << "setBank called with val: " << val << endl;
}
*/

void SynthGUI::setProgram(int val)
{
    cerr << "setProgram called with val: " << val << endl;
}


/*Standard handlers for the audio slots, these perform manipulations of knob values
 that are common in audio applications*/

void SynthGUI::changed_zero_to_x(int a_value, QLabel * a_label, int a_port)
{
    float val = float(a_value) * .01;
    a_label->setText(QString("%1").arg(val));
    
    if (!m_suppressHostUpdate) {
	lo_send(m_host, m_controlPath, "if", a_port, val);     
    }
}

void SynthGUI::changed_integer(int a_value, QLabel * a_label, int a_port)
{
    float val = float(a_value);
    a_label->setText(QString("%1").arg(val));
    
    if (!m_suppressHostUpdate) {
	lo_send(m_host, m_controlPath, "if", a_port, val);
    }
}

void SynthGUI::changed_seconds(int a_value, QLabel * a_label, int a_port)
{
    float sec = float(a_value) * .01;
    a_label->setText(QString("%1").arg(sec));
    
    if (!m_suppressHostUpdate) {
	lo_send(m_host, m_controlPath, "if", a_port, sec);
    }
}

void SynthGUI::changed_pitch(int a_value, QLabel * a_label, int a_port)
{
    /*We need to send midi note number to the synth, as it probably still needs to process it as
     midi_note number.  We use this to display hz to the user*/
    
    float f_value = float(a_value);
    float f_hz = f_pit_midi_note_to_hz(f_value);
    
    a_label->setText(QString("%1 hz").arg((int)f_hz));
    
    if (!m_suppressHostUpdate) {
	lo_send(m_host, m_controlPath, "if", a_port, f_value);     
    }    
}

void SynthGUI::changed_decibels(int a_value, QLabel * a_label, int a_port)
{
    /*Decibels is a reasonable way to display this to the user, so just use it as it is*/
    a_label->setText(QString("%1").arg(a_value));
    
    if (!m_suppressHostUpdate) {
	lo_send(m_host, m_controlPath, "if", a_port, float(a_value));
    }
}




/*GUI Step 7:  Implement the event handlers from step 3.*/

void SynthGUI::attackChanged(int value)
{
    changed_seconds(value,m_attackLabel,LTS_PORT_ATTACK);
}

void
SynthGUI::decayChanged(int value)
{
    changed_seconds(value,m_decayLabel,LTS_PORT_DECAY);
}

void SynthGUI::sustainChanged(int value)
{
    changed_decibels(value, m_sustainLabel, LTS_PORT_SUSTAIN);    
}

void SynthGUI::releaseChanged(int value)
{
    changed_seconds(value, m_releaseLabel, LTS_PORT_RELEASE);    
}

void SynthGUI::timbreChanged(int value)
{
    changed_pitch(value, m_timbreLabel, LTS_PORT_TIMBRE);    
}

void SynthGUI::resChanged(int value)
{
    changed_decibels(value, m_resLabel, LTS_PORT_RES);    
}

void SynthGUI::distChanged(int value)
{
    changed_integer(value, m_distLabel, LTS_PORT_DIST);
}


void SynthGUI::filterAttackChanged(int value)
{
    changed_seconds(value,m_filter_attackLabel,LTS_PORT_FILTER_ATTACK);
}

void
SynthGUI::filterDecayChanged(int value)
{
    changed_seconds(value,m_filter_decayLabel,LTS_PORT_FILTER_DECAY);
}

void SynthGUI::filterSustainChanged(int value)
{
    changed_zero_to_x(value, m_filter_sustainLabel, LTS_PORT_FILTER_SUSTAIN);    
}

void SynthGUI::filterReleaseChanged(int value)
{
    changed_seconds(value, m_filter_releaseLabel, LTS_PORT_FILTER_RELEASE);    
}

void SynthGUI::noiseAmpChanged(int value)
{
    changed_decibels(value, m_noise_ampLabel, LTS_PORT_NOISE_AMP);
}

void SynthGUI::filterEnvAmtChanged(int value)
{
    float val = float(value); // / 100.0;
    m_filter_env_amtLabel->setText(QString("%1").arg(val));

    if (!m_suppressHostUpdate) {
	lo_send(m_host, m_controlPath, "if", LTS_PORT_FILTER_ENV_AMT, val);
    }
}

void SynthGUI::distWetChanged(int value)
{
    //TODO:  make a "Changed no label" method
    if (!m_suppressHostUpdate) {
	lo_send(m_host, m_controlPath, "if", LTS_PORT_DIST_WET, (float(value) * .01));
    }
}

void SynthGUI::osc1TypeChanged(int value)
{
    if (!m_suppressHostUpdate) {
	lo_send(m_host, m_controlPath, "if", LTS_PORT_OSC1_TYPE, float(value));
    }
}

void SynthGUI::osc1PitchChanged(int value)
{
    changed_integer(value, m_osc1_pitchLabel, LTS_PORT_OSC1_PITCH);
}

void SynthGUI::osc1TuneChanged(int value)
{
    changed_zero_to_x(value, m_osc1_tuneLabel, LTS_PORT_OSC1_TUNE);
}

void SynthGUI::osc1VolumeChanged(int value)
{
    changed_decibels(value, m_osc1_volumeLabel, LTS_PORT_OSC1_VOLUME);
}

void SynthGUI::osc2TypeChanged(int value)
{
    if (!m_suppressHostUpdate) {
	lo_send(m_host, m_controlPath, "if", LTS_PORT_OSC2_TYPE, float(value));
    }
}

void SynthGUI::osc2PitchChanged(int value)
{
    changed_integer(value, m_osc2_pitchLabel, LTS_PORT_OSC2_PITCH);
}

void SynthGUI::osc2TuneChanged(int value)
{
    changed_zero_to_x(value, m_osc2_tuneLabel, LTS_PORT_OSC2_TUNE);
}

void SynthGUI::osc2VolumeChanged(int value)
{
    changed_decibels(value, m_osc2_volumeLabel, LTS_PORT_OSC2_VOLUME);
}

void SynthGUI::masterVolumeChanged(int value)
{
    changed_decibels(value, m_master_volumeLabel, LTS_PORT_MASTER_VOLUME);
}




void SynthGUI::masterUnisonVoicesChanged(int value)
{
    changed_integer(value, m_master_unison_voicesLabel, LTS_PORT_MASTER_UNISON_VOICES);
}


void SynthGUI::masterUnisonSpreadChanged(int value)
{    
    changed_zero_to_x(value, m_master_unison_spreadLabel, LTS_PORT_MASTER_UNISON_SPREAD);
}


void SynthGUI::masterGlideChanged(int value)
{
    changed_zero_to_x(value, m_master_glideLabel, LTS_PORT_MASTER_GLIDE);
}


void SynthGUI::masterPitchbendAmtChanged(int value)
{
    changed_integer(value, m_master_pitchbend_amtLabel, LTS_PORT_MASTER_PITCHBEND_AMT);
}

/*
void SynthGUI::bankChanged(int value)
{
    cerr << "Program change not yet implemented.  Bank# " << value << endl;
}
*/

void SynthGUI::programChanged(int value)
{    
    if(presets_tab_delimited[m_program->currentIndex()].compare("empty") != 0)
    {
        QStringList f_preset_values = presets_tab_delimited[m_program->currentIndex()].split("\t");
        //TODO:  change f_i back to zero when there is something at that index
        for(int f_i = 1; f_i < LTS_PORT_MAX; f_i++)
        {
            if(f_i > f_preset_values.count())
            {
                cerr << "programChanged:  f_i is greater than f_preset_values.count(), preset not fully loaded.\n";
                break;
            }
            
            int f_preset_value_int = f_preset_values.at(f_i).toInt();
            
            v_set_control(f_i, f_preset_value_int);
            v_control_changed(f_i, f_preset_value_int);
        }
    }    
}
    
void SynthGUI::programSaved()
{
    int f_compare_text = m_program->currentText().compare("empty");
    
    if(f_compare_text == 0)
    {
        QMessageBox * f_qmess = new QMessageBox(this);
        
        QString * f_compare_text_string = new QString();
        f_compare_text_string->setNum(f_compare_text);
        
        f_compare_text_string->append("  \"" + presets_tab_delimited[m_program->currentIndex()] + "\"\n" +
        "You must change the name of the preset before you can save it.");
        
        f_qmess->setText(*f_compare_text_string);
        f_qmess->show();
    }
    else
    {
        QString f_result = m_program->currentText();                
        
        //TODO:  change f_i back to zero when there is something at that index
        for(int f_i = 1; f_i < LTS_PORT_MAX; f_i++)
        {
            QString * f_number = new QString();
            f_number->setNum(i_get_control(f_i));
            f_result.append("\t");  
            f_result.append(f_number);
        }
                
        presets_tab_delimited[m_program->currentIndex()] = f_result;
        
        QString * f_result2 = new QString();
        
        for(int f_i = 0; f_i < 128; f_i++)
        {
            f_result2->append(presets_tab_delimited[f_i] + "\n");
        }
        
        QFile * f_preset_file = new QFile(QDir::homePath() + "/dssi/" + LMS_PLUGIN_NAME + "-presets.tsv");
        
        f_preset_file->open(QIODevice::WriteOnly);
        
        f_preset_file->write(f_result2->toUtf8());
        f_preset_file->flush();
        
        f_preset_file->close();
                
        m_program->setItemText(m_program->currentIndex(), m_program->currentText());
    }
}



void SynthGUI::v_set_control(int a_port, float a_value)
{
    /*GUI Step 8:  Add the controls you created to the control handler*/
    switch (a_port) {
    case LTS_PORT_ATTACK:
	cerr << "gui setting attack to " << a_value << endl;
	setAttack(a_value);
	break;

    case LTS_PORT_DECAY:
	cerr << "gui setting decay to " << a_value << endl;
	setDecay(a_value);
	break;

    case LTS_PORT_SUSTAIN:
	cerr << "gui setting sustain to " << a_value << endl;
	setSustain(a_value);
	break;

    case LTS_PORT_RELEASE:
	cerr << "gui setting release to " << a_value << endl;
	setRelease(a_value);
	break;

    case LTS_PORT_TIMBRE:
	cerr << "gui setting timbre to " << a_value << endl;
	setTimbre(a_value);
	break;

    case LTS_PORT_RES:
	cerr << "gui setting res to " << a_value << endl;
	setRes(a_value);
	break;
        
    case LTS_PORT_DIST:
	cerr << "gui setting res to " << a_value << endl;
	setDist(a_value);
	break;

    case LTS_PORT_FILTER_ATTACK:
	cerr << "gui setting attack to " << a_value << endl;
	setFilterAttack(a_value);
	break;

    case LTS_PORT_FILTER_DECAY:
	cerr << "gui setting decay to " << a_value << endl;
	setFilterDecay(a_value);
	break;

    case LTS_PORT_FILTER_SUSTAIN:
	cerr << "gui setting sustain to " << a_value << endl;
	setFilterSustain(a_value);
	break;

    case LTS_PORT_FILTER_RELEASE:
	cerr << "gui setting release to " << a_value << endl;
	setFilterRelease(a_value);
	break;

    case LTS_PORT_NOISE_AMP:
        cerr << "gui setting noise amp to " << a_value << endl;
        setNoiseAmp(a_value);
        break;
    
    case LTS_PORT_DIST_WET:
        cerr << "gui setting dist wet to " << a_value << endl;
        setDistWet(a_value);
        break;
            
    case LTS_PORT_FILTER_ENV_AMT:
        cerr << "gui setting filter env amt to " << a_value << endl;
        setFilterEnvAmt(a_value);
        break;
    
    case LTS_PORT_OSC1_TYPE:
        cerr << "gui setting osc1type to " << a_value << endl;
        setOsc1Type(a_value);
        break;
            
    case LTS_PORT_OSC1_PITCH:
        cerr << "gui setting osc1pitch to " << a_value << endl;
        setOsc1Pitch(a_value);
        break;
    
    case LTS_PORT_OSC1_TUNE:
        cerr << "gui setting osc1tune to " << a_value << endl;
        setOsc1Tune(a_value);
        break;
    
    case LTS_PORT_OSC1_VOLUME:
        cerr << "gui setting osc1vol amp to " << a_value << endl;
        setOsc1Volume(a_value);
        break;
        
    case LTS_PORT_OSC2_TYPE:
        cerr << "gui setting osc2type to " << a_value << endl;
        setOsc2Type(a_value);
        break;
            
    case LTS_PORT_OSC2_PITCH:
        cerr << "gui setting osc2pitch to " << a_value << endl;
        setOsc2Pitch(a_value);
        break;
    
    case LTS_PORT_OSC2_TUNE:
        cerr << "gui setting osc2tune to " << a_value << endl;
        setOsc2Tune(a_value);
        break;
    
    case LTS_PORT_OSC2_VOLUME:
        cerr << "gui setting osc2vol amp to " << a_value << endl;
        setOsc2Volume(a_value);
        break;
        
    case LTS_PORT_MASTER_VOLUME:
        cerr << "gui setting noise amp to " << a_value << endl;
        setMasterVolume(a_value);
        break;
    
    case LTS_PORT_MASTER_UNISON_VOICES:
        cerr << "gui setting unison voices to " << a_value << endl;
        setMasterUnisonVoices(a_value);
        break;

    case LTS_PORT_MASTER_UNISON_SPREAD:
        cerr << "gui setting unison spread to " << a_value << endl;
        setMasterUnisonSpread(a_value);
        break;

    case LTS_PORT_MASTER_GLIDE:
        cerr << "gui setting glide to " << a_value << endl;
        setMasterGlide(a_value);
        break;

    case LTS_PORT_MASTER_PITCHBEND_AMT:
        cerr << "gui setting pitchbend to " << a_value << endl;
        setMasterPitchbendAmt(a_value);
        break;
                
    default:
	cerr << "Warning: received request to set nonexistent port " << a_port << endl;
    }
}

void SynthGUI::v_control_changed(int a_port, int a_value)
{
       /*GUI Step 8.25:  Add the controls you created to the control handler*/
    switch (a_port) {
    case LTS_PORT_ATTACK:
	attackChanged(a_value);
	break;
    case LTS_PORT_DECAY:	
	decayChanged(a_value);
	break;
    case LTS_PORT_SUSTAIN:
	sustainChanged(a_value);
	break;
    case LTS_PORT_RELEASE:
	releaseChanged(a_value);
	break;
    case LTS_PORT_TIMBRE:
	timbreChanged(a_value);
	break;
    case LTS_PORT_RES:
	resChanged(a_value);
	break;        
    case LTS_PORT_DIST:
	distChanged(a_value);
	break;
    case LTS_PORT_FILTER_ATTACK:
	filterAttackChanged(a_value);
	break;
    case LTS_PORT_FILTER_DECAY:
	filterDecayChanged(a_value);
	break;
    case LTS_PORT_FILTER_SUSTAIN:
	filterSustainChanged(a_value);
	break;
    case LTS_PORT_FILTER_RELEASE:
	filterReleaseChanged(a_value);
	break;
    case LTS_PORT_NOISE_AMP:
        noiseAmpChanged(a_value);
        break;    
    case LTS_PORT_DIST_WET:
        setDistWet(a_value);
        break;
    case LTS_PORT_FILTER_ENV_AMT:
        filterEnvAmtChanged(a_value);
        break;    
    case LTS_PORT_OSC1_TYPE:
        osc1TypeChanged(a_value);
        break;            
    case LTS_PORT_OSC1_PITCH:
        osc1PitchChanged(a_value);
        break;    
    case LTS_PORT_OSC1_TUNE:
        osc1TuneChanged(a_value);
        break;    
    case LTS_PORT_OSC1_VOLUME:
        osc1VolumeChanged(a_value);
        break;
    case LTS_PORT_OSC2_TYPE:
        osc2TypeChanged(a_value);
        break;            
    case LTS_PORT_OSC2_PITCH:
        osc2PitchChanged(a_value);
        break;    
    case LTS_PORT_OSC2_TUNE:
        osc2TuneChanged(a_value);
        break;    
    case LTS_PORT_OSC2_VOLUME:
        osc2VolumeChanged(a_value);
        break;
    case LTS_PORT_MASTER_VOLUME:
        masterVolumeChanged(a_value);
        break;
    case LTS_PORT_MASTER_UNISON_VOICES:
        masterUnisonVoicesChanged(a_value);
        break;
    case LTS_PORT_MASTER_UNISON_SPREAD:
        masterUnisonSpreadChanged(a_value);
        break;
    case LTS_PORT_MASTER_GLIDE:
        masterGlideChanged(a_value);
        break;
    case LTS_PORT_MASTER_PITCHBEND_AMT:
        masterPitchbendAmtChanged(a_value);
        break;
    default:
	cerr << "Warning: received request to set nonexistent port " << a_port << endl;
    }
}

/*TODO:  For the forseeable future, this will only be used for getting the values to write back to 
 the presets.tsv file;  It should probably return a string that can be re-interpreted into other values for
 complex controls that could have multiple ints, or string values, etc...*/
int SynthGUI::i_get_control(int a_port)
{
        /*GUI Step 8.5:  Add the controls you created to the control handler
         TODO:  Renumber the GUI steps*/
    switch (a_port) {
    case LTS_PORT_ATTACK:
        return m_attack->value();
    case LTS_PORT_DECAY:
        return m_decay->value();
    case LTS_PORT_SUSTAIN:
        return m_sustain->value();
    case LTS_PORT_RELEASE:
        return m_release->value();
    case LTS_PORT_TIMBRE:
        return m_timbre->value();
    case LTS_PORT_RES:
        return m_res->value();        
    case LTS_PORT_DIST:
        return m_dist->value();
    case LTS_PORT_FILTER_ATTACK:
        return m_filter_attack->value();
    case LTS_PORT_FILTER_DECAY:
        return m_filter_decay->value();
    case LTS_PORT_FILTER_SUSTAIN:
        return m_filter_sustain->value();
    case LTS_PORT_FILTER_RELEASE:
        return m_filter_release->value();
    case LTS_PORT_NOISE_AMP:
        return m_noise_amp->value();
    case LTS_PORT_DIST_WET:
        return m_dist_wet->value();
    case LTS_PORT_FILTER_ENV_AMT:
        return m_filter_env_amt->value();
    case LTS_PORT_OSC1_TYPE:
        return m_osc1_type->currentIndex();
    case LTS_PORT_OSC1_PITCH:
        return m_osc1_pitch->value();
    case LTS_PORT_OSC1_TUNE:
        return m_osc1_tune->value();
    case LTS_PORT_OSC1_VOLUME:
        return m_osc1_volume->value();
    case LTS_PORT_OSC2_TYPE:
        return m_osc2_type->currentIndex();
    case LTS_PORT_OSC2_PITCH:
        return m_osc2_pitch->value();
    case LTS_PORT_OSC2_TUNE:
        return m_osc2_tune->value();
    case LTS_PORT_OSC2_VOLUME:
        return m_osc2_volume->value();
    case LTS_PORT_MASTER_VOLUME:
        return m_master_volume->value();
    case LTS_PORT_MASTER_UNISON_VOICES:
        return m_master_unison_voices->value();
    case LTS_PORT_MASTER_UNISON_SPREAD:
        return m_master_unison_spread->value();
    case LTS_PORT_MASTER_GLIDE:
        return m_master_glide->value();
    case LTS_PORT_MASTER_PITCHBEND_AMT:
        return m_master_pitchbend_amt->value();
    default:
	cerr << "Warning: received request to get nonexistent port " << a_port << endl;
    }
}



void SynthGUI::test_press()
{
    //unsigned char noteon[4] = { 0x00, 0x90, 0x3C, 0x40 };
    unsigned char noteon[4] = { 0x00, 0x90, 0x2A, 0x66 };
    lo_send(m_host, m_midiPath, "m", noteon);
}

void SynthGUI::oscRecv()
{
    if (osc_server) {
	lo_server_recv_noblock(osc_server, 1);
    }
}

void SynthGUI::test_release()
{
    unsigned char noteoff[4] = { 0x00, 0x90, 0x2A, 0x00 };

    lo_send(m_host, m_midiPath, "m", noteoff);
}

void SynthGUI::aboutToQuit()
{
    if (!m_hostRequestedQuit) lo_send(m_host, m_exitingPath, "");
}

SynthGUI::~SynthGUI()
{
    lo_address_free(m_host);
}


void osc_error(int num, const char *msg, const char *path)
{
#ifdef LMS_DEBUG_MODE_QT
    cerr << "Error: liblo server error " << num
	 << " in path \"" << (path ? path : "(null)")
	 << "\": " << msg << endl;
#endif
}

int debug_handler(const char *path, const char *types, lo_arg **argv,
	      int argc, void *data, void *user_data)
{
    int i;
#ifdef LMS_DEBUG_MODE_QT
      cerr << "Warning: unhandled OSC message in GUI:" << endl;
#endif
    

    for (i = 0; i < argc; ++i) {
#ifdef LMS_DEBUG_MODE_QT
	cerr << "arg " << i << ": type '" << types[i] << "': ";
#endif
        lo_arg_pp((lo_type)types[i], argv[i]);
#ifdef LMS_DEBUG_MODE_QT
	cerr << endl;
#endif
    }
#ifdef LMS_DEBUG_MODE_QT
    cerr << "(path is <" << path << ">)" << endl;
#endif
    return 1;
}

int program_handler(const char *path, const char *types, lo_arg **argv,
	       int argc, void *data, void *user_data)
{
    SynthGUI *gui = static_cast<SynthGUI *>(user_data);
    
    int bank = 0;
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
    
    cerr << "Bank:  " << bank << ", Program:  " << program << endl;

    gui->setProgram(program);

    return 0;
}

int configure_handler(const char *path, const char *types, lo_arg **argv,
		  int argc, void *data, void *user_data)
{
    return 0;
}

int rate_handler(const char *path, const char *types, lo_arg **argv,
	     int argc, void *data, void *user_data)
{
    return 0; /* ignore it */
}

int show_handler(const char *path, const char *types, lo_arg **argv,
	     int argc, void *data, void *user_data)
{
    SynthGUI *gui = static_cast<SynthGUI *>(user_data);
    while (!gui->ready()) sleep(1);
    if (gui->isVisible()) gui->raise();
    else gui->show();
    return 0;
}

int hide_handler(const char *path, const char *types, lo_arg **argv,
	     int argc, void *data, void *user_data)
{
    SynthGUI *gui = static_cast<SynthGUI *>(user_data);
    gui->hide();
    return 0;
}

int quit_handler(const char *path, const char *types, lo_arg **argv,
	     int argc, void *data, void *user_data)
{
    SynthGUI *gui = static_cast<SynthGUI *>(user_data);
    gui->setHostRequestedQuit(true);
    qApp->quit();
    return 0;
}

int control_handler(const char *path, const char *types, lo_arg **argv,
		int argc, void *data, void *user_data)
{
    SynthGUI *gui = static_cast<SynthGUI *>(user_data);

    if (argc < 2) {
	cerr << "Error: too few arguments to control_handler" << endl;
	return 1;
    }

    const int port = argv[0]->i;
    const float value = argv[1]->f;

    gui->v_set_control(port, value);    

    return 0;
}

int main(int argc, char **argv)
{
    cerr << "synth_qt_gui starting..." << endl;

    QApplication application(argc, argv);

    if (application.argc() != 5) {
	cerr << "usage: "
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

    SynthGUI gui(host, port,
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

    osc_server = lo_server_new(NULL, osc_error);
    lo_server_add_method(osc_server, myControlPath, "if", control_handler, &gui);
    lo_server_add_method(osc_server, myProgramPath, "ii", program_handler, &gui);
    lo_server_add_method(osc_server, myConfigurePath, "ss", configure_handler, &gui);
    lo_server_add_method(osc_server, myRatePath, "i", rate_handler, &gui);
    lo_server_add_method(osc_server, myShowPath, "", show_handler, &gui);
    lo_server_add_method(osc_server, myHidePath, "", hide_handler, &gui);
    lo_server_add_method(osc_server, myQuitPath, "", quit_handler, &gui);
    lo_server_add_method(osc_server, NULL, NULL, debug_handler, &gui);

    lo_address hostaddr = lo_address_new(host, port);
    lo_send(hostaddr,
	    QByteArray(path) + "/update",
	    "s",
	    (QByteArray(lo_server_get_url(osc_server))+QByteArray(path+1)).data());

    QObject::connect(&application, SIGNAL(aboutToQuit()), &gui, SLOT(aboutToQuit()));

    gui.setReady(true);
    return application.exec();
}

