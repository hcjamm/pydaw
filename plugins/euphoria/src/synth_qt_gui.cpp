/* -*- c-basic-offset: 4 -*-  vi:set ts=8 sts=4 sw=4: */

/* synth_qt_gui.cpp

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
 */

#include "synth_qt_gui.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QPushButton>
#include <QTimer>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QPainter>
#include <QGroupBox>
#include <QTextStream>
#include <QString>
#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <math.h>
#include <sndfile.h>
#include <quuid.h>
#include "dssi.h"
#include "synth.h"
#include <QFont>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/SM/SMlib.h>

static int handle_x11_error(Display *dpy, XErrorEvent *err)
{
    char errstr[256];
    XGetErrorText(dpy, err->error_code, errstr, 256);
    if (err->error_code != BadWindow) {
	std::cerr << "euphoria_qt_gui: X Error: "
		  << errstr << " " << err->error_code
		  << "\nin major opcode:  " << err->request_code << std::endl;
    }
    return 0;
}
#endif

/*This allows the executable to run standalone for debugging.  This should normally be commented out*/
//#define LMS_DEBUG_STANDALONE

using std::endl;

lo_server osc_server = 0;

static QTextStream cerr(stderr);

/*Used for outputting sampler parameters to text files*/
#define LMS_DELIMITER "|"

/*These define the index of each column in m_sample_table.  Re-order these if you add or remove columns*/
#define SMP_TB_RADIOBUTTON_INDEX 0
#define SMP_TB_NOTE_INDEX 1
#define SMP_TB_LOW_NOTE_INDEX 2
#define SMP_TB_HIGH_NOTE_INDEX 3
#define SMP_TB_VOLUME_INDEX 4
#define SMP_TB_VEL_SENS_INDEX 5
#define SMP_TB_VEL_LOW_INDEX 6
#define SMP_TB_VEL_HIGH_INDEX 7
#define SMP_TB_FILE_PATH_INDEX 8

