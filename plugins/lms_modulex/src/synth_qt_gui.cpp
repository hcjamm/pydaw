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
#include <QTextStream>
#include <QTimer>
#include <iostream>
#include <unistd.h>

#include <QPixmap>
#include <QFile>
#include <QDir>
#include <QMessageBox>

#include <stdlib.h>

#include "synth.h"
#include "meta.h"

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/SM/SMlib.h>

static int modulex_handle_x11_error(Display *dpy, XErrorEvent *err)
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

lo_server modulex_osc_server = 0;

static QTextStream modulex_cerr(stderr);

modulex_gui::modulex_gui(const char * host, const char * port,
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
    
    QFile f_stylesheet_file("/usr/lib/pydaw2/themes/default/style.txt");
    f_stylesheet_file.open(QIODevice::ReadOnly | QIODevice::Text);
    this->setStyleSheet(f_stylesheet_file.readAll());
    f_stylesheet_file.close();

    m_main_vboxlayout = new QVBoxLayout();
    this->setLayout(m_main_vboxlayout);
    m_tab_widget = new QTabWidget(this);    
    m_fx_tab = new QWidget(this);
    m_delay_tab = new QWidget(this);
    m_main_vboxlayout->addWidget(m_tab_widget);
    m_tab_widget->addTab(m_fx_tab, QString("FX"));
    m_tab_widget->addTab(m_delay_tab, QString("Delay"));
        
    m_main_layout = new LMS_main_layout(m_fx_tab);
    m_delay_layout = new LMS_main_layout(m_delay_tab);
    
    LMS_style_info * f_info = new LMS_style_info(51);
    //f_info->LMS_set_label_style("QLabel{background-color: white; border: 1px solid black;  border-radius: 6px;}", 60);
    //f_info->LMS_set_value_style(QString("color : white; background-color: rgba(0,0,0,0);"), 64);
    
    m_fx0 = new LMS_multieffect(this, QString("FX1"), f_info, MODULEX_FX0_KNOB0, MODULEX_FX0_KNOB1, MODULEX_FX0_KNOB2, MODULEX_FX0_COMBOBOX);
    connect(m_fx0->lms_knob1->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx0knob0Changed(int)));
    connect(m_fx0->lms_knob2->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx0knob1Changed(int)));
    connect(m_fx0->lms_knob3->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx0knob2Changed(int)));
    connect(m_fx0->lms_combobox->lms_combobox,  SIGNAL(currentIndexChanged(int)), this, SLOT(fx0comboboxChanged(int)));
    
    m_main_layout->lms_add_widget(m_fx0->lms_groupbox->lms_groupbox);
        
    m_fx1 = new LMS_multieffect(this, QString("FX2"), f_info, MODULEX_FX1_KNOB0, MODULEX_FX1_KNOB1, MODULEX_FX1_KNOB2, MODULEX_FX1_COMBOBOX);
    connect(m_fx1->lms_knob1->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx1knob0Changed(int)));
    connect(m_fx1->lms_knob2->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx1knob1Changed(int)));
    connect(m_fx1->lms_knob3->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx1knob2Changed(int)));
    connect(m_fx1->lms_combobox->lms_combobox,  SIGNAL(currentIndexChanged(int)), this, SLOT(fx1comboboxChanged(int)));
    
    m_main_layout->lms_add_widget(m_fx1->lms_groupbox->lms_groupbox);
    m_main_layout->lms_add_layout();    
        
    m_fx2 = new LMS_multieffect(this, QString("FX3"), f_info, MODULEX_FX2_KNOB0, MODULEX_FX2_KNOB1, MODULEX_FX2_KNOB2, MODULEX_FX2_COMBOBOX);
    connect(m_fx2->lms_knob1->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx2knob0Changed(int)));
    connect(m_fx2->lms_knob2->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx2knob1Changed(int)));
    connect(m_fx2->lms_knob3->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx2knob2Changed(int)));
    connect(m_fx2->lms_combobox->lms_combobox,  SIGNAL(currentIndexChanged(int)), this, SLOT(fx2comboboxChanged(int)));
    
    m_main_layout->lms_add_widget(m_fx2->lms_groupbox->lms_groupbox);
   
    m_fx3 = new LMS_multieffect(this, QString("FX4"), f_info, MODULEX_FX3_KNOB0, MODULEX_FX3_KNOB1, MODULEX_FX3_KNOB2, MODULEX_FX3_COMBOBOX);
    connect(m_fx3->lms_knob1->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx3knob0Changed(int)));
    connect(m_fx3->lms_knob2->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx3knob1Changed(int)));
    connect(m_fx3->lms_knob3->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx3knob2Changed(int)));
    connect(m_fx3->lms_combobox->lms_combobox,  SIGNAL(currentIndexChanged(int)), this, SLOT(fx3comboboxChanged(int)));
    
    m_main_layout->lms_add_widget(m_fx3->lms_groupbox->lms_groupbox);
    m_main_layout->lms_add_layout();    
        
    m_fx4 = new LMS_multieffect(this, QString("FX5"), f_info, MODULEX_FX4_KNOB0, MODULEX_FX4_KNOB1, MODULEX_FX4_KNOB2, MODULEX_FX4_COMBOBOX);
    connect(m_fx4->lms_knob1->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx4knob0Changed(int)));
    connect(m_fx4->lms_knob2->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx4knob1Changed(int)));
    connect(m_fx4->lms_knob3->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx4knob2Changed(int)));
    connect(m_fx4->lms_combobox->lms_combobox,  SIGNAL(currentIndexChanged(int)), this, SLOT(fx4comboboxChanged(int)));
    
    m_main_layout->lms_add_widget(m_fx4->lms_groupbox->lms_groupbox);
            
    m_fx5 = new LMS_multieffect(this, QString("FX6"), f_info, MODULEX_FX5_KNOB0, MODULEX_FX5_KNOB1, MODULEX_FX5_KNOB2, MODULEX_FX5_COMBOBOX);
    connect(m_fx5->lms_knob1->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx5knob0Changed(int)));
    connect(m_fx5->lms_knob2->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx5knob1Changed(int)));
    connect(m_fx5->lms_knob3->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx5knob2Changed(int)));
    connect(m_fx5->lms_combobox->lms_combobox,  SIGNAL(currentIndexChanged(int)), this, SLOT(fx5comboboxChanged(int)));
    
    m_main_layout->lms_add_widget(m_fx5->lms_groupbox->lms_groupbox);
    m_main_layout->lms_add_layout();    
    
    m_fx6 = new LMS_multieffect(this, QString("FX7"), f_info, MODULEX_FX6_KNOB0, MODULEX_FX6_KNOB1, MODULEX_FX6_KNOB2, MODULEX_FX6_COMBOBOX);
    connect(m_fx6->lms_knob1->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx6knob0Changed(int)));
    connect(m_fx6->lms_knob2->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx6knob1Changed(int)));
    connect(m_fx6->lms_knob3->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx6knob2Changed(int)));
    connect(m_fx6->lms_combobox->lms_combobox,  SIGNAL(currentIndexChanged(int)), this, SLOT(fx6comboboxChanged(int)));
    
    m_main_layout->lms_add_widget(m_fx6->lms_groupbox->lms_groupbox);

    m_fx7 = new LMS_multieffect(this, QString("FX8"), f_info, MODULEX_FX7_KNOB0, MODULEX_FX7_KNOB1, MODULEX_FX7_KNOB2, MODULEX_FX7_COMBOBOX);
    connect(m_fx7->lms_knob1->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx7knob0Changed(int)));
    connect(m_fx7->lms_knob2->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx7knob1Changed(int)));
    connect(m_fx7->lms_knob3->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx7knob2Changed(int)));
    connect(m_fx7->lms_combobox->lms_combobox,  SIGNAL(currentIndexChanged(int)), this, SLOT(fx7comboboxChanged(int)));
    
    m_main_layout->lms_add_widget(m_fx7->lms_groupbox->lms_groupbox);
    m_main_layout->lms_add_layout();
    
    
    delay_groupbox = new LMS_group_box(this, QString("Delay"), f_info);
    delay_groupbox->lms_groupbox->setMaximumHeight(150);
    m_delay_layout->lms_add_widget(delay_groupbox->lms_groupbox);
    
    m_delaytime  = new LMS_knob_regular(QString("Time"), 10, 100, 1, 50, QString(""), this, f_info, lms_kc_decimal, MODULEX_DELAY_TIME);
    delay_groupbox->lms_add_h(m_delaytime);
    connect(m_delaytime->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(delayTimeChanged(int)));
        
    m_feedback = new LMS_knob_regular(QString("Feedback"), -20, 0, 1, -12, QString(""), this, f_info, lms_kc_integer, MODULEX_FEEDBACK);
    delay_groupbox->lms_add_h(m_feedback);
    connect(m_feedback->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(feedbackChanged(int)));
    
    m_dry = new LMS_knob_regular(QString("Dry"), -30, 0, 1, 0, QString(""), this, f_info, lms_kc_integer, MODULEX_DRY);
    delay_groupbox->lms_add_h(m_dry);
    connect(m_dry->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(dryChanged(int)));
    
    m_wet = new LMS_knob_regular(QString("Wet"), -30, 0, 1, -6, QString(""), this, f_info, lms_kc_integer, MODULEX_WET);
    delay_groupbox->lms_add_h(m_wet);
    connect(m_wet->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(wetChanged(int)));
        
    m_duck = new LMS_knob_regular(QString("Duck"), -40, 0, 1, -6, QString(""), this, f_info, lms_kc_integer, MODULEX_DUCK);
    delay_groupbox->lms_add_h(m_duck);
    connect(m_duck->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(duckChanged(int)));
    
    m_cutoff = new LMS_knob_regular(QString("Cutoff"), 20, 124, 1, -6, QString(""), this, f_info, lms_kc_pitch, MODULEX_CUTOFF);
    delay_groupbox->lms_add_h(m_cutoff);
    connect(m_cutoff->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(cutoffChanged(int)));
    
    m_stereo = new LMS_knob_regular(QString("Stereo"), 0, 100, 1, 100, QString(""), this, f_info, lms_kc_decimal, MODULEX_STEREO);
    delay_groupbox->lms_add_h(m_stereo);
    connect(m_stereo->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(stereoChanged(int)));
        
    QGroupBox * f_gb_bpm = new QGroupBox(this); 
    f_gb_bpm->setMaximumHeight(150);
    f_gb_bpm->setTitle(QString("Tempo Sync"));
    f_gb_bpm->setAlignment(Qt::AlignHCenter);    
    QGridLayout *f_gb_bpm_layout = new QGridLayout(f_gb_bpm);
    
    m_bpm_spinbox = new QDoubleSpinBox(this);
    m_bpm_spinbox->setGeometry(QRect(100, 130, 71, 27));
    m_bpm_spinbox->setDecimals(1);
    m_bpm_spinbox->setMinimum(60);
    m_bpm_spinbox->setMaximum(200);
    m_bpm_spinbox->setSingleStep(0.1);
    m_bpm_spinbox->setValue(140.0f);
    
    QStringList f_beat_fracs = QStringList() << QString("1/4") << QString("1/3") << QString("1/2") << QString("2/3") << QString("3/4") << QString("1");
    
    m_beat_frac = new QComboBox(this);
    m_beat_frac->setMinimumWidth(75);
    m_beat_frac->addItems(f_beat_fracs);
    m_beat_frac->setCurrentIndex(2);
        
    m_sync_bpm = new QPushButton(this);
    m_sync_bpm->setText("Sync");
    m_sync_bpm->setMaximumWidth(60);
    connect(m_sync_bpm, SIGNAL(pressed()), this, SLOT(bpmSyncPressed()));
    
    QLabel * f_bpm_label = new QLabel("BPM",  this);
    f_bpm_label->setMinimumWidth(60);
    f_bpm_label->setAlignment(Qt::AlignCenter);
    //f_bpm_label->setStyleSheet("color: black; background-color: white; border: 1px solid black;  border-radius: 6px;");
    
    QLabel * f_beat_label = new QLabel("Beats",  this);
    f_beat_label->setMinimumWidth(60);
    f_beat_label->setAlignment(Qt::AlignCenter);
    //f_beat_label->setStyleSheet("color: black; background-color: white; border: 1px solid black;  border-radius: 6px;");
    
    f_gb_bpm_layout->addWidget(f_bpm_label, 0, 0, Qt::AlignCenter);
    f_gb_bpm_layout->addWidget(m_bpm_spinbox, 1, 0, Qt::AlignCenter);
    f_gb_bpm_layout->addWidget(f_beat_label, 0, 1, Qt::AlignCenter);
    f_gb_bpm_layout->addWidget(m_beat_frac, 1, 1, Qt::AlignCenter);
    f_gb_bpm_layout->addWidget(m_sync_bpm, 2, 1, Qt::AlignCenter);
    
    m_delay_layout->lms_add_widget(f_gb_bpm);
    m_delay_layout->lms_add_layout();
    m_delay_layout->lms_add_vertical_spacer();
    
    QTimer *myTimer = new QTimer(this);
    connect(myTimer, SIGNAL(timeout()), this, SLOT(oscRecv()));
    myTimer->setSingleShot(false);
    myTimer->start(0);
    
    m_suppressHostUpdate = false;

}

