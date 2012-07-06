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

#include <QGroupBox>
#include <QLabel>
#include <QGridLayout>
#include <QGroupBox>
#include <QDial>
#include <QPixmap>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QMessageBox>

#include <stdlib.h>
#include "../../libmodsynth/lib/amp.h"
#include "../../libmodsynth/lib/pitch_core.h"

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

#ifdef LMS_DEBUG_MODE_QT    
    static QTextStream cerr(stderr);
#endif    


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
    
    this->setWindowTitle(QString("LMS Delay  Powered by LibModSynth."));
    
    /*Set the CSS style that will "cascade" on the other controls.  Other control's styles can be overridden by running their own setStyleSheet method*/
    this->setStyleSheet("QPushButton {background-color: black; border-style: outset; border-width: 2px; border-radius: 10px;border-color: white;font: bold 14px; min-width: 10em; padding: 6px; color:white;}  QAbstractItemView {outline: none;} QComboBox{border:1px solid white;border-radius:3px; padding:1px;background-color:black;color:white} QComboBox::drop-down{color:white;background-color:black;padding:2px;border-radius:2px;} QDial{background-color:rgb(152, 152, 152);} QFrame{background-color:rgb(0,0,0);} QGroupBox {color: white; border: 2px solid gray;  border-radius: 10px;  margin-top: 1ex; } QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 3px;} QMessageBox{color:white;background-color:black;}");
    
#ifdef LMS_DEBUG_MODE_QT    
    cerr << "Creating the GUI controls" << endl;    
#endif
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    QHBoxLayout *layout_row0 = new QHBoxLayout();
    QHBoxLayout *layout_row1 = new QHBoxLayout();
    
    layout->addLayout(layout_row0);
    layout->addLayout(layout_row1, -1);        
        
    int f_row = 0;
    int f_column = 0;
    
    int f_gb_layout_row = 0;
    int f_gb_layout_column = 0;
    /*Lay out the controls you declared in the first step*/
    
#ifdef LMS_DEBUG_MODE_QT    
    cerr << "Creating the Filter controls" << endl;    