SamplerGUI::SamplerGUI(bool stereo, const char * host, const char * port,
		       QByteArray controlPath, QByteArray midiPath, QByteArray configurePath,
		       QByteArray exitingPath, QWidget *w) :
    QFrame(w),
    m_controlPath(controlPath),
    m_midiPath(midiPath),
    m_configurePath(configurePath),
    m_exitingPath(exitingPath),
    m_previewWidth(800),
    m_previewHeight(200),
    m_suppressHostUpdate(true),
    m_hostRequestedQuit(false),
    m_ready(false)
{   
    
#ifndef LMS_DEBUG_STANDALONE
    m_host = lo_address_new(host, port);
#endif    
    this->setStyleSheet("QMessageBox{color:white;background-color:black;}  QDial{background-color:rgb(152, 152, 152);} QTabBar::tab:selected { color:black;background-color:#BBBBBB;} QTableView QTableCornerButton::section {background: black; border: 2px outset white;} QComboBox{color:white; background-color:black;} QTabBar::tab {background-color:black;  border: 2px solid white;  border-bottom-color: #333333; border-top-left-radius: 4px;  border-top-right-radius: 4px;  min-width: 8ex;  padding: 2px; color:white;} QHeaderView::section {background: black; color: white;border:2px solid white;} QPushButton {background-color: black; border-style: outset; border-width: 2px; border-radius: 10px;border-color: white;font: bold 14px; min-width: 60px; padding: 6px; color:white;}  QAbstractItemView {outline: none;} QLabel{color:black;background-color:white;border:solid 2px white;border-radius:2px;} QFrame{background-color:qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0.273, stop:0 rgba(90, 90, 90, 255), stop:1 rgba(60, 60, 60, 255))} QGroupBox {background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #111111, stop: 1 #222222); border: 2px solid white;  border-radius: 10px;  margin-top: 1ex;} QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 3px; color:black; background-color: white; border solid 2px white; border-radius:3px;}");
    
    m_handle_control_updates = true;
    m_creating_instrument_file = FALSE;
    m_suppress_selected_sample_changed = FALSE;
    
    LMS_style_info * a_style = new LMS_style_info(64);
    a_style->LMS_set_value_style(QString("color : white; background-color: rgba(0,0,0,0);"), 64);
    a_style->LMS_set_label_style(QString("QLabel{color:black;background-color:white;border:solid 2px white;border-radius:2px; text-align : center;}"), 64);
    //a_style->LMS_set_value_style("")
    
    QList <LMS_mod_matrix_column*> f_sample_table_columns;
        
    f_sample_table_columns << new LMS_mod_matrix_column(radiobutton, QString(""), 0, 1, 0);  //Selected row      
    f_sample_table_columns << new LMS_mod_matrix_column(note_selector, QString("Sample Pitch"), 0, 1, 3);  //Sample base pitch
    f_sample_table_columns << new LMS_mod_matrix_column(note_selector, QString("Low Note"), 0, 1, -2);  //Low Note
    f_sample_table_columns << new LMS_mod_matrix_column(note_selector, QString("High Note"), 0, 1, 8);  //High Note    
    f_sample_table_columns << new LMS_mod_matrix_column(spinbox, QString("Volume"), -50, 36, -6);  //Volume
    f_sample_table_columns << new LMS_mod_matrix_column(spinbox, QString("Vel. Sens."), 0, 20, 10);  //Velocity Sensitivity
    f_sample_table_columns << new LMS_mod_matrix_column(spinbox, QString("Low Vel."), 0, 127, 0);  //Low Velocity
    f_sample_table_columns << new LMS_mod_matrix_column(spinbox, QString("High Vel."), 0, 127, 127);  //High Velocity
    f_sample_table_columns << new LMS_mod_matrix_column(no_widget, QString("Path"), 0, 1, 0);  //File path            
    
    m_sample_table = new LMS_mod_matrix(this, LMS_MAX_SAMPLE_COUNT, f_sample_table_columns, LMS_FIRST_SAMPLE_TABLE_PORT, a_style);
        
    m_file_selector = new LMS_file_select(this);
        /*Set all of the array variables that are per-sample*/
        for(int i = 0; i < LMS_MAX_SAMPLE_COUNT; i++)        
        {           
            QRadioButton * f_rb = (QRadioButton*)m_sample_table->lms_mod_matrix->cellWidget(i , SMP_TB_RADIOBUTTON_INDEX);         
            connect(f_rb, SIGNAL(clicked()), this, SLOT(selectionChanged()));
            
            m_note_indexes[i] = 0;                 
        }
    
        /*Code generated by Qt4 Designer*/
    
        actionMove_files_to_single_directory = new QAction(this);
        actionMove_files_to_single_directory->setObjectName(QString::fromUtf8("actionMove_files_to_single_directory"));
        actionSave_instrument_to_file = new QAction(this);
        actionSave_instrument_to_file->setObjectName(QString::fromUtf8("actionSave_instrument_to_file"));
        actionOpen_instrument_from_file = new QAction(this);
        actionOpen_instrument_from_file->setObjectName(QString::fromUtf8("actionOpen_instrument_from_file"));
        actionMapToWhiteKeys = new QAction(this);
        actionMapToWhiteKeys->setObjectName(QString::fromUtf8("actionMapToWhiteKeys"));
        actionClearAllSamples = new QAction(this);
        actionClearAllSamples->setObjectName(QString::fromUtf8("actionClearAllSamples"));
        
        menubar = new QMenuBar(this);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 25));
        menuFile = new QMenu(menubar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));        
        menubar->addAction(menuFile->menuAction());
        menuFile->addAction(actionMove_files_to_single_directory);
        menuFile->addAction(actionSave_instrument_to_file);
        menuFile->addAction(actionOpen_instrument_from_file);
        menuFile->addAction(actionMapToWhiteKeys);
        menuFile->addAction(actionClearAllSamples);
        
        actionMove_files_to_single_directory->setText(QApplication::translate("MainWindow", "Move files to single directory", 0, QApplication::UnicodeUTF8));
        actionSave_instrument_to_file->setText(QApplication::translate("MainWindow", "Save instrument to file", 0, QApplication::UnicodeUTF8));
        actionOpen_instrument_from_file->setText(QApplication::translate("MainWindow", "Open instrument from file", 0, QApplication::UnicodeUTF8));
        actionMapToWhiteKeys->setText(QString("Map All Samples to 1 White Key"));
        actionClearAllSamples->setText(QString("Clear All Samples"));
        menuFile->setTitle(QApplication::translate("MainWindow", "Menu", 0, QApplication::UnicodeUTF8));
        
        if (this->objectName().isEmpty())
        this->setObjectName(QString::fromUtf8("Frame"));
        this->resize(1200, 800);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(this->sizePolicy().hasHeightForWidth());
        this->setSizePolicy(sizePolicy);
        this->setFrameShape(QFrame::StyledPanel);
        this->setFrameShadow(QFrame::Raised);
        
        m_smp_tab_main_verticalLayout = new QVBoxLayout();
        m_smp_tab_main_verticalLayout->setObjectName(QString::fromUtf8("m_smp_tab_main_verticalLayout"));
        
        m_main_v_layout = new QVBoxLayout(this);
        m_main_v_layout->setObjectName(QString::fromUtf8("m_main_v_layout"));
        m_main_v_layout->addWidget(menubar);
        m_main_tab = new QTabWidget(this);
        m_main_tab->setObjectName(QString::fromUtf8("m_main_tab"));
        m_main_tab->setStyleSheet(QString::fromUtf8(""));
        m_sample_tab = new QWidget();
        m_sample_tab->setObjectName(QString::fromUtf8("m_sample_tab"));
        m_sample_tab_horizontalLayout = new QHBoxLayout(m_sample_tab);
        m_sample_tab_horizontalLayout->setObjectName(QString::fromUtf8("m_sample_tab_horizontalLayout"));        
        m_smp_tab_scrollAreaWidgetContents = new QWidget(m_sample_tab);
        m_smp_tab_scrollAreaWidgetContents->setObjectName(QString::fromUtf8("m_smp_tab_scrollAreaWidgetContents"));
        m_smp_tab_scrollAreaWidgetContents->setGeometry(QRect(0, 0, 966, 728));
        horizontalLayout = new QHBoxLayout(m_smp_tab_scrollAreaWidgetContents);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        
        m_smp_tab_scrollAreaWidgetContents->setStyleSheet(QString("QTableView::item { border: 1px solid white; } QTableWidget::setShowGrid{border: 1px solid white;} QWidget{background-color:black; color:white} QComboBox{background-color:black; color:white; border:solid 1px white;} QComboBox:editable {background-color:black; color:white;} QSpinBox{color:black;background-color:white;}"));

        
        //m_smp_tab_main_verticalLayout->addLayout(m_loop_start_end_Layout);
        m_smp_tab_main_verticalLayout->addWidget(m_sample_table->lms_mod_matrix, Qt::AlignCenter); 
        m_smp_tab_main_verticalLayout->addLayout(m_file_selector->lms_layout);
        
        m_file_browser = new LMS_file_browser(this);
        preview_file = QString("");
        
        connect(m_file_browser->m_folders_listWidget, SIGNAL(itemClicked( QListWidgetItem*)), this, SLOT(file_browser_folder_clicked(QListWidgetItem*)));
        connect(m_file_browser->m_bookmarks_listWidget, SIGNAL(itemClicked( QListWidgetItem*)), this, SLOT(file_browser_bookmark_clicked(QListWidgetItem*)));
        connect(m_file_browser->m_load_pushButton, SIGNAL(clicked()), this, SLOT(file_browser_load_button_pressed()));
        connect(m_file_browser->m_preview_pushButton, SIGNAL(clicked()), this, SLOT(file_browser_preview_button_pressed()));
        connect(m_file_browser->m_up_pushButton, SIGNAL(clicked()), this, SLOT(file_browser_up_button_pressed()));
        connect(m_file_browser->m_bookmark_button, SIGNAL(clicked()), this, SLOT(file_browser_bookmark_button_pressed()));
        connect(m_file_browser->m_bookmarks_delete_button, SIGNAL(clicked()), this, SLOT(file_browser_bookmark_delete_button_pressed()));
        
        horizontalLayout->addLayout(m_file_browser->m_file_browser_verticalLayout, -1);
        
        horizontalLayout->addLayout(m_smp_tab_main_verticalLayout);
        /*
        QStringList f_midi_channels = QStringList() << QString("1") << QString("2") << QString("3") << QString("4") << QString("5") << QString("6")
                << QString("7") << QString("8") << QString("9") << QString("10") << QString("11") << QString("12") << QString("13") << QString("14") 
                << QString("15") << QString("16") << QString("All") ;
        */
        m_global_midi_settings_groupbox = new LMS_group_box(this, QString("MIDI Settings"), a_style);
        m_global_midi_octaves_offset = new LMS_spinbox(QString("Offset"), -3, 3, 1, 0, this, a_style, LMS_GLOBAL_MIDI_OCTAVES_OFFSET);
        //m_global_midi_channel = new LMS_combobox(QString("Channel"), this, f_midi_channels, LMS_GLOBAL_MIDI_CHANNEL, a_style);
        
        m_global_midi_settings_groupbox->lms_add_h(m_global_midi_octaves_offset);
        //m_global_midi_settings_groupbox->lms_add_h(m_global_midi_channel);
        
        QHBoxLayout * f_settings_and_logo_hlayout = new QHBoxLayout();
        f_settings_and_logo_hlayout->addWidget(m_global_midi_settings_groupbox->lms_groupbox, -1, Qt::AlignLeft);
        
        
        QLabel * f_logo_label = new QLabel("", this);    
        f_logo_label->setTextFormat(Qt::RichText);

        QString f_logo_text = "<html><img src=\"data:image/png;base64,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\"/></html>";
        f_logo_label->setText(f_logo_text);
        f_logo_label->setMinimumSize(90, 30);   
        f_logo_label->setAlignment(Qt::AlignCenter);
        f_logo_label->show();
        
        f_settings_and_logo_hlayout->addWidget(f_logo_label, -1, Qt::AlignRight);
        
        
        m_smp_tab_main_verticalLayout->addLayout(f_settings_and_logo_hlayout, -1);
        
        connect(m_global_midi_octaves_offset->lms_spinbox, SIGNAL(valueChanged(int)), this, SLOT(global_midi_octaves_offsetChanged(int)));
        //connect(m_global_midi_channel->lms_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(global_midi_channelChanged(int)));

        m_sample_tab_horizontalLayout->addWidget(m_smp_tab_scrollAreaWidgetContents);

        m_main_tab->addTab(m_sample_tab, QString());
        m_poly_fx_tab = new QWidget();
        m_poly_fx_tab->setObjectName(QString::fromUtf8("m_poly_fx_tab"));
        
        m_main_tab->addTab(m_poly_fx_tab, QString());
                
        m_main_v_layout->addWidget(m_main_tab);
        
        this->setWindowTitle(QApplication::translate("Frame", "Euphoria - Powered by LibModSynth", 0, QApplication::UnicodeUTF8));
        m_main_tab->setTabText(m_main_tab->indexOf(m_sample_tab), QApplication::translate("Frame", "Samples", 0, QApplication::UnicodeUTF8));
        m_main_tab->setTabText(m_main_tab->indexOf(m_poly_fx_tab), QApplication::translate("Frame", "Poly FX", 0, QApplication::UnicodeUTF8));
        
        m_main_tab->setCurrentIndex(0);

        m_sample_table->lms_mod_matrix->resizeColumnsToContents();
        m_sample_table->lms_mod_matrix->resizeRowsToContents();
        
        m_sample_table->lms_mod_matrix->horizontalHeader()->setStretchLastSection(TRUE);
        
        QMetaObject::connectSlotsByName(this);
    
        /*Connect slots manually*/
        connect(m_file_selector->lms_open_button, SIGNAL(pressed()), this, SLOT(fileSelect()));
        connect(m_file_selector->lms_clear_button, SIGNAL(pressed()), this, SLOT(clearFile()));
        connect(m_file_selector->lms_open_in_editor_button, SIGNAL(pressed()), this, SLOT(openInEditor()));
        connect(m_file_selector->lms_reload_button, SIGNAL(pressed()), this, SLOT(reloadSample()));
        connect(actionMove_files_to_single_directory, SIGNAL(triggered()), this, SLOT(moveSamplesToSingleDirectory()));
        connect(actionSave_instrument_to_file, SIGNAL(triggered()), this, SLOT(saveInstrumentToSingleFile()));
        connect(actionOpen_instrument_from_file, SIGNAL(triggered()), this, SLOT(openInstrumentFromFile()));
        connect(actionMapToWhiteKeys, SIGNAL(triggered()), this, SLOT(mapAllSamplesToOneWhiteKey()));
        connect(actionClearAllSamples, SIGNAL(triggered()), this, SLOT(clearAllSamples()));
        
        /*synth_qt_gui.cpp Autogenerated connections*/
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[0]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_pitch0Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[0]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_pitch0Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[1]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_pitch1Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[1]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_pitch1Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[2]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_pitch2Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[2]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_pitch2Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[3]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_pitch3Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[3]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_pitch3Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[4]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_pitch4Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[4]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_pitch4Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[5]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_pitch5Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[5]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_pitch5Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[6]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_pitch6Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[6]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_pitch6Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[7]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_pitch7Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[7]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_pitch7Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[8]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_pitch8Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[8]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_pitch8Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[9]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_pitch9Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[9]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_pitch9Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[10]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_pitch10Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[10]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_pitch10Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[11]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_pitch11Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[11]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_pitch11Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[12]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_pitch12Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[12]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_pitch12Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[13]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_pitch13Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[13]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_pitch13Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[14]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_pitch14Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[14]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_pitch14Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[15]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_pitch15Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[15]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_pitch15Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[16]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_pitch16Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[16]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_pitch16Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[17]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_pitch17Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[17]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_pitch17Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[18]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_pitch18Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[18]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_pitch18Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[19]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_pitch19Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[19]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_pitch19Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[20]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_pitch20Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[20]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_pitch20Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[21]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_pitch21Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[21]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_pitch21Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[22]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_pitch22Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[22]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_pitch22Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[23]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_pitch23Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[23]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_pitch23Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[24]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_pitch24Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[24]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_pitch24Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[25]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_pitch25Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[25]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_pitch25Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[26]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_pitch26Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[26]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_pitch26Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[27]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_pitch27Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[27]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_pitch27Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[28]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_pitch28Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[28]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_pitch28Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[29]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_pitch29Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[29]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_pitch29Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[30]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_pitch30Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[30]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_pitch30Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[31]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_pitch31Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[31]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_pitch31Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[0]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_lnote0Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[0]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_lnote0Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[1]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_lnote1Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[1]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_lnote1Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[2]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_lnote2Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[2]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_lnote2Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[3]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_lnote3Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[3]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_lnote3Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[4]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_lnote4Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[4]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_lnote4Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[5]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_lnote5Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[5]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_lnote5Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[6]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_lnote6Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[6]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_lnote6Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[7]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_lnote7Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[7]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_lnote7Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[8]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_lnote8Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[8]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_lnote8Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[9]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_lnote9Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[9]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_lnote9Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[10]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_lnote10Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[10]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_lnote10Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[11]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_lnote11Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[11]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_lnote11Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[12]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_lnote12Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[12]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_lnote12Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[13]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_lnote13Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[13]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_lnote13Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[14]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_lnote14Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[14]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_lnote14Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[15]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_lnote15Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[15]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_lnote15Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[16]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_lnote16Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[16]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_lnote16Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[17]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_lnote17Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[17]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_lnote17Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[18]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_lnote18Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[18]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_lnote18Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[19]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_lnote19Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[19]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_lnote19Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[20]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_lnote20Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[20]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_lnote20Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[21]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_lnote21Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[21]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_lnote21Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[22]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_lnote22Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[22]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_lnote22Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[23]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_lnote23Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[23]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_lnote23Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[24]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_lnote24Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[24]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_lnote24Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[25]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_lnote25Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[25]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_lnote25Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[26]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_lnote26Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[26]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_lnote26Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[27]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_lnote27Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[27]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_lnote27Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[28]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_lnote28Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[28]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_lnote28Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[29]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_lnote29Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[29]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_lnote29Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[30]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_lnote30Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[30]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_lnote30Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[31]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_lnote31Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[31]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_lnote31Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[0]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_hnote0Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[0]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_hnote0Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[1]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_hnote1Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[1]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_hnote1Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[2]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_hnote2Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[2]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_hnote2Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[3]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_hnote3Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[3]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_hnote3Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[4]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_hnote4Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[4]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_hnote4Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[5]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_hnote5Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[5]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_hnote5Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[6]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_hnote6Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[6]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_hnote6Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[7]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_hnote7Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[7]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_hnote7Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[8]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_hnote8Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[8]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_hnote8Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[9]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_hnote9Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[9]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_hnote9Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[10]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_hnote10Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[10]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_hnote10Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[11]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_hnote11Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[11]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_hnote11Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[12]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_hnote12Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[12]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_hnote12Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[13]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_hnote13Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[13]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_hnote13Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[14]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_hnote14Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[14]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_hnote14Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[15]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_hnote15Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[15]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_hnote15Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[16]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_hnote16Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[16]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_hnote16Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[17]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_hnote17Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[17]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_hnote17Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[18]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_hnote18Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[18]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_hnote18Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[19]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_hnote19Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[19]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_hnote19Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[20]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_hnote20Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[20]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_hnote20Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[21]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_hnote21Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[21]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_hnote21Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[22]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_hnote22Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[22]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_hnote22Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[23]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_hnote23Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[23]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_hnote23Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[24]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_hnote24Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[24]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_hnote24Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[25]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_hnote25Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[25]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_hnote25Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[26]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_hnote26Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[26]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_hnote26Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[27]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_hnote27Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[27]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_hnote27Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[28]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_hnote28Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[28]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_hnote28Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[29]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_hnote29Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[29]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_hnote29Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[30]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_hnote30Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[30]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_hnote30Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[31]))->lms_note, SIGNAL(currentIndexChanged(int)), this, SLOT(sample_hnote31Changed(int)));
        connect(((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[31]))->lms_octave, SIGNAL(valueChanged(int)), this, SLOT(sample_hnote31Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vol0Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vol1Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vol2Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vol3Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[4]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vol4Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[5]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vol5Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[6]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vol6Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[7]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vol7Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[8]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vol8Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[9]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vol9Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[10]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vol10Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[11]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vol11Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[12]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vol12Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[13]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vol13Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[14]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vol14Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[15]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vol15Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[16]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vol16Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[17]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vol17Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[18]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vol18Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[19]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vol19Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[20]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vol20Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[21]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vol21Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[22]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vol22Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[23]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vol23Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[24]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vol24Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[25]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vol25Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[26]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vol26Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[27]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vol27Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[28]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vol28Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[29]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vol29Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[30]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vol30Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[31]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vol31Changed(int)));
        //new
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_SENS_INDEX]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_sens0Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_SENS_INDEX]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_sens1Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_SENS_INDEX]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_sens2Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_SENS_INDEX]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_sens3Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_SENS_INDEX]->controls[4]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_sens4Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_SENS_INDEX]->controls[5]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_sens5Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_SENS_INDEX]->controls[6]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_sens6Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_SENS_INDEX]->controls[7]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_sens7Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_SENS_INDEX]->controls[8]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_sens8Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_SENS_INDEX]->controls[9]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_sens9Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_SENS_INDEX]->controls[10]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_sens10Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_SENS_INDEX]->controls[11]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_sens11Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_SENS_INDEX]->controls[12]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_sens12Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_SENS_INDEX]->controls[13]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_sens13Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_SENS_INDEX]->controls[14]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_sens14Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_SENS_INDEX]->controls[15]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_sens15Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_SENS_INDEX]->controls[16]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_sens16Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_SENS_INDEX]->controls[17]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_sens17Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_SENS_INDEX]->controls[18]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_sens18Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_SENS_INDEX]->controls[19]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_sens19Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_SENS_INDEX]->controls[20]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_sens20Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_SENS_INDEX]->controls[21]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_sens21Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_SENS_INDEX]->controls[22]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_sens22Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_SENS_INDEX]->controls[23]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_sens23Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_SENS_INDEX]->controls[24]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_sens24Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_SENS_INDEX]->controls[25]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_sens25Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_SENS_INDEX]->controls[26]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_sens26Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_SENS_INDEX]->controls[27]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_sens27Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_SENS_INDEX]->controls[28]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_sens28Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_SENS_INDEX]->controls[29]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_sens29Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_SENS_INDEX]->controls[30]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_sens30Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_SENS_INDEX]->controls[31]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_sens31Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_LOW_INDEX]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_low0Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_LOW_INDEX]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_low1Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_LOW_INDEX]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_low2Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_LOW_INDEX]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_low3Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_LOW_INDEX]->controls[4]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_low4Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_LOW_INDEX]->controls[5]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_low5Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_LOW_INDEX]->controls[6]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_low6Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_LOW_INDEX]->controls[7]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_low7Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_LOW_INDEX]->controls[8]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_low8Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_LOW_INDEX]->controls[9]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_low9Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_LOW_INDEX]->controls[10]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_low10Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_LOW_INDEX]->controls[11]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_low11Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_LOW_INDEX]->controls[12]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_low12Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_LOW_INDEX]->controls[13]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_low13Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_LOW_INDEX]->controls[14]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_low14Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_LOW_INDEX]->controls[15]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_low15Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_LOW_INDEX]->controls[16]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_low16Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_LOW_INDEX]->controls[17]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_low17Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_LOW_INDEX]->controls[18]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_low18Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_LOW_INDEX]->controls[19]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_low19Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_LOW_INDEX]->controls[20]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_low20Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_LOW_INDEX]->controls[21]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_low21Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_LOW_INDEX]->controls[22]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_low22Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_LOW_INDEX]->controls[23]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_low23Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_LOW_INDEX]->controls[24]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_low24Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_LOW_INDEX]->controls[25]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_low25Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_LOW_INDEX]->controls[26]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_low26Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_LOW_INDEX]->controls[27]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_low27Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_LOW_INDEX]->controls[28]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_low28Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_LOW_INDEX]->controls[29]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_low29Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_LOW_INDEX]->controls[30]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_low30Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_LOW_INDEX]->controls[31]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_low31Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_high0Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_high1Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_high2Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_high3Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[4]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_high4Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[5]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_high5Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[6]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_high6Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[7]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_high7Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[8]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_high8Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[9]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_high9Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[10]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_high10Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[11]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_high11Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[12]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_high12Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[13]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_high13Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[14]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_high14Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[15]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_high15Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[16]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_high16Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[17]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_high17Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[18]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_high18Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[19]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_high19Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[20]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_high20Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[21]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_high21Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[22]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_high22Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[23]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_high23Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[24]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_high24Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[25]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_high25Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[26]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_high26Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[27]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_high27Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[28]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_high28Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[29]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_high29Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[30]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_high30Changed(int)));
        connect((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[31]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(sample_vel_high31Changed(int)));
        
        //From PolyFX mod matrix

/*End synth_qt_gui.cpp Autogenerated connections*/
        
        //m_view_sample_tab
        
        m_view_sample_tab = new QWidget();
        m_view_sample_tab->setObjectName(QString::fromUtf8("m_view_sample_tab"));  
        m_view_sample_tab->setStyleSheet(QString("color : white; background-color : black;"));
        m_main_tab->addTab(m_view_sample_tab, QString());                
        m_main_tab->setTabText(m_main_tab->indexOf(m_view_sample_tab), QApplication::translate("Frame", "View", 0, QApplication::UnicodeUTF8));
                
        m_view_sample_tab_main_vlayout = new QVBoxLayout(m_view_sample_tab);
        m_view_sample_tab_main_vlayout->setObjectName(QString::fromUtf8("m_view_sample_tab_main_vlayout"));
        m_view_sample_tab_main_vlayout->setContentsMargins(0, 0, 0, 0);
        m_sample_start_hlayout = new QHBoxLayout();
        m_sample_start_hlayout->setObjectName(QString::fromUtf8("m_sample_start_hlayout"));
        m_sample_start_left_hspacer = new QSpacerItem(0, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        m_sample_start_hlayout->addItem(m_sample_start_left_hspacer);

        m_sample_start_hslider = new QSlider(m_view_sample_tab);
        m_sample_start_hslider->setObjectName(QString::fromUtf8("m_sample_start_hslider"));
        m_sample_start_hslider->setMinimumSize(QSize(800, 0));
        m_sample_start_hslider->setMaximum(10000);
        m_sample_start_hslider->setOrientation(Qt::Horizontal);
        m_sample_start_hslider->setTickPosition(QSlider::NoTicks);

        m_sample_start_hlayout->addWidget(m_sample_start_hslider);

        m_sample_start_right_hspacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        m_sample_start_hlayout->addItem(m_sample_start_right_hspacer);


        m_view_sample_tab_main_vlayout->addLayout(m_sample_start_hlayout);

        m_sample_graph_hlayout = new QHBoxLayout();
        m_sample_graph_hlayout->setObjectName(QString::fromUtf8("m_sample_graph_hlayout"));
        m_sample_graph_left_hspacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        m_sample_graph_hlayout->addItem(m_sample_graph_left_hspacer);

        m_sample_graph = new LMS_sample_graph(LMS_MAX_SAMPLE_COUNT, 400, 800, m_view_sample_tab);
        m_sample_graph_hlayout->addWidget(m_sample_graph->m_sample_graph);
        
        
        m_sample_graph_right_hspacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        m_sample_graph_hlayout->addItem(m_sample_graph_right_hspacer);


        m_view_sample_tab_main_vlayout->addLayout(m_sample_graph_hlayout);

        m_sample_end_hlayout = new QHBoxLayout();
        m_sample_end_hlayout->setObjectName(QString::fromUtf8("m_sample_end_hlayout"));
        m_sample_end_left_hspacer = new QSpacerItem(0, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        m_sample_end_hlayout->addItem(m_sample_end_left_hspacer);

        m_sample_end_hslider = new QSlider(m_view_sample_tab);
        m_sample_end_hslider->setObjectName(QString::fromUtf8("m_sample_end_hslider"));
        m_sample_end_hslider->setMinimumSize(QSize(800, 0));
        m_sample_end_hslider->setLayoutDirection(Qt::RightToLeft);
        m_sample_end_hslider->setMaximum(10000);
        m_sample_end_hslider->setValue(0);
        m_sample_end_hslider->setOrientation(Qt::Horizontal);
        m_sample_end_hslider->setInvertedAppearance(false);
        m_sample_end_hslider->setInvertedControls(false);
        m_sample_end_hslider->setTickPosition(QSlider::NoTicks);
        
        connect(m_sample_start_hslider, SIGNAL(valueChanged(int)), this, SLOT(sampleStartChanged(int)));
        connect(m_sample_end_hslider, SIGNAL(valueChanged(int)), this, SLOT(sampleEndChanged(int)));

        m_sample_end_hlayout->addWidget(m_sample_end_hslider);

        m_sample_end_right_hspacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        m_sample_end_hlayout->addItem(m_sample_end_right_hspacer);


        m_view_sample_tab_main_vlayout->addLayout(m_sample_end_hlayout);

        m_sample_view_select_sample_hlayout = new QHBoxLayout();
        m_sample_view_select_sample_hlayout->setObjectName(QString::fromUtf8("m_sample_view_select_sample_hlayout"));
        m_sample_view_extra_controls_left_hspacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        m_sample_view_select_sample_hlayout->addItem(m_sample_view_extra_controls_left_hspacer);

        m_sample_view_extra_controls_gridview = new QGridLayout();
        m_sample_view_extra_controls_gridview->setObjectName(QString::fromUtf8("m_sample_view_extra_controls_gridview"));
        m_selected_sample_index_combobox = new QComboBox(m_view_sample_tab);
        m_selected_sample_index_combobox->setObjectName(QString::fromUtf8("m_selected_sample_index_combobox"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(m_selected_sample_index_combobox->sizePolicy().hasHeightForWidth());
        m_selected_sample_index_combobox->setSizePolicy(sizePolicy1);
        m_selected_sample_index_combobox->setMinimumSize(QSize(160, 0));
        m_selected_sample_index_combobox->setMaximumSize(QSize(160, 16777215));
        
        for(int f_i = 0; f_i < LMS_MAX_SAMPLE_COUNT; f_i++)
        {
            m_selected_sample_index_combobox->addItem(QString(""));
        }

        m_sample_view_extra_controls_gridview->addWidget(m_selected_sample_index_combobox, 1, 0, 1, 1);
        
        connect(m_selected_sample_index_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(viewSampleSelectedIndexChanged(int)));

        m_selected_sample_index_label = new QLabel(m_view_sample_tab);
        m_selected_sample_index_label->setObjectName(QString::fromUtf8("m_selected_sample_index_label"));

        m_sample_view_extra_controls_gridview->addWidget(m_selected_sample_index_label, 0, 0, 1, 1);


        m_sample_view_select_sample_hlayout->addLayout(m_sample_view_extra_controls_gridview);

        m_sample_view_extra_controls_right_hspacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        m_sample_view_select_sample_hlayout->addItem(m_sample_view_extra_controls_right_hspacer);


        m_view_sample_tab_main_vlayout->addLayout(m_sample_view_select_sample_hlayout);

        m_sample_view_file_select_hlayout = new QHBoxLayout();
        m_sample_view_file_select_hlayout->setObjectName(QString::fromUtf8("m_sample_view_file_select_hlayout"));
        m_sample_view_file_select_left_hspacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        m_sample_view_file_select_hlayout->addItem(m_sample_view_file_select_left_hspacer);

        m_view_file_selector = new LMS_file_select(m_view_sample_tab);
        m_view_file_selector->lms_file_path->setMinimumWidth(400);

        m_sample_view_file_select_hlayout->addLayout(m_view_file_selector->lms_layout);
        
        connect(m_view_file_selector->lms_open_button, SIGNAL(pressed()), this, SLOT(fileSelect()));
        connect(m_view_file_selector->lms_clear_button, SIGNAL(pressed()), this, SLOT(clearFile()));
        connect(m_view_file_selector->lms_open_in_editor_button, SIGNAL(pressed()), this, SLOT(openInEditor()));
        connect(m_view_file_selector->lms_reload_button, SIGNAL(pressed()), this, SLOT(reloadSample()));

        m_sample_view_file_select_right_hspacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        m_sample_view_file_select_hlayout->addItem(m_sample_view_file_select_right_hspacer);


        m_view_sample_tab_main_vlayout->addLayout(m_sample_view_file_select_hlayout);

        m_view_sample_tab_lower_vspacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        m_view_sample_tab_main_vlayout->addItem(m_view_sample_tab_lower_vspacer);
        
        m_selected_sample_index_label->setText(QApplication::translate("MainWindow", "Selected Sample", 0, QApplication::UnicodeUTF8));

        for(int f_i = 0; f_i < LMS_MAX_SAMPLE_COUNT; f_i++)
        {
            m_sample_starts[f_i] = 0;
            m_sample_ends[f_i] = 0;
        }

        
        //end m_view_sample_tab
        
        
        //From Ray-V PolyFX
        
        QStringList f_lfo_types = QStringList() << "Off" << "Sine" << "Triangle";
   
        /*This string is generated by running presets_to_qstring.pl script in the packaging folder on the .tsv file associated with this plugin
        in ~/dssi .  If modifying this plugin by changing the number of parameters to be saved by presets, you should comment this out
        and uncomment the section above it.*/
        
        //LMS_style_info * f_info = new LMS_style_info(64);
        //f_info->LMS_set_label_style("background-color: white; border: 1px solid black;  border-radius: 6px; QComboBox{color:white;background-color:black;}", 60);

        m_main_layout = new LMS_main_layout(m_poly_fx_tab);
        
        //From Modulex
        
        m_fx0 = new LMS_multieffect(this, QString("FX1"), a_style, LMS_FX0_KNOB0, LMS_FX0_KNOB1, LMS_FX0_KNOB2, LMS_FX0_COMBOBOX);
        connect(m_fx0->lms_knob1->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx0knob0Changed(int)));
        connect(m_fx0->lms_knob2->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx0knob1Changed(int)));
        connect(m_fx0->lms_knob3->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx0knob2Changed(int)));
        connect(m_fx0->lms_combobox->lms_combobox,  SIGNAL(currentIndexChanged(int)), this, SLOT(fx0comboboxChanged(int)));

        m_main_layout->lms_add_widget(m_fx0->lms_groupbox->lms_groupbox);

        m_fx1 = new LMS_multieffect(this, QString("FX2"), a_style, LMS_FX1_KNOB0, LMS_FX1_KNOB1, LMS_FX1_KNOB2, LMS_FX1_COMBOBOX);
        connect(m_fx1->lms_knob1->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx1knob0Changed(int)));
        connect(m_fx1->lms_knob2->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx1knob1Changed(int)));
        connect(m_fx1->lms_knob3->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx1knob2Changed(int)));
        connect(m_fx1->lms_combobox->lms_combobox,  SIGNAL(currentIndexChanged(int)), this, SLOT(fx1comboboxChanged(int)));

        m_main_layout->lms_add_widget(m_fx1->lms_groupbox->lms_groupbox);

        m_main_layout->lms_add_layout();    

        m_fx2 = new LMS_multieffect(this, QString("FX3"), a_style, LMS_FX2_KNOB0, LMS_FX2_KNOB1, LMS_FX2_KNOB2, LMS_FX2_COMBOBOX);
        connect(m_fx2->lms_knob1->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx2knob0Changed(int)));
        connect(m_fx2->lms_knob2->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx2knob1Changed(int)));
        connect(m_fx2->lms_knob3->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx2knob2Changed(int)));
        connect(m_fx2->lms_combobox->lms_combobox,  SIGNAL(currentIndexChanged(int)), this, SLOT(fx2comboboxChanged(int)));

        m_main_layout->lms_add_widget(m_fx2->lms_groupbox->lms_groupbox);

        m_fx3 = new LMS_multieffect(this, QString("FX4"), a_style, LMS_FX3_KNOB0, LMS_FX3_KNOB1, LMS_FX3_KNOB2, LMS_FX3_COMBOBOX);
        connect(m_fx3->lms_knob1->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx3knob0Changed(int)));
        connect(m_fx3->lms_knob2->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx3knob1Changed(int)));
        connect(m_fx3->lms_knob3->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx3knob2Changed(int)));
        connect(m_fx3->lms_combobox->lms_combobox,  SIGNAL(currentIndexChanged(int)), this, SLOT(fx3comboboxChanged(int)));

        m_main_layout->lms_add_widget(m_fx3->lms_groupbox->lms_groupbox);

        m_main_layout->lms_add_layout();  
        
        
        //New mod matrix
        
        QList <LMS_mod_matrix_column*> f_mod_matrix_columns;
        
        f_mod_matrix_columns << new LMS_mod_matrix_column(spinbox, QString("FX1\nCtrl1"), -100, 100, 0); 
        f_mod_matrix_columns << new LMS_mod_matrix_column(spinbox, QString("FX1\nCtrl2"), -100, 100, 0); 
        f_mod_matrix_columns << new LMS_mod_matrix_column(spinbox, QString("FX1\nCtrl3"), -100, 100, 0); 
        
        f_mod_matrix_columns << new LMS_mod_matrix_column(spinbox, QString("FX2\nCtrl1"), -100, 100, 0); 
        f_mod_matrix_columns << new LMS_mod_matrix_column(spinbox, QString("FX2\nCtrl2"), -100, 100, 0); 
        f_mod_matrix_columns << new LMS_mod_matrix_column(spinbox, QString("FX2\nCtrl3"), -100, 100, 0);  
        
        f_mod_matrix_columns << new LMS_mod_matrix_column(spinbox, QString("FX3\nCtrl1"), -100, 100, 0);  
        f_mod_matrix_columns << new LMS_mod_matrix_column(spinbox, QString("FX3\nCtrl2"), -100, 100, 0);  
        f_mod_matrix_columns << new LMS_mod_matrix_column(spinbox, QString("FX3\nCtrl3"), -100, 100, 0);  
        
        f_mod_matrix_columns << new LMS_mod_matrix_column(spinbox, QString("FX4\nCtrl1"), -100, 100, 0);  
        f_mod_matrix_columns << new LMS_mod_matrix_column(spinbox, QString("FX4\nCtrl2"), -100, 100, 0);  
        f_mod_matrix_columns << new LMS_mod_matrix_column(spinbox, QString("FX4\nCtrl3"), -100, 100, 0);  
        
        m_polyfx_mod_matrix[0] = new LMS_mod_matrix(this, LMS_MODULATOR_COUNT, f_mod_matrix_columns, LMS_PFXMATRIX_FIRST_PORT, a_style);
        
        m_polyfx_mod_matrix[0]->lms_mod_matrix->setVerticalHeaderLabels(QStringList() << QString("ADSR Amp") << QString("ADSR 2") << QString("Ramp Env") << QString("LFO"));
        
        m_main_layout->lms_add_widget(m_polyfx_mod_matrix[0]->lms_mod_matrix);
        
        m_polyfx_mod_matrix[0]->lms_mod_matrix->resizeColumnsToContents();

        m_main_layout->lms_add_layout();  
        
        //Connect all ports from PolyFX mod matrix
        
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[0]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst0src0ctrl0Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[1]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst0src0ctrl1Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[2]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst0src0ctrl2Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[0]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst0src1ctrl0Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[1]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst0src1ctrl1Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[2]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst0src1ctrl2Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[0]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst0src2ctrl0Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[1]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst0src2ctrl1Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[2]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst0src2ctrl2Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[0]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst0src3ctrl0Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[1]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst0src3ctrl1Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[2]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst0src3ctrl2Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[3]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst1src0ctrl0Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[4]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst1src0ctrl1Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[5]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst1src0ctrl2Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[3]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst1src1ctrl0Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[4]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst1src1ctrl1Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[5]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst1src1ctrl2Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[3]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst1src2ctrl0Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[4]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst1src2ctrl1Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[5]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst1src2ctrl2Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[3]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst1src3ctrl0Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[4]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst1src3ctrl1Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[5]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst1src3ctrl2Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[6]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst2src0ctrl0Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[7]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst2src0ctrl1Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[8]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst2src0ctrl2Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[6]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst2src1ctrl0Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[7]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst2src1ctrl1Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[8]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst2src1ctrl2Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[6]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst2src2ctrl0Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[7]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst2src2ctrl1Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[8]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst2src2ctrl2Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[6]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst2src3ctrl0Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[7]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst2src3ctrl1Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[8]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst2src3ctrl2Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[9]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst3src0ctrl0Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[10]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst3src0ctrl1Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[11]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst3src0ctrl2Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[9]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst3src1ctrl0Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[10]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst3src1ctrl1Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[11]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst3src1ctrl2Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[9]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst3src2ctrl0Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[10]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst3src2ctrl1Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[11]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst3src2ctrl2Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[9]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst3src3ctrl0Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[10]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst3src3ctrl1Changed(int)));
        connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[11]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst3src3ctrl2Changed(int)));
        
        //End new mod matrix
        
        
        
        //End from Modulex
        
        m_adsr_amp = new LMS_adsr_widget(this, a_style, TRUE, LMS_ATTACK, LMS_DECAY, LMS_SUSTAIN, LMS_RELEASE, QString("ADSR Amp"));
        
        m_adsr_amp->lms_release->lms_knob->setMinimum(5);  //overriding the default for this, because we want a low minimum default that won't click

        m_main_layout->lms_add_widget(m_adsr_amp->lms_groupbox_adsr->lms_groupbox);

        connect(m_adsr_amp->lms_attack->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(attackChanged(int)));    
        connect(m_adsr_amp->lms_decay->lms_knob,   SIGNAL(valueChanged(int)), this, SLOT(decayChanged(int)));        
        connect(m_adsr_amp->lms_sustain->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(sustainChanged(int)));        
        connect(m_adsr_amp->lms_release->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(releaseChanged(int)));    

        m_groupbox_noise = new LMS_group_box(this, QString("Noise"), a_style);
        m_main_layout->lms_add_widget(m_groupbox_noise->lms_groupbox);

        m_noise_amp = new LMS_knob_regular(QString("Vol"), -60, 0, 1, 30, QString(""), m_groupbox_noise->lms_groupbox, a_style, lms_kc_integer, LMS_NOISE_AMP);
        m_groupbox_noise->lms_add_h(m_noise_amp);
        connect(m_noise_amp->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(noiseAmpChanged(int)));
        
        m_noise_type = new LMS_combobox(QString("Type"), this, QStringList() << QString("Off") << QString("White") << QString("Pink"), LMS_NOISE_TYPE, a_style);
        m_groupbox_noise->lms_add_h(m_noise_type);
        connect(m_noise_type->lms_combobox,  SIGNAL(currentIndexChanged(int)), this, SLOT(noise_typeChanged(int)));
        
        m_adsr_filter = new LMS_adsr_widget(this, a_style, FALSE, LMS_FILTER_ATTACK, LMS_FILTER_DECAY, LMS_FILTER_SUSTAIN, LMS_FILTER_RELEASE, QString("ADSR 2"));

        m_main_layout->lms_add_widget(m_adsr_filter->lms_groupbox_adsr->lms_groupbox);

        connect(m_adsr_filter->lms_attack->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(filterAttackChanged(int)));
        connect(m_adsr_filter->lms_decay->lms_knob,   SIGNAL(valueChanged(int)), this, SLOT(filterDecayChanged(int))); 
        connect(m_adsr_filter->lms_sustain->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(filterSustainChanged(int)));
        connect(m_adsr_filter->lms_release->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(filterReleaseChanged(int)));
        
        m_main_layout->lms_add_layout();

        m_master = new LMS_master_widget(this, a_style, LMS_MASTER_VOLUME, -1, 
                -1, LMS_MASTER_GLIDE, LMS_MASTER_PITCHBEND_AMT, QString("Master"), FALSE);
        m_main_layout->lms_add_widget(m_master->lms_groupbox->lms_groupbox);    
        
        m_master->lms_master_volume->lms_knob->setMinimum(-24);
        m_master->lms_master_volume->lms_knob->setMaximum(24);
                
        connect(m_master->lms_master_volume->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(masterVolumeChanged(int)));
        connect(m_master->lms_master_glide->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(masterGlideChanged(int)));    
        connect(m_master->lms_master_pitchbend_amt->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(masterPitchbendAmtChanged(int)));

        m_pitch_env = new LMS_ramp_env(this, a_style, LMS_PITCH_ENV_TIME, -1, -1, FALSE, QString("Ramp Env"), FALSE);
        m_main_layout->lms_add_widget(m_pitch_env->lms_groupbox->lms_groupbox);

        connect(m_pitch_env->lms_time_knob->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(pitchEnvTimeChanged(int)));

        m_lfo = new LMS_lfo_widget(this, a_style, LMS_LFO_FREQ, LMS_LFO_TYPE, f_lfo_types, QString("LFO"));
        m_main_layout->lms_add_widget(m_lfo->lms_groupbox->lms_groupbox);

        connect(m_lfo->lms_freq_knob->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(LFOfreqChanged(int)));
        connect(m_lfo->lms_type_combobox->lms_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(LFOtypeChanged(int)));
        
        //End Ray-V
        
    QTimer *myTimer = new QTimer(this);
    connect(myTimer, SIGNAL(timeout()), this, SLOT(oscRecv()));
    myTimer->setSingleShot(false);
    myTimer->start(0);

    m_suppressHostUpdate = false;
}