void modulex_gui::lms_set_value(float val, LMS_control * a_ctrl)
{
    m_suppressHostUpdate = true;
    a_ctrl->lms_set_value(int(val));
    m_suppressHostUpdate = false;
}

void modulex_gui::setFX0knob0(float val){ lms_set_value(val, m_fx0->lms_knob1); }
void modulex_gui::setFX0knob1(float val){ lms_set_value(val, m_fx0->lms_knob2); }
void modulex_gui::setFX0knob2(float val){ lms_set_value(val, m_fx0->lms_knob3); }
void modulex_gui::setFX0combobox(float val){ lms_set_value(val, m_fx0->lms_combobox); }

void modulex_gui::setFX1knob0(float val){ lms_set_value(val, m_fx1->lms_knob1); }
void modulex_gui::setFX1knob1(float val){ lms_set_value(val, m_fx1->lms_knob2); }
void modulex_gui::setFX1knob2(float val){ lms_set_value(val, m_fx1->lms_knob3); }
void modulex_gui::setFX1combobox(float val){ lms_set_value(val, m_fx1->lms_combobox); }

void modulex_gui::setFX2knob0(float val){ lms_set_value(val, m_fx2->lms_knob1); }
void modulex_gui::setFX2knob1(float val){ lms_set_value(val, m_fx2->lms_knob2); }
void modulex_gui::setFX2knob2(float val){ lms_set_value(val, m_fx2->lms_knob3); }
void modulex_gui::setFX2combobox(float val){ lms_set_value(val, m_fx2->lms_combobox); }

