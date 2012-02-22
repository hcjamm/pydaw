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
#include "../libmodsynth/lib/amp.h"
#include "../libmodsynth/lib/pitch_core.h"

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
//#define LMS_DEBUG_MODE_QT

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
#define LTS_PORT_PITCH_ENV_TIME 28
#define LTS_PORT_PITCH_ENV_AMT 29
#define LTS_PORT_PROGRAM_CHANGE 30  //This must be last
//#define LTS_PORT_MAX 31  


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
    
    /*Set the CSS style that will "cascade" on the other controls.  Other control's styles can be overridden by running their own setStyleSheet method*/
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
        f_file->open(QIODevice::ReadWrite | QIODevice::Text);
                  
        /*
            cerr << "Failed to open preset file:  " << f_preset_path << "/" << LMS_PLUGIN_NAME << "-presets.tsv" << "\n";
        
            for(int i = 0; i < 128; i++) 
            {
                f_programs_list[i] = "empty";
                presets_tab_delimited[i] = "empty";
            }
         */
        
        /*This string is generated by running presets_to_qstring.pl script in the packaging folder on the .tsv file associated with this plugin
         in ~/dssi .  If modifying this plugin by changing the number of parameters to be saved by presets, you should comment this out
         and uncomment the section above it.*/

        f_file->write("classic 5th pad\t100\t100\t-7\t120\t92\t-16\t15\t100\t100\t1\t162\t-12\t24\t1\t0\t0\t0\t0\t0\t7\t0\t0\t-16\t4\t42\t1\t18\t1\t0\n303 acid lead\t21\t26\t-9\t45\t70\t0\t36\t12\t29\t1\t99\t-30\t36\t100\t0\t0\t0\t0\t4\t0\t0\t0\t-8\t1\t1\t1\t18\t1\t0\nhoover\t21\t26\t-9\t45\t124\t-16\t15\t12\t29\t1\t99\t-12\t0\t1\t0\t0\t0\t0\t4\t0\t0\t0\t-8\t4\t42\t1\t18\t32\t-12\nbendy saw\t1\t49\t-3\t16\t124\t-16\t15\t100\t100\t1\t162\t-60\t0\t1\t0\t12\t0\t0\t4\t0\t0\t0\t-16\t1\t42\t54\t36\t1\t0\nsupersaw lead\t3\t49\t-3\t16\t124\t-15\t36\t1\t33\t1\t162\t-12\t0\t1\t0\t12\t0\t-6\t4\t0\t0\t0\t-16\t5\t41\t1\t17\t1\t0\n3rd Plucks\t3\t49\t-20\t195\t90\t-9\t36\t1\t9\t1\t73\t-12\t36\t1\t0\t0\t0\t-6\t0\t5\t0\t0\t-16\t5\t50\t1\t17\t1\t0\nsquare lead\t3\t49\t-12\t60\t124\t-9\t36\t1\t21\t1\t73\t-12\t36\t1\t1\t0\t0\t-6\t4\t0\t0\t0\t-16\t4\t50\t1\t17\t1\t0\ntriangle kick drum\t3\t49\t-12\t60\t124\t-9\t36\t1\t21\t1\t73\t-37\t36\t1\t2\t0\t0\t-6\t4\t0\t0\t0\t-5\t4\t50\t1\t17\t17\t-24\nnoise snare\t3\t16\t-30\t14\t124\t-16\t36\t1\t10\t1\t73\t-3\t0\t1\t4\t0\t0\t-6\t4\t0\t0\t0\t-5\t4\t50\t1\t17\t17\t-24\nelectro open hihat\t16\t23\t-30\t14\t101\t-3\t36\t18\t10\t1\t73\t-3\t36\t100\t4\t0\t0\t-6\t4\t0\t0\t0\t-18\t4\t50\t1\t17\t17\t-24\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\n");

        f_file->flush();
        
        f_file->seek(0);         
    }
        
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
    
    /*v_add_knob_to_layout(m_osc1_pitch, minus12_to_12, 0, m_osc1_pitchLabel, f_gb_osc1_layout, QString("Pitch"), 
            f_gb_layout_column, f_gb_layout_row, SIGNAL(valueChanged(int)), SLOT(osc1PitchChanged(int)));*/
    
    f_gb_layout_column++;
    
    m_osc1_tune = get_knob(minus1_to_1);
    m_osc1_tuneLabel = newQLabel(this);
    add_widget(f_gb_osc1_layout, f_gb_layout_column, f_gb_layout_row, "Tune", m_osc1_tune, m_osc1_tuneLabel);
    connect(m_osc1_tune, SIGNAL(valueChanged(int)), this, SLOT(osc1TuneChanged(int)));
    osc1TuneChanged(m_osc1_tune->value());
    
    f_gb_layout_column++;
    
    m_osc1_volume = get_knob(decibels_30_to_0, -6);
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
    attackChanged  (m_attack->value());
    
    f_gb_layout_column++;
        
    m_decay   =  get_knob(zero_to_one); //newQDial(  1, 100,  1,  25); // s * 100
    m_decayLabel   = newQLabel(this);
    add_widget(f_gb_adsr_layout, f_gb_layout_column, f_gb_layout_row, "Decay",m_decay, m_decayLabel);
    connect(m_decay,   SIGNAL(valueChanged(int)), this, SLOT(decayChanged(int)));
    decayChanged  (m_decay->value());
    
    f_gb_layout_column++;
    
    m_sustain =  get_knob(decibels_30_to_0); // newQDial(  0, 100,  1,  75); // %
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
    distChanged (m_dist->value());
    
    f_gb_layout_column++;
    
    m_dist_wet  = get_knob(zero_to_one);    
    add_widget_no_label(f_gb_dist_layout, f_gb_layout_column, f_gb_layout_row, "Wet", m_dist_wet);
    connect(m_dist_wet,  SIGNAL(valueChanged(int)), this, SLOT(distWetChanged(int)));
    distWetChanged (m_dist_wet->value());
    
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
    //filterAttackChanged  (m_filter_attack  ->value());
    
    f_gb_layout_column++;
        
    m_filter_decay   =  get_knob(zero_to_one); //newQDial(  1, 100,  1,  25); // s * 100
    m_filter_decayLabel   = newQLabel(this);
    add_widget(f_gb_adsr_f_layout, f_gb_layout_column, f_gb_layout_row, "Decay",m_filter_decay, m_filter_decayLabel);
    connect(m_filter_decay,   SIGNAL(valueChanged(int)), this, SLOT(filterDecayChanged(int)));
    //filterDecayChanged  (m_filter_decay ->value());
    
    f_gb_layout_column++;
    
    m_filter_sustain =  get_knob(zero_to_one); // newQDial(  0, 100,  1,  75); // %
    m_filter_sustainLabel = newQLabel(this);
    add_widget(f_gb_adsr_f_layout, f_gb_layout_column, f_gb_layout_row, "Sustain", m_filter_sustain, m_filter_sustainLabel);
    connect(m_filter_sustain, SIGNAL(valueChanged(int)), this, SLOT(filterSustainChanged(int)));
    //filterSustainChanged(m_filter_sustain->value());
    
    f_gb_layout_column++;
    
    m_filter_release = get_knob(zero_to_four); //newQDial(  1, 400, 10, 200); // s * 100
    m_filter_releaseLabel = newQLabel(this);
    add_widget(f_gb_adsr_f_layout, f_gb_layout_column, f_gb_layout_row, "Release", m_filter_release, m_filter_releaseLabel);
    connect(m_filter_release, SIGNAL(valueChanged(int)), this, SLOT(filterReleaseChanged(int)));    
    //filterReleaseChanged(m_filter_release->value());
        
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
    //timbreChanged (m_timbre ->value());
    
    f_gb_layout_column++;
    
    m_res  =  get_knob(decibels_30_to_0); 
    m_resLabel  = newQLabel(this);
    add_widget(_gb_filter_layout, f_gb_layout_column, f_gb_layout_row, "Res", m_res, m_resLabel);
    connect(m_res,  SIGNAL(valueChanged(int)), this, SLOT(resChanged(int)));
    //resChanged (m_res ->value());
    
    f_gb_layout_column++;
    
    m_filter_env_amt  =  get_knob(minus36_to_36); 
    m_filter_env_amtLabel  = newQLabel(this);
    add_widget(_gb_filter_layout, f_gb_layout_column, f_gb_layout_row, "Env", m_filter_env_amt, m_filter_env_amtLabel);
    connect(m_filter_env_amt,  SIGNAL(valueChanged(int)), this, SLOT(filterEnvAmtChanged(int)));
    //filterEnvAmtChanged (m_filter_env_amt ->value());
    
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
    //masterVolumeChanged (m_master_volume ->value());
    
    f_gb_layout_column++;
    
    m_master_unison_voices  =  newQDial(1, 7, 1, 1);
    m_master_unison_voicesLabel  = newQLabel(this);
    add_widget(f_gb_master_vol_layout, f_gb_layout_column, f_gb_layout_row, "Unison",m_master_unison_voices, m_master_unison_voicesLabel);
    connect(m_master_unison_voices,  SIGNAL(valueChanged(int)), this, SLOT( masterUnisonVoicesChanged(int)));
    //masterUnisonVoicesChanged (m_master_unison_voices ->value());
    
    f_gb_layout_column++;
    
    m_master_unison_spread  =  get_knob(zero_to_one);
    m_master_unison_spreadLabel  = newQLabel(this);
    add_widget(f_gb_master_vol_layout, f_gb_layout_column, f_gb_layout_row, "Spread",m_master_unison_spread, m_master_unison_spreadLabel);
    connect(m_master_unison_spread,  SIGNAL(valueChanged(int)), this, SLOT(masterUnisonSpreadChanged(int)));
    //masterUnisonSpreadChanged (m_master_unison_spread ->value());
    
    f_gb_layout_column++;
    
    m_master_glide  =  get_knob(zero_to_one);
    m_master_glideLabel  = newQLabel(this);
    add_widget(f_gb_master_vol_layout, f_gb_layout_column, f_gb_layout_row, "Glide",m_master_glide, m_master_glideLabel);
    connect(m_master_glide,  SIGNAL(valueChanged(int)), this, SLOT(masterGlideChanged(int)));
    //masterGlideChanged (m_master_glide ->value());
    
    f_gb_layout_column++;
    
    m_master_pitchbend_amt  =  newQDial(1, 36, 1, 2);
    m_master_pitchbend_amtLabel  = newQLabel(this);
    add_widget(f_gb_master_vol_layout, f_gb_layout_column, f_gb_layout_row, "Pitchbend",m_master_pitchbend_amt, m_master_pitchbend_amtLabel);
    connect(m_master_pitchbend_amt,  SIGNAL(valueChanged(int)), this, SLOT(masterPitchbendAmtChanged(int)));
    //masterPitchbendAmtChanged (m_master_pitchbend_amt ->value());
    
    f_gb_layout_column++;
    
    
    layout_row3->addWidget(f_gb_master_vol, -1, Qt::AlignLeft);
    f_column++;
    f_gb_layout_row = 0;
    f_gb_layout_column = 0;
    
    
    /*The Master Volume GroupBox*/
    QGroupBox * f_gb_pitch_env = newGroupBox("Pitch Envelope", this); 
    QGridLayout *f_gb_pitch_env_layout = new QGridLayout(f_gb_pitch_env);
    
    
    m_pitch_env_amt = get_knob(minus36_to_36, 0);
    m_pitch_env_amtLabel = newQLabel(this);
    add_widget(f_gb_pitch_env_layout, f_gb_layout_column, f_gb_layout_row, "Amount", m_pitch_env_amt, m_pitch_env_amtLabel);
    connect(m_pitch_env_amt, SIGNAL(valueChanged(int)), this, SLOT(pitchEnvAmtChanged(int)));
    //pitchEnvAmtChanged(m_pitch_env_amt->value());
    
    f_gb_layout_column++;
    
    m_pitch_env_time = get_knob(zero_to_two, 0);
    m_pitch_env_timeLabel = newQLabel(this);
    add_widget(f_gb_pitch_env_layout, f_gb_layout_column, f_gb_layout_row, "Time", m_pitch_env_time, m_pitch_env_timeLabel);
    connect(m_pitch_env_time, SIGNAL(valueChanged(int)), this, SLOT(pitchEnvTimeChanged(int)));
   //pitchEnvTimeChanged(m_pitch_env_time->value());
    
    layout_row3->addWidget(f_gb_pitch_env, -1, Qt::AlignLeft);
    f_column++;
    f_gb_layout_row = 0;
    f_gb_layout_column = 0;
    
    QLabel * f_logo_label = new QLabel("", this);
    //f_logo_label->setPixmap(QPixmap(QString::fromUtf8("/usr/local/lib/dssi/synth/ray-v-logo.png")));
    f_logo_label->setTextFormat(Qt::RichText);
    /*This string is a base64 encoded .png image I created using Gimp for the logo.  To get the base64 encoded string,
     run it through png_to_base64.pl in the packaging folder*/
    QString f_logo_text = "<html><img src=\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAASwAAAB4CAYAAABIFc8gAAAAAXNSR0IArs4c6QAAAAZiS0dEAP8A/wD/oL2nkwAAAAlwSFlzAAALEwAACxMBAJqcGAAAAAd0SU1FB9wBGRcsNc+yXSkAAAAZdEVYdENvbW1lbnQAQ3JlYXRlZCB3aXRoIEdJTVBXgQ4XAAAgAElEQVR42uy9eZxkZXn2/73Pqaqurq5eZl9hGGBgDCCgIioqKJKoiLYGxKgEkaBi1ER9XxOXKL+4kFdjXuMao+IWScQlg+KSEBVwCy6AoKzDMDB7z/T0Vr1UneV+/zjnVJ8+fdbqRvNzuj5zptauOsvzXM91X/cmZ9sYzL1JynNJeS3pedJr4ZvmeF1zvqcZ35v0W9xcSn5v6RZ/O9tOvbaywOdFblrw9bz7/GjsW9HnS2M0uAgRwMoLTlnAJTkGhXZ4gbVDwNJFGMyptyNtMKWAVRFgkkcJFBZyfTtZdBcylgsB15EMWgFg5QGlvPedDsIiIJXnPS04EH4ng+B/8uDLYE9FgWohjxeLuSzGcXSyb5pzXC+BVg7AMnMAUd7XFmPFLAJG4fv/XwPX7woEOwClxQCqPOw97fu1Q4AoeixZY7nIfhVZbHMdw5EIWgFgZQGSZLyXBGJFQSvrYuZ5nAVgna7E+vsAYIsEUEXBaqEsvdMxs1iAtdj71ckCuwRaPmCVUgBLUl7LA1xZA1pzrkZZYJX2PPo3SYNjsYDp930ASYGJvhCGvljAkPe6dKLV5tmvImO5o2M4kkCrlINJFd06HXxZFzjuedZ7YXE/r9AvCwQi+T0FOFngZC/C2rNAIgmkJOE+/B2akyF2wgLzsipJWSyzjuGI9hKWCoKTsUDgKgpYWSCVZ8sLVpIBIvIogc5immi/zf0oMtGLMPci46aIphl3nbMAt9MFOM8YDo9LXQhonW0jRwrLKhUAJyPj9SxTMelCRy9WJyDlLhJwdQoM8lsGl98G+BX5jSywKsrc84JDUX0zD0vsVOrIWnjTxmXWQqpZY/dIAa1SBlAZOe+TgCvrQmvK4As/dzNASgoAmMQ8LgJkyuKYfY9GkOT/RM1qIVunwJAGXGnHmdd8zbtfeRbaLHkiaewcsWEN1QygMnI8z8u2WOBFdmMeJ92nAVeW4FkEGPS3ADL/UwZnXo9g3IQ3OpQY0rSiNHNLc17jOLDt1GQtMo7dnItslok75/b7zrJKKcwp7jUzAlpGBuuSHFQ6y95PAqnoYyNy70bYlNsB05IcgJF3tVvIqigLAC59lDSrIqZgHvYe95k8C11ep0xehlXEM55nn4LNifyWEWFZRgLjirKtI1p4LyUMmKTNzHg/aRDGmYZpgJW0+sQBlRsDWhK6sG7kQrsFVmDNqb1JTqCQHECyUFN0IfqaFHw/j8BuxIytLMkhbbHTHOCQ5Ukmgxku1FSNG7vh8Rc3Ft0U0CJlQdUjDbCygMoMAZWZAFxmipmYdqGzqH0cOCWBVdIWZVmSMZDz5h8WAbHFAK+FsKzfNmAlMasoWJkpjD4JtPKaXm6Oa5xmDmaZrGljWWNAKMyQJAJM0c9lLUaJQPX7Lr6XEsDKjHlshhiZmQJeWUwrbvJFL7YbY+8nbU5O4NIEsMpagYsEl2oBZqQ5gKIToOsE2KSD97PMwSxN1MihiUpOZu4mAEXWNS5iwgbPs6yFOK0qKlPE3dzQb7gZ1/eIZVmllMFkhkDKjAGpuMdZoJVXqHRTGJYTeWzGvOZmmIx5AEsLAFLWa51E1GsOIJMOQHAhAFaUYSVpoiadyQpZWlGW4yWPSZhHZ8tifXHjNu85djPMziOaZZVSVsAoSEmEaZVymIuLAVhJjCoMVsG94/+ek9NELAJYnQBTXq9jkVI7ad8tBb9bFgG4JIdJlaWLmhmglTRusha5LNMwbt/zhPAkmYSaYhmQA4CNyHVMi84/IllWHGBFmZURAahSAvOKA68iF5qCYOWGQMoJ/a4TYlJOinmYtvouFmjpIoJXp+CYh2lJwffyApaQ7bwppbCuqBmWF7DSQKsIYGWZqkn7EzduA93KjuxDVGyPzhNNWeSPSNE9elHCICURcOpKYFmlFF0rjmVRcPClAZUTA1zREAcnQbg3FkF8LwJUnRQUXGgplU4YVicmYfixGZnY4YWxlMOpYySARJb5leSg0RRdKPheIwFkFwpYTuRzZuj3rJixKsz3dgNL1XCjDCtp4JRCW/C8EmFipQSmledC56HTUZAKVionB3hFRc8klkVB8b3TqqidmI2LAYadald5zcI0dpVkHpZD40VyjJ00gNAUNh1nFpKiu6UBlhHDgEgZuxI6Njv0u2GrIMq44sy/Qmbg76uOVSI7jMEMDaxyhFWVI4BmZmgSeVzUaewqvIV1Kzvy2EhgWkmgleYpXAhQ6QJBZjGqVhYNHJUOwCzJtEoS2uNYWBxTT1rs8koIcYtTkkmYpbdJBuNLYlfB39j+8YX3wYwZf3GxhHGm4REZUFrKEEIDUCqH2FUYwCoxgFXK0LLISe/dFNCKApQZes1IAbeigLWYYFUUWLTg7xTV3mQBr2UBlpHiyAmPEyNlzMSBVl7NKI1ldQpYaZpa3L44MWagHXpsRoR2N4WpHvER7nEmYZx3ME54r0bYVjkBtIyUlTKJTietUm4CAIXByowBrzTQSsrf6pT5LPS+U0aV12EA6aESnXawyfISpnkEkzzQZsbYyQNYadc6ziTMA1hGCtsL/44T+q5WZLyHr0/UCoimji2BVg7RPQm0yhEw6ooxEdNMw049LHHmYCkETk5O0EoS3xfLJOy0smQR8FqsWvZ5AKqTlJ68YBUdI5UYlmWmMJskVh6+vmYKo84LWGYBhhXVrYJbJQRaZsThE5UusrTe6PEfsak5SRcoKqyXY3QtMyTER0EruooWFVDTvIJmBKDMkH4VZxp2ClhZLKvTss6dmnpFgXGxAStvtYMoAJQiwroZM1bysKy0Rc4kOYA4qT4aOZlVlnkaDVA2/PEYaFfhz5mhz4XPl5vBWLOqpx4xgCUJ7Co6eEoRcDL3fIQztr9JzvPOouL9mz23/iP/xIq0XxPvJcNApQfX6KJl1rBL/bSqxzNafywj/U/mUP/ZDEtpHluyI/toR/Y9jmVlmQmLbQ4WbZpR9L4oIOb1DHZSS11SAEAiQntaHF+bzf9kPf/LGmKtP1hAFFFBUfqfzB2n/ZBtGU6aOAkgDrCyWGGsOXjHM7lg7BbOCM6FIKL+1677M75/wj+xzT+euJShOI3PpfO69kecSZgmvEfNvTla1fY3ch7QEBQkGAmqIogqeA+88a0CqKoiEgw+1wHGwQGxBXMGqTXupH7o63o0gmkuY2blc9i1/rXc33cmIzEmYBJw2RFPYhHRPW2yF+3skxdkFgJUCwGsOJaUB6TSvitpEUxaFMMs3QTM5edy+9C/8SyUJiiqs7s1fhtb7UPUSitpxbAbMwJaeU3CLHOwDVjOOOWJn3GSQCMY3yr+Mq3S5UzoTERcd0PfGWVhRapCZJmAv/cmYlJqTtKACnsNy0DFO52KKhOoSHudUUFQH58Q1eCaCsFf+KuS+tRLVH3IE0wUQxTTHcEY+pJsPvAl3dL/dPZs/Qw/q25mMoFZFWVYceVm8k7QojXoOwGtooXq8uZEdgJWkD9cIo2xlCOP4/TP0tF/w11D13GeuqgokxKMMqhqk65HPsgpx17NXQlOGjOFUcexw7wmoQCy6x84SZtUFGmgOq2I+AO6LqbKpndxm39sSfpWnvI1cedUj1SgSjIJJQfTig6wNlUHnVANrpJ2CVLxh5nOAykRUfBWJ3ACwiV4X6VgKBgoBmgFpDx+i676xRN43jHv4raNb+DBGGZlFGBYefWrPBO3KKB0ClpFvj8LtPI2P83rZCDDzCrF6FRxwGUCZu0EWj2PZUfjDjkaKCk6IQKqlEDsw9/Uxx57NfcmMKwocLkJ5n+eNKJ5gHXw3zlVFQtRC2XatyOWq2p3z8nsqJ1AM8KoikbNZwnvS17CDBs+BbQkOMO+KoUlQl2VMlCXMPh7nNnHLiQgWyKoKi4iqqou0EKwgSn/TyugZaDijNO1439zBmBsfAM7EsA1DFZ2ARG2k3LHRZtm5DXlirY56xSAi5qDecEwzSyMBh+Hw2ba4Lbm5dw1eTvHKVoCyopMIViANX0/6yd+zoreMxgroGG5HXoJ2+837qJ35h6OEmHSAy1slF5FDRFKq1/KbRlgldWwZQmkCngJs7SsOE3LF9C95U+g7INVWaHu8SmZM7Q9bqXtf6KioK6HZqIi2lLFAkqCWIpOCJRVpIJqC0fcHW/R0ysrsFe/jN2hfXUij4sEjRa9aYeA1YmpGH3fLQhWukCwytLRkkT4uEh3M8KwokHJ7UVxw2vZtfNvddIZwxSkpKolwAKxXcXa+1FOOfHz3BrjKczSsDSHkyCWXe35v5ziutiIWKBW6KjF7GN84xt4qCBYLYFUhyZh3qC/OZvMok5Al3qBuij1p9t6q49kSW5hcRoY09u1Mr2dysxOyjMPa2X4W9Rbu6gq4qoqiNRUdRTVJkJZVQUbY8dbOG3lCzlk1GhFACtNv8rjHczroVmMvomLxaay6phrTrCSDoA5yzSMS/mSkCkYNhODZOmSlDFX/BHbD14nJ6tgeQK8OKBNESrDN3K8utwmRqpZmFfDytN0RYb/g62ef0htBRuhjtID1JY/k9uknPg9RZqyLN0KAlaWLd8eZIovmeuseuUbfMQM1nkpOmYd6qdh1U/DCibA8f8X3fMJlu/+oK5s7mbap2gq0PCZlwVYrQP0PvxuTtx8NfcxG/cSjcPKMgfzajxJ4mYawLiLAFxZjTniXodizRfSJlCR/np5gCBarijMsual6mx8I/cdvE5PR8U3H3UGqCjiWEPUD1yrR699OXuJDzguwrAkC6yGvsw66wA9oA3fHHRCo0I3vIlfkb/Z8GLejqiKo5C/ftH8xGgPSwjCGETFdxAqoRU0SWyMO/EqJXTj6xndcCUjvzqPDWM/pOarXyIwqkgTtKxQ3vNJNh/zbh6SUvs37IhZmOQdzJrMRZtmLrTRa1HBPqtXIwUBqyi7ytIB48ytqDkYZVZB1HubwfeewXTPKQxN3qV9vsxQRsRG1QKsA5/nhLUv5yD58wnTgFXSxPEDn2Orr1kFi2Y3SrcItdpWhvuezATZQaaP5u2IbKSat2qkCRihSFBfS/d8gr5sVTQJek7WupRwH/sd9v9sK5uau6n4f1EXxQVxFXXdCcqj32flsj9kOMKw8sZe5WEceVpNkTKZ3ZyAU9QMdHOyLBLAJM1bmBes3Bzn0wiZenGOnKRYrQogqy5mR+MunhCI9ILOqM+yx37CeusQ1fJKLPIlP2sBwGq/bo9QGfsxmzyvIJYglqp2eR5MKa38Y/0N8dV1Fwu0luphhQCLTsDKMwmDuHbfJBTP5xcEslMsHys66Q2jinvyN9l/+1kc7U4xpSqeaSg4Ao4i9vB3dM2yP2QswcvjkK8uUhxYLaQJrFsQwLKYlpsTtNwUk1ApXpGhaO30OGAMR7obxOcTxmVUmIBseC27H/47nuA2KAmUVFFBbMDWprL34xy36Z1sJz4XNa5xaZpXM1Yg3/MxNjuez9r73dDXmDVtbXwT28nuqJPkZV6MjIUj1iTM0zkkDAwBVPmnVv1JIUTE9rg6SGmTv52hXj8Fe9XzmDlwHRXQinorr4PigDoTv2QgooOEwcpM0TDSACurn2KngNWJeZgHpDoNbShybG6O+ywtK62pSVwRyJLZB8vPZc+h61mHUBGVLo9hqYWIfehresymd/Iw2VVH8xbxm6c3Hfwqx/lgZQliK9oN1IBa31nsKPXPa5Sal40ngViRx0teQpKLsc0reewH9PlXygtk8ClWXJpDkrckeiHn9HHrPpkm11ERwVTF8HUEBdQ62K4aEd6/cMXRPAMWQNwpzJEfMDD2Q1Y0d1OzRqjYI1ScMUrWCGWnQVmbmGYPttmLVe7DLq1juu9MDi9/JsN9T2NUSolVTRcLuH6bgFWkV2QWIERruMfF9sWF0MiG17Dr0PUc5QWOagmYQcQGtRq/oXfi5wz0nsEE6bXds/YvDrxo3EF96jcsByZCGlYlGKEbXsN9ORfhvJ7gLP3xiGVeJfKVuk3xokiQP+j/72njfv5gVj0hybECCWD0noqDB1btNt9eQIWoPa4V5saUxYFVnMnC5L3URr/H8vFbWTZ1B/2T2+nRFoivkcVM1hlA7XExnHGkuUdL3Ev/6PdZ/sjVcmKpT91VF7N709vZ0bWRJukNEVJB62dbOH/mIZZ553hOMjkbXsvPjv8wdzwagOW2MH66jlc64/To7NKhIFLZoAef/DCfIV8F1zgdKynaPS3mzxg4j7Hu45ie3kEZlbKIltWLg7IQsff8k27aesacyHctCFhJDJs9H+dYVbFAbQEbtOoL7rXKRqZWPJ+hHEC/kLi5JfMwwSQsAlz+Y19VCmWre4+EHGJm+PeijU4J6R9uz+PasVbe4A4mkSj22Jwo6SBfK0/xNn55Mueo53VyAEeQaVBHUSc6qGQWJNsz2DtcMVTURNWwJ8Tc9ynWD31RN656GftP+Cj3STmzk0ssgK3+Ux58+G/lNB8k/VQCAcEY/oZuPf7D3JODqRUGrH2f4Fh7jC4RGm2Hr5el0LXqAu71xXA3B2jFMaysvpfRrT2GVl3Ivl3v5xgVyqqURGiqV2uqNfxt1qrDg2LmOs95nS2oC8M3sBG0CTRRmn7sVRkorRrkwYIMtRPPcGEz8Pe1L6FRgBonbio+sxJEEC/RmTkmYZoelq/So+N5JGXWgBREDBQxK7NMjPmNM6IJ23M2nxfOCDSAhp+zNuUFKHqFTWhXEpBukB5BexC6gYoIJT+WfxpooDqO0HBmZHr/Nay+7Wk8vrmXOl7Bw0rkPnhcibzWBXQd9ZfsK/WqoqqoNBAaoA1UWzO76Rnexobo3yzGtv9L/AFgqTKt/nkRVZUKHPVW7vf3Ne58Jr2edD2ywGpOtdv1f85eKpi+SRiI7w6obR/EPPhl1kR+I/XaJ+zjHNH/4JdZZw35mik46qm0QfiNu/GNPFAAoPKa8p14D48I5mUUdOPPAzQJhHaRWbtlrhAvHQDhPK9i4w66VXFUfa+fYPi51EbXBuyEiRE3UeYMYiEof4Plg1NVlTrQjUiv4kXuI/QqWhfRXpA63mfq3mel13suJqCiOoXQQJlo/ALz54/hsc099MSAVupm9mKuuIA9+LmU6kV6z4gv/u77DFsT/rYIQM3528l7GWjczjr8sAGEGdBuFWr9T2Jv1wbcEDhVUkCqnABQcQX7hOx+l+Wu9bgDT2cUxPs+kYqilqh3PvZ/kfUpC1Xaa3Glk0zAPPAFjvbHhu2fj1pbbD+Tg9VNHvvN4XRZDP1yyUuYwzsmaSZj2/TTdlazl9ascwAx0V2cAIpRk1An75prhoinnxmKSmVjO9k53IewrXWl6RazYRnU/NeqRg2j/nid6t5As7weq3o0dnUTTtcxNLs3Y9sjak7dT2V6O+XmTiqTd2vl8Hep42gFmFBoodrEM1d6nUnK97yME0/9PveG0kjymC268X+xZ+hajm7nZwqTKC3AGvkBa1uHqFVWzjogMjSPrPQj3fMhTkRxfHHZFrTsh9ix7tVtdhWNb0vTC/N4C+PismL7W669jAMj/8UJIlTUO8dTIJag9sjN9LUOUK2smXM+OsmzVIDWIcyRW1gBTKmXO2ijQU8DKa19uT6Qg1G5BRwBeQDqiAawUoHPxrKhdnUrT7wSX+WJK+GR9D25bpN3hyaK4KqXx2igGNWjsJgb8wPxRf/nHY//oYoIpb6zmF59McNrXsaE2ZdYu8g060jXUVjLzsUCJgEm76H80DtYMXIDK9XlkM8wG34Vi/rYj6jvfCebNr+HPQWEYa2fQqvvyYyN/YQqUEa1JIijqO020X0f59hN7+QhFh7Do+rCoW+wCWiCtPzlpxvoLq+jtfpihpnNXnByApamAFY0XScJrNqBx6v+mNEdb1a3uU9MUNNbNL2od1q4+z7N2k1vZx/5S/okevL2fYKN2sSFILLeHy0iYi7XqTWXsSeFXaV5ipPiw/Lklh6x+lWchkWCeSjJBrR6hUWRNssSL7ohLeK3kInYuJPKwa8jCK228KmzFL7nVFrEZPuTXTW1VD2B1qa3Mf7E7Tx02k3sXn8lE2Zf20RJM3XmmJc9j0FP/hoHT/9v9lc3MwD0eiYlM4I0geaej7HSGaca8/elBDOlDJTXv4b9iPiml5RVsLxqAWINfZmNGaZlHvOzDFQO/hsbrSG6AAdVu70fIuU1F/JIzPGXE8ysPOZ5mDmVU8BqTuNVMZEVL2LYCyuQLl9HDGqnW0NfZmXK9S7HPE4Mqzj4Fdbix3sJ2IL2CNKDUlv5HPb64St5WdVCzMJUJngksS2jw/icuZ8TREOqlS68/mEbrNwZjHsvoVdbMiMem5kS8QNWgfIK3LWXMR4j8KcJue1B/MS7eeiYv+VwdROaMriTBON5IFM/HecxX2TU6KIH1R6gJ2AAzgTOno+zKsdkniMAr3oJo10b1PKeq+l5NNVC1Zq6j+6xm1mRIIBXYvSmON2pAlT2fZZNXiUCbB8EPC+KqbrhjTycMfHT9MKk9wNATSqfHE3ZEcBYfyXDIlpGtYRIWdv6kthTd1OZ+Bl9aYtUHu1q4mf0Td1NDREbEVsF23cuIaKy4Q1sj2FBLvn7JWqCxkWGKX9Ea1lGhls3y5STWSFIJB78M9lbIpNTC+Ou59E9+RvGUJ0AGiCqSo+IJ3qvvZSWWWv/vUG+COqk5gelDgf6nK33TOzN72MaoYZIzRfzLcDa/2mWZ6zs8yaRGJhrXs7hQCgX6JpNwBV73yfZmOKtS/Potd9r7qZn9BaWIVgqYilSAmqqdPefxWj16LbYngVa5QxmU0phUFmdagzA6NlKq/eJTAuUUcoCZU9z0xZg7ft0m2XFpfokPZ4DlHs/yVoFC1VLFAuVdqJzz0mM9z6O8QLg5BYAryKVaY9owOqEBbUjr9qeQh/AZAH5nurC3n+m9tOj6Bm9iRlfvG7h0fKu4JIZVXTDGxlJ8CyaCVpIHrBKMyXNjOcloLTxL5jsPQMRpar4ZhbiTO9Epu6nm+Q+jrH7uOENHJQuTBHPLBTPc2WBtg5+i35nvN0nMs5Mq2S59fd8nI04OKjH3FD1651Jad2fsSfLtCa++3cpg+HGVQNJAqw5AvzqSzisUBLRsl82uQViiWAd3EbdbSUuBmmygQEYamEeuoEVwbhTtIV4+6xIadVL2pphWt5iUvJ9Hv2qkzZzv/f6VdSLlxXOkGIR+neehuV4ojBsfyPLU060h03TyOQ9lA5/m8rej9G9439T/8VJ9D/wWkzrEFN48U3TQFOVkkJZBMFAjvtHxrrWxXYgSWviaf4WNgMw+57UrslkerWT1BHBHfshdeKL25nE94Y0KqvRlc9jQpWy+qV1fN3Gdidh/+diY5DSYtHmvH/wy6xqhzJ4GwBd67S56mIOk94oN20hSDo/SZVI83STMdZewrjZh6Hq/5YqgjqK2PYIHPoqy0hPrjaTjmPo31hhH0ZAHBEcYTb3zOhWa8OVPEJ6p3KH5P6IeVnVUjhDipdwASdF26Dlf8mMeik51T0fkb49H9G+dvUZ78KrBGEPsz3nXMUvk+w1omj6k2YSL8rb9XMU654mhLHlYzjrL2c6AqYG+atgZlWajL6f5oCI0nYDcHtPw1GPEZn+gHdVcSZupbru8jldgMOetsTj2PB6hg9+jWP85h5lnT1P1v7Ps2LDGxgmvUpA7D4f/jZ9Mw+Ln5+HLardCj1Az4o/5oAY85h49Hvd0PkJ9t1J+GzcwiI5gaq9mT3oiuczcfBLXgCvKmUVsYI6Wfs/T9/qlzKR5QlkbgqYC7gH/oWVnsmtll8wsiZQU6gteyYHzP45YRNhAHIeBbOQJQArbhImU9LACpwNGp3wgeYAXtR4Q4UGSEOUhgiTijTUqx46oeiEqjRQnUCDaHMm/Mjzaf9iikIfUDe76d3yMdz1V7TBqkhZnLSSz3FgZeYwVxL1lp7TcVA1/DaNfiiAuFP3eHWeSE9bmjeZ+5/KdM+pWIqWdLaNlCWI3bhTuiZuozePZzRqtu39FCs9T5gGuph3aQ3Y8Dr2d6g3mTn0qLT4vMw+fRuuYAyvh0AJoSw+WAlij/yAruZeKilifmw55OYeKmM30xtmm+GI6PWv4sEOwMp5lMzBIyacoRMNK/bESds5OEd0bwEtQSZRaYjXaLWh6AQqXqoH0kBk0gM4X1AXD7DwGlROeTglJtBn9mnPxjdTOuN+Dq9/FdMpjClfqk92txSTYqWj502+nj/ARsQQwUbUYx6iao/mZxLRbd2fMSJCRZAKQjlwu6Nq7fsnVmSEc8zT7VqHqIzcSK8EE9SrhlADqfU9hZnu47GYX95aMkzwvMCWp0EDCY4h6TuL6dpWHETKopQRKp6ZrJYozv5r6C8KtPs+zQrXaYdJ2EBV8WLRujZhL38uwxmAFDYLHbKLCubJBS0EXEeCSVgEtOatAn6/QULQ1VKVih/s4Of5+3mG0q49oF4/Q6/iAqgrioPXJaeFUhFRUZV691Z6TvsezcqaeTWHNEFjiwZW5C1S2L5XC2P0FiqNOzEn78KYfgCcUcQeB2ccdScRdds9Y4NgWQlK2QcVDEVpIFjqieQVVdQeby8WLvMTv6PlduYc27rLGNv5N6yyR9XESwD2nBJKa+hr9Bz/j5SM7thjj3OP6/5Ps8KdET+yXS3f1DRBzbWvYChynqKmayfVNYvUN8/qicjqlzHx0Du07jUn8fbN8+5hDV1LfdM7GEsZw/NA4eB1LMdPxVEv2r8dN7fqRe0igZqTTSVpWHnzCpe8gxmApUXBS0U83hCCCFX6QbtVpd3mS1VVCFrk+spV+0Kp+gPNEVFLVCxFp7z+lFjT94p9+1na99jv0vBX/DhwkgxHQWb1CXWR4eupDn2ZyvC3UZ3GVW33NXTF676o/v63gaktz/lnpW1AqNiKNvyyuuKbceqM5Wa28yLajCq6+mVM7P0YNV+7qYDYiDrOuHDgWh1Yd3l7kmbqbQe+yADeAutGBsYAACAASURBVNESwfZTgCitxFn90rYHNi4ERVMWB83nsMlcGCE9pUbXvpLDO/+WAVpSQiijOuHdizV1v3aN/5juvrPa+X7RfZwDvGM/pD51HwZ4CwzetQdBDBPd8Hp2poBVXrE9b+WOJe9gAmBphoCcLgD6BapCGlYvUBWPRtdVJTShZ3HFAzgNXSxRUNurO4QFGCpqeVqWNGd2ws8fQ9eWf6Ky7nIa5O90k8aw2qZA8xHMO59Dbeo+VCCoGR4ddMFxekDt08t2S9gQgGnQvxpaAg1F6vjtf5zJVCDJXEA2vp7RfZ+gX93A46czKC3EE5vXXd4Wm1MBa+xmuqfua8eI2YpURbRHoWf1ixgzKsUmwc0lLgj/pvhceg6FDFaqoPOut97Nor1/O9vmBxH2ES0dJIBbWY2z/DxmDn9LvXgsD8T9ZGWx916j9b6zaCYAavg72fcZBrwGvl4qjgg1VWoo3X1PYbR6dFtTTWNVca3lOqk1X6SRyBHJsPJE08age9QIoopQU5WBs229j3ldc7TtdbNHKc08jNl8hMrMLjUOf4vqyI3apW5bT/FMR1ELFUtVeeBKesSkZ+0raORgJ6SYLm1Wdd8r6D/wr+KiOiXgqIgtiqWCi6ohUFKvfr0hiKGiEjpebw9nk741SEwKyp8oWpdQgqXkS2xNqo9O9/E0l51L8/CNAWBJE1EbVWvip1Qn76Gr5zFtJirzXLr+d+79NH0hcdlF1fBduLL+z2OL0kF67XgBsRBtBqAUOtJ27WxfDggEg7YAOutJph6w7ognUiKeVABdexljh7/Fam2na8m0ol6drG30uB/FCJnJcWY37jTm8Deoo0zixV1Zql4Uvgil1ZfwcAK7chKAyiE5HitPak7eObmkYWXY+/ORPzz0wEQx/bfciP4xR6spDdCqDyD1U5nBazYwNvUA5T0fpm/fp1ipjgz7ydQCehiooxj3/Tm1+uNp1k+hlWECppqD6iC3nkh3cyfTeAN8Wr2DsUC6/NQaU72k7op/pIZPGoLy0BJU8pvN+g76NIr44p6joQmu8yd+Xm9R+5iO/xj7fnYim7xCclr23PBiqWpr/yfpP+5DDKeZcvYY5qHrqSJM+YuD4bnvtdb3ROyek9qOjSTNZv6+e9RyBmTSGxE6l2r5By9+72+vN9zs+PHlzbq/FLiR/bYjxxKwLmfF8xmrbGB1ay8lbY89sUFtewxn6Dp61l6avsANfZk+ZwJXxK971T77IuYKba69hH0ZYFXULMxqy7ZkDhYErHwsQMJewqCMb/uxy/wqCjBb/mWePlLbgrvlIxzsO4O++6/QFerIsD/WbYFJVQyayL2voO/xP+eQGPETOofYLvdeRm9zp2cOCdJUdAa0KkIXqnVEukvL1Ox/ktJ7JlLdhHZthK6jwOzBNapg9uKIqRHxNGAH6t5cos9vWjDtnxhRXXjAYPextKqbcWd2eDFZAk1FW4JY+6/T7mM/gEg5kWFx4PP06ZS4ftlfVwUD9Sb86pe3qx1kebfmvh5QKvX6VAYmdGgx8/tXzibJ+xqozprbGu0UFB47UcYVhF/IqouY2P0hugXpQtRSVQs/KXr/F6iFACs20/XAF+j1yyBbApaIF4emqrVVF7BXSoliehZo5TEDU3W6JXaVLrprDu9ge3J6nj4NCH4UOYL4lHBwXlI3mjngteZPGVVF779Cl6OM+Sxl2guZEGvyV3TtfLv2b76a0Zzsas5t9PtUhq4FRJriCeM2nu5WVaS04rnas+H1qgPnYvmgKDmAfc75cVu+seyVQQl/gZBeciSpkzIRj+H4Q39Du8GoCI4qtjMkevAr2rv6pfNYRXuy7v8cPb7ZZOEFoHYDmP046y7nYOjaRSefncQ02i11oR4S8SToDO6BlR9kHJTRDtrjqleNMfA5+4w3iJkKEtOdBNGcjW9maM8/skXRJkpJkBn147ImfkT3zEOUqpvbZvKccT7zEKWxH9EFOuGbghZKl19/SNa9lp0ZGpWTk13lFdqXQCrhZiQAVdqJnD+xRCS8iraFVG9wBxfSjtw7We+vvZSRY95DA6Qf6AOpi9D02pXrzK4P0+WMpxbpS/QSbn8z3YhX9kUVS70cxS5E6ke9Sasnf4PWsvOw/YJ7SS721G36QQyJtsDSNvZkubbTMvoVYOMbOWTU1BCvcmo5iM4GtfZ/lp6kEzFxK9XGnRh+RQZLoUuUHqC2+gVMS2VeoKOTIC7bgWCP36tPRL1KrCJ1oFegLuq9hlJHqKtXE70uqnUR/32RXkV7gHqIhduR3wlvQYFEF3C61tHsewpNZkvwlAlacjm09l1DT9J12vcp+tUVT6j3WtB3A92K1GonM9l7OuMpAOWkAFdWPmHRwn1HtDkY5yXM24W4PQn9dVQl1ISCWcdZlF25ZDcrncPCjnozhw5ep7XGHZRBS/5Xt1BaNKXrgdfpwNYvxKakJLrTG7+iPHUXKqK2n3rRBVSBnrWXatex/4fJDA9bHo+qTt+H6XfecX2fqMqsS9EhvV5SUnXQtsfMqMKqQaaGrpWKilZQmuIzhJGbqFpjSLl//vXd88/UESw8E8j7Pt8+W/tq3ReZnHbMJLTiGNjZNv8SMokT9BhNMNPDHZY0HMAb5GMmNbdoj6k1L2Nk7MesnE2IphWU9hm6lu7N727renMWjKHr6EW8hQuwUal5c0PN1S9lVw5wclJMxKyyMkupOB0yLAoyCHfWApjVJ4IxqbOTMrpSRh8nbV4XGxNny4cYwvDqaCvUUJriDcaZoS9TsQ7PqS6aeYEPbaOigq0qti+yApi9j6N0wifbZlTehgKJ+sTUAxg+2ZxlWNIO4lpIzln7fsPrOQxaFvXqnPuTzhKwdl3NQBQ0nEmMQ1+nC/WqEPiXrKYq3bWT1ek7s93bz44AlkYex11DK8drVsL1tnJu0b9rBfu15lKGzF6knRANrte7EKv5sDDyn3RHz+XknVRmdgqoWuLVAnOD5khGN/aGV7cBKwmk0kCrqI61JLbnAKw8cVeJYqv4bQiZbe3lyacyB7Di6H10ACcNZLvvqUyseRkNhKoIVY8RiVf6w6V13xUsJ7vc7Kx+dTOm1zXa64ICVEXo7n2Sl6ZM/m4naa+7M9sxVMSVUByXh1lKxuqbG7x6z2Cy9wlYOluFwQFaqLQOXktVde61PfBFepyJNhDZvr5WAi2teTkHI9crDFoz/jVqhTarINiEt+j3FP3b4HEbKIwu7JUXMIGXtlQRpOLnF9qKWvs+Sz2qve18H8s8M1p8c5qaL7jXlj+Lw2bfrNlZYEsLZ1iKvVoEhpVk3rgZjMKh7QqaDUzy3NNKxmqbBFqxq+lxf8/e8jJMP7Lbb1VOS1Rah2+gq3WwDb5uFsVu7fcc7b7yZng1NTH6noBDducTNwfD8gDrIQxUXfVW+pAnbB7DchJ0PSdFvG3v17pXMSKiZfHrXgk4oPbMXvTQ16iFP7vfm7ThcywARg1n/WvZG5lw4WunCXpSKwcYxT1P+nyrIJiFv9Ne92cM+YX9Sn6SeAvxPjv8Lbqc8dmF1BlHDn+bbo+RquWnUGngPFp3BTsTromdE7QWUrCPJdBKZlhpLCt15dfAV+/3J/S/JDAV7RRQsnPQ/fZr5RU0113OiEjQnkorvkbRxJHmfa9keU7xWgMTUhVXBNPXVaT+RFp5jjlhC5uyrjroxF1450CxUbW9eFKRiEnoZIQLZLKsNZdw2FyFqJ/QrEFqCVj7r2mzCrUOYU78EtMrgyw2SBd+y6oVz2bSrLUZVdy1aIZYVhZwpAFaGjClPW+lAGR7PPU/nZHurWqBV8EBoYRn+tvuFO7+z1MPQGX/5+h1p7xrp4jta5k1oLu6CWf5c+cxTmcBQNVpGMuSOZjiJczTrmguM5B2iKDOrk0QYVhFTIdWEqhteDN7pIbhxx1Vgs+r0Dr8Xbomfu7rFhmg4zZ8v3ogvXl4Rdea3PWM4jSNOedr1wfosQ+1J78V6FeziZexXlI7YeCnTgQp46z9E8ZktuFEOfjd0e9Tae7yRPqH3sEyLxIdSwVLpB3xyrpXsydl0bAiwGzlAJxmzPO015oZf5+HfdmAtfoihkT88+CJ7xYqQe/CvuDYDvwL/R5wBxUvpOxXwCiveiGPxGiqdow3287wFC7VvXqUGFYxoApE8Xb9PgnlkLX19zSwauVYjedMoMpKmmtewggiXTrbwMDrAag0H3oXyxJWwzkDpftYVFBDBEODKHRRd/JXc9pVOaQnuCYNUts+jO76e7pQZvzQiRk/5MgT9zysTMo/c1Nc5ImgdfRfM4RJySvuh+F31Zl0bRr7PkXdnUEPfpVu0AlUW6iKKt0o3bUtsOxcDsewxfC1m44AjJUDfOLAKOu1ZsbzTPBa+yp2U8b047HKoLYfc2ZP3k518teUJn9NeeI2TEVnfGHeVj8O3yih61/HgwnAZCeA1UJKymSJ7Uu3UFgDCdpVkmYVfmzMxraH++a0b3bEJU/eOCniy+gaR7+VXfu/qCvUkgqqZaCBSAvV1uiN9I7/lErfk5lhbvXPcAKt1LZiT96NgYoB6iK4KE7jDsz+c+Zk9kvKAEoE+YfezYA9Jo4X/OjlpYlSiYxMJ2Mwx52buHPmAlJeRWvgWUyN/IeWBamoalO8Qol64Ev0VI+hZo/JaCAwh2qhl1e+WPdEwhhaIUE7zLCcyCISB6qaEZ6RdL3TSluHwxuywgRKXevQgacxMvo96QEt41Vobfh9M+19n6IfrzTQDCItUWZmPdDa3fckRqvHMJnAOJ2E409bXBYqti+ZgzkYVhpozTGFgpIx4aHoW1xZpmArQ6id97nqMTRWXMCwaKhlldICaanS3HkVy7ME0tpjaAXxPn69bhtwhv+r3SDUSXDbp+kZNmDP7ET2fwJTUI8ZqDRFabUbzfpx3DHn1E4R3HO1jTrhY+wGyioatHF3BCZnHmZqx9upodrymJfOqPrFeyrqbLiS3ZFz7STojTMFWVUcU8q7ZZmMqdvaS9mFlwFQAi35IsC0ikwe+hrVQ9voFqSB6qiijq//mSKUVr2U7TlDMdKuWZazZElsXyTAigJVWgyKhfi5caE2XyGPYR53dp4BOAe0Nr2dnRjqd4ORiohaiJedP/I9qY7dQleK3mD3nMSMZzZhqNfjcAaYGfkusv/zVNMAKW3QNn6Ncef5rFAHy48VE2arqATRHhJin+HJkBaAaCeYHHNArHoMzdpxOCglNAAtL7nZOugnOaNTQA2RHkVqK57JZGUt0xnH2kwIa2jlNAGbGeZe2mtp3xH9rfYYW/Vi9pZWqyUyW2lV0SlRppv7GWvuYxqvuu1UeEKY/UyvvYxH/ONNCrvpJA5rSWx/FAAraTWPgpbdfj7bg2KWYM0+t2KAycrpJUr0DNUfy+jAuYwAFYGKr2f5n9fmjqtYmQYuvWcyKSUMvzmE4ekzMg1MbX8jPVPbZ/sI5vFgAvb+a6jdcRZrZ+6Tlnp17S0/BCrw3PnMUwKOleV5ijPBM5OR11zGsF8quIRqyQtzkGGBhgjDcwoDqbL2ch6OAHo4vmqGUKOLAtcya2sWeK1Z4Dva+yUlmiuex25Vyr5nuSLeeLcEsRA5rF5p7poH4F6y8/Jns9coe41PYuIG48ZC+Dpm5RAuRbYvImBludYTGIf45feCICwRz0gUUky/Tgd6ezv6r3hAhIqiZd8b1BKhJdAav4Xuke9TTfA22tVNTG94PeOCdgniBaKKNhCGnQkmb3siqx65mj63OY9tzPNIuTPY917GqvtfJcucSca8uvUeq+p7KuqbrVV/WIZrGBaJ7cnd3GDD6zlg1Nrt373OyN7vNbwYNrrxtZryBnTlCxlKACsrxjR0aSegZ4JRHgG9CMNqZbw+7zc2/DnbRaj4aTplFcoqNFVo+E1X24urX++GY97JHSQHx9opYOWQvxzyUtnjBYjuaWAVnTgGs3WJDMAIFabT2QvfftBifjmPtAsVJ8DG9r4bOIeh3icwOv5z6UapeKxCWwpNESo738XaZc/kXhJqtm9+L3sPf5djp+7xTUsv6dUSpeGMw8530rP3M3Sv+RMmayfQqm2lWTuR1vRDlMdvpdq4jWrjDrom76GiU9jeJPDqegGUV2Oc+l12/bBXtnpMVFvt3o3a9qBm5RBK6PwZkYXGjTgGBDDMHpyVg4wPXSs9oF3+xCwrTCL04DGOsirl1RfyCMkpNuEk47Bp6EZitfIEuEK+GvtGwnUPKjWUisSo1U/lcM8pHGrcRR31QhxEtOzHXImA6TeYqCl0107U8e4T2onObkKITZZ+lRbhnlVGZskczAlYcSzLCXllgkFtRGK3Ku3HngcGDRJfPRyzUi5CXAXItMEbDOC2JrHxjdx7z0v1TPUApxyUn1GlNf5TqR/+rtaWP5vxGNASo4Js/TSP3P50jsVFvfwzaSI66Vdlsps7KT/yd1L3knElaNeliLrqp/aIMK3KjFdBAhORSmWlGqf/lB1GlaZXVwvEYzh+VVKNAlbcYA7XEYt6CMMlg4Pr1T6HG/+CAwe/pCeqSBCvVlHPXPbPuICh7obXsTNnnJwT0nSaOdlFtCpp9FpDerejMGAleQjTMg8coLTqIrY37pIzfOG9rFAllK/k71EJpbTqRdwTOuaZEDBHTUErBbTyRLizJLZ3bhK6BTyCduT1lt8qRsQLbu8N4rD8uKw8ZkOWZynx8aoXs6u2lQna7dilF6+9egu0tfNdbEzTO3rPZPSYd7Efgx6EXlTrXu11XF+QbaDaIOibCA2vj6LXN1GQSf8xQC9Cb+0E7Tr52zxY3UTD/x0NksFl1jOhC/Q2hQNV51V96H0cE71PZMoHcb/UipbQgE1od/9ZTFQ3M5mi9cUBVzMyFrLM/DQnSxFZoFlgHM0ZK+tfy33SrY4gQQmeOiK9eGVs6r6cgFHF3viX3B36bpfs2MAiIQ1LYvujoGElgZQTE5/jbbPVsevq1VSqz5Z86liIzfu4ufZK7gIqCDX8Wky+TmM3fkn90L/Tk+ZaP/pt7DztFn7dezq2CL1ePXHqIvR6/f9wRLQpwrR4HqVmoN35NZfqIFUR6msuZeTxv+AXvaczRrs3I14dYKGuUJ8tXJ4o5s8LG8lYsWNX9bV/xn4/NaWMagmRSpudCqU1f8pDGQCVlvuZpEmmBQdnOVSKjJOsoNP2VupncuCp7PA1LL9Gl1d/y9uoC1LpO5OdpRXtRSZuH50MHStPSeSllvSLqGElibpGyBykzRr8ID3fxKjPOu+91BOfdbciK0eebPSkoNHALHTCWtaGK3lgz/s5dWYPfYGLcjaMQHj43Xr8yhfyY1IadvadSetxP2N494fY+PD79BjnsITrWLmouJ74JJ6fTzDUS/A2pQdddo4eXH8F9654HsPMqxcGolJvNwZqH6JaGYM13PbdiNGAnBg9K/isrH4Jhx64Uo9Tx9eA1Kt6ClBazvTaS9iTg12FASo6aZ0YHSuPbhM1B0kwCZ0EM7CUMy6tPU7WX8GdI/+pjxHoUl9GJCQOKqprL+V2ZkvV2CEvaZhp2SGWmaVjLYRdLd2SwOFsmw3EtzMPypWYvlYVvG6Ense1/o4W5NMMwNIEITartXzaFtVBoq3fE1ug24epjPwnA43fUJ/eTr35ED3TD9Ntj1A2B7AqK2iVVtDsWk1z2bM4tPoS9pk97WoGaTWzksrSpAnSRooQnXTuBTD2f4GV913OFq/TtjbwarbXEXrXXcbDJ/wzt8cwnCbpuYBu5LW0/Lk8EzQr2j3u2pZybtG/kxg9UCOabVwcWpR1zsQI8llMOW97ryVzcAEMK1ihovW0TWbLjbghAdggvptvXsBKE2HNAqBlxEzkrJbobaAoLae56iVMrJoPIIWqjZJdS4sEr2CUXWV516ImPoBz4POs9TvHBBOqy/9D3Xw1d2YwqqQJ2Aq9H9bQ7BhmkTc9J4lhGRRrghEdu6XIeMgCLJfkwFkrEuaRlZmQN6RhiV11AFjhfm/RbjbBBW9FqDgxwCY5ASvNLMzr5k4DqjiwygtYWd2jNcPrWXRLYxxRc48Ukyq8TzLzMNWxH9EvyiReN+cSXmG6Wu/pjJdXtNmSlQJcSUKzGwNi0ayITgAryVPoppiFaWDlJABWdBFK0m/jAHsmZArHabx5RHfIbue2xK5yhDWEgSuYJHaIZRFxVYfZlZNgDnYCWORwc5sZzzsFLDJAK4tlQb7mmHli0DR0PMSEBbgx+2IAsvuDbFBHbIKuOEIZxVQwV7+MB0JesDQNK+71sPfMjTDtTkzC6HG7zM++yMp3jYLOPJ0zcv3TACuOZUW1OjsCTjbzMxKyarIt3RaBYUlIzDVCgEVogoQnr5nDHOzEJEwzEeKYVB6wSgOsOLaSp7qE5nAoFHE0hMHKjQErIkDlxoCVqoVx4CusBW0i4uc0allFxOzXmXWv4iHm17bKCo4MT15N8SLn1enSTNzw9TJTTEOXuYGk4XitOI0vDbDi8mbDDDLKuNyYc1LEG5hLu1q6pWtY0QEU1q3CXsFSgnZlkN67L0/AXBZgGSkg1QlYLQS08piHSZ+LM+uiml3S7zkpzEP2fZq19kFKwLSgtkKPCnVBe1YPcp/R3U5idsjfICLKKuJCL5L6LBZqv8bcLkthlpm3GYiZIg1IgoaVFX+YVF02GgeXBV6FbkvmYLaG5YYmhREjAAcXJI1dSQ6wymsaSQoIJQFVkmfQWABYSYY4qjlArIiHzI2wC40BMSs6mdWFPZ9gE17E/4yqTIMuR1Et4RzzPu4IgVUSw7IzvF9h0T2u2mZekTntPGSZhnFsK81bnGUBuKTHIsYxrIX0HVxiV4vAsKL5aVZoMkQ1ATfnYMgDWkU9R0nAlcWs8piDRc1CcrCoJJM36xjDNyfH+XUffAtbp+6mjhelbwE9+OEpy5/JnspqJomvdZXkLUxKyo7TrRzS++6RU3gXsivhppVFciOCe5wFIAl6Y1oJ7CibtJnvLU/LDc0U2JfY1cJMwrTJmcZcKKhh5dF0skArCaiMmO/JAqtOQCstEFByMMi48I2ANZSZn28ZZroKuMPfZNXej7DZZ1fTeGWZe1FESpSP/QD/TbiOWbw3MFevSDpLRykCWOHGu3HmoBkDMkYErJyCFkBa8n+cwJ6HXbkkd1XPBVxLt3iT0IgAFymDJI+ITQf6Tt5JnQRMRZhVGlB1yrKyblku/LApSBiMQucsuF6mupiTv6ancTsDQ1/m2NEb2aDKjCCWijZQ6RPVukJ9xfnc33MSoyRHq+ctnRIXyZ0VFJlm9khEr5IEdqkZ41Ej589NGaekAFYSaLkxjoU4dpXW0Tl3ovMSu8rPsLJYRJaQncVAssRoyTG5JQWwkoDKSGBWjxZYSQ5xPSmSPW7imIB5c4m/ELyCPv4MdwVRFWxUJ/Hank2iGKCqInSt1okT/5kfEZ9S00oxB/MwK3cRACt6rwlCvBtzjpI0v6wxmpSFkRaYGgbpPDmeeYFqCZw6ZFhughgZN5HyxDNBekmZPAyLFPBJA6k0VtUJWMkCAEvI33AhbuDP+SoVGrO1qHERVVGxFWbECxKtCmIiWsNQ99gP8l+lFW3tKi6a3ckBVHaCjpM3sjvNWRHHsuL0LJibM5mkbWWNhTSN1c0AriRWlccrmEtoX2JXnYnu0dWLmM9lgRU5PGd5I6AlA7gkQ69aDO0qEbQc/yhMSQUrEvbZjLCExIYgAbdSlYbXU9DvHYZ6+Z1KFSip18Kse/PVfGf1S9hFfMxUuKLCnJAGVWxRbIxUVmUnmUKqqDOKujPogN+D6OEW2qrAxn6g1+tlk/PmRoAsupC6BeQAyI4TnANa2sK1wK1UcpnAeRKelwBpEQCLCBgZkYESTKakVTCPUA/5Y5PSQCsLnIwCQCUpv5ubYRnJjExyiOzh853HrOiCoPeOqgbfJWKoqohQ6j2T7ce+h5/2n8Mw87vytGKYVdhEdFzFEXCM+CjuVHZhN1B3Ag7OoJUWOgZ0T6Pd3j0TY95B1E/r2CSKLq5GDPt3U6SA9r1le79dLs1LOWt/n6OorbiVdK1qUSozLLGrYoAFc6PZk1zrSv4Ypqy4pDwMK8usEoqZf3nE9kLA5Xr1oaUkhXW4OJM7OUBSQLxejGW/Zj5GF63SSsYrq3SsayOj6y7nNysu4ADJ6SZJMVjhfDgbTaw57yYI4a64HliZLrq1B0YG0HIdKKFOC+oz8MBh6B/LpZfmuZYaY06GJQs3dVHVRJbVBiFb0aomdhMvyqqWTMFFBqwkGh6eaJpg7hQxCfOsrGmsqOhW1AyUnGK696KG2l7nZ1dGCruK7V70dIu3z5qQmhQDlyc/rkVy1QEXsF2PYUWBKn2yjsLBGdwVy9GJfr8ZoH+djS6Y6kI39Cee0zg9K41lSYr+lUuyUE3Ul9qb7aLGLMNaSrv5HwhYSatblikoOalvXtDKC1g8ikCVC7hsoJzNCtPCGJKE2yQvVJF0EyeFKbnMT9NxVr548A6Aka9u25JgEsZrOZOeGdjTizrFGHUSw8rDttLGZ+q1t10P+cP7tuzCwQP+sa8EtNtFG4pWspOvyQlWHbOrwcFBBdi2bZssAdb8gZJF0/OAVR7QgvlhDVmmITmAiw70qo4AC0Us2h05BJBlFw7e5w/8x5AclmG6Lm7JQF3FMPy/dlxUBBdJcK+rrycqpo9ehv/Lqn7rQ+a64OcDloutBpah2C7YKtiGYiNzEt5t4l3585iEMVSj76+fcVF/s/QK4HhB1gIHFH0YQ2+k4n599F9uuCdubMgPkZsOwpOfjFbWzTH35jyWKdg5jB7Tg6HLvfF46DAyIEhpGeK6CBaCi2G7uGIgpRIiJeb0zmxMzaVqjSkwA8CaK4Ew7aJdfmModVB3Gi210AnvLGm1jJb7/e6WCWDVPQLf/QFcdffs2LkK9JxL4abNS8xrMUzCNIAqClZFgCuN9aQxmLz61KIy5iTh8AAAE/pJREFUq+A10VQgtGPBSjHUk79M20VdMAzBFcUUwTSgpIqr80HLUBfHBcMAW0Dc4Pf8futqxIKd42tTDoLjCJapOIrvEVRsV3EMs53wDvmCRNXY1yN9b3jW/aKyMXJujhbkaFx5GjPG3xKf1M2q1Sg3w64x5LjL5pmHs7dhKNlQ6cZt+u9XbWTCQJa3kPo0xriLaxu4tiA4MGUjFRO6u5FlFw0eBnA+t215dB8aOm8suoDO+MJ8qYn2TMCEhbs7eM1Gp1soQ+jqLaDGXLDafQ/64G3AQ8BV6FWRHzgH5KYlU7EwYOWh6lGdoBPAKlplsQhoFWVTnYYwxH5WOwAsRzFcxSgb4Apu2atXpeo5ANXyYhVK4kUyzKmxL4JZ8ltbKIjhswdVLzLLjy11IqAVNA1xXMWWsIblsStHBNux5zCsXAGifX957tt9sGq54n5ATPdH1rLJW0uN7rrY5maxzPOAP0663vuPhzNBbt2D/sEhaK6MOclTMDGMbulHpquzi+lwE+0xkIlRpLsLeqrodBUpKdgthBYilt/ky7+Zdc/D2Bj3ntf70P4EHdd0oeyi7mF02ER763BMDXUddHQc7GHP+9k80GaHbdB68DY44SCcDYxeBzuOhd6j4aabgLvhqqtyBWy3b0eyKRhlWGliZ1Jwny4CYMW9HleSWAreLzZQpX7edcEFqcR/bh5gqeMxrIrXzEINMPxOYCoBIAguiisuppqRCG4Dxw9nEAkBtAjq+lxLdB7LcvB7KRrgimBrCLTUBzJzbgBxvihuR54HoIZ7tfV/vvHu6d3ITTfjAmNvWM7e5Yf50VXwznN+CDwVv3HQ7M0w4VZQroLW2QhPm8+yug8gN02gj18P9dDfGy5MAqtK6FS/b576IF6qouLCwRlk+WdPeX/wN7VrTrl66pV3vWXGh+Z6/Fh0AXosdJ+NHtMDleVtnV7FRJetBJlGbxqCcyZQXTf7PeVxj1mdDUydjg6d7/2OAme/CG5+8RKzejQZVhx4kcObUxS8koBKCwDTQsy+ooAFILY75zPRz0XDQdQUsBXFnFt9oPzLNb3d1zz21cZ45RSapWMx3YbWrAdbZ+358vQr77orTmzvuuG44yo3HXWuMVQ7VWZKaxAcrdr73PWNWyff9POvuKumgy4vriM46uKUwal+5rFbKz/e8AqZKp+EYmi3/St33eQ3xq+++RtFAUuQ4wHc5TOfn9oM9gDuOcCqg7D8bvTWW1G+49lAW7Yge9+INk6Arm8f95LaNadcq6KXP+f913/2TBBuhpvO8n2goAMXveAVovIZRX/x+Hdc/5SeZeiyCwetmQu2f2j60l//1clvfd6gWOYlKE8Cmhj6S3dj46/G/+H7DwPQhVQbaNeNx10ZHFTXt4+7suvbx10Z6FUjX93WE3Nd3YFLzn8BM6WXrw6+W/iFu2rqLeMfv/GhYFzay4Cfo6xC+l73rGXGgZ63icoTgZMGYfQg3Kl3uh/k/G/cEieyDw4OPh54NvBHwOOAvcAvgA9v27btv7NE9+C1ouxscHBwOfAO4EzgJGAUuBP4+23btt2SJfoPDg6+FLgIeCKwdtu2beZvm2GRg2XFPe5EdO8E2JJYXSepNVIQlFJBrttN3ZcAsNohDOJ6Ieu2jVkCtUEH/ubprzG3L7tcVGYnj2vWZcxcW/32cU8u3b/skxN/d8u/gtdiTBTTFah97pSPztuhyUqf8cDyE/ted94Lxz/8Xy9y10xZgGMqrgNOz7ue+tzSPSuuEsQI/c2zjO2VZw285g/XJABWYgUCRXcJstU4XL0Q+IfSMtCnwRDogQvA+G9vWmzZgxw4AGdch1x3EbrlsydXPZtarp7azGev8lnWRX8AQy/yWaPK1T5l/ATgih8hX/3m8X/Z9Z3N+8U2r55zvl3ZYD7S9+zeNz9j68QHf7BPTXS4CUelX9d5ptnAn1zwl2KZ75vz3coGc6jnOX2vPe+E8Y/fuAfQkn+1+v7x+U8zML6JV9InuPUDm8Q1zl924Qvedtpp138gbAb6k/5LkZ/e4m9/wuIl31sR4DkHuCFuX4HzBwcH37pt27b3p3gr/xq4+nfBsMJt54s0U8jrfs8qZlbkb+Jik7ISVdOeJ3VUdnK+337NcnCm3XmZ/VHhur1Nu9iO4qDYM4pdvn1Nlw9WNS25e61jR/6/xhW3nzfztF1XOCum/l1R29y+7NXVr2/ZiGKpYjnQNJSWW7Pua5124O8mL73rT8c+fOMfTv3pXZe0Tt//XjXdhjhGb/3vnvQn+C2pDMWufm/TQOneFe8SxNCy81Dz3J0XTLzjJye3nrL72VpydxjDtbenMKz462joFwHENd47cNELPjPw4he8oP+Vz1kFIGUPvM5+B+x7G/q4x3nazd0noaLyr3iNHVYPXPz8M845G57yFBj7tLc4DVz8/DOA1cDMeHX83ypr5i58YpvvVfR6x3Re7Gwc36Ql5y8UnQAq5q7evwr2u2Sjw0998GPB3zWf++DHRr66rfrrD3hbZFx5322Z71HRb7gl58XWyvFNarpvACaAinGw9tfBfFFBz/vqefUQWD2ioq8xzdbRrus+E/iUBxjy3jvuGNwamX9v9e934FmPAWj8MfDjvMwpbgPOD43DV4TApjcEVo8ArwY2As+Y3VfeNzg4+AcpP/s+4L+AlwHdv01trZTBmBZq+i0264qGXeTZ96z9z3OyU03FhtPWQpIYVjjoVroByzcJu0zc3k+efoWo9KihI6NfvOEZlF0BDOuPHt43Dbf2X/FHYox0D3Z957hLWi964F2ui4h43smxL3zrFeGzMPP8B8f1eQ/eP/Cmc59i7u59hnGwdibwEcC1wa1/ZetlomKq6R4a/ddvPiuY1PZpQ78Gnjpw0QvuFJWVEcBKXcBmLrz/H6pfOXGDqLxGVC5FuVTGu1h24eAEcO3UK+98T/O5O/bcsBm9IXQdtm3bNjM4OPhR4H+JY1xpn8XP63Wk0UArPwRxjCt9BvfxVS+8aWZorqiNot+4/qXXX0Q3cvZzAPj4wMXPFxz5EK48Nfhs/xg6/Jy73rTiR8f9OcDUK+96E0CllTweVfSbo1+5/qKbf+g9P/tpfGLg4ucb4hgfQnla+PO1Vu0tPgAcGr/65s3OlhF9xnuQq65i9+N/wU1HvWfQBF4JvA14eehnVvv3L9i2bduv/cfjPpB8vdOJNTg4eDqwzffK3rpt27ZrQ2//VbCvwDHbtm0LjmMPcNPgYOK+hm/f2bZt2/m/a4YVvXhxRceKMrGFtOXO+z1pbC+JyRVlh6nvVW1vi7wXZilzGJYLtuPimL4ILlPlMwCczaNvpezOq5ygvZbXU3CyfKzrYjnQMoSWGlil29bU+t74zMsGXv68zwxc/Pwbll84+NMVLx681dzd+wyPTphb/FXTNsA2GuWTfb3pGmKqMWhP65+jnjIyKjHMvPhea/Qr179Bq/ZjVfRvgB/6v9kLvLp2zWN3Lbtw8Df+ZIjePuF/z8UrLj1/+X8+Bf3JT6DnI+cvBy4G1HXcT8SOGUM/B8D07P5MX3HnF7wVQ04IXvvlBHr30Pxab2ssL89RNabQXtn9PKAc9DdQ7bFuCH13aDWTszyng77G2TKi4MVZAfzyCShQ8z96cuQI/s2/v2VwcPCvBwcHnzw4ONi1kAk9ODi40T//ZeC+bdu2PSnykaf6968OgVX4lrSvc4jd71p0T9KjNIFl5AlHeDS8IEWZXlIgrBZgUJp1jMNNWFdK9LSGE3F9aQrXtlEstGRiYBnHAZQeXPaJZRcOuomOBMs43lUs/Niv8o3HrKl96tQbRKWaeMJUKj4guY7gYpmbAOyTDn2HuRUXXAHX3jx6Q+WuNW8LHV9Wn8H2AuYHht4DvLfng2cY5VvXPVNc4xJfj3kM8Gbg/RGTZsfg4OB3gefIdPkVXMUHr7oK4447Sq8EqsB/PP7x39zxvRfMiUr3TseZ+77L3f5rvubVfNbO8do/nQbzK7XOu6bjNgy10M0xn2qeu/M70b8Zu+Y7Dy27cDD47vCxb/VMYvlKcP0GAQbnXcfHRL7zo765tiykCU0ODg7+B/CBqOieA6z6gNt9BrUXODXmY4FZ+tXBwZSxNn9fw7dv/K4Ay8hgNnkZTxo7Www2lsXq8uhkSjEtLe/ntWSj/6+9s4+N4jjj8DO36zvjD2yc2lGdEkFM27QqX4pUSpKStND8E0K2UaqmUiNXpY2qoDQqVG0JNHI/UpSWNlXTpohSFDUpSgkqA4nAAlJEWyCKpeJihCDOB6LYfJnE5MD2nW/n7R+7xmdz59u7s4OBfaXV7d7Ozs3dzD73m3ffmTnbd5kaG+4HuqSwlIVrK4wY3EQCF7n0x5G+tNVlw26UKAtFvxL6EdwJGz69TIkqlYhc6J95+tELi1vnnXt69+yzL+lbE7edemlYaEUqkhbFrlKI8fxorggmYjD0YCRhuRkUljDyVL9e6MFbqD0/R/b8HcoXtpgTv9+6670NuhF40k/3cJZ2+Jz/utJxHNXa6gioFQDGmOeaQCIZtNnFZS39gdrtoEoakrYyjvnfCaSrHYn1Dr22d3Gbt9DGYX8b+Y+sJEj9cWkwxCVYtwM3Ag8Aa4FDvsJ5ANjnOM4X8oCVDbQBH/G7lTO11okMSQsq67Bynx4PCitoiMFoPgUcDZ/XWCm6oCpMoknv3NmjSDLq7Q88Mk+eRJLezSZGoSbWemorOgHj9qIkhWCbI6SsuaYy0Xz6j9u/EVVEDKiUIlIaHTorhRIvsj1pQPXYswD6P3HuB+8v37+zBLAsxHXBOlXWBjyU9pRIFAiWeZeUNds6WPcl+jh8QZCoYD4wmGogerxq4bDfIOh4OZpvHQT1y2l5OA7rfUft1Cy/4Tb/JqsG7vG/axUQr69/c9vdjSNUbpN/auUISvpw5vfjlTC5Cw6/PJjEKayNHwLmiZKtWzZvuT+fRqa17gM2+xuO49wA7MELNfg6sDtgVq3AzUACmKO17hqprMAWrbXDVWaRPKa1KMQPVYxvKujnBL1mrNSdVNcgE3q9reo8Jpoc/PyTKeRcAmMJEpFBhSZg1ASMXYqhxOz3vPfRz5V3VliWwo3auDH7shkVUkqRilr024oUoioAVFWi04qQikTo98cE9lsnKx8f5vj3/GEx9z8AkXjs25bCRAXXVZhKG6PKcEnY38misMhRH3L3u/CVjZ4Dp6lpSNf4s/7r0Sw3rfGBBrDE3wBWrVlzJDXSeLtpz3jbSHU/edCLkwCI7ZgyESBWD3VTvTCKj9dkXpxiWpW35WiD//YV8J2+0inYtNbngC/7hwsDqqvlPuAMcK/W+kiusgKfL7asV0xh7bGRu1IFxXx8WNG6QZVePmVTo/Q9VUktUlI7eFyRlv5TO6ev7vlm27IMfjWlQImNijft/UXlj+76lhJVXfnD+S1SkVxvbrz4ep/Tfshuq51knZj4MaujYp6KRxd1/23rF1EoZQEl5jBJa47dUr+2esW8pcnbOw7FdkxdEDld/n1lVN2wLqEA0n9Hx2+jO6Y0KlF1VY33v+5O6W5M3t7xTnTP5CnWiYkvKFG1I3T7Ms5CENvW8HDZ+umzJWJ2JiO8syjqnjzgqpjj2HOAucDSNCWVzf4M/ATvcTx4oRjrBk5ma6M3PZa73m5ZgLplAfAgbwLTJ/xp5i5gPvcci5fUwZlZUA9SD8KDeeU/YKt8yNYAxxzH+QPwT9+fVIMXBrYA+KrW+jNpoDkIvAj8Azjm19NM4Hk/yYkAsHoIeMo/3K21fm0syjqugDXQIAb2C4QXVwBexYCoUNiqHDAdkm9sW8OS2LaGJZky6rvvrWd7Gw894TZ0J92bP7jXOj7xFSWqXsVjKyPxGBWr52S6bMDHpNyp3avsozdsVkbV2e01L9rtg2N6TXXfTyPdpU+mXSOA9Dzy346SNz76XdVd+qxy1TT77Ul77bcnpfVvUz9WSftnOYA1BFpl66c/7zmdI49jQKUyuUY57gMpm7LochxnY5qfa5PW+mx6moE26jiFtRlRskGJWqVE3Va2dlZ32Vpv2tP3N+lIvhkO7ZnoC76/6TXgpjS1mMumA09nOWeAFQHy+Etam5yfLfJ9IFZK64LLOj66hLkr46qzQsMoismfAF2my6z0lWmPDTjl47/e3Xphxf4GU55cLpbZJUqOAQlR0imWecOUJ3/ZP+PMnenO+/hT/9qXmvbefWK7ewXpFiWdUuJud+suLj6/rvl3wyB36UHB+XXNf3Xr4wvEMq8K0inIGYmY7T2PtM7o3vDqr3yYPhMAVgD0LD5YI0qW+grqKN5CrnG8J4ZbgUV4MT+pHD/JmgyO+IDQyG09Sw6sFiVP+H6cpP89f5OeJjm3Y1OBXbkDvgP9e0AzXjBowldJ+3xYD1csM3zF0wKcA04BG4FHgU9qrZsDfHTJh1TWcWGjpqQyqbLRBt84Un75jDkMOtNEkLm9gi5Dlm0tyGz+uJHSZMovq0otps4dx/kasAHo0lrXXqn2Ek5XPM59WFeLFeFrG+tuaaYYtVzjM7Plm2kGzUzAC1rGXCtvB4VUzplDR+FGn++/Lh+t9hIUXiGkrg67ZubXGQcgCzKgWhXwmu+889ngkgtKEuDaMYGVP3PAHcALeI7nyVrr3vD2DO2aBdY4hKAKAK5CIJar7iSA0soFo7wWDgnVSWghsK4tuKk89gtd0adQcAVVUiGsQguBdZ1CK1+IFVNXUsR+VlCFsAotBNb1B61cUFJF1pfkcZzXnPshrEILgRWCK980+QIrqIIaEUYhrEILgXX9QSsfKBVTT5LH+yGoQguBFUKrqHoYrTqSAs+FsApt3Nj/AalKUeEK7JTpAAAAAElFTkSuQmCC\"/></html>";
    f_logo_label->setText(f_logo_text);
    f_logo_label->setMinimumSize(90, 30);   
    f_logo_label->show();
    layout_row3->addWidget(f_logo_label, -1, Qt::AlignRight);
        
        
    /*IMPORTANT:  Adding the last row, don't delete this*/
    
    layout->addLayout(layout_row3, -1);
        
    /*End test button code, DO NOT remove the code below this*/

    QTimer *myTimer = new QTimer(this);
    connect(myTimer, SIGNAL(timeout()), this, SLOT(oscRecv()));
    myTimer->setSingleShot(false);
    myTimer->start(0);
    
    m_suppressHostUpdate = false;
        
    //Load the first preset    
    //EDIT:  commenting this out, as it actually causes the first preset to be loaded everytime you open
    //the plugin GUI in a host.
    /*setProgram(0);   
    programChanged(0);*/
       
        
}

