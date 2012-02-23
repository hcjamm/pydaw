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
    /*GUI Step 2:  Event handlers for setting knob values*/
    void setAttack (float sec);
    void setDecay  (float sec);
    void setSustain(float val);
    void setRelease(float sec);
    void setTimbre (float val);
    void setRes (float val);
    void setDist (float val);
        
    void setFilterAttack (float sec);
    void setFilterDecay  (float sec);
    void setFilterSustain(float val);
    void setFilterRelease(float sec);
    
    void setNoiseAmp(float val);
    
    void setFilterEnvAmt(float val);
    void setDistWet(float val);
    void setOsc1Type(float val);
    void setOsc1Pitch(float val);
    void setOsc1Tune(float val);
    void setOsc1Volume(float val);
    void setOsc2Type(float val);
    void setOsc2Pitch(float val);
    void setOsc2Tune(float val);
    void setOsc2Volume(float val);
    void setMasterVolume(float val);
    
    void setMasterUnisonVoices(float val);
    void setMasterUnisonSpread(float val);
    void setMasterGlide(float val);
    void setMasterPitchbendAmt(float val);
    
    void setPitchEnvTime(float val);
    void setPitchEnvAmt(float val);
    
    void setProgram(float val);    
    
    void aboutToQuit();
    
protected slots:
    /*GUI Step 3:  Event handlers for receiving changed knob values*/
    void attackChanged (int);
    void decayChanged  (int);
    void sustainChanged(int);
    void releaseChanged(int);
    void timbreChanged (int);
    void resChanged (int);
    void distChanged (int);
        
    void filterAttackChanged (int);
    void filterDecayChanged  (int);
    void filterSustainChanged(int);
    void filterReleaseChanged(int);
    
    void noiseAmpChanged(int);

    
    void filterEnvAmtChanged(int);
    void distWetChanged(int);
    void osc1TypeChanged(int);
    void osc1PitchChanged(int);
    void osc1TuneChanged(int);
    void osc1VolumeChanged(int);
    void osc2TypeChanged(int);
    void osc2PitchChanged(int);
    void osc2TuneChanged(int);
    void osc2VolumeChanged(int);
    void masterVolumeChanged(int);
    
    void masterUnisonVoicesChanged(int);
    void masterUnisonSpreadChanged(int);
    void masterGlideChanged(int);
    void masterPitchbendAmtChanged(int);
    
    void pitchEnvTimeChanged(int);
    void pitchEnvAmtChanged(int);
    
    void programChanged(int);
    void programSaved();
    
    
    void test_press();
    void test_release();
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
       
    
    QString presets_tab_delimited [128];
    
    
    /*GUI Step1:  Declare a QLabel and QDial for each knob.  Also declare any other controls that set/receive values here*/
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
    
    QDial *m_filter_env_amt;
    QLabel *m_filter_env_amtLabel;  
    
    QDial *m_res;
    QLabel *m_resLabel;    
    QDial *m_dist;
    QLabel *m_distLabel;
    
    QDial *m_dist_wet;
    
    QDial *m_filter_attack;
    QLabel *m_filter_attackLabel;
    QDial *m_filter_decay;
    QLabel *m_filter_decayLabel;    
    QDial *m_filter_sustain;
    QLabel *m_filter_sustainLabel;
    QDial *m_filter_release;
    QLabel *m_filter_releaseLabel;    
    
    QDial *m_noise_amp;
    QLabel *m_noise_ampLabel;
    
    
    QComboBox *m_osc1_type;
    QDial *m_osc1_pitch;
    QLabel *m_osc1_pitchLabel;    
    QDial *m_osc1_tune;
    QLabel *m_osc1_tuneLabel;
    QDial *m_osc1_volume;
    QLabel *m_osc1_volumeLabel;   
    
    QComboBox *m_osc2_type;
    QDial *m_osc2_pitch;
    QLabel *m_osc2_pitchLabel;    
    QDial *m_osc2_tune;
    QLabel *m_osc2_tuneLabel;
    QDial *m_osc2_volume;
    QLabel *m_osc2_volumeLabel;   
    
    QDial *m_master_volume;
    QLabel *m_master_volumeLabel;       
    
    QDial *m_master_unison_voices;
    QLabel *m_master_unison_voicesLabel;       
    
    QDial *m_master_unison_spread;
    QLabel *m_master_unison_spreadLabel;       
    
    QDial *m_master_glide;
    QLabel *m_master_glideLabel;       
    
    QDial *m_master_pitchbend_amt;
    QLabel *m_master_pitchbend_amtLabel;
    
    QDial *m_pitch_env_time;
    QLabel *m_pitch_env_timeLabel;
    
    QDial *m_pitch_env_amt;
    QLabel *m_pitch_env_amtLabel;
    
    QComboBox *m_program;
    QPushButton *m_prog_save;
    
    
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
