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

#include <stdlib.h>
#include "libmodsynth/lib/amp.h"
#include "libmodsynth/lib/pitch_core.h"

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/SM/SMlib.h>
#include <qt4/QtGui/qgroupbox.h>
#include <qt4/QtGui/qlayout.h>
#include <qt4/QtGui/qlabel.h>
#include <qt4/QtGui/qgridlayout.h>
//#include <qt4/QtGui/qformlayout.h>
#include <QFormLayout>
#include <qt4/QtGui/qboxlayout.h>
#include <QGroupBox>

static int handle_x11_error(Display *dpy, XErrorEvent *err)
{
    char errstr[256];
    XGetErrorText(dpy, err->error_code, errstr, 256);
    if (err->error_code != BadWindow) {
	std::cerr << "less_trivial_synth_qt_gui: X Error: "
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
#define LTS_PORT_ATTACK_F  8
#define LTS_PORT_DECAY_F   9
#define LTS_PORT_SUSTAIN_F 10
#define LTS_PORT_RELEASE_F 11

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

    QGridLayout *layout = new QGridLayout(this);
    //QVBoxLayout *layout = new QVBoxLayout();
        
    int _row = 0;
    int _column = 0;
    
    
    /*GUI Step 4:  Lay out the controls you declared in the first step*/
    
    
    /*The amplitude ADSR GroupBox*/
    QGroupBox * _gb_adsr = _newGroupBox("ADSR", this);
    QGridLayout *_gb_adsr_layout = new QGridLayout(_gb_adsr);
    
    int _gb_layout_row = 0;
    int _gb_layout_column = 0;
    
    m_attack = _get_knob(zero_to_one);
    m_attackLabel = _newQLabel(this);
    _add_knob(_gb_adsr_layout, _gb_layout_column, _gb_layout_row, "Attack",m_attack, m_attackLabel);
    connect(m_attack,   SIGNAL(valueChanged(int)), this, SLOT(attackChanged(int)));
    attackChanged  (m_attack  ->value());
    
    _gb_layout_column++;
        
    m_decay   =  _get_knob(zero_to_one); //newQDial(  1, 100,  1,  25); // s * 100
    m_decayLabel   = _newQLabel(this);
    _add_knob(_gb_adsr_layout, _gb_layout_column, _gb_layout_row, "Decay",m_decay, m_decayLabel);
    connect(m_decay,   SIGNAL(valueChanged(int)), this, SLOT(decayChanged(int)));
    decayChanged  (m_decay  ->value());
    
    _gb_layout_column++;
    
    m_sustain =  _get_knob(decibels_0); // newQDial(  0, 100,  1,  75); // %
    m_sustainLabel = _newQLabel(this);
    _add_knob(_gb_adsr_layout, _gb_layout_column, _gb_layout_row, "Sustain", m_sustain, m_sustainLabel);    
    connect(m_sustain, SIGNAL(valueChanged(int)), this, SLOT(sustainChanged(int)));
    sustainChanged(m_sustain->value());
    
    _gb_layout_column++;
    
    m_release = _get_knob(zero_to_four); //newQDial(  1, 400, 10, 200); // s * 100
    m_releaseLabel = _newQLabel(this);
    _add_knob(_gb_adsr_layout, _gb_layout_column, _gb_layout_row, "Release", m_release, m_releaseLabel);
    connect(m_release, SIGNAL(valueChanged(int)), this, SLOT(releaseChanged(int)));
    releaseChanged(m_release->value());
        
    _gb_layout_column++;
    
    layout->addWidget(_gb_adsr, _row, _column, Qt::AlignCenter);
    
    
    _column++;
    
    
    /*The Filter GroupBox*/
    QGroupBox * _gb_filter = _newGroupBox("LP Filter", this); 
    QGridLayout *_gb_filter_layout = new QGridLayout(_gb_filter);
    
    _gb_layout_row = 0;
    _gb_layout_column = 0;
    
    m_timbre  =  _get_knob(pitch);  //newQDial(  39, 136,  1,  82); // s * 100
    m_timbreLabel  = _newQLabel(this);
    _add_knob(_gb_filter_layout, _gb_layout_column, _gb_layout_row, "Timbre",m_timbre, m_timbreLabel);
    connect(m_timbre,  SIGNAL(valueChanged(int)), this, SLOT(timbreChanged(int)));
    timbreChanged (m_timbre ->value());
    
    _gb_layout_column++;
    
    m_res  =  _get_knob(decibels_0); 
    m_resLabel  = _newQLabel(this);
    _add_knob(_gb_filter_layout, _gb_layout_column, _gb_layout_row, "Res", m_res, m_resLabel);
    connect(m_res,  SIGNAL(valueChanged(int)), this, SLOT(resChanged(int)));
    resChanged (m_res ->value());
    
    _gb_layout_column++;
    
    layout->addWidget(_gb_filter, _row, _column, Qt::AlignCenter);
    
    _column++;
    
    
    /*The Distortion GroupBox*/
    QGroupBox * _gb_dist = _newGroupBox("Distortion", this);    
    QGridLayout *_gb_dist_layout = new QGridLayout(_gb_dist);
    
    _gb_layout_row = 0;
    _gb_layout_column = 0;
    
    
    m_dist  = newQDial(  -6, 36,  1,  -6); 
    m_distLabel  = new QLabel(this);
    _add_knob(_gb_dist_layout, _column, _row, "Gain", m_dist, m_distLabel);
    connect(m_dist,  SIGNAL(valueChanged(int)), this, SLOT(distChanged(int)));
    distChanged (m_dist ->value());
    
    layout->addWidget(_gb_dist, _row, _column, Qt::AlignCenter);
    
    _column++;
    
    /*Start a new row*/
    _row++;
    _column = 0;
    
    
    /*The filter ADSR GroupBox*/
    QGroupBox * _gb_adsr_f = _newGroupBox("ADSR Filter", this);
    QGridLayout *_gb_adsr_f_layout = new QGridLayout(_gb_adsr_f);
    
    _gb_layout_row = 0;
    _gb_layout_column = 0;
    
    m_attack_f = _get_knob(zero_to_one);
    m_attackLabel_f = _newQLabel(this);
    _add_knob(_gb_adsr_f_layout, _gb_layout_column, _gb_layout_row, "Attack",m_attack_f, m_attackLabel_f);
    connect(m_attack_f,   SIGNAL(valueChanged(int)), this, SLOT(attack_fChanged(int)));
    attack_fChanged  (m_attack_f  ->value());
    
    _gb_layout_column++;
        
    m_decay_f   =  _get_knob(zero_to_one); //newQDial(  1, 100,  1,  25); // s * 100
    m_decayLabel_f   = new QLabel(this);
    _add_knob(_gb_adsr_f_layout, _gb_layout_column, _gb_layout_row, "Decay",m_decay_f, m_decayLabel_f);
    connect(m_decay_f,   SIGNAL(valueChanged(int)), this, SLOT(decay_fChanged(int)));
    decay_fChanged  (m_decay_f ->value());
    
    _gb_layout_column++;
    
    m_sustain_f =  _get_knob(decibels_0); // newQDial(  0, 100,  1,  75); // %
    m_sustainLabel_f = new QLabel(this);
    _add_knob(_gb_adsr_f_layout, _gb_layout_column, _gb_layout_row, "Sustain", m_sustain_f, m_sustainLabel_f);
    connect(m_sustain_f, SIGNAL(valueChanged(int)), this, SLOT(sustain_fChanged(int)));
    sustain_fChanged(m_sustain_f->value());
    
    _gb_layout_column++;
    
    m_release_f = _get_knob(zero_to_four); //newQDial(  1, 400, 10, 200); // s * 100
    m_releaseLabel_f = new QLabel(this);
    _add_knob(_gb_adsr_f_layout, _gb_layout_column, _gb_layout_row, "Release", m_release_f, m_releaseLabel_f);
    connect(m_release_f, SIGNAL(valueChanged(int)), this, SLOT(release_fChanged(int)));
    release_fChanged(m_release_f->value());
        
    _gb_layout_column++;
    
    layout->addWidget(_gb_adsr_f, _row, _column, Qt::AlignCenter);    
    
    
    
    _column++;
    
    
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



void SynthGUI::_add_knob(QGridLayout * _layout, int position_x, int position_y, QString _label_text, QDial * _knob,
    QLabel * _label)
{    
    int _real_pos_y = (position_y) * 3;  // + 1;  ????
    QLabel * _knob_title = new QLabel(_label_text,  this);
    _knob_title->setMinimumWidth(60);
    _knob_title->setAlignment(Qt::AlignCenter);
    _knob_title->setStyleSheet("background-color: white; border: 1px solid black;  border-radius: 6px;");
    
    _layout->addWidget(_knob_title, (_real_pos_y), position_x, Qt::AlignCenter);    
    _layout->addWidget(_knob,  (_real_pos_y + 1), position_x);
    _layout->addWidget(_label,  (_real_pos_y + 2), position_x, Qt::AlignCenter);     
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

QDial * SynthGUI::_get_knob(_knob_type _ktype)
{
    switch(_ktype)
    {
        case decibels_0:
            return newQDial(-60, 0, 1, -6);
        case decibels_plus_12:
            return newQDial(  -60, 12,  1,  -6);
        case decibels_plus_24:
            return newQDial(  -60, 24,  1,  -6);            
        case decibels_plus_6:            
            return newQDial(  -60, 6,  1,  -6);
        case pitch:
            return newQDial(  20, 124,  1,  105);
        case zero_to_four:
            return newQDial(  1, 400,  4,  75);
        case zero_to_one:
            return newQDial(  1, 100,  1,  15);
        case zero_to_two:
            return newQDial(  1, 200,  2,  25);        
    }
    
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
    dial->setNotchesVisible(true);    
    dial->setMaximumHeight(66);
    dial->setMaximumWidth(66);
    
    //dial->setFocusPolicy(Qt::NoFocus);
    
    return dial;
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
    m_sustain->setValue(int(val) * 100);
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

void SynthGUI::setAttack_f (float sec)
{
    m_suppressHostUpdate = true;
    m_attack_f->setValue(int(sec * 100));
    m_suppressHostUpdate = false;
}

void SynthGUI::setDecay_f  (float sec)
{
    m_suppressHostUpdate = true;
    m_decay_f->setValue(int(sec * 100));
    m_suppressHostUpdate = false;
}

void SynthGUI::setSustain_f(float val)
{
    m_suppressHostUpdate = true;
    m_sustain_f->setValue(int(val * 100));
    m_suppressHostUpdate = false;
}

void SynthGUI::setRelease_f(float sec)
{
    m_suppressHostUpdate = true;
    m_release_f->setValue(int(sec * 100));
    m_suppressHostUpdate = false;
}

/*Standard handlers for the audio slots, these perform manipulations of knob values
 that are common in audio applications*/

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
    float val = float(value); // / 100.0;
    m_distLabel->setText(QString("%1").arg(val));

    if (!m_suppressHostUpdate) {
	lo_send(m_host, m_controlPath, "if", LTS_PORT_DIST, val);
    }
}


void SynthGUI::attack_fChanged(int value)
{
    _changed_seconds(value,m_attackLabel_f,LTS_PORT_ATTACK_F);
}

void
SynthGUI::decay_fChanged(int value)
{
    _changed_seconds(value,m_decayLabel_f,LTS_PORT_DECAY_F);
}

void SynthGUI::sustain_fChanged(int value)
{
    _changed_decibels(value, m_sustainLabel_f, LTS_PORT_SUSTAIN_F);    
}

void SynthGUI::release_fChanged(int value)
{
    _changed_seconds(value, m_releaseLabel_f, LTS_PORT_RELEASE_F);    
}


void SynthGUI::test_press()
{
    unsigned char noteon[4] = { 0x00, 0x90, 0x3C, 0x40 };

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
    unsigned char noteoff[4] = { 0x00, 0x90, 0x3C, 0x00 };

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
    cerr << "Error: liblo server error " << num
	 << " in path \"" << (path ? path : "(null)")
	 << "\": " << msg << endl;
}

int debug_handler(const char *path, const char *types, lo_arg **argv,
	      int argc, void *data, void *user_data)
{
    int i;

    cerr << "Warning: unhandled OSC message in GUI:" << endl;

    for (i = 0; i < argc; ++i) {
	cerr << "arg " << i << ": type '" << types[i] << "': ";
        lo_arg_pp((lo_type)types[i], argv[i]);
	cerr << endl;
    }

    cerr << "(path is <" << path << ">)" << endl;
    return 1;
}

int program_handler(const char *path, const char *types, lo_arg **argv,
	       int argc, void *data, void *user_data)
{
    cerr << "Program handler not yet implemented" << endl;
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

    case LTS_PORT_ATTACK_F:
	cerr << "gui setting attack to " << value << endl;
	gui->setAttack_f(value);
	break;

    case LTS_PORT_DECAY_F:
	cerr << "gui setting decay to " << value << endl;
	gui->setDecay_f(value);
	break;

    case LTS_PORT_SUSTAIN_F:
	cerr << "gui setting sustain to " << value << endl;
	gui->setSustain_f(value);
	break;

    case LTS_PORT_RELEASE_F:
	cerr << "gui setting release to " << value << endl;
	gui->setRelease_f(value);
	break;


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