/*I'm leaving this in here for now, but at the present it doesn't work*/
void SynthGUI::v_add_knob_to_layout(QDial * a_knob, e_knob_type a_knob_type, int a_default_value, QLabel * a_label, QGridLayout * a_layout, QString a_title,
int a_gb_layout_column, int a_gb_layout_row, const char * a_signal, const char * a_slot)
{
    a_knob = get_knob(a_knob_type, a_default_value);
    a_label  = newQLabel(this);
    add_widget(a_layout, a_gb_layout_column, a_gb_layout_row, a_title,a_knob, a_label);
    connect(a_knob,  a_signal, this, a_slot);
    //masterPitchbendAmtChanged (m_master_pitchbend_amt ->value());
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

QDial * SynthGUI::get_knob(e_knob_type a_ktype, int a_default_value)
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
        case decibels_30_to_0:
            f_min = -30; f_max = 0; f_step = 1; f_value = -9;            
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
    m_attack->setValue(int(sec));
    m_suppressHostUpdate = false;
}

void SynthGUI::setDecay(float sec)
{
    m_suppressHostUpdate = true;
    m_decay->setValue(int(sec));
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
    m_release->setValue(int(sec));
    m_suppressHostUpdate = false;
}

void SynthGUI::setTimbre(float val)
{
    m_suppressHostUpdate = true;
    m_timbre->setValue(int(val));
    m_suppressHostUpdate = false;
}

