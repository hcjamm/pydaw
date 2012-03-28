/* -*- c-basic-offset: 4 -*-  vi:set ts=8 sts=4 sw=4: */

/* trivial_sampler_qt_gui.h

   DSSI Soft Synth Interface
   Constructed by Chris Cannam, Steve Harris and Sean Bolton

   A straightforward DSSI plugin sampler Qt GUI.

   This example file is in the public domain.
*/

#ifndef _TRIVIAL_SAMPLER_QT_GUI_H_INCLUDED_
#define _TRIVIAL_SAMPLER_QT_GUI_H_INCLUDED_

#include <QFrame>
#include <QCheckBox>
#include <QSpinBox>
#include <QLabel>
#include <QSlider>
#include <QLayout>

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDial>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMainWindow>
#include <QtGui/QPushButton>
#include <QtGui/QScrollArea>
#include <QtGui/QSlider>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QTabWidget>
#include <QtGui/QTableWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include <QRadioButton>

#include "synth.h"

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
    
    void retranslateUi(QFrame * Frame);
    void findSelected();
    
public slots:        
    void setSampleFile(QString file);
    void updateSampleTable();
    
    void setSelection(int);
    
    void setSampleStart(int);
    void setSampleEnd(int);
    void setSampleStartFine(int);
    void setSampleEndFine(int);
    
    void setLoopStart(int);
    void setLoopEnd(int);
    void setLoopStartFine(int);
    void setLoopEndFine(int);
    
    /*
    void setRetune(bool retune);
    void setBasePitch(int pitch);
    void setSustain(bool sustain);
    void setRelease(int ms);
    void setBalance(int balance);
    void setProjectDirectory(QString dir);
    */
    void aboutToQuit();

protected slots:
    void fileSelect();
    
    void selectionChanged();
    
    void sampleStartChanged(int);
    void sampleEndChanged(int);
    void sampleStartFineChanged(int);
    void sampleEndFineChanged(int);
    
    void loopStartChanged(int);
    void loopEndChanged(int);
    void loopStartFineChanged(int);
    void loopEndFineChanged(int);
    /*
    void retuneChanged(bool);
    void basePitchChanged(int);
    void sustainChanged(bool);
    void releaseChanged(int);
    void balanceChanged(int);
    void test_press();
    void test_release();
    */
    void oscRecv();
    void generatePreview(QString file);

