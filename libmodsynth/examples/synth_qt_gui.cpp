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
//Comment this out when you compile a stable release version on the plugin
#define LMS_DEBUG_MODE_QT

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

using std::endl;

/*GUI Step 6:  Define ports for each control that will send messages to the plugin, such as dials/knobs or faders.*/
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
    
    QGridLayout *layout = new QGridLayout(this);
    //QVBoxLayout *layout = new QVBoxLayout();
        
    int _row = 0;
    int _column = 0;
    
    int _gb_layout_row = 0;
    int _gb_layout_column = 0;
    /*GUI Step 4:  Lay out the controls you declared in the first step*/
    
    /*The oscillator1 GroupBox*/
    QGroupBox * _gb_osc1 = _newGroupBox("Osc1", this);
    QGridLayout *_gb_osc1_layout = new QGridLayout(_gb_osc1);
    
    m_osc1_pitch = _get_knob(minus12_to_12);
    m_osc1_pitchLabel = _newQLabel(this);
    _add_widget(_gb_osc1_layout, _gb_layout_column, _gb_layout_row, "Pitch", m_osc1_pitch, m_osc1_pitchLabel);
    connect(m_osc1_pitch, SIGNAL(valueChanged(int)), this, SLOT(osc1PitchChanged(int)));
    osc1PitchChanged(m_osc1_pitch->value());
    
    _gb_layout_column++;
    
    m_osc1_tune = _get_knob(minus1_to_1);
    m_osc1_tuneLabel = _newQLabel(this);
    _add_widget(_gb_osc1_layout, _gb_layout_column, _gb_layout_row, "Tune", m_osc1_tune, m_osc1_tuneLabel);
    connect(m_osc1_tune, SIGNAL(valueChanged(int)), this, SLOT(osc1TuneChanged(int)));
    osc1TuneChanged(m_osc1_tune->value());
    
    _gb_layout_column++;
    
    m_osc1_volume = _get_knob(decibels_0, -6);
    m_osc1_volumeLabel = _newQLabel(this);
    _add_widget(_gb_osc1_layout, _gb_layout_column, _gb_layout_row, "Vol", m_osc1_volume, m_osc1_volumeLabel);
    connect(m_osc1_volume, SIGNAL(valueChanged(int)), this, SLOT(osc1VolumeChanged(int)));
    osc1VolumeChanged(m_osc1_volume->value());
    
    _gb_layout_column++;
    
    m_osc1_type = _get_combobox(_osc_types, _osc_types_count , this);     
    _add_widget_no_label(_gb_osc1_layout, _gb_layout_column, _gb_layout_row, "Type", m_osc1_type);
    connect(m_osc1_type, SIGNAL(currentIndexChanged(int)), this, SLOT(osc1TypeChanged(int)));
    osc1TypeChanged(m_osc1_type->currentIndex());
    
    layout->addWidget(_gb_osc1, _row, _column, Qt::AlignCenter); 
    _gb_layout_row = 0;
    _gb_layout_column = 0;
    _column++;
    
    /*The amplitude ADSR GroupBox*/
    QGroupBox * _gb_adsr = _newGroupBox("ADSR Amp", this);
    QGridLayout *_gb_adsr_layout = new QGridLayout(_gb_adsr);
    
    
    
    m_attack = _get_knob(zero_to_one);
    m_attackLabel = _newQLabel(this);
    _add_widget(_gb_adsr_layout, _gb_layout_column, _gb_layout_row, "Attack",m_attack, m_attackLabel);
    connect(m_attack,   SIGNAL(valueChanged(int)), this, SLOT(attackChanged(int)));
    attackChanged  (m_attack  ->value());
    
    _gb_layout_column++;
        
    m_decay   =  _get_knob(zero_to_one); //newQDial(  1, 100,  1,  25); // s * 100
    m_decayLabel   = _newQLabel(this);
    _add_widget(_gb_adsr_layout, _gb_layout_column, _gb_layout_row, "Decay",m_decay, m_decayLabel);
    connect(m_decay,   SIGNAL(valueChanged(int)), this, SLOT(decayChanged(int)));
    decayChanged  (m_decay  ->value());
    
    _gb_layout_column++;
    
    m_sustain =  _get_knob(decibels_0); // newQDial(  0, 100,  1,  75); // %
    m_sustainLabel = _newQLabel(this);
    _add_widget(_gb_adsr_layout, _gb_layout_column, _gb_layout_row, "Sustain", m_sustain, m_sustainLabel);    
    connect(m_sustain, SIGNAL(valueChanged(int)), this, SLOT(sustainChanged(int)));
    sustainChanged(m_sustain->value());
    
    _gb_layout_column++;
    
    m_release = _get_knob(zero_to_four); //newQDial(  1, 400, 10, 200); // s * 100
    m_releaseLabel = _newQLabel(this);
    _add_widget(_gb_adsr_layout, _gb_layout_column, _gb_layout_row, "Release", m_release, m_releaseLabel);
    connect(m_release, SIGNAL(valueChanged(int)), this, SLOT(releaseChanged(int)));
    releaseChanged(m_release->value());
    
    layout->addWidget(_gb_adsr, _row, _column, Qt::AlignCenter);        
    _column++;
    _gb_layout_row = 0;
    _gb_layout_column = 0;
        
    
    /*The Distortion GroupBox*/
    QGroupBox * _gb_dist = _newGroupBox("Distortion", this);    
    QGridLayout *_gb_dist_layout = new QGridLayout(_gb_dist);
    
    
    m_dist  = newQDial(  -6, 36,  1,  -6); 
    m_distLabel  = new QLabel(this);
    _add_widget(_gb_dist_layout, _gb_layout_column, _gb_layout_row, "Gain", m_dist, m_distLabel);
    connect(m_dist,  SIGNAL(valueChanged(int)), this, SLOT(distChanged(int)));
    distChanged (m_dist ->value());
    
    _gb_layout_column++;
    
    m_dist_wet  = _get_knob(zero_to_one);    
    _add_widget_no_label(_gb_dist_layout, _gb_layout_column, _gb_layout_row, "Wet", m_dist_wet);
    connect(m_dist_wet,  SIGNAL(valueChanged(int)), this, SLOT(distWetChanged(int)));
    distWetChanged (m_dist_wet ->value());
    
    layout->addWidget(_gb_dist, _row, _column, Qt::AlignCenter);    
    _column++;
    _gb_layout_row = 0;
    _gb_layout_column = 0;
    
    
    
    /*The Noise Amp GroupBox*/
    QGroupBox * _gb_noise_amp = _newGroupBox("Noise", this);    
    QGridLayout *_gb_noise_amp_layout = new QGridLayout(_gb_noise_amp);
    
    
    m_noise_amp  = _get_knob(decibels_0);
    m_noise_ampLabel  = new QLabel(this);
    _add_widget(_gb_noise_amp_layout, _column, _row, "Vol", m_noise_amp, m_noise_ampLabel);
    connect(m_noise_amp,  SIGNAL(valueChanged(int)), this, SLOT(noiseAmpChanged(int)));
    noiseAmpChanged (m_dist ->value());
    
    
    layout->addWidget(_gb_noise_amp, _row, _column, Qt::AlignCenter);            
    /*Start a new row*/
    _row++;
    _column = 0;
    
    
    /*The oscillator2 GroupBox*/
    QGroupBox * _gb_osc2 = _newGroupBox("Osc2", this);
    QGridLayout *_gb_osc2_layout = new QGridLayout(_gb_osc2);
    
    m_osc2_pitch = _get_knob(minus12_to_12);
    m_osc2_pitchLabel = _newQLabel(this);
    _add_widget(_gb_osc2_layout, _gb_layout_column, _gb_layout_row, "Pitch", m_osc2_pitch, m_osc2_pitchLabel);
    connect(m_osc2_pitch, SIGNAL(valueChanged(int)), this, SLOT(osc2PitchChanged(int)));
    osc2PitchChanged(m_osc2_pitch->value());
            
    _gb_layout_column++;
    
    m_osc2_tune = _get_knob(minus1_to_1);
    m_osc2_tuneLabel = _newQLabel(this);
    _add_widget(_gb_osc2_layout, _gb_layout_column, _gb_layout_row, "Tune", m_osc2_tune, m_osc2_tuneLabel);
    connect(m_osc2_tune, SIGNAL(valueChanged(int)), this, SLOT(osc2TuneChanged(int)));
    osc2TuneChanged(m_osc2_tune->value());
    
    _gb_layout_column++;
    
    m_osc2_volume = _get_knob(decibels_0, -60);
    m_osc2_volumeLabel = _newQLabel(this);
    _add_widget(_gb_osc2_layout, _gb_layout_column, _gb_layout_row, "Vol", m_osc2_volume, m_osc2_volumeLabel);
    connect(m_osc2_volume, SIGNAL(valueChanged(int)), this, SLOT(osc2VolumeChanged(int)));
    osc2VolumeChanged(m_osc2_volume->value());
    
    _gb_layout_column++;
    
    m_osc2_type = _get_combobox(_osc_types, _osc_types_count , this);     
    _add_widget_no_label(_gb_osc2_layout, _gb_layout_column, _gb_layout_row, "Type", m_osc2_type);
    connect(m_osc2_type, SIGNAL(currentIndexChanged(int)), this, SLOT(osc2typeChanged(int)));
    osc2TypeChanged(m_osc2_type->currentIndex());
    
    
    layout->addWidget(_gb_osc2, _row, _column, Qt::AlignCenter);        
    _gb_layout_row = 0;
    _gb_layout_column = 0;
    _column++;
    
    /*The filter ADSR GroupBox*/
    QGroupBox * _gb_adsr_f = _newGroupBox("ADSR Filter", this);
    QGridLayout *_gb_adsr_f_layout = new QGridLayout(_gb_adsr_f);
    
    _gb_layout_row = 0;
    _gb_layout_column = 0;
    
    m_filter_attack = _get_knob(zero_to_one);
    m_filter_attackLabel = _newQLabel(this);
    _add_widget(_gb_adsr_f_layout, _gb_layout_column, _gb_layout_row, "Attack",m_filter_attack, m_filter_attackLabel);
    connect(m_filter_attack,   SIGNAL(valueChanged(int)), this, SLOT(filterAttackChanged(int)));
    filterAttackChanged  (m_filter_attack  ->value());
    
    _gb_layout_column++;
        
    m_filter_decay   =  _get_knob(zero_to_one); //newQDial(  1, 100,  1,  25); // s * 100
    m_filter_decayLabel   = _newQLabel(this);
    _add_widget(_gb_adsr_f_layout, _gb_layout_column, _gb_layout_row, "Decay",m_filter_decay, m_filter_decayLabel);
    connect(m_filter_decay,   SIGNAL(valueChanged(int)), this, SLOT(filterDecayChanged(int)));
    filterDecayChanged  (m_filter_decay ->value());
    
    _gb_layout_column++;
    
    m_filter_sustain =  _get_knob(zero_to_one); // newQDial(  0, 100,  1,  75); // %
    m_filter_sustainLabel = _newQLabel(this);
    _add_widget(_gb_adsr_f_layout, _gb_layout_column, _gb_layout_row, "Sustain", m_filter_sustain, m_filter_sustainLabel);
    connect(m_filter_sustain, SIGNAL(valueChanged(int)), this, SLOT(filterSustainChanged(int)));
    filterSustainChanged(m_filter_sustain->value());
    
    _gb_layout_column++;
    
    m_filter_release = _get_knob(zero_to_four); //newQDial(  1, 400, 10, 200); // s * 100
    m_filter_releaseLabel = _newQLabel(this);
    _add_widget(_gb_adsr_f_layout, _gb_layout_column, _gb_layout_row, "Release", m_filter_release, m_filter_releaseLabel);
    connect(m_filter_release, SIGNAL(valueChanged(int)), this, SLOT(filterReleaseChanged(int)));    
    filterReleaseChanged(m_filter_release->value());
        
    
    layout->addWidget(_gb_adsr_f, _row, _column, Qt::AlignCenter);        
    _gb_layout_row = 0;
    _gb_layout_column = 0;
    _column++;
    
    
    /*The Filter GroupBox*/
    QGroupBox * _gb_filter = _newGroupBox("LP Filter", this); 
    QGridLayout *_gb_filter_layout = new QGridLayout(_gb_filter);
    
    m_timbre  =  _get_knob(pitch);  //newQDial(  39, 136,  1,  82); // s * 100
    m_timbreLabel  = _newQLabel(this);
    _add_widget(_gb_filter_layout, _gb_layout_column, _gb_layout_row, "Timbre",m_timbre, m_timbreLabel);
    connect(m_timbre,  SIGNAL(valueChanged(int)), this, SLOT(timbreChanged(int)));
    timbreChanged (m_timbre ->value());
    
    _gb_layout_column++;
    
    m_res  =  _get_knob(decibels_0); 
    m_resLabel  = _newQLabel(this);
    _add_widget(_gb_filter_layout, _gb_layout_column, _gb_layout_row, "Res", m_res, m_resLabel);
    connect(m_res,  SIGNAL(valueChanged(int)), this, SLOT(resChanged(int)));
    resChanged (m_res ->value());
    
    _gb_layout_column++;
    
    m_filter_env_amt  =  _get_knob(minus36_to_36); 
    m_filter_env_amtLabel  = _newQLabel(this);
    _add_widget(_gb_filter_layout, _gb_layout_column, _gb_layout_row, "Env", m_filter_env_amt, m_filter_env_amtLabel);
    connect(m_filter_env_amt,  SIGNAL(valueChanged(int)), this, SLOT(filterEnvAmtChanged(int)));
    filterEnvAmtChanged (m_filter_env_amt ->value());
    
    
    layout->addWidget(_gb_filter, _row, _column, Qt::AlignCenter);    
    _column++;
    _gb_layout_row = 0;
    _gb_layout_column = 0;
    
    
    /*The Master Volume GroupBox*/
    QGroupBox * _gb_master_vol = _newGroupBox("Master", this); 
    QGridLayout *_gb_master_vol_layout = new QGridLayout(_gb_master_vol);
    
    m_master_volume  =  _get_knob(decibels_plus_12);
    m_master_volumeLabel  = _newQLabel(this);
    _add_widget(_gb_master_vol_layout, _gb_layout_column, _gb_layout_row, "Volume",m_master_volume, m_master_volumeLabel);
    connect(m_master_volume,  SIGNAL(valueChanged(int)), this, SLOT(masterVolumeChanged(int)));
    masterVolumeChanged (m_master_volume ->value());
    
    _gb_layout_column++;
    
    
    /*Begin new stuff*/
    
    m_master_unison_voices  =  newQDial(1, 7, 1, 1);
    m_master_unison_voicesLabel  = _newQLabel(this);
    _add_widget(_gb_master_vol_layout, _gb_layout_column, _gb_layout_row, "Unison",m_master_unison_voices, m_master_unison_voicesLabel);
    connect(m_master_unison_voices,  SIGNAL(valueChanged(int)), this, SLOT( masterUnisonVoicesChanged(int)));
    masterUnisonVoicesChanged (m_master_unison_voices ->value());
    
    _gb_layout_column++;
    
    m_master_unison_spread  =  _get_knob(zero_to_one);
    m_master_unison_spreadLabel  = _newQLabel(this);
    _add_widget(_gb_master_vol_layout, _gb_layout_column, _gb_layout_row, "Spread",m_master_unison_spread, m_master_unison_spreadLabel);
    connect(m_master_unison_spread,  SIGNAL(valueChanged(int)), this, SLOT(masterUnisonSpreadChanged(int)));
    masterUnisonSpreadChanged (m_master_unison_spread ->value());
    
    _gb_layout_column++;
    
    m_master_glide  =  _get_knob(zero_to_one);
    m_master_glideLabel  = _newQLabel(this);
    _add_widget(_gb_master_vol_layout, _gb_layout_column, _gb_layout_row, "Glide",m_master_glide, m_master_glideLabel);
    connect(m_master_glide,  SIGNAL(valueChanged(int)), this, SLOT(masterGlideChanged(int)));
    masterGlideChanged (m_master_glide ->value());
    
    _gb_layout_column++;
    
    m_master_pitchbend_amt  =  newQDial(1, 36, 1, 2);
    m_master_pitchbend_amtLabel  = _newQLabel(this);
    _add_widget(_gb_master_vol_layout, _gb_layout_column, _gb_layout_row, "Pitchbend",m_master_pitchbend_amt, m_master_pitchbend_amtLabel);
    connect(m_master_pitchbend_amt,  SIGNAL(valueChanged(int)), this, SLOT(masterPitchbendAmtChanged(int)));
    masterPitchbendAmtChanged (m_master_pitchbend_amt ->value());
    
    _gb_layout_column++;
    
    
    /*End new stuff*/
    
    layout->addWidget(_gb_master_vol, _row, _column, Qt::AlignCenter);    
    _column++;
    _gb_layout_row = 0;
    _gb_layout_column = 0;
    
    
    
    /*This is the test button for playing a note without a keyboard, you should remove this before distributing your plugin*/
    
    QPushButton *testButton = new QPushButton("Test", this);
    connect(testButton, SIGNAL(pressed()), this, SLOT(test_press()));
    connect(testButton, SIGNAL(released()), this, SLOT(test_release()));    
    /*This adds the test button below the last column*/
    layout->addWidget(testButton, (_row + 4), _column, Qt::AlignCenter);
    
    /*End test button code, DO NOT remove the code below this*/

    QTimer *myTimer = new QTimer(this);
    connect(myTimer, SIGNAL(timeout()), this, SLOT(oscRecv()));
    myTimer->setSingleShot(false);
    myTimer->start(0);
    
    m_suppressHostUpdate = false;
}



