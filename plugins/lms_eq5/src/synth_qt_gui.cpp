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
    
    this->setWindowTitle(QString("LMS EQ5  Powered by LibModSynth."));
    
    /*Set the CSS style that will "cascade" on the other controls.  Other control's styles can be overridden by running their own setStyleSheet method*/
    this->setStyleSheet("QGroupBox {background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #E0E0E0, stop: 1 #FFFFFF); border: 2px solid gray;  border-radius: 10px;  margin-top: 1ex; } QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 3px; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #FFOECE, stop: 1 #FFFFFF); }");
    
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
    
    QGroupBox * f_gb_filter = newGroupBox("Parametric EQ", this); 
    QGridLayout *f_gb_filter_layout = new QGridLayout(f_gb_filter);
    
    /*EQ1*/
    m_pitch1  =  get_knob(pitch); 
    m_pitch1Label = newQLabel(this);
    add_widget(f_gb_filter_layout, f_gb_layout_column, f_gb_layout_row, "Freq",m_pitch1, m_pitch1Label);    
    connect(m_pitch1,  SIGNAL(valueChanged(int)), this, SLOT(pitch1Changed(int)));
        
    //f_gb_layout_column++;
    f_gb_layout_row += 3;
    
    m_gain1  =   newQDial(-24, 24, 1, 0);
    m_gain1Label  = newQLabel(this);
    add_widget(f_gb_filter_layout, f_gb_layout_column, f_gb_layout_row, "Boost",m_gain1, m_gain1Label);
    connect(m_gain1,  SIGNAL(valueChanged(int)), this, SLOT(gain1Changed(int)));
        
    //f_gb_layout_column++;
    f_gb_layout_row += 3;
        
    m_res1  =   newQDial(-24, -1, 1, -15);
    m_res1Label  = newQLabel(this);
    add_widget(f_gb_filter_layout, f_gb_layout_column, f_gb_layout_row, "Q",m_res1, m_res1Label);
    connect(m_res1,  SIGNAL(valueChanged(int)), this, SLOT(res1Changed(int)));
        
    f_gb_layout_column++;
    f_gb_layout_row = 0;
    
    /*EQ2*/
    
    m_pitch2  =  get_knob(pitch); 
    m_pitch2Label = newQLabel(this);
    add_widget(f_gb_filter_layout, f_gb_layout_column, f_gb_layout_row, "Freq",m_pitch2, m_pitch2Label);    
    connect(m_pitch2,  SIGNAL(valueChanged(int)), this, SLOT(pitch2Changed(int)));
        
    //f_gb_layout_column++;
    f_gb_layout_row += 3;
    
    m_gain2  =   newQDial(-24, 24, 1, 0);
    m_gain2Label  = newQLabel(this);
    add_widget(f_gb_filter_layout, f_gb_layout_column, f_gb_layout_row, "Boost",m_gain2, m_gain2Label);
    connect(m_gain2,  SIGNAL(valueChanged(int)), this, SLOT(gain2Changed(int)));
        
    //f_gb_layout_column++;
    f_gb_layout_row += 3;
        
    m_res2  =   newQDial(-24, -1, 1, -15);
    m_res2Label  = newQLabel(this);
    add_widget(f_gb_filter_layout, f_gb_layout_column, f_gb_layout_row, "Q",m_res2, m_res2Label);
    connect(m_res2,  SIGNAL(valueChanged(int)), this, SLOT(res2Changed(int)));
        
    f_gb_layout_column++;
    f_gb_layout_row = 0;
    
    /*EQ3*/
    
    m_pitch3  =  get_knob(pitch); 
    m_pitch3Label = newQLabel(this);
    add_widget(f_gb_filter_layout, f_gb_layout_column, f_gb_layout_row, "Freq",m_pitch3, m_pitch3Label);    
    connect(m_pitch3,  SIGNAL(valueChanged(int)), this, SLOT(pitch3Changed(int)));
        
    //f_gb_layout_column++;
    f_gb_layout_row += 3;
    
    m_gain3  =   newQDial(-24, 24, 1, 0);
    m_gain3Label  = newQLabel(this);
    add_widget(f_gb_filter_layout, f_gb_layout_column, f_gb_layout_row, "Boost",m_gain3, m_gain3Label);
    connect(m_gain3,  SIGNAL(valueChanged(int)), this, SLOT(gain3Changed(int)));
        
    //f_gb_layout_column++;
    f_gb_layout_row += 3;
        
    m_res3  =   newQDial(-24, -1, 1, -15);
    m_res3Label  = newQLabel(this);
    add_widget(f_gb_filter_layout, f_gb_layout_column, f_gb_layout_row, "Q",m_res3, m_res3Label);
    connect(m_res3,  SIGNAL(valueChanged(int)), this, SLOT(res3Changed(int)));
        
    f_gb_layout_column++;
    f_gb_layout_row = 0;
    
    /*EQ4*/
    
    m_pitch4  =  get_knob(pitch); 
    m_pitch4Label = newQLabel(this);
    add_widget(f_gb_filter_layout, f_gb_layout_column, f_gb_layout_row, "Freq",m_pitch4, m_pitch4Label);    
    connect(m_pitch4,  SIGNAL(valueChanged(int)), this, SLOT(pitch4Changed(int)));
        
    //f_gb_layout_column++;
    f_gb_layout_row += 3;
    
    m_gain4  =   newQDial(-24, 24, 1, 0);
    m_gain4Label  = newQLabel(this);
    add_widget(f_gb_filter_layout, f_gb_layout_column, f_gb_layout_row, "Boost",m_gain4, m_gain4Label);
    connect(m_gain4,  SIGNAL(valueChanged(int)), this, SLOT(gain4Changed(int)));
        
    //f_gb_layout_column++;
    f_gb_layout_row += 3;
        
    m_res4  =   newQDial(-24, -1, 1, -15);
    m_res4Label  = newQLabel(this);
    add_widget(f_gb_filter_layout, f_gb_layout_column, f_gb_layout_row, "Q",m_res4, m_res4Label);
    connect(m_res4,  SIGNAL(valueChanged(int)), this, SLOT(res4Changed(int)));
        
    f_gb_layout_column++;
    f_gb_layout_row = 0;
    
    /*EQ5*/
    
    m_pitch5  =  get_knob(pitch); 
    m_pitch5Label = newQLabel(this);
    add_widget(f_gb_filter_layout, f_gb_layout_column, f_gb_layout_row, "Freq",m_pitch5, m_pitch5Label);    
    connect(m_pitch5,  SIGNAL(valueChanged(int)), this, SLOT(pitch5Changed(int)));
        
    //f_gb_layout_column++;
    f_gb_layout_row += 3;
    
    m_gain5  =   newQDial(-24, 24, 1, 0);
    m_gain5Label  = newQLabel(this);
    add_widget(f_gb_filter_layout, f_gb_layout_column, f_gb_layout_row, "Boost",m_gain5, m_gain5Label);
    connect(m_gain5,  SIGNAL(valueChanged(int)), this, SLOT(gain5Changed(int)));
        
    //f_gb_layout_column++;
    f_gb_layout_row += 3;
        
    m_res5  =   newQDial(-24, -1, 1, -15);
    m_res5Label  = newQLabel(this);
    add_widget(f_gb_filter_layout, f_gb_layout_column, f_gb_layout_row, "Q",m_res5, m_res5Label);
    connect(m_res5,  SIGNAL(valueChanged(int)), this, SLOT(res5Changed(int)));
        
    f_gb_layout_column++;
    f_gb_layout_row = 0;
    
    /*End EQ sections*/
    
    layout_row0->addWidget(f_gb_filter, -1, Qt::AlignLeft);
    f_column++;
    f_gb_layout_row = 0;
    f_gb_layout_column = 0;
        
    QLabel * f_logo_label = new QLabel("", this);    
    f_logo_label->setTextFormat(Qt::RichText);
    /*This string is a base64 encoded .png image I created using Gimp for the logo.  To get the base64 encoded string,
     run it through png_to_base64.pl in the packaging folder*/
    QString f_logo_text = "<html><img src=\"data:image/png;base64,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\"/></html>";
    f_logo_label->setText(f_logo_text);
    f_logo_label->setMinimumSize(90, 30);   
    f_logo_label->show();
    layout_row1->addWidget(f_logo_label, -1, Qt::AlignRight);
                
        
    /*DO NOT remove the code below this*/

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