void SynthGUI::setRes(float val)
{
    m_suppressHostUpdate = true;
    m_res->setValue(int(val));
    m_suppressHostUpdate = false;
}

void SynthGUI::setDist(float val)
{
    m_suppressHostUpdate = true;
    m_dist->setValue(int(val)); 
    m_suppressHostUpdate = false;
}

void SynthGUI::setFilterAttack (float sec)
{    
    m_suppressHostUpdate = true;
    m_filter_attack->setValue(int(sec));
    m_suppressHostUpdate = false;
}

void SynthGUI::setFilterDecay  (float sec)
{
    m_suppressHostUpdate = true;
    m_filter_decay->setValue(int(sec));
    m_suppressHostUpdate = false;
}

void SynthGUI::setFilterSustain(float val)
{
    m_suppressHostUpdate = true;
    m_filter_sustain->setValue(int(val));
    m_suppressHostUpdate = false;
}

void SynthGUI::setFilterRelease(float sec)
{
    m_suppressHostUpdate = true;
    m_filter_release->setValue(int(sec));
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
    m_dist_wet->setValue(int(val));
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
    m_osc1_tune->setValue(int(val));
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
    m_osc2_tune->setValue(int(val));
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

void SynthGUI::setPitchEnvAmt(float val)
{
    m_suppressHostUpdate = true;
    m_pitch_env_amt->setValue(int(val));
    m_suppressHostUpdate = false;
}


void SynthGUI::setPitchEnvTime(float val)
{
    m_suppressHostUpdate = true;
    m_pitch_env_time->setValue(int(val));
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
#ifdef LMS_DEBUG_MODE_QT
    cerr << "setProgram called with val: " << val << endl;
#endif
    m_suppressHostUpdate = true;
    m_program->setCurrentIndex(val);
    m_suppressHostUpdate = false;
}


/*Standard handlers for the audio slots, these perform manipulations of knob values
 that are common in audio applications*/

void SynthGUI::changed_zero_to_x(int a_value, QLabel * a_label, int a_port)
{
    float val = float(a_value) * .01;
    a_label->setText(QString("%1").arg(val));
    
    if (!m_suppressHostUpdate) {
	lo_send(m_host, m_controlPath, "if", a_port, float(a_value));     
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
	lo_send(m_host, m_controlPath, "if", a_port, float(a_value));
    }
}

void SynthGUI::changed_pitch(int a_value, QLabel * a_label, int a_port)
{
    /*We need to send midi note number to the synth, as it probably still needs to process it as
     midi_note number.  We use this to display hz to the user*/
    
    float f_value = float(a_value);
    float f_hz = f_pit_midi_note_to_hz_fast(f_value);
    
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
	lo_send(m_host, m_controlPath, "if", LTS_PORT_DIST_WET, (float(value)));
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

void SynthGUI::pitchEnvAmtChanged(int value)
{
    changed_integer(value, m_pitch_env_amtLabel, LTS_PORT_PITCH_ENV_AMT);
}

void SynthGUI::pitchEnvTimeChanged(int value)
{
    changed_seconds(value, m_pitch_env_timeLabel, LTS_PORT_PITCH_ENV_TIME);
}

/*
void SynthGUI::bankChanged(int value)
{
    cerr << "Bank change not yet implemented.  Bank# " << value << endl;
}
*/

void SynthGUI::programChanged(int value)
{    
    int f_adjusted_value = 0;
    
    if(value <= 127)
        f_adjusted_value = value;
    
    if(presets_tab_delimited[f_adjusted_value].compare("empty") != 0)
    {
        QStringList f_preset_values = presets_tab_delimited[f_adjusted_value].split("\t");
        //TODO:  change f_i back to zero when there is something at that index
        for(int f_i = 1; f_i < LTS_PORT_PROGRAM_CHANGE; f_i++)
        {
            if(f_i > f_preset_values.count())
            {
                cerr << "programChanged:  f_i is greater than f_preset_values.count(), preset not fully loaded.\n";
                break;
            }
            /*TODO:  Some error handling here to prevent index-out-of-bounds exceptions from crashing the GUI*/
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
        for(int f_i = 1; f_i < LTS_PORT_PROGRAM_CHANGE; f_i++)
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

void SynthGUI::v_print_port_name_to_cerr(int a_port)
{
    switch (a_port) {
    case LTS_PORT_ATTACK:
	cerr << "LTS_PORT_ATTACK";
	break;
    case LTS_PORT_DECAY:
	cerr << "LTS_PORT_DECAY";
	break;
    case LTS_PORT_SUSTAIN:
	cerr << "LTS_PORT_SUSTAIN";
	break;
    case LTS_PORT_RELEASE:
	cerr << "LTS_PORT_RELEASE";
	break;
    case LTS_PORT_TIMBRE:
	cerr << "LTS_PORT_TIMBRE";
	break;
    case LTS_PORT_RES:
	cerr << "LTS_PORT_RES";
	break;        
    case LTS_PORT_DIST:
	cerr << "LTS_PORT_DIST";
	break;
    case LTS_PORT_FILTER_ATTACK:
	cerr << "LTS_PORT_FILTER_ATTACK";
	break;
    case LTS_PORT_FILTER_DECAY:
	cerr << "LTS_PORT_FILTER_DECAY";
	break;
    case LTS_PORT_FILTER_SUSTAIN:
	cerr << "LTS_PORT_FILTER_SUSTAIN";
	break;
    case LTS_PORT_FILTER_RELEASE:
	cerr << "LTS_PORT_FILTER_RELEASE";
	break;
    case LTS_PORT_NOISE_AMP:
        cerr << "LTS_PORT_NOISE_AMP";
        break;    
    case LTS_PORT_DIST_WET:
        cerr << "LTS_PORT_DIST_WET";
        break;            
    case LTS_PORT_FILTER_ENV_AMT:
        cerr << "LTS_PORT_FILTER_ENV_AMT";
        break;    
    case LTS_PORT_OSC1_TYPE:
        cerr << "LTS_PORT_OSC1_TYPE";
        break;            
    case LTS_PORT_OSC1_PITCH:
        cerr << "LTS_PORT_OSC1_PITCH";
        break;    
    case LTS_PORT_OSC1_TUNE:
        cerr << "LTS_PORT_OSC1_TUNE";
        break;    
    case LTS_PORT_OSC1_VOLUME:
        cerr << "LTS_PORT_OSC1_VOLUME";
        break;        
    case LTS_PORT_OSC2_TYPE:
        cerr << "LTS_PORT_OSC2_TYPE";
        break;            
    case LTS_PORT_OSC2_PITCH:
        cerr << "LTS_PORT_OSC2_PITCH";
        break;    
    case LTS_PORT_OSC2_TUNE:
        cerr << "LTS_PORT_OSC2_TUNE";
        break;    
    case LTS_PORT_OSC2_VOLUME:
        cerr << "LTS_PORT_OSC2_VOLUME";
        break;        
    case LTS_PORT_MASTER_VOLUME:
        cerr << "LTS_PORT_MASTER_VOLUME";
        break;        
    case LTS_PORT_MASTER_UNISON_VOICES:
        cerr << "LTS_PORT_MASTER_UNISON_VOICES";
        break;
    case LTS_PORT_MASTER_UNISON_SPREAD:
        cerr << "LTS_PORT_MASTER_UNISON_SPREAD";
        break;
    case LTS_PORT_MASTER_GLIDE:
        cerr << "LTS_PORT_MASTER_GLIDE";
        break;
    case LTS_PORT_MASTER_PITCHBEND_AMT:
        cerr << "LTS_PORT_MASTER_PITCHBEND_AMT";
        break;
    case LTS_PORT_PITCH_ENV_AMT:
        cerr << "LTS_PORT_PITCH_ENV_AMT ";
        break;
    case LTS_PORT_PITCH_ENV_TIME:
        cerr << "LTS_PORT_PITCH_ENV_TIME ";
        break;        
    case LTS_PORT_PROGRAM_CHANGE:
        cerr << "LTS_PORT_PROGRAM_CHANGE ";
        break;
    default:
	cerr << "Warning: received request to set nonexistent port " << a_port ;
    }
}

void SynthGUI::v_set_control(int a_port, float a_value)
{

#ifdef LMS_DEBUG_MODE_QT    
    cerr << "v_set_control called.  ";  
    v_print_port_name_to_cerr(a_port);
    cerr << "  value: " << a_value << endl;
#endif
    
    /*GUI Step 8:  Add the controls you created to the control handler*/
    
    switch (a_port) {
    case LTS_PORT_ATTACK:
	setAttack(a_value);
	break;
    case LTS_PORT_DECAY:
	setDecay(a_value);
	break;
    case LTS_PORT_SUSTAIN:
	setSustain(a_value);
	break;
    case LTS_PORT_RELEASE:
	setRelease(a_value);
	break;
    case LTS_PORT_TIMBRE:
	setTimbre(a_value);
	break;
    case LTS_PORT_RES:
	setRes(a_value);
	break;        
    case LTS_PORT_DIST:
	setDist(a_value);
	break;
    case LTS_PORT_FILTER_ATTACK:
	setFilterAttack(a_value);
	break;
    case LTS_PORT_FILTER_DECAY:
	setFilterDecay(a_value);
	break;
    case LTS_PORT_FILTER_SUSTAIN:
	setFilterSustain(a_value);
	break;
    case LTS_PORT_FILTER_RELEASE:
	setFilterRelease(a_value);
	break;
    case LTS_PORT_NOISE_AMP:
        setNoiseAmp(a_value);
        break;    
    case LTS_PORT_DIST_WET:
        setDistWet(a_value);
        break;            
    case LTS_PORT_FILTER_ENV_AMT:
        setFilterEnvAmt(a_value);
        break;    
    case LTS_PORT_OSC1_TYPE:
        setOsc1Type(a_value);
        break;            
    case LTS_PORT_OSC1_PITCH:
        setOsc1Pitch(a_value);
        break;    
    case LTS_PORT_OSC1_TUNE:
        setOsc1Tune(a_value);
        break;    
    case LTS_PORT_OSC1_VOLUME:
        setOsc1Volume(a_value);
        break;        
    case LTS_PORT_OSC2_TYPE:
        setOsc2Type(a_value);
        break;            
    case LTS_PORT_OSC2_PITCH:
        setOsc2Pitch(a_value);
        break;    
    case LTS_PORT_OSC2_TUNE:
        setOsc2Tune(a_value);
        break;    
    case LTS_PORT_OSC2_VOLUME:
        setOsc2Volume(a_value);
        break;        
    case LTS_PORT_MASTER_VOLUME:
        setMasterVolume(a_value);
        break;    
    case LTS_PORT_MASTER_UNISON_VOICES:
        setMasterUnisonVoices(a_value);
        break;
    case LTS_PORT_MASTER_UNISON_SPREAD:
        setMasterUnisonSpread(a_value);
        break;
    case LTS_PORT_MASTER_GLIDE:
        setMasterGlide(a_value);
        break;
    case LTS_PORT_MASTER_PITCHBEND_AMT:
        setMasterPitchbendAmt(a_value);
        break;
    case LTS_PORT_PITCH_ENV_AMT:
        setPitchEnvAmt(a_value);
        break;
    case LTS_PORT_PITCH_ENV_TIME:
        setPitchEnvTime(a_value);
        break;            
    case LTS_PORT_PROGRAM_CHANGE:
        setProgram(a_value);
        break;
    }
}

void SynthGUI::v_control_changed(int a_port, int a_value)
{
    
#ifdef LMS_DEBUG_MODE_QT    
    cerr << "v_control_changed called.  ";  
    v_print_port_name_to_cerr(a_port);
    cerr << "  value: " << a_value << endl;
#endif
    
       /*GUI Step 9:  Add the controls you created to the control handler*/
    
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
        distWetChanged(a_value);
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
    case LTS_PORT_PITCH_ENV_AMT:
        pitchEnvAmtChanged(a_value);
        break;
    case LTS_PORT_PITCH_ENV_TIME:
        pitchEnvTimeChanged(a_value);
        break;
    case LTS_PORT_PROGRAM_CHANGE:
        programChanged(a_value);
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
        /*GUI Step 10:  Add the controls you created to the control handler
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
    case LTS_PORT_PITCH_ENV_AMT:
        return m_pitch_env_amt->value();
    case LTS_PORT_PITCH_ENV_TIME:
        return m_pitch_env_time->value();
    case LTS_PORT_PROGRAM_CHANGE:
        return m_program->currentIndex();
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

#ifdef LMS_DEBUG_MODE_QT
    cerr << "control_handler called.  port:  " << port << " , value " << value << endl;
#endif
    
    gui->v_set_control(port, value);  
    gui->v_control_changed(port, value);

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