void SynthGUI::_add_widget(QGridLayout * _layout, int position_x, int position_y, QString _label_text,  QWidget * _widget,
    QLabel * _label)
{   
    QLabel * _knob_title = new QLabel(_label_text,  this);
    _knob_title->setMinimumWidth(60);  //TODO:  make this a constant
    _knob_title->setAlignment(Qt::AlignCenter);
    _knob_title->setStyleSheet("background-color: white; border: 1px solid black;  border-radius: 6px;");  //TODO:  make this a constant string for all knobs
    
    _layout->addWidget(_knob_title, position_y, position_x, Qt::AlignCenter);    
    _layout->addWidget(_widget,  (position_y + 1), position_x);
    _layout->addWidget(_label,  (position_y + 2), position_x, Qt::AlignCenter);     
}

void SynthGUI::_add_widget_no_label(QGridLayout * _layout, int position_x, int position_y, QString _label_text, QWidget * _widget)
{
    QLabel * _knob_title = new QLabel(_label_text,  this);
    _knob_title->setMinimumWidth(60);  //TODO:  make this a constant
    _knob_title->setAlignment(Qt::AlignCenter);
    _knob_title->setStyleSheet("background-color: white; border: 1px solid black;  border-radius: 6px;");    //TODO:  make this a constant string for all knobs
    
    _layout->addWidget(_knob_title, position_y, position_x, Qt::AlignCenter);    
    _layout->addWidget(_widget,  (position_y + 1), position_x);    
}