void modulex_gui::setFX3knob0(float val){ lms_set_value(val, m_fx3->lms_knob1); }
void modulex_gui::setFX3knob1(float val){ lms_set_value(val, m_fx3->lms_knob2); }
void modulex_gui::setFX3knob2(float val){ lms_set_value(val, m_fx3->lms_knob3); }
void modulex_gui::setFX3combobox(float val){ lms_set_value(val, m_fx3->lms_combobox); }


void modulex_gui::setFX4knob0(float val){ lms_set_value(val, m_fx4->lms_knob1); }
void modulex_gui::setFX4knob1(float val){ lms_set_value(val, m_fx4->lms_knob2); }
void modulex_gui::setFX4knob2(float val){ lms_set_value(val, m_fx4->lms_knob3); }
void modulex_gui::setFX4combobox(float val){ lms_set_value(val, m_fx4->lms_combobox); }

void modulex_gui::setFX5knob0(float val){ lms_set_value(val, m_fx5->lms_knob1); }
void modulex_gui::setFX5knob1(float val){ lms_set_value(val, m_fx5->lms_knob2); }
void modulex_gui::setFX5knob2(float val){ lms_set_value(val, m_fx5->lms_knob3); }
void modulex_gui::setFX5combobox(float val){ lms_set_value(val, m_fx5->lms_combobox); }