/*EQ1*/
void SynthGUI::setGain1(float val)
{
    m_suppressHostUpdate = true;
    m_gain1->setValue(int(val));
    m_suppressHostUpdate = false;
}

void SynthGUI::setPitch1(float val)
{
    m_suppressHostUpdate = true;
    m_pitch1->setValue(int(val));
    m_suppressHostUpdate = false;
}

void SynthGUI::setRes1(float val)
{
    m_suppressHostUpdate = true;
    m_res1->setValue(int(val));
    m_suppressHostUpdate = false;
}

/*EQ2*/
void SynthGUI::setGain2(float val)
{
    m_suppressHostUpdate = true;
    m_gain2->setValue(int(val));
    m_suppressHostUpdate = false;
}

void SynthGUI::setPitch2(float val)
{
    m_suppressHostUpdate = true;
    m_pitch2->setValue(int(val));
    m_suppressHostUpdate = false;
}

void SynthGUI::setRes2(float val)
{
    m_suppressHostUpdate = true;
    m_res2->setValue(int(val));
    m_suppressHostUpdate = false;
}

/*EQ3*/
void SynthGUI::setGain3(float val)
{
    m_suppressHostUpdate = true;
    m_gain3->setValue(int(val));
    m_suppressHostUpdate = false;
}