QGroupBox * SynthGUI::_newGroupBox(QString _title, QWidget * _parent)
{
    QGroupBox * _result = new QGroupBox(_parent);
    
    _result->setTitle(_title);
    _result->setAlignment(Qt::AlignHCenter);
    return _result;
}

QLabel * SynthGUI::_newQLabel(QWidget * _parent)
{
    QLabel * _result = new QLabel(_parent);
    //_result->setStyleSheet("background-color: white; border: 2px solid black;  border-radius: 6px;");
    return _result;
}

QDial * SynthGUI::_get_knob(_knob_type _ktype, int _default_value)
{
    int _min, _max, _step, _value;
    
    switch(_ktype)
    {
        case decibels_0:
                _min = -60; _max = 0; _step = 1; _value = -6; 
                break;
        case decibels_plus_12:
            _min = -60; _max = 12; _step = 1; _value = -6;            
            break;
        case decibels_plus_24:
            _min = -60; _max = 24; _step = 1; _value = -6;            
            break;
        case decibels_plus_6:            
            _min = -60; _max = 6; _step = 1; _value = -6;            
            break;
        case pitch:
            _min = 20; _max = 124; _step = 1; _value = 105;            
            break;
        case zero_to_four:
            _min = 1; _max = 400; _step = 4; _value = 75;            
            break;
        case zero_to_one:
            _min = 1; _max = 100; _step = 1; _value = 15;            
            break;
        case zero_to_two:
            _min = 1; _max = 200; _step = 2; _value = 25;            
            break;
        case minus1_to_1:
            _min = -100; _max = 100; _step = 1; _value = 0;            
            break;
        case minus12_to_12:
            _min = -12; _max = 12; _step = 1; _value = 0;            
            break;
        case minus24_to_24:
            _min = -24; _max = 24; _step = 1; _value = 0;            
            break;
        case minus36_to_36:
            _min = -36; _max = 36; _step = 1; _value = 0;            
            break;
    }
    
    if(_default_value != 333)  //This makes the assumption that we will never pick 333 as a default value
    {
        _value = _default_value;
    }
    
     return newQDial(_min, _max, _step, _value);
    
}