void modulex_gui::setFX6knob0(float val){ lms_set_value(val, m_fx6->lms_knob1); }
void modulex_gui::setFX6knob1(float val){ lms_set_value(val, m_fx6->lms_knob2); }
void modulex_gui::setFX6knob2(float val){ lms_set_value(val, m_fx6->lms_knob3); }
void modulex_gui::setFX6combobox(float val){ lms_set_value(val, m_fx6->lms_combobox); }

void modulex_gui::setFX7knob0(float val){ lms_set_value(val, m_fx7->lms_knob1); }
void modulex_gui::setFX7knob1(float val){ lms_set_value(val, m_fx7->lms_knob2); }
void modulex_gui::setFX7knob2(float val){ lms_set_value(val, m_fx7->lms_knob3); }
void modulex_gui::setFX7combobox(float val){ lms_set_value(val, m_fx7->lms_combobox); }


void modulex_gui::setDelayTime(float val){ lms_set_value(val, m_delaytime); }
void modulex_gui::setFeedback(float val){ lms_set_value(val, m_feedback); }
void modulex_gui::setDry(float val){ lms_set_value(val, m_dry); }
void modulex_gui::setWet(float val){ lms_set_value(val, m_wet); }
void modulex_gui::setDuck(float val){ lms_set_value(val, m_duck); }
void modulex_gui::setCutoff(float val){ lms_set_value(val, m_cutoff); }
void modulex_gui::setStereo(float val){ lms_set_value(val, m_stereo); }


void modulex_gui::lms_value_changed(int a_value, LMS_control * a_ctrl)
{
    a_ctrl->lms_value_changed(a_value);

    if (!m_suppressHostUpdate) {
        lo_send(m_host, m_controlPath, "if", (a_ctrl->lms_port), float(a_value));
    }
}

void modulex_gui::fx0knob0Changed(int value){ lms_value_changed(value, m_fx0->lms_knob1); }
void modulex_gui::fx0knob1Changed(int value){ lms_value_changed(value, m_fx0->lms_knob2); }
void modulex_gui::fx0knob2Changed(int value){ lms_value_changed(value, m_fx0->lms_knob3); }
void modulex_gui::fx0comboboxChanged(int value){ lms_value_changed(value, m_fx0->lms_combobox); m_fx0->lms_combobox_changed(); }

void modulex_gui::fx1knob0Changed(int value){ lms_value_changed(value, m_fx1->lms_knob1); }
void modulex_gui::fx1knob1Changed(int value){ lms_value_changed(value, m_fx1->lms_knob2); }
void modulex_gui::fx1knob2Changed(int value){ lms_value_changed(value, m_fx1->lms_knob3); }
void modulex_gui::fx1comboboxChanged(int value){ lms_value_changed(value, m_fx1->lms_combobox); m_fx1->lms_combobox_changed(); }

void modulex_gui::fx2knob0Changed(int value){ lms_value_changed(value, m_fx2->lms_knob1); }
void modulex_gui::fx2knob1Changed(int value){ lms_value_changed(value, m_fx2->lms_knob2); }
void modulex_gui::fx2knob2Changed(int value){ lms_value_changed(value, m_fx2->lms_knob3); }
void modulex_gui::fx2comboboxChanged(int value){ lms_value_changed(value, m_fx2->lms_combobox); m_fx2->lms_combobox_changed(); }

void modulex_gui::fx3knob0Changed(int value){ lms_value_changed(value, m_fx3->lms_knob1); }
void modulex_gui::fx3knob1Changed(int value){ lms_value_changed(value, m_fx3->lms_knob2); }
void modulex_gui::fx3knob2Changed(int value){ lms_value_changed(value, m_fx3->lms_knob3); }
void modulex_gui::fx3comboboxChanged(int value){ lms_value_changed(value, m_fx3->lms_combobox); m_fx3->lms_combobox_changed(); }

void modulex_gui::fx4knob0Changed(int value){ lms_value_changed(value, m_fx4->lms_knob1); }
void modulex_gui::fx4knob1Changed(int value){ lms_value_changed(value, m_fx4->lms_knob2); }
void modulex_gui::fx4knob2Changed(int value){ lms_value_changed(value, m_fx4->lms_knob3); }
void modulex_gui::fx4comboboxChanged(int value){ lms_value_changed(value, m_fx4->lms_combobox); m_fx4->lms_combobox_changed(); }

