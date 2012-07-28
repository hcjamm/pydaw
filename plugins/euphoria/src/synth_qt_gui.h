/* -*- c-basic-offset: 4 -*-  vi:set ts=8 sts=4 sw=4: */

/* synth_qt_gui.h
 * 
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.   
*/

#ifndef _TRIVIAL_SAMPLER_QT_GUI_H_INCLUDED_
#define _TRIVIAL_SAMPLER_QT_GUI_H_INCLUDED_

#include <QFrame>
#include <QCheckBox>
#include <QSpinBox>
#include <QLabel>
#include <QSlider>
#include <QLayout>
#include <QMenuBar>
#include <QMenu>
#include <QAction>

#include <QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QComboBox>
#include <QDial>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QScrollArea>
#include <QSlider>
#include <QSpacerItem>
#include <QSpinBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QWidget>
#include <QRadioButton>

//#include "../../libmodsynth/widgets/ui_modules/sample_viewer.h"
#include "../../libmodsynth/widgets/mod_matrix.h"
#include "../../libmodsynth/widgets/lms_file_select.h"
#include "ports.h"

#include "../../libmodsynth/widgets/group_box.h"
#include "../../libmodsynth/widgets/lms_combobox.h"
#include "../../libmodsynth/widgets/lms_main_layout.h"
#include "../../libmodsynth/widgets/ui_modules/adsr.h"
#include "../../libmodsynth/widgets/ui_modules/oscillator.h"
#include "../../libmodsynth/widgets/ui_modules/filter.h"
#include "../../libmodsynth/widgets/ui_modules/ramp_env.h"
#include "../../libmodsynth/widgets/ui_modules/lfo.h"
#include "../../libmodsynth/widgets/ui_modules/master.h"
#include "../../libmodsynth/widgets/sample_graph.h"
#include "../../libmodsynth/widgets/ui_modules/multieffect_basic.h"
#include "../../libmodsynth/widgets/lms_file_browser.h"

extern "C" {
#include <lo/lo.h>
}

class SamplerGUI : public QFrame
{
    Q_OBJECT

public:
    SamplerGUI(bool stereo, const char * host, const char * port,
	       QByteArray controlPath, QByteArray midiPath, QByteArray programPath,
	       QByteArray exitingPath, QWidget *w = 0);
    virtual ~SamplerGUI();

    bool ready() const { return m_ready; }
    void setReady(bool ready) { m_ready = ready; }

    void setHostRequestedQuit(bool r) { m_hostRequestedQuit = r; }
    
    /*Moved here to be accessible by the control_handler*/
    LMS_mod_matrix * m_sample_table;    
    
    //New mod matrix
    LMS_mod_matrix * m_polyfx_mod_matrix[LMS_EFFECTS_GROUPS_COUNT];
    
    bool m_suppressHostUpdate;
    
    /*To prevent controls that update other controls from going berserk*/
    bool m_handle_control_updates;
    /*Setting this to true causes the moveSamplesToSingleDirectory() to use a tmp folder, and not 
     to reset the sample locations to the new folder*/
    bool m_creating_instrument_file;
    /*This is used when creating an instrument file*/
    QString m_inst_file_tmp_path;
    void calculate_fx_label(int,int,QLabel*);
    
    void generate_files_string(int);
    void generate_files_string();
    QString files_string;
    QString preview_file;
    
    void set_selected_sample_combobox_item(int,QString);
    LMS_file_select * m_file_selector;
    LMS_file_browser * m_file_browser;
    
    //From Ray-V   TODO:  Are these necessary?
    
    void v_set_control(int, float);
    void v_control_changed(int, int, bool);
    int i_get_control(int);
    
    void v_print_port_name_to_cerr(int);
    
    void lms_value_changed(int, LMS_control *);
    void lms_monofx_value_changed(int, LMS_control*, int);
    void lms_set_value(float, LMS_control *);
    void load_files(QStringList);
        
    //End from Ray-V
        