QCheckBox * SynthGUI::_get_checkbox(std::string _text)
{
    QCheckBox * _checkbox = new QCheckBox(this);
    
    _checkbox->setText(QString::fromStdString(_text));
    
    //TODO:  add a skin to make it look like a toggle-switch
        
    return _checkbox;
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
    
    //dial->setFocusPolicy(Qt::NoFocus);
    
    return dial;
}

QComboBox * SynthGUI::_get_combobox(QString _choices [], int _count,  QWidget * _parent)
{
    QComboBox * _result = new QComboBox(_parent);
    QStringList _items;
    
    for(int i = 0; i < _count; i++)
    {
        _items << _choices[i];
    }
    
    _result->addItems(_items);
    
    return _result;
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
    m_noise_amp->setValue(val);
    m_suppressHostUpdate = false;
}

void SynthGUI::setFilterEnvAmt(float val)
{
    m_suppressHostUpdate = true;
    m_filter_env_amt->setValue(val);
    m_suppressHostUpdate = false;
}

void SynthGUI::setDistWet(float val)
{
    m_suppressHostUpdate = true;
    m_dist_wet->setValue(val * 100);
    m_suppressHostUpdate = false;
}

void SynthGUI::setOsc1Type(float val)
{
    m_suppressHostUpdate = true;
    m_osc1_type->setCurrentIndex((int)val);
    m_suppressHostUpdate = false;
}