void SamplerGUI::clearAllSamples()
{
    for(int i = 0; i < LMS_MAX_SAMPLE_COUNT; i++)
    {
        
        set_selected_sample_combobox_item(i, QString(""));

        m_sample_graph->clearPixmap(i);
        
        QTableWidgetItem * f_item = new QTableWidgetItem();
        f_item->setText(QString(""));
        f_item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        m_sample_table->lms_mod_matrix->setItem(i, SMP_TB_FILE_PATH_INDEX, f_item);
    }
    
    generate_files_string();
    
#ifndef LMS_DEBUG_STANDALONE
    lo_send(m_host, m_configurePath, "ss", "load", files_string.toLocal8Bit().data());
#endif
}

void SamplerGUI::mapAllSamplesToOneWhiteKey()
{
    int f_i = 0;
    int f_current_note = 36;
    bool f_has_started = FALSE;
    
    int i_white_notes = 0;
    
    int f_white_notes[7] = {2,2,1,2,2,2,1};
    
    for(f_i = 0; f_i < LMS_MAX_SAMPLE_COUNT; f_i++)
    {
        QString f_current_string = (m_sample_table->lms_mod_matrix->item(f_i, SMP_TB_FILE_PATH_INDEX)->text());
        
        if(!f_current_string.isEmpty())
        {
            f_has_started = TRUE;
        }
        
        if(f_has_started && (!f_current_string.isEmpty()))
        {
            ((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[f_i]))->lms_set_value(f_current_note);
            ((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[f_i]))->lms_set_value(f_current_note);
            ((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[f_i]))->lms_set_value(f_current_note);
            
            f_current_note += f_white_notes[i_white_notes];
            
            i_white_notes++;
            if(i_white_notes >= 7)
            {
                i_white_notes = 0;
            }
            
        }
    }
}

void SamplerGUI::viewSampleSelectedIndexChanged(int a_index)
{
    if(m_suppress_selected_sample_changed)
    {
        return;
    }
        
    QRadioButton * f_radio_button = (QRadioButton*)m_sample_table->lms_mod_matrix->cellWidget(a_index , SMP_TB_RADIOBUTTON_INDEX);    
    f_radio_button->click();    
}

void SamplerGUI::sampleStartChanged(int a_value)
{
        m_sample_table->find_selected_radio_button(SMP_TB_RADIOBUTTON_INDEX);    
        
        m_sample_starts[(m_sample_table->lms_selected_column)] = a_value;
#ifndef LMS_DEBUG_STANDALONE
    if (!m_suppressHostUpdate) {        
	lo_send(m_host, m_controlPath, "if", (LMS_SAMPLE_START_PORT_RANGE_MIN + (m_sample_table->lms_selected_column)), (float)(a_value));
    }
#endif    
}

void SamplerGUI::sampleEndChanged(int a_value)
{
    m_sample_table->find_selected_radio_button(SMP_TB_RADIOBUTTON_INDEX);    
    
    m_sample_ends[(m_sample_table->lms_selected_column)] = a_value;
#ifndef LMS_DEBUG_STANDALONE
    if (!m_suppressHostUpdate) {        
	lo_send(m_host, m_controlPath, "if", (LMS_SAMPLE_END_PORT_RANGE_MIN + (m_sample_table->lms_selected_column)), (float)(a_value));
    }
#endif    
}

void SamplerGUI::setSampleFile(QString files)
{
    m_suppressHostUpdate = true;

    cerr << "Calling SamplerGUI::setSampleFile with string:\n" << files << "\n";
    
    m_files = files;    
    
    files.replace(QString(LMS_FILES_STRING_RELOAD_DELIMITER), QString(LMS_FILES_STRING_DELIMITER));
    
    QStringList f_file_list = files.split(QString(LMS_FILES_STRING_DELIMITER));
    
    for(int f_i = 0; f_i < f_file_list.count(); f_i++)
    {
        if(!f_file_list[f_i].isEmpty())
        {
            cerr << "Setting " << f_file_list[f_i] << " at index " << f_i << "\n";
        }
        
        QTableWidgetItem * f_item = new QTableWidgetItem();
        f_item->setText(f_file_list[f_i]);
        f_item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        m_sample_table->lms_mod_matrix->setItem(f_i, SMP_TB_FILE_PATH_INDEX, f_item);       
        
        if(f_file_list[f_i].isEmpty())
        {
            continue;
        }
        
        QStringList f_path_sections = f_file_list[f_i].split(QString("/"));        
        
        set_selected_sample_combobox_item(f_i, f_path_sections.at((f_path_sections.count() - 1)));
        
        m_sample_graph->generatePreview(f_file_list[f_i], f_i);
    }

    m_sample_table->lms_mod_matrix->resizeColumnsToContents();
    
    selectionChanged();
    
    m_suppressHostUpdate = false;
}

void SamplerGUI::setSelection(int a_value)
{
    m_suppressHostUpdate = true;
    //m_selected_sample[a_value]->setChecked(true);
    m_suppressHostUpdate = false;
}

/* void SamplerGUI::set_selected_sample_combobox_item(
 * int a_index, //Currently, you should only set this to (m_sample_table->lms_selected_column), but I'm leaving it there for when it can be implemented to work otherwise
 * QString a_text)  //The text of the new item
 */
void SamplerGUI::set_selected_sample_combobox_item(int a_index, QString a_text)
{
    m_suppress_selected_sample_changed = TRUE;
    m_selected_sample_index_combobox->removeItem(a_index);
    m_selected_sample_index_combobox->insertItem(a_index, a_text);
    m_selected_sample_index_combobox->setCurrentIndex(a_index);
    m_suppress_selected_sample_changed = FALSE;
}

void SamplerGUI::fileSelect()
{   
    QStringList paths = m_file_selector->open_button_pressed_multiple(this);
    
    m_view_file_selector->lms_set_file(m_file_selector->lms_get_file());    
    
    load_files(paths);
}

void SamplerGUI::load_files(QStringList paths)
{
    if(!paths.isEmpty())
    {
        m_sample_table->find_selected_radio_button(SMP_TB_RADIOBUTTON_INDEX);
            
        int f_sample_index_to_load = (m_sample_table->lms_selected_column);
            
        for(int i = 0; i < paths.count(); i++)
        {
            QString path = paths[i];

            if(!path.isEmpty())
            {
                if(!QFile::exists(path))
                {
                    QMessageBox::warning(this, QString("Error"), QString("File cannot be read."));
                    continue;
                }
                
                QStringList f_path_sections = path.split(QString("/"));

                set_selected_sample_combobox_item(f_sample_index_to_load, f_path_sections.at((f_path_sections.count() - 1)));

                m_sample_graph->generatePreview(path, f_sample_index_to_load);

                QTableWidgetItem * f_item = new QTableWidgetItem();
                f_item->setText(path);
                f_item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
                m_sample_table->lms_mod_matrix->setItem(f_sample_index_to_load, SMP_TB_FILE_PATH_INDEX, f_item);
                                
                f_sample_index_to_load++;
                
                if(f_sample_index_to_load >= LMS_MAX_SAMPLE_COUNT)
                {
                    cerr << "Multiple sample loading index exceeded LMS_MAX_SAMPLE count, not all samples were loaded\n";
                    break;
                }
            }
        }
        generate_files_string();
                
#ifndef LMS_DEBUG_STANDALONE
        lo_send(m_host, m_configurePath, "ss", "load", files_string.toLocal8Bit().data());
        lo_send(m_host, m_configurePath, "ss", "lastdir", m_file_selector->lms_last_directory.toLocal8Bit().data());
#endif
                
        m_sample_table->lms_mod_matrix->resizeColumnsToContents();
        selectionChanged();
    }
}

void SamplerGUI::generate_files_string()
{
    generate_files_string(-1);
}

void SamplerGUI::generate_files_string(int a_index)
{
    files_string = QString("");
        
    for(int f_i = 0; f_i < LMS_MAX_SAMPLE_COUNT; f_i++)
    {
        files_string.append(m_sample_table->lms_mod_matrix->item(f_i, SMP_TB_FILE_PATH_INDEX)->text());
        
        if((a_index != -1) && (f_i == a_index))
        {
            files_string.append(LMS_FILES_STRING_RELOAD_DELIMITER);
        }
        else
        {
            files_string.append(LMS_FILES_STRING_DELIMITER);
        }
    }
    
    files_string.append(preview_file + LMS_FILES_STRING_DELIMITER);
    
    //cerr << files_string << "\n";
}

void SamplerGUI::clearFile()
{ 
    m_sample_table->find_selected_radio_button(SMP_TB_RADIOBUTTON_INDEX);
    
    m_sample_graph->clearPixmap((m_sample_table->lms_selected_column));
    
    m_sample_start_hslider->setValue(0);
    m_sample_end_hslider->setValue(0);
    
    set_selected_sample_combobox_item((m_sample_table->lms_selected_column), QString(""));

    QTableWidgetItem * f_item = new QTableWidgetItem();
    f_item->setText(QString(""));
    f_item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    m_sample_table->lms_mod_matrix->setItem((m_sample_table->lms_selected_column), SMP_TB_FILE_PATH_INDEX, f_item);
    m_file_selector->clear_button_pressed();
    m_view_file_selector->clear_button_pressed();
    
    generate_files_string();
    
#ifndef LMS_DEBUG_STANDALONE
        lo_send(m_host, m_configurePath, "ss", "load", files_string.toLocal8Bit().data());
#endif
    
    //cerr << files_string;
}

void SamplerGUI::openInEditor()
{
    m_file_selector->open_in_editor_button_pressed(this);
}

void SamplerGUI::reloadSample()
{
    QString path = m_file_selector->lms_file_path->text();
    
    if(!path.isEmpty())
    {
        m_sample_table->find_selected_radio_button(SMP_TB_RADIOBUTTON_INDEX);
        
        generate_files_string((m_sample_table->lms_selected_column));
    
#ifndef LMS_DEBUG_STANDALONE
        lo_send(m_host, m_configurePath, "ss", "load", files_string.toLocal8Bit().data());
#endif
        
        m_sample_graph->generatePreview(path, (m_sample_table->lms_selected_column));
    }
}

void SamplerGUI::selectionChanged()
{
    if(m_suppress_selected_sample_changed)
    {
        return;
    }
    
    m_suppress_selected_sample_changed = TRUE;
    
    m_sample_table->find_selected_radio_button(SMP_TB_RADIOBUTTON_INDEX);
    m_selected_sample_index_combobox->setCurrentIndex((m_sample_table->lms_selected_column));    
    m_suppress_selected_sample_changed = FALSE;
        
    m_selected_sample_index_combobox->setCurrentIndex((m_sample_table->lms_selected_column));
    m_sample_graph->indexChanged((m_sample_table->lms_selected_column));
    
#ifndef LMS_DEBUG_STANDALONE
    if (!m_suppressHostUpdate) {        
	lo_send(m_host, m_controlPath, "if", Sampler_SELECTED_SAMPLE, (float)(m_sample_table->lms_selected_column));
    }
#endif    
    
    m_file_selector->lms_set_file(m_sample_table->lms_mod_matrix->item(m_sample_table->lms_selected_column, SMP_TB_FILE_PATH_INDEX)->text());
    m_view_file_selector->lms_set_file(m_sample_table->lms_mod_matrix->item(m_sample_table->lms_selected_column, SMP_TB_FILE_PATH_INDEX)->text());
    
    m_suppressHostUpdate = TRUE;
    
    m_sample_start_hslider->setValue(m_sample_starts[(m_sample_table->lms_selected_column)]);
    m_sample_end_hslider->setValue(m_sample_ends[(m_sample_table->lms_selected_column)]);
    
    m_suppressHostUpdate = FALSE;
    
}

void SamplerGUI::file_browser_load_button_pressed()
{
    QStringList f_result = m_file_browser->files_opened();
    
    for(int f_i = 0; f_i < f_result.count(); f_i++)
    {
        QString f_temp = QString(m_file_browser->m_folder_path_lineedit->text() + "/" + f_result.at(f_i));
        
        f_result.removeAt(f_i);
        f_result.insert(f_i, f_temp);
    }
    
    load_files(f_result);
}

void SamplerGUI::file_browser_up_button_pressed()
{
    m_file_browser->up_one_folder();
}

void SamplerGUI::file_browser_preview_button_pressed()
{
    QList<QListWidgetItem*> f_list = m_file_browser->m_files_listWidget->selectedItems();
    
    if(f_list.count() > 0)    
    {
        preview_file = m_file_browser->m_folder_path_lineedit->text() + QString("/") + f_list[0]->text();
        
        generate_files_string();
                
#ifndef LMS_DEBUG_STANDALONE
        lo_send(m_host, m_configurePath, "ss", "load", files_string.toLocal8Bit().data());
        lo_send(m_host, m_configurePath, "ss", "lastdir", m_file_browser->m_folder_path_lineedit->text().toLocal8Bit().data());
#endif
        
        preview_file = QString("");        
    }
}

void SamplerGUI::file_browser_folder_clicked(QListWidgetItem * a_item)
{
    m_file_browser->folder_opened(a_item->text(), TRUE);
}

void SamplerGUI::file_browser_bookmark_clicked(QListWidgetItem * a_item)
{
    m_file_browser->bookmark_clicked(a_item->text());
}

void SamplerGUI::file_browser_bookmark_button_pressed()
{
    m_file_browser->bookmark_button_pressed();
}

void SamplerGUI::file_browser_bookmark_delete_button_pressed()
{
    m_file_browser->bookmark_delete_button_pressed();
}

void SamplerGUI::sample_pitchChanged(int a_control_index)
{
    m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[a_control_index]->lms_value_changed(0);
#ifndef LMS_DEBUG_STANDALONE
    if (!m_suppressHostUpdate) {        
	lo_send(m_host, m_controlPath, "if",                 
                (m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[a_control_index]->lms_port), 
                (float)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[a_control_index]->lms_get_value()));
    }
#endif    
}

void SamplerGUI::sample_lnoteChanged(int a_control_index)
{
    m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[a_control_index]->lms_value_changed(0);
#ifndef LMS_DEBUG_STANDALONE
    if (!m_suppressHostUpdate) {        
	lo_send(m_host, m_controlPath, "if",                 
                (m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[a_control_index]->lms_port), 
                (float)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[a_control_index]->lms_get_value()));
    }