    int m_sample_starts[LMS_MAX_SAMPLE_COUNT];  //0 to 10000, not the actual sample number.  0 == the very start of the sample, 10000 == the very end
    int m_sample_ends[LMS_MAX_SAMPLE_COUNT];  //0 to 10000, not the actual sample number.  READ CAREFULLY:  0 == the very end of the sample, 10000 == the very beginning (IT'S INVERTED)
    int m_sample_loop_starts[LMS_MAX_SAMPLE_COUNT];  //0 to 10000, not the actual sample number.  0 == the very start of the sample, 10000 == the very end
    int m_sample_loop_ends[LMS_MAX_SAMPLE_COUNT];  //0 to 10000, not the actual sample number.  READ CAREFULLY:  0 == the very end of the sample, 10000 == the very beginning (IT'S INVERTED)
    int m_sample_loop_modes[LMS_MAX_SAMPLE_COUNT];
    
    int m_mono_fx_values[LMS_MONO_FX_GROUPS_COUNT][LMS_MONO_FX_COUNT][LMS_PORTS_PER_MOD_EFFECT];
    int m_sample_selected_monofx_groups[LMS_MAX_SAMPLE_COUNT];
    
public slots:
    void setSampleFile(QString file);
    void clearFile();
    void setSelection(int);
    void openInEditor();
    void reloadSample();
    void moveSamplesToSingleDirectory();
    void saveInstrumentToSingleFile();
    void openInstrumentFromFile();
    void sampleStartChanged(int);
    void sampleEndChanged(int);
    void sampleLoopStartChanged(int);
    void sampleLoopEndChanged(int);
    void viewSampleSelectedIndexChanged(int);
    void loopModeChanged(int);
    void mapAllSamplesToOneWhiteKey();
    void clearAllSamples();
    
    //From Ray-V PolyFX
    void setAttack (float);
    void setDecay  (float);
    void setSustain(float);
    void setRelease(float);
            
    void setFilterAttack (float);
    void setFilterDecay  (float);
    void setFilterSustain(float);
    void setFilterRelease(float);
    
    void setNoiseAmp(float);
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
    
    void setmonoFX0knob0 (float val);
    void setmonoFX0knob1 (float val);
    void setmonoFX0knob2 (float val);
    void setmonoFX0combobox (float val);
    
    void setmonoFX1knob0 (float val);
    void setmonoFX1knob1 (float val);
    void setmonoFX1knob2 (float val);
    void setmonoFX1combobox (float val);
    
    void setmonoFX2knob0 (float val);
    void setmonoFX2knob1 (float val);
    void setmonoFX2knob2 (float val);
    void setmonoFX2combobox (float val);
    
    void setmonoFX3knob0 (float val);
    void setmonoFX3knob1 (float val);
    void setmonoFX3knob2 (float val);
    void setmonoFX3combobox (float val);    
    
    void set_global_midi_octaves_offset(float val);
    //void set_global_midi_channel(float val);
    
    void aboutToQuit();

protected slots:
    void fileSelect();    
    void selectionChanged();   
    void file_browser_load_button_pressed();
    void file_browser_up_button_pressed();
    void file_browser_preview_button_pressed();
    void file_browser_folder_clicked(QListWidgetItem *);
    void file_browser_bookmark_clicked(QListWidgetItem *);
    void file_browser_bookmark_button_pressed();
    void file_browser_bookmark_delete_button_pressed();
    
    void oscRecv();
    
    /*synth_qt_gui.h Autogenerated slots*/

