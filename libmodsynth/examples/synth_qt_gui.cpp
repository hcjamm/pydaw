/* -*- c-basic-offset: 4 -*-  vi:set ts=8 sts=4 sw=4: */

/* less_trivial_synth_qt_gui.cpp

   DSSI Soft Synth Interface
   Constructed by Chris Cannam and Steve Harris

   This is an example Qt GUI for an example DSSI synth plugin.

   This example file is in the public domain.
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

//#define LTS_PORT_FREQ    1
#define LTS_PORT_ATTACK  1
#define LTS_PORT_DECAY   2
#define LTS_PORT_SUSTAIN 3
#define LTS_PORT_RELEASE 4
#define LTS_PORT_TIMBRE  5
#define LTS_PORT_RES  6
#define LTS_PORT_DIST  7

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

    QGridLayout *layout = new QGridLayout(this);
    
    int _row = 0;
    int _column = 0;
          
    m_attack = _get_knob(zero_to_one);
    m_attackLabel = new QLabel(this);
    _add_knob(layout, _column, _row, "Attack",m_attack, m_attackLabel);
    connect(m_attack,   SIGNAL(valueChanged(int)), this, SLOT(attackChanged(int)));
    attackChanged  (m_attack  ->value());
    
    _column++;
    
    m_decay   =  _get_knob(zero_to_one); //newQDial(  1, 100,  1,  25); // s * 100
    m_decayLabel   = new QLabel(this);
    _add_knob(layout, _column, _row, "Decay",m_decay, m_decayLabel);
    /*
    layout->addWidget(new QLabel("Decay",      this), 0, 2, Qt::AlignCenter);
    layout->addWidget(m_decay,   1, 2);    
    layout->addWidget(m_decayLabel,   2, 2, Qt::AlignCenter);
    */
    connect(m_decay,   SIGNAL(valueChanged(int)), this, SLOT(decayChanged(int)));
    decayChanged  (m_decay  ->value());
    
    _column++;
    
    m_sustain =  _get_knob(decibels_0); // newQDial(  0, 100,  1,  75); // %
    m_sustainLabel = new QLabel(this);
    _add_knob(layout, _column, _row, "Sustain", m_sustain, m_sustainLabel);
    /*
    layout->addWidget(new QLabel("Sustain",    this), 0, _column, Qt::AlignCenter);
    layout->addWidget(m_sustain, 1, _column);
    layout->addWidget(m_sustainLabel, 2, _column, Qt::AlignCenter);
    */    
    connect(m_sustain, SIGNAL(valueChanged(int)), this, SLOT(sustainChanged(int)));
    sustainChanged(m_sustain->value());
    
    _column++;
    
    
    m_release = _get_knob(zero_to_four); //newQDial(  1, 400, 10, 200); // s * 100
    m_releaseLabel = new QLabel(this);
    layout->addWidget(new QLabel("Release",    this), 0, _column, Qt::AlignCenter);
    layout->addWidget(m_release, 1, _column);
    layout->addWidget(m_releaseLabel, 2, _column, Qt::AlignCenter);
    connect(m_release, SIGNAL(valueChanged(int)), this, SLOT(releaseChanged(int)));
    releaseChanged(m_release->value());
    
    _column++;
    
        
    m_timbre  =  _get_knob(pitch);  //newQDial(  39, 136,  1,  82); // s * 100
    m_timbreLabel  = new QLabel(this);
    _add_knob(layout, _column, _row, "Timbre",m_timbre, m_timbreLabel);
    /*
    layout->addWidget(new QLabel("Timbre",     this), 0, 5, Qt::AlignCenter);
    layout->addWidget(m_timbre,  1, 5);
    layout->addWidget(m_timbreLabel,  2, 5, Qt::AlignCenter);
    */
    connect(m_timbre,  SIGNAL(valueChanged(int)), this, SLOT(timbreChanged(int)));
    timbreChanged (m_timbre ->value());
    
    _column++;
    
    
    m_res  = newQDial(  -50, 0,  1,  -36); 
    m_resLabel  = new QLabel(this);
    layout->addWidget(new QLabel("Res",     this), 0, 6, Qt::AlignCenter);
    layout->addWidget(m_res,  1, 6);
    layout->addWidget(m_resLabel,  2, 6, Qt::AlignCenter);
    connect(m_res,  SIGNAL(valueChanged(int)), this, SLOT(resChanged(int)));
    resChanged (m_res ->value());
    
    _column++;
    
    
    m_dist  = newQDial(  -6, 36,  1,  -6); 
    m_distLabel  = new QLabel(this);
    layout->addWidget(new QLabel("Dist",     this), 0, 7, Qt::AlignCenter);
    layout->addWidget(m_dist,  1, 7);
    layout->addWidget(m_distLabel,  2, 7, Qt::AlignCenter);
    connect(m_dist,  SIGNAL(valueChanged(int)), this, SLOT(distChanged(int)));
    distChanged (m_dist ->value());
    
    _column++;
    
    //Start a new row
    _row++;
    _column = 0;

    /*add some GUI elements*/
    
    //_column++;
    
    QPushButton *testButton = new QPushButton("Test", this);
    connect(testButton, SIGNAL(pressed()), this, SLOT(test_press()));
    connect(testButton, SIGNAL(released()), this, SLOT(test_release()));
    layout->addWidget(testButton, 3, 7, Qt::AlignCenter);

    QTimer *myTimer = new QTimer(this);
    connect(myTimer, SIGNAL(timeout()), this, SLOT(oscRecv()));
    myTimer->setSingleShot(false);
    myTimer->start(0);
    
    m_suppressHostUpdate = false;
}



