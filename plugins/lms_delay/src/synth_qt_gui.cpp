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
#include <QTimer>
#include <iostream>
#include <unistd.h>

#include <QGroupBox>
#include <QLabel>
#include <QGridLayout>
#include <QGroupBox>
#include <QDial>
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
    
    this->setWindowTitle(QString("LMS Delay  Powered by LibModSynth."));
    
    /*Set the CSS style that will "cascade" on the other controls.  Other control's styles can be overridden by running their own setStyleSheet method*/
    this->setStyleSheet("QLabel {color: white} QPushButton {background-color: black; border-style: outset; border-width: 2px; border-radius: 10px;border-color: white;font: bold 14px; min-width: 10em; padding: 6px; color:white;}  QAbstractItemView {outline: none;} QComboBox{border:1px solid white;border-radius:3px; padding:1px;background-color:black;color:white} QComboBox::drop-down{color:white;background-color:black;padding:2px;border-radius:2px;} QDial{background-color:rgb(152, 152, 152);} QFrame{background-color:rgb(0,0,0);} QGroupBox {color: white; border: 2px solid gray;  border-radius: 10px;  margin-top: 1ex; } QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 3px;} QMessageBox{color:white;background-color:black;}");
    
    main_layout = new LMS_main_layout(this);
    
    LMS_style_info * f_info = new LMS_style_info(60);
    
    delay_groupbox = new LMS_group_box(this, QString("Delay"), f_info);
    main_layout->lms_add_widget(delay_groupbox->lms_groupbox);
    
    m_delaytime  = new LMS_knob_regular(QString("Time"), 10, 100, 1, 50, QString(""), this, f_info, lms_kc_decimal, LMS_DELAY_TIME);
    delay_groupbox->lms_add_h(m_delaytime);
    connect(m_delaytime->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(delayTimeChanged(int)));
        
    m_feedback = new LMS_knob_regular(QString("Feedback"), -20, 0, 1, -12, QString(""), this, f_info, lms_kc_integer, LMS_FEEDBACK);
    delay_groupbox->lms_add_h(m_feedback);
    connect(m_feedback->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(feedbackChanged(int)));
    
    m_dry = new LMS_knob_regular(QString("Dry"), -30, 0, 1, 0, QString(""), this, f_info, lms_kc_integer, LMS_DRY);
    delay_groupbox->lms_add_h(m_dry);
    connect(m_dry->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(dryChanged(int)));
    
    m_wet = new LMS_knob_regular(QString("Wet"), -30, 0, 1, -6, QString(""), this, f_info, lms_kc_integer, LMS_WET);
    delay_groupbox->lms_add_h(m_wet);
    connect(m_wet->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(wetChanged(int)));
        
    m_duck = new LMS_knob_regular(QString("Duck"), -40, 0, 1, -6, QString(""), this, f_info, lms_kc_integer, LMS_DUCK);
    delay_groupbox->lms_add_h(m_duck);
    connect(m_duck->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(duckChanged(int)));
    
    m_cutoff = new LMS_knob_regular(QString("Cutoff"), 20, 124, 1, -6, QString(""), this, f_info, lms_kc_pitch, LMS_CUTOFF);
    delay_groupbox->lms_add_h(m_cutoff);
    connect(m_cutoff->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(cutoffChanged(int)));
    
    m_stereo = new LMS_knob_regular(QString("Stereo"), 0, 100, 1, 100, QString(""), this, f_info, lms_kc_decimal, LMS_STEREO);
    delay_groupbox->lms_add_h(m_stereo);
    connect(m_stereo->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(stereoChanged(int)));
        
    QGroupBox * f_gb_bpm = new QGroupBox(this); 
    f_gb_bpm->setTitle(QString("Tempo Sync"));
    f_gb_bpm->setAlignment(Qt::AlignHCenter);    
    QGridLayout *f_gb_bpm_layout = new QGridLayout(f_gb_bpm);
    
    m_bpm_spinbox = new QDoubleSpinBox(this);
    m_bpm_spinbox->setGeometry(QRect(100, 130, 71, 27));
    m_bpm_spinbox->setDecimals(1);
    m_bpm_spinbox->setMinimum(60);
    m_bpm_spinbox->setMaximum(200);
    m_bpm_spinbox->setSingleStep(0.1);
    
    QStringList f_beat_fracs = QStringList() << QString("1/4") << QString("1/3") << QString("1/2") << QString("2/3") << QString("3/4") << QString("1");
    
    m_beat_frac = new QComboBox(this); // get_combobox(f_beat_fracs, f_beat_fracs_count , this);
    m_beat_frac->addItems(f_beat_fracs);
    
    m_sync_bpm = new QPushButton(this);
    m_sync_bpm->setText("Sync");
    connect(m_sync_bpm, SIGNAL(pressed()), this, SLOT(bpmSyncPressed()));
    
    QLabel * f_bpm_label = new QLabel("BPM",  this);
    f_bpm_label->setMinimumWidth(60);
    f_bpm_label->setAlignment(Qt::AlignCenter);
    f_bpm_label->setStyleSheet("color: black; background-color: white; border: 1px solid black;  border-radius: 6px;");
    
    QLabel * f_beat_label = new QLabel("Beats",  this);
    f_beat_label->setMinimumWidth(60);
    f_beat_label->setAlignment(Qt::AlignCenter);
    f_beat_label->setStyleSheet("color: black; background-color: white; border: 1px solid black;  border-radius: 6px;");
    
    f_gb_bpm_layout->addWidget(f_bpm_label, 0, 0, Qt::AlignCenter);
    f_gb_bpm_layout->addWidget(m_bpm_spinbox, 1, 0, Qt::AlignCenter);
    f_gb_bpm_layout->addWidget(f_beat_label, 0, 1, Qt::AlignCenter);
    f_gb_bpm_layout->addWidget(m_beat_frac, 1, 1, Qt::AlignCenter);
    f_gb_bpm_layout->addWidget(m_sync_bpm, 2, 1, Qt::AlignCenter);
    
    main_layout->lms_add_widget(f_gb_bpm);
        
    /*End test button code, DO NOT remove the code below this*/

    QTimer *myTimer = new QTimer(this);
    connect(myTimer, SIGNAL(timeout()), this, SLOT(oscRecv()));
    myTimer->setSingleShot(false);
    myTimer->start(0);
    
    m_suppressHostUpdate = false;

}