#endif    
    
}
void SamplerGUI::sample_hnoteChanged(int a_control_index)
{
    m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[a_control_index]->lms_value_changed(0);
#ifndef LMS_DEBUG_STANDALONE
    if (!m_suppressHostUpdate) {        
	lo_send(m_host, m_controlPath, "if",                 
                (m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[a_control_index]->lms_port), 
                (float)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[a_control_index]->lms_get_value()));
    }
#endif    
    
}

void SamplerGUI::sample_volChanged(int a_control_index)
{
    m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[a_control_index]->lms_value_changed(0);
#ifndef LMS_DEBUG_STANDALONE
    if (!m_suppressHostUpdate) {        
	lo_send(m_host, m_controlPath, "if",                 
                (m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[a_control_index]->lms_port), 
                (float)(m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[a_control_index]->lms_get_value()));
    }
#endif
}

void SamplerGUI::sample_vel_sensChanged(int a_control_index)
{
    m_sample_table->lms_mm_columns[SMP_TB_VEL_SENS_INDEX]->controls[a_control_index]->lms_value_changed(0);
#ifndef LMS_DEBUG_STANDALONE
    if (!m_suppressHostUpdate) {        
	lo_send(m_host, m_controlPath, "if",                 
                (LMS_SAMPLE_VEL_SENS_PORT_RANGE_MIN + a_control_index), 
                (float)(m_sample_table->lms_mm_columns[SMP_TB_VEL_SENS_INDEX]->controls[a_control_index]->lms_get_value()));
    }
#endif
}

void SamplerGUI::sample_vel_lowChanged(int a_control_index)
{    
    m_sample_table->lms_mm_columns[SMP_TB_VEL_LOW_INDEX]->controls[a_control_index]->lms_value_changed(0);
#ifndef LMS_DEBUG_STANDALONE
    if (!m_suppressHostUpdate) {        
	lo_send(m_host, m_controlPath, "if",                 
                (LMS_SAMPLE_VEL_LOW_PORT_RANGE_MIN + a_control_index), 
                (float)(m_sample_table->lms_mm_columns[SMP_TB_VEL_LOW_INDEX]->controls[a_control_index]->lms_get_value()));
    }
#endif
}

void SamplerGUI::sample_vel_highChanged(int a_control_index)
{    
    m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[a_control_index]->lms_value_changed(0);
#ifndef LMS_DEBUG_STANDALONE
    if (!m_suppressHostUpdate) {        
	lo_send(m_host, m_controlPath, "if",
                (LMS_SAMPLE_VEL_HIGH_PORT_RANGE_MIN + a_control_index),
                (float)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[a_control_index]->lms_get_value()));
    }
#else
    int test_value = (m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[a_control_index]->lms_get_value());
    int test_port = (m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[a_control_index]->lms_port);   
    int ought_to_be_right_port = LMS_SAMPLE_VEL_HIGH_PORT_RANGE_MIN + a_control_index;
    cerr << LMS_SAMPLE_VEL_HIGH_PORT_RANGE_MIN << " " << LMS_SAMPLE_VEL_HIGH_PORT_RANGE_MAX << " " << SMP_TB_VEL_HIGH_INDEX << "\n";
#endif
}

void SamplerGUI::pfxmatrix_Changed(int a_port, int a_fx_group, int a_dst, int a_ctrl, int a_src)
{
    //m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[a_control_index]->lms_value_changed(0);    
#ifndef LMS_DEBUG_STANDALONE
    if (!m_suppressHostUpdate) {        
	lo_send(m_host, m_controlPath, "if",
                a_port,
                //(float)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[a_control_index]->lms_get_value())
                (float)(m_polyfx_mod_matrix[a_fx_group]->lms_mm_columns[((a_dst * LMS_CONTROLS_PER_MOD_EFFECT) + a_ctrl)]->controls[a_src]->lms_get_value())
                );
    }
#endif
}

void SamplerGUI::saveInstrumentToSingleFile()
{       
    QString f_selected_path = QFileDialog::getSaveFileName(this, "Select an file to save the instrument to...", ".", "Euphoria Instrument Files (*.u4ia)");
        
    if(!f_selected_path.isEmpty())
    {        
        if(!f_selected_path.endsWith(QString(".u4ia")))
            f_selected_path.append(QString(".u4ia"));
        
        QFileInfo f_qfileinfo(f_selected_path);
        
        bool f_paths_are_relative = TRUE;

        for(int i = 0; i < LMS_MAX_SAMPLE_COUNT; i++)        
        {
            if(m_sample_table->lms_mod_matrix->item(i, SMP_TB_FILE_PATH_INDEX)->text().isEmpty() || m_sample_table->lms_mod_matrix->item(i, SMP_TB_FILE_PATH_INDEX)->text().isNull())
                continue;
            
            if(!m_sample_table->lms_mod_matrix->item(i, SMP_TB_FILE_PATH_INDEX)->text().startsWith(f_qfileinfo.absolutePath()))
            {
                f_paths_are_relative = FALSE;
                cerr << m_sample_table->lms_mod_matrix->item(i, SMP_TB_FILE_PATH_INDEX)->text() << " does not begin with " << f_qfileinfo.absolutePath() << "\n";
                break;
            }
        }
      
        m_creating_instrument_file = TRUE;
        //moveSamplesToSingleDirectory();
        m_creating_instrument_file = FALSE;
        
        QFile file( f_selected_path );
        if ( file.open(QIODevice::ReadWrite) )
        {
            QTextStream stream( &file );
            
            if(f_paths_are_relative)
                stream << LMS_FILES_ATTRIBUTE_RELATIVE_PATH << "\n";
            else
                stream << LMS_FILES_ATTRIBUTE_ABSOLUTE_PATH << "\n";
            
            stream << LMS_FILE_FILES_TAG << "\n";
            
            for(int i = 0; i < LMS_MAX_SAMPLE_COUNT; i++)        
            {        
                if(f_paths_are_relative)
                {
                    QDir f_dir(f_qfileinfo.absolutePath());
                                        
                    stream << i << LMS_FILES_STRING_DELIMITER << 
                            f_dir.relativeFilePath(m_sample_table->lms_mod_matrix->item(i, SMP_TB_FILE_PATH_INDEX)->text()) << LMS_FILES_STRING_DELIMITER <<
                             i_get_control((i + LMS_SAMPLE_PITCH_PORT_RANGE_MIN)) << LMS_FILES_STRING_DELIMITER <<
                             i_get_control((i + LMS_PLAY_PITCH_LOW_PORT_RANGE_MIN)) << LMS_FILES_STRING_DELIMITER <<
                             i_get_control((i + LMS_PLAY_PITCH_HIGH_PORT_RANGE_MIN)) << LMS_FILES_STRING_DELIMITER << 
                             i_get_control((i + LMS_SAMPLE_VOLUME_PORT_RANGE_MIN)) << LMS_FILES_STRING_DELIMITER <<
                             i_get_control((i + LMS_SAMPLE_START_PORT_RANGE_MIN)) << LMS_FILES_STRING_DELIMITER <<
                             i_get_control((i + LMS_SAMPLE_END_PORT_RANGE_MIN)) << LMS_FILES_STRING_DELIMITER << 
                             i_get_control((i + LMS_SAMPLE_VEL_SENS_PORT_RANGE_MIN)) << LMS_FILES_STRING_DELIMITER <<
                             i_get_control((i + LMS_SAMPLE_VEL_LOW_PORT_RANGE_MIN)) << LMS_FILES_STRING_DELIMITER << 
                             i_get_control((i + LMS_SAMPLE_VEL_HIGH_PORT_RANGE_MIN)) << "\n";
                }
                else
                {
                    stream << i << LMS_FILES_STRING_DELIMITER << m_sample_table->lms_mod_matrix->item(i, SMP_TB_FILE_PATH_INDEX)->text() << LMS_FILES_STRING_DELIMITER <<
                             i_get_control((i + LMS_SAMPLE_PITCH_PORT_RANGE_MIN)) << LMS_FILES_STRING_DELIMITER <<
                             i_get_control((i + LMS_PLAY_PITCH_LOW_PORT_RANGE_MIN)) << LMS_FILES_STRING_DELIMITER <<
                             i_get_control((i + LMS_PLAY_PITCH_HIGH_PORT_RANGE_MIN)) << LMS_FILES_STRING_DELIMITER << 
                             i_get_control((i + LMS_SAMPLE_VOLUME_PORT_RANGE_MIN)) << LMS_FILES_STRING_DELIMITER <<
                             i_get_control((i + LMS_SAMPLE_START_PORT_RANGE_MIN)) << LMS_FILES_STRING_DELIMITER <<
                             i_get_control((i + LMS_SAMPLE_END_PORT_RANGE_MIN)) << LMS_FILES_STRING_DELIMITER << 
                             i_get_control((i + LMS_SAMPLE_VEL_SENS_PORT_RANGE_MIN)) << LMS_FILES_STRING_DELIMITER <<
                             i_get_control((i + LMS_SAMPLE_VEL_LOW_PORT_RANGE_MIN)) << LMS_FILES_STRING_DELIMITER << 
                             i_get_control((i + LMS_SAMPLE_VEL_HIGH_PORT_RANGE_MIN)) << "\n";     
                }
            }
            
            stream << LMS_FILE_CONTROLS_TAG << "\n";
            stream << LMS_FILE_CONTROLS_TAG_EUP_V1 << "\n";
            
            for(int i = LMS_FIRST_CONTROL_PORT; i < LMS_LAST_REGULAR_CONTROL_PORT; i++)        
            {   
                stream << i << LMS_FILE_PORT_VALUE_SEPARATOR << i_get_control(i) << "\n";                
            }   
            
            file.close();
        }
        else
        {
            cerr << "Error opening file for Read/Write, you may not have permissions to that directory\n";
        }
    }
}

void SamplerGUI::moveSamplesToSingleDirectory()
{
    QString f_selected_path = QString("");
    
    if(m_creating_instrument_file)
    {        
        f_selected_path = m_inst_file_tmp_path;
    }
    else
    {
        f_selected_path = QFileDialog::getExistingDirectory(this, "Select a directory to move the samples to...", ".");
    }
    
    if(!f_selected_path.isEmpty())
    {
        //TODO:  check that the directory is empty...
        
        m_sample_table->find_selected_radio_button(SMP_TB_RADIOBUTTON_INDEX);
        int f_current_radio_button = m_sample_table->lms_selected_column;

        for(int i = 0; i < LMS_MAX_SAMPLE_COUNT; i++)        
        {           
            QString f_current_file_path = m_sample_table->lms_mod_matrix->item(i, SMP_TB_FILE_PATH_INDEX)->text();
            
            if(f_current_file_path.isNull())
                continue;
            
            if((f_current_file_path.isEmpty()))
                continue;
            
            if(f_current_file_path.startsWith(f_selected_path, Qt::CaseInsensitive))
                continue;
            
            QFile * f_current_file = new QFile(f_current_file_path);
            QStringList f_file_arr = f_current_file->fileName().split("/");
            
            QString f_new_file = f_selected_path + QString("/")  + f_file_arr[(f_file_arr.count() - 1)];
                
#ifdef LMS_DEBUG_STANDALONE
            std::string f_string = f_new_file.toStdString();
#endif
                
            f_current_file->copy(f_new_file);
                
            QTableWidgetItem * f_item = new QTableWidgetItem();
            f_item->setText(f_new_file);
            f_item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
            m_sample_table->lms_mod_matrix->setItem(i, SMP_TB_FILE_PATH_INDEX, f_item);
            
            QRadioButton * f_radio_button = (QRadioButton*)m_sample_table->lms_mod_matrix->cellWidget(i , SMP_TB_RADIOBUTTON_INDEX);                
            f_radio_button->setChecked(TRUE);
                
            generate_files_string();
#ifndef LMS_DEBUG_STANDALONE
            lo_send(m_host, m_configurePath, "ss", "load", files_string.toLocal8Bit().data());
#endif                      
        }
        
        /*Select the radio button that was originally selected*/
        QRadioButton * f_radio_button = (QRadioButton*)m_sample_table->lms_mod_matrix->cellWidget(f_current_radio_button , SMP_TB_RADIOBUTTON_INDEX);                
        f_radio_button->setChecked(TRUE);
    }
}

void SamplerGUI::openInstrumentFromFile()
{
    QString f_selected_path = QFileDialog::getOpenFileName(this, "Select an instrument file to open...", ".", "Euphoria Instrument Files (*.u4ia)");  
    
    if(!f_selected_path.isEmpty())
    {
        QString f_clear_all_files = QString("");
        
        for(int i = 0; i < LMS_TOTAL_SAMPLE_COUNT; i++)
        {
            f_clear_all_files.append(LMS_FILES_STRING_DELIMITER);
        }
        
#ifndef LMS_DEBUG_STANDALONE
            lo_send(m_host, m_configurePath, "ss", "load", f_clear_all_files.toLocal8Bit().data());
#endif              
        
        int f_current_stage = 0; //0 == none, 1 == files, 2 == controls
        int f_line_number = 0;
    
        QFile file(f_selected_path);
        if(!file.open(QIODevice::ReadOnly)) {
            QMessageBox::information(0, "error", file.errorString());
            return;
        }

        QTextStream in(&file);
                
        QString f_path_prefix = QString("");
        bool f_use_path_prefix = FALSE;
        bool f_is_current_app_and_control_version = FALSE;

        while(!in.atEnd()) {
            QString line = in.readLine();    
            
            if(line.compare(QString(LMS_FILES_ATTRIBUTE_RELATIVE_PATH)) == 0)
            {
                f_path_prefix = QFileInfo(f_selected_path).absolutePath();
                f_use_path_prefix = TRUE;
                continue;
            }
        
            if(line.compare(QString(LMS_FILE_FILES_TAG)) == 0)
            {
                f_current_stage = 1; continue;
            }
            else if(line.compare(QString(LMS_FILE_CONTROLS_TAG)) == 0)
            {
                f_current_stage = 2; 
                f_is_current_app_and_control_version = FALSE;  //must be reset each time this tag is found, otherwise you might start reading another sampler's controls
                continue;
            }
            else if(line.compare(QString(LMS_FILE_CONTROLS_TAG_EUP_V1)) == 0)
            {
                f_is_current_app_and_control_version = TRUE; continue;
            }
            
            switch(f_current_stage)
            {
                case 0:{
                    cerr << "f_current_stage == 0 on line# " << f_line_number << ".  " << line << "\n";
                }break;
                case 1:{
                    QStringList file_arr = line.split(LMS_FILES_STRING_DELIMITER);
                    
                    int f_sample_index = file_arr.at(0).toInt();
                    
                    if(file_arr.count() < 2)
                    {
                        cerr << "Malformed file definition at line# " << f_line_number << ".  " << line << "\n";                        
                    }                    
                    else
                    {
                        if(file_arr.at(1).isEmpty())
                        {
                            QTableWidgetItem * f_item = new QTableWidgetItem();
                            f_item->setText(QString(""));
                            f_item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
                            m_sample_table->lms_mod_matrix->setItem(f_sample_index, SMP_TB_FILE_PATH_INDEX, f_item);   
                        }
                        else
                        {
                            QString f_full_path = file_arr.at(1);
                            
                            if(f_use_path_prefix)
                                    f_full_path = QString(f_path_prefix + "/" + file_arr.at(1));
                            
                            if(QFile::exists(f_full_path))
                            {
                                cerr << "Setting " << file_arr.at(0) << " to " << f_full_path << "\n";
                                QTableWidgetItem * f_item = new QTableWidgetItem();
                                f_item->setText(f_full_path);
                                f_item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
                                int f_sample_index = file_arr.at(0).toInt();
                                m_sample_table->lms_mod_matrix->setItem(f_sample_index, SMP_TB_FILE_PATH_INDEX, f_item);   
                                m_sample_graph->generatePreview(f_full_path, f_sample_index);
                                
                                QStringList file_sections = f_full_path.split("/");
                                set_selected_sample_combobox_item(f_sample_index, file_sections.at((file_sections.count() - 1)));
                            }
                            else
                            {
                                cerr << "Invalid file " << line << "  full path: " << f_full_path << "\n";
                            }
                        }
                    }
                    
                    for(int i = 2; i < file_arr.count(); i++)
                    {
                        switch(i)
                        {
                            case 2:
                                v_set_control((f_sample_index + LMS_SAMPLE_PITCH_PORT_RANGE_MIN), file_arr.at(i).toFloat());
                                break;
                            case 3:
                                v_set_control((f_sample_index + LMS_PLAY_PITCH_LOW_PORT_RANGE_MIN), file_arr.at(i).toFloat());
                                break;
                            case 4:
                                v_set_control((f_sample_index + LMS_PLAY_PITCH_HIGH_PORT_RANGE_MIN), file_arr.at(i).toFloat());
                                break;
                            case 5:
                                v_set_control((f_sample_index + LMS_SAMPLE_VOLUME_PORT_RANGE_MIN), file_arr.at(i).toFloat());
                                break;
                            case 6:
                                v_set_control((f_sample_index + LMS_SAMPLE_START_PORT_RANGE_MIN), file_arr.at(i).toFloat());
                                break;
                            case 7:
                                v_set_control((f_sample_index + LMS_SAMPLE_END_PORT_RANGE_MIN), file_arr.at(i).toFloat());
                                break;
                            case 8:
                                v_set_control((f_sample_index + LMS_SAMPLE_VEL_SENS_PORT_RANGE_MIN), file_arr.at(i).toFloat());
                                break;
                            case 9:
                                v_set_control((f_sample_index + LMS_SAMPLE_VEL_LOW_PORT_RANGE_MIN), file_arr.at(i).toFloat());
                                break;
                            case 10:
                                v_set_control((f_sample_index + LMS_SAMPLE_VEL_HIGH_PORT_RANGE_MIN), file_arr.at(i).toFloat());
                                break;
                        }
                    }
                }
                    break;
                case 2:{
                    if(!f_is_current_app_and_control_version)
                    {
                        continue;
                    }
                    
                    QStringList f_control_port_value_pair = line.split(LMS_FILE_PORT_VALUE_SEPARATOR);
                    
                    if(f_control_port_value_pair.count() != 2)
                    {
                        cerr << "Malformed control/port definition at line# " << f_line_number << ".  " << line << "\n";
                        break;
                    }
                    
                    v_set_control(f_control_port_value_pair.at(0).toInt(), f_control_port_value_pair.at(1).toFloat());
                    v_control_changed(f_control_port_value_pair.at(0).toInt(), f_control_port_value_pair.at(1).toInt(), FALSE);
                }
                    break;
                default:{
                    cerr << "Invalid f_current_stage " << f_current_stage << "\n";
                }
                    break;
            }
         
            f_line_number++;
        }

        file.close();
       
        m_sample_table->lms_mod_matrix->resizeColumnsToContents();
        
        m_suppress_selected_sample_changed = TRUE;
        
        m_sample_start_hslider->setValue(m_sample_starts[m_selected_sample_index_combobox->currentIndex()]);
        m_sample_end_hslider->setValue(m_sample_ends[m_selected_sample_index_combobox->currentIndex()]);
        
        m_suppress_selected_sample_changed = FALSE;
        
        generate_files_string();
#ifndef LMS_DEBUG_STANDALONE
            lo_send(m_host, m_configurePath, "ss", "load", files_string.toLocal8Bit().data());
#endif              
    }
    
}

//From Modulex

void SamplerGUI::fx0knob0Changed(int value){ lms_value_changed(value, m_fx0->lms_knob1); }
void SamplerGUI::fx0knob1Changed(int value){ lms_value_changed(value, m_fx0->lms_knob2); }
void SamplerGUI::fx0knob2Changed(int value){ lms_value_changed(value, m_fx0->lms_knob3); }
void SamplerGUI::fx0comboboxChanged(int value){ lms_value_changed(value, m_fx0->lms_combobox); m_fx0->lms_combobox_changed(); }

void SamplerGUI::fx1knob0Changed(int value){ lms_value_changed(value, m_fx1->lms_knob1); }
void SamplerGUI::fx1knob1Changed(int value){ lms_value_changed(value, m_fx1->lms_knob2); }
void SamplerGUI::fx1knob2Changed(int value){ lms_value_changed(value, m_fx1->lms_knob3); }
void SamplerGUI::fx1comboboxChanged(int value){ lms_value_changed(value, m_fx1->lms_combobox); m_fx1->lms_combobox_changed(); }