void modulex_gui::fx5knob0Changed(int value){ lms_value_changed(value, m_fx5->lms_knob1); }
void modulex_gui::fx5knob1Changed(int value){ lms_value_changed(value, m_fx5->lms_knob2); }
void modulex_gui::fx5knob2Changed(int value){ lms_value_changed(value, m_fx5->lms_knob3); }
void modulex_gui::fx5comboboxChanged(int value){ lms_value_changed(value, m_fx5->lms_combobox); m_fx5->lms_combobox_changed(); }

void modulex_gui::fx6knob0Changed(int value){ lms_value_changed(value, m_fx6->lms_knob1); }
void modulex_gui::fx6knob1Changed(int value){ lms_value_changed(value, m_fx6->lms_knob2); }
void modulex_gui::fx6knob2Changed(int value){ lms_value_changed(value, m_fx6->lms_knob3); }
void modulex_gui::fx6comboboxChanged(int value){ lms_value_changed(value, m_fx6->lms_combobox); m_fx6->lms_combobox_changed(); }

void modulex_gui::fx7knob0Changed(int value){ lms_value_changed(value, m_fx7->lms_knob1); }
void modulex_gui::fx7knob1Changed(int value){ lms_value_changed(value, m_fx7->lms_knob2); }
void modulex_gui::fx7knob2Changed(int value){ lms_value_changed(value, m_fx7->lms_knob3); }
void modulex_gui::fx7comboboxChanged(int value){ lms_value_changed(value, m_fx7->lms_combobox); m_fx7->lms_combobox_changed(); }


void modulex_gui::delayTimeChanged(int value){ lms_value_changed(value, m_delaytime); }
void modulex_gui::feedbackChanged(int value){ lms_value_changed(value, m_feedback); }
void modulex_gui::dryChanged(int value){ lms_value_changed(value, m_dry); }
void modulex_gui::wetChanged(int value){ lms_value_changed(value, m_wet); }
void modulex_gui::duckChanged(int value){ lms_value_changed(value, m_duck); }
void modulex_gui::cutoffChanged(int value){ lms_value_changed(value, m_cutoff); }
void modulex_gui::stereoChanged(int value){ lms_value_changed(value, m_stereo); }

void modulex_gui::bpmSyncPressed()
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



void modulex_gui::v_print_port_name_to_cerr(int a_port)
{
#ifdef LMS_DEBUG_MODE_QT
    switch (a_port) {
    case MODULEX_FX0_KNOB0:
	modulex_cerr << "LMS_FX1_KNOB1";
	break;
    case MODULEX_FX0_KNOB1:
	modulex_cerr << "LMS_FX1_KNOB2";
	break;        
    case LMS_DIST:
	modulex_cerr << "LMS_DIST";
	break;
    default:
	modulex_cerr << "Warning: received request to set nonexistent port " << a_port ;
        break;
    }
#endif
}

void modulex_gui::v_set_control(int a_port, float a_value)
{

#ifdef LMS_DEBUG_MODE_QT    
    modulex_cerr << "v_set_control called.  ";  
    v_print_port_name_to_cerr(a_port);
    modulex_cerr << "  value: " << a_value << endl;
#endif
    
    /*Add the controls you created to the control handler*/
    
    switch (a_port) 
    {
        case MODULEX_FX0_KNOB0:	setFX0knob0(a_value); break;
        case MODULEX_FX0_KNOB1:	setFX0knob1(a_value); break;        
        case MODULEX_FX0_KNOB2:	setFX0knob2(a_value); break;        
        case MODULEX_FX0_COMBOBOX: setFX0combobox(a_value); break;
        
        case MODULEX_FX1_KNOB0:	setFX1knob0(a_value); break;
        case MODULEX_FX1_KNOB1:	setFX1knob1(a_value); break;        
        case MODULEX_FX1_KNOB2:	setFX1knob2(a_value); break;        
        case MODULEX_FX1_COMBOBOX: setFX1combobox(a_value); break;
        
        case MODULEX_FX2_KNOB0:	setFX2knob0(a_value); break;
        case MODULEX_FX2_KNOB1:	setFX2knob1(a_value); break;        
        case MODULEX_FX2_KNOB2:	setFX2knob2(a_value); break;        
        case MODULEX_FX2_COMBOBOX: setFX2combobox(a_value); break;
        
        case MODULEX_FX3_KNOB0:	setFX3knob0(a_value); break;
        case MODULEX_FX3_KNOB1:	setFX3knob1(a_value); break;        
        case MODULEX_FX3_KNOB2:	setFX3knob2(a_value); break;        
        case MODULEX_FX3_COMBOBOX: setFX3combobox(a_value); break;
        
        case MODULEX_FX4_KNOB0:	setFX4knob0(a_value); break;
        case MODULEX_FX4_KNOB1:	setFX4knob1(a_value); break;        
        case MODULEX_FX4_KNOB2:	setFX4knob2(a_value); break;        
        case MODULEX_FX4_COMBOBOX: setFX4combobox(a_value); break;
        
        case MODULEX_FX5_KNOB0:	setFX5knob0(a_value); break;
        case MODULEX_FX5_KNOB1:	setFX5knob1(a_value); break;        
        case MODULEX_FX5_KNOB2:	setFX5knob2(a_value); break;        
        case MODULEX_FX5_COMBOBOX: setFX5combobox(a_value); break;
        
        case MODULEX_FX6_KNOB0:	setFX6knob0(a_value); break;
        case MODULEX_FX6_KNOB1:	setFX6knob1(a_value); break;        
        case MODULEX_FX6_KNOB2:	setFX6knob2(a_value); break;        
        case MODULEX_FX6_COMBOBOX: setFX6combobox(a_value); break;
        
        case MODULEX_FX7_KNOB0:	setFX7knob0(a_value); break;
        case MODULEX_FX7_KNOB1:	setFX7knob1(a_value); break;        
        case MODULEX_FX7_KNOB2:	setFX7knob2(a_value); break;        
        case MODULEX_FX7_COMBOBOX: setFX7combobox(a_value); break;
        
        case MODULEX_DELAY_TIME: setDelayTime(a_value); break;
        case MODULEX_FEEDBACK: setFeedback(a_value); break;
        case MODULEX_DRY: setDry(a_value); break;
        case MODULEX_WET: setWet(a_value); break;
        case MODULEX_DUCK: setDuck(a_value); break;
        case MODULEX_CUTOFF: setCutoff(a_value); break;
        case MODULEX_STEREO: setStereo(a_value); break;        
    }
}