    void sample_note0Changed(int);
    void sample_note1Changed(int);
    void sample_note2Changed(int);
    void sample_note3Changed(int);
    void sample_note4Changed(int);
    void sample_note5Changed(int);
    void sample_note6Changed(int);
    void sample_note7Changed(int);
    void sample_note8Changed(int);
    void sample_note9Changed(int);
    void sample_note10Changed(int);
    void sample_note11Changed(int);
    void sample_note12Changed(int);
    void sample_note13Changed(int);
    void sample_note14Changed(int);
    void sample_note15Changed(int);
    void sample_note16Changed(int);
    void sample_note17Changed(int);
    void sample_note18Changed(int);
    void sample_note19Changed(int);
    void sample_note20Changed(int);
    void sample_note21Changed(int);
    void sample_note22Changed(int);
    void sample_note23Changed(int);
    void sample_note24Changed(int);
    void sample_note25Changed(int);
    void sample_note26Changed(int);
    void sample_note27Changed(int);
    void sample_note28Changed(int);
    void sample_note29Changed(int);
    void sample_note30Changed(int);
    void sample_note31Changed(int);
    void sample_lnote0Changed(int);
    void sample_lnote1Changed(int);
    void sample_lnote2Changed(int);
    void sample_lnote3Changed(int);
    void sample_lnote4Changed(int);
    void sample_lnote5Changed(int);
    void sample_lnote6Changed(int);
    void sample_lnote7Changed(int);
    void sample_lnote8Changed(int);
    void sample_lnote9Changed(int);
    void sample_lnote10Changed(int);
    void sample_lnote11Changed(int);
    void sample_lnote12Changed(int);
    void sample_lnote13Changed(int);
    void sample_lnote14Changed(int);
    void sample_lnote15Changed(int);
    void sample_lnote16Changed(int);
    void sample_lnote17Changed(int);
    void sample_lnote18Changed(int);
    void sample_lnote19Changed(int);
    void sample_lnote20Changed(int);
    void sample_lnote21Changed(int);
    void sample_lnote22Changed(int);
    void sample_lnote23Changed(int);
    void sample_lnote24Changed(int);
    void sample_lnote25Changed(int);
    void sample_lnote26Changed(int);
    void sample_lnote27Changed(int);
    void sample_lnote28Changed(int);
    void sample_lnote29Changed(int);
    void sample_lnote30Changed(int);
    void sample_lnote31Changed(int);
    void sample_hnote0Changed(int);
    void sample_hnote1Changed(int);
    void sample_hnote2Changed(int);
    void sample_hnote3Changed(int);
    void sample_hnote4Changed(int);
    void sample_hnote5Changed(int);
    void sample_hnote6Changed(int);
    void sample_hnote7Changed(int);
    void sample_hnote8Changed(int);
    void sample_hnote9Changed(int);
    void sample_hnote10Changed(int);
    void sample_hnote11Changed(int);
    void sample_hnote12Changed(int);
    void sample_hnote13Changed(int);
    void sample_hnote14Changed(int);
    void sample_hnote15Changed(int);
    void sample_hnote16Changed(int);
    void sample_hnote17Changed(int);
    void sample_hnote18Changed(int);
    void sample_hnote19Changed(int);
    void sample_hnote20Changed(int);
    void sample_hnote21Changed(int);
    void sample_hnote22Changed(int);
    void sample_hnote23Changed(int);
    void sample_hnote24Changed(int);
    void sample_hnote25Changed(int);
    void sample_hnote26Changed(int);
    void sample_hnote27Changed(int);
    void sample_hnote28Changed(int);
    void sample_hnote29Changed(int);
    void sample_hnote30Changed(int);
    void sample_hnote31Changed(int);
    void sample_vol0Changed(int);
    void sample_vol1Changed(int);
    void sample_vol2Changed(int);
    void sample_vol3Changed(int);
    void sample_vol4Changed(int);
    void sample_vol5Changed(int);
    void sample_vol6Changed(int);
    void sample_vol7Changed(int);
    void sample_vol8Changed(int);
    void sample_vol9Changed(int);
    void sample_vol10Changed(int);
    void sample_vol11Changed(int);
    void sample_vol12Changed(int);
    void sample_vol13Changed(int);
    void sample_vol14Changed(int);
    void sample_vol15Changed(int);
    void sample_vol16Changed(int);
    void sample_vol17Changed(int);
    void sample_vol18Changed(int);
    void sample_vol19Changed(int);
    void sample_vol20Changed(int);
    void sample_vol21Changed(int);
    void sample_vol22Changed(int);
    void sample_vol23Changed(int);
    void sample_vol24Changed(int);
    void sample_vol25Changed(int);
    void sample_vol26Changed(int);
    void sample_vol27Changed(int);
    void sample_vol28Changed(int);
    void sample_vol29Changed(int);
    void sample_vol30Changed(int);
    void sample_vol31Changed(int);
    //new
    void sample_vel_sens0Changed(int);
    void sample_vel_sens1Changed(int);
    void sample_vel_sens2Changed(int);
    void sample_vel_sens3Changed(int);
    void sample_vel_sens4Changed(int);
    void sample_vel_sens5Changed(int);
    void sample_vel_sens6Changed(int);
    void sample_vel_sens7Changed(int);
    void sample_vel_sens8Changed(int);
    void sample_vel_sens9Changed(int);
    void sample_vel_sens10Changed(int);
    void sample_vel_sens11Changed(int);
    void sample_vel_sens12Changed(int);
    void sample_vel_sens13Changed(int);
    void sample_vel_sens14Changed(int);
    void sample_vel_sens15Changed(int);
    void sample_vel_sens16Changed(int);
    void sample_vel_sens17Changed(int);
    void sample_vel_sens18Changed(int);
    void sample_vel_sens19Changed(int);
    void sample_vel_sens20Changed(int);
    void sample_vel_sens21Changed(int);
    void sample_vel_sens22Changed(int);
    void sample_vel_sens23Changed(int);
    void sample_vel_sens24Changed(int);
    void sample_vel_sens25Changed(int);
    void sample_vel_sens26Changed(int);
    void sample_vel_sens27Changed(int);
    void sample_vel_sens28Changed(int);
    void sample_vel_sens29Changed(int);
    void sample_vel_sens30Changed(int);
    void sample_vel_sens31Changed(int);
    void sample_vel_low0Changed(int);
    void sample_vel_low1Changed(int);
    void sample_vel_low2Changed(int);
    void sample_vel_low3Changed(int);
    void sample_vel_low4Changed(int);
    void sample_vel_low5Changed(int);
    void sample_vel_low6Changed(int);
    void sample_vel_low7Changed(int);
    void sample_vel_low8Changed(int);
    void sample_vel_low9Changed(int);
    void sample_vel_low10Changed(int);
    void sample_vel_low11Changed(int);
    void sample_vel_low12Changed(int);
    void sample_vel_low13Changed(int);
    void sample_vel_low14Changed(int);
    void sample_vel_low15Changed(int);
    void sample_vel_low16Changed(int);
    void sample_vel_low17Changed(int);
    void sample_vel_low18Changed(int);
    void sample_vel_low19Changed(int);
    void sample_vel_low20Changed(int);
    void sample_vel_low21Changed(int);
    void sample_vel_low22Changed(int);
    void sample_vel_low23Changed(int);
    void sample_vel_low24Changed(int);
    void sample_vel_low25Changed(int);
    void sample_vel_low26Changed(int);
    void sample_vel_low27Changed(int);
    void sample_vel_low28Changed(int);
    void sample_vel_low29Changed(int);
    void sample_vel_low30Changed(int);
    void sample_vel_low31Changed(int);
    void sample_vel_high0Changed(int);
    void sample_vel_high1Changed(int);
    void sample_vel_high2Changed(int);
    void sample_vel_high3Changed(int);
    void sample_vel_high4Changed(int);
    void sample_vel_high5Changed(int);
    void sample_vel_high6Changed(int);
    void sample_vel_high7Changed(int);
    void sample_vel_high8Changed(int);
    void sample_vel_high9Changed(int);
    void sample_vel_high10Changed(int);
    void sample_vel_high11Changed(int);
    void sample_vel_high12Changed(int);
    void sample_vel_high13Changed(int);
    void sample_vel_high14Changed(int);
    void sample_vel_high15Changed(int);
    void sample_vel_high16Changed(int);
    void sample_vel_high17Changed(int);
    void sample_vel_high18Changed(int);
    void sample_vel_high19Changed(int);
    void sample_vel_high20Changed(int);
    void sample_vel_high21Changed(int);
    void sample_vel_high22Changed(int);
    void sample_vel_high23Changed(int);
    void sample_vel_high24Changed(int);
    void sample_vel_high25Changed(int);
    void sample_vel_high26Changed(int);
    void sample_vel_high27Changed(int);
    void sample_vel_high28Changed(int);
    void sample_vel_high29Changed(int);
    void sample_vel_high30Changed(int);
    void sample_vel_high31Changed(int);