void SynthGUI::setOsc1Pitch(float val)
{
    m_suppressHostUpdate = true;
    m_osc1_pitch->setValue(val);
    m_suppressHostUpdate = false;
}

void SynthGUI::setOsc1Tune(float val)
{
    m_suppressHostUpdate = true;
    m_osc1_tune->setValue(val * 100);
    m_suppressHostUpdate = false;
}

void SynthGUI::setOsc1Volume(float val)
{
    m_suppressHostUpdate = true;
    m_osc1_volume->setValue(val);
    m_suppressHostUpdate = false;
}

void SynthGUI::setOsc2Type(float val)
{
    m_suppressHostUpdate = true;
    m_osc2_type->setCurrentIndex((int)val);
    m_suppressHostUpdate = false;
}

void SynthGUI::setOsc2Pitch(float val)
{
    m_suppressHostUpdate = true;
    m_osc2_pitch->setValue(val);
    m_suppressHostUpdate = false;
}

void SynthGUI::setOsc2Tune(float val)
{
    m_suppressHostUpdate = true;
    m_osc2_tune->setValue(val * 100);
    m_suppressHostUpdate = false;
}

void SynthGUI::setOsc2Volume(float val)
{
    m_suppressHostUpdate = true;
    m_osc2_volume->setValue(val);
    m_suppressHostUpdate = false;
}

