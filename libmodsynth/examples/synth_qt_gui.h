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

#include <string>
#include <stdlib.h>


extern "C" {
#include <lo/lo.h>
}

/*Defines ranges and other sane defaults depending on what a knob controls*/
enum _knob_type
{
    decibels_0,
    decibels_plus_6,
    decibels_plus_12,
    decibels_plus_24,
    pitch,
    zero_to_one,
    zero_to_two,
    zero_to_four            
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
        
public slots:
    /*GUI Step 2:  Event handlers for setting knob values*/
    void setAttack (float sec);
    void setDecay  (float sec);
    void setSustain(float val);
    void setRelease(float sec);
    void setTimbre (float val);
    void setRes (float val);
    void setDist (float val);
    void aboutToQuit();
    
    
    void setAttack_f (float sec);
    void setDecay_f  (float sec);
    void setSustain_f(float val);
    void setRelease_f(float sec);
    
protected slots:
    /*GUI Step 3:  Event handlers for receiving changed knob values*/
    void attackChanged (int);
    void decayChanged  (int);
    void sustainChanged(int);
    void releaseChanged(int);
    void timbreChanged (int);
    void resChanged (int);
    void distChanged (int);
    void test_press();
    void test_release();
    void oscRecv();
    
    void attack_fChanged (int);
    void decay_fChanged  (int);
    void sustain_fChanged(int);
    void release_fChanged(int);

protected:
    QDial *newQDial( int, int, int, int );
    
    QDial *_get_knob(_knob_type);
    
    //TODO:  update this to be more flexible about layout types
    void _add_knob(QGridLayout * _layout, int position_x, int position_y, QString _label_text, QDial * _knob,    
    QLabel * _label);
    
    QGroupBox * _newGroupBox(QString, QWidget *);
    
    QLabel * _newQLabel(QWidget *);
    
    QCheckBox * _get_checkbox(std::string _text);
    
    void _changed_seconds(int, QLabel *, int);
    void _changed_pitch(int, QLabel *, int);
    void _changed_decibels(int, QLabel *, int);
    
    /*GUI Step1:  Declare a QLabel and QDial for each control (or other controls) here*/
    QDial *m_attack;
    QLabel *m_attackLabel;
    QDial *m_decay;
    QLabel *m_decayLabel;    
    QDial *m_sustain;
    QLabel *m_sustainLabel;
    QDial *m_release;
    QLabel *m_releaseLabel;    
    QDial *m_timbre;
    QLabel *m_timbreLabel;    
    QDial *m_res;
    QLabel *m_resLabel;    
    QDial *m_dist;
    QLabel *m_distLabel;
    
    
    QDial *m_attack_f;
    QLabel *m_attackLabel_f;
    QDial *m_decay_f;
    QLabel *m_decayLabel_f;    
    QDial *m_sustain_f;
    QLabel *m_sustainLabel_f;
    QDial *m_release_f;
    QLabel *m_releaseLabel_f;    
    

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