void SamplerGUI::fx2knob0Changed(int value){ lms_value_changed(value, m_fx2->lms_knob1); }
void SamplerGUI::fx2knob1Changed(int value){ lms_value_changed(value, m_fx2->lms_knob2); }
void SamplerGUI::fx2knob2Changed(int value){ lms_value_changed(value, m_fx2->lms_knob3); }
void SamplerGUI::fx2comboboxChanged(int value){ lms_value_changed(value, m_fx2->lms_combobox); m_fx2->lms_combobox_changed(); }

void SamplerGUI::fx3knob0Changed(int value){ lms_value_changed(value, m_fx3->lms_knob1); }
void SamplerGUI::fx3knob1Changed(int value){ lms_value_changed(value, m_fx3->lms_knob2); }
void SamplerGUI::fx3knob2Changed(int value){ lms_value_changed(value, m_fx3->lms_knob3); }
void SamplerGUI::fx3comboboxChanged(int value){ lms_value_changed(value, m_fx3->lms_combobox); m_fx3->lms_combobox_changed(); }

//End from Modulex

/*synth_qt_gui.cpp Autogenerated slots*/

void SamplerGUI::sample_pitch0Changed(int a_value){sample_pitchChanged(0);}
void SamplerGUI::sample_pitch1Changed(int a_value){sample_pitchChanged(1);}
void SamplerGUI::sample_pitch2Changed(int a_value){sample_pitchChanged(2);}
void SamplerGUI::sample_pitch3Changed(int a_value){sample_pitchChanged(3);}
void SamplerGUI::sample_pitch4Changed(int a_value){sample_pitchChanged(4);}
void SamplerGUI::sample_pitch5Changed(int a_value){sample_pitchChanged(5);}
void SamplerGUI::sample_pitch6Changed(int a_value){sample_pitchChanged(6);}
void SamplerGUI::sample_pitch7Changed(int a_value){sample_pitchChanged(7);}
void SamplerGUI::sample_pitch8Changed(int a_value){sample_pitchChanged(8);}
void SamplerGUI::sample_pitch9Changed(int a_value){sample_pitchChanged(9);}
void SamplerGUI::sample_pitch10Changed(int a_value){sample_pitchChanged(10);}
void SamplerGUI::sample_pitch11Changed(int a_value){sample_pitchChanged(11);}
void SamplerGUI::sample_pitch12Changed(int a_value){sample_pitchChanged(12);}
void SamplerGUI::sample_pitch13Changed(int a_value){sample_pitchChanged(13);}
void SamplerGUI::sample_pitch14Changed(int a_value){sample_pitchChanged(14);}
void SamplerGUI::sample_pitch15Changed(int a_value){sample_pitchChanged(15);}
void SamplerGUI::sample_pitch16Changed(int a_value){sample_pitchChanged(16);}
void SamplerGUI::sample_pitch17Changed(int a_value){sample_pitchChanged(17);}
void SamplerGUI::sample_pitch18Changed(int a_value){sample_pitchChanged(18);}
void SamplerGUI::sample_pitch19Changed(int a_value){sample_pitchChanged(19);}
void SamplerGUI::sample_pitch20Changed(int a_value){sample_pitchChanged(20);}
void SamplerGUI::sample_pitch21Changed(int a_value){sample_pitchChanged(21);}
void SamplerGUI::sample_pitch22Changed(int a_value){sample_pitchChanged(22);}
void SamplerGUI::sample_pitch23Changed(int a_value){sample_pitchChanged(23);}
void SamplerGUI::sample_pitch24Changed(int a_value){sample_pitchChanged(24);}
void SamplerGUI::sample_pitch25Changed(int a_value){sample_pitchChanged(25);}
void SamplerGUI::sample_pitch26Changed(int a_value){sample_pitchChanged(26);}
void SamplerGUI::sample_pitch27Changed(int a_value){sample_pitchChanged(27);}
void SamplerGUI::sample_pitch28Changed(int a_value){sample_pitchChanged(28);}
void SamplerGUI::sample_pitch29Changed(int a_value){sample_pitchChanged(29);}
void SamplerGUI::sample_pitch30Changed(int a_value){sample_pitchChanged(30);}
void SamplerGUI::sample_pitch31Changed(int a_value){sample_pitchChanged(31);}
void SamplerGUI::sample_lnote0Changed(int a_value){sample_lnoteChanged(0);}
void SamplerGUI::sample_lnote1Changed(int a_value){sample_lnoteChanged(1);}
void SamplerGUI::sample_lnote2Changed(int a_value){sample_lnoteChanged(2);}
void SamplerGUI::sample_lnote3Changed(int a_value){sample_lnoteChanged(3);}
void SamplerGUI::sample_lnote4Changed(int a_value){sample_lnoteChanged(4);}
void SamplerGUI::sample_lnote5Changed(int a_value){sample_lnoteChanged(5);}
void SamplerGUI::sample_lnote6Changed(int a_value){sample_lnoteChanged(6);}
void SamplerGUI::sample_lnote7Changed(int a_value){sample_lnoteChanged(7);}
void SamplerGUI::sample_lnote8Changed(int a_value){sample_lnoteChanged(8);}
void SamplerGUI::sample_lnote9Changed(int a_value){sample_lnoteChanged(9);}
void SamplerGUI::sample_lnote10Changed(int a_value){sample_lnoteChanged(10);}
void SamplerGUI::sample_lnote11Changed(int a_value){sample_lnoteChanged(11);}
void SamplerGUI::sample_lnote12Changed(int a_value){sample_lnoteChanged(12);}
void SamplerGUI::sample_lnote13Changed(int a_value){sample_lnoteChanged(13);}
void SamplerGUI::sample_lnote14Changed(int a_value){sample_lnoteChanged(14);}
void SamplerGUI::sample_lnote15Changed(int a_value){sample_lnoteChanged(15);}
void SamplerGUI::sample_lnote16Changed(int a_value){sample_lnoteChanged(16);}
void SamplerGUI::sample_lnote17Changed(int a_value){sample_lnoteChanged(17);}
void SamplerGUI::sample_lnote18Changed(int a_value){sample_lnoteChanged(18);}
void SamplerGUI::sample_lnote19Changed(int a_value){sample_lnoteChanged(19);}
void SamplerGUI::sample_lnote20Changed(int a_value){sample_lnoteChanged(20);}
void SamplerGUI::sample_lnote21Changed(int a_value){sample_lnoteChanged(21);}
void SamplerGUI::sample_lnote22Changed(int a_value){sample_lnoteChanged(22);}
void SamplerGUI::sample_lnote23Changed(int a_value){sample_lnoteChanged(23);}
void SamplerGUI::sample_lnote24Changed(int a_value){sample_lnoteChanged(24);}
void SamplerGUI::sample_lnote25Changed(int a_value){sample_lnoteChanged(25);}
void SamplerGUI::sample_lnote26Changed(int a_value){sample_lnoteChanged(26);}
void SamplerGUI::sample_lnote27Changed(int a_value){sample_lnoteChanged(27);}
void SamplerGUI::sample_lnote28Changed(int a_value){sample_lnoteChanged(28);}
void SamplerGUI::sample_lnote29Changed(int a_value){sample_lnoteChanged(29);}
void SamplerGUI::sample_lnote30Changed(int a_value){sample_lnoteChanged(30);}
void SamplerGUI::sample_lnote31Changed(int a_value){sample_lnoteChanged(31);}
void SamplerGUI::sample_hnote0Changed(int a_value){sample_hnoteChanged(0);}
void SamplerGUI::sample_hnote1Changed(int a_value){sample_hnoteChanged(1);}
void SamplerGUI::sample_hnote2Changed(int a_value){sample_hnoteChanged(2);}
void SamplerGUI::sample_hnote3Changed(int a_value){sample_hnoteChanged(3);}
void SamplerGUI::sample_hnote4Changed(int a_value){sample_hnoteChanged(4);}
void SamplerGUI::sample_hnote5Changed(int a_value){sample_hnoteChanged(5);}
void SamplerGUI::sample_hnote6Changed(int a_value){sample_hnoteChanged(6);}
void SamplerGUI::sample_hnote7Changed(int a_value){sample_hnoteChanged(7);}
void SamplerGUI::sample_hnote8Changed(int a_value){sample_hnoteChanged(8);}
void SamplerGUI::sample_hnote9Changed(int a_value){sample_hnoteChanged(9);}
void SamplerGUI::sample_hnote10Changed(int a_value){sample_hnoteChanged(10);}
void SamplerGUI::sample_hnote11Changed(int a_value){sample_hnoteChanged(11);}
void SamplerGUI::sample_hnote12Changed(int a_value){sample_hnoteChanged(12);}
void SamplerGUI::sample_hnote13Changed(int a_value){sample_hnoteChanged(13);}
void SamplerGUI::sample_hnote14Changed(int a_value){sample_hnoteChanged(14);}
void SamplerGUI::sample_hnote15Changed(int a_value){sample_hnoteChanged(15);}
void SamplerGUI::sample_hnote16Changed(int a_value){sample_hnoteChanged(16);}
void SamplerGUI::sample_hnote17Changed(int a_value){sample_hnoteChanged(17);}
void SamplerGUI::sample_hnote18Changed(int a_value){sample_hnoteChanged(18);}
void SamplerGUI::sample_hnote19Changed(int a_value){sample_hnoteChanged(19);}
void SamplerGUI::sample_hnote20Changed(int a_value){sample_hnoteChanged(20);}
void SamplerGUI::sample_hnote21Changed(int a_value){sample_hnoteChanged(21);}
void SamplerGUI::sample_hnote22Changed(int a_value){sample_hnoteChanged(22);}
void SamplerGUI::sample_hnote23Changed(int a_value){sample_hnoteChanged(23);}
void SamplerGUI::sample_hnote24Changed(int a_value){sample_hnoteChanged(24);}
void SamplerGUI::sample_hnote25Changed(int a_value){sample_hnoteChanged(25);}
void SamplerGUI::sample_hnote26Changed(int a_value){sample_hnoteChanged(26);}
void SamplerGUI::sample_hnote27Changed(int a_value){sample_hnoteChanged(27);}
void SamplerGUI::sample_hnote28Changed(int a_value){sample_hnoteChanged(28);}
void SamplerGUI::sample_hnote29Changed(int a_value){sample_hnoteChanged(29);}
void SamplerGUI::sample_hnote30Changed(int a_value){sample_hnoteChanged(30);}
void SamplerGUI::sample_hnote31Changed(int a_value){sample_hnoteChanged(31);}
void SamplerGUI::sample_vol0Changed(int a_value){sample_volChanged(0);}
void SamplerGUI::sample_vol1Changed(int a_value){sample_volChanged(1);}
void SamplerGUI::sample_vol2Changed(int a_value){sample_volChanged(2);}
void SamplerGUI::sample_vol3Changed(int a_value){sample_volChanged(3);}
void SamplerGUI::sample_vol4Changed(int a_value){sample_volChanged(4);}
void SamplerGUI::sample_vol5Changed(int a_value){sample_volChanged(5);}
void SamplerGUI::sample_vol6Changed(int a_value){sample_volChanged(6);}
void SamplerGUI::sample_vol7Changed(int a_value){sample_volChanged(7);}
void SamplerGUI::sample_vol8Changed(int a_value){sample_volChanged(8);}
void SamplerGUI::sample_vol9Changed(int a_value){sample_volChanged(9);}
void SamplerGUI::sample_vol10Changed(int a_value){sample_volChanged(10);}
void SamplerGUI::sample_vol11Changed(int a_value){sample_volChanged(11);}
void SamplerGUI::sample_vol12Changed(int a_value){sample_volChanged(12);}
void SamplerGUI::sample_vol13Changed(int a_value){sample_volChanged(13);}
void SamplerGUI::sample_vol14Changed(int a_value){sample_volChanged(14);}
void SamplerGUI::sample_vol15Changed(int a_value){sample_volChanged(15);}
void SamplerGUI::sample_vol16Changed(int a_value){sample_volChanged(16);}
void SamplerGUI::sample_vol17Changed(int a_value){sample_volChanged(17);}
void SamplerGUI::sample_vol18Changed(int a_value){sample_volChanged(18);}
void SamplerGUI::sample_vol19Changed(int a_value){sample_volChanged(19);}
void SamplerGUI::sample_vol20Changed(int a_value){sample_volChanged(20);}
void SamplerGUI::sample_vol21Changed(int a_value){sample_volChanged(21);}
void SamplerGUI::sample_vol22Changed(int a_value){sample_volChanged(22);}
void SamplerGUI::sample_vol23Changed(int a_value){sample_volChanged(23);}
void SamplerGUI::sample_vol24Changed(int a_value){sample_volChanged(24);}
void SamplerGUI::sample_vol25Changed(int a_value){sample_volChanged(25);}
void SamplerGUI::sample_vol26Changed(int a_value){sample_volChanged(26);}
void SamplerGUI::sample_vol27Changed(int a_value){sample_volChanged(27);}
void SamplerGUI::sample_vol28Changed(int a_value){sample_volChanged(28);}
void SamplerGUI::sample_vol29Changed(int a_value){sample_volChanged(29);}
void SamplerGUI::sample_vol30Changed(int a_value){sample_volChanged(30);}
void SamplerGUI::sample_vol31Changed(int a_value){sample_volChanged(31);}
//new
void SamplerGUI::sample_vel_sens0Changed(int a_value){sample_vel_sensChanged(0);}
void SamplerGUI::sample_vel_sens1Changed(int a_value){sample_vel_sensChanged(1);}
void SamplerGUI::sample_vel_sens2Changed(int a_value){sample_vel_sensChanged(2);}
void SamplerGUI::sample_vel_sens3Changed(int a_value){sample_vel_sensChanged(3);}
void SamplerGUI::sample_vel_sens4Changed(int a_value){sample_vel_sensChanged(4);}
void SamplerGUI::sample_vel_sens5Changed(int a_value){sample_vel_sensChanged(5);}
void SamplerGUI::sample_vel_sens6Changed(int a_value){sample_vel_sensChanged(6);}
void SamplerGUI::sample_vel_sens7Changed(int a_value){sample_vel_sensChanged(7);}
void SamplerGUI::sample_vel_sens8Changed(int a_value){sample_vel_sensChanged(8);}
void SamplerGUI::sample_vel_sens9Changed(int a_value){sample_vel_sensChanged(9);}
void SamplerGUI::sample_vel_sens10Changed(int a_value){sample_vel_sensChanged(10);}
void SamplerGUI::sample_vel_sens11Changed(int a_value){sample_vel_sensChanged(11);}
void SamplerGUI::sample_vel_sens12Changed(int a_value){sample_vel_sensChanged(12);}
void SamplerGUI::sample_vel_sens13Changed(int a_value){sample_vel_sensChanged(13);}
void SamplerGUI::sample_vel_sens14Changed(int a_value){sample_vel_sensChanged(14);}
void SamplerGUI::sample_vel_sens15Changed(int a_value){sample_vel_sensChanged(15);}
void SamplerGUI::sample_vel_sens16Changed(int a_value){sample_vel_sensChanged(16);}
void SamplerGUI::sample_vel_sens17Changed(int a_value){sample_vel_sensChanged(17);}
void SamplerGUI::sample_vel_sens18Changed(int a_value){sample_vel_sensChanged(18);}
void SamplerGUI::sample_vel_sens19Changed(int a_value){sample_vel_sensChanged(19);}
void SamplerGUI::sample_vel_sens20Changed(int a_value){sample_vel_sensChanged(20);}
void SamplerGUI::sample_vel_sens21Changed(int a_value){sample_vel_sensChanged(21);}
void SamplerGUI::sample_vel_sens22Changed(int a_value){sample_vel_sensChanged(22);}
void SamplerGUI::sample_vel_sens23Changed(int a_value){sample_vel_sensChanged(23);}
void SamplerGUI::sample_vel_sens24Changed(int a_value){sample_vel_sensChanged(24);}
void SamplerGUI::sample_vel_sens25Changed(int a_value){sample_vel_sensChanged(25);}
void SamplerGUI::sample_vel_sens26Changed(int a_value){sample_vel_sensChanged(26);}
void SamplerGUI::sample_vel_sens27Changed(int a_value){sample_vel_sensChanged(27);}
void SamplerGUI::sample_vel_sens28Changed(int a_value){sample_vel_sensChanged(28);}
void SamplerGUI::sample_vel_sens29Changed(int a_value){sample_vel_sensChanged(29);}
void SamplerGUI::sample_vel_sens30Changed(int a_value){sample_vel_sensChanged(30);}
void SamplerGUI::sample_vel_sens31Changed(int a_value){sample_vel_sensChanged(31);}
void SamplerGUI::sample_vel_low0Changed(int a_value){sample_vel_lowChanged(0);}
void SamplerGUI::sample_vel_low1Changed(int a_value){sample_vel_lowChanged(1);}
void SamplerGUI::sample_vel_low2Changed(int a_value){sample_vel_lowChanged(2);}
void SamplerGUI::sample_vel_low3Changed(int a_value){sample_vel_lowChanged(3);}
void SamplerGUI::sample_vel_low4Changed(int a_value){sample_vel_lowChanged(4);}
void SamplerGUI::sample_vel_low5Changed(int a_value){sample_vel_lowChanged(5);}
void SamplerGUI::sample_vel_low6Changed(int a_value){sample_vel_lowChanged(6);}
void SamplerGUI::sample_vel_low7Changed(int a_value){sample_vel_lowChanged(7);}
void SamplerGUI::sample_vel_low8Changed(int a_value){sample_vel_lowChanged(8);}
void SamplerGUI::sample_vel_low9Changed(int a_value){sample_vel_lowChanged(9);}
void SamplerGUI::sample_vel_low10Changed(int a_value){sample_vel_lowChanged(10);}
void SamplerGUI::sample_vel_low11Changed(int a_value){sample_vel_lowChanged(11);}
void SamplerGUI::sample_vel_low12Changed(int a_value){sample_vel_lowChanged(12);}
void SamplerGUI::sample_vel_low13Changed(int a_value){sample_vel_lowChanged(13);}
void SamplerGUI::sample_vel_low14Changed(int a_value){sample_vel_lowChanged(14);}
void SamplerGUI::sample_vel_low15Changed(int a_value){sample_vel_lowChanged(15);}
void SamplerGUI::sample_vel_low16Changed(int a_value){sample_vel_lowChanged(16);}
void SamplerGUI::sample_vel_low17Changed(int a_value){sample_vel_lowChanged(17);}
void SamplerGUI::sample_vel_low18Changed(int a_value){sample_vel_lowChanged(18);}
void SamplerGUI::sample_vel_low19Changed(int a_value){sample_vel_lowChanged(19);}
void SamplerGUI::sample_vel_low20Changed(int a_value){sample_vel_lowChanged(20);}
void SamplerGUI::sample_vel_low21Changed(int a_value){sample_vel_lowChanged(21);}
void SamplerGUI::sample_vel_low22Changed(int a_value){sample_vel_lowChanged(22);}
void SamplerGUI::sample_vel_low23Changed(int a_value){sample_vel_lowChanged(23);}
void SamplerGUI::sample_vel_low24Changed(int a_value){sample_vel_lowChanged(24);}
void SamplerGUI::sample_vel_low25Changed(int a_value){sample_vel_lowChanged(25);}
void SamplerGUI::sample_vel_low26Changed(int a_value){sample_vel_lowChanged(26);}
void SamplerGUI::sample_vel_low27Changed(int a_value){sample_vel_lowChanged(27);}
void SamplerGUI::sample_vel_low28Changed(int a_value){sample_vel_lowChanged(28);}
void SamplerGUI::sample_vel_low29Changed(int a_value){sample_vel_lowChanged(29);}
void SamplerGUI::sample_vel_low30Changed(int a_value){sample_vel_lowChanged(30);}
void SamplerGUI::sample_vel_low31Changed(int a_value){sample_vel_lowChanged(31);}
void SamplerGUI::sample_vel_high0Changed(int a_value){sample_vel_highChanged(0);}
void SamplerGUI::sample_vel_high1Changed(int a_value){sample_vel_highChanged(1);}
void SamplerGUI::sample_vel_high2Changed(int a_value){sample_vel_highChanged(2);}
void SamplerGUI::sample_vel_high3Changed(int a_value){sample_vel_highChanged(3);}
void SamplerGUI::sample_vel_high4Changed(int a_value){sample_vel_highChanged(4);}
void SamplerGUI::sample_vel_high5Changed(int a_value){sample_vel_highChanged(5);}
void SamplerGUI::sample_vel_high6Changed(int a_value){sample_vel_highChanged(6);}
void SamplerGUI::sample_vel_high7Changed(int a_value){sample_vel_highChanged(7);}
void SamplerGUI::sample_vel_high8Changed(int a_value){sample_vel_highChanged(8);}
void SamplerGUI::sample_vel_high9Changed(int a_value){sample_vel_highChanged(9);}
void SamplerGUI::sample_vel_high10Changed(int a_value){sample_vel_highChanged(10);}
void SamplerGUI::sample_vel_high11Changed(int a_value){sample_vel_highChanged(11);}
void SamplerGUI::sample_vel_high12Changed(int a_value){sample_vel_highChanged(12);}
void SamplerGUI::sample_vel_high13Changed(int a_value){sample_vel_highChanged(13);}
void SamplerGUI::sample_vel_high14Changed(int a_value){sample_vel_highChanged(14);}
void SamplerGUI::sample_vel_high15Changed(int a_value){sample_vel_highChanged(15);}
void SamplerGUI::sample_vel_high16Changed(int a_value){sample_vel_highChanged(16);}
void SamplerGUI::sample_vel_high17Changed(int a_value){sample_vel_highChanged(17);}
void SamplerGUI::sample_vel_high18Changed(int a_value){sample_vel_highChanged(18);}
void SamplerGUI::sample_vel_high19Changed(int a_value){sample_vel_highChanged(19);}
void SamplerGUI::sample_vel_high20Changed(int a_value){sample_vel_highChanged(20);}
void SamplerGUI::sample_vel_high21Changed(int a_value){sample_vel_highChanged(21);}
void SamplerGUI::sample_vel_high22Changed(int a_value){sample_vel_highChanged(22);}
void SamplerGUI::sample_vel_high23Changed(int a_value){sample_vel_highChanged(23);}
void SamplerGUI::sample_vel_high24Changed(int a_value){sample_vel_highChanged(24);}
void SamplerGUI::sample_vel_high25Changed(int a_value){sample_vel_highChanged(25);}
void SamplerGUI::sample_vel_high26Changed(int a_value){sample_vel_highChanged(26);}
void SamplerGUI::sample_vel_high27Changed(int a_value){sample_vel_highChanged(27);}
void SamplerGUI::sample_vel_high28Changed(int a_value){sample_vel_highChanged(28);}
void SamplerGUI::sample_vel_high29Changed(int a_value){sample_vel_highChanged(29);}
void SamplerGUI::sample_vel_high30Changed(int a_value){sample_vel_highChanged(30);}
void SamplerGUI::sample_vel_high31Changed(int a_value){sample_vel_highChanged(31);}
//From PolyFX mod matrix
void SamplerGUI::pfxmatrix_grp0dst0src0ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST0SRC0CTRL0, 0, 0, 0, 0);}
void SamplerGUI::pfxmatrix_grp0dst0src0ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST0SRC0CTRL1, 0, 0, 1, 0);}
void SamplerGUI::pfxmatrix_grp0dst0src0ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST0SRC0CTRL2, 0, 0, 2, 0);}
void SamplerGUI::pfxmatrix_grp0dst0src1ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST0SRC1CTRL0, 0, 0, 0, 1);}
void SamplerGUI::pfxmatrix_grp0dst0src1ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST0SRC1CTRL1, 0, 0, 1, 1);}
void SamplerGUI::pfxmatrix_grp0dst0src1ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST0SRC1CTRL2, 0, 0, 2, 1);}
void SamplerGUI::pfxmatrix_grp0dst0src2ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST0SRC2CTRL0, 0, 0, 0, 2);}
void SamplerGUI::pfxmatrix_grp0dst0src2ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST0SRC2CTRL1, 0, 0, 1, 2);}
void SamplerGUI::pfxmatrix_grp0dst0src2ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST0SRC2CTRL2, 0, 0, 2, 2);}
void SamplerGUI::pfxmatrix_grp0dst0src3ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST0SRC3CTRL0, 0, 0, 0, 3);}
void SamplerGUI::pfxmatrix_grp0dst0src3ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST0SRC3CTRL1, 0, 0, 1, 3);}
void SamplerGUI::pfxmatrix_grp0dst0src3ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST0SRC3CTRL2, 0, 0, 2, 3);}
void SamplerGUI::pfxmatrix_grp0dst1src0ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST1SRC0CTRL0, 0, 1, 0, 0);}
void SamplerGUI::pfxmatrix_grp0dst1src0ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST1SRC0CTRL1, 0, 1, 1, 0);}
void SamplerGUI::pfxmatrix_grp0dst1src0ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST1SRC0CTRL2, 0, 1, 2, 0);}
void SamplerGUI::pfxmatrix_grp0dst1src1ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST1SRC1CTRL0, 0, 1, 0, 1);}
void SamplerGUI::pfxmatrix_grp0dst1src1ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST1SRC1CTRL1, 0, 1, 1, 1);}
void SamplerGUI::pfxmatrix_grp0dst1src1ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST1SRC1CTRL2, 0, 1, 2, 1);}
void SamplerGUI::pfxmatrix_grp0dst1src2ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST1SRC2CTRL0, 0, 1, 0, 2);}
void SamplerGUI::pfxmatrix_grp0dst1src2ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST1SRC2CTRL1, 0, 1, 1, 2);}
void SamplerGUI::pfxmatrix_grp0dst1src2ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST1SRC2CTRL2, 0, 1, 2, 2);}
void SamplerGUI::pfxmatrix_grp0dst1src3ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST1SRC3CTRL0, 0, 1, 0, 3);}
void SamplerGUI::pfxmatrix_grp0dst1src3ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST1SRC3CTRL1, 0, 1, 1, 3);}
void SamplerGUI::pfxmatrix_grp0dst1src3ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST1SRC3CTRL2, 0, 1, 2, 3);}
void SamplerGUI::pfxmatrix_grp0dst2src0ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST2SRC0CTRL0, 0, 2, 0, 0);}
void SamplerGUI::pfxmatrix_grp0dst2src0ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST2SRC0CTRL1, 0, 2, 1, 0);}
void SamplerGUI::pfxmatrix_grp0dst2src0ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST2SRC0CTRL2, 0, 2, 2, 0);}
void SamplerGUI::pfxmatrix_grp0dst2src1ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST2SRC1CTRL0, 0, 2, 0, 1);}
void SamplerGUI::pfxmatrix_grp0dst2src1ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST2SRC1CTRL1, 0, 2, 1, 1);}
void SamplerGUI::pfxmatrix_grp0dst2src1ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST2SRC1CTRL2, 0, 2, 2, 1);}
void SamplerGUI::pfxmatrix_grp0dst2src2ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST2SRC2CTRL0, 0, 2, 0, 2);}
void SamplerGUI::pfxmatrix_grp0dst2src2ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST2SRC2CTRL1, 0, 2, 1, 2);}
void SamplerGUI::pfxmatrix_grp0dst2src2ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST2SRC2CTRL2, 0, 2, 2, 2);}
void SamplerGUI::pfxmatrix_grp0dst2src3ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST2SRC3CTRL0, 0, 2, 0, 3);}
void SamplerGUI::pfxmatrix_grp0dst2src3ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST2SRC3CTRL1, 0, 2, 1, 3);}
void SamplerGUI::pfxmatrix_grp0dst2src3ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST2SRC3CTRL2, 0, 2, 2, 3);}
void SamplerGUI::pfxmatrix_grp0dst3src0ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST3SRC0CTRL0, 0, 3, 0, 0);}
void SamplerGUI::pfxmatrix_grp0dst3src0ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST3SRC0CTRL1, 0, 3, 1, 0);}
void SamplerGUI::pfxmatrix_grp0dst3src0ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST3SRC0CTRL2, 0, 3, 2, 0);}
void SamplerGUI::pfxmatrix_grp0dst3src1ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST3SRC1CTRL0, 0, 3, 0, 1);}
void SamplerGUI::pfxmatrix_grp0dst3src1ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST3SRC1CTRL1, 0, 3, 1, 1);}
void SamplerGUI::pfxmatrix_grp0dst3src1ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST3SRC1CTRL2, 0, 3, 2, 1);}
void SamplerGUI::pfxmatrix_grp0dst3src2ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST3SRC2CTRL0, 0, 3, 0, 2);}
void SamplerGUI::pfxmatrix_grp0dst3src2ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST3SRC2CTRL1, 0, 3, 1, 2);}
void SamplerGUI::pfxmatrix_grp0dst3src2ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST3SRC2CTRL2, 0, 3, 2, 2);}
void SamplerGUI::pfxmatrix_grp0dst3src3ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST3SRC3CTRL0, 0, 3, 0, 3);}
void SamplerGUI::pfxmatrix_grp0dst3src3ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST3SRC3CTRL1, 0, 3, 1, 3);}
void SamplerGUI::pfxmatrix_grp0dst3src3ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST3SRC3CTRL2, 0, 3, 2, 3);}