void SynthGUI::setMasterVolume(float val)
{
    m_suppressHostUpdate = true;
    m_master_volume->setValue(val);
    m_suppressHostUpdate = false;
}

/*Begin new stuff*/

void SynthGUI::setMasterUnisonVoices(float val)
{
    m_suppressHostUpdate = true;
    m_master_unison_voices->setValue(val);
    m_suppressHostUpdate = false;
}

void SynthGUI::setMasterUnisonSpread(float val)
{
    m_suppressHostUpdate = true;
    m_master_unison_spread->setValue(val);
    m_suppressHostUpdate = false;
}

void SynthGUI::setMasterGlide(float val)
{
    m_suppressHostUpdate = true;
    m_master_glide->setValue(val);
    m_suppressHostUpdate = false;
}

void SynthGUI::setMasterPitchbendAmt(float val)
{
    m_suppressHostUpdate = true;
    m_master_pitchbend_amt->setValue(val);
    m_suppressHostUpdate = false;
}


/*End new stuff*/


/*Standard handlers for the audio slots, these perform manipulations of knob values
 that are common in audio applications*/

void SynthGUI::_changed_zero_to_x(int value, QLabel * _label, int _port)
{
    float val = float(value) * .01;
    _label->setText(QString("%1").arg(val));
    
    if (!m_suppressHostUpdate) {
	lo_send(m_host, m_controlPath, "if", _port, val);     
    }
}

void SynthGUI::_changed_integer(int value, QLabel * _label, int _port)
{
    float val = float(value);
    _label->setText(QString("%1").arg(val));
    
    if (!m_suppressHostUpdate) {
	lo_send(m_host, m_controlPath, "if", _port, val);
    }
}

void SynthGUI::_changed_seconds(int value, QLabel * _label, int _port)
{
    float sec = float(value) * .01;
    _label->setText(QString("%1").arg(sec));
    
    if (!m_suppressHostUpdate) {
	lo_send(m_host, m_controlPath, "if", _port, sec);
    }
}

void SynthGUI::_changed_pitch(int value, QLabel * _label, int _port)
{
    /*We need to send midi note number to the synth, as it probably still needs to process it as
     midi_note number.  We use this to display hz to the user*/
    
    float _f_value = float(value);
    float _hz = _pit_midi_note_to_hz(_f_value);
    
    _label->setText(QString("%1 hz").arg((int)_hz));
    
    if (!m_suppressHostUpdate) {
	lo_send(m_host, m_controlPath, "if", _port, _f_value);     
    }    
}