void SynthGUI::setPitch3(float val)
{
    m_suppressHostUpdate = true;
    m_pitch3->setValue(int(val));
    m_suppressHostUpdate = false;
}

void SynthGUI::setRes3(float val)
{
    m_suppressHostUpdate = true;
    m_res3->setValue(int(val));
    m_suppressHostUpdate = false;
}


/*EQ4*/
void SynthGUI::setGain4(float val)
{
    m_suppressHostUpdate = true;
    m_gain4->setValue(int(val));
    m_suppressHostUpdate = false;
}

void SynthGUI::setPitch4(float val)
{
    m_suppressHostUpdate = true;
    m_pitch4->setValue(int(val));
    m_suppressHostUpdate = false;
}

void SynthGUI::setRes4(float val)
{
    m_suppressHostUpdate = true;
    m_res4->setValue(int(val));
    m_suppressHostUpdate = false;
}


/*EQ5*/
void SynthGUI::setGain5(float val)
{
    m_suppressHostUpdate = true;
    m_gain5->setValue(int(val));
    m_suppressHostUpdate = false;
}

void SynthGUI::setPitch5(float val)
{
    m_suppressHostUpdate = true;
    m_pitch5->setValue(int(val));
    m_suppressHostUpdate = false;
}

void SynthGUI::setRes5(float val)
{
    m_suppressHostUpdate = true;
    m_res5->setValue(int(val));
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

/*EQ1*/
void SynthGUI::pitch1Changed(int value)
{
    changed_pitch(value, m_pitch1Label, LMS_PITCH1);        
}

void SynthGUI::gain1Changed(int value)
{
    changed_decibels(value, m_gain1Label, LMS_GAIN1);    
}

void SynthGUI::res1Changed(int value)
{
    changed_decibels(value, m_res1Label, LMS_RES1);    
}

/*EQ2*/
void SynthGUI::pitch2Changed(int value)
{
    changed_pitch(value, m_pitch2Label, LMS_PITCH2);        
}

void SynthGUI::gain2Changed(int value)
{
    changed_decibels(value, m_gain2Label, LMS_GAIN2);    
}

void SynthGUI::res2Changed(int value)
{
    changed_decibels(value, m_res2Label, LMS_RES2);    
}

/*EQ3*/
void SynthGUI::pitch3Changed(int value)
{
    changed_pitch(value, m_pitch3Label, LMS_PITCH3);        
}

void SynthGUI::gain3Changed(int value)
{
    changed_decibels(value, m_gain3Label, LMS_GAIN3);    
}

void SynthGUI::res3Changed(int value)
{
    changed_decibels(value, m_res3Label, LMS_RES3);    
}

/*EQ4*/
void SynthGUI::pitch4Changed(int value)
{
    changed_pitch(value, m_pitch4Label, LMS_PITCH4);        
}

void SynthGUI::gain4Changed(int value)
{
    changed_decibels(value, m_gain4Label, LMS_GAIN4);    
}

void SynthGUI::res4Changed(int value)
{
    changed_decibels(value, m_res4Label, LMS_RES4);    
}

/*EQ5*/
void SynthGUI::pitch5Changed(int value)
{
    changed_pitch(value, m_pitch5Label, LMS_PITCH5);        
}

void SynthGUI::gain5Changed(int value)
{
    changed_decibels(value, m_gain5Label, LMS_GAIN5);    
}

void SynthGUI::res5Changed(int value)
{
    changed_decibels(value, m_res5Label, LMS_RES5);    
}




void SynthGUI::v_print_port_name_to_cerr(int a_port)
{
#ifdef LMS_DEBUG_MODE_QT
    switch (a_port) {
    case LMS_GAIN1:
	cerr << "LMS_CUTOFF";
	break;
    case LMS_PITCH1:
	cerr << "LMS_RES";
	break;        
    case LMS_DIST:
	cerr << "LMS_DIST";
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
        /*EQ1*/
        case LMS_GAIN1:
            setGain1(a_value);
            break;
        case LMS_PITCH1:
            setPitch1(a_value);
            break;
        case LMS_RES1:
            setRes1(a_value);
            break;
            /*EQ2*/
        case LMS_GAIN2:
            setGain2(a_value);
            break;
        case LMS_PITCH2:
            setPitch2(a_value);
            break;
        case LMS_RES2:
            setRes2(a_value);
            break;
            /*EQ3*/
        case LMS_GAIN3:
            setGain3(a_value);
            break;
        case LMS_PITCH3:
            setPitch3(a_value);
            break;
        case LMS_RES3:
            setRes3(a_value);
            break;
            /*EQ4*/
        case LMS_GAIN4:
            setGain4(a_value);
            break;
        case LMS_PITCH4:
            setPitch4(a_value);
            break;
        case LMS_RES4:
            setRes4(a_value);
            break;
            /*EQ5*/
        case LMS_GAIN5:
            setGain5(a_value);
            break;
        case LMS_PITCH5:
            setPitch5(a_value);
            break;
        case LMS_RES5:
            setRes5(a_value);
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
    
    switch (a_port) 
    {
        /*EQ1*/
        case LMS_GAIN1:
            gain1Changed(a_value);
            break;
        case LMS_PITCH1:
            pitch1Changed(a_value);
            break;
        case LMS_RES1:
            res1Changed(a_value);
            break;
        /*EQ2*/
        case LMS_GAIN2:
            gain2Changed(a_value);
            break;
        case LMS_PITCH2:
            pitch2Changed(a_value);
            break;
        case LMS_RES2:
            res2Changed(a_value);
            break;
        /*EQ3*/
        case LMS_GAIN3:
            gain3Changed(a_value);
            break;
        case LMS_PITCH3:
            pitch3Changed(a_value);
            break;
        case LMS_RES3:
            res3Changed(a_value);
            break;
        /*EQ4*/
        case LMS_GAIN4:
            gain4Changed(a_value);
            break;
        case LMS_PITCH4:
            pitch4Changed(a_value);
            break;
        case LMS_RES4:
            res4Changed(a_value);
            break;
        /*EQ5*/
        case LMS_GAIN5:
            gain5Changed(a_value);
            break;
        case LMS_PITCH5:
            pitch5Changed(a_value);
            break;
        case LMS_RES5:
            res5Changed(a_value);
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
        /*EQ1*/
        case LMS_PITCH1:
            return m_pitch1->value();
        case LMS_GAIN1:
            return m_gain1->value();
        case LMS_RES1:
            return m_gain1->value();
            
        /*EQ2*/
        case LMS_PITCH2:
            return m_pitch2->value();
        case LMS_GAIN2:
            return m_gain2->value();
        case LMS_RES2:
            return m_gain2->value();

        /*EQ3*/
        case LMS_PITCH3:
            return m_pitch3->value();
        case LMS_GAIN3:
            return m_gain3->value();
        case LMS_RES3:
            return m_gain3->value();

        /*EQ4*/
        case LMS_PITCH4:
            return m_pitch4->value();
        case LMS_GAIN4:
            return m_gain4->value();
        case LMS_RES4:
            return m_gain4->value();

        /*EQ5*/
        case LMS_PITCH5:
            return m_pitch5->value();
        case LMS_GAIN5:
            return m_gain5->value();
        case LMS_RES5:
            return m_gain5->value();

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

