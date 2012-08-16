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
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QDoubleSpinBox>

#include "../../libmodsynth/widgets/knob_regular.h"
#include "../../libmodsynth/widgets/group_box.h"
#include "../../libmodsynth/widgets/lms_main_layout.h"

#include <string>
#include <stdlib.h>


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
    
    void lms_set_value(float val, LMS_control * a_ctrl);
    void lms_value_changed(int a_value, LMS_control * a_ctrl);
        
public slots:
    /*Event handlers for setting knob values*/
    void setDelayTime (float val);
    void setFeedback (float val);
    void setDry(float val);
    void setWet(float val);
    void setDuck(float val);
    void setCutoff(float val);
    void setStereo(float val);
        
    void aboutToQuit();
    
protected slots:
    /*Event handlers for receiving changed knob values*/
    void delayTimeChanged (int);
    void feedbackChanged (int);
    void dryChanged(int);
    void wetChanged(int);
    void duckChanged(int);
    void cutoffChanged(int);
    void stereoChanged(int);
    void bpmSyncPressed();
        
    void oscRecv();
protected:
    
    LMS_main_layout * main_layout;
    
    LMS_group_box *delay_groupbox;
    
    /*Declare a QLabel and QDial for each knob.  Also declare any other controls that set/receive values here*/
    LMS_knob_regular *m_delaytime;
    
    LMS_knob_regular *m_feedback;
    
    LMS_knob_regular *m_dry;
    
    LMS_knob_regular *m_wet;
    
    LMS_knob_regular *m_duck;
    
    LMS_knob_regular *m_cutoff;
    
    LMS_knob_regular *m_stereo;
    
    /*BPM sync box*/
    QDoubleSpinBox * m_bpm_spinbox;        
    QComboBox * m_beat_frac;    
    QPushButton * m_sync_bpm;
    
    
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