void
SynthGUI::_add_knob(QGridLayout * _layout, int position_x, int position_y, std::string _label_text, QDial * _knob,
    QLabel * _label)
{    
    int _real_pos_y = (position_y) * 3;  // + 1;  ????
    
    cerr << "gui _add_knob _real_pos_y == " << _real_pos_y << endl;
    
    _layout->addWidget(new QLabel(QString::fromStdString(_label_text),     this), (_real_pos_y), position_x, Qt::AlignCenter);    
    cerr << "gui _add_knob addWidget new QLabel " << endl;
    
    _layout->addWidget(_knob,  (_real_pos_y + 1), position_x);
    cerr << "gui _add_knob addWidget _knob " << endl;
    
    _layout->addWidget(_label,  (_real_pos_y + 2), position_x, Qt::AlignCenter);        
    cerr << "gui _add_knob addWidget _label " << endl;    
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


void
SynthGUI::setAttack(float sec)
{
    m_suppressHostUpdate = true;
    m_attack->setValue(int(sec * 100));
    m_suppressHostUpdate = false;
}

void
SynthGUI::setDecay(float sec)
{
    m_suppressHostUpdate = true;
    m_decay->setValue(int(sec * 100));
    m_suppressHostUpdate = false;
}

void
SynthGUI::setSustain(float percent)
{
    m_suppressHostUpdate = true;
    m_sustain->setValue(int(percent));
    m_suppressHostUpdate = false;
}

void
SynthGUI::setRelease(float sec)
{
    m_suppressHostUpdate = true;
    m_release->setValue(int(sec * 100));
    m_suppressHostUpdate = false;
}

void
SynthGUI::setTimbre(float val)
{
    m_suppressHostUpdate = true;
    m_timbre->setValue(int(val));  // * 100));
    m_suppressHostUpdate = false;
}

void
SynthGUI::setRes(float val)
{
    m_suppressHostUpdate = true;
    m_res->setValue(int(val));  // * 100));
    m_suppressHostUpdate = false;
}

void
SynthGUI::setDist(float val)
{
    m_suppressHostUpdate = true;
    m_dist->setValue(int(val));  // * 100));
    m_suppressHostUpdate = false;
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
    _label->setText(QString("%1 hz").arg(_hz));
    
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



void
SynthGUI::attackChanged(int value)
{
    _changed_seconds(value,m_attackLabel,LTS_PORT_ATTACK);
    
    /*
    float sec = float(value) * .01;  //  / 100.0;
    m_attackLabel->setText(QString("%1 sec").arg(sec));

    if (!m_suppressHostUpdate) {
	lo_send(m_host, m_controlPath, "if", LTS_PORT_ATTACK, sec);
    }
    */
}

void
SynthGUI::decayChanged(int value)
{
    _changed_seconds(value,m_decayLabel,LTS_PORT_DECAY);
    
    /*
    float sec = float(value) * .01;  // / 100.0;
    m_decayLabel->setText(QString("%1 sec").arg(sec));

    if (!m_suppressHostUpdate) {
	lo_send(m_host, m_controlPath, "if", LTS_PORT_DECAY, sec);
    }
    */
}

void
SynthGUI::sustainChanged(int value)
{
    _changed_decibels(value, m_sustainLabel, LTS_PORT_SUSTAIN);
    
    /*
    m_sustainLabel->setText(QString("%1 %").arg(value));

    if (!m_suppressHostUpdate) {
	lo_send(m_host, m_controlPath, "if", LTS_PORT_SUSTAIN, float(value));
    }
    */
}

void
SynthGUI::releaseChanged(int value)
{
    _changed_seconds(value, m_releaseLabel, LTS_PORT_RELEASE);
    
    /*
    float sec = float(value) * .01; // / 100.0;
    m_releaseLabel->setText(QString("%1 sec").arg(sec));

    if (!m_suppressHostUpdate) {
	lo_send(m_host, m_controlPath, "if", LTS_PORT_RELEASE, sec);
    }
    */
}

void
SynthGUI::timbreChanged(int value)
{
    _changed_pitch(value, m_timbreLabel, LTS_PORT_TIMBRE);
    /*
    float val = float(value); // / 100.0;
    m_timbreLabel->setText(QString("%1").arg(val));

    if (!m_suppressHostUpdate) {
	lo_send(m_host, m_controlPath, "if", LTS_PORT_TIMBRE, val);
    }
    */
}

void
SynthGUI::resChanged(int value)
{
    float val = float(value); // / 100.0;
    m_resLabel->setText(QString("%1").arg(val));

    if (!m_suppressHostUpdate) {
	lo_send(m_host, m_controlPath, "if", LTS_PORT_RES, val);
    }
}

void
SynthGUI::distChanged(int value)
{
    float val = float(value); // / 100.0;
    m_distLabel->setText(QString("%1").arg(val));

    if (!m_suppressHostUpdate) {
	lo_send(m_host, m_controlPath, "if", LTS_PORT_DIST, val);
    }
}

void
SynthGUI::test_press()
{
    unsigned char noteon[4] = { 0x00, 0x90, 0x3C, 0x40 };

    lo_send(m_host, m_midiPath, "m", noteon);
}

void
SynthGUI::oscRecv()
{
    if (osc_server) {
	lo_server_recv_noblock(osc_server, 1);
    }
}

void
SynthGUI::test_release()
{
    unsigned char noteoff[4] = { 0x00, 0x90, 0x3C, 0x00 };

    lo_send(m_host, m_midiPath, "m", noteoff);
}

void
SynthGUI::aboutToQuit()
{
    if (!m_hostRequestedQuit) lo_send(m_host, m_exitingPath, "");
}

SynthGUI::~SynthGUI()
{
    lo_address_free(m_host);
}


QDial *
SynthGUI::newQDial( int minValue, int maxValue, int pageStep, int value )
{
    QDial *dial = new QDial( this );
    dial->setMinimum( minValue );
    dial->setMaximum( maxValue );
    dial->setPageStep( pageStep );
    dial->setValue( value );
    dial->setNotchesVisible(true);
    dial->setGeometry(0,0,30,30);
    return dial;
}

void
osc_error(int num, const char *msg, const char *path)
{
    cerr << "Error: liblo server error " << num
	 << " in path \"" << (path ? path : "(null)")
	 << "\": " << msg << endl;
}

int
debug_handler(const char *path, const char *types, lo_arg **argv,
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

int
program_handler(const char *path, const char *types, lo_arg **argv,
	       int argc, void *data, void *user_data)
{
    cerr << "Program handler not yet implemented" << endl;
    return 0;
}

int
configure_handler(const char *path, const char *types, lo_arg **argv,
		  int argc, void *data, void *user_data)
{
    return 0;
}

int
rate_handler(const char *path, const char *types, lo_arg **argv,
	     int argc, void *data, void *user_data)
{
    return 0; /* ignore it */
}

int
show_handler(const char *path, const char *types, lo_arg **argv,
	     int argc, void *data, void *user_data)
{
    SynthGUI *gui = static_cast<SynthGUI *>(user_data);
    while (!gui->ready()) sleep(1);
    if (gui->isVisible()) gui->raise();
    else gui->show();
    return 0;
}

int
hide_handler(const char *path, const char *types, lo_arg **argv,
	     int argc, void *data, void *user_data)
{
    SynthGUI *gui = static_cast<SynthGUI *>(user_data);
    gui->hide();
    return 0;
}

int
quit_handler(const char *path, const char *types, lo_arg **argv,
	     int argc, void *data, void *user_data)
{
    SynthGUI *gui = static_cast<SynthGUI *>(user_data);
    gui->setHostRequestedQuit(true);
    qApp->quit();
    return 0;
}

int
control_handler(const char *path, const char *types, lo_arg **argv,
		int argc, void *data, void *user_data)
{
    SynthGUI *gui = static_cast<SynthGUI *>(user_data);

    if (argc < 2) {
	cerr << "Error: too few arguments to control_handler" << endl;
	return 1;
    }

    const int port = argv[0]->i;
    const float value = argv[1]->f;

    switch (port) {
/*
    case LTS_PORT_FREQ:
	cerr << "gui setting frequency to " << value << endl;
	gui->setTuning(value);
	break;
*/
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
        
    default:
	cerr << "Warning: received request to set nonexistent port " << port << endl;
    }

    return 0;
}

int
main(int argc, char **argv)
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