void SynthGUI::lms_set_value(float val, LMS_control * a_ctrl)
{
    m_suppressHostUpdate = true;
    a_ctrl->lms_set_value(int(val));
    m_suppressHostUpdate = false;
}

void SynthGUI::lms_value_changed(int a_value, LMS_control * a_ctrl)
{
    a_ctrl->lms_value_changed(a_value);

    if (!m_suppressHostUpdate) {
        lo_send(m_host, m_controlPath, "if", (a_ctrl->lms_port), float(a_value));
    }
}

void SynthGUI::setDelayTime(float val){ lms_set_value(val, m_delaytime); }

void SynthGUI::setFeedback(float val){ lms_set_value(val, m_feedback); }

void SynthGUI::setDry(float val){ lms_set_value(val, m_dry); }

void SynthGUI::setWet(float val){ lms_set_value(val, m_wet); }

void SynthGUI::setDuck(float val){ lms_set_value(val, m_duck); }

void SynthGUI::setCutoff(float val){ lms_set_value(val, m_cutoff); }

void SynthGUI::setStereo(float val){ lms_set_value(val, m_stereo); }



void SynthGUI::delayTimeChanged(int value){ lms_value_changed(value, m_delaytime); }

void SynthGUI::feedbackChanged(int value){ lms_value_changed(value, m_feedback); }

void SynthGUI::dryChanged(int value){ lms_value_changed(value, m_dry); }

void SynthGUI::wetChanged(int value){ lms_value_changed(value, m_wet); }

void SynthGUI::duckChanged(int value){ lms_value_changed(value, m_duck); }

void SynthGUI::cutoffChanged(int value){ lms_value_changed(value, m_cutoff); }

void SynthGUI::stereoChanged(int value){ lms_value_changed(value, m_stereo); }

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
    
    m_delaytime->lms_set_value(f_result);
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
            return m_delaytime->lms_get_value();
        case LMS_FEEDBACK:
            return m_feedback->lms_get_value();
        case LMS_DRY:
            return m_dry->lms_get_value();
        case LMS_WET:
            return m_wet->lms_get_value();
        case LMS_DUCK:
            return m_duck->lms_get_value();
        case LMS_CUTOFF:
            return m_cutoff->lms_get_value();
        case LMS_STEREO:
            return m_stereo->lms_get_value();
        default:
            cerr << "Warning: received request to get nonexistent port " << a_port << endl;
            return 0;
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

