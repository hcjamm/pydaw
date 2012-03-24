/* -*- c-basic-offset: 4 -*-  vi:set ts=8 sts=4 sw=4: */

/* synth_qt_gui.h


This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

*/

#ifndef _SYNTH_QT_GUI_H_INCLUDED_
#define _SYNTH_QT_GUI_H_INCLUDED_

#include <QFrame>
#include <QDial>
#include <QLabel>
#include <QLayout>
#include <QCheckBox>
#include <QGroupBox>
#include <QComboBox>
#include <QPushButton>
#include <qt4/QtXml/QDomDocument>
#include <QPixmap>
#include <QFile>
#include <QDir>

#include <string>
#include <stdlib.h>


extern "C" {
#include <lo/lo.h>
}

/*Defines ranges and other sane defaults depending on what a knob controls*/
enum e_knob_type
{
    decibels_0,
    decibels_plus_6,
    decibels_plus_12,
    decibels_plus_24,
    decibels_20_to_0,
    decibels_30_to_0,
    pitch,
    zero_to_one,
    zero_to_two,
    zero_to_four,     
    minus1_to_1,
    minus12_to_12,
    minus24_to_24,
    minus36_to_36
};

class SynthGUI : public QFrame
{
    Q_OBJECT

public:
    SynthGUI(const char * host, const char * port,
	     QByteArray controlPath, QByteArray midiPath, QByteArray programPath,
	     QByteArray exitingPath, QWidget *w = 0);
    virtual ~SynthGUI();

    bool ready() const { return m_ready; }
    void setReady(bool ready) { m_ready = ready; }

    void setHostRequestedQuit(bool r) { m_hostRequestedQuit = r; }
    
    
    void v_set_control(int, float);
    void v_control_changed(int, int, bool);
    int i_get_control(int);
    
    void v_add_knob_to_layout(QDial *, e_knob_type, int, QLabel *, QGridLayout *, QString, int, int, const char *, const char *);
    
    void v_print_port_name_to_cerr(int);
        
public slots:
    /*Event handlers for setting knob values*/
    void setGain1 (float val);
    void setPitch1 (float val);
    void setRes1 (float val);
    
    void setGain2 (float val);
    void setPitch2 (float val);
    void setRes2 (float val);
    
    void setGain3 (float val);
    void setPitch3 (float val);
    void setRes3 (float val);
    
    void setGain4 (float val);
    void setPitch4 (float val);
    void setRes4 (float val);
    
    void setGain5 (float val);
    void setPitch5 (float val);
    void setRes5 (float val);
    
    void aboutToQuit();
    
protected slots:
    /*Event handlers for receiving changed knob values*/
    void gain1Changed (int);
    void pitch1Changed (int);
    void res1Changed (int);
    
    void gain2Changed (int);
    void pitch2Changed (int);
    void res2Changed (int);
    
    void gain3Changed (int);
    void pitch3Changed (int);
    void res3Changed (int);
    
    void gain4Changed (int);
    void pitch4Changed (int);
    void res4Changed (int);
    
    void gain5Changed (int);
    void pitch5Changed (int);
    void res5Changed (int);
    
    void oscRecv();
protected:
    QDial *newQDial( int, int, int, int );
    
    QDial *get_knob(e_knob_type, int _default_value = 333);
    
    //TODO:  update this to be more flexible about layout types
    void add_widget(QGridLayout * _layout, int position_x, int position_y, QString _label_text,  QWidget * _widget,    
    QLabel * _label);  
    
    void add_widget_no_label(QGridLayout * _layout, int position_x, int position_y, QString _label_text, QWidget * _widget);
    
    QGroupBox * newGroupBox(QString, QWidget *);
    
    QLabel * newQLabel(QWidget *);
    
    QCheckBox * get_checkbox(std::string _text);
    
    QComboBox * get_combobox(QString _choices [], int _count, QWidget * _parent);
    
    void changed_seconds(int, QLabel *, int);
    void changed_pitch(int, QLabel *, int);
    void changed_decibels(int, QLabel *, int);
    void changed_zero_to_x(int, QLabel *, int);
    void changed_integer(int value, QLabel * _label, int _port);
       
    /*Declare a QLabel and QDial for each knob.  Also declare any other controls that set/receive values here*/
    QDial *m_pitch1;
    QLabel *m_pitch1Label;
    QDial *m_gain1;
    QLabel *m_gain1Label;    
    QDial *m_res1;
    QLabel *m_res1Label;
    
    QDial *m_pitch2;
    QLabel *m_pitch2Label;
    QDial *m_gain2;
    QLabel *m_gain2Label;    
    QDial *m_res2;
    QLabel *m_res2Label;
    
    QDial *m_pitch3;
    QLabel *m_pitch3Label;
    QDial *m_gain3;
    QLabel *m_gain3Label;    
    QDial *m_res3;
    QLabel *m_res3Label;
    
    QDial *m_pitch4;
    QLabel *m_pitch4Label;
    QDial *m_gain4;
    QLabel *m_gain4Label;    
    QDial *m_res4;
    QLabel *m_res4Label;
    
    QDial *m_pitch5;
    QLabel *m_pitch5Label;
    QDial *m_gain5;
    QLabel *m_gain5Label;    
    QDial *m_res5;
    QLabel *m_res5Label;
    
    lo_address m_host;
    QByteArray m_controlPath;
    QByteArray m_midiPath;
    QByteArray m_programPath;
    QByteArray m_exitingPath;

    bool m_suppressHostUpdate;
    bool m_hostRequestedQuit;
    bool m_ready;    
};


#endif
