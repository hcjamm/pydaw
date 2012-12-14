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

#ifndef RAYV_SYNTH_QT_GUI_H
#define RAYV_SYNTH_QT_GUI_H

#include <QFrame>
#include <QDial>
#include <QLabel>
#include <QLayout>
#include <QCheckBox>
#include <QGroupBox>
#include <QComboBox>
#include <QPushButton>
#include <QPixmap>
#include <QTabWidget>

#include <string>
#include <stdlib.h>

#include "../../libmodsynth/widgets/presets.h"
#include "../../libmodsynth/widgets/group_box.h"
#include "../../libmodsynth/widgets/lms_combobox.h"
#include "../../libmodsynth/widgets/lms_checkbox.h"
#include "../../libmodsynth/widgets/lms_main_layout.h"
#include "../../libmodsynth/widgets/ui_modules/adsr.h"
#include "../../libmodsynth/widgets/ui_modules/oscillator.h"
#include "../../libmodsynth/widgets/ui_modules/filter.h"
#include "../../libmodsynth/widgets/ui_modules/ramp_env.h"
#include "../../libmodsynth/widgets/ui_modules/lfo.h"
#include "../../libmodsynth/widgets/ui_modules/master.h"
#include "../../libmodsynth/widgets/lms_session_manager.h"
#include "../../libmodsynth/widgets/ui_modules/multieffect_basic.h"
#include "../../libmodsynth/widgets/mod_matrix.h"

extern "C" {
#include <lo/lo.h>
}

class rayv_gui : public QFrame
{
    Q_OBJECT

public:
    rayv_gui(const char * host, const char * port,
	     QByteArray controlPath, QByteArray midiPath, QByteArray programPath,
	     QByteArray exitingPath, QWidget *w = 0);
    virtual ~rayv_gui();

    bool ready() const { return m_ready; }
    void setReady(bool ready) { m_ready = ready; }

    void setHostRequestedQuit(bool r) { m_hostRequestedQuit = r; }
    
    LMS_mod_matrix * m_polyfx_mod_matrix[1]; //[EUPHORIA_EFFECTS_GROUPS_COUNT];
    
    void v_set_control(int, float);
    void v_control_changed(int, int, bool);
    int i_get_control(int);
    
    void v_print_port_name_to_cerr(int);
    
    void lms_value_changed(int, LMS_control *);
    void lms_set_value(float, LMS_control *);
    
    bool m_suppressHostUpdate;
            
public slots:
    /*Event handlers for setting knob values*/
    void setAttackMain (float);
    void setDecayMain  (float);
    void setSustainMain(float);
    void setReleaseMain(float);
    
    void setAttack1 (float);
    void setDecay1  (float);
    void setSustain1(float);
    void setRelease1(float);
    void setADSR1checked(float);
    
    void setAttack2 (float);
    void setDecay2  (float);
    void setSustain2(float);
    void setRelease2(float);
    void setADSR2checked(float);
    
    void setNoiseAmp(float);
    
    void setOsc1Type(float);
    void setOsc1Pitch(float);
    void setOsc1Tune(float);
    void setOsc1Volume(float);
    void setOsc2Type(float);
    void setOsc2Pitch(float);
    void setOsc2Tune(float);
    void setOsc2Volume(float);
    
   
    void setProgram(float);    
    
    
    void setAttack (float);
    void setDecay  (float);
    void setSustain(float);
    void setRelease(float);    
            
    void setFilterAttack (float);
    void setFilterDecay  (float);
    void setFilterSustain(float);
    void setFilterRelease(float);

    void setNoiseType(float);
    
    void setMasterVolume(float);
    
    void setMasterUnisonVoices(float);
    void setMasterUnisonSpread(float);
    void setMasterGlide(float);
    void setMasterPitchbendAmt(float);
    
    void setPitchEnvTime(float);
        
    void setLFOfreq(float);
    void setLFOtype(float);
        
