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
    
    void set_selected_sample_combobox_item(int,QString);
    
    //From Ray-V   TODO:  Are these necessary?
    
    void v_set_control(int, float);
    void v_control_changed(int, int, bool);
    int i_get_control(int);
    
    void v_print_port_name_to_cerr(int);
    
    void lms_value_changed(int, LMS_control *);
    void lms_set_value(float, LMS_control *);
        
    //End from Ray-V
        
    int m_sample_starts[LMS_MAX_SAMPLE_COUNT];  //0 to 10000, not the actual sample number.  0 == the very start of the sample, 10000 == the very end
    int m_sample_ends[LMS_MAX_SAMPLE_COUNT];  //0 to 10000, not the actual sample number.  READ CAREFULLY:  0 == the very end of the sample, 10000 == the very beginning (IT'S INVERTED)
    
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
    void viewSampleSelectedIndexChanged(int);
    
    //From Ray-V PolyFX
    void setAttack (float);
    void setDecay  (float);
    void setSustain(float);
    void setRelease(float);
    void setTimbre (float);
    void setRes (float);
    void setFilterType (float val);
    void setDist (float);
        
    void setFilterAttack (float);
    void setFilterDecay  (float);
    void setFilterSustain(float);
    void setFilterRelease(float);
    
    void setNoiseAmp(float);
    
    void setFilterEnvAmt(float);
    void setDistWet(float);    
    void setMasterVolume(float);
    
    void setMasterUnisonVoices(float);
    void setMasterUnisonSpread(float);
    void setMasterGlide(float);
    void setMasterPitchbendAmt(float);
    
    void setPitchEnvTime(float);
    void setPitchEnvAmt(float);
    
    void setLFOfreq(float);
    void setLFOtype(float);
    void setLFOamp(float);
    void setLFOpitch(float);
    void setLFOcutoff(float);
    //End from Ray-V
    
    
    void aboutToQuit();

protected slots:
    void fileSelect();    
    void selectionChanged();   
    
    void oscRecv();
    
    /*synth_qt_gui.h Autogenerated slots*/

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

/*End synth_qt_gui.h Autogenerated slots*/

    //From Ray-V
    
    void attackChanged (int);
    void decayChanged  (int);
    void sustainChanged(int);
    void releaseChanged(int);
    void timbreChanged (int);
    void filterTypeChanged (int);
    void resChanged (int);
    void distChanged (int);
        
    void filterAttackChanged (int);
    void filterDecayChanged  (int);
    void filterSustainChanged(int);
    void filterReleaseChanged(int);
    
    void noiseAmpChanged(int);

    
    void filterEnvAmtChanged(int);
    void distWetChanged(int);
    void masterVolumeChanged(int);
    
    void masterUnisonVoicesChanged(int);
    void masterUnisonSpreadChanged(int);
    void masterGlideChanged(int);
    void masterPitchbendAmtChanged(int);
    
    void pitchEnvTimeChanged(int);
    void pitchEnvAmtChanged(int);
    
    void LFOfreqChanged(int);
    void LFOtypeChanged(int);
    void LFOampChanged(int);
    void LFOpitchChanged(int);
    void LFOcutoffChanged(int);
    
    //End from Ray-V

protected:
    bool m_suppress_selected_sample_changed;
    //From Ray-V PolyFX
    
    LMS_main_layout * m_main_layout;
    
    LMS_adsr_widget * m_adsr_amp;
    LMS_adsr_widget * m_adsr_filter;        
    LMS_filter_widget * m_filter;
    LMS_knob_regular * m_filter_env_amt;
    LMS_ramp_env * m_pitch_env;
    
    LMS_lfo_widget * m_lfo;
    LMS_knob_regular *m_lfo_amp;
    LMS_knob_regular *m_lfo_pitch;
    LMS_knob_regular *m_lfo_cutoff;
    
    LMS_master_widget * m_master;
    
    LMS_group_box * m_groupbox_distortion;
    LMS_knob_regular *m_dist;
    LMS_knob_regular *m_dist_wet;
        
    LMS_group_box * m_groupbox_noise;
    LMS_knob_regular *m_noise_amp;
    LMS_sample_graph *m_sample_graph;
    
    //End from Ray-V
    
    void sample_pitchChanged(int);
    void sample_lnoteChanged(int);
    void sample_hnoteChanged(int);
    void sample_volChanged(int);
    /*The currently selected sample for viewing/editing */
    //int m_selected_sample_index;    
    /*The index of C, C#, D, D#, E, etc... in the QCombobox*/
    int m_note_indexes [LMS_MAX_SAMPLE_COUNT];
    
    QAction *actionMove_files_to_single_directory;
    QAction *actionSave_instrument_to_file;
    QAction *actionOpen_instrument_from_file;
    QMenuBar *menubar;
    QMenu *menuFile;
        
    //LMS_sample_viewer * m_sample_viewer;
    LMS_file_select * m_file_selector;
    
    QVBoxLayout *m_smp_tab_main_verticalLayout;
    QVBoxLayout *m_main_v_layout;
    QTabWidget *m_main_tab;
    QWidget *m_sample_tab;
    QHBoxLayout *horizontalLayout_2;
    QScrollArea *m_smp_tab_scrollArea;
    QWidget *m_smp_tab_scrollAreaWidgetContents;
    QHBoxLayout *horizontalLayout;
    QWidget *m_poly_fx_tab;
    QWidget *m_view_sample_tab;
    
    //m_view_sample_tab
    
    //QWidget *verticalLayoutWidget;
    QVBoxLayout *m_view_sample_tab_main_vlayout;
    QHBoxLayout *m_sample_start_hlayout;
    QSpacerItem *m_sample_start_left_hspacer;
    QSlider *m_sample_start_hslider;
    QSpacerItem *m_sample_start_right_hspacer;
    QHBoxLayout *m_sample_graph_hlayout;
    QSpacerItem *m_sample_graph_left_hspacer;
    QLabel *m_sample_graph_label;
    QSpacerItem *m_sample_graph_right_hspacer;
    QHBoxLayout *m_sample_end_hlayout;
    QSpacerItem *m_sample_end_left_hspacer;
    QSlider *m_sample_end_hslider;
    QSpacerItem *m_sample_end_right_hspacer;
    QHBoxLayout *m_sample_view_select_sample_hlayout;
    QSpacerItem *m_sample_view_extra_controls_left_hspacer;
    QGridLayout *m_sample_view_extra_controls_gridview;
    QComboBox *m_selected_sample_index_combobox;
    QLabel *m_selected_sample_index_label;
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