#endif    
    
    QGroupBox * f_gb_filter = newGroupBox("Delay", this); 
    QGridLayout *f_gb_filter_layout = new QGridLayout(f_gb_filter);
    
    m_delaytime  =   newQDial(10, 100, 1, 50);//get_knob(zero_to_one); //newQDial(0, 4, 1, 2);
    m_delaytimeLabel  = newQLabel(this);
    add_widget(f_gb_filter_layout, f_gb_layout_column, f_gb_layout_row, "Time",m_delaytime, m_delaytimeLabel);
    connect(m_delaytime,  SIGNAL(valueChanged(int)), this, SLOT(delayTimeChanged(int)));
        
    f_gb_layout_column++;
    
    m_feedback  =  newQDial(-15, 0, 1, -6); //get_knob(decibels_20_to_0); 
    m_feedbackLabel  = newQLabel(this);
    add_widget(f_gb_filter_layout, f_gb_layout_column, f_gb_layout_row, "Feedback", m_feedback, m_feedbackLabel);
    connect(m_feedback,  SIGNAL(valueChanged(int)), this, SLOT(feedbackChanged(int)));
        
    f_gb_layout_column++;
    
    m_dry  =   get_knob(decibels_30_to_0); 
    m_dryLabel  = newQLabel(this);
    add_widget(f_gb_filter_layout, f_gb_layout_column, f_gb_layout_row, "Dry",m_dry, m_dryLabel);
    connect(m_dry,  SIGNAL(valueChanged(int)), this, SLOT(dryChanged(int)));
        
    f_gb_layout_column++;
    
    m_wet  =  get_knob(decibels_30_to_0); 
    m_wetLabel  = newQLabel(this);
    add_widget(f_gb_filter_layout, f_gb_layout_column, f_gb_layout_row, "Wet", m_wet, m_wetLabel);
    connect(m_wet,  SIGNAL(valueChanged(int)), this, SLOT(wetChanged(int)));
        
    f_gb_layout_column++;
    
    m_duck  =  newQDial(-40, 0, 1, -20); // get_knob(decibels_0);
    m_duckLabel  = newQLabel(this);
    add_widget(f_gb_filter_layout, f_gb_layout_column, f_gb_layout_row, "Duck",m_duck, m_duckLabel);
    connect(m_duck,  SIGNAL(valueChanged(int)), this, SLOT(duckChanged(int)));
        
    f_gb_layout_column++;
    
    m_cutoff  =  newQDial(40, 118, 1, 105); // get_knob(pitch); 
    m_cutoffLabel  = newQLabel(this);
    add_widget(f_gb_filter_layout, f_gb_layout_column, f_gb_layout_row, "Cutoff", m_cutoff, m_cutoffLabel);
    connect(m_cutoff,  SIGNAL(valueChanged(int)), this, SLOT(cutoffChanged(int)));
    
    f_gb_layout_column++;
    
    m_stereo  =  get_knob(zero_to_one); 
    m_stereoLabel  = newQLabel(this);
    add_widget(f_gb_filter_layout, f_gb_layout_column, f_gb_layout_row, "Stereo", m_stereo, m_stereoLabel);
    connect(m_stereo,  SIGNAL(valueChanged(int)), this, SLOT(stereoChanged(int)));
    
    f_gb_layout_column++;
    
    layout_row0->addWidget(f_gb_filter, -1, Qt::AlignLeft);
    f_column++;
    f_gb_layout_row = 0;
    f_gb_layout_column = 0;
    
    QGroupBox * f_gb_bpm = newGroupBox("Tempo Sync", this); 
    QGridLayout *f_gb_bpm_layout = new QGridLayout(f_gb_bpm);
    
    m_bpm_spinbox = new QDoubleSpinBox(this);
    m_bpm_spinbox->setGeometry(QRect(100, 130, 71, 27));
    m_bpm_spinbox->setDecimals(1);
    m_bpm_spinbox->setMinimum(60);
    m_bpm_spinbox->setMaximum(200);
    m_bpm_spinbox->setSingleStep(0.1);
    
    QString f_beat_fracs [] = {"1/4", "1/3", "1/2", "2/3", "3/4", "1"};
    int f_beat_fracs_count = 6;    
    m_beat_frac = get_combobox(f_beat_fracs, f_beat_fracs_count , this);     
    
    m_sync_bpm = new QPushButton(this);
    m_sync_bpm->setText("Sync");
    connect(m_sync_bpm, SIGNAL(pressed()), this, SLOT(bpmSyncPressed()));
    
    QLabel * f_bpm_label = new QLabel("BPM",  this);
    f_bpm_label->setMinimumWidth(60);
    f_bpm_label->setAlignment(Qt::AlignCenter);
    f_bpm_label->setStyleSheet("background-color: white; border: 1px solid black;  border-radius: 6px;");
    
    QLabel * f_beat_label = new QLabel("Beats",  this);
    f_beat_label->setMinimumWidth(60);
    f_beat_label->setAlignment(Qt::AlignCenter);
    f_beat_label->setStyleSheet("background-color: white; border: 1px solid black;  border-radius: 6px;");
    
    f_gb_bpm_layout->addWidget(f_bpm_label, 0, 0, Qt::AlignCenter);
    f_gb_bpm_layout->addWidget(m_bpm_spinbox, 1, 0, Qt::AlignCenter);
    f_gb_bpm_layout->addWidget(f_beat_label, 0, 1, Qt::AlignCenter);
    f_gb_bpm_layout->addWidget(m_beat_frac, 1, 1, Qt::AlignCenter);
    f_gb_bpm_layout->addWidget(m_sync_bpm, 2, 1, Qt::AlignCenter);
    
    layout_row0->addWidget(f_gb_bpm, -1, Qt::AlignLeft);
        
    /*End test button code, DO NOT remove the code below this*/

    QTimer *myTimer = new QTimer(this);
    connect(myTimer, SIGNAL(timeout()), this, SLOT(oscRecv()));
    myTimer->setSingleShot(false);
    myTimer->start(0);
    
    m_suppressHostUpdate = false;

}

/*I'm leaving this in here for now, but at the present it doesn't work*/
void SynthGUI::v_add_knob_to_layout(QDial * a_knob, e_knob_type a_knob_type, int a_default_value, QLabel * a_label, QGridLayout * a_layout, QString a_title,
int a_gb_layout_column, int a_gb_layout_row, const char * a_signal, const char * a_slot)
{
    a_knob = get_knob(a_knob_type, a_default_value);
    a_label  = newQLabel(this);
    add_widget(a_layout, a_gb_layout_column, a_gb_layout_row, a_title,a_knob, a_label);
    connect(a_knob,  a_signal, this, a_slot);    
}