    void setFX0knob0 (float val);
    void setFX0knob1 (float val);
    void setFX0knob2 (float val);
    void setFX0combobox (float val);
    
    void setFX1knob0 (float val);
    void setFX1knob1 (float val);
    void setFX1knob2 (float val);
    void setFX1combobox (float val);
    
    void setFX2knob0 (float val);
    void setFX2knob1 (float val);
    void setFX2knob2 (float val);
    void setFX2combobox (float val);
    
    void setFX3knob0 (float val);
    void setFX3knob1 (float val);
    void setFX3knob2 (float val);
    void setFX3combobox (float val);
    
    void aboutToQuit();
    
protected slots:
    /*Event handlers for receiving changed knob values*/
    void attackMainChanged (int);
    void decayMainChanged  (int);
    void sustainMainChanged(int);
    void releaseMainChanged(int);
    
    void attack1Changed (int);
    void decay1Changed  (int);
    void sustain1Changed(int);
    void release1Changed(int);
    void adsr1checkChanged(bool);
    
    void attack2Changed (int);
    void decay2Changed  (int);
    void sustain2Changed(int);
    void release2Changed(int);
    void adsr2checkChanged(bool);
    
    void noiseAmpChanged(int);
    
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
    
    void programChanged(int);
    void programSaved();
    
    void oscRecv();
    
    
    void attackChanged (int);
    void decayChanged  (int);
    void sustainChanged(int);
    void releaseChanged(int);
            
    void filterAttackChanged (int);
    void filterDecayChanged  (int);
    void filterSustainChanged(int);
    void filterReleaseChanged(int);
        
    void noise_typeChanged(int);
    
    void pitchEnvTimeChanged(int);
        
    void LFOfreqChanged(int);
    void LFOtypeChanged(int);
    
    void fx0knob0Changed(int);
    void fx0knob1Changed(int);
    void fx0knob2Changed(int);
    void fx0comboboxChanged(int);
    
    void fx1knob0Changed(int);
    void fx1knob1Changed(int);
    void fx1knob2Changed(int);
    void fx1comboboxChanged(int);
    
    void fx2knob0Changed(int);
    void fx2knob1Changed(int);
    void fx2knob2Changed(int);
    void fx2comboboxChanged(int);
    