    void sample_pitch0Changed(int);
    void sample_pitch1Changed(int);
    void sample_pitch2Changed(int);
    void sample_pitch3Changed(int);
    void sample_pitch4Changed(int);
    void sample_pitch5Changed(int);
    void sample_pitch6Changed(int);
    void sample_pitch7Changed(int);
    void sample_pitch8Changed(int);
    void sample_pitch9Changed(int);
    void sample_pitch10Changed(int);
    void sample_pitch11Changed(int);
    void sample_pitch12Changed(int);
    void sample_pitch13Changed(int);
    void sample_pitch14Changed(int);
    void sample_pitch15Changed(int);
    void sample_pitch16Changed(int);
    void sample_pitch17Changed(int);
    void sample_pitch18Changed(int);
    void sample_pitch19Changed(int);
    void sample_pitch20Changed(int);
    void sample_pitch21Changed(int);
    void sample_pitch22Changed(int);
    void sample_pitch23Changed(int);
    void sample_pitch24Changed(int);
    void sample_pitch25Changed(int);
    void sample_pitch26Changed(int);
    void sample_pitch27Changed(int);
    void sample_pitch28Changed(int);
    void sample_pitch29Changed(int);
    void sample_pitch30Changed(int);
    void sample_pitch31Changed(int);
    void sample_tune0Changed(int);
    void sample_tune1Changed(int);
    void sample_tune2Changed(int);
    void sample_tune3Changed(int);
    void sample_tune4Changed(int);
    void sample_tune5Changed(int);
    void sample_tune6Changed(int);
    void sample_tune7Changed(int);
    void sample_tune8Changed(int);
    void sample_tune9Changed(int);
    void sample_tune10Changed(int);
    void sample_tune11Changed(int);
    void sample_tune12Changed(int);
    void sample_tune13Changed(int);
    void sample_tune14Changed(int);
    void sample_tune15Changed(int);
    void sample_tune16Changed(int);
    void sample_tune17Changed(int);
    void sample_tune18Changed(int);
    void sample_tune19Changed(int);
    void sample_tune20Changed(int);
    void sample_tune21Changed(int);
    void sample_tune22Changed(int);
    void sample_tune23Changed(int);
    void sample_tune24Changed(int);
    void sample_tune25Changed(int);
    void sample_tune26Changed(int);
    void sample_tune27Changed(int);
    void sample_tune28Changed(int);
    void sample_tune29Changed(int);
    void sample_tune30Changed(int);
    void sample_tune31Changed(int);
    void sample_interpolation_mode0Changed(int);
    void sample_interpolation_mode1Changed(int);
    void sample_interpolation_mode2Changed(int);
    void sample_interpolation_mode3Changed(int);
    void sample_interpolation_mode4Changed(int);
    void sample_interpolation_mode5Changed(int);
    void sample_interpolation_mode6Changed(int);
    void sample_interpolation_mode7Changed(int);
    void sample_interpolation_mode8Changed(int);
    void sample_interpolation_mode9Changed(int);
    void sample_interpolation_mode10Changed(int);
    void sample_interpolation_mode11Changed(int);
    void sample_interpolation_mode12Changed(int);
    void sample_interpolation_mode13Changed(int);
    void sample_interpolation_mode14Changed(int);
    void sample_interpolation_mode15Changed(int);
    void sample_interpolation_mode16Changed(int);
    void sample_interpolation_mode17Changed(int);
    void sample_interpolation_mode18Changed(int);
    void sample_interpolation_mode19Changed(int);
    void sample_interpolation_mode20Changed(int);
    void sample_interpolation_mode21Changed(int);
    void sample_interpolation_mode22Changed(int);
    void sample_interpolation_mode23Changed(int);
    void sample_interpolation_mode24Changed(int);
    void sample_interpolation_mode25Changed(int);
    void sample_interpolation_mode26Changed(int);
    void sample_interpolation_mode27Changed(int);
    void sample_interpolation_mode28Changed(int);
    void sample_interpolation_mode29Changed(int);
    void sample_interpolation_mode30Changed(int);
    void sample_interpolation_mode31Changed(int);
/*End synth_qt_gui.h Autogenerated slots*/