void SynthGUI::add_widget(QGridLayout * a_layout, int a_position_x, int a_position_y, QString a_label_text,  QWidget * a_widget,
    QLabel * _label)
{   
    QLabel * f_knob_title = new QLabel(a_label_text,  this);
    f_knob_title->setMinimumWidth(60);
    f_knob_title->setAlignment(Qt::AlignCenter);
    f_knob_title->setStyleSheet("background-color: white; border: 1px solid black;  border-radius: 6px;");
    
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
        case decibels_20_to_0:
            f_min = -20; f_max = 0; f_step = 1; f_value = -9;
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

/*newQDial(
 * int minValue,
 * int maxValue,
 * int pageStep,
 * int value
 * );
 */
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

void SynthGUI::setDelayTime(float val)
{
    m_suppressHostUpdate = true;
    m_delaytime->setValue(int(val));
    m_suppressHostUpdate = false;
}

void SynthGUI::setFeedback(float val)
{
    m_suppressHostUpdate = true;
    m_feedback->setValue(int(val));
    m_suppressHostUpdate = false;
}

void SynthGUI::setDry(float val)
{
    m_suppressHostUpdate = true;
    m_dry->setValue(int(val));
    m_suppressHostUpdate = false;
}

void SynthGUI::setWet(float val)
{
    m_suppressHostUpdate = true;
    m_wet->setValue(int(val));
    m_suppressHostUpdate = false;
}

void SynthGUI::setDuck(float val)
{
    m_suppressHostUpdate = true;
    m_duck->setValue(int(val));
    m_suppressHostUpdate = false;
}

void SynthGUI::setCutoff(float val)
{
    m_suppressHostUpdate = true;
    m_cutoff->setValue(int(val));
    m_suppressHostUpdate = false;
}

void SynthGUI::setStereo(float val)
{
    m_suppressHostUpdate = true;
    m_stereo->setValue(int(val));
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




/*Implement the event handlers from step 3.*/

void SynthGUI::delayTimeChanged(int value)
{
    changed_seconds(value, m_delaytimeLabel, LMS_DELAY_TIME); 
    
    /*
    QString f_value;
    
    switch(value)
    {
        case 0:
            m_delaytimeLabel->setText("1/4");            
            break;
        case 1:
            m_delaytimeLabel->setText("1/3");
            break;
        case 2:
            m_delaytimeLabel->setText("1/2");
            break;
        case 3:
            m_delaytimeLabel->setText("1");
            break;
        case 4:
            m_delaytimeLabel->setText("2");
            break;
    }
    
    if (!m_suppressHostUpdate) {
	lo_send(m_host, m_controlPath, "if", LMS_DELAY_TIME, float(value));
    }
     */
}

void SynthGUI::feedbackChanged(int value)
{
    changed_decibels(value, m_feedbackLabel, LMS_FEEDBACK);    
}

void SynthGUI::dryChanged(int value)
{
    changed_decibels(value, m_dryLabel, LMS_DRY);    
}

void SynthGUI::wetChanged(int value)
{
    changed_decibels(value, m_wetLabel, LMS_WET);    
}

void SynthGUI::duckChanged(int value)
{
    changed_decibels(value, m_duckLabel, LMS_DUCK);    
}

void SynthGUI::cutoffChanged(int value)
{
    changed_pitch(value, m_cutoffLabel, LMS_CUTOFF);    
}

void SynthGUI::stereoChanged(int value)
{
    changed_zero_to_x(value, m_stereoLabel, LMS_STEREO);    
}

void SynthGUI::bpmSyncPressed()
{
    float f_frac = 1.0f;
    
    switch(m_beat_frac->currentIndex())            
    {
        case 0:  // 1/4
            f_frac = 0.25f;
            break;
        case 1:  // 1/3
            f_frac = 0.3333f;
            break;
        case 2:  // 1/2
            f_frac = 0.5f;
            break;
        case 3:  // 2/3
            f_frac = 0.6666f;
            break;
        case 4:  // 3/4
            f_frac = 0.75f;
            break;
        case 5:  // 1
            f_frac = 1.0f;
            break;
    }
        
    float f_seconds_per_beat = 60/(m_bpm_spinbox->value());
    
    float f_result = (int)(f_seconds_per_beat * f_frac * 100);
    
    /*TODO: Possibly use the built in functions for this*/
    m_delaytime->setValue(f_result);
}


void SynthGUI::v_print_port_name_to_cerr(int a_port)
{
#ifdef LMS_DEBUG_MODE_QT
    switch (a_port) {
    case LMS_DELAY_TIME:
	cerr << "LMS_DELAY_TIME";
	break;
    case LMS_FEEDBACK:
	cerr << "LMS_FEEDBACK";
	break;        
    case LMS_DRY:
	cerr << "LMS_DRY";
	break;
    case LMS_WET:
	cerr << "LMS_WET";
	break;        
    case LMS_DUCK:
	cerr << "LMS_DUCK";
	break;
    case LMS_CUTOFF:
	cerr << "LMS_CUTOFF";
	break;        
    case LMS_STEREO:
	cerr << "LMS_STEREO";
	break;        
    default:
	cerr << "Warning: received request to set nonexistent port " << a_port ;
        break;
    }
#endif
}

void SynthGUI::v_set_control(int a_port, float a_value)
{

#ifdef LMS_DEBUG_MODE_QT    
    cerr << "v_set_control called.  ";  
    v_print_port_name_to_cerr(a_port);
    cerr << "  value: " << a_value << endl;
#endif
    
    /*Add the controls you created to the control handler*/
    
    switch (a_port) 
    {
        case LMS_DELAY_TIME:
            setDelayTime(a_value);
            break;
        case LMS_FEEDBACK:
            setFeedback(a_value);
            break;
        case LMS_DRY:
            setDry(a_value);
            break;
        case LMS_WET:
            setWet(a_value);
            break;
        case LMS_DUCK:
            setDuck(a_value);
            break;
        case LMS_CUTOFF:
            setCutoff(a_value);
            break;
        case LMS_STEREO:
            setStereo(a_value);
            break;
    }
}

void SynthGUI::v_control_changed(int a_port, int a_value, bool a_suppress_host_update)
{
    
#ifdef LMS_DEBUG_MODE_QT    
    cerr << "v_control_changed called.  ";  
    v_print_port_name_to_cerr(a_port);
    cerr << "  value: " << a_value << endl;
#endif
    
    if(a_suppress_host_update)
        m_suppressHostUpdate = true;
       /*Add the controls you created to the control handler*/
    
    switch (a_port) {
        case LMS_DELAY_TIME:
            delayTimeChanged(a_value);
            break;
        case LMS_FEEDBACK:
            feedbackChanged(a_value);
            break;
        case LMS_DRY:
            dryChanged(a_value);
            break;
        case LMS_WET:
            wetChanged(a_value);
            break;
        case LMS_DUCK:
            duckChanged(a_value);
            break;
        case LMS_CUTOFF:
            cutoffChanged(a_value);
            break;
        case LMS_STEREO:
            stereoChanged(a_value);
            break;
        default:
#ifdef LMS_DEBUG_MODE_QT
                cerr << "Warning: received request to set nonexistent port " << a_port << endl;
#endif
            break;
    }
    
    if(a_suppress_host_update)
        m_suppressHostUpdate = false;
}

/*TODO:  For the forseeable future, this will only be used for getting the values to write back to 
 the presets.tsv file;  It should probably return a string that can be re-interpreted into other values for
 complex controls that could have multiple ints, or string values, etc...*/
int SynthGUI::i_get_control(int a_port)
{        
    switch (a_port) {
        case LMS_DELAY_TIME:
            return m_delaytime->value();
        case LMS_FEEDBACK:
            return m_feedback->value();
        case LMS_DRY:
            return m_dry->value();
        case LMS_WET:
            return m_wet->value();
        case LMS_DUCK:
            return m_duck->value();
        case LMS_CUTOFF:
            return m_cutoff->value();
        case LMS_STEREO:
            return m_stereo->value();
        default:
#ifdef LMS_DEBUG_MODE_QT
            cerr << "Warning: received request to get nonexistent port " << a_port << endl;
#endif
         break;
    }
}



void SynthGUI::oscRecv()
{
    if (osc_server) {
	lo_server_recv_noblock(osc_server, 1);
    }
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
    //not implemented on this plugin
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
        
#ifdef LMS_DEBUG_MODE_QT
	cerr << "Error: too few arguments to control_handler" << endl;
#endif
	return 1;
    }

    const int port = argv[0]->i;
    const float value = argv[1]->f;

#ifdef LMS_DEBUG_MODE_QT
    cerr << "control_handler called.  port:  " << port << " , value " << value << endl;
#endif

    gui->v_set_control(port, value);  
     
    gui->v_control_changed(port, value, true);

    return 0;
}

int main(int argc, char **argv)
{
#ifdef LMS_DEBUG_MODE_QT
    cerr << "Qt GUI main() called..." << endl;
#endif
    
    QApplication application(argc, argv);

    if (application.argc() != 5) {
#ifdef LMS_DEBUG_MODE_QT
	cerr << "usage: "
	     << application.argv()[0] 
	     << " <osc url>"
	     << " <plugin dllname>"
	     << " <plugin label>"
	     << " <user-friendly id>"
	     << endl;

#endif
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
#ifdef LMS_DEBUG_MODE_QT
    cerr << "Adding lo server methods" << endl;
#endif
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
    
#ifdef LMS_DEBUG_MODE_QT
    cerr << "Starting GUI now..." << endl;
#endif
    
    return application.exec();
}