void modulex_gui::v_control_changed(int a_port, int a_value, bool a_suppress_host_update)
{
    
#ifdef LMS_DEBUG_MODE_QT    
    modulex_cerr << "v_control_changed called.  ";  
    v_print_port_name_to_cerr(a_port);
    modulex_cerr << "  value: " << a_value << endl;
#endif
    
    if(a_suppress_host_update)
        m_suppressHostUpdate = true;
       /*Add the controls you created to the control handler*/
    
    switch (a_port) {
        case MODULEX_FX0_KNOB0:	fx0knob0Changed(a_value); break;
        case MODULEX_FX0_KNOB1:	fx0knob1Changed(a_value); break;
        case MODULEX_FX0_KNOB2:	fx0knob2Changed(a_value); break;  
        case MODULEX_FX0_COMBOBOX:  fx0comboboxChanged(a_value); break;
        
        case MODULEX_FX1_KNOB0:	fx1knob0Changed(a_value); break;
        case MODULEX_FX1_KNOB1:	fx1knob1Changed(a_value); break;
        case MODULEX_FX1_KNOB2:	fx1knob2Changed(a_value); break;  
        case MODULEX_FX1_COMBOBOX:  fx1comboboxChanged(a_value); break;
        
        case MODULEX_FX2_KNOB0:	fx2knob0Changed(a_value); break;
        case MODULEX_FX2_KNOB1:	fx2knob1Changed(a_value); break;
        case MODULEX_FX2_KNOB2:	fx2knob2Changed(a_value); break;  
        case MODULEX_FX2_COMBOBOX:  fx2comboboxChanged(a_value); break;
        
        case MODULEX_FX3_KNOB0:	fx3knob0Changed(a_value); break;
        case MODULEX_FX3_KNOB1:	fx3knob1Changed(a_value); break;
        case MODULEX_FX3_KNOB2:	fx3knob2Changed(a_value); break;  
        case MODULEX_FX3_COMBOBOX:  fx3comboboxChanged(a_value); break;
        
        
        case MODULEX_FX4_KNOB0:	fx4knob0Changed(a_value); break;
        case MODULEX_FX4_KNOB1:	fx4knob1Changed(a_value); break;
        case MODULEX_FX4_KNOB2:	fx4knob2Changed(a_value); break;  
        case MODULEX_FX4_COMBOBOX:  fx4comboboxChanged(a_value); break;
        
        case MODULEX_FX5_KNOB0:	fx5knob0Changed(a_value); break;
        case MODULEX_FX5_KNOB1:	fx5knob1Changed(a_value); break;
        case MODULEX_FX5_KNOB2:	fx5knob2Changed(a_value); break;  
        case MODULEX_FX5_COMBOBOX:  fx5comboboxChanged(a_value); break;
        
        case MODULEX_FX6_KNOB0:	fx6knob0Changed(a_value); break;
        case MODULEX_FX6_KNOB1:	fx6knob1Changed(a_value); break;
        case MODULEX_FX6_KNOB2:	fx6knob2Changed(a_value); break;  
        case MODULEX_FX6_COMBOBOX:  fx6comboboxChanged(a_value); break;
        
        case MODULEX_FX7_KNOB0:	fx7knob0Changed(a_value); break;
        case MODULEX_FX7_KNOB1:	fx7knob1Changed(a_value); break;
        case MODULEX_FX7_KNOB2:	fx7knob2Changed(a_value); break;  
        case MODULEX_FX7_COMBOBOX:  fx7comboboxChanged(a_value); break;

        case MODULEX_DELAY_TIME: delayTimeChanged(a_value); break;
        case MODULEX_FEEDBACK: feedbackChanged(a_value); break;
        case MODULEX_DRY: dryChanged(a_value); break;
        case MODULEX_WET: wetChanged(a_value); break;
        case MODULEX_DUCK: duckChanged(a_value); break;
        case MODULEX_CUTOFF: cutoffChanged(a_value); break;
        case MODULEX_STEREO: stereoChanged(a_value); break;
        
        default:
#ifdef LMS_DEBUG_MODE_QT
            modulex_cerr << "Warning: received request to set nonexistent port " << a_port << endl;
#endif
            break;
    }
    
    if(a_suppress_host_update)
        m_suppressHostUpdate = false;
}


