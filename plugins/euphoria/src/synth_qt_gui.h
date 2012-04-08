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
#include "../../libmodsynth/widgets/ui_modules/sample_viewer.h"
#include "../../libmodsynth/widgets/mod_matrix.h"
#include "../../libmodsynth/widgets/lms_file_select.h"

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
    
    /*To prevent controls that update other controls from going berserk*/
    bool m_handle_control_updates;
    
    void calculate_fx_label(int,int,QLabel*);
public slots:
    void setSampleFile(QString file);
        
    void setSelection(int);
    
    void aboutToQuit();

protected slots:
    void fileSelect();    
    void selectionChanged();
    
    
    void oscRecv();
    

protected:
    /*The currently selected sample for viewing/editing */
    //int m_selected_sample_index;    
    /*The index of C, C#, D, D#, E, etc... in the QCombobox*/
    int m_note_indexes [LMS_MAX_SAMPLE_COUNT];
    
    //LMS_sample_viewer * m_sample_viewer;
    LMS_file_select * m_file_selector;
    
    QVBoxLayout *m_smp_tab_main_verticalLayout;
    QHBoxLayout *horizontalLayout_5;
    QTabWidget *m_main_tab;
    QWidget *m_sample_tab;
    QHBoxLayout *horizontalLayout_2;
    QScrollArea *m_smp_tab_scrollArea;
    QWidget *m_smp_tab_scrollAreaWidgetContents;
    QHBoxLayout *horizontalLayout;
    QWidget *m_poly_fx_tab;
    QHBoxLayout *horizontalLayout_4;
    QVBoxLayout *m_poly_fx_vertical_layout;
    QHBoxLayout *m_poly_fx_Layout;
    
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
