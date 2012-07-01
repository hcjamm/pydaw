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
#include <QFont>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/SM/SMlib.h>
#include <qt4/QtGui/qabstractbutton.h>


static int handle_x11_error(Display *dpy, XErrorEvent *err)
{
    char errstr[256];
    XGetErrorText(dpy, err->error_code, errstr, 256);
    if (err->error_code != BadWindow) {
	std::cerr << "trivial_sampler_qt_gui: X Error: "
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
#define SMP_TB_FILE_PATH_INDEX 5

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
    this->setStyleSheet("QMessageBox{color:white;background-color:black;} QTabBar::tab:selected { color:black;background-color:#BBBBBB;} QTableView QTableCornerButton::section {background: black; border: 2px outset white;} QComboBox{color:white; background-color:black;} QTabBar::tab {background-color:black;  border: 2px solid white;  border-bottom-color: #333333; border-top-left-radius: 4px;  border-top-right-radius: 4px;  min-width: 8ex;  padding: 2px; color:white;} QHeaderView::section {background: black; color: white;border:2px solid white;} QPushButton {background-color: black; border-style: outset; border-width: 2px; border-radius: 10px;border-color: white;font: bold 14px; min-width: 10em; padding: 6px; color:white;}  QAbstractItemView {outline: none;} QLabel{color:black;background-color:white;border:solid 2px white;border-radius:2px;} QFrame{background-color:qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0.273, stop:0 rgba(90, 90, 90, 255), stop:1 rgba(60, 60, 60, 255))} QGroupBox {background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #111111, stop: 1 #222222); border: 2px solid white;  border-radius: 10px;  margin-top: 1ex;} QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 3px; color:black; background-color: white; border solid 2px white; border-radius:3px;}");
    
    m_handle_control_updates = true;
    m_creating_instrument_file = FALSE;
    m_suppress_selected_sample_changed = FALSE;
    
    LMS_style_info * a_style = new LMS_style_info(64);
    //a_style->LMS_set_value_style("")
        
    QList <LMS_mod_matrix_column*> f_sample_table_columns;
    
    f_sample_table_columns << new LMS_mod_matrix_column(radiobutton, QString(""), 0, 1, 0);  //Selected row      
    f_sample_table_columns << new LMS_mod_matrix_column(note_selector, QString("Sample Pitch"), 0, 1, 3);  //Sample base pitch
    f_sample_table_columns << new LMS_mod_matrix_column(note_selector, QString("Low Note"), 0, 1, -2);  //Low Note
    f_sample_table_columns << new LMS_mod_matrix_column(note_selector, QString("High Note"), 0, 1, 8);  //High Note    
    f_sample_table_columns << new LMS_mod_matrix_column(spinbox, QString("Volume"), -50, 36, -6);  //Volume
    f_sample_table_columns << new LMS_mod_matrix_column(no_widget, QString("Path"), 0, 1, 0);  //File path            
    
    m_sample_table = new LMS_mod_matrix(this, LMS_MAX_SAMPLE_COUNT, f_sample_table_columns, LMS_FIRST_MOD_MATRIX_PORT, a_style);
        
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
        this->resize(1024, 800);
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
        horizontalLayout_2 = new QHBoxLayout(m_sample_tab);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        m_smp_tab_scrollArea = new QScrollArea(m_sample_tab);
        m_smp_tab_scrollArea->setObjectName(QString::fromUtf8("m_smp_tab_scrollArea"));
        m_smp_tab_scrollArea->setStyleSheet(QString::fromUtf8(""));
        m_smp_tab_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        m_smp_tab_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        m_smp_tab_scrollArea->setWidgetResizable(true);
        m_smp_tab_scrollAreaWidgetContents = new QWidget();
        m_smp_tab_scrollAreaWidgetContents->setObjectName(QString::fromUtf8("m_smp_tab_scrollAreaWidgetContents"));
        m_smp_tab_scrollAreaWidgetContents->setGeometry(QRect(0, 0, 966, 728));
        horizontalLayout = new QHBoxLayout(m_smp_tab_scrollAreaWidgetContents);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        
        m_smp_tab_scrollAreaWidgetContents->setStyleSheet(QString("QTableView::item { border: 1px solid white; } QTableWidget::setShowGrid{border: 1px solid white;} QWidget{background-color:black; color:white} QComboBox{background-color:black; color:white; border:solid 1px white;} QComboBox:editable {background-color:black; color:white;} QSpinBox{color:black;background-color:white;}"));


        //m_smp_tab_main_verticalLayout->addLayout(m_loop_start_end_Layout);
        m_smp_tab_main_verticalLayout->addWidget(m_sample_table->lms_mod_matrix, Qt::AlignCenter); 
        m_smp_tab_main_verticalLayout->addLayout(m_file_selector->lms_layout);
        
        horizontalLayout->addLayout(m_smp_tab_main_verticalLayout);

        m_smp_tab_scrollArea->setWidget(m_smp_tab_scrollAreaWidgetContents);

        horizontalLayout_2->addWidget(m_smp_tab_scrollArea);

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
        
/*End synth_qt_gui.cpp Autogenerated connections*/
        
        //m_view_sample_tab
        
        m_view_sample_tab = new QWidget();
        m_view_sample_tab->setObjectName(QString::fromUtf8("m_view_sample_tab"));        
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
        
        LMS_style_info * f_info = new LMS_style_info(64);
        f_info->LMS_set_label_style("background-color: white; border: 1px solid black;  border-radius: 6px; QComboBox{color:white;background-color:black;}", 60);

        m_main_layout = new LMS_main_layout(m_poly_fx_tab);

        QLabel * f_logo_label = new QLabel("", this);    
        f_logo_label->setTextFormat(Qt::RichText);
        /*This string is a base64 encoded .png image I created using Gimp for the logo.  To get the base64 encoded string,
        run it through png_to_base64.pl in the packaging folder*/

        QString f_logo_text = "<html><img src=\"data:image/png;base64,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\"/></html>";
        f_logo_label->setText(f_logo_text);
        f_logo_label->setMinimumSize(90, 30);   
        f_logo_label->setAlignment(Qt::AlignCenter);
        f_logo_label->show();
        m_main_layout->lms_add_widget(f_logo_label);
                
        m_main_layout->lms_add_layout();

        m_adsr_amp = new LMS_adsr_widget(this, f_info, TRUE, LMS_ATTACK, LMS_DECAY, LMS_SUSTAIN, LMS_RELEASE, QString("ADSR Amp"));

        m_main_layout->lms_add_widget(m_adsr_amp->lms_groupbox_adsr->lms_groupbox);

        connect(m_adsr_amp->lms_attack->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(attackChanged(int)));    
        connect(m_adsr_amp->lms_decay->lms_knob,   SIGNAL(valueChanged(int)), this, SLOT(decayChanged(int)));        
        connect(m_adsr_amp->lms_sustain->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(sustainChanged(int)));        
        connect(m_adsr_amp->lms_release->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(releaseChanged(int)));    

        m_groupbox_distortion = new LMS_group_box(this, QString("Distortion"), f_info);
        m_main_layout->lms_add_widget(m_groupbox_distortion->lms_groupbox);

        m_dist = new LMS_knob_regular(QString("Gain"), -6, 36, 1, 12, QString("12"), m_groupbox_distortion->lms_groupbox, f_info, lms_kc_integer, LMS_DIST);
        m_groupbox_distortion->lms_add_h(m_dist);
        connect(m_dist->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(distChanged(int)));

        m_dist_wet = new LMS_knob_regular(QString("Wet"), 0, 100, 1, 0, QString(""), m_groupbox_distortion->lms_groupbox, f_info, lms_kc_none, LMS_DIST_WET);
        m_groupbox_distortion->lms_add_h(m_dist_wet);
        connect(m_dist_wet->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(distWetChanged(int)));

        m_groupbox_noise = new LMS_group_box(this, QString("Noise"), f_info);
        m_main_layout->lms_add_widget(m_groupbox_noise->lms_groupbox);

        m_noise_amp = new LMS_knob_regular(QString("Vol"), -60, 0, 1, 30, QString(""), m_groupbox_noise->lms_groupbox, f_info, lms_kc_integer, LMS_NOISE_AMP);
        m_groupbox_noise->lms_add_h(m_noise_amp);
        connect(m_noise_amp->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(noiseAmpChanged(int)));

        m_main_layout->lms_add_layout();    
        
        m_adsr_filter = new LMS_adsr_widget(this, f_info, FALSE, LMS_FILTER_ATTACK, LMS_FILTER_DECAY, LMS_FILTER_SUSTAIN, LMS_FILTER_RELEASE, QString("ADSR Filter"));

        m_main_layout->lms_add_widget(m_adsr_filter->lms_groupbox_adsr->lms_groupbox);

        connect(m_adsr_filter->lms_attack->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(filterAttackChanged(int)));
        connect(m_adsr_filter->lms_decay->lms_knob,   SIGNAL(valueChanged(int)), this, SLOT(filterDecayChanged(int))); 
        connect(m_adsr_filter->lms_sustain->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(filterSustainChanged(int)));
        connect(m_adsr_filter->lms_release->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(filterReleaseChanged(int)));
        
        m_filter = new LMS_filter_widget(this, f_info, LMS_TIMBRE, LMS_RES, LMS_FILTER_TYPE, TRUE);

        m_main_layout->lms_add_widget(m_filter->lms_groupbox->lms_groupbox);
        
        connect(m_filter->lms_cutoff_knob->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(timbreChanged(int)));
        connect(m_filter->lms_res_knob->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(resChanged(int)));
        connect(m_filter->lms_filter_type->lms_combobox,  SIGNAL(currentIndexChanged(int)), this, SLOT(filterTypeChanged(int)));

        m_filter_env_amt  = new LMS_knob_regular(QString("Env Amt"), -36, 36, 1, 0, QString("0"), m_filter->lms_groupbox->lms_groupbox, f_info, lms_kc_integer, LMS_FILTER_ENV_AMT);
        m_filter->lms_groupbox->lms_add_h(m_filter_env_amt);
        connect(m_filter_env_amt->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(filterEnvAmtChanged(int)));

        m_main_layout->lms_add_layout();

        m_master = new LMS_master_widget(this, f_info, LMS_MASTER_VOLUME, -1, 
                -1, LMS_MASTER_GLIDE, LMS_MASTER_PITCHBEND_AMT, QString("Master"), FALSE);
        m_main_layout->lms_add_widget(m_master->lms_groupbox->lms_groupbox);    
                
        connect(m_master->lms_master_volume->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(masterVolumeChanged(int)));
        connect(m_master->lms_master_glide->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(masterGlideChanged(int)));    
        connect(m_master->lms_master_pitchbend_amt->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(masterPitchbendAmtChanged(int)));

        m_pitch_env = new LMS_ramp_env(this, f_info, LMS_PITCH_ENV_TIME, LMS_PITCH_ENV_AMT, -1, FALSE, QString("Pitch Env"));
        m_main_layout->lms_add_widget(m_pitch_env->lms_groupbox->lms_groupbox);

        connect(m_pitch_env->lms_amt_knob->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(pitchEnvAmtChanged(int)));
        connect(m_pitch_env->lms_time_knob->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(pitchEnvTimeChanged(int)));

        m_lfo = new LMS_lfo_widget(this, f_info, LMS_LFO_FREQ, LMS_LFO_TYPE, f_lfo_types, QString("LFO"));
        m_main_layout->lms_add_widget(m_lfo->lms_groupbox->lms_groupbox);

        connect(m_lfo->lms_freq_knob->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(LFOfreqChanged(int)));
        connect(m_lfo->lms_type_combobox->lms_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(LFOtypeChanged(int)));

        m_lfo_amp  = new LMS_knob_regular(QString("Amp"), -24, 24, 1, 0, QString("0"), m_lfo->lms_groupbox->lms_groupbox, f_info, lms_kc_integer, LMS_LFO_AMP);
        m_lfo->lms_groupbox->lms_add_h(m_lfo_amp);
        connect(m_lfo_amp->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(LFOampChanged(int)));

        m_lfo_pitch = new LMS_knob_regular(QString("Pitch"), -36, 36, 1, 0, QString("0"), m_lfo->lms_groupbox->lms_groupbox, f_info, lms_kc_integer, LMS_LFO_PITCH);
        m_lfo->lms_groupbox->lms_add_h(m_lfo_pitch);
        connect(m_lfo_pitch->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(LFOpitchChanged(int)));

        m_lfo_cutoff  = new LMS_knob_regular(QString("Filter"), -48, 48, 1, 0, QString("0"), m_lfo->lms_groupbox->lms_groupbox, f_info, lms_kc_integer, LMS_LFO_FILTER);
        m_lfo->lms_groupbox->lms_add_h(m_lfo_cutoff);
        connect(m_lfo_cutoff->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(LFOcutoffChanged(int)));    
        
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
    
    //cerr << files_string;
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
                                        
                    stream << i << LMS_DELIMITER << f_dir.relativeFilePath(m_sample_table->lms_mod_matrix->item(i, SMP_TB_FILE_PATH_INDEX)->text()) << "\n";    
                }
                else
                {
                    stream << i << LMS_DELIMITER << m_sample_table->lms_mod_matrix->item(i, SMP_TB_FILE_PATH_INDEX)->text() << "\n";     
                }
            }
            
            stream << LMS_FILE_CONTROLS_TAG << "\n";
            stream << LMS_FILE_CONTROLS_TAG_EUP_V1 << "\n";
            
            for(int i = LMS_FIRST_CONTROL_PORT; i < Sampler_Stereo_COUNT; i++)        
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
        
        for(int i = 0; i < LMS_MAX_SAMPLE_COUNT; i++)
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
                    QStringList file_arr = line.split(LMS_DELIMITER);
                    
                    if(file_arr.count() != 2)
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
                            m_sample_table->lms_mod_matrix->setItem(file_arr.at(0).toInt(), SMP_TB_FILE_PATH_INDEX, f_item);   
                        }
                        else
                        {
                            QString f_full_path = file_arr.at(1);
                            
                            if(f_use_path_prefix)
                                    f_full_path = QString(f_path_prefix + "/" + file_arr.at(1));
                            
                            if(QFile::exists(f_full_path))
                            {
                                cerr << "Setting " << file_arr.at(0) << " to " << f_full_path;
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

/*End synth_qt_gui.cpp Autogenerated slots*/

//Begin Ray-V PolyFX

void SamplerGUI::lms_set_value(float val, LMS_control * a_ctrl )
{    
    m_suppressHostUpdate = true;
    a_ctrl->lms_set_value(int(val));
    m_suppressHostUpdate = false;     
}


void SamplerGUI::setAttack(float a_value){ lms_set_value(a_value, m_adsr_amp->lms_attack);}
void SamplerGUI::setDecay(float a_value){ lms_set_value(a_value, m_adsr_amp->lms_decay); }
void SamplerGUI::setSustain(float a_value){lms_set_value(a_value, m_adsr_amp->lms_sustain);}
void SamplerGUI::setRelease(float a_value){lms_set_value(a_value, m_adsr_amp->lms_release);}
void SamplerGUI::setTimbre(float a_value){lms_set_value(a_value, m_filter->lms_cutoff_knob);}
void SamplerGUI::setFilterType(float a_value){lms_set_value(a_value, m_filter->lms_filter_type);}
void SamplerGUI::setRes(float a_value){lms_set_value(a_value, m_filter->lms_res_knob);}
void SamplerGUI::setDist(float a_value){lms_set_value(a_value, m_dist);}
void SamplerGUI::setFilterAttack (float a_value){lms_set_value(a_value, m_adsr_filter->lms_attack);}
void SamplerGUI::setFilterDecay  (float a_value){lms_set_value(a_value, m_adsr_filter->lms_decay);}
void SamplerGUI::setFilterSustain(float a_value){lms_set_value(a_value, m_adsr_filter->lms_sustain);}
void SamplerGUI::setFilterRelease(float a_value){lms_set_value(a_value, m_adsr_filter->lms_release);}
void SamplerGUI::setNoiseAmp(float a_value){lms_set_value(a_value, m_noise_amp);}
void SamplerGUI::setFilterEnvAmt(float a_value){lms_set_value(a_value, m_filter_env_amt);}
void SamplerGUI::setDistWet(float a_value){lms_set_value(a_value, m_dist_wet);}
void SamplerGUI::setMasterVolume(float a_value){lms_set_value(a_value, m_master->lms_master_volume);}
void SamplerGUI::setMasterUnisonVoices(float a_value){lms_set_value(a_value, m_master->lms_master_unison_voices);}
void SamplerGUI::setMasterUnisonSpread(float a_value){lms_set_value(a_value, m_master->lms_master_unison_spread);}
void SamplerGUI::setMasterGlide(float a_value){lms_set_value(a_value, m_master->lms_master_glide);}
void SamplerGUI::setMasterPitchbendAmt(float a_value){lms_set_value(a_value, m_master->lms_master_pitchbend_amt);}
void SamplerGUI::setPitchEnvAmt(float a_value){lms_set_value(a_value, m_pitch_env->lms_amt_knob);}
void SamplerGUI::setPitchEnvTime(float a_value){lms_set_value(a_value, m_pitch_env->lms_time_knob);}
void SamplerGUI::setLFOfreq(float a_value){lms_set_value(a_value, m_lfo->lms_freq_knob);}
void SamplerGUI::setLFOtype(float a_value){lms_set_value(a_value, m_lfo->lms_type_combobox);}
void SamplerGUI::setLFOamp(float a_value){lms_set_value(a_value, m_lfo_amp);}
void SamplerGUI::setLFOpitch(float a_value){lms_set_value(a_value, m_lfo_pitch);}
void SamplerGUI::setLFOcutoff(float a_value){lms_set_value(a_value, m_lfo_cutoff);}

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
void SamplerGUI::timbreChanged(int a_value){lms_value_changed(a_value, m_filter->lms_cutoff_knob);}
void SamplerGUI::filterTypeChanged(int a_value){lms_value_changed(a_value, m_filter->lms_filter_type);}
void SamplerGUI::resChanged(int a_value){lms_value_changed(a_value, m_filter->lms_res_knob);}
void SamplerGUI::distChanged(int a_value){lms_value_changed(a_value, m_dist);}
void SamplerGUI::filterAttackChanged(int a_value){lms_value_changed(a_value, m_adsr_filter->lms_attack);}
void SamplerGUI::filterDecayChanged(int a_value){lms_value_changed(a_value, m_adsr_filter->lms_decay);}
void SamplerGUI::filterSustainChanged(int a_value){lms_value_changed(a_value, m_adsr_filter->lms_sustain);}
void SamplerGUI::filterReleaseChanged(int a_value){lms_value_changed(a_value, m_adsr_filter->lms_release);}
void SamplerGUI::noiseAmpChanged(int a_value){lms_value_changed(a_value, m_noise_amp);}
void SamplerGUI::filterEnvAmtChanged(int a_value){lms_value_changed(a_value, m_filter_env_amt);}
void SamplerGUI::distWetChanged(int a_value){lms_value_changed(a_value, m_dist_wet);}
void SamplerGUI::masterVolumeChanged(int a_value){lms_value_changed(a_value, m_master->lms_master_volume);}
void SamplerGUI::masterUnisonVoicesChanged(int a_value){lms_value_changed(a_value, m_master->lms_master_unison_voices);}
void SamplerGUI::masterUnisonSpreadChanged(int a_value){lms_value_changed(a_value, m_master->lms_master_unison_spread);}
void SamplerGUI::masterGlideChanged(int a_value){lms_value_changed(a_value, m_master->lms_master_glide);}
void SamplerGUI::masterPitchbendAmtChanged(int a_value){lms_value_changed(a_value, m_master->lms_master_pitchbend_amt);}
void SamplerGUI::pitchEnvAmtChanged(int a_value){lms_value_changed(a_value, m_pitch_env->lms_amt_knob);}
void SamplerGUI::pitchEnvTimeChanged(int a_value){lms_value_changed(a_value, m_pitch_env->lms_time_knob);}
void SamplerGUI::LFOfreqChanged(int a_value){lms_value_changed(a_value, m_lfo->lms_freq_knob);}
void SamplerGUI::LFOtypeChanged(int a_value){lms_value_changed(a_value, m_lfo->lms_type_combobox);}
void SamplerGUI::LFOampChanged(int a_value){lms_value_changed(a_value, m_lfo_amp);}
void SamplerGUI::LFOpitchChanged(int a_value){lms_value_changed(a_value, m_lfo_pitch);}
void SamplerGUI::LFOcutoffChanged(int a_value){lms_value_changed(a_value, m_lfo_cutoff);}


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

    if((port < LMS_SAMPLE_PITCH_PORT_RANGE_MIN) && (port < LMS_SAMPLE_PITCH_PORT_RANGE_MIN))
    {
        switch (port) {
            case Sampler_SELECTED_SAMPLE: setSelection(a_value); break;
            case LMS_ATTACK: setAttack(a_value); break;
            case LMS_DECAY: setDecay(a_value); break;
            case LMS_SUSTAIN: setSustain(a_value); break;
            case LMS_RELEASE: setRelease(a_value); break;
            case LMS_TIMBRE: setTimbre(a_value); break;
            case LMS_RES: setRes(a_value); break;
            case LMS_FILTER_TYPE: setFilterType(a_value); break;
            case LMS_DIST: setDist(a_value); break;
            case LMS_FILTER_ATTACK: setFilterAttack(a_value); break;
            case LMS_FILTER_DECAY: setFilterDecay(a_value); break;
            case LMS_FILTER_SUSTAIN: setFilterSustain(a_value); break;
            case LMS_FILTER_RELEASE: setFilterRelease(a_value); break;
            case LMS_NOISE_AMP: setNoiseAmp(a_value); break;    
            case LMS_DIST_WET: setDistWet(a_value); break;
            case LMS_FILTER_ENV_AMT: setFilterEnvAmt(a_value); break;    
            case LMS_MASTER_VOLUME: setMasterVolume(a_value); break;    
            //case LMS_MASTER_UNISON_VOICES: setMasterUnisonVoices(a_value); break;
            //case LMS_MASTER_UNISON_SPREAD: setMasterUnisonSpread(a_value); break;
            case LMS_MASTER_GLIDE: setMasterGlide(a_value); break;
            case LMS_MASTER_PITCHBEND_AMT: setMasterPitchbendAmt(a_value); break;
            case LMS_PITCH_ENV_AMT: setPitchEnvAmt(a_value); break;
            case LMS_PITCH_ENV_TIME: setPitchEnvTime(a_value); break;                
            case LMS_LFO_FREQ: setLFOfreq(a_value); break;            
            case LMS_LFO_TYPE:  setLFOtype(a_value);  break;            
            case LMS_LFO_AMP: setLFOamp(a_value); break;            
            case LMS_LFO_PITCH: setLFOpitch(a_value); break;            
            case LMS_LFO_FILTER: setLFOcutoff(a_value); break;
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
            case LMS_TIMBRE: timbreChanged(a_value); break;
            case LMS_RES: resChanged(a_value); break;
            case LMS_FILTER_TYPE: filterTypeChanged(a_value); break;
            case LMS_DIST: distChanged(a_value); break;
            case LMS_FILTER_ATTACK: filterAttackChanged(a_value); break;
            case LMS_FILTER_DECAY: filterDecayChanged(a_value); break;
            case LMS_FILTER_SUSTAIN: filterSustainChanged(a_value); break;
            case LMS_FILTER_RELEASE: filterReleaseChanged(a_value); break;
            case LMS_NOISE_AMP: noiseAmpChanged(a_value); break;    
            case LMS_DIST_WET: distWetChanged(a_value); break;
            case LMS_FILTER_ENV_AMT: filterEnvAmtChanged(a_value); break;        
            case LMS_MASTER_VOLUME: masterVolumeChanged(a_value); break;
            //case LMS_MASTER_UNISON_VOICES: masterUnisonVoicesChanged(a_value); break;
            //case LMS_MASTER_UNISON_SPREAD: masterUnisonSpreadChanged(a_value); break;
            case LMS_MASTER_GLIDE: masterGlideChanged(a_value); break;
            case LMS_MASTER_PITCHBEND_AMT: masterPitchbendAmtChanged(a_value); break;
            case LMS_PITCH_ENV_AMT: pitchEnvAmtChanged(a_value); break;
            case LMS_PITCH_ENV_TIME: pitchEnvTimeChanged(a_value); break;
            case LMS_LFO_FREQ: LFOfreqChanged(a_value); break;
            case LMS_LFO_TYPE: LFOtypeChanged(a_value); break;
            case LMS_LFO_AMP: LFOampChanged(a_value); break;
            case LMS_LFO_PITCH: LFOpitchChanged(a_value); break;
            case LMS_LFO_FILTER: LFOcutoffChanged(a_value); break;            
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
    else
    {
        cerr << "v_control_changed called with invalid port " << port << "\n";
    }
    
    if(a_suppress_host_update)
        m_suppressHostUpdate = false;
    
}

/*TODO:  For the forseeable future, this will only be used for getting the values to write back to 
 the presets.tsv file;  It should probably return a string that can be re-interpreted into other values for
 complex controls that could have multiple ints, or string values, etc...*/
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
            case LMS_TIMBRE: return  m_filter->lms_cutoff_knob->lms_get_value();
            case LMS_RES: return m_filter->lms_res_knob->lms_get_value();  
            case LMS_FILTER_TYPE:return m_filter->lms_filter_type->lms_get_value();
            case LMS_DIST: return m_dist->lms_get_value();
            case LMS_FILTER_ATTACK: return m_adsr_filter->lms_attack->lms_get_value();
            case LMS_FILTER_DECAY: return m_adsr_filter->lms_decay->lms_get_value();
            case LMS_FILTER_SUSTAIN: return m_adsr_filter->lms_sustain->lms_get_value();
            case LMS_FILTER_RELEASE: return m_adsr_filter->lms_release->lms_get_value();
            case LMS_NOISE_AMP: return m_noise_amp->lms_get_value();
            case LMS_DIST_WET: return m_dist_wet->lms_get_value();
            case LMS_FILTER_ENV_AMT: return m_filter_env_amt->lms_get_value();
            case LMS_MASTER_VOLUME: return m_master->lms_master_volume->lms_get_value();
            case LMS_MASTER_GLIDE: return m_master->lms_master_glide->lms_get_value();
            case LMS_MASTER_PITCHBEND_AMT: return m_master->lms_master_pitchbend_amt->lms_get_value();
            case LMS_PITCH_ENV_AMT: return m_pitch_env->lms_amt_knob->lms_get_value();
            case LMS_PITCH_ENV_TIME: return m_pitch_env->lms_time_knob->lms_get_value();
            case LMS_LFO_FREQ: return m_lfo->lms_freq_knob->lms_get_value();
            case LMS_LFO_TYPE: return m_lfo->lms_type_combobox->lms_get_value();
            case LMS_LFO_AMP: return m_lfo_amp->lms_get_value();
            case LMS_LFO_PITCH: return m_lfo_pitch->lms_get_value();
            case LMS_LFO_FILTER: return m_lfo_cutoff->lms_get_value();
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