    //From Ray-V
    
    void attackChanged (int);
    void decayChanged  (int);
    void sustainChanged(int);
    void releaseChanged(int);
            
    void filterAttackChanged (int);
    void filterDecayChanged  (int);
    void filterSustainChanged(int);
    void filterReleaseChanged(int);
    
    void noiseAmpChanged(int);
    void noise_typeChanged(int);
    
    void masterVolumeChanged(int);
    
    void masterUnisonVoicesChanged(int);
    void masterUnisonSpreadChanged(int);
    void masterGlideChanged(int);
    void masterPitchbendAmtChanged(int);
    
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
    
    void monofx0knob0Changed(int);
    void monofx0knob1Changed(int);
    void monofx0knob2Changed(int);
    void monofx0comboboxChanged(int);
    
    void monofx1knob0Changed(int);
    void monofx1knob1Changed(int);
    void monofx1knob2Changed(int);
    void monofx1comboboxChanged(int);
    
    void monofx2knob0Changed(int);
    void monofx2knob1Changed(int);
    void monofx2knob2Changed(int);
    void monofx2comboboxChanged(int);
    
    void monofx3knob0Changed(int);
    void monofx3knob1Changed(int);
    void monofx3knob2Changed(int);
    void monofx3comboboxChanged(int);
    