int modulex_gui::i_get_control(int a_port)
{        
    switch (a_port) {
    case MODULEX_FX0_KNOB0: return m_fx0->lms_knob1->lms_get_value();
    case MODULEX_FX0_KNOB1: return m_fx0->lms_knob2->lms_get_value();
    case MODULEX_FX0_KNOB2: return m_fx0->lms_knob3->lms_get_value();
    case MODULEX_FX0_COMBOBOX: return m_fx0->lms_combobox->lms_get_value();
    
    case MODULEX_FX1_KNOB0: return m_fx1->lms_knob1->lms_get_value();
    case MODULEX_FX1_KNOB1: return m_fx1->lms_knob2->lms_get_value();
    case MODULEX_FX1_KNOB2: return m_fx1->lms_knob3->lms_get_value();
    case MODULEX_FX1_COMBOBOX: return m_fx1->lms_combobox->lms_get_value();
    
    case MODULEX_FX2_KNOB0: return m_fx2->lms_knob1->lms_get_value();
    case MODULEX_FX2_KNOB1: return m_fx2->lms_knob2->lms_get_value();
    case MODULEX_FX2_KNOB2: return m_fx2->lms_knob3->lms_get_value();
    case MODULEX_FX2_COMBOBOX: return m_fx2->lms_combobox->lms_get_value();
    
    case MODULEX_FX3_KNOB0: return m_fx3->lms_knob1->lms_get_value();
    case MODULEX_FX3_KNOB1: return m_fx3->lms_knob2->lms_get_value();
    case MODULEX_FX3_KNOB2: return m_fx3->lms_knob3->lms_get_value();
    case MODULEX_FX3_COMBOBOX: return m_fx3->lms_combobox->lms_get_value();            
    
    case MODULEX_FX4_KNOB0: return m_fx4->lms_knob1->lms_get_value();
    case MODULEX_FX4_KNOB1: return m_fx4->lms_knob2->lms_get_value();
    case MODULEX_FX4_KNOB2: return m_fx4->lms_knob3->lms_get_value();
    case MODULEX_FX4_COMBOBOX: return m_fx4->lms_combobox->lms_get_value();
    
    case MODULEX_FX5_KNOB0: return m_fx5->lms_knob1->lms_get_value();
    case MODULEX_FX5_KNOB1: return m_fx5->lms_knob2->lms_get_value();
    case MODULEX_FX5_KNOB2: return m_fx5->lms_knob3->lms_get_value();
    case MODULEX_FX5_COMBOBOX: return m_fx5->lms_combobox->lms_get_value();
    
    case MODULEX_FX6_KNOB0: return m_fx6->lms_knob1->lms_get_value();
    case MODULEX_FX6_KNOB1: return m_fx6->lms_knob2->lms_get_value();
    case MODULEX_FX6_KNOB2: return m_fx6->lms_knob3->lms_get_value();
    case MODULEX_FX6_COMBOBOX: return m_fx6->lms_combobox->lms_get_value();
    
    case MODULEX_FX7_KNOB0: return m_fx7->lms_knob1->lms_get_value();
    case MODULEX_FX7_KNOB1: return m_fx7->lms_knob2->lms_get_value();
    case MODULEX_FX7_KNOB2: return m_fx7->lms_knob3->lms_get_value();
    case MODULEX_FX7_COMBOBOX: return m_fx7->lms_combobox->lms_get_value();
        
    case MODULEX_DELAY_TIME: return m_delaytime->lms_get_value();
    case MODULEX_FEEDBACK: return m_feedback->lms_get_value();
    case MODULEX_DRY: return m_dry->lms_get_value();
    case MODULEX_WET: return m_wet->lms_get_value();
    case MODULEX_DUCK: return m_duck->lms_get_value();
    case MODULEX_CUTOFF: return m_cutoff->lms_get_value();
    case MODULEX_STEREO: return m_stereo->lms_get_value();    
    
    default:
	modulex_cerr << "Warning: received request to get nonexistent port " << a_port << endl;
        return 0;
    }
}

void modulex_gui::oscRecv()
{
    if (modulex_osc_server) {
	lo_server_recv_noblock(modulex_osc_server, 1);
    }
}

void modulex_gui::aboutToQuit()
{
    if (!m_hostRequestedQuit) lo_send(m_host, m_exitingPath, "");
}

modulex_gui::~modulex_gui()
{
    lo_address_free(m_host);
}


void modulex_osc_error(int num, const char *msg, const char *path)
{
#ifdef LMS_DEBUG_MODE_QT
    modulex_cerr << "Error: liblo server error " << num
	 << " in path \"" << (path ? path : "(null)")
	 << "\": " << msg << endl;
#endif
}