/*End synth_qt_gui.cpp Autogenerated slots*/

void SamplerGUI::global_midi_octaves_offsetChanged(int a_value)
{
    lms_value_changed(a_value, m_global_midi_octaves_offset);
}
/*
void SamplerGUI::global_midi_channelChanged(int a_value)
{
    lms_value_changed(a_value, m_global_midi_channel);
}
*/
void SamplerGUI::set_global_midi_octaves_offset(float val)
{
    lms_set_value(val, m_global_midi_octaves_offset);
}
/*
void SamplerGUI::set_global_midi_channel(float val)
{
    lms_set_value(val, m_global_midi_channel);
}
*/

//Begin Ray-V PolyFX

void SamplerGUI::lms_set_value(float val, LMS_control * a_ctrl )
{    
    m_suppressHostUpdate = true;
    a_ctrl->lms_set_value(int(val));
    m_suppressHostUpdate = false;     
}

//From Modulex

void SamplerGUI::setFX0knob0(float val){ lms_set_value(val, m_fx0->lms_knob1); }
void SamplerGUI::setFX0knob1(float val){ lms_set_value(val, m_fx0->lms_knob2); }
void SamplerGUI::setFX0knob2(float val){ lms_set_value(val, m_fx0->lms_knob3); }
void SamplerGUI::setFX0combobox(float val){ lms_set_value(val, m_fx0->lms_combobox); }

void SamplerGUI::setFX1knob0(float val){ lms_set_value(val, m_fx1->lms_knob1); }
void SamplerGUI::setFX1knob1(float val){ lms_set_value(val, m_fx1->lms_knob2); }
void SamplerGUI::setFX1knob2(float val){ lms_set_value(val, m_fx1->lms_knob3); }
void SamplerGUI::setFX1combobox(float val){ lms_set_value(val, m_fx1->lms_combobox); }

void SamplerGUI::setFX2knob0(float val){ lms_set_value(val, m_fx2->lms_knob1); }
void SamplerGUI::setFX2knob1(float val){ lms_set_value(val, m_fx2->lms_knob2); }
void SamplerGUI::setFX2knob2(float val){ lms_set_value(val, m_fx2->lms_knob3); }
void SamplerGUI::setFX2combobox(float val){ lms_set_value(val, m_fx2->lms_combobox); }

void SamplerGUI::setFX3knob0(float val){ lms_set_value(val, m_fx3->lms_knob1); }
void SamplerGUI::setFX3knob1(float val){ lms_set_value(val, m_fx3->lms_knob2); }
void SamplerGUI::setFX3knob2(float val){ lms_set_value(val, m_fx3->lms_knob3); }
void SamplerGUI::setFX3combobox(float val){ lms_set_value(val, m_fx3->lms_combobox); }

//End from Modulex

void SamplerGUI::setAttack(float a_value){ lms_set_value(a_value, m_adsr_amp->lms_attack);}
void SamplerGUI::setDecay(float a_value){ lms_set_value(a_value, m_adsr_amp->lms_decay); }
void SamplerGUI::setSustain(float a_value){lms_set_value(a_value, m_adsr_amp->lms_sustain);}
void SamplerGUI::setRelease(float a_value){lms_set_value(a_value, m_adsr_amp->lms_release);}
void SamplerGUI::setFilterAttack (float a_value){lms_set_value(a_value, m_adsr_filter->lms_attack);}
void SamplerGUI::setFilterDecay  (float a_value){lms_set_value(a_value, m_adsr_filter->lms_decay);}
void SamplerGUI::setFilterSustain(float a_value){lms_set_value(a_value, m_adsr_filter->lms_sustain);}
void SamplerGUI::setFilterRelease(float a_value){lms_set_value(a_value, m_adsr_filter->lms_release);}
void SamplerGUI::setNoiseAmp(float a_value){lms_set_value(a_value, m_noise_amp);}
void SamplerGUI::setNoiseType(float a_value){lms_set_value(a_value, m_noise_type);}
void SamplerGUI::setMasterVolume(float a_value){lms_set_value(a_value, m_master->lms_master_volume);}
void SamplerGUI::setMasterUnisonVoices(float a_value){lms_set_value(a_value, m_master->lms_master_unison_voices);}
void SamplerGUI::setMasterUnisonSpread(float a_value){lms_set_value(a_value, m_master->lms_master_unison_spread);}
void SamplerGUI::setMasterGlide(float a_value){lms_set_value(a_value, m_master->lms_master_glide);}
void SamplerGUI::setMasterPitchbendAmt(float a_value){lms_set_value(a_value, m_master->lms_master_pitchbend_amt);}
void SamplerGUI::setPitchEnvTime(float a_value){lms_set_value(a_value, m_pitch_env->lms_time_knob);}
void SamplerGUI::setLFOfreq(float a_value){lms_set_value(a_value, m_lfo->lms_freq_knob);}
void SamplerGUI::setLFOtype(float a_value){lms_set_value(a_value, m_lfo->lms_type_combobox);}

void SamplerGUI::lms_value_changed(int a_value, LMS_control * a_ctrl)
{    
    a_ctrl->lms_value_changed(a_value);

    if (!m_suppressHostUpdate) {
        lo_send(m_host, m_controlPath, "if", (a_ctrl->lms_port), float(a_value));
    }    
}

void SamplerGUI::attackChanged(int a_value){lms_value_changed(a_value, m_adsr_amp->lms_attack);}
void SamplerGUI::decayChanged(int a_value){lms_value_changed(a_value, m_adsr_amp->lms_decay);}
void SamplerGUI::sustainChanged(int a_value){lms_value_changed(a_value, m_adsr_amp->lms_sustain);}
void SamplerGUI::releaseChanged(int a_value){lms_value_changed(a_value, m_adsr_amp->lms_release);}
void SamplerGUI::filterAttackChanged(int a_value){lms_value_changed(a_value, m_adsr_filter->lms_attack);}
void SamplerGUI::filterDecayChanged(int a_value){lms_value_changed(a_value, m_adsr_filter->lms_decay);}
void SamplerGUI::filterSustainChanged(int a_value){lms_value_changed(a_value, m_adsr_filter->lms_sustain);}
void SamplerGUI::filterReleaseChanged(int a_value){lms_value_changed(a_value, m_adsr_filter->lms_release);}
void SamplerGUI::noiseAmpChanged(int a_value){lms_value_changed(a_value, m_noise_amp);}
void SamplerGUI::noise_typeChanged(int a_value){lms_value_changed(a_value, m_noise_type);}
void SamplerGUI::masterVolumeChanged(int a_value){lms_value_changed(a_value, m_master->lms_master_volume);}
void SamplerGUI::masterUnisonVoicesChanged(int a_value){lms_value_changed(a_value, m_master->lms_master_unison_voices);}
void SamplerGUI::masterUnisonSpreadChanged(int a_value){lms_value_changed(a_value, m_master->lms_master_unison_spread);}
void SamplerGUI::masterGlideChanged(int a_value){lms_value_changed(a_value, m_master->lms_master_glide);}
void SamplerGUI::masterPitchbendAmtChanged(int a_value){lms_value_changed(a_value, m_master->lms_master_pitchbend_amt);}
void SamplerGUI::pitchEnvTimeChanged(int a_value){lms_value_changed(a_value, m_pitch_env->lms_time_knob);}
void SamplerGUI::LFOfreqChanged(int a_value){lms_value_changed(a_value, m_lfo->lms_freq_knob);}
void SamplerGUI::LFOtypeChanged(int a_value){lms_value_changed(a_value, m_lfo->lms_type_combobox);}

void SamplerGUI::v_print_port_name_to_cerr(int a_port)
{
#ifdef LMS_DEBUG_MODE_QT
    switch (a_port) {
    case LMS_ATTACK: cerr << "LMS_ATTACK"; break;
    case LMS_DECAY: cerr << "LMS_DECAY"; break;
    case LMS_SUSTAIN: cerr << "LMS_SUSTAIN"; break;
    case LMS_RELEASE: cerr << "LMS_RELEASE"; break;
    case LMS_TIMBRE: cerr << "LMS_TIMBRE"; break;
    case LMS_RES: cerr << "LMS_RES"; break;        
    case LMS_DIST: cerr << "LMS_DIST"; break;
    case LMS_FILTER_ATTACK: cerr << "LMS_FILTER_ATTACK"; break;
    case LMS_FILTER_DECAY: cerr << "LMS_FILTER_DECAY"; break;
    case LMS_FILTER_SUSTAIN: cerr << "LMS_FILTER_SUSTAIN"; break;
    case LMS_FILTER_RELEASE: cerr << "LMS_FILTER_RELEASE"; break;
    case LMS_NOISE_AMP: cerr << "LMS_NOISE_AMP"; break;    
    case LMS_DIST_WET: cerr << "LMS_DIST_WET"; break;            
    case LMS_FILTER_ENV_AMT: cerr << "LMS_FILTER_ENV_AMT"; break;    
    case LMS_OSC1_TYPE: cerr << "LMS_OSC1_TYPE"; break;            
    case LMS_OSC1_PITCH: cerr << "LMS_OSC1_PITCH"; break;    
    case LMS_OSC1_TUNE: cerr << "LMS_OSC1_TUNE"; break;    
    case LMS_OSC1_VOLUME: cerr << "LMS_OSC1_VOLUME"; break;        
    case LMS_OSC2_TYPE: cerr << "LMS_OSC2_TYPE"; break;            
    case LMS_OSC2_PITCH: cerr << "LMS_OSC2_PITCH"; break;    
    case LMS_OSC2_TUNE: cerr << "LMS_OSC2_TUNE";  break;    
    case LMS_OSC2_VOLUME: cerr << "LMS_OSC2_VOLUME"; break;        
    case LMS_MASTER_VOLUME: cerr << "LMS_MASTER_VOLUME"; break;
    case LMS_MASTER_UNISON_VOICES: cerr << "LMS_MASTER_UNISON_VOICES"; break;
    case LMS_MASTER_UNISON_SPREAD: cerr << "LMS_MASTER_UNISON_SPREAD"; break;
    case LMS_MASTER_GLIDE: cerr << "LMS_MASTER_GLIDE"; break;
    case LMS_MASTER_PITCHBEND_AMT: cerr << "LMS_MASTER_PITCHBEND_AMT"; break;
    case LMS_PITCH_ENV_AMT: cerr << "LMS_PITCH_ENV_AMT "; break;
    case LMS_PITCH_ENV_TIME: cerr << "LMS_PITCH_ENV_TIME ";  break;        
    case LMS_PROGRAM_CHANGE: cerr << "LMS_PROGRAM_CHANGE "; break;
    default: cerr << "Warning: received request to set nonexistent port " << a_port ; break;
    }
#endif
}