    void sample_selected_monofx_groupChanged(int);
    
    void global_midi_octaves_offsetChanged(int);
    
    //void global_midi_channelChanged(int);
    
    //End from Modulex
    
    //From PolyFX mod matrix
    
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
    bool m_suppress_selected_sample_changed;
    
    //From Modulex
    
    LMS_multieffect * m_fx0;
    LMS_multieffect * m_fx1;
    LMS_multieffect * m_fx2;
    LMS_multieffect * m_fx3;
    
    
    LMS_multieffect * m_mono_fx0;
    LMS_multieffect * m_mono_fx1;
    LMS_multieffect * m_mono_fx2;
    LMS_multieffect * m_mono_fx3;
    
    //End from Modulex
    
    LMS_group_box *m_global_midi_settings_groupbox;
    LMS_spinbox *m_global_midi_octaves_offset;
    LMS_combobox *m_global_midi_channel;
    
    //From Ray-V PolyFX
    
    LMS_main_layout * m_main_layout;
    
    LMS_adsr_widget * m_adsr_amp;
    LMS_adsr_widget * m_adsr_filter;        
    
    LMS_ramp_env * m_pitch_env;
    
    LMS_lfo_widget * m_lfo;
    
    LMS_master_widget * m_master;
            
    LMS_group_box * m_groupbox_noise;
    LMS_knob_regular *m_noise_amp;
    LMS_combobox *m_noise_type;
    LMS_sample_graph *m_sample_graph;
    
    void sample_noteChanged(int);
    void sample_lnoteChanged(int);
    void sample_hnoteChanged(int);
    void sample_volChanged(int);

    void sample_vel_highChanged(int);
    void sample_vel_lowChanged(int);
    void sample_vel_sensChanged(int);
    //new
    void sample_pitchChanged(int);
    void sample_tuneChanged(int);
    void sample_interpolation_modeChanged(int);
    