void SynthGUI::_changed_decibels(int value, QLabel * _label, int _port)
{
    /*Decibels is a reasonable way to display this to the user, so just use it as it is*/
    _label->setText(QString("%1").arg(value));
    
    if (!m_suppressHostUpdate) {
	lo_send(m_host, m_controlPath, "if", _port, float(value));
    }
}

/*GUI Step 7:  Implement the event handlers from step 3.*/

void SynthGUI::attackChanged(int value)
{
    _changed_seconds(value,m_attackLabel,LTS_PORT_ATTACK);
}

void
SynthGUI::decayChanged(int value)
{
    _changed_seconds(value,m_decayLabel,LTS_PORT_DECAY);
}

void SynthGUI::sustainChanged(int value)
{
    _changed_decibels(value, m_sustainLabel, LTS_PORT_SUSTAIN);    
}

void SynthGUI::releaseChanged(int value)
{
    _changed_seconds(value, m_releaseLabel, LTS_PORT_RELEASE);    
}

void SynthGUI::timbreChanged(int value)
{
    _changed_pitch(value, m_timbreLabel, LTS_PORT_TIMBRE);    
}

void SynthGUI::resChanged(int value)
{
    _changed_decibels(value, m_resLabel, LTS_PORT_RES);    
}

void SynthGUI::distChanged(int value)
{
    _changed_integer(value, m_distLabel, LTS_PORT_DIST);
}


void SynthGUI::filterAttackChanged(int value)
{
    _changed_seconds(value,m_filter_attackLabel,LTS_PORT_FILTER_ATTACK);
}

void
SynthGUI::filterDecayChanged(int value)
{
    _changed_seconds(value,m_filter_decayLabel,LTS_PORT_FILTER_DECAY);
}

void SynthGUI::filterSustainChanged(int value)
{
    _changed_zero_to_x(value, m_filter_sustainLabel, LTS_PORT_FILTER_SUSTAIN);    
}

void SynthGUI::filterReleaseChanged(int value)
{
    _changed_seconds(value, m_filter_releaseLabel, LTS_PORT_FILTER_RELEASE);    
}

void SynthGUI::noiseAmpChanged(int value)
{
    _changed_decibels(value, m_noise_ampLabel, LTS_PORT_NOISE_AMP);
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
    _changed_integer(value, m_osc1_pitchLabel, LTS_PORT_OSC1_PITCH);
}

void SynthGUI::osc1TuneChanged(int value)
{
    _changed_zero_to_x(value, m_osc1_tuneLabel, LTS_PORT_OSC1_TUNE);
}

void SynthGUI::osc1VolumeChanged(int value)
{
    _changed_decibels(value, m_osc1_volumeLabel, LTS_PORT_OSC1_VOLUME);
}

void SynthGUI::osc2TypeChanged(int value)
{
    if (!m_suppressHostUpdate) {
	lo_send(m_host, m_controlPath, "if", LTS_PORT_OSC2_TYPE, float(value));
    }
}

void SynthGUI::osc2PitchChanged(int value)
{
    _changed_integer(value, m_osc2_pitchLabel, LTS_PORT_OSC2_PITCH);
}

void SynthGUI::osc2TuneChanged(int value)
{
    _changed_zero_to_x(value, m_osc2_tuneLabel, LTS_PORT_OSC2_TUNE);
}

void SynthGUI::osc2VolumeChanged(int value)
{
    _changed_decibels(value, m_osc2_volumeLabel, LTS_PORT_OSC2_VOLUME);
}

void SynthGUI::masterVolumeChanged(int value)
{
    _changed_decibels(value, m_master_volumeLabel, LTS_PORT_MASTER_VOLUME);
}


/*Begin new stuff*/


void SynthGUI::masterUnisonVoicesChanged(int value)
{
    _changed_integer(value, m_master_unison_voicesLabel, LTS_PORT_MASTER_UNISON_VOICES);
}


void SynthGUI::masterUnisonSpreadChanged(int value)
{    
    _changed_zero_to_x(value, m_master_unison_spreadLabel, LTS_PORT_MASTER_UNISON_SPREAD);
}


void SynthGUI::masterGlideChanged(int value)
{
    _changed_zero_to_x(value, m_master_glideLabel, LTS_PORT_MASTER_GLIDE);
}


void SynthGUI::masterPitchbendAmtChanged(int value)
{
    _changed_integer(value, m_master_pitchbend_amtLabel, LTS_PORT_MASTER_PITCHBEND_AMT);
}

/*End new stuff*/

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
#ifdef LMS_DEBUG_MODE_QT
    cerr << "Program handler not yet implemented" << endl;