void SamplerGUI::v_set_control(int port, float a_value)
{

    if(port < LMS_SAMPLE_PITCH_PORT_RANGE_MIN)
    {
        switch (port) {
            case Sampler_SELECTED_SAMPLE: setSelection(a_value); break;
            case LMS_ATTACK: setAttack(a_value); break;
            case LMS_DECAY: setDecay(a_value); break;
            case LMS_SUSTAIN: setSustain(a_value); break;
            case LMS_RELEASE: setRelease(a_value); break;
            case LMS_FILTER_ATTACK: setFilterAttack(a_value); break;
            case LMS_FILTER_DECAY: setFilterDecay(a_value); break;
            case LMS_FILTER_SUSTAIN: setFilterSustain(a_value); break;
            case LMS_FILTER_RELEASE: setFilterRelease(a_value); break;
            case LMS_NOISE_AMP: setNoiseAmp(a_value); break;        
            case LMS_NOISE_TYPE: setNoiseType(a_value); break;
            case LMS_MASTER_VOLUME: setMasterVolume(a_value); break;
            case LMS_MASTER_GLIDE: setMasterGlide(a_value); break;
            case LMS_MASTER_PITCHBEND_AMT: setMasterPitchbendAmt(a_value); break;            
            case LMS_PITCH_ENV_TIME: setPitchEnvTime(a_value); break;                
            case LMS_LFO_FREQ: setLFOfreq(a_value); break;            
            case LMS_LFO_TYPE:  setLFOtype(a_value);  break;
            //From Modulex            
            case LMS_FX0_KNOB0:	setFX0knob0(a_value); break;
            case LMS_FX0_KNOB1:	setFX0knob1(a_value); break;        
            case LMS_FX0_KNOB2:	setFX0knob2(a_value); break;        
            case LMS_FX0_COMBOBOX: setFX0combobox(a_value); break;

            case LMS_FX1_KNOB0:	setFX1knob0(a_value); break;
            case LMS_FX1_KNOB1:	setFX1knob1(a_value); break;        
            case LMS_FX1_KNOB2:	setFX1knob2(a_value); break;        
            case LMS_FX1_COMBOBOX: setFX1combobox(a_value); break;

            case LMS_FX2_KNOB0:	setFX2knob0(a_value); break;
            case LMS_FX2_KNOB1:	setFX2knob1(a_value); break;        
            case LMS_FX2_KNOB2:	setFX2knob2(a_value); break;        
            case LMS_FX2_COMBOBOX: setFX2combobox(a_value); break;

            case LMS_FX3_KNOB0:	setFX3knob0(a_value); break;
            case LMS_FX3_KNOB1:	setFX3knob1(a_value); break;        
            case LMS_FX3_KNOB2:	setFX3knob2(a_value); break;        
            case LMS_FX3_COMBOBOX: setFX3combobox(a_value); break;
            //End from Modulex            
            //From PolyFX mod matrix
            case LMS_PFXMATRIX_GRP0DST0SRC0CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[0]->controls[0]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST0SRC0CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[1]->controls[0]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST0SRC0CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[2]->controls[0]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST0SRC1CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[0]->controls[1]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST0SRC1CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[1]->controls[1]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST0SRC1CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[2]->controls[1]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST0SRC2CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[0]->controls[2]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST0SRC2CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[1]->controls[2]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST0SRC2CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[2]->controls[2]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST0SRC3CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[0]->controls[3]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST0SRC3CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[1]->controls[3]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST0SRC3CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[2]->controls[3]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST1SRC0CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[3]->controls[0]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST1SRC0CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[4]->controls[0]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST1SRC0CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[5]->controls[0]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST1SRC1CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[3]->controls[1]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST1SRC1CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[4]->controls[1]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST1SRC1CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[5]->controls[1]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST1SRC2CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[3]->controls[2]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST1SRC2CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[4]->controls[2]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST1SRC2CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[5]->controls[2]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST1SRC3CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[3]->controls[3]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST1SRC3CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[4]->controls[3]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST1SRC3CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[5]->controls[3]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST2SRC0CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[6]->controls[0]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST2SRC0CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[7]->controls[0]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST2SRC0CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[8]->controls[0]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST2SRC1CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[6]->controls[1]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST2SRC1CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[7]->controls[1]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST2SRC1CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[8]->controls[1]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST2SRC2CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[6]->controls[2]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST2SRC2CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[7]->controls[2]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST2SRC2CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[8]->controls[2]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST2SRC3CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[6]->controls[3]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST2SRC3CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[7]->controls[3]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST2SRC3CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[8]->controls[3]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST3SRC0CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[9]->controls[0]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST3SRC0CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[10]->controls[0]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST3SRC0CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[11]->controls[0]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST3SRC1CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[9]->controls[1]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST3SRC1CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[10]->controls[1]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST3SRC1CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[11]->controls[1]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST3SRC2CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[9]->controls[2]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST3SRC2CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[10]->controls[2]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST3SRC2CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[11]->controls[2]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST3SRC3CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[9]->controls[3]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST3SRC3CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[10]->controls[3]->lms_get_widget()))->setValue(a_value); break;
            case LMS_PFXMATRIX_GRP0DST3SRC3CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[11]->controls[3]->lms_get_widget()))->setValue(a_value); break;
            //End PolyFX mod matrix
            //case LMS_GLOBAL_MIDI_CHANNEL: set_global_midi_channel(a_value); break;
            case LMS_GLOBAL_MIDI_OCTAVES_OFFSET: set_global_midi_octaves_offset(a_value); break;
        }
    
    }
    else if((port >= LMS_SAMPLE_PITCH_PORT_RANGE_MIN) && (port < LMS_SAMPLE_PITCH_PORT_RANGE_MAX))
    {
        ((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[(port - LMS_SAMPLE_PITCH_PORT_RANGE_MIN)]))->lms_set_value(a_value);
    }
    else if((port >= LMS_PLAY_PITCH_LOW_PORT_RANGE_MIN) && (port < LMS_PLAY_PITCH_LOW_PORT_RANGE_MAX))
    {
        ((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[(port - LMS_PLAY_PITCH_LOW_PORT_RANGE_MIN)]))->lms_set_value(a_value);
    }    
    else if((port >= LMS_PLAY_PITCH_HIGH_PORT_RANGE_MIN) && (port < LMS_PLAY_PITCH_HIGH_PORT_RANGE_MAX))
    {
        ((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[(port - LMS_PLAY_PITCH_HIGH_PORT_RANGE_MIN)]))->lms_set_value(a_value);
    }
    else if((port >= LMS_SAMPLE_VOLUME_PORT_RANGE_MIN) && (port < LMS_SAMPLE_VOLUME_PORT_RANGE_MAX))
    {
        ((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[(port - LMS_SAMPLE_VOLUME_PORT_RANGE_MIN)]->lms_get_widget()))->setValue(a_value);
    }
    else if((port >= LMS_SAMPLE_START_PORT_RANGE_MIN) && (port < LMS_SAMPLE_START_PORT_RANGE_MAX))
    {
        m_sample_starts[(port - LMS_SAMPLE_START_PORT_RANGE_MIN)] = a_value;
    }
    else if((port >= LMS_SAMPLE_END_PORT_RANGE_MIN) && (port < LMS_SAMPLE_END_PORT_RANGE_MAX))
    {
        m_sample_ends[(port - LMS_SAMPLE_END_PORT_RANGE_MIN)] = a_value;
    }
    else if((port >= LMS_SAMPLE_VEL_SENS_PORT_RANGE_MIN) && (port < LMS_SAMPLE_VEL_SENS_PORT_RANGE_MAX))
    {
        ((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_SENS_INDEX]->controls[(port - LMS_SAMPLE_VEL_SENS_PORT_RANGE_MIN)]->lms_get_widget()))->setValue(a_value);
    }
    else if((port >= LMS_SAMPLE_VEL_LOW_PORT_RANGE_MIN) && (port < LMS_SAMPLE_VEL_LOW_PORT_RANGE_MAX))
    {
        ((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_LOW_INDEX]->controls[(port - LMS_SAMPLE_VEL_LOW_PORT_RANGE_MIN)]->lms_get_widget()))->setValue(a_value);
    }
    else if((port >= LMS_SAMPLE_VEL_HIGH_PORT_RANGE_MIN) && (port < LMS_SAMPLE_VEL_HIGH_PORT_RANGE_MAX))
    {
        ((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[(port - LMS_SAMPLE_VEL_HIGH_PORT_RANGE_MIN)]->lms_get_widget()))->setValue(a_value);
    }
    else
    {
        cerr << "v_set_control called with invalid port " << port << "\n";
    }

}

void SamplerGUI::v_control_changed(int port, int a_value, bool a_suppress_host_update)
{
    
#ifdef LMS_DEBUG_MODE_QT    
    cerr << "v_control_changed called.  ";  
    v_print_port_name_to_cerr(a_port);
    cerr << "  value: " << a_value << endl;
#endif
    
    if(a_suppress_host_update)
        m_suppressHostUpdate = true;      
    
    if((port < LMS_SAMPLE_PITCH_PORT_RANGE_MIN) && (port < LMS_SAMPLE_PITCH_PORT_RANGE_MIN))
    {
        switch (port) 
        {
            case Sampler_SELECTED_SAMPLE: selectionChanged(); break;
            case LMS_ATTACK: attackChanged(a_value); break;
            case LMS_DECAY: decayChanged(a_value); break;
            case LMS_SUSTAIN: sustainChanged(a_value); break;
            case LMS_RELEASE: releaseChanged(a_value); break;
            case LMS_FILTER_ATTACK: filterAttackChanged(a_value); break;
            case LMS_FILTER_DECAY: filterDecayChanged(a_value); break;
            case LMS_FILTER_SUSTAIN: filterSustainChanged(a_value); break;
            case LMS_FILTER_RELEASE: filterReleaseChanged(a_value); break;
            case LMS_NOISE_AMP: noiseAmpChanged(a_value); break;      
            case LMS_NOISE_TYPE: noise_typeChanged(a_value); break;      
            case LMS_MASTER_VOLUME: masterVolumeChanged(a_value); break;
            case LMS_MASTER_GLIDE: masterGlideChanged(a_value); break;
            case LMS_MASTER_PITCHBEND_AMT: masterPitchbendAmtChanged(a_value); break;            
            case LMS_PITCH_ENV_TIME: pitchEnvTimeChanged(a_value); break;
            case LMS_LFO_FREQ: LFOfreqChanged(a_value); break;
            case LMS_LFO_TYPE: LFOtypeChanged(a_value); break;
            //From Modulex            
            case LMS_FX0_KNOB0:	fx0knob0Changed(a_value); break;
            case LMS_FX0_KNOB1:	fx0knob1Changed(a_value); break;
            case LMS_FX0_KNOB2:	fx0knob2Changed(a_value); break;  
            case LMS_FX0_COMBOBOX:  fx0comboboxChanged(a_value); break;

            case LMS_FX1_KNOB0:	fx1knob0Changed(a_value); break;
            case LMS_FX1_KNOB1:	fx1knob1Changed(a_value); break;
            case LMS_FX1_KNOB2:	fx1knob2Changed(a_value); break;  
            case LMS_FX1_COMBOBOX:  fx1comboboxChanged(a_value); break;

            case LMS_FX2_KNOB0:	fx2knob0Changed(a_value); break;
            case LMS_FX2_KNOB1:	fx2knob1Changed(a_value); break;
            case LMS_FX2_KNOB2:	fx2knob2Changed(a_value); break;  
            case LMS_FX2_COMBOBOX:  fx2comboboxChanged(a_value); break;

            case LMS_FX3_KNOB0:	fx3knob0Changed(a_value); break;
            case LMS_FX3_KNOB1:	fx3knob1Changed(a_value); break;
            case LMS_FX3_KNOB2:	fx3knob2Changed(a_value); break;  
            case LMS_FX3_COMBOBOX:  fx3comboboxChanged(a_value); break;
            //End from Modulex
            //From PolyFX mod matrix
            case LMS_PFXMATRIX_GRP0DST0SRC0CTRL0:  pfxmatrix_grp0dst0src0ctrl0Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST0SRC0CTRL1:  pfxmatrix_grp0dst0src0ctrl1Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST0SRC0CTRL2:  pfxmatrix_grp0dst0src0ctrl2Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST0SRC1CTRL0:  pfxmatrix_grp0dst0src1ctrl0Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST0SRC1CTRL1:  pfxmatrix_grp0dst0src1ctrl1Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST0SRC1CTRL2:  pfxmatrix_grp0dst0src1ctrl2Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST0SRC2CTRL0:  pfxmatrix_grp0dst0src2ctrl0Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST0SRC2CTRL1:  pfxmatrix_grp0dst0src2ctrl1Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST0SRC2CTRL2:  pfxmatrix_grp0dst0src2ctrl2Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST0SRC3CTRL0:  pfxmatrix_grp0dst0src3ctrl0Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST0SRC3CTRL1:  pfxmatrix_grp0dst0src3ctrl1Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST0SRC3CTRL2:  pfxmatrix_grp0dst0src3ctrl2Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST1SRC0CTRL0:  pfxmatrix_grp0dst1src0ctrl0Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST1SRC0CTRL1:  pfxmatrix_grp0dst1src0ctrl1Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST1SRC0CTRL2:  pfxmatrix_grp0dst1src0ctrl2Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST1SRC1CTRL0:  pfxmatrix_grp0dst1src1ctrl0Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST1SRC1CTRL1:  pfxmatrix_grp0dst1src1ctrl1Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST1SRC1CTRL2:  pfxmatrix_grp0dst1src1ctrl2Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST1SRC2CTRL0:  pfxmatrix_grp0dst1src2ctrl0Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST1SRC2CTRL1:  pfxmatrix_grp0dst1src2ctrl1Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST1SRC2CTRL2:  pfxmatrix_grp0dst1src2ctrl2Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST1SRC3CTRL0:  pfxmatrix_grp0dst1src3ctrl0Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST1SRC3CTRL1:  pfxmatrix_grp0dst1src3ctrl1Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST1SRC3CTRL2:  pfxmatrix_grp0dst1src3ctrl2Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST2SRC0CTRL0:  pfxmatrix_grp0dst2src0ctrl0Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST2SRC0CTRL1:  pfxmatrix_grp0dst2src0ctrl1Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST2SRC0CTRL2:  pfxmatrix_grp0dst2src0ctrl2Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST2SRC1CTRL0:  pfxmatrix_grp0dst2src1ctrl0Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST2SRC1CTRL1:  pfxmatrix_grp0dst2src1ctrl1Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST2SRC1CTRL2:  pfxmatrix_grp0dst2src1ctrl2Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST2SRC2CTRL0:  pfxmatrix_grp0dst2src2ctrl0Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST2SRC2CTRL1:  pfxmatrix_grp0dst2src2ctrl1Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST2SRC2CTRL2:  pfxmatrix_grp0dst2src2ctrl2Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST2SRC3CTRL0:  pfxmatrix_grp0dst2src3ctrl0Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST2SRC3CTRL1:  pfxmatrix_grp0dst2src3ctrl1Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST2SRC3CTRL2:  pfxmatrix_grp0dst2src3ctrl2Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST3SRC0CTRL0:  pfxmatrix_grp0dst3src0ctrl0Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST3SRC0CTRL1:  pfxmatrix_grp0dst3src0ctrl1Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST3SRC0CTRL2:  pfxmatrix_grp0dst3src0ctrl2Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST3SRC1CTRL0:  pfxmatrix_grp0dst3src1ctrl0Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST3SRC1CTRL1:  pfxmatrix_grp0dst3src1ctrl1Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST3SRC1CTRL2:  pfxmatrix_grp0dst3src1ctrl2Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST3SRC2CTRL0:  pfxmatrix_grp0dst3src2ctrl0Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST3SRC2CTRL1:  pfxmatrix_grp0dst3src2ctrl1Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST3SRC2CTRL2:  pfxmatrix_grp0dst3src2ctrl2Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST3SRC3CTRL0:  pfxmatrix_grp0dst3src3ctrl0Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST3SRC3CTRL1:  pfxmatrix_grp0dst3src3ctrl1Changed(a_value); break;
            case LMS_PFXMATRIX_GRP0DST3SRC3CTRL2:  pfxmatrix_grp0dst3src3ctrl2Changed(a_value); break;
            
            case LMS_GLOBAL_MIDI_OCTAVES_OFFSET: global_midi_octaves_offsetChanged(a_value); break;
        }
    
    }
    else if((port >= LMS_SAMPLE_PITCH_PORT_RANGE_MIN) && (port < LMS_SAMPLE_PITCH_PORT_RANGE_MAX))
    {
        sample_pitchChanged((port - LMS_SAMPLE_PITCH_PORT_RANGE_MIN));
    }
    else if((port >= LMS_PLAY_PITCH_LOW_PORT_RANGE_MIN) && (port < LMS_PLAY_PITCH_LOW_PORT_RANGE_MAX))
    {
        sample_lnoteChanged((port - LMS_PLAY_PITCH_LOW_PORT_RANGE_MIN));
    }    
    else if((port >= LMS_PLAY_PITCH_HIGH_PORT_RANGE_MIN) && (port < LMS_PLAY_PITCH_HIGH_PORT_RANGE_MAX))
    {
        sample_hnoteChanged((port - LMS_PLAY_PITCH_HIGH_PORT_RANGE_MIN));
    }
    else if((port >= LMS_SAMPLE_VOLUME_PORT_RANGE_MIN) && (port < LMS_SAMPLE_VOLUME_PORT_RANGE_MAX))
    {
        sample_volChanged((port - LMS_SAMPLE_VOLUME_PORT_RANGE_MIN));
    }
    else if((port >= LMS_SAMPLE_START_PORT_RANGE_MIN) && (port < LMS_SAMPLE_START_PORT_RANGE_MAX))
    {   
        if (!m_suppressHostUpdate) 
        {
                lo_send(m_host, m_controlPath, "if", port, float(a_value));
        }        
    }
    else if((port >= LMS_SAMPLE_END_PORT_RANGE_MIN) && (port < LMS_SAMPLE_END_PORT_RANGE_MAX))
    {
        if (!m_suppressHostUpdate) 
        {
                lo_send(m_host, m_controlPath, "if", port, float(a_value));
        }
    }
    else if((port >= LMS_SAMPLE_VEL_SENS_PORT_RANGE_MIN) && (port < LMS_SAMPLE_VEL_SENS_PORT_RANGE_MAX))
    {
        sample_vel_sensChanged((port - LMS_SAMPLE_VEL_SENS_PORT_RANGE_MIN));
    }
    else if((port >= LMS_SAMPLE_VEL_LOW_PORT_RANGE_MIN) && (port < LMS_SAMPLE_VEL_LOW_PORT_RANGE_MAX))
    {
        sample_vel_lowChanged((port - LMS_SAMPLE_VEL_LOW_PORT_RANGE_MIN));
    }
    else if((port >= LMS_SAMPLE_VEL_HIGH_PORT_RANGE_MIN) && (port < LMS_SAMPLE_VEL_HIGH_PORT_RANGE_MAX))
    {
        sample_vel_highChanged((port - LMS_SAMPLE_VEL_HIGH_PORT_RANGE_MIN));
    }
    else
    {
        cerr << "v_control_changed called with invalid port " << port << "\n";
    }
    
    if(a_suppress_host_update)
        m_suppressHostUpdate = false;
    
}