    void pfxmatrix_Changed(int,int,int,int,int);
    /*The currently selected sample for viewing/editing */
    //int m_selected_sample_index;    
    /*The index of C, C#, D, D#, E, etc... in the QCombobox*/
    int m_note_indexes [LMS_MAX_SAMPLE_COUNT];
    
    QAction *actionMove_files_to_single_directory;
    QAction *actionSave_instrument_to_file;
    QAction *actionOpen_instrument_from_file;
    QAction *actionMapToWhiteKeys;
    QAction *actionClearAllSamples;
    
    QMenuBar *menubar;
    QMenu *menuFile;
        
    //LMS_sample_viewer * m_sample_viewer;        
    QVBoxLayout *m_smp_tab_main_verticalLayout;
    QVBoxLayout *m_main_v_layout;
    QTabWidget *m_main_tab;
    QWidget *m_sample_tab;
    QHBoxLayout *m_sample_tab_horizontalLayout;    
    QWidget *m_smp_tab_scrollAreaWidgetContents;
    QHBoxLayout *horizontalLayout;
    QWidget *m_poly_fx_tab;
    QWidget *m_mono_fx_tab;
    LMS_main_layout *m_mono_fx_tab_main_layout;
    QHBoxLayout *m_mono_fx_tab_selected_hlayout;
    QComboBox *m_mono_fx_tab_selected_sample;
    QComboBox *m_mono_fx_tab_selected_group;
    QLabel *m_mono_fx_tab_selected_sample_label;
    QLabel *m_mono_fx_tab_selected_group_label;
    
    QWidget *m_view_sample_tab;
    
    //m_view_sample_tab
    
    //QWidget *verticalLayoutWidget;
    QVBoxLayout *m_view_sample_tab_main_vlayout;
    QHBoxLayout *m_sample_start_hlayout;
    QSpacerItem *m_sample_start_left_hspacer;
    QHBoxLayout *m_sample_loop_start_hlayout;
    QSpacerItem *m_sample_loop_start_left_hspacer;
    QSlider *m_sample_start_hslider;
    QSlider *m_sample_end_hslider;
    QSlider *m_sample_loop_start_hslider;
    QSlider *m_sample_loop_end_hslider;
    QSpacerItem *m_sample_start_right_hspacer;
    QSpacerItem *m_sample_loop_start_right_hspacer;
    QHBoxLayout *m_sample_graph_hlayout;
    QSpacerItem *m_sample_graph_left_hspacer;
    QLabel *m_sample_graph_label;
    QSpacerItem *m_sample_graph_right_hspacer;
    QHBoxLayout *m_sample_end_hlayout;
    QHBoxLayout *m_sample_loop_end_hlayout;
    QSpacerItem *m_sample_end_left_hspacer;    
    QSpacerItem *m_sample_end_right_hspacer;
    QSpacerItem *m_sample_loop_end_left_hspacer;    
    QSpacerItem *m_sample_loop_end_right_hspacer;
    QHBoxLayout *m_sample_view_select_sample_hlayout;
    QSpacerItem *m_sample_view_extra_controls_left_hspacer;
    QGridLayout *m_sample_view_extra_controls_gridview;
    QComboBox *m_selected_sample_index_combobox;
    QLabel *m_selected_sample_index_label;
    QComboBox *m_loop_mode_combobox;
    QLabel *m_loop_mode_label;
    QSpacerItem *m_sample_view_extra_controls_right_hspacer;
    QHBoxLayout *m_sample_view_file_select_hlayout;
    QSpacerItem *m_sample_view_file_select_left_hspacer;    
    QSpacerItem *m_sample_view_file_select_right_hspacer;
    QSpacerItem *m_view_sample_tab_lower_vspacer;

    LMS_file_select * m_view_file_selector;    
    
    //end m_view_sample_tab
    
    lo_address m_host;
    QByteArray m_controlPath;
    QByteArray m_midiPath;
    QByteArray m_configurePath;
    QByteArray m_exitingPath;

    QString m_files;
    QString m_projectDir;
    int m_previewWidth;
    int m_previewHeight;

    bool m_hostRequestedQuit;
    bool m_ready;
};


#endif