#endif
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

    /*GUI Step 8:  Add the controls you created to the control handler*/
    switch (port) {
    case LTS_PORT_ATTACK:
	cerr << "gui setting attack to " << value << endl;
	gui->setAttack(value);
	break;

    case LTS_PORT_DECAY:
	cerr << "gui setting decay to " << value << endl;
	gui->setDecay(value);
	break;

    case LTS_PORT_SUSTAIN:
	cerr << "gui setting sustain to " << value << endl;
	gui->setSustain(value);
	break;

    case LTS_PORT_RELEASE:
	cerr << "gui setting release to " << value << endl;
	gui->setRelease(value);
	break;

    case LTS_PORT_TIMBRE:
	cerr << "gui setting timbre to " << value << endl;
	gui->setTimbre(value);
	break;

    case LTS_PORT_RES:
	cerr << "gui setting res to " << value << endl;
	gui->setRes(value);
	break;
        
    case LTS_PORT_DIST:
	cerr << "gui setting res to " << value << endl;
	gui->setDist(value);
	break;

    case LTS_PORT_FILTER_ATTACK:
	cerr << "gui setting attack to " << value << endl;
	gui->setFilterAttack(value);
	break;

    case LTS_PORT_FILTER_DECAY:
	cerr << "gui setting decay to " << value << endl;
	gui->setFilterDecay(value);
	break;

    case LTS_PORT_FILTER_SUSTAIN:
	cerr << "gui setting sustain to " << value << endl;
	gui->setFilterSustain(value);
	break;

    case LTS_PORT_FILTER_RELEASE:
	cerr << "gui setting release to " << value << endl;
	gui->setFilterRelease(value);
	break;

    case LTS_PORT_NOISE_AMP:
        cerr << "setting noise amp to " << value << endl;
        gui->setNoiseAmp(value);
        break;
    
    case LTS_PORT_DIST_WET:
        cerr << "setting dist wet to " << value << endl;
        gui->setDistWet(value);
        break;
            
    case LTS_PORT_FILTER_ENV_AMT:
        cerr << "setting filter env amt to " << value << endl;
        gui->setFilterEnvAmt(value);
        break;
    
    case LTS_PORT_OSC1_TYPE:
        cerr << "setting osc1type to " << value << endl;
        gui->setOsc1Type(value);
        break;
            
    case LTS_PORT_OSC1_PITCH:
        cerr << "setting osc1pitch to " << value << endl;
        gui->setOsc1Pitch(value);
        break;
    
    case LTS_PORT_OSC1_TUNE:
        cerr << "setting osc1tune to " << value << endl;
        gui->setOsc1Tune(value);
        break;
    
    case LTS_PORT_OSC1_VOLUME:
        cerr << "setting osc1vol amp to " << value << endl;
        gui->setOsc1Volume(value);
        break;
            
        
    case LTS_PORT_OSC2_TYPE:
        cerr << "setting osc2type to " << value << endl;
        gui->setOsc2Type(value);
        break;
            
    case LTS_PORT_OSC2_PITCH:
        cerr << "setting osc2pitch to " << value << endl;
        gui->setOsc2Pitch(value);
        break;
    
    case LTS_PORT_OSC2_TUNE:
        cerr << "setting osc2tune to " << value << endl;
        gui->setOsc2Tune(value);
        break;
    
    case LTS_PORT_OSC2_VOLUME:
        cerr << "setting osc2vol amp to " << value << endl;
        gui->setOsc2Volume(value);
        break;
            
        
    case LTS_PORT_MASTER_VOLUME:
        cerr << "setting noise amp to " << value << endl;
        gui->setMasterVolume(value);
        break;
    
        /*Begin new stuff*/
        

    case LTS_PORT_MASTER_UNISON_VOICES:
        cerr << "setting unison voices to " << value << endl;
        gui->setMasterUnisonVoices(value);
        break;


    case LTS_PORT_MASTER_UNISON_SPREAD:
        cerr << "setting unison spread to " << value << endl;
        gui->setMasterUnisonSpread(value);
        break;


    case LTS_PORT_MASTER_GLIDE:
        cerr << "setting glide to " << value << endl;
        gui->setMasterGlide(value);
        break;


    case LTS_PORT_MASTER_PITCHBEND_AMT:
        cerr << "setting pitchbend to " << value << endl;
        gui->setMasterPitchbendAmt(value);
        break;

        
        
        /*End new stuff*/
        
    default:
	cerr << "Warning: received request to set nonexistent port " << port << endl;
    }

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