int SamplerGUI::i_get_control(int port)
{
    /*Add the controls you created to the control handler*/
    
    if((port < LMS_SAMPLE_PITCH_PORT_RANGE_MIN) && (port < LMS_SAMPLE_PITCH_PORT_RANGE_MIN))
    {
        switch (port) 
        {
            case Sampler_SELECTED_SAMPLE: m_sample_table->find_selected_radio_button(SMP_TB_RADIOBUTTON_INDEX); return m_sample_table->lms_selected_column;
            case LMS_ATTACK: return  m_adsr_amp->lms_attack->lms_get_value();
            case LMS_DECAY:  return m_adsr_amp->lms_decay->lms_get_value();
            case LMS_SUSTAIN: return m_adsr_amp->lms_sustain->lms_get_value();
            case LMS_RELEASE: return m_adsr_amp->lms_release->lms_get_value();
            case LMS_FILTER_ATTACK: return m_adsr_filter->lms_attack->lms_get_value();
            case LMS_FILTER_DECAY: return m_adsr_filter->lms_decay->lms_get_value();
            case LMS_FILTER_SUSTAIN: return m_adsr_filter->lms_sustain->lms_get_value();
            case LMS_FILTER_RELEASE: return m_adsr_filter->lms_release->lms_get_value();
            case LMS_NOISE_AMP: return m_noise_amp->lms_get_value();
            case LMS_NOISE_TYPE: return m_noise_type->lms_get_value();
            case LMS_MASTER_VOLUME: return m_master->lms_master_volume->lms_get_value();
            case LMS_MASTER_GLIDE: return m_master->lms_master_glide->lms_get_value();
            case LMS_MASTER_PITCHBEND_AMT: return m_master->lms_master_pitchbend_amt->lms_get_value();
            case LMS_PITCH_ENV_TIME: return m_pitch_env->lms_time_knob->lms_get_value();
            case LMS_LFO_FREQ: return m_lfo->lms_freq_knob->lms_get_value();
            case LMS_LFO_TYPE: return m_lfo->lms_type_combobox->lms_get_value();
            
            //From Modulex            
            case LMS_FX0_KNOB0: return m_fx0->lms_knob1->lms_get_value();
            case LMS_FX0_KNOB1: return m_fx0->lms_knob2->lms_get_value();
            case LMS_FX0_KNOB2: return m_fx0->lms_knob3->lms_get_value();
            case LMS_FX0_COMBOBOX: return m_fx0->lms_combobox->lms_get_value();

            case LMS_FX1_KNOB0: return m_fx1->lms_knob1->lms_get_value();
            case LMS_FX1_KNOB1: return m_fx1->lms_knob2->lms_get_value();
            case LMS_FX1_KNOB2: return m_fx1->lms_knob3->lms_get_value();
            case LMS_FX1_COMBOBOX: return m_fx1->lms_combobox->lms_get_value();

            case LMS_FX2_KNOB0: return m_fx2->lms_knob1->lms_get_value();
            case LMS_FX2_KNOB1: return m_fx2->lms_knob2->lms_get_value();
            case LMS_FX2_KNOB2: return m_fx2->lms_knob3->lms_get_value();
            case LMS_FX2_COMBOBOX: return m_fx2->lms_combobox->lms_get_value();

            case LMS_FX3_KNOB0: return m_fx3->lms_knob1->lms_get_value();
            case LMS_FX3_KNOB1: return m_fx3->lms_knob2->lms_get_value();
            case LMS_FX3_KNOB2: return m_fx3->lms_knob3->lms_get_value();
            case LMS_FX3_COMBOBOX: return m_fx3->lms_combobox->lms_get_value();
            //End from Modulex
            //From PolyFX mod matrix
            case LMS_PFXMATRIX_GRP0DST0SRC0CTRL0: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[0]->controls[0]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST0SRC0CTRL1: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[1]->controls[0]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST0SRC0CTRL2: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[2]->controls[0]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST0SRC1CTRL0: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[0]->controls[1]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST0SRC1CTRL1: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[1]->controls[1]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST0SRC1CTRL2: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[2]->controls[1]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST0SRC2CTRL0: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[0]->controls[2]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST0SRC2CTRL1: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[1]->controls[2]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST0SRC2CTRL2: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[2]->controls[2]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST0SRC3CTRL0: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[0]->controls[3]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST0SRC3CTRL1: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[1]->controls[3]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST0SRC3CTRL2: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[2]->controls[3]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST1SRC0CTRL0: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[3]->controls[0]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST1SRC0CTRL1: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[4]->controls[0]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST1SRC0CTRL2: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[5]->controls[0]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST1SRC1CTRL0: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[3]->controls[1]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST1SRC1CTRL1: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[4]->controls[1]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST1SRC1CTRL2: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[5]->controls[1]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST1SRC2CTRL0: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[3]->controls[2]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST1SRC2CTRL1: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[4]->controls[2]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST1SRC2CTRL2: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[5]->controls[2]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST1SRC3CTRL0: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[3]->controls[3]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST1SRC3CTRL1: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[4]->controls[3]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST1SRC3CTRL2: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[5]->controls[3]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST2SRC0CTRL0: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[6]->controls[0]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST2SRC0CTRL1: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[7]->controls[0]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST2SRC0CTRL2: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[8]->controls[0]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST2SRC1CTRL0: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[6]->controls[1]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST2SRC1CTRL1: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[7]->controls[1]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST2SRC1CTRL2: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[8]->controls[1]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST2SRC2CTRL0: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[6]->controls[2]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST2SRC2CTRL1: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[7]->controls[2]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST2SRC2CTRL2: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[8]->controls[2]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST2SRC3CTRL0: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[6]->controls[3]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST2SRC3CTRL1: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[7]->controls[3]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST2SRC3CTRL2: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[8]->controls[3]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST3SRC0CTRL0: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[9]->controls[0]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST3SRC0CTRL1: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[10]->controls[0]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST3SRC0CTRL2: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[11]->controls[0]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST3SRC1CTRL0: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[9]->controls[1]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST3SRC1CTRL1: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[10]->controls[1]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST3SRC1CTRL2: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[11]->controls[1]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST3SRC2CTRL0: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[9]->controls[2]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST3SRC2CTRL1: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[10]->controls[2]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST3SRC2CTRL2: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[11]->controls[2]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST3SRC3CTRL0: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[9]->controls[3]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST3SRC3CTRL1: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[10]->controls[3]->lms_get_widget()))->value();
            case LMS_PFXMATRIX_GRP0DST3SRC3CTRL2: return ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[11]->controls[3]->lms_get_widget()))->value();

            case LMS_GLOBAL_MIDI_OCTAVES_OFFSET: return m_global_midi_octaves_offset->lms_get_value();
            //End from PolyFX mod matrix            
            default: cerr << "i_get_control called with invalid port " << port << "\n"; return 0;
        }    
    }
    else if((port >= LMS_SAMPLE_PITCH_PORT_RANGE_MIN) && (port < LMS_SAMPLE_PITCH_PORT_RANGE_MAX))
    {
        return ((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[(port - LMS_SAMPLE_PITCH_PORT_RANGE_MIN)]))->lms_get_value();
    }
    else if((port >= LMS_PLAY_PITCH_LOW_PORT_RANGE_MIN) && (port < LMS_PLAY_PITCH_LOW_PORT_RANGE_MAX))
    {
        return ((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[(port - LMS_PLAY_PITCH_LOW_PORT_RANGE_MIN)]))->lms_get_value();
    }    
    else if((port >= LMS_PLAY_PITCH_HIGH_PORT_RANGE_MIN) && (port < LMS_PLAY_PITCH_HIGH_PORT_RANGE_MAX))
    {
        return ((LMS_note_selector*)(m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[(port - LMS_PLAY_PITCH_HIGH_PORT_RANGE_MIN)]))->lms_get_value();
    }
    else if((port >= LMS_SAMPLE_VOLUME_PORT_RANGE_MIN) && (port < LMS_SAMPLE_VOLUME_PORT_RANGE_MAX))
    {
        return ((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[(port - LMS_SAMPLE_VOLUME_PORT_RANGE_MIN)]->lms_get_widget()))->value();
    }    
    else if((port >= LMS_SAMPLE_START_PORT_RANGE_MIN) && (port < LMS_SAMPLE_START_PORT_RANGE_MAX))
    {
        return m_sample_starts[(port - LMS_SAMPLE_START_PORT_RANGE_MIN)];
    }        
    else if((port >= LMS_SAMPLE_END_PORT_RANGE_MIN) && (port < LMS_SAMPLE_END_PORT_RANGE_MAX))
    {
        return m_sample_ends[(port - LMS_SAMPLE_END_PORT_RANGE_MIN)];
    }
    else if((port >= LMS_SAMPLE_VEL_SENS_PORT_RANGE_MIN) && (port < LMS_SAMPLE_VEL_SENS_PORT_RANGE_MAX))
    {
        return ((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_SENS_INDEX]->controls[(port - LMS_SAMPLE_VEL_SENS_PORT_RANGE_MIN)]->lms_get_widget()))->value();
    }
    else if((port >= LMS_SAMPLE_VEL_LOW_PORT_RANGE_MIN) && (port < LMS_SAMPLE_VEL_LOW_PORT_RANGE_MAX))
    {
        return ((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_LOW_INDEX]->controls[(port - LMS_SAMPLE_VEL_LOW_PORT_RANGE_MIN)]->lms_get_widget()))->value();
    }
    else if((port >= LMS_SAMPLE_VEL_HIGH_PORT_RANGE_MIN) && (port < LMS_SAMPLE_VEL_HIGH_PORT_RANGE_MAX))
    {
        return ((QSpinBox*)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[(port - LMS_SAMPLE_VEL_HIGH_PORT_RANGE_MIN)]->lms_get_widget()))->value();
    }
    else
    {
        cerr << "i_get_control called with invalid port " << port << "\n";
        return 0;
    }
}

void
SamplerGUI::oscRecv()
{
#ifndef LMS_DEBUG_STANDALONE
    if (osc_server) {
	lo_server_recv_noblock(osc_server, 1);
    }
#endif
}

void
SamplerGUI::aboutToQuit()
{
#ifndef LMS_DEBUG_STANDALONE
    if (!m_hostRequestedQuit) lo_send(m_host, m_exitingPath, "");
#endif
}

SamplerGUI::~SamplerGUI()
{
#ifndef LMS_DEBUG_STANDALONE
    lo_address_free(m_host);
#endif
}


void osc_error(int num, const char *msg, const char *path)
{
    cerr << "Error: liblo server error " << num
	 << " in path \"" << (path ? path : "(null)")
	 << "\": " << msg << endl;
}

int debug_handler(const char *path, const char *types, lo_arg **argv,
	      int argc, void *data, void *user_data)
{
    int i;

    cerr << "Warning: unhandled OSC message in GUI:" << endl;

    for (i = 0; i < argc; ++i) {
	cerr << "arg " << i << ": type '" << types[i] << "': ";
#ifndef LMS_DEBUG_STANDALONE        
        lo_arg_pp((lo_type)types[i], argv[i]);
#endif        
	cerr << endl;
    }

    cerr << "(path is <" << path << ">)" << endl;
    return 1;
}

int configure_handler(const char *path, const char *types, lo_arg **argv,
		  int argc, void *data, void *user_data)
{
    SamplerGUI *gui = static_cast<SamplerGUI *>(user_data);
    const char *key = (const char *)&argv[0]->s;
    const char *value = (const char *)&argv[1]->s;

    cerr << "GUI configure_handler:  Key:  " << QString::fromLocal8Bit(key) << " , Value:" << QString::fromLocal8Bit(value);
    
    if (!strcmp(key, "load")) {
	gui->setSampleFile(QString::fromLocal8Bit(value));
    } else if (!strcmp(key, DSSI_PROJECT_DIRECTORY_KEY)) {
	//gui->setProjectDirectory(QString::fromLocal8Bit(value));
    } else if (!strcmp(key, "lastdir")) {
        gui->m_file_selector->lms_last_directory = QString::fromLocal8Bit(value);
        gui->m_file_browser->folder_opened(QString::fromLocal8Bit(value), FALSE);
    }

    return 0;
}

int rate_handler(const char *path, const char *types, lo_arg **argv,
	     int argc, void *data, void *user_data)
{
    return 0;
}

int show_handler(const char *path, const char *types, lo_arg **argv,
	     int argc, void *data, void *user_data)
{
    SamplerGUI *gui = static_cast<SamplerGUI *>(user_data);
    while (!gui->ready()) sleep(1);
    if (gui->isVisible()) gui->raise();
    else {
	QRect geometry = gui->geometry();
	QPoint p(QApplication::desktop()->width()/2 - geometry.width()/2,
		 QApplication::desktop()->height()/2 - geometry.height()/2);
	gui->move(p);
	gui->show();
    }

    return 0;
}

int hide_handler(const char *path, const char *types, lo_arg **argv,
	     int argc, void *data, void *user_data)
{
    SamplerGUI *gui = static_cast<SamplerGUI *>(user_data);
    gui->hide();
    return 0;
}

int quit_handler(const char *path, const char *types, lo_arg **argv,
	     int argc, void *data, void *user_data)
{
    SamplerGUI *gui = static_cast<SamplerGUI *>(user_data);
    gui->setHostRequestedQuit(true);
    qApp->quit();
    return 0;
}

int control_handler(const char *path, const char *types, lo_arg **argv,
		int argc, void *data, void *user_data)
{
    SamplerGUI *gui = static_cast<SamplerGUI *>(user_data);

    if (argc < 2) {
	cerr << "Error: too few arguments to control_handler" << endl;
	return 1;
    }

    const int port = argv[0]->i;
    const float value = argv[1]->f;
    
    
    if((port < LMS_SAMPLE_PITCH_PORT_RANGE_MIN))
    {
        gui->v_set_control(port, value);
    }
    else if((port >= LMS_SAMPLE_PITCH_PORT_RANGE_MIN) && (port < LMS_SAMPLE_PITCH_PORT_RANGE_MAX))
    {
        int f_value = port - LMS_SAMPLE_PITCH_PORT_RANGE_MIN;
        //cerr << "LMS_SAMPLE_PITCH_PORT_RANGE_MIN Port " << port << " f_value " << f_value  << endl;
        gui->m_suppressHostUpdate = TRUE;
        gui->m_sample_table->lms_mm_columns[SMP_TB_NOTE_INDEX]->controls[f_value]->lms_set_value(value);
        gui->m_suppressHostUpdate = FALSE;
    }
    else if((port >= LMS_PLAY_PITCH_LOW_PORT_RANGE_MIN) && (port < LMS_PLAY_PITCH_LOW_PORT_RANGE_MAX))
    {
        int f_value = port - LMS_PLAY_PITCH_LOW_PORT_RANGE_MIN;
        //cerr << "LMS_PLAY_PITCH_LOW_PORT_RANGE_MIN Port " << port << " f_value " << f_value  << endl;
        gui->m_suppressHostUpdate = TRUE;
        gui->m_sample_table->lms_mm_columns[SMP_TB_LOW_NOTE_INDEX]->controls[f_value]->lms_set_value(value);
        gui->m_suppressHostUpdate = FALSE;
    }
    else if((port >= LMS_PLAY_PITCH_HIGH_PORT_RANGE_MIN) && (port < LMS_PLAY_PITCH_HIGH_PORT_RANGE_MAX))
    {
        int f_value = port - LMS_PLAY_PITCH_HIGH_PORT_RANGE_MIN;
        //cerr << "LMS_PLAY_PITCH_HIGH_PORT_RANGE_MIN Port " << port << " f_value " << f_value  << endl;
        gui->m_suppressHostUpdate = TRUE;
        gui->m_sample_table->lms_mm_columns[SMP_TB_HIGH_NOTE_INDEX]->controls[f_value]->lms_set_value(value);
        gui->m_suppressHostUpdate = FALSE;
    }
    else if((port >= LMS_SAMPLE_VOLUME_PORT_RANGE_MIN ) && (port < LMS_SAMPLE_VOLUME_PORT_RANGE_MAX))
    {
        int f_value = port - LMS_SAMPLE_VOLUME_PORT_RANGE_MIN;
        //cerr << "LMS_SAMPLE_VOLUME_PORT_RANGE_MIN Port " << port << " f_value " << f_value  << endl;
        gui->m_suppressHostUpdate = TRUE;
        gui->m_sample_table->lms_mm_columns[SMP_TB_VOLUME_INDEX]->controls[f_value]->lms_set_value(value);
        gui->m_suppressHostUpdate = FALSE;
    }
    else if((port >= LMS_SAMPLE_START_PORT_RANGE_MIN ) && (port < LMS_SAMPLE_START_PORT_RANGE_MAX))
    {
        int f_value = port - LMS_SAMPLE_START_PORT_RANGE_MIN;
        //cerr << "LMS_SAMPLE_VOLUME_PORT_RANGE_MIN Port " << port << " f_value " << f_value  << endl;
        gui->m_sample_starts[f_value] = (int)value;
    }
    else if((port >= LMS_SAMPLE_END_PORT_RANGE_MIN ) && (port < LMS_SAMPLE_END_PORT_RANGE_MAX))
    {
        int f_value = port - LMS_SAMPLE_END_PORT_RANGE_MIN;
        //cerr << "LMS_SAMPLE_VOLUME_PORT_RANGE_MIN Port " << port << " f_value " << f_value  << endl;
        gui->m_sample_ends[f_value] = (int)value;
    }
    else if((port >= LMS_SAMPLE_VEL_SENS_PORT_RANGE_MIN ) && (port < LMS_SAMPLE_VEL_SENS_PORT_RANGE_MAX))
    {
        int f_value = port - LMS_SAMPLE_VEL_SENS_PORT_RANGE_MIN;        
        gui->m_suppressHostUpdate = TRUE;
        gui->m_sample_table->lms_mm_columns[SMP_TB_VEL_SENS_INDEX]->controls[f_value]->lms_set_value(value);
        gui->m_suppressHostUpdate = FALSE;
    }
    else if((port >= LMS_SAMPLE_VEL_LOW_PORT_RANGE_MIN ) && (port < LMS_SAMPLE_VEL_LOW_PORT_RANGE_MAX))
    {
        int f_value = port - LMS_SAMPLE_VEL_LOW_PORT_RANGE_MIN;        
        gui->m_suppressHostUpdate = TRUE;
        gui->m_sample_table->lms_mm_columns[SMP_TB_VEL_LOW_INDEX]->controls[f_value]->lms_set_value(value);
        gui->m_suppressHostUpdate = FALSE;
    }
    else if((port >= LMS_SAMPLE_VEL_HIGH_PORT_RANGE_MIN ) && (port < LMS_SAMPLE_VEL_HIGH_PORT_RANGE_MAX))
    {
        int f_value = port - LMS_SAMPLE_VEL_HIGH_PORT_RANGE_MIN;        
        gui->m_suppressHostUpdate = TRUE;
        gui->m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[f_value]->lms_set_value(value);
        gui->m_suppressHostUpdate = FALSE;
    }
    else
    {
        cerr << "Warning: received request to set nonexistent port " << port << endl;
    }

    return 0;
}

int main(int argc, char **argv)
{
    cerr << "Euphoria GUI starting..." << endl;

    QApplication application(argc, argv);
    
#ifndef LMS_DEBUG_STANDALONE    
    if (application.argc() != 5) {
	cerr << "usage: "
	     << application.argv()[0] 
	     << " <osc url>"
	     << " <plugin dllname>"
	     << " <plugin label>"
	     << " <user-friendly id>"
	     << endl;
	return 2;        
    }
#endif
    
#ifdef Q_WS_X11
    XSetErrorHandler(handle_x11_error);
#endif

#ifndef LMS_DEBUG_STANDALONE
    char *url = application.argv()[1];

    char *host = lo_url_get_hostname(url);
    char *port = lo_url_get_port(url);
    char *path = lo_url_get_path(url);

    char *label = application.argv()[3];
    bool stereo = false;
    if (QString(label).toLower() == QString(Sampler_Stereo_LABEL).toLower()) {
	stereo = true;
    }
#else
    char *url = "testing";

    char *host = "localhost";
    char *port = "10000";
    char *path = "/usr/lib/dssi/test";

    char *label = "Debug Mode - No Audio";
    bool stereo = true;    
#endif
    
    SamplerGUI gui(stereo, host, port,
		   QByteArray(path) + "/control",
		   QByteArray(path) + "/midi",
		   QByteArray(path) + "/configure",
		   QByteArray(path) + "/exiting",
		   0);
		 
    QByteArray myControlPath = QByteArray(path) + "/control";
    QByteArray myConfigurePath = QByteArray(path) + "/configure";
    QByteArray myRatePath = QByteArray(path) + "/sample-rate";
    QByteArray myShowPath = QByteArray(path) + "/show";
    QByteArray myHidePath = QByteArray(path) + "/hide";
    QByteArray myQuitPath = QByteArray(path) + "/quit";
#ifndef LMS_DEBUG_STANDALONE
    osc_server = lo_server_new(NULL, osc_error);
    lo_server_add_method(osc_server, myControlPath, "if", control_handler, &gui);
    lo_server_add_method(osc_server, myConfigurePath, "ss", configure_handler, &gui);
    lo_server_add_method(osc_server, myRatePath, "i", rate_handler, &gui);
    lo_server_add_method(osc_server, myShowPath, "", show_handler, &gui);
    lo_server_add_method(osc_server, myHidePath, "", hide_handler, &gui);
    lo_server_add_method(osc_server, myQuitPath, "", quit_handler, &gui);
    lo_server_add_method(osc_server, NULL, NULL, debug_handler, &gui);

    lo_address hostaddr = lo_address_new(host, port);
    lo_send(hostaddr,
	    QByteArray(path) + "/update",
	    "s",
	    (QByteArray(lo_server_get_url(osc_server)) + QByteArray(path+1)).data());
#endif
    QObject::connect(&application, SIGNAL(aboutToQuit()), &gui, SLOT(aboutToQuit()));

    gui.setReady(true);
#ifdef LMS_DEBUG_STANDALONE
    gui.show();
#endif
    return application.exec();
}