int modulex_debug_handler(const char *path, const char *types, lo_arg **argv,
	      int argc, void *data, void *user_data)
{
    int i;
#ifdef LMS_DEBUG_MODE_QT
      modulex_cerr << "Warning: unhandled OSC message in GUI:" << endl;
#endif
    

    for (i = 0; i < argc; ++i) {
#ifdef LMS_DEBUG_MODE_QT
	modulex_cerr << "arg " << i << ": type '" << types[i] << "': ";
#endif
        lo_arg_pp((lo_type)types[i], argv[i]);
#ifdef LMS_DEBUG_MODE_QT
	modulex_cerr << endl;
#endif
    }
#ifdef LMS_DEBUG_MODE_QT
    modulex_cerr << "(path is <" << path << ">)" << endl;
#endif
    return 1;
}

int modulex_program_handler(const char *path, const char *types, lo_arg **argv,
	       int argc, void *data, void *user_data)
{
    //not implemented on this plugin
    return 0;
}

int modulex_configure_handler(const char *path, const char *types, lo_arg **argv,
		  int argc, void *data, void *user_data)
{
    return 0;
}

int modulex_rate_handler(const char *path, const char *types, lo_arg **argv,
	     int argc, void *data, void *user_data)
{
    return 0; /* ignore it */
}

int modulex_show_handler(const char *path, const char *types, lo_arg **argv,
	     int argc, void *data, void *user_data)
{
    modulex_gui *gui = static_cast<modulex_gui *>(user_data);
    while (!gui->ready()) sleep(1);
    if (gui->isVisible()) gui->raise();
    else gui->show();
    return 0;
}

int modulex_hide_handler(const char *path, const char *types, lo_arg **argv,
	     int argc, void *data, void *user_data)
{
    modulex_gui *gui = static_cast<modulex_gui *>(user_data);
    gui->hide();
    return 0;
}

int modulex_quit_handler(const char *path, const char *types, lo_arg **argv,
	     int argc, void *data, void *user_data)
{
    modulex_gui *gui = static_cast<modulex_gui *>(user_data);
    gui->setHostRequestedQuit(true);
    qApp->quit();
    return 0;
}

int modulex_control_handler(const char *path, const char *types, lo_arg **argv,
		int argc, void *data, void *user_data)
{
    modulex_gui *gui = static_cast<modulex_gui *>(user_data);

    if (argc < 2) {
        
#ifdef LMS_DEBUG_MODE_QT
	modulex_cerr << "Error: too few arguments to control_handler" << endl;
#endif
	return 1;
    }

    const int port = argv[0]->i;
    const float value = argv[1]->f;

#ifdef LMS_DEBUG_MODE_QT
    modulex_cerr << "control_handler called.  port:  " << port << " , value " << value << endl;
#endif

    gui->v_set_control(port, value);  
    
    return 0;
}

int main(int argc, char **argv)
{
#ifdef LMS_DEBUG_MODE_QT
    modulex_cerr << "Qt GUI main() called..." << endl;
#endif
    
    QApplication application(argc, argv);

    if (application.argc() != 5) {
#ifdef LMS_DEBUG_MODE_QT
	modulex_cerr << "usage: "
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
    XSetErrorHandler(modulex_handle_x11_error);
#endif

    char *url = application.argv()[1];

    char *host = lo_url_get_hostname(url);
    char *port = lo_url_get_port(url);
    char *path = lo_url_get_path(url);

    modulex_gui gui(host, port,
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
    modulex_cerr << "Adding lo server methods" << endl;
#endif
    modulex_osc_server = lo_server_new(NULL, modulex_osc_error);
    lo_server_add_method(modulex_osc_server, myControlPath, "if", modulex_control_handler, &gui);
    lo_server_add_method(modulex_osc_server, myProgramPath, "ii", modulex_program_handler, &gui);
    lo_server_add_method(modulex_osc_server, myConfigurePath, "ss", modulex_configure_handler, &gui);
    lo_server_add_method(modulex_osc_server, myRatePath, "i", modulex_rate_handler, &gui);
    lo_server_add_method(modulex_osc_server, myShowPath, "", modulex_show_handler, &gui);
    lo_server_add_method(modulex_osc_server, myHidePath, "", modulex_hide_handler, &gui);
    lo_server_add_method(modulex_osc_server, myQuitPath, "", modulex_quit_handler, &gui);
    lo_server_add_method(modulex_osc_server, NULL, NULL, modulex_debug_handler, &gui);

    lo_address hostaddr = lo_address_new(host, port);
    lo_send(hostaddr,
	    QByteArray(path) + "/update",
	    "s",
	    (QByteArray(lo_server_get_url(modulex_osc_server))+QByteArray(path+1)).data());

    QObject::connect(&application, SIGNAL(aboutToQuit()), &gui, SLOT(aboutToQuit()));

    gui.setReady(true);
    
    gui.setWindowTitle(QString("PyDAW - Modulex - ") +  application.argv()[3]);
#ifdef LMS_DEBUG_MODE_QT
    modulex_cerr << "Starting GUI now..." << endl;
#endif
    
    return application.exec();
}

