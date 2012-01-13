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

#define LTS_PORT_FREQ    1
#define LTS_PORT_ATTACK  2
#define LTS_PORT_DECAY   3
#define LTS_PORT_SUSTAIN 4
#define LTS_PORT_RELEASE 5
#define LTS_PORT_TIMBRE  6
#define LTS_PORT_RES  7
#define LTS_PORT_DIST  8

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
        
    m_tuning  = newQDial(100, 600, 10, 400); // (Hz - 400) * 10
    m_attack  = newQDial(  1, 100,  1,  25); // s * 100
    m_decay   = newQDial(  1, 100,  1,  25); // s * 100
    m_sustain = newQDial(  0, 100,  1,  75); // %
    m_release = newQDial(  1, 400, 10, 200); // s * 100
    /*Updated for LibModSynth testing*/
    m_timbre  = newQDial(  39, 136,  1,  82); // s * 100
    m_res  = newQDial(  -50, 0,  1,  -36); 
    m_dist  = newQDial(  -6, 36,  1,  -6); 
    
    m_tuningLabel  = new QLabel(this);
    m_attackLabel  = new QLabel(this);
    m_decayLabel   = new QLabel(this);
    m_sustainLabel = new QLabel(this);
    m_releaseLabel = new QLabel(this);
    m_timbreLabel  = new QLabel(this);
    m_resLabel  = new QLabel(this);
    m_distLabel  = new QLabel(this);

    layout->addWidget(new QLabel("Pitch of A", this), 0, 0, Qt::AlignCenter);
    layout->addWidget(new QLabel("Attack",     this), 0, 1, Qt::AlignCenter);
    layout->addWidget(new QLabel("Decay",      this), 0, 2, Qt::AlignCenter);
    layout->addWidget(new QLabel("Sustain",    this), 0, 3, Qt::AlignCenter);
    layout->addWidget(new QLabel("Release",    this), 0, 4, Qt::AlignCenter);
    layout->addWidget(new QLabel("Timbre",     this), 0, 5, Qt::AlignCenter);
    layout->addWidget(new QLabel("Res",     this), 0, 6, Qt::AlignCenter);
    layout->addWidget(new QLabel("Dist",     this), 0, 7, Qt::AlignCenter);
    
    layout->addWidget(m_tuning,  1, 0);
    layout->addWidget(m_attack,  1, 1);
    layout->addWidget(m_decay,   1, 2);
    layout->addWidget(m_sustain, 1, 3);
    layout->addWidget(m_release, 1, 4);
    layout->addWidget(m_timbre,  1, 5);
    layout->addWidget(m_res,  1, 6);
    layout->addWidget(m_dist,  1, 7);
    
    layout->addWidget(m_tuningLabel,  2, 0, Qt::AlignCenter);
    layout->addWidget(m_attackLabel,  2, 1, Qt::AlignCenter);
    layout->addWidget(m_decayLabel,   2, 2, Qt::AlignCenter);
    layout->addWidget(m_sustainLabel, 2, 3, Qt::AlignCenter);
    layout->addWidget(m_releaseLabel, 2, 4, Qt::AlignCenter);
    layout->addWidget(m_timbreLabel,  2, 5, Qt::AlignCenter);
    layout->addWidget(m_resLabel,  2, 6, Qt::AlignCenter);
    layout->addWidget(m_distLabel,  2, 7, Qt::AlignCenter);

    connect(m_tuning,  SIGNAL(valueChanged(int)), this, SLOT(tuningChanged(int)));
    connect(m_attack,  SIGNAL(valueChanged(int)), this, SLOT(attackChanged(int)));
    connect(m_decay,   SIGNAL(valueChanged(int)), this, SLOT(decayChanged(int)));
    connect(m_sustain, SIGNAL(valueChanged(int)), this, SLOT(sustainChanged(int)));
    connect(m_release, SIGNAL(valueChanged(int)), this, SLOT(releaseChanged(int)));
    connect(m_timbre,  SIGNAL(valueChanged(int)), this, SLOT(timbreChanged(int)));
    connect(m_res,  SIGNAL(valueChanged(int)), this, SLOT(resChanged(int)));
    connect(m_dist,  SIGNAL(valueChanged(int)), this, SLOT(distChanged(int)));

    // cause some initial updates
    tuningChanged (m_tuning ->value());
    attackChanged (m_attack ->value());
    decayChanged  (m_decay  ->value());
    sustainChanged(m_sustain->value());
    releaseChanged(m_release->value());
    timbreChanged (m_timbre ->value());
    resChanged (m_res ->value());
    distChanged (m_dist ->value());

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
SynthGUI::setTuning(float hz)
{
    m_suppressHostUpdate = true;
    m_tuning->setValue(int((hz - 400.0) * 10.0));
    m_suppressHostUpdate = false;
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

void
SynthGUI::tuningChanged(int value)
{
    float hz = float(value) / 10.0 + 400.0;
    m_tuningLabel->setText(QString("%1 Hz").arg(hz));

    if (!m_suppressHostUpdate) {
	cerr << "Sending to host: " << m_controlPath
	     << " port " << LTS_PORT_FREQ << " to " << hz << endl;
	lo_send(m_host, m_controlPath, "if", LTS_PORT_FREQ, hz);
    }
}

void
SynthGUI::attackChanged(int value)
{
    float sec = float(value) / 100.0;
    m_attackLabel->setText(QString("%1 sec").arg(sec));

    if (!m_suppressHostUpdate) {
	lo_send(m_host, m_controlPath, "if", LTS_PORT_ATTACK, sec);
    }
}

void
SynthGUI::decayChanged(int value)
{
    float sec = float(value) / 100.0;
    m_decayLabel->setText(QString("%1 sec").arg(sec));

    if (!m_suppressHostUpdate) {
	lo_send(m_host, m_controlPath, "if", LTS_PORT_DECAY, sec);
    }
}

void
SynthGUI::sustainChanged(int value)
{
    m_sustainLabel->setText(QString("%1 %").arg(value));

    if (!m_suppressHostUpdate) {
	lo_send(m_host, m_controlPath, "if", LTS_PORT_SUSTAIN, float(value));
    }
}

void
SynthGUI::releaseChanged(int value)
{
    float sec = float(value) / 100.0;
    m_releaseLabel->setText(QString("%1 sec").arg(sec));

    if (!m_suppressHostUpdate) {
	lo_send(m_host, m_controlPath, "if", LTS_PORT_RELEASE, sec);
    }
}

void
SynthGUI::timbreChanged(int value)
{
    float val = float(value); // / 100.0;
    m_timbreLabel->setText(QString("%1").arg(val));

    if (!m_suppressHostUpdate) {
	lo_send(m_host, m_controlPath, "if", LTS_PORT_TIMBRE, val);
    }
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

    case LTS_PORT_FREQ:
	cerr << "gui setting frequency to " << value << endl;
	gui->setTuning(value);
	break;

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
    cerr << "less_trivial_synth_qt_gui starting..." << endl;

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

