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
#include <QGroupBox>
#include <QComboBox>
#include <QPixmap>

#include <string>
#include <stdlib.h>

#include "../../libmodsynth/widgets/ui_modules/filter.h"
#include "../../libmodsynth/widgets/lms_main_layout.h"

extern "C" {
#include <lo/lo.h>
}

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
        
    void v_print_port_name_to_cerr(int);
    void lms_set_value(float val, LMS_control * a_ctrl );
    void lms_value_changed(int a_value, LMS_control * a_ctrl);
        
public slots:
    /*Event handlers for setting knob values*/
    void setCutoff (float val);
    void setRes (float val);
    void setType (float val);
        
    void aboutToQuit();
    
protected slots:
    /*Event handlers for receiving changed knob values*/
    void cutoffChanged (int);
    void resChanged (int);
    void typeChanged (int);
        
    void oscRecv();
protected:
    
    /*Declare a QLabel and QDial for each knob.  Also declare any other controls that set/receive values here*/
    LMS_main_layout * m_main_layout;
    LMS_filter_widget * m_filter_widget;
    
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