protected:
    //QLabel *m_sampleFile;
    //QLabel *m_duration;
    //QLabel *m_channels;
    //QLabel *m_sampleRate;
    //QLabel *m_preview;
    //QLabel *m_balanceLabel;
    //QCheckBox *m_retune;
    //QSpinBox *m_basePitch;
    //QCheckBox *m_sustain;
    //QSpinBox *m_release;
    //QSlider *m_balance;
    
    QHBoxLayout *horizontalLayout_5;
    QTabWidget *m_main_tab;
    QWidget *tab_4;
    QHBoxLayout *horizontalLayout_2;
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout_20;
    QTableWidget *m_sample_table;
    
    QRadioButton * m_selected_sample[LMS_MAX_SAMPLE_COUNT];
    int m_selected_sample_index;
    QPixmap m_sample_graphs[LMS_MAX_SAMPLE_COUNT];
    int m_note_indexes [LMS_MAX_SAMPLE_COUNT];
    
    QHBoxLayout *horizontalLayout_16;
    QVBoxLayout *verticalLayout_21;
    QHBoxLayout *horizontalLayout_17;
    QLineEdit *m_file_path;
    QPushButton *m_load_sample;
    QHBoxLayout *horizontalLayout_18;
    QSpacerItem *horizontalSpacer_9;
    QGridLayout *gridLayout_6;
    QSpinBox *m_lnote;
    QLabel *m_sample_numLabel_6;
    QLabel *m_hvelLabel_10;
    QLabel *m_lnoteLabel_5;
    QSpinBox *m_sample_vol;
    QComboBox *m_note;
    QSpinBox *m_lvel;
    QSpinBox *m_hnote;
    QLabel *m_noteLabel_8;
    QLabel *m_noteLabel_9;
    QComboBox *m_play_mode;
    QSpinBox *m_sample_num;
    QSpinBox *m_octave;
    QLabel *m_octaveLabel_5;
    QLabel *m_lvelLabel_5;
    QLabel *m_hvelLabel_9;
    QLabel *m_hnoteLabel_5;
    QSpinBox *m_hvel;
    QLabel *m_noteLabel_10;
    QComboBox *m_sample_fx_group;
    QPushButton *m_update_sample;
    QSpacerItem *horizontalSpacer_10;
    QVBoxLayout *verticalLayout_22;
    QLabel *m_sample_graph;
    QGridLayout *gridLayout_10;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label_21;
    QSpinBox *m_sample_start_fine;
    QLabel *label_22;
    QSpinBox *m_sample_end_fine;
    QSpacerItem *horizontalSpacer_5;
    QSlider *m_sample_start;
    QLabel *label_23;
    QSlider *m_sample_end;
    QGridLayout *gridLayout_11;
    QHBoxLayout *horizontalLayout_7;
    QLabel *label_24;
    QSpinBox *m_loop_start_fine;
    QLabel *label_25;
    QSpinBox *m_loop_end_fine;
    QSpacerItem *horizontalSpacer_6;
    QSlider *m_loop_start;
    QLabel *label_26;
    QSlider *m_loop_end;
    QWidget *tab_2;
    QHBoxLayout *horizontalLayout_4;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label;
    QSpinBox *m_edit_fx_group_num;
    QSpacerItem *horizontalSpacer_3;
    QGridLayout *gridLayout;
    QGroupBox *groupBox_5;
    QVBoxLayout *verticalLayout_6;
    QGridLayout *gridLayout_7;
    QDial *m_fx_knob_4_3;
    QLabel *m_fx_label_4_2;
    QLabel *label_19;
    QDial *m_fx_knob_4_2;
    QComboBox *comboBox_5;
    QLabel *m_fx_label_4_1;
    QLabel *m_fx_label_4_3;
    QDial *m_fx_knob_4_1;
    QSpacerItem *horizontalSpacer_2;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout_2;
    QGridLayout *gridLayout_3;
    QDial *m_fx_knob_1_3;
    QLabel *m_fx_label_1_2;
    QLabel *label_8;
    QDial *m_fx_knob_1_2;
    QComboBox *comboBox_2;
    QLabel *m_fx_label_1_1;
    QLabel *m_fx_label_1_3;
    QDial *m_fx_knob_1_1;
    QGroupBox *groupBox_3;
    QVBoxLayout *verticalLayout_3;
    QGridLayout *gridLayout_4;
    QDial *m_fx_knob_2_3;
    QLabel *m_fx_label_2_2;
    QLabel *label_11;
    QDial *m_fx_knob_2_2;
    QComboBox *comboBox_3;
    QLabel *m_fx_label_2_1;
    QLabel *m_fx_label_2_3;
    QDial *m_fx_knob_2_1;
    QGroupBox *groupBox_4;
    QVBoxLayout *verticalLayout_5;
    QGridLayout *gridLayout_5;
    QDial *m_fx_knob_3_3;
    QLabel *m_fx_label_3_2;
    QLabel *label_15;
    QDial *m_fx_knob_3_2;
    QComboBox *comboBox_4;
    QLabel *m_fx_label_3_1;
    QLabel *m_fx_label_3_3;
    QDial *m_fx_knob_3_1;
    QSpacerItem *horizontalSpacer;
    QGroupBox *groupBox_6;
    QVBoxLayout *verticalLayout_7;
    QGridLayout *gridLayout_8;
    QDial *m_fx_knob_5_3;
    QLabel *m_fx_label_5_2;
    QLabel *label_29;
    QDial *m_fx_knob_5_2;
    QComboBox *comboBox_6;
    QLabel *m_fx_label_5_1;
    QLabel *m_fx_label_5_3;
    QDial *m_fx_knob_5_1;
    QGroupBox *groupBox_8;
    QVBoxLayout *verticalLayout_9;
    QGridLayout *gridLayout_12;
    QLabel *label_4;
    QDial *m_adsr1_a;
    QDial *m_adsr1_s;
    QDial *m_adsr1_d;
    QDial *m_adsr1_r;
    QLabel *label_36;
    QLabel *label_37;
    QLabel *label_38;
    QGroupBox *groupBox_9;
    QVBoxLayout *verticalLayout_11;
    QGridLayout *gridLayout_14;
    QDial *m_adsr2_a;
    QDial *m_adsr2_s;
    QDial *m_adsr2_d;
    QDial *m_adsr2_r;
    QLabel *label_43;
    QLabel *label_44;
    QLabel *m_adsr2Label_a;
    QLabel *label_45;
    QGroupBox *groupBox_10;
    QVBoxLayout *verticalLayout_12;
    QGridLayout *gridLayout_15;
    QDial *dial_32;
    QLabel *label_46;
    QLabel *label_47;
    QDial *dial_33;
    QComboBox *comboBox_8;
    QLabel *label_48;
    QLabel *label_49;
    QDial *dial_34;
    QLabel *label_3;
    QTableWidget *tableWidget_2;
    QWidget *tab;
    QLabel *label_6;
    
    lo_address m_host;
    QByteArray m_controlPath;
    QByteArray m_midiPath;
    QByteArray m_configurePath;
    QByteArray m_exitingPath;

    QString m_file;
    QString m_projectDir;
    int m_previewWidth;
    int m_previewHeight;

    bool m_suppressHostUpdate;
    bool m_hostRequestedQuit;
    bool m_ready;
};


#endif