    void fx3knob0Changed(int);
    void fx3knob1Changed(int);
    void fx3knob2Changed(int);
    void fx3comboboxChanged(int);    
    
    
    void pfxmatrix_grp0dst0src0ctrl0Changed(int);
    void pfxmatrix_grp0dst0src0ctrl1Changed(int);
    void pfxmatrix_grp0dst0src0ctrl2Changed(int);
    void pfxmatrix_grp0dst0src1ctrl0Changed(int);
    void pfxmatrix_grp0dst0src1ctrl1Changed(int);
    void pfxmatrix_grp0dst0src1ctrl2Changed(int);
    void pfxmatrix_grp0dst0src2ctrl0Changed(int);
    void pfxmatrix_grp0dst0src2ctrl1Changed(int);
    void pfxmatrix_grp0dst0src2ctrl2Changed(int);
    void pfxmatrix_grp0dst0src3ctrl0Changed(int);
    void pfxmatrix_grp0dst0src3ctrl1Changed(int);
    void pfxmatrix_grp0dst0src3ctrl2Changed(int);
    void pfxmatrix_grp0dst1src0ctrl0Changed(int);
    void pfxmatrix_grp0dst1src0ctrl1Changed(int);
    void pfxmatrix_grp0dst1src0ctrl2Changed(int);
    void pfxmatrix_grp0dst1src1ctrl0Changed(int);
    void pfxmatrix_grp0dst1src1ctrl1Changed(int);
    void pfxmatrix_grp0dst1src1ctrl2Changed(int);
    void pfxmatrix_grp0dst1src2ctrl0Changed(int);
    void pfxmatrix_grp0dst1src2ctrl1Changed(int);
    void pfxmatrix_grp0dst1src2ctrl2Changed(int);
    void pfxmatrix_grp0dst1src3ctrl0Changed(int);
    void pfxmatrix_grp0dst1src3ctrl1Changed(int);
    void pfxmatrix_grp0dst1src3ctrl2Changed(int);
    void pfxmatrix_grp0dst2src0ctrl0Changed(int);
    void pfxmatrix_grp0dst2src0ctrl1Changed(int);
    void pfxmatrix_grp0dst2src0ctrl2Changed(int);
    void pfxmatrix_grp0dst2src1ctrl0Changed(int);
    void pfxmatrix_grp0dst2src1ctrl1Changed(int);
    void pfxmatrix_grp0dst2src1ctrl2Changed(int);
    void pfxmatrix_grp0dst2src2ctrl0Changed(int);
    void pfxmatrix_grp0dst2src2ctrl1Changed(int);
    void pfxmatrix_grp0dst2src2ctrl2Changed(int);
    void pfxmatrix_grp0dst2src3ctrl0Changed(int);
    void pfxmatrix_grp0dst2src3ctrl1Changed(int);
    void pfxmatrix_grp0dst2src3ctrl2Changed(int);
    void pfxmatrix_grp0dst3src0ctrl0Changed(int);
    void pfxmatrix_grp0dst3src0ctrl1Changed(int);
    void pfxmatrix_grp0dst3src0ctrl2Changed(int);
    void pfxmatrix_grp0dst3src1ctrl0Changed(int);
    void pfxmatrix_grp0dst3src1ctrl1Changed(int);
    void pfxmatrix_grp0dst3src1ctrl2Changed(int);
    void pfxmatrix_grp0dst3src2ctrl0Changed(int);
    void pfxmatrix_grp0dst3src2ctrl1Changed(int);
    void pfxmatrix_grp0dst3src2ctrl2Changed(int);
    void pfxmatrix_grp0dst3src3ctrl0Changed(int);
    void pfxmatrix_grp0dst3src3ctrl1Changed(int);
    void pfxmatrix_grp0dst3src3ctrl2Changed(int);
    
    
protected:
    void pfxmatrix_Changed(int,int,int,int,int);
    
    QVBoxLayout * m_window_layout;
    QTabWidget * m_tab_widget;
    QWidget * m_osc_tab;
    
    QWidget *m_poly_fx_tab;
    
    LMS_multieffect * m_fx0;
    LMS_multieffect * m_fx1;
    LMS_multieffect * m_fx2;
    LMS_multieffect * m_fx3;
    
    LMS_main_layout * m_oscillator_layout;
    
    LMS_adsr_widget * m_adsr_amp_main;
    LMS_oscillator_widget * m_osc1;
    LMS_oscillator_widget * m_osc2;
    
    LMS_adsr_widget * m_adsr_amp1;
    LMS_checkbox * m_adsr_amp1_checkbox;
    LMS_adsr_widget * m_adsr_amp2;
    LMS_checkbox * m_adsr_amp2_checkbox;
    
    LMS_master_widget * m_master;
            
    LMS_main_layout * m_main_layout;
    
    LMS_adsr_widget * m_adsr_amp;
    LMS_adsr_widget * m_adsr_filter;        
    
    LMS_ramp_env * m_pitch_env;
    
    LMS_lfo_widget * m_lfo;
            
    LMS_group_box * m_groupbox_noise;
    LMS_knob_regular *m_noise_amp;
    LMS_combobox *m_noise_type;
            
    LMS_preset_manager * m_program;
    
    lo_address m_host;
    QByteArray m_controlPath;
    QByteArray m_midiPath;
    QByteArray m_programPath;
    QByteArray m_exitingPath;
    
    bool m_hostRequestedQuit;
    bool m_ready;    
};


#endif
