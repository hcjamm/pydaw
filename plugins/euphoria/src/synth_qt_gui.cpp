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
#include "synth.h"

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
#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <math.h>
#include <sndfile.h>

#include "dssi.h"

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/SM/SMlib.h>
#include <qt4/QtGui/qapplication.h>
#include <qt4/QtCore/qstring.h>

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

using std::endl;

lo_server osc_server = 0;

static QTextStream cerr(stderr);

#define NO_SAMPLE_TEXT "<none loaded>   "
/*For sliders that calculate as a percentage, like sample start/end or loop start/end*/
#define SLIDER_LENGTH 10000
#define SLIDER_LENGTH_RECIP 1/SLIDER_LENGTH

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
    
    /*
     * Example of how to setup a filebrowser treeview
     * 
    #include <QtGui/QApplication>
    #include <QtGui/QDirModel>
    #include <QtGui/QTreeView>

    int main(int argc, char *argv[])
    {
        QApplication app(argc, argv);

        QDirModel model;
        QTreeView tree;

        tree.setModel(&model);

        tree.setRootIndex(model.index(QDir::homePath()));
        tree.setColumnHidden( 1, true );
        tree.setColumnHidden( 2, true );
        tree.setColumnHidden( 3, true );

        tree.setWindowTitle(QObject::tr("Dir View:")+QDir::homePath());
        tree.resize(640, 480);
        tree.show();

        return app.exec();
    }
     */
    
    /* Example of QSignalMapper from here:  http://stackoverflow.com/questions/1332110/selecting-qcombobox-in-qtablewidget
     * 

    Here's a modification of the QSignalMapper documentation to fit your situation:

    QSignalMapper* signalMapper = new QSignalMapper(this);

    for (each row in table) {
        QComboBox* combo = new QComboBox();
        table->setCellWidget(row,col,combo);                         
        combo->setCurrentIndex(node.type()); 
        connect(combo, SIGNAL(currentIndexChanged(int)), signalMapper, SLOT(map()));
        signalMapper->setMapping(combo, QString("%1-%2).arg(row).arg(col));
    }

    connect(signalMapper, SIGNAL(mapped(const QString &)),
            this, SIGNAL(changed(const QString &)));

    In the handler function ::changed(QString position):

    QStringList coordinates = position.split("-");
    int row = coordinates[0].toInt();
    int col = coordinates[1].toInt();
    QComboBox* combo=(QComboBox*)table->cellWidget(row, col);  
    combo->currentIndex()

    Note that a QString is a pretty clumsy way to pass this information. A better choice would be a new QModelIndex that you pass, and which the changed function would then delete.

    The downside to this solution is that you lose the value that currentIndexChanged emits, but you can query the QComboBox for its index from ::changed.

     */
    
    m_host = lo_address_new(host, port);
    
    
        if (this->objectName().isEmpty())
        this->setObjectName(QString::fromUtf8("Frame"));
        this->resize(1023, 801);
        this->setFrameShape(QFrame::StyledPanel);
        this->setFrameShadow(QFrame::Raised);
        horizontalLayout_5 = new QHBoxLayout(this);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        m_main_tab = new QTabWidget(this);
        m_main_tab->setObjectName(QString::fromUtf8("m_main_tab"));
        m_main_tab->setStyleSheet(QString::fromUtf8(""));
        tab_4 = new QWidget();
        tab_4->setObjectName(QString::fromUtf8("tab_4"));
        horizontalLayout_2 = new QHBoxLayout(tab_4);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        scrollArea = new QScrollArea(tab_4);
        scrollArea->setObjectName(QString::fromUtf8("scrollArea"));
        scrollArea->setStyleSheet(QString::fromUtf8(""));
        scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scrollArea->setWidgetResizable(true);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName(QString::fromUtf8("scrollAreaWidgetContents"));
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 963, 904));
        horizontalLayout = new QHBoxLayout(scrollAreaWidgetContents);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        verticalLayout_20 = new QVBoxLayout();
        verticalLayout_20->setObjectName(QString::fromUtf8("verticalLayout_20"));
        m_sample_table = new QTableWidget(scrollAreaWidgetContents);
        if (m_sample_table->columnCount() < 17)
            m_sample_table->setColumnCount(17);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        m_sample_table->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        m_sample_table->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        m_sample_table->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        m_sample_table->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        m_sample_table->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        m_sample_table->setHorizontalHeaderItem(5, __qtablewidgetitem5);
        QTableWidgetItem *__qtablewidgetitem6 = new QTableWidgetItem();
        m_sample_table->setHorizontalHeaderItem(6, __qtablewidgetitem6);
        QTableWidgetItem *__qtablewidgetitem7 = new QTableWidgetItem();
        m_sample_table->setHorizontalHeaderItem(7, __qtablewidgetitem7);
        QTableWidgetItem *__qtablewidgetitem8 = new QTableWidgetItem();
        m_sample_table->setHorizontalHeaderItem(8, __qtablewidgetitem8);
        QTableWidgetItem *__qtablewidgetitem9 = new QTableWidgetItem();
        m_sample_table->setHorizontalHeaderItem(9, __qtablewidgetitem9);
        QTableWidgetItem *__qtablewidgetitem10 = new QTableWidgetItem();
        m_sample_table->setHorizontalHeaderItem(10, __qtablewidgetitem10);
        QTableWidgetItem *__qtablewidgetitem11 = new QTableWidgetItem();
        m_sample_table->setHorizontalHeaderItem(11, __qtablewidgetitem11);
        QTableWidgetItem *__qtablewidgetitem12 = new QTableWidgetItem();
        m_sample_table->setHorizontalHeaderItem(12, __qtablewidgetitem12);
        QTableWidgetItem *__qtablewidgetitem13 = new QTableWidgetItem();
        m_sample_table->setHorizontalHeaderItem(13, __qtablewidgetitem13);
        QTableWidgetItem *__qtablewidgetitem14 = new QTableWidgetItem();
        m_sample_table->setHorizontalHeaderItem(14, __qtablewidgetitem14);
        QTableWidgetItem *__qtablewidgetitem15 = new QTableWidgetItem();
        m_sample_table->setHorizontalHeaderItem(15, __qtablewidgetitem15);
        QTableWidgetItem *__qtablewidgetitem16 = new QTableWidgetItem();
        m_sample_table->setHorizontalHeaderItem(16, __qtablewidgetitem16);
        if (m_sample_table->rowCount() < LMS_MAX_SAMPLE_COUNT)
            m_sample_table->setRowCount(LMS_MAX_SAMPLE_COUNT);
        m_sample_table->setObjectName(QString::fromUtf8("m_sample_table"));
        m_sample_table->setMinimumSize(QSize(0, 300));
        m_sample_table->setMaximumSize(QSize(1920, 800));
        m_sample_table->setRowCount(LMS_MAX_SAMPLE_COUNT);
        m_sample_table->horizontalHeader()->setCascadingSectionResizes(true);
        m_sample_table->horizontalHeader()->setDefaultSectionSize(60);
        m_sample_table->horizontalHeader()->setMinimumSectionSize(42);
        m_sample_table->horizontalHeader()->setStretchLastSection(true);

        /*Set all of the array variables that are per-sample*/
        for(int i = 0; i < LMS_MAX_SAMPLE_COUNT; i++)        
        {
            m_selected_sample[i] = new QRadioButton(this);
            if(i == 0)
            {
                m_selected_sample[0]->setChecked(true);
            }
            
            connect(m_selected_sample[i], SIGNAL(clicked()), this, SLOT(selectionChanged()));
            m_sample_table->setCellWidget(i, 0, m_selected_sample[i]);
            
            QPixmap pmap(m_previewWidth, m_previewHeight);
            pmap.fill();
            
            m_sample_graphs[i] = pmap;
            
            m_note_indexes[i] = 0;
            m_sample_counts[i] = 0;
        }
        
        verticalLayout_20->addWidget(m_sample_table);

        horizontalLayout_16 = new QHBoxLayout();
        horizontalLayout_16->setObjectName(QString::fromUtf8("horizontalLayout_16"));
        verticalLayout_21 = new QVBoxLayout();
        verticalLayout_21->setObjectName(QString::fromUtf8("verticalLayout_21"));
        horizontalLayout_17 = new QHBoxLayout();
        horizontalLayout_17->setObjectName(QString::fromUtf8("horizontalLayout_17"));
        m_file_path = new QLineEdit(scrollAreaWidgetContents);
        m_file_path->setObjectName(QString::fromUtf8("m_file_path"));
        m_file_path->setReadOnly(true);

        horizontalLayout_17->addWidget(m_file_path);

        m_load_sample = new QPushButton(scrollAreaWidgetContents);
        m_load_sample->setObjectName(QString::fromUtf8("m_load_sample"));

        horizontalLayout_17->addWidget(m_load_sample);


        verticalLayout_21->addLayout(horizontalLayout_17);

        horizontalLayout_18 = new QHBoxLayout();
        horizontalLayout_18->setObjectName(QString::fromUtf8("horizontalLayout_18"));
        horizontalSpacer_9 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_18->addItem(horizontalSpacer_9);

        gridLayout_6 = new QGridLayout();
        gridLayout_6->setObjectName(QString::fromUtf8("gridLayout_6"));
        m_lnote = new QSpinBox(scrollAreaWidgetContents);
        m_lnote->setObjectName(QString::fromUtf8("m_lnote"));
        m_lnote->setMinimum(1);
        m_lnote->setMaximum(127);

        gridLayout_6->addWidget(m_lnote, 2, 3, 1, 1);

        m_sample_numLabel_6 = new QLabel(scrollAreaWidgetContents);
        m_sample_numLabel_6->setObjectName(QString::fromUtf8("m_sample_numLabel_6"));

        gridLayout_6->addWidget(m_sample_numLabel_6, 1, 0, 1, 1);

        m_hvelLabel_10 = new QLabel(scrollAreaWidgetContents);
        m_hvelLabel_10->setObjectName(QString::fromUtf8("m_hvelLabel_10"));

        gridLayout_6->addWidget(m_hvelLabel_10, 1, 7, 1, 1);

        m_lnoteLabel_5 = new QLabel(scrollAreaWidgetContents);
        m_lnoteLabel_5->setObjectName(QString::fromUtf8("m_lnoteLabel_5"));

        gridLayout_6->addWidget(m_lnoteLabel_5, 1, 3, 1, 1);

        m_sample_vol = new QSpinBox(scrollAreaWidgetContents);
        m_sample_vol->setObjectName(QString::fromUtf8("m_sample_vol"));
        m_sample_vol->setMinimum(-50);
        m_sample_vol->setMaximum(36);
        m_sample_vol->setValue(-6);

        gridLayout_6->addWidget(m_sample_vol, 2, 7, 1, 1);

        m_note = new QComboBox(scrollAreaWidgetContents);
        m_note->setObjectName(QString::fromUtf8("m_note"));
        m_note->setMaxVisibleItems(12);

        gridLayout_6->addWidget(m_note, 2, 1, 1, 1);

        m_lvel = new QSpinBox(scrollAreaWidgetContents);
        m_lvel->setObjectName(QString::fromUtf8("m_lvel"));
        m_lvel->setMinimum(1);
        m_lvel->setMaximum(127);

        gridLayout_6->addWidget(m_lvel, 2, 5, 1, 1);

        m_hnote = new QSpinBox(scrollAreaWidgetContents);
        m_hnote->setObjectName(QString::fromUtf8("m_hnote"));
        m_hnote->setMinimum(1);
        m_hnote->setMaximum(127);
        m_hnote->setValue(127);

        gridLayout_6->addWidget(m_hnote, 2, 4, 1, 1);

        m_noteLabel_8 = new QLabel(scrollAreaWidgetContents);
        m_noteLabel_8->setObjectName(QString::fromUtf8("m_noteLabel_8"));

        gridLayout_6->addWidget(m_noteLabel_8, 1, 1, 1, 1);

        m_noteLabel_9 = new QLabel(scrollAreaWidgetContents);
        m_noteLabel_9->setObjectName(QString::fromUtf8("m_noteLabel_9"));

        gridLayout_6->addWidget(m_noteLabel_9, 1, 8, 1, 1);

        m_play_mode = new QComboBox(scrollAreaWidgetContents);
        m_play_mode->setObjectName(QString::fromUtf8("m_play_mode"));
        m_play_mode->setMaxVisibleItems(12);

        gridLayout_6->addWidget(m_play_mode, 2, 8, 1, 1);

        m_sample_num = new QSpinBox(scrollAreaWidgetContents);
        m_sample_num->setObjectName(QString::fromUtf8("m_sample_num"));
        m_sample_num->setMinimum(0);
        m_sample_num->setMaximum(32);
        m_sample_num->setValue(0);

        gridLayout_6->addWidget(m_sample_num, 2, 0, 1, 1);

        m_octave = new QSpinBox(scrollAreaWidgetContents);
        m_octave->setObjectName(QString::fromUtf8("m_octave"));
        m_octave->setMinimum(-2);
        m_octave->setMaximum(8);
        m_octave->setValue(3);

        gridLayout_6->addWidget(m_octave, 2, 2, 1, 1);

        m_octaveLabel_5 = new QLabel(scrollAreaWidgetContents);
        m_octaveLabel_5->setObjectName(QString::fromUtf8("m_octaveLabel_5"));

        gridLayout_6->addWidget(m_octaveLabel_5, 1, 2, 1, 1);

        m_lvelLabel_5 = new QLabel(scrollAreaWidgetContents);
        m_lvelLabel_5->setObjectName(QString::fromUtf8("m_lvelLabel_5"));

        gridLayout_6->addWidget(m_lvelLabel_5, 1, 5, 1, 1);

        m_hvelLabel_9 = new QLabel(scrollAreaWidgetContents);
        m_hvelLabel_9->setObjectName(QString::fromUtf8("m_hvelLabel_9"));

        gridLayout_6->addWidget(m_hvelLabel_9, 1, 6, 1, 1);

        m_hnoteLabel_5 = new QLabel(scrollAreaWidgetContents);
        m_hnoteLabel_5->setObjectName(QString::fromUtf8("m_hnoteLabel_5"));

        gridLayout_6->addWidget(m_hnoteLabel_5, 1, 4, 1, 1);

        m_hvel = new QSpinBox(scrollAreaWidgetContents);
        m_hvel->setObjectName(QString::fromUtf8("m_hvel"));
        m_hvel->setMinimum(1);
        m_hvel->setMaximum(127);
        m_hvel->setValue(127);

        gridLayout_6->addWidget(m_hvel, 2, 6, 1, 1);

        m_noteLabel_10 = new QLabel(scrollAreaWidgetContents);
        m_noteLabel_10->setObjectName(QString::fromUtf8("m_noteLabel_10"));

        gridLayout_6->addWidget(m_noteLabel_10, 1, 9, 1, 1);

        m_sample_fx_group = new QComboBox(scrollAreaWidgetContents);
        m_sample_fx_group->setObjectName(QString::fromUtf8("m_sample_fx_group"));
        m_sample_fx_group->setMaxVisibleItems(12);

        gridLayout_6->addWidget(m_sample_fx_group, 2, 9, 1, 1);


        horizontalLayout_18->addLayout(gridLayout_6);

        m_update_sample = new QPushButton(scrollAreaWidgetContents);
        m_update_sample->setObjectName(QString::fromUtf8("m_update_sample"));

        horizontalLayout_18->addWidget(m_update_sample);


        verticalLayout_21->addLayout(horizontalLayout_18);


        horizontalLayout_16->addLayout(verticalLayout_21);

        horizontalSpacer_10 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_16->addItem(horizontalSpacer_10);


        verticalLayout_20->addLayout(horizontalLayout_16);

        verticalLayout_22 = new QVBoxLayout();
        verticalLayout_22->setObjectName(QString::fromUtf8("verticalLayout_22"));
        m_sample_graph = new QLabel(scrollAreaWidgetContents);
        m_sample_graph->setObjectName(QString::fromUtf8("m_sample_graph"));
        m_sample_graph->setMinimumSize(QSize(0, 200));
        m_sample_graph->setStyleSheet(QString::fromUtf8("QLabel {background-color: white;};"));
        m_sample_graph->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);

        verticalLayout_22->addWidget(m_sample_graph);


        verticalLayout_20->addLayout(verticalLayout_22);

        gridLayout_10 = new QGridLayout();
        gridLayout_10->setObjectName(QString::fromUtf8("gridLayout_10"));
        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        label_21 = new QLabel(scrollAreaWidgetContents);
        label_21->setObjectName(QString::fromUtf8("label_21"));

        horizontalLayout_6->addWidget(label_21);

        m_sample_start_fine = new QSpinBox(scrollAreaWidgetContents);
        m_sample_start_fine->setObjectName(QString::fromUtf8("m_sample_start_fine"));
        m_sample_start_fine->setMaximumSize(QSize(100, 16777215));
        m_sample_start_fine->setMaximum(100000);

        horizontalLayout_6->addWidget(m_sample_start_fine);

        label_22 = new QLabel(scrollAreaWidgetContents);
        label_22->setObjectName(QString::fromUtf8("label_22"));

        horizontalLayout_6->addWidget(label_22);

        m_sample_end_fine = new QSpinBox(scrollAreaWidgetContents);
        m_sample_end_fine->setObjectName(QString::fromUtf8("m_sample_end_fine"));
        m_sample_end_fine->setMaximumSize(QSize(100, 16777215));
        m_sample_end_fine->setMaximum(100000);

        horizontalLayout_6->addWidget(m_sample_end_fine);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_5);


        gridLayout_10->addLayout(horizontalLayout_6, 3, 0, 1, 1);

        m_sample_start = new QSlider(scrollAreaWidgetContents);
        m_sample_start->setObjectName(QString::fromUtf8("m_sample_start"));
        m_sample_start->setMaximum(10000);
        m_sample_start->setOrientation(Qt::Horizontal);

        gridLayout_10->addWidget(m_sample_start, 1, 0, 1, 1);

        label_23 = new QLabel(scrollAreaWidgetContents);
        label_23->setObjectName(QString::fromUtf8("label_23"));

        gridLayout_10->addWidget(label_23, 0, 0, 1, 1);

        m_sample_end = new QSlider(scrollAreaWidgetContents);
        m_sample_end->setObjectName(QString::fromUtf8("m_sample_end"));
        m_sample_end->setMaximum(10000);
        m_sample_end->setSliderPosition(10000);
        m_sample_end->setOrientation(Qt::Horizontal);

        gridLayout_10->addWidget(m_sample_end, 2, 0, 1, 1);


        verticalLayout_20->addLayout(gridLayout_10);

        gridLayout_11 = new QGridLayout();
        gridLayout_11->setObjectName(QString::fromUtf8("gridLayout_11"));
        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        label_24 = new QLabel(scrollAreaWidgetContents);
        label_24->setObjectName(QString::fromUtf8("label_24"));

        horizontalLayout_7->addWidget(label_24);

        m_loop_start_fine = new QSpinBox(scrollAreaWidgetContents);
        m_loop_start_fine->setObjectName(QString::fromUtf8("m_loop_start_fine"));
        m_loop_start_fine->setMaximumSize(QSize(100, 16777215));
        m_loop_start_fine->setMaximum(100000);

        horizontalLayout_7->addWidget(m_loop_start_fine);

        label_25 = new QLabel(scrollAreaWidgetContents);
        label_25->setObjectName(QString::fromUtf8("label_25"));

        horizontalLayout_7->addWidget(label_25);

        m_loop_end_fine = new QSpinBox(scrollAreaWidgetContents);
        m_loop_end_fine->setObjectName(QString::fromUtf8("m_loop_end_fine"));
        m_loop_end_fine->setMaximumSize(QSize(100, 16777215));
        m_loop_end_fine->setMaximum(100000);

        horizontalLayout_7->addWidget(m_loop_end_fine);

        horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer_6);


        gridLayout_11->addLayout(horizontalLayout_7, 3, 0, 1, 1);

        m_loop_start = new QSlider(scrollAreaWidgetContents);
        m_loop_start->setObjectName(QString::fromUtf8("m_loop_start"));
        m_loop_start->setMaximum(10000);
        m_loop_start->setOrientation(Qt::Horizontal);

        gridLayout_11->addWidget(m_loop_start, 1, 0, 1, 1);

        label_26 = new QLabel(scrollAreaWidgetContents);
        label_26->setObjectName(QString::fromUtf8("label_26"));

        gridLayout_11->addWidget(label_26, 0, 0, 1, 1);

        m_loop_end = new QSlider(scrollAreaWidgetContents);
        m_loop_end->setObjectName(QString::fromUtf8("m_loop_end"));
        m_loop_end->setMaximum(10000);
        m_loop_end->setSliderPosition(10000);
        m_loop_end->setOrientation(Qt::Horizontal);

        gridLayout_11->addWidget(m_loop_end, 2, 0, 1, 1);


        verticalLayout_20->addLayout(gridLayout_11);


        horizontalLayout->addLayout(verticalLayout_20);

        scrollArea->setWidget(scrollAreaWidgetContents);

        horizontalLayout_2->addWidget(scrollArea);

        m_main_tab->addTab(tab_4, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QString::fromUtf8("tab_2"));
        horizontalLayout_4 = new QHBoxLayout(tab_2);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label = new QLabel(tab_2);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_3->addWidget(label);

        m_edit_fx_group_num = new QSpinBox(tab_2);
        m_edit_fx_group_num->setObjectName(QString::fromUtf8("m_edit_fx_group_num"));
        m_edit_fx_group_num->setMinimum(1);
        m_edit_fx_group_num->setMaximum(4);

        horizontalLayout_3->addWidget(m_edit_fx_group_num);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_3);


        verticalLayout->addLayout(horizontalLayout_3);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        groupBox_5 = new QGroupBox(tab_2);
        groupBox_5->setObjectName(QString::fromUtf8("groupBox_5"));
        groupBox_5->setStyleSheet(QString::fromUtf8("QGroupBox{border-color: rgb(0, 0, 127); background-color: white;};"));
        verticalLayout_6 = new QVBoxLayout(groupBox_5);
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        gridLayout_7 = new QGridLayout();
        gridLayout_7->setObjectName(QString::fromUtf8("gridLayout_7"));
        m_fx_knob_4_3 = new QDial(groupBox_5);
        m_fx_knob_4_3->setObjectName(QString::fromUtf8("m_fx_knob_4_3"));
        m_fx_knob_4_3->setMinimumSize(QSize(48, 48));
        m_fx_knob_4_3->setMaximumSize(QSize(48, 48));

        gridLayout_7->addWidget(m_fx_knob_4_3, 1, 2, 1, 1);

        m_fx_label_4_2 = new QLabel(groupBox_5);
        m_fx_label_4_2->setObjectName(QString::fromUtf8("m_fx_label_4_2"));

        gridLayout_7->addWidget(m_fx_label_4_2, 0, 1, 1, 1);

        label_19 = new QLabel(groupBox_5);
        label_19->setObjectName(QString::fromUtf8("label_19"));

        gridLayout_7->addWidget(label_19, 0, 3, 1, 1);

        m_fx_knob_4_2 = new QDial(groupBox_5);
        m_fx_knob_4_2->setObjectName(QString::fromUtf8("m_fx_knob_4_2"));
        m_fx_knob_4_2->setMinimumSize(QSize(48, 48));
        m_fx_knob_4_2->setMaximumSize(QSize(48, 48));

        gridLayout_7->addWidget(m_fx_knob_4_2, 1, 1, 1, 1);

        comboBox_5 = new QComboBox(groupBox_5);
        comboBox_5->setObjectName(QString::fromUtf8("comboBox_5"));
        comboBox_5->setMinimumSize(QSize(60, 0));
        comboBox_5->setMaximumSize(QSize(60, 16777215));

        gridLayout_7->addWidget(comboBox_5, 1, 3, 1, 1);

        m_fx_label_4_1 = new QLabel(groupBox_5);
        m_fx_label_4_1->setObjectName(QString::fromUtf8("m_fx_label_4_1"));

        gridLayout_7->addWidget(m_fx_label_4_1, 0, 0, 1, 1);

        m_fx_label_4_3 = new QLabel(groupBox_5);
        m_fx_label_4_3->setObjectName(QString::fromUtf8("m_fx_label_4_3"));

        gridLayout_7->addWidget(m_fx_label_4_3, 0, 2, 1, 1);

        m_fx_knob_4_1 = new QDial(groupBox_5);
        m_fx_knob_4_1->setObjectName(QString::fromUtf8("m_fx_knob_4_1"));
        m_fx_knob_4_1->setMinimumSize(QSize(48, 48));
        m_fx_knob_4_1->setMaximumSize(QSize(48, 48));
        m_fx_knob_4_1->setStyleSheet(QString::fromUtf8(""));

        gridLayout_7->addWidget(m_fx_knob_4_1, 1, 0, 1, 1);


        verticalLayout_6->addLayout(gridLayout_7);


        gridLayout->addWidget(groupBox_5, 1, 0, 1, 1);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_2, 1, 3, 1, 1);

        groupBox_2 = new QGroupBox(tab_2);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        groupBox_2->setStyleSheet(QString::fromUtf8("QGroupBox{border-color: rgb(0, 0, 127); background-color: white;};"));
        verticalLayout_2 = new QVBoxLayout(groupBox_2);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        gridLayout_3 = new QGridLayout();
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        m_fx_knob_1_3 = new QDial(groupBox_2);
        m_fx_knob_1_3->setObjectName(QString::fromUtf8("m_fx_knob_1_3"));
        m_fx_knob_1_3->setMinimumSize(QSize(48, 48));
        m_fx_knob_1_3->setMaximumSize(QSize(48, 48));

        gridLayout_3->addWidget(m_fx_knob_1_3, 1, 2, 1, 1);

        m_fx_label_1_2 = new QLabel(groupBox_2);
        m_fx_label_1_2->setObjectName(QString::fromUtf8("m_fx_label_1_2"));

        gridLayout_3->addWidget(m_fx_label_1_2, 0, 1, 1, 1);

        label_8 = new QLabel(groupBox_2);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        gridLayout_3->addWidget(label_8, 0, 3, 1, 1);

        m_fx_knob_1_2 = new QDial(groupBox_2);
        m_fx_knob_1_2->setObjectName(QString::fromUtf8("m_fx_knob_1_2"));
        m_fx_knob_1_2->setMinimumSize(QSize(48, 48));
        m_fx_knob_1_2->setMaximumSize(QSize(48, 48));

        gridLayout_3->addWidget(m_fx_knob_1_2, 1, 1, 1, 1);

        comboBox_2 = new QComboBox(groupBox_2);
        comboBox_2->setObjectName(QString::fromUtf8("comboBox_2"));
        comboBox_2->setMinimumSize(QSize(60, 0));
        comboBox_2->setMaximumSize(QSize(60, 16777215));

        gridLayout_3->addWidget(comboBox_2, 1, 3, 1, 1);

        m_fx_label_1_1 = new QLabel(groupBox_2);
        m_fx_label_1_1->setObjectName(QString::fromUtf8("m_fx_label_1_1"));

        gridLayout_3->addWidget(m_fx_label_1_1, 0, 0, 1, 1);

        m_fx_label_1_3 = new QLabel(groupBox_2);
        m_fx_label_1_3->setObjectName(QString::fromUtf8("m_fx_label_1_3"));

        gridLayout_3->addWidget(m_fx_label_1_3, 0, 2, 1, 1);

        m_fx_knob_1_1 = new QDial(groupBox_2);
        m_fx_knob_1_1->setObjectName(QString::fromUtf8("m_fx_knob_1_1"));
        m_fx_knob_1_1->setMinimumSize(QSize(48, 48));
        m_fx_knob_1_1->setMaximumSize(QSize(48, 48));
        m_fx_knob_1_1->setStyleSheet(QString::fromUtf8(""));

        gridLayout_3->addWidget(m_fx_knob_1_1, 1, 0, 1, 1);

        verticalLayout_2->addLayout(gridLayout_3);

        gridLayout->addWidget(groupBox_2, 0, 0, 1, 1);

        groupBox_3 = new QGroupBox(tab_2);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        groupBox_3->setStyleSheet(QString::fromUtf8("QGroupBox{border-color: rgb(0, 0, 127); background-color: white;};"));
        verticalLayout_3 = new QVBoxLayout(groupBox_3);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        gridLayout_4 = new QGridLayout();
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        m_fx_knob_2_3 = new QDial(groupBox_3);
        m_fx_knob_2_3->setObjectName(QString::fromUtf8("m_fx_knob_2_3"));
        m_fx_knob_2_3->setMinimumSize(QSize(48, 48));
        m_fx_knob_2_3->setMaximumSize(QSize(48, 48));

        gridLayout_4->addWidget(m_fx_knob_2_3, 1, 2, 1, 1);

        m_fx_label_2_2 = new QLabel(groupBox_3);
        m_fx_label_2_2->setObjectName(QString::fromUtf8("m_fx_label_2_2"));

        gridLayout_4->addWidget(m_fx_label_2_2, 0, 1, 1, 1);

        label_11 = new QLabel(groupBox_3);
        label_11->setObjectName(QString::fromUtf8("label_11"));

        gridLayout_4->addWidget(label_11, 0, 3, 1, 1);

        m_fx_knob_2_2 = new QDial(groupBox_3);
        m_fx_knob_2_2->setObjectName(QString::fromUtf8("m_fx_knob_2_2"));
        m_fx_knob_2_2->setMinimumSize(QSize(48, 48));
        m_fx_knob_2_2->setMaximumSize(QSize(48, 48));

        gridLayout_4->addWidget(m_fx_knob_2_2, 1, 1, 1, 1);

        comboBox_3 = new QComboBox(groupBox_3);
        comboBox_3->setObjectName(QString::fromUtf8("comboBox_3"));
        comboBox_3->setMinimumSize(QSize(60, 0));
        comboBox_3->setMaximumSize(QSize(60, 16777215));

        gridLayout_4->addWidget(comboBox_3, 1, 3, 1, 1);

        m_fx_label_2_1 = new QLabel(groupBox_3);
        m_fx_label_2_1->setObjectName(QString::fromUtf8("m_fx_label_2_1"));

        gridLayout_4->addWidget(m_fx_label_2_1, 0, 0, 1, 1);

        m_fx_label_2_3 = new QLabel(groupBox_3);
        m_fx_label_2_3->setObjectName(QString::fromUtf8("m_fx_label_2_3"));

        gridLayout_4->addWidget(m_fx_label_2_3, 0, 2, 1, 1);

        m_fx_knob_2_1 = new QDial(groupBox_3);
        m_fx_knob_2_1->setObjectName(QString::fromUtf8("m_fx_knob_2_1"));
        m_fx_knob_2_1->setMinimumSize(QSize(48, 48));
        m_fx_knob_2_1->setMaximumSize(QSize(48, 48));
        m_fx_knob_2_1->setStyleSheet(QString::fromUtf8(""));

        gridLayout_4->addWidget(m_fx_knob_2_1, 1, 0, 1, 1);

        verticalLayout_3->addLayout(gridLayout_4);

        gridLayout->addWidget(groupBox_3, 0, 1, 1, 1);

        groupBox_4 = new QGroupBox(tab_2);
        groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
        groupBox_4->setStyleSheet(QString::fromUtf8("QGroupBox{border-color: rgb(0, 0, 127); background-color: white;};"));
        verticalLayout_5 = new QVBoxLayout(groupBox_4);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        gridLayout_5 = new QGridLayout();
        gridLayout_5->setObjectName(QString::fromUtf8("gridLayout_5"));
        m_fx_knob_3_3 = new QDial(groupBox_4);
        m_fx_knob_3_3->setObjectName(QString::fromUtf8("m_fx_knob_3_3"));
        m_fx_knob_3_3->setMinimumSize(QSize(48, 48));
        m_fx_knob_3_3->setMaximumSize(QSize(48, 48));

        gridLayout_5->addWidget(m_fx_knob_3_3, 1, 2, 1, 1);

        m_fx_label_3_2 = new QLabel(groupBox_4);
        m_fx_label_3_2->setObjectName(QString::fromUtf8("m_fx_label_3_2"));

        gridLayout_5->addWidget(m_fx_label_3_2, 0, 1, 1, 1);

        label_15 = new QLabel(groupBox_4);
        label_15->setObjectName(QString::fromUtf8("label_15"));

        gridLayout_5->addWidget(label_15, 0, 3, 1, 1);

        m_fx_knob_3_2 = new QDial(groupBox_4);
        m_fx_knob_3_2->setObjectName(QString::fromUtf8("m_fx_knob_3_2"));
        m_fx_knob_3_2->setMinimumSize(QSize(48, 48));
        m_fx_knob_3_2->setMaximumSize(QSize(48, 48));

        gridLayout_5->addWidget(m_fx_knob_3_2, 1, 1, 1, 1);

        comboBox_4 = new QComboBox(groupBox_4);
        comboBox_4->setObjectName(QString::fromUtf8("comboBox_4"));
        comboBox_4->setMinimumSize(QSize(60, 0));
        comboBox_4->setMaximumSize(QSize(60, 16777215));

        gridLayout_5->addWidget(comboBox_4, 1, 3, 1, 1);

        m_fx_label_3_1 = new QLabel(groupBox_4);
        m_fx_label_3_1->setObjectName(QString::fromUtf8("m_fx_label_3_1"));

        gridLayout_5->addWidget(m_fx_label_3_1, 0, 0, 1, 1);

        m_fx_label_3_3 = new QLabel(groupBox_4);
        m_fx_label_3_3->setObjectName(QString::fromUtf8("m_fx_label_3_3"));

        gridLayout_5->addWidget(m_fx_label_3_3, 0, 2, 1, 1);

        m_fx_knob_3_1 = new QDial(groupBox_4);
        m_fx_knob_3_1->setObjectName(QString::fromUtf8("m_fx_knob_3_1"));
        m_fx_knob_3_1->setMinimumSize(QSize(48, 48));
        m_fx_knob_3_1->setMaximumSize(QSize(48, 48));
        m_fx_knob_3_1->setStyleSheet(QString::fromUtf8(""));

        gridLayout_5->addWidget(m_fx_knob_3_1, 1, 0, 1, 1);

        verticalLayout_5->addLayout(gridLayout_5);

        gridLayout->addWidget(groupBox_4, 0, 2, 1, 1);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer, 0, 3, 1, 1);

        groupBox_6 = new QGroupBox(tab_2);
        groupBox_6->setObjectName(QString::fromUtf8("groupBox_6"));
        groupBox_6->setStyleSheet(QString::fromUtf8("QGroupBox{border-color: rgb(0, 0, 127); background-color: white;};"));
        verticalLayout_7 = new QVBoxLayout(groupBox_6);
        verticalLayout_7->setObjectName(QString::fromUtf8("verticalLayout_7"));
        gridLayout_8 = new QGridLayout();
        gridLayout_8->setObjectName(QString::fromUtf8("gridLayout_8"));
        m_fx_knob_5_3 = new QDial(groupBox_6);
        m_fx_knob_5_3->setObjectName(QString::fromUtf8("m_fx_knob_5_3"));
        m_fx_knob_5_3->setMinimumSize(QSize(48, 48));
        m_fx_knob_5_3->setMaximumSize(QSize(48, 48));

        gridLayout_8->addWidget(m_fx_knob_5_3, 1, 2, 1, 1);

        m_fx_label_5_2 = new QLabel(groupBox_6);
        m_fx_label_5_2->setObjectName(QString::fromUtf8("m_fx_label_5_2"));

        gridLayout_8->addWidget(m_fx_label_5_2, 0, 1, 1, 1);

        label_29 = new QLabel(groupBox_6);
        label_29->setObjectName(QString::fromUtf8("label_29"));

        gridLayout_8->addWidget(label_29, 0, 3, 1, 1);

        m_fx_knob_5_2 = new QDial(groupBox_6);
        m_fx_knob_5_2->setObjectName(QString::fromUtf8("m_fx_knob_5_2"));
        m_fx_knob_5_2->setMinimumSize(QSize(48, 48));
        m_fx_knob_5_2->setMaximumSize(QSize(48, 48));

        gridLayout_8->addWidget(m_fx_knob_5_2, 1, 1, 1, 1);

        comboBox_6 = new QComboBox(groupBox_6);
        comboBox_6->setObjectName(QString::fromUtf8("comboBox_6"));
        comboBox_6->setMinimumSize(QSize(60, 0));
        comboBox_6->setMaximumSize(QSize(60, 16777215));

        gridLayout_8->addWidget(comboBox_6, 1, 3, 1, 1);

        m_fx_label_5_1 = new QLabel(groupBox_6);
        m_fx_label_5_1->setObjectName(QString::fromUtf8("m_fx_label_5_1"));

        gridLayout_8->addWidget(m_fx_label_5_1, 0, 0, 1, 1);

        m_fx_label_5_3 = new QLabel(groupBox_6);
        m_fx_label_5_3->setObjectName(QString::fromUtf8("m_fx_label_5_3"));

        gridLayout_8->addWidget(m_fx_label_5_3, 0, 2, 1, 1);

        m_fx_knob_5_1 = new QDial(groupBox_6);
        m_fx_knob_5_1->setObjectName(QString::fromUtf8("m_fx_knob_5_1"));
        m_fx_knob_5_1->setMinimumSize(QSize(48, 48));
        m_fx_knob_5_1->setMaximumSize(QSize(48, 48));
        m_fx_knob_5_1->setStyleSheet(QString::fromUtf8(""));

        gridLayout_8->addWidget(m_fx_knob_5_1, 1, 0, 1, 1);


        verticalLayout_7->addLayout(gridLayout_8);


        gridLayout->addWidget(groupBox_6, 1, 1, 1, 1);

        groupBox_8 = new QGroupBox(tab_2);
        groupBox_8->setObjectName(QString::fromUtf8("groupBox_8"));
        groupBox_8->setStyleSheet(QString::fromUtf8("QGroupBox{border-color: rgb(0, 0, 127); background-color: white;};"));
        verticalLayout_9 = new QVBoxLayout(groupBox_8);
        verticalLayout_9->setObjectName(QString::fromUtf8("verticalLayout_9"));
        gridLayout_12 = new QGridLayout();
        gridLayout_12->setObjectName(QString::fromUtf8("gridLayout_12"));
        label_4 = new QLabel(groupBox_8);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setAlignment(Qt::AlignCenter);

        gridLayout_12->addWidget(label_4, 0, 0, 1, 1);

        m_adsr1_a = new QDial(groupBox_8);
        m_adsr1_a->setObjectName(QString::fromUtf8("m_adsr1_a"));
        m_adsr1_a->setMinimumSize(QSize(48, 48));
        m_adsr1_a->setMaximumSize(QSize(48, 48));
        m_adsr1_a->setStyleSheet(QString::fromUtf8(""));

        gridLayout_12->addWidget(m_adsr1_a, 1, 0, 1, 1);

        m_adsr1_s = new QDial(groupBox_8);
        m_adsr1_s->setObjectName(QString::fromUtf8("m_adsr1_s"));
        m_adsr1_s->setMinimumSize(QSize(48, 48));
        m_adsr1_s->setMaximumSize(QSize(48, 48));

        gridLayout_12->addWidget(m_adsr1_s, 1, 2, 1, 1);

        m_adsr1_d = new QDial(groupBox_8);
        m_adsr1_d->setObjectName(QString::fromUtf8("m_adsr1_d"));
        m_adsr1_d->setMinimumSize(QSize(48, 48));
        m_adsr1_d->setMaximumSize(QSize(48, 48));

        gridLayout_12->addWidget(m_adsr1_d, 1, 1, 1, 1);

        m_adsr1_r = new QDial(groupBox_8);
        m_adsr1_r->setObjectName(QString::fromUtf8("m_adsr1_r"));
        m_adsr1_r->setMinimumSize(QSize(48, 48));
        m_adsr1_r->setMaximumSize(QSize(48, 48));

        gridLayout_12->addWidget(m_adsr1_r, 1, 3, 1, 1);

        label_36 = new QLabel(groupBox_8);
        label_36->setObjectName(QString::fromUtf8("label_36"));
        label_36->setAlignment(Qt::AlignCenter);

        gridLayout_12->addWidget(label_36, 0, 1, 1, 1);

        label_37 = new QLabel(groupBox_8);
        label_37->setObjectName(QString::fromUtf8("label_37"));
        label_37->setAlignment(Qt::AlignCenter);

        gridLayout_12->addWidget(label_37, 0, 2, 1, 1);

        label_38 = new QLabel(groupBox_8);
        label_38->setObjectName(QString::fromUtf8("label_38"));
        label_38->setAlignment(Qt::AlignCenter);

        gridLayout_12->addWidget(label_38, 0, 3, 1, 1);


        verticalLayout_9->addLayout(gridLayout_12);


        gridLayout->addWidget(groupBox_8, 2, 0, 1, 1);

        groupBox_9 = new QGroupBox(tab_2);
        groupBox_9->setObjectName(QString::fromUtf8("groupBox_9"));
        groupBox_9->setStyleSheet(QString::fromUtf8("QGroupBox{border-color: rgb(0, 0, 127); background-color: white;};"));
        verticalLayout_11 = new QVBoxLayout(groupBox_9);
        verticalLayout_11->setObjectName(QString::fromUtf8("verticalLayout_11"));
        gridLayout_14 = new QGridLayout();
        gridLayout_14->setObjectName(QString::fromUtf8("gridLayout_14"));
        m_adsr2_a = new QDial(groupBox_9);
        m_adsr2_a->setObjectName(QString::fromUtf8("m_adsr2_a"));
        m_adsr2_a->setMinimumSize(QSize(48, 48));
        m_adsr2_a->setMaximumSize(QSize(48, 48));
        m_adsr2_a->setStyleSheet(QString::fromUtf8(""));

        gridLayout_14->addWidget(m_adsr2_a, 1, 0, 1, 1);

        m_adsr2_s = new QDial(groupBox_9);
        m_adsr2_s->setObjectName(QString::fromUtf8("m_adsr2_s"));
        m_adsr2_s->setMinimumSize(QSize(48, 48));
        m_adsr2_s->setMaximumSize(QSize(48, 48));

        gridLayout_14->addWidget(m_adsr2_s, 1, 2, 1, 1);

        m_adsr2_d = new QDial(groupBox_9);
        m_adsr2_d->setObjectName(QString::fromUtf8("m_adsr2_d"));
        m_adsr2_d->setMinimumSize(QSize(48, 48));
        m_adsr2_d->setMaximumSize(QSize(48, 48));

        gridLayout_14->addWidget(m_adsr2_d, 1, 1, 1, 1);

        m_adsr2_r = new QDial(groupBox_9);
        m_adsr2_r->setObjectName(QString::fromUtf8("m_adsr2_r"));
        m_adsr2_r->setMinimumSize(QSize(48, 48));
        m_adsr2_r->setMaximumSize(QSize(48, 48));

        gridLayout_14->addWidget(m_adsr2_r, 1, 3, 1, 1);

        label_43 = new QLabel(groupBox_9);
        label_43->setObjectName(QString::fromUtf8("label_43"));
        label_43->setAlignment(Qt::AlignCenter);

        gridLayout_14->addWidget(label_43, 0, 3, 1, 1);

        label_44 = new QLabel(groupBox_9);
        label_44->setObjectName(QString::fromUtf8("label_44"));
        label_44->setAlignment(Qt::AlignCenter);

        gridLayout_14->addWidget(label_44, 0, 2, 1, 1);

        m_adsr2Label_a = new QLabel(groupBox_9);
        m_adsr2Label_a->setObjectName(QString::fromUtf8("m_adsr2Label_a"));
        m_adsr2Label_a->setAlignment(Qt::AlignCenter);

        gridLayout_14->addWidget(m_adsr2Label_a, 0, 0, 1, 1);

        label_45 = new QLabel(groupBox_9);
        label_45->setObjectName(QString::fromUtf8("label_45"));
        label_45->setAlignment(Qt::AlignCenter);

        gridLayout_14->addWidget(label_45, 0, 1, 1, 1);


        verticalLayout_11->addLayout(gridLayout_14);


        gridLayout->addWidget(groupBox_9, 2, 1, 1, 1);

        groupBox_10 = new QGroupBox(tab_2);
        groupBox_10->setObjectName(QString::fromUtf8("groupBox_10"));
        groupBox_10->setStyleSheet(QString::fromUtf8("QGroupBox{border-color: rgb(0, 0, 127); background-color: white;};"));
        verticalLayout_12 = new QVBoxLayout(groupBox_10);
        verticalLayout_12->setObjectName(QString::fromUtf8("verticalLayout_12"));
        gridLayout_15 = new QGridLayout();
        gridLayout_15->setObjectName(QString::fromUtf8("gridLayout_15"));
        dial_32 = new QDial(groupBox_10);
        dial_32->setObjectName(QString::fromUtf8("dial_32"));
        dial_32->setMinimumSize(QSize(48, 48));
        dial_32->setMaximumSize(QSize(48, 48));

        gridLayout_15->addWidget(dial_32, 1, 2, 1, 1);

        label_46 = new QLabel(groupBox_10);
        label_46->setObjectName(QString::fromUtf8("label_46"));

        gridLayout_15->addWidget(label_46, 0, 1, 1, 1);

        label_47 = new QLabel(groupBox_10);
        label_47->setObjectName(QString::fromUtf8("label_47"));

        gridLayout_15->addWidget(label_47, 0, 3, 1, 1);

        dial_33 = new QDial(groupBox_10);
        dial_33->setObjectName(QString::fromUtf8("dial_33"));
        dial_33->setMinimumSize(QSize(48, 48));
        dial_33->setMaximumSize(QSize(48, 48));

        gridLayout_15->addWidget(dial_33, 1, 1, 1, 1);

        comboBox_8 = new QComboBox(groupBox_10);
        comboBox_8->setObjectName(QString::fromUtf8("comboBox_8"));
        comboBox_8->setMinimumSize(QSize(60, 0));
        comboBox_8->setMaximumSize(QSize(60, 16777215));

        gridLayout_15->addWidget(comboBox_8, 1, 3, 1, 1);

        label_48 = new QLabel(groupBox_10);
        label_48->setObjectName(QString::fromUtf8("label_48"));

        gridLayout_15->addWidget(label_48, 0, 0, 1, 1);

        label_49 = new QLabel(groupBox_10);
        label_49->setObjectName(QString::fromUtf8("label_49"));

        gridLayout_15->addWidget(label_49, 0, 2, 1, 1);

        dial_34 = new QDial(groupBox_10);
        dial_34->setObjectName(QString::fromUtf8("dial_34"));
        dial_34->setMinimumSize(QSize(48, 48));
        dial_34->setMaximumSize(QSize(48, 48));
        dial_34->setStyleSheet(QString::fromUtf8(""));

        gridLayout_15->addWidget(dial_34, 1, 0, 1, 1);


        verticalLayout_12->addLayout(gridLayout_15);


        gridLayout->addWidget(groupBox_10, 1, 2, 1, 1);


        verticalLayout->addLayout(gridLayout);

        label_3 = new QLabel(tab_2);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        verticalLayout->addWidget(label_3);

        tableWidget_2 = new QTableWidget(tab_2);
        if (tableWidget_2->columnCount() < 17)
            tableWidget_2->setColumnCount(17);
        QTableWidgetItem *__qtablewidgetitem17 = new QTableWidgetItem();
        tableWidget_2->setHorizontalHeaderItem(0, __qtablewidgetitem17);
        QTableWidgetItem *__qtablewidgetitem18 = new QTableWidgetItem();
        tableWidget_2->setHorizontalHeaderItem(1, __qtablewidgetitem18);
        QTableWidgetItem *__qtablewidgetitem19 = new QTableWidgetItem();
        tableWidget_2->setHorizontalHeaderItem(2, __qtablewidgetitem19);
        QTableWidgetItem *__qtablewidgetitem20 = new QTableWidgetItem();
        tableWidget_2->setHorizontalHeaderItem(3, __qtablewidgetitem20);
        QTableWidgetItem *__qtablewidgetitem21 = new QTableWidgetItem();
        tableWidget_2->setHorizontalHeaderItem(4, __qtablewidgetitem21);
        QTableWidgetItem *__qtablewidgetitem22 = new QTableWidgetItem();
        tableWidget_2->setHorizontalHeaderItem(5, __qtablewidgetitem22);
        QTableWidgetItem *__qtablewidgetitem23 = new QTableWidgetItem();
        tableWidget_2->setHorizontalHeaderItem(6, __qtablewidgetitem23);
        QTableWidgetItem *__qtablewidgetitem24 = new QTableWidgetItem();
        tableWidget_2->setHorizontalHeaderItem(7, __qtablewidgetitem24);
        QTableWidgetItem *__qtablewidgetitem25 = new QTableWidgetItem();
        tableWidget_2->setHorizontalHeaderItem(8, __qtablewidgetitem25);
        QTableWidgetItem *__qtablewidgetitem26 = new QTableWidgetItem();
        tableWidget_2->setHorizontalHeaderItem(9, __qtablewidgetitem26);
        QTableWidgetItem *__qtablewidgetitem27 = new QTableWidgetItem();
        tableWidget_2->setHorizontalHeaderItem(10, __qtablewidgetitem27);
        QTableWidgetItem *__qtablewidgetitem28 = new QTableWidgetItem();
        tableWidget_2->setHorizontalHeaderItem(11, __qtablewidgetitem28);
        QTableWidgetItem *__qtablewidgetitem29 = new QTableWidgetItem();
        tableWidget_2->setHorizontalHeaderItem(12, __qtablewidgetitem29);
        QTableWidgetItem *__qtablewidgetitem30 = new QTableWidgetItem();
        tableWidget_2->setHorizontalHeaderItem(13, __qtablewidgetitem30);
        QTableWidgetItem *__qtablewidgetitem31 = new QTableWidgetItem();
        tableWidget_2->setHorizontalHeaderItem(14, __qtablewidgetitem31);
        QTableWidgetItem *__qtablewidgetitem32 = new QTableWidgetItem();
        tableWidget_2->setHorizontalHeaderItem(15, __qtablewidgetitem32);
        QTableWidgetItem *__qtablewidgetitem33 = new QTableWidgetItem();
        tableWidget_2->setHorizontalHeaderItem(16, __qtablewidgetitem33);
        if (tableWidget_2->rowCount() < 5)
            tableWidget_2->setRowCount(5);
        QTableWidgetItem *__qtablewidgetitem34 = new QTableWidgetItem();
        tableWidget_2->setVerticalHeaderItem(0, __qtablewidgetitem34);
        QTableWidgetItem *__qtablewidgetitem35 = new QTableWidgetItem();
        tableWidget_2->setVerticalHeaderItem(1, __qtablewidgetitem35);
        QTableWidgetItem *__qtablewidgetitem36 = new QTableWidgetItem();
        tableWidget_2->setVerticalHeaderItem(2, __qtablewidgetitem36);
        QTableWidgetItem *__qtablewidgetitem37 = new QTableWidgetItem();
        tableWidget_2->setVerticalHeaderItem(3, __qtablewidgetitem37);
        QTableWidgetItem *__qtablewidgetitem38 = new QTableWidgetItem();
        tableWidget_2->setVerticalHeaderItem(4, __qtablewidgetitem38);
        tableWidget_2->setObjectName(QString::fromUtf8("tableWidget_2"));

        verticalLayout->addWidget(tableWidget_2);


        horizontalLayout_4->addLayout(verticalLayout);

        m_main_tab->addTab(tab_2, QString());
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        label_6 = new QLabel(tab);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(120, 110, 391, 331));
        label_6->setTextFormat(Qt::RichText);
        label_6->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        m_main_tab->addTab(tab, QString());

        horizontalLayout_5->addWidget(m_main_tab);

        retranslateUi(this);

        m_main_tab->setCurrentIndex(0);
        m_note->setCurrentIndex(0);
        m_play_mode->setCurrentIndex(0);
        m_sample_fx_group->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(this);
    
        /*Connect slots manually*/
    
        connect(m_load_sample, SIGNAL(pressed()), this, SLOT(fileSelect()));
        connect(m_update_sample, SIGNAL(pressed()), this, SLOT(updateSampleTable()));
        
        connect(m_sample_start, SIGNAL(valueChanged(int)), this, SLOT(sampleStartChanged(int)));
        connect(m_sample_start_fine, SIGNAL(valueChanged(int)), this, SLOT(sampleStartFineChanged(int)));
        connect(m_sample_end, SIGNAL(valueChanged(int)), this, SLOT(sampleEndChanged(int)));
        connect(m_sample_end_fine, SIGNAL(valueChanged(int)), this, SLOT(sampleEndFineChanged(int)));
        

    /*
    QGridLayout *layout = new QGridLayout(this);
    
    QGroupBox *sampleBox = new QGroupBox("Sample", this);
    layout->addWidget(sampleBox, 0, 0, 1, 2);

    QGridLayout *sampleLayout = new QGridLayout(sampleBox);

    sampleLayout->addWidget(new QLabel("File:  "), 0, 0);

    m_sampleFile = new QLabel(NO_SAMPLE_TEXT);
    m_sampleFile->setFrameStyle(QFrame::Box | QFrame::Plain);
    sampleLayout->addWidget(m_sampleFile, 0, 1, 1, 3);

    m_duration = new QLabel("0.00 sec");
    sampleLayout->addWidget(m_duration, 2, 1, Qt::AlignLeft);
    m_sampleRate = new QLabel;
    sampleLayout->addWidget(m_sampleRate, 2, 2, Qt:: AlignCenter);
    m_channels = new QLabel;
    sampleLayout->addWidget(m_channels, 2, 3, Qt::AlignRight);

    QPixmap pmap(m_previewWidth, m_previewHeight);
    pmap.fill();
    m_preview = new QLabel;
    m_preview->setFrameStyle(QFrame::Box | QFrame::Plain);
    m_preview->setAlignment(Qt::AlignCenter);
    m_preview->setPixmap(pmap);
    sampleLayout->addWidget(m_preview, 1, 1, 1, 3);

    QPushButton *loadButton = new QPushButton(" ... ");
    sampleLayout->addWidget(loadButton, 0, 5);
    connect(loadButton, SIGNAL(pressed()), this, SLOT(fileSelect()));

    QPushButton *testButton = new QPushButton("Test");
    connect(testButton, SIGNAL(pressed()), this, SLOT(test_press()));
    connect(testButton, SIGNAL(released()), this, SLOT(test_release()));
    sampleLayout->addWidget(testButton, 1, 5);

    if (stereo) {
	m_balanceLabel = new QLabel("Balance:  ");
	sampleLayout->addWidget(m_balanceLabel, 3, 0);
    m_balance = new QSlider();
    m_balance->setMinimum(-100);
    m_balance->setMaximum(100);
    m_balance->setPageStep(25);
    m_balance->setValue(0);
    m_balance->setOrientation(Qt::Horizontal);
    m_balance->setTickPosition(QSlider::TicksBelow);
    
    sampleLayout->addWidget(m_balance, 3, 1, 1, 3);

	connect(m_balance, SIGNAL(valueChanged(int)), this, SLOT(balanceChanged(int)));
    } else {
	m_balance = 0;
	m_balanceLabel = 0;
    }

    QGroupBox *tuneBox = new QGroupBox("Tuned playback");
    layout->addWidget(tuneBox, 1, 0);
    
    QGridLayout *tuneLayout = new QGridLayout(tuneBox);

    m_retune = new QCheckBox("Enable");
    m_retune->setChecked(true);
    tuneLayout->addWidget(m_retune, 0, 0, Qt::AlignLeft);
    connect(m_retune, SIGNAL(toggled(bool)), this, SLOT(retuneChanged(bool)));

    tuneLayout->addWidget(new QLabel("Base pitch: "), 1, 0);

    m_basePitch = new QSpinBox;
    m_basePitch->setMinimum(0);
    m_basePitch->setMaximum(120);
    m_basePitch->setValue(60);
    tuneLayout->addWidget(m_basePitch, 1, 1);
    connect(m_basePitch, SIGNAL(valueChanged(int)), this, SLOT(basePitchChanged(int)));

    QGroupBox *noteOffBox = new QGroupBox("Note Off");
    layout->addWidget(noteOffBox, 1, 1);
    
    QGridLayout *noteOffLayout = new QGridLayout(noteOffBox);

    m_sustain = new QCheckBox("Enable");
    m_sustain->setChecked(true);
    noteOffLayout->addWidget(m_sustain, 0, 0, Qt::AlignLeft);
    connect(m_sustain, SIGNAL(toggled(bool)), this, SLOT(sustainChanged(bool)));
    
    noteOffLayout->addWidget(new QLabel("Release: "), 1, 0);

    m_release = new QSpinBox;
    m_release->setMinimum(0);
    m_release->setMaximum(int(Sampler_RELEASE_MAX * 1000));
    m_release->setValue(0);
    m_release->setSuffix("ms");
    m_release->setSingleStep(10);
    noteOffLayout->addWidget(m_release, 1, 1);
    connect(m_release, SIGNAL(valueChanged(int)), this, SLOT(releaseChanged(int)));

    // cause some initial updates
    retuneChanged     (m_retune    ->isChecked());
    basePitchChanged  (m_basePitch ->value());
    sustainChanged    (m_sustain   ->isChecked());
    releaseChanged    (m_release   ->value());
    if (stereo) {
	balanceChanged(m_balance   ->value());
    }
     */

    QTimer *myTimer = new QTimer(this);
    connect(myTimer, SIGNAL(timeout()), this, SLOT(oscRecv()));
    myTimer->setSingleShot(false);
    myTimer->start(0);

    m_suppressHostUpdate = false;
}

void SamplerGUI::generatePreview(QString path)
{
    SF_INFO info;
    SNDFILE *file;
    /*
    QPixmap pmap(m_previewWidth, m_previewHeight);
    pmap.fill();
    */
    findSelected();

    printf("set sample index\n");
        
    info.format = 0;
    file = sf_open(path.toLocal8Bit(), SFM_READ, &info);
    printf("Opened SNDFILE\n");
    if (file && info.frames > 0) {

	float binSize = (float)info.frames / m_previewWidth;
	float peak[2] = { 0.0f, 0.0f }, mean[2] = { 0.0f, 0.0f };
	float *frame = (float *)malloc(info.channels * sizeof(float));
	int bin = 0;
        
	QPainter paint(&(m_sample_graphs[m_selected_sample_index]));

	for (size_t i = 0; i < info.frames; ++i) {

	    sf_readf_float(file, frame, 1);

	    if (fabs(frame[0]) > peak[0]) peak[0] = fabs(frame[0]);
	    mean[0] += fabs(frame[0]);
		
	    if (info.channels > 1) {
		if (fabs(frame[1]) > peak[1]) peak[1] = fabs(frame[1]);
		mean[1] += fabs(frame[1]);
	    }

	    if (i == size_t((bin + 1) * binSize)) {

		float silent = 1.0 / float(m_previewHeight);

		if (info.channels == 1) {
		    mean[1] = mean[0];
		    peak[1] = peak[0];
		}

		mean[0] /= binSize;
		mean[1] /= binSize;

		int m = m_previewHeight / 2;

		paint.setPen(Qt::black);
		paint.drawLine(bin, m, bin, int(m - m * peak[0]));
		if (peak[0] > silent && peak[1] > silent) {
		    paint.drawLine(bin, m, bin, int(m + m * peak[1]));
		}

		paint.setPen(Qt::gray);
		paint.drawLine(bin, m, bin, int(m - m * mean[0]));
		if (mean[0] > silent && mean[1] > silent) {
		    paint.drawLine(bin, m, bin, int(m + m * mean[1]));
		}

		paint.setPen(Qt::black);
		paint.drawPoint(bin, int(m - m * peak[0]));
		if (peak[0] > silent && peak[1] > silent) {
		    paint.drawPoint(bin, int(m + m * peak[1]));
		}

		mean[0] = mean[1] = 0.0f;
		peak[0] = peak[1] = 0.0f;

		++bin;
	    }
	}
        
	int duration = int(100.0 * float(info.frames) / float(info.samplerate));
        
        m_sample_counts[m_selected_sample_index] = info.frames;
        m_sample_start_fine->setMaximum(info.frames);
        m_sample_end_fine->setMaximum(info.frames);
        m_loop_start_fine->setMaximum(info.frames);
        m_loop_end_fine->setMaximum(info.frames);
        
        /*Set seconds*/
        QTableWidgetItem *f_set_seconds = new QTableWidgetItem;
        QString * f_seconds = new QString();                
        f_seconds->setNum((float(info.frames) / float(info.samplerate)));
        f_set_seconds->setText(*f_seconds);
        m_sample_table->setItem(m_selected_sample_index, 11, f_set_seconds);
        
        /*Set samples*/
        QTableWidgetItem *f_set_samples = new QTableWidgetItem;
        QString * f_samples = new QString();                
        f_samples->setNum((info.frames));
        f_set_samples->setText(*f_samples);
        m_sample_table->setItem(m_selected_sample_index, 12, f_set_samples);
        
        /*Trigger start/end changes to update m_sample_table*/
        sampleStartChanged(m_sample_start->value());
        sampleEndChanged(m_sample_start->value());
        loopStartChanged(m_sample_start->value());
        loopEndChanged(m_sample_start->value());
                
	std::cout << "duration " << duration << std::endl;
	
        /*m_duration->setText(QString("%1.%2%3 sec")
			    .arg(duration / 100)
			    .arg((duration / 10) % 10)
			    .arg((duration % 10)));*/
	/*m_sampleRate->setText(QString("%1 Hz")
			      .arg(info.samplerate));*/
	
        //m_channels->setText(info.channels > 1 ? (m_balance ? "stereo" : "stereo (to mix)") : "mono");
	
        /*if (m_balanceLabel) {
	    m_balanceLabel->setText(info.channels == 1 ? "Pan:  " : "Balance:  ");
	}*/

    } else {
        /*
	m_duration->setText("0.00 sec");
	m_sampleRate->setText("");
	m_channels->setText("");
         */
    }
    
    if (file) sf_close(file);

    //m_preview->setPixmap(pmap);
    m_sample_graph->setPixmap(m_sample_graphs[m_selected_sample_index]);
    
}
/*
void SamplerGUI::setProjectDirectory(QString dir)
{
    QFileInfo info(dir);
    if (info.exists() && info.isDir() && info.isReadable()) {
	m_projectDir = dir;
    }
}
*/

void SamplerGUI::setSampleFile(QString file)
{
    m_suppressHostUpdate = true;
    //m_sampleFile->setText(QFileInfo(file).fileName());
    //m_file_path->setText(QFileInfo(file).fileName());
    m_file_path->setText(file);
    m_file = file;
    
    updateSampleTable();
    
    generatePreview(file);
    
    m_sample_table->resizeColumnsToContents();
    
    m_suppressHostUpdate = false;
}

void SamplerGUI::updateSampleTable()
{    
 
    findSelected();
    
    /*Set the file path*/
    QTableWidgetItem *f_set_file = new QTableWidgetItem;
    f_set_file->setText(m_file);
    m_sample_table->setItem(m_selected_sample_index, 1, f_set_file);
    /*Set the note*/
    QTableWidgetItem *f_set_note = new QTableWidgetItem;                
    f_set_note->setText(m_note->currentText());
    m_note_indexes[m_selected_sample_index] = m_note->currentIndex();
    m_sample_table->setItem(m_selected_sample_index, 2, f_set_note);
    /*Set the octave*/
    QTableWidgetItem *f_set_octave = new QTableWidgetItem;
    QString * f_octave = new QString();                
    f_octave->setNum((m_octave->value()));
    f_set_octave->setText(*f_octave);
    m_sample_table->setItem(m_selected_sample_index, 3, f_set_octave);
    /*Set the low note*/
    QTableWidgetItem *f_set_low_note = new QTableWidgetItem;
    QString * f_low_note = new QString();                
    f_low_note->setNum((m_lnote->value()));
    f_set_low_note->setText(*f_low_note);
    m_sample_table->setItem(m_selected_sample_index, 4, f_set_low_note);
    /*Set the high note*/
    QTableWidgetItem *f_set_high_note = new QTableWidgetItem;
    QString * f_high_note = new QString();                
    f_high_note->setNum((m_hnote->value()));
    f_set_high_note->setText(*f_high_note);
    m_sample_table->setItem(m_selected_sample_index, 5, f_set_high_note);
    /*Set the low velocity*/
    QTableWidgetItem *f_set_low_vel = new QTableWidgetItem;
    QString * f_low_vel = new QString();                
    f_low_vel->setNum((m_lvel->value()));
    f_set_low_vel->setText(*f_low_vel);
    m_sample_table->setItem(m_selected_sample_index, 6, f_set_low_vel);
    /*Set the high velocity*/
    QTableWidgetItem *f_set_high_vel = new QTableWidgetItem;
    QString * f_high_vel = new QString();                
    f_high_vel->setNum((m_hvel->value()));
    f_set_high_vel->setText(*f_high_vel);
    m_sample_table->setItem(m_selected_sample_index, 7, f_set_high_vel);
    /*Set the high volume*/
    QTableWidgetItem *f_set_vol = new QTableWidgetItem;
    QString * f_vol = new QString();                
    f_vol->setNum((m_sample_vol->value()));
    f_set_vol->setText(*f_vol);
    m_sample_table->setItem(m_selected_sample_index, 8, f_set_vol);
    /*Set the FX group*/
    QTableWidgetItem *f_set_fx = new QTableWidgetItem;                
    f_set_fx->setText(m_sample_fx_group->currentText());
    m_sample_table->setItem(m_selected_sample_index, 9, f_set_fx);
    /*Set the mode*/
    QTableWidgetItem *f_set_mode = new QTableWidgetItem;                
    f_set_mode->setText(m_play_mode->currentText());
    m_sample_table->setItem(m_selected_sample_index, 10, f_set_mode);
    
}


void SamplerGUI::setSelection(int a_value)
{
    m_suppressHostUpdate = true;
    m_selected_sample[a_value]->setChecked(true);
    m_suppressHostUpdate = false;
}

void SamplerGUI::setSampleStart(int a_value)
{
    m_suppressHostUpdate = true;
    m_sample_start->setValue(a_value);
    m_suppressHostUpdate = false;
}

void SamplerGUI::setSampleEnd(int a_value)
{
    m_suppressHostUpdate = true;
    m_sample_end->setValue(a_value);
    m_suppressHostUpdate = false;
}

void SamplerGUI::setSampleStartFine(int a_value)
{
    m_suppressHostUpdate = true;
    m_sample_start_fine->setValue(a_value);
    m_suppressHostUpdate = false;
}

void SamplerGUI::setSampleEndFine(int a_value)
{
    m_suppressHostUpdate = true;
    m_sample_end_fine->setValue(a_value);
    m_suppressHostUpdate = false;
}

void SamplerGUI::setLoopStart(int a_value)
{
    m_suppressHostUpdate = true;
    m_loop_start->setValue(a_value);
    m_suppressHostUpdate = false;
}

void SamplerGUI::setLoopEnd(int a_value)
{
    m_suppressHostUpdate = true;
    m_loop_end->setValue(a_value);
    m_suppressHostUpdate = false;
}

void SamplerGUI::setLoopStartFine(int a_value)
{
    m_suppressHostUpdate = true;
    m_loop_start_fine->setValue(a_value);
    m_suppressHostUpdate = false;
}

void SamplerGUI::setLoopEndFine(int a_value)
{
    m_suppressHostUpdate = true;
    m_loop_end_fine->setValue(a_value);
    m_suppressHostUpdate = false;
}



/*
void
SamplerGUI::setRetune(bool retune)
{
    m_suppressHostUpdate = true;
    m_retune->setChecked(retune);
    m_basePitch->setEnabled(retune);
    m_suppressHostUpdate = false;
}

void
SamplerGUI::setBasePitch(int pitch)
{
    m_suppressHostUpdate = true;
    m_basePitch->setValue(pitch);
    m_suppressHostUpdate = false;
}

void
SamplerGUI::setSustain(bool sustain)
{
    m_suppressHostUpdate = true;
    m_sustain->setChecked(sustain);
    m_release->setEnabled(sustain);
    m_suppressHostUpdate = false;
}

void
SamplerGUI::setRelease(int ms)
{
    m_suppressHostUpdate = true;
    m_release->setValue(ms);
    m_suppressHostUpdate = false;
}

void
SamplerGUI::setBalance(int balance)
{
    m_suppressHostUpdate = true;
    if (m_balance) {
	m_balance->setValue(balance);
    }
    m_suppressHostUpdate = false;
}

void
SamplerGUI::retuneChanged(bool retune)
{
    if (!m_suppressHostUpdate) {
	lo_send(m_host, m_controlPath, "if", Sampler_RETUNE, retune ? 1.0 : 0.0);
    }
    m_basePitch->setEnabled(retune);
}

void
SamplerGUI::basePitchChanged(int value)
{
    if (!m_suppressHostUpdate) {
	lo_send(m_host, m_controlPath, "if", Sampler_BASE_PITCH, (float)value);
    }
}

void
SamplerGUI::sustainChanged(bool on)
{
    if (!m_suppressHostUpdate) {
	lo_send(m_host, m_controlPath, "if", Sampler_SUSTAIN, on ? 0.0 : 1.0);
    }
    m_release->setEnabled(on);
}

void
SamplerGUI::releaseChanged(int release)
{
    if (!m_suppressHostUpdate) {
	float v = (float)release / 1000.0;
	if (v < Sampler_RELEASE_MIN) v = Sampler_RELEASE_MIN;
	lo_send(m_host, m_controlPath, "if", Sampler_RELEASE, v);
    }
}

void
SamplerGUI::balanceChanged(int balance)
{
    if (!m_suppressHostUpdate) {
	float v = (float)balance / 100.0;
	lo_send(m_host, m_controlPath, "if", Sampler_BALANCE, v);
    }
}
*/
void
SamplerGUI::fileSelect()
{
    QString orig = m_file;
    if (orig.isEmpty()) {
	if (!m_projectDir.isEmpty()) {
	    orig = m_projectDir;
	} else {
	    orig = ".";
	}
    }

    QString path = QFileDialog::getOpenFileName
        (this, "Select an audio sample file", orig, "Audio files (*.wav *.aiff)");

    if (!path.isEmpty()) {

	SF_INFO info;
	SNDFILE *file;

	info.format = 0;
	file = sf_open(path.toLocal8Bit(), SFM_READ, &info);

	if (!file) {
	    QMessageBox::warning
		(this, "Couldn't load audio file",
		 QString("Couldn't load audio sample file '%1'").arg(path),
		 QMessageBox::Ok, 0);
	    return;
	}
	
	if (info.frames > Sampler_FRAMES_MAX) {
	    QMessageBox::warning
		(this, "Couldn't use audio file",
		 QString("Audio sample file '%1' is too large (%2 frames, maximum is %3)").arg(path).arg((int)info.frames).arg(Sampler_FRAMES_MAX),
		 QMessageBox::Ok, 0);
	    sf_close(file);
	    return;
	} else {
	    sf_close(file);
	    lo_send(m_host, m_configurePath, "ss", "load", path.toLocal8Bit().data());
	    setSampleFile(path);
	}
    }
}

/* This finds the selected row and sets m_selected_sample_index to it's index.
 */
void SamplerGUI::findSelected()
{
    for(int i = 0; i < LMS_MAX_SAMPLE_COUNT; i++)        
    {
        if(m_selected_sample[i]->isChecked())
        {
            m_selected_sample_index = i;
            break;
        }
    }
}

void SamplerGUI::selectionChanged()
{
    findSelected();
    /*These 2 will never be null, and should be set regardless of whether a sample is loaded*/
    m_sample_graph->setPixmap(m_sample_graphs[m_selected_sample_index]);
    m_sample_num->setValue(m_selected_sample_index);
    
    QTableWidgetItem * f_item_file = m_sample_table->item(m_selected_sample_index, 1);    
    
    /*If no file loaded, then the selection is empty.  Clear the file path and don't bother setting any other widgets*/
    if(!f_item_file)
    {
        m_file_path->setText("");
        return;
    }
    
    m_file_path->setText((f_item_file->text()));
    
    QTableWidgetItem * f_item_note = m_sample_table->item(m_selected_sample_index, 2);    
    if(f_item_note){
        m_note->setCurrentIndex(m_note_indexes[m_selected_sample_index]);
    }    
    QTableWidgetItem * f_item_octave = m_sample_table->item(m_selected_sample_index, 3);    
    if(f_item_octave){
        int f_octave = f_item_octave->text().toInt();
        printf("f_octave = %i\n", f_octave);
        m_octave->setValue(f_octave);
    }
    QTableWidgetItem * f_item_lnote = m_sample_table->item(m_selected_sample_index, 4);    
    if(f_item_lnote){
        int f_lnote = f_item_lnote->text().toInt();
        printf("f_lnote = %i\n", f_lnote);
        m_lnote->setValue(f_lnote);
    }
    QTableWidgetItem * f_item_hnote = m_sample_table->item(m_selected_sample_index, 5);
    if(f_item_hnote){
        int f_hnote = f_item_hnote->text().toInt();
        printf("f_hnote = %i\n", f_hnote);
        m_hnote->setValue(f_hnote);
    }
}

void SamplerGUI::sampleStartChanged(int a_value)
{
    findSelected();
    
    if(m_sample_counts[m_selected_sample_index] > 0)
    {
        int f_value = ((int)((m_sample_counts[m_selected_sample_index]) * SLIDER_LENGTH_RECIP * (m_sample_start->value())));
                
        QTableWidgetItem * f_widget = new QTableWidgetItem;
        f_widget->setText(QString::number(f_value));
        setSampleStartFine(f_value);
        m_sample_table->setItem(m_selected_sample_index, 13, f_widget);
    }
}

void SamplerGUI::sampleEndChanged(int a_value)
{
    findSelected();
    
    if(m_sample_counts[m_selected_sample_index] > 0)
    {
        int f_value = ((int)((m_sample_counts[m_selected_sample_index]) * SLIDER_LENGTH_RECIP * (m_sample_end->value())));
                
        QTableWidgetItem * f_widget = new QTableWidgetItem;
        f_widget->setText(QString::number(f_value));
        setSampleEndFine(f_value);
        m_sample_table->setItem(m_selected_sample_index, 14, f_widget);
    }
}

void SamplerGUI::sampleStartFineChanged(int a_value)
{
    /*
    findSelected();
    
    if(m_sample_counts[m_selected_sample_index] > 0)
    {
        int f_value = ((int)(((m_sample_counts[m_selected_sample_index])/(m_sample_start_fine->value())) * SLIDER_LENGTH));
                
        QTableWidgetItem * f_widget = new QTableWidgetItem;
        f_widget->setText(QString::number(f_value));
        setSampleStart(f_value);
        m_sample_table->setItem(m_selected_sample_index, 15, f_widget);
    }
     */
}

void SamplerGUI::sampleEndFineChanged(int a_value)
{
    
}

void SamplerGUI::loopStartChanged(int a_value)
{
    
}

void SamplerGUI::loopEndChanged(int a_value)
{
    
}

void SamplerGUI::loopStartFineChanged(int a_value)
{
    
}

void SamplerGUI::loopEndFineChanged(int a_value)
{
    
}


void
SamplerGUI::oscRecv()
{
    if (osc_server) {
	lo_server_recv_noblock(osc_server, 1);
    }
}

/*
void
SamplerGUI::test_press()
{
    unsigned char noteon[4] = { 0x00, 0x90, 0x3C, 60 };

    lo_send(m_host, m_midiPath, "m", noteon);
}


void
SamplerGUI::test_release()
{
    unsigned char noteoff[4] = { 0x00, 0x90, 0x3C, 0x00 };

    lo_send(m_host, m_midiPath, "m", noteoff);
}
*/
void
SamplerGUI::aboutToQuit()
{
    if (!m_hostRequestedQuit) lo_send(m_host, m_exitingPath, "");
}

SamplerGUI::~SamplerGUI()
{
    lo_address_free(m_host);
}


void SamplerGUI::retranslateUi(QFrame* Frame)
{
    Frame->setWindowTitle(QApplication::translate("Euphoria Sampler - Powered by LibModSynth", "Frame", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem = m_sample_table->horizontalHeaderItem(0);
    ___qtablewidgetitem->setText(QApplication::translate("Frame", "Select", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem1 = m_sample_table->horizontalHeaderItem(1);
    ___qtablewidgetitem1->setText(QApplication::translate("Frame", "File", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem2 = m_sample_table->horizontalHeaderItem(2);
    ___qtablewidgetitem2->setText(QApplication::translate("Frame", "Note", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem3 = m_sample_table->horizontalHeaderItem(3);
    ___qtablewidgetitem3->setText(QApplication::translate("Frame", "Octave", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem4 = m_sample_table->horizontalHeaderItem(4);
    ___qtablewidgetitem4->setText(QApplication::translate("Frame", "Low Note", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem5 = m_sample_table->horizontalHeaderItem(5);
    ___qtablewidgetitem5->setText(QApplication::translate("Frame", "High Note", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem6 = m_sample_table->horizontalHeaderItem(6);
    ___qtablewidgetitem6->setText(QApplication::translate("Frame", "Low Vel", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem7 = m_sample_table->horizontalHeaderItem(7);
    ___qtablewidgetitem7->setText(QApplication::translate("Frame", "High Vel", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem8 = m_sample_table->horizontalHeaderItem(8);
    ___qtablewidgetitem8->setText(QApplication::translate("Frame", "Volume", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem9 = m_sample_table->horizontalHeaderItem(9);
    ___qtablewidgetitem9->setText(QApplication::translate("Frame", "FX Group", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem10 = m_sample_table->horizontalHeaderItem(10);
    ___qtablewidgetitem10->setText(QApplication::translate("Frame", "Mode", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem11 = m_sample_table->horizontalHeaderItem(11);
    ___qtablewidgetitem11->setText(QApplication::translate("Frame", "Length Seconds", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem12 = m_sample_table->horizontalHeaderItem(12);
    ___qtablewidgetitem12->setText(QApplication::translate("Frame", "Length Samples", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem13 = m_sample_table->horizontalHeaderItem(13);
    ___qtablewidgetitem13->setText(QApplication::translate("Frame", "Start", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem14 = m_sample_table->horizontalHeaderItem(14);
    ___qtablewidgetitem14->setText(QApplication::translate("Frame", "End", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem15 = m_sample_table->horizontalHeaderItem(15);
    ___qtablewidgetitem15->setText(QApplication::translate("Frame", "Loop Start", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem16 = m_sample_table->horizontalHeaderItem(16);
    ___qtablewidgetitem16->setText(QApplication::translate("Frame", "Loop End", 0, QApplication::UnicodeUTF8));
    m_load_sample->setText(QApplication::translate("Frame", "Load File", 0, QApplication::UnicodeUTF8));
    m_sample_numLabel_6->setText(QApplication::translate("Frame", "Sample#", 0, QApplication::UnicodeUTF8));
    m_hvelLabel_10->setText(QApplication::translate("Frame", "Volume", 0, QApplication::UnicodeUTF8));
    m_lnoteLabel_5->setText(QApplication::translate("Frame", "<html><head/><body><p>Low<br />Note</p></body></html>", 0, QApplication::UnicodeUTF8));
    m_note->clear();
    m_note->insertItems(0, QStringList()
        << QApplication::translate("Frame", "C", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "C#", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "D", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "D#", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "E", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "F", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "F#", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "G", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "G#", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "A", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "A#", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "B", 0, QApplication::UnicodeUTF8)
    );
    m_noteLabel_8->setText(QApplication::translate("Frame", "Note", 0, QApplication::UnicodeUTF8));
    m_noteLabel_9->setText(QApplication::translate("Frame", "Mode", 0, QApplication::UnicodeUTF8));
    m_play_mode->clear();
    m_play_mode->insertItems(0, QStringList()
        << QApplication::translate("Frame", "Play Once", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "Loop", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "Reverse", 0, QApplication::UnicodeUTF8)
    );
    m_octaveLabel_5->setText(QApplication::translate("Frame", "Octave", 0, QApplication::UnicodeUTF8));
    m_lvelLabel_5->setText(QApplication::translate("Frame", "<html><head/><body><p>Low<br />Velocity</p></body></html>", 0, QApplication::UnicodeUTF8));
    m_hvelLabel_9->setText(QApplication::translate("Frame", "<html><head/><body><p>High<br />Velocity</p></body></html>", 0, QApplication::UnicodeUTF8));
    m_hnoteLabel_5->setText(QApplication::translate("Frame", "<html><head/><body><p>High<br />Note</p></body></html>", 0, QApplication::UnicodeUTF8));
    m_noteLabel_10->setText(QApplication::translate("Frame", "FX Group", 0, QApplication::UnicodeUTF8));
    m_sample_fx_group->clear();
    m_sample_fx_group->insertItems(0, QStringList()
        << QApplication::translate("Frame", "1", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "2", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "3", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "4", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "None", 0, QApplication::UnicodeUTF8)
    );
    m_update_sample->setText(QApplication::translate("Frame", "Update", 0, QApplication::UnicodeUTF8));
    m_sample_graph->setText(QApplication::translate("Frame", "pixmap", 0, QApplication::UnicodeUTF8));
    label_21->setText(QApplication::translate("Frame", "Start", 0, QApplication::UnicodeUTF8));
    label_22->setText(QApplication::translate("Frame", "End", 0, QApplication::UnicodeUTF8));
    label_23->setText(QApplication::translate("Frame", "Sample Start/End", 0, QApplication::UnicodeUTF8));
    label_24->setText(QApplication::translate("Frame", "Start", 0, QApplication::UnicodeUTF8));
    label_25->setText(QApplication::translate("Frame", "End", 0, QApplication::UnicodeUTF8));
    label_26->setText(QApplication::translate("Frame", "Loop Start/End", 0, QApplication::UnicodeUTF8));
    m_main_tab->setTabText(m_main_tab->indexOf(tab_4), QApplication::translate("Frame", "Samples", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("Frame", "Edit FX Group: ", 0, QApplication::UnicodeUTF8));
    groupBox_5->setTitle(QApplication::translate("Frame", "FX4", 0, QApplication::UnicodeUTF8));
    m_fx_label_4_2->setText(QApplication::translate("Frame", "None", 0, QApplication::UnicodeUTF8));
    label_19->setText(QApplication::translate("Frame", "Type", 0, QApplication::UnicodeUTF8));
    comboBox_5->clear();
    comboBox_5->insertItems(0, QStringList()
        << QApplication::translate("Frame", "Off", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "LP2", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "LP4", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "HP2", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "HP4", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "BP2", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "BP4", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "EQ", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "Distortion", 0, QApplication::UnicodeUTF8)
    );
    m_fx_label_4_1->setText(QApplication::translate("Frame", "None", 0, QApplication::UnicodeUTF8));
    m_fx_label_4_3->setText(QApplication::translate("Frame", "None", 0, QApplication::UnicodeUTF8));
    groupBox_2->setTitle(QApplication::translate("Frame", "FX1", 0, QApplication::UnicodeUTF8));
    m_fx_label_1_2->setText(QApplication::translate("Frame", "None", 0, QApplication::UnicodeUTF8));
    label_8->setText(QApplication::translate("Frame", "Type", 0, QApplication::UnicodeUTF8));
    comboBox_2->clear();
    comboBox_2->insertItems(0, QStringList()
        << QApplication::translate("Frame", "Off", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "LP2", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "LP4", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "HP2", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "HP4", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "BP2", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "BP4", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "EQ", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "Distortion", 0, QApplication::UnicodeUTF8)
    );
    m_fx_label_1_1->setText(QApplication::translate("Frame", "None", 0, QApplication::UnicodeUTF8));
    m_fx_label_1_3->setText(QApplication::translate("Frame", "None", 0, QApplication::UnicodeUTF8));
    groupBox_3->setTitle(QApplication::translate("Frame", "FX2", 0, QApplication::UnicodeUTF8));
    m_fx_label_2_2->setText(QApplication::translate("Frame", "None", 0, QApplication::UnicodeUTF8));
    label_11->setText(QApplication::translate("Frame", "Type", 0, QApplication::UnicodeUTF8));
    comboBox_3->clear();
    comboBox_3->insertItems(0, QStringList()
        << QApplication::translate("Frame", "Off", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "LP2", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "LP4", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "HP2", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "HP4", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "BP2", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "BP4", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "EQ", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "Distortion", 0, QApplication::UnicodeUTF8)
    );
    m_fx_label_2_1->setText(QApplication::translate("Frame", "None", 0, QApplication::UnicodeUTF8));
    m_fx_label_2_3->setText(QApplication::translate("Frame", "None", 0, QApplication::UnicodeUTF8));
    groupBox_4->setTitle(QApplication::translate("Frame", "FX3", 0, QApplication::UnicodeUTF8));
    m_fx_label_3_2->setText(QApplication::translate("Frame", "None", 0, QApplication::UnicodeUTF8));
    label_15->setText(QApplication::translate("Frame", "Type", 0, QApplication::UnicodeUTF8));
    comboBox_4->clear();
    comboBox_4->insertItems(0, QStringList()
        << QApplication::translate("Frame", "Off", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "LP2", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "LP4", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "HP2", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "HP4", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "BP2", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "BP4", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "EQ", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "Distortion", 0, QApplication::UnicodeUTF8)
    );
    m_fx_label_3_1->setText(QApplication::translate("Frame", "None", 0, QApplication::UnicodeUTF8));
    m_fx_label_3_3->setText(QApplication::translate("Frame", "None", 0, QApplication::UnicodeUTF8));
    groupBox_6->setTitle(QApplication::translate("Frame", "FX5", 0, QApplication::UnicodeUTF8));
    m_fx_label_5_2->setText(QApplication::translate("Frame", "None", 0, QApplication::UnicodeUTF8));
    label_29->setText(QApplication::translate("Frame", "Type", 0, QApplication::UnicodeUTF8));
    comboBox_6->clear();
    comboBox_6->insertItems(0, QStringList()
        << QApplication::translate("Frame", "Off", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "LP2", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "LP4", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "HP2", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "HP4", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "BP2", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "BP4", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "EQ", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "Distortion", 0, QApplication::UnicodeUTF8)
    );
    m_fx_label_5_1->setText(QApplication::translate("Frame", "None", 0, QApplication::UnicodeUTF8));
    m_fx_label_5_3->setText(QApplication::translate("Frame", "None", 0, QApplication::UnicodeUTF8));
    groupBox_8->setTitle(QApplication::translate("Frame", "ADSR1", 0, QApplication::UnicodeUTF8));
    label_4->setText(QApplication::translate("Frame", "A", 0, QApplication::UnicodeUTF8));
    label_36->setText(QApplication::translate("Frame", "D", 0, QApplication::UnicodeUTF8));
    label_37->setText(QApplication::translate("Frame", "S", 0, QApplication::UnicodeUTF8));
    label_38->setText(QApplication::translate("Frame", "R", 0, QApplication::UnicodeUTF8));
    groupBox_9->setTitle(QApplication::translate("Frame", "ADSR2", 0, QApplication::UnicodeUTF8));
    label_43->setText(QApplication::translate("Frame", "R", 0, QApplication::UnicodeUTF8));
    label_44->setText(QApplication::translate("Frame", "S", 0, QApplication::UnicodeUTF8));
    m_adsr2Label_a->setText(QApplication::translate("Frame", "A", 0, QApplication::UnicodeUTF8));
    label_45->setText(QApplication::translate("Frame", "D", 0, QApplication::UnicodeUTF8));
    groupBox_10->setTitle(QApplication::translate("Frame", "LFO", 0, QApplication::UnicodeUTF8));
    label_46->setText(QApplication::translate("Frame", "None", 0, QApplication::UnicodeUTF8));
    label_47->setText(QApplication::translate("Frame", "Type", 0, QApplication::UnicodeUTF8));
    comboBox_8->clear();
    comboBox_8->insertItems(0, QStringList()
        << QApplication::translate("Frame", "Sine", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "Triangle", 0, QApplication::UnicodeUTF8)
        << QApplication::translate("Frame", "Off", 0, QApplication::UnicodeUTF8)
    );
    label_48->setText(QApplication::translate("Frame", "Freq", 0, QApplication::UnicodeUTF8));
    label_49->setText(QApplication::translate("Frame", "None", 0, QApplication::UnicodeUTF8));
    label_3->setText(QApplication::translate("Frame", "Modulation Matrix", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem17 = tableWidget_2->horizontalHeaderItem(0);
    ___qtablewidgetitem17->setText(QApplication::translate("Frame", "Volume", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem18 = tableWidget_2->horizontalHeaderItem(1);
    ___qtablewidgetitem18->setText(QApplication::translate("Frame", "Pitch", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem19 = tableWidget_2->horizontalHeaderItem(2);
    ___qtablewidgetitem19->setText(QApplication::translate("Frame", "FX1-1", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem20 = tableWidget_2->horizontalHeaderItem(3);
    ___qtablewidgetitem20->setText(QApplication::translate("Frame", "FX1-2", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem21 = tableWidget_2->horizontalHeaderItem(4);
    ___qtablewidgetitem21->setText(QApplication::translate("Frame", "FX1-3", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem22 = tableWidget_2->horizontalHeaderItem(5);
    ___qtablewidgetitem22->setText(QApplication::translate("Frame", "FX2-1", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem23 = tableWidget_2->horizontalHeaderItem(6);
    ___qtablewidgetitem23->setText(QApplication::translate("Frame", "FX2-2", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem24 = tableWidget_2->horizontalHeaderItem(7);
    ___qtablewidgetitem24->setText(QApplication::translate("Frame", "FX2-3", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem25 = tableWidget_2->horizontalHeaderItem(8);
    ___qtablewidgetitem25->setText(QApplication::translate("Frame", "FX3-1", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem26 = tableWidget_2->horizontalHeaderItem(9);
    ___qtablewidgetitem26->setText(QApplication::translate("Frame", "FX3-2", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem27 = tableWidget_2->horizontalHeaderItem(10);
    ___qtablewidgetitem27->setText(QApplication::translate("Frame", "FX3-3", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem28 = tableWidget_2->horizontalHeaderItem(11);
    ___qtablewidgetitem28->setText(QApplication::translate("Frame", "FX4-1", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem29 = tableWidget_2->horizontalHeaderItem(12);
    ___qtablewidgetitem29->setText(QApplication::translate("Frame", "FX4-2", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem30 = tableWidget_2->horizontalHeaderItem(13);
    ___qtablewidgetitem30->setText(QApplication::translate("Frame", "FX4-3", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem31 = tableWidget_2->horizontalHeaderItem(14);
    ___qtablewidgetitem31->setText(QApplication::translate("Frame", "FX5-1", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem32 = tableWidget_2->horizontalHeaderItem(15);
    ___qtablewidgetitem32->setText(QApplication::translate("Frame", "FX5-2", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem33 = tableWidget_2->horizontalHeaderItem(16);
    ___qtablewidgetitem33->setText(QApplication::translate("Frame", "FX5-3", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem34 = tableWidget_2->verticalHeaderItem(0);
    ___qtablewidgetitem34->setText(QApplication::translate("Frame", "ADSR1", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem35 = tableWidget_2->verticalHeaderItem(1);
    ___qtablewidgetitem35->setText(QApplication::translate("Frame", "ADSR2", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem36 = tableWidget_2->verticalHeaderItem(2);
    ___qtablewidgetitem36->setText(QApplication::translate("Frame", "LFO1", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem37 = tableWidget_2->verticalHeaderItem(3);
    ___qtablewidgetitem37->setText(QApplication::translate("Frame", "Velocity", 0, QApplication::UnicodeUTF8));
    QTableWidgetItem *___qtablewidgetitem38 = tableWidget_2->verticalHeaderItem(4);
    ___qtablewidgetitem38->setText(QApplication::translate("Frame", "Pitch", 0, QApplication::UnicodeUTF8));
    m_main_tab->setTabText(m_main_tab->indexOf(tab_2), QApplication::translate("Frame", "Poly FX", 0, QApplication::UnicodeUTF8));
    /*
    label_6->setText(QApplication::translate("Frame", "<html><img src=\\\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAASwAAADYCAYAAABRCd7OAAAAAXNSR0IArs4c6QAAAAZiS0dEAP8A/wD/oL2nkwAAAAlwSFlzAAALEwAACxMBAJqcGAAAAAd0SU1FB9wDGhM4J33Zr2YAAAAZdEVYdENvbW1lbnQAQ3JlYXRlZCB3aXRoIEdJTVBXgQ4XAAAgAElEQVR42uy9d5glV3UtvvapcEPnnpnuyUFxJIGEUMCIaBOdZLCfA/wMfjYOBJFtYx6P8ODhZ2NjjMHpYYEJz/aHE9gmg0mWBEoozWhy6J7pns7pxgpn/f6oU1WnbvdoZqQBtVCf75uZDtPhnqpaZ++1114bWFtra22trbW1ttbW2lpba2ttra21tbbW1tpaW2trba2ttbW21tbaWltra22trbW1ttbW2lpba2ttra21tbbW1tpaW2trba2ttbW2VtOStS1YW4+XdVDP4WI1gP16Fg4AgYgIAQooApKA0H4wkg+JSPIOODk7hxvWX7C2mWtrba2tHwBocRZjnH7YX08yQbG19agsd20L1tbjYe3Xc3BFy4UySAA4oOcqnmBAAIIgREGDEOokvEowiQKASYTlOMSMiLQA4Bsn9uLZWy9f29jVmBIe5BwulgEc4CwEkl5Lc10FLHwkiapF0o8mX0ECELHuBRTeSn8Vpt+FhIgCSey/fw9uvOoZq/dh4AwulXXYp6ehRGUviyJJhiHJh5LXZvbD2isIzR4KgCQ1EWuv2LFD6d/pxaMmdjvr1u7m016fWVwqg9n7h/XsM7TgOSTXQxADSFCKNAjVsdmAgOILZErH+rO7vXXfW9vVVQxYJpSWi83p9GiskIQnsupB69FaaZoiskZLAsD39ByqABQ0LlbJdTmkFz1K9P8B/C0HssOhE0NE"
                    "E0y3jzTnLyU/UwlBSehosNqg/tpio/GOp3RvPbC2y6s0JTzAOXWxDGgTbe0wsZV2kqMpj7AoECGEAhENbeIJ0MQPYkVRFHNTaIACkeROIQFJ4g9ClC+a8xc5g7OeCL41vk+euWn3qiMQDnBWLjFgfoizXZpYT5AiwiTaBGg9BJIEkIAAQjERloZAJf8PgEr3yvxHCk38JWCSvEAAJWDkNBqnRCQGgDkSA49z0NqrZxGbO+5itY4AcFTP9oYSv1qDv1SGGu5nucsX1ycYaxPwmts0fTsNeOEAzlQcTE+idarsOBeI41wB4IB16K+RWqsFsPZzVl0iA/qQnu/W0L8Wg08idEAgjhLwYZbziJUcWhUXK3nJ/xFtfS4Brey/CQFCC7SvlAofjKY/c5m7/mvP3LSbf/6Ff5TX/PjPr5ob5KCew8UyQOp9OCjD10XkjRRuJBARjCEJXmlzW4t05HNZ0td531skSpYmG4CXNG2EoyCIK10P3lcf+8yVXZtHHu9g9YCehYjAB3GpDNBkBzta5KuEfHE3VO+AVKtV8XsADQ0ddebYHTsoAsf59sTo4tdrs8dedMll3kJUjzuen3gNtFYBYJncXwNAJPFLFdRbS1BKIC2aoCGJDQyVZYCLUmBYksiKaXwtxYeVafRQvFsIQoEhIP2Bw923zh87eUP/zn2rCawAQAsUAH0QwxfE4O+5kKeX4GoNBNqc2JQEkrNg1H4ibJJLLNLEkH5JIJqGY0U4UwAcEC3Rz0PJH3rt+97+rg/97nui5SfE4wWsZgABLpMBK9qaf3Ib8WsA/sgAvN4hlPtdeGWNKNKIw87joxO8HMKhuPP7Zmfmjk6cxMgll548uP9QDUDV/K/YAi2sAdejCFgpBbmfs8Mx+ARAt6vwunpY2kBAU5Lcn2KirE5G2A6wOg/+lN20sMx+ylwRtRC35g8GS6f8kue7pdI1APaZ/6JQ5KMfPcDKE9uNmrzcEcV++hUXTh+FmqbewAzUl21yvj8daWPnwwOTPoKAI0q1d"
                    "RSNtBdGgpLT5Sj1hAt3X9YDYM7a/cdNunKfngUAPMEi1/fouRfEEr/KBXcPSKlrHSvrXFGeRhzECVjZd+zy6wJAxHWOLk4Fe2enl+rbt87/V23h6Of/4m9qANYBaJk/6b2o1yDlUQQszfTooBZK2ICer0BzCOiHQDShxcaP7BERC4Gk+NCluGYevE7wAgRaazjiOZON1vy3jx2d3X3h9sXBWMcAPACh+YF6NZxo+UtgDKLdBmsRiB5BN5JUkCukGcvBW7j8bXREo+YHQSgCx4vbjeY9R443ei7Y3NpQ6V66+zt3+tZ1NdWvH27QuptT8KAKYHWQU6pF56WRxL/qA9uGWertl+p6EXFihC0NHSdsYFayRR7iW4eHibFGRkZxy6mJ+VPP/NFj99/6zZnRj36qbABr3uxxhDUR9ioALHMNNBRdoW4BQR1oxIgDAVQMxMX0JsWdnMFclpx0cjjLsx3EIBzQaQSt8MjkZOhv27g0s1AjgG7rRIuQcv6rALA0oCnQLTCoi671QvcChKbZxofEIDEyDvtBST9vgbn1pgvoIAriyckpzm4bmm9XsLT33ge6APjmT5D/aj+sYDULgYaAeIKp0D7A+f4G9Su06P9WhVo/hHJvv5SGQDKSqEFQJ3oTc4vm+8rOe9J8RI9OT7f23nk38cTdnP3sF0oAhsyhGZj70TEH6eMyFV81gGU/lVpAzaTsa6qE5OkujHTE2Z0g1fmvfY9k31QYa42RVru+EOmZvpETDoBBAEsA2ubLwkc7gtDmR2sQJDUhZFJ8MPXOFLmXgzSyAJTF9wtpIrHsUDDfRZFOGIbygFKnBhnPj42eHABQt0Cq/cMKWHdxGgKiCw4uMZzV/Xp6V4DolQCe2081MIxqf1X8AY04ihC3TaMNstQ8ywAoBd2VOVQU4UCwcGhudgn33DeEvy/Vcct3CKAGoGEOBuc0pMfa+sFHWBmXBQ1Qg9SpMBhJsb2AUIXjJZNNrkhm2rJSZCSAuXGMKkbHsdo3N98Ya9RnLhwZdQ"
                    "CsN99hyaQ88aPNZcXM94oQrSXNpI1IOhMkIDuybXLdzkAy2sTeU0nDrOQNwxkKACitnVaj4XwhaE9WPHd2bn4h5VVqZm/SKPRRj0TP57pdzwAArrZ0b3dz5poAfA3IJw+K17uFlWFP3GrMKIglDhIlc9I7mN2GyS1q3T/5XpOAozwZCxbj7+3fHyLWJTzwYB8OH10EUDbPjlqJnV2DlUcJsGLzzIRCuEiiLMMLm6ssXMYji6kD2iG2HWUVJFl5ciRkFoRneBWGau7IsYqulNxD+w5WDW8Qmciqbf59VHka5tGkAXSQCbjk9Jbkz0ORVLdpk6yCypzPykqs7AiwCIBKCcKlJX/qrntddPcoaTRTwNJWqhL9MN2wd3IKAuAaC6zu5MwzYujXecRlG8TvHkJlkyuOHyNqxaJDQCQvRFvHqGRq2/yMLYCX0seOHpM9ex4EPFcwNe1hYlIZoELHYbAGVI96hJXJfwkKqCnMn74sfSlSloSs0EOSQF0ib5AiQZ8FWsy+oQEvHUUiR0cGsLgwzPHJtomsljpC8Ud12cw/BdQ2WJkUeqVzd4X+2U6xmpyGAUz3jnCcOKrVS7jtyDAqlYhhWDf7k/6rUGC/HtvrO5wGAVxrgdUdeuZ5MXmTL7hgC/y+9axuhoiECBsajAsbSIswZUeQS5vLIpionqPjY2PR4f0HgYFeYqlGMIvsI/MnxiopAD3uASsNkShCAoyF2vBYpsiXMcU50WJXuJY1DeZJj8BODcXCvEweQR1rwexsF2pL/ViqzyPRvpSRVAvVagAs2kctk6qg4eGYF0iXa8wMeNtJM/O/Up7F1q0xjbjyhEYpxu3AxciJPnh+HXHcDaBiAN21IgE81kHru3oWgMZ1sj4HK07fGFO/0gO2bYE/sF4qW0DogFGdkpDrzMDJXAmx+16lsO35mwIBFYCloxMTbYyPV1H2a6g3Aiu6TyP8R52WWAMsFHkmMtEvmNgnuzhGEHr6CyUrnGUdAZZ0RB2WsJukFjaaPhrNCtpByXo"
                    "QndUCWGkllUlhQif6tIJgg53VUnkIwkN4OlS0W6FTmkugw1BhYaEC16+ALFvRp+rY4sfsA3WbngRF40cssPoOp38xJF5REmzaRm9gA8pbQIkDRI2kH1CKR6bYwZTVOSBWdJVCGwlfeTKFVnTPwUNNdFUFYRhhYbGNXHtlA5ZeA6zVkBJakZYmk+gBy4qDYmtGl+U87GjQKRqkFfIlEZvmMlxWGLnQsYco9h6C6HzUU0JTRTWcFu2gU6Qzcbb2Sx6Krz3dvorFuGgtaLUd+PRAOKcBq8fsupVTAJCB1V5NNSczL4vIl1cFQ9vhD66T0mYCUYCokXGoMI2tKdqnqV4erOYQJiSW3ZaOPn70gNp74CDR39dGFIeYW2ha0VXQEWGtpYSPfoSVPoxGkGIqhKQVKJ3pkZDTPI42h2WrvNmRvmgtiGMFrTsfwlXxMObdk4QWJtKPpB+JVi6Sn+Bymo1eSfKw4l7leWbSsUkgjhWiWDp4L6ymfTrX9Wn9aWzCswC6eKoaMGnhRHkWM79K6F/sg6zfztL6PvjDWiQMyIahp+wSR8732TFvemmkI4BNW+81E/5qfDzcc/hIjFIpxPxigChqW4DV7uCw1sBqtURYOjmsErNFsJCvc6X+uJXSHFnhinI5uVJMl5gwZjpRf63Gh7CgwwKpRWVphSk1FCWzWUbHc0XGFT6SNfyIBeiPWZBK1xf1KVTFgQPBDUZjdYee6W2J/vUYfNEGyvqd4g9VWVoXA+0Quml2Na37dew5xRFHPHgIJWRMbdo0C9uaYx2pALSPT0wGODnmY+vmOprNlpUONk2EFa2B1aqKsNKwQIGMqVPNaP4IUR4qsuogqqQzZLC+WGSZUwFXMC9YdWkOLcBKChLLHUoKAHMm7luwMnzLslOB1jddsez4WFxf0KMoQeDAwQ2m1ebWaGp9A3xlDL5gCFh3AcqbyvD7I0Erom4hrUVA7DwP6RHrKUcc+GwjDuqMozKk5AIOi/dvWkhkyXFlHmzdc/RoA57nIYpDLNXS"
                    "dNDmr9YA6we81EM+jMYChohhnKypk0Msk7mnqMXU7jp3cWDhY4XPSf626Z9OPpZ/3ETvzKpldjVnVQGWZFvJRHBISwRqvWbzGqVjX0whw2xY9rUWE5Z/bfp5yTcjCyh+CHTW/8JZuKjAh+BpBqy+zektbYU3htAvGAIGL0RpS1n8/pBohohb1nGXbmHShWzu0JLyHAd+tH9hfPa9n7h59tjkZLsq5fTiLL9PQQh8PTJ21Nmz90Giv7cJHYeYW7AjLJu/Wmt4XjUpoXnsYsRQIkkEkWWFhbABxkYmMUMxmirjX2cTPZmZXTHQoCk4k1bqaB5irsrIKvvNmUmjTNsS0oeGmeXuMq6Ky15/4X1mddIVa3xSTMeZdDHyMQ9ZfdBwhHiaDAEAvqkndwXkTRQ+ZRMwcCFL20sodQeCeixxYDbJUtYaU7akv1CVpaQAt/Vge3bml9/8ltbI9GzwKz994zYAfkyGdlNnymwx8XTXI6dORfcfOhqhXI5Qqwdot+10cCVJw1qU9WhHWFbzM3Riz5+d8OnDmT2ZSGUP9lmVBQnIv4ap0V/6vgnFO4MSYIXiy6p7KNNugJiEaV1iBladEaZYgRSzemgeVVqq+Wwvzaum0IqsWMCr05Fcj6X1Oc7AAfFs2QAA+Iae3BkKXk/hU7cQgxfR31kSv6stXIoYB3mpg8xyOaSkJ6QsVQW4rW8efWDhv730v/Puf/k39YwbfsTZPDDoF/Y4vQiSBbwKQHt0YiIMjx8vIYoitNp2dJWmhNEaUK22lLAAXonUCFbXiJW+SZ620IoymFfMrEeNWTsiJS365xmRFPSmWUa4Su+LjMOSdI+EGfcr1uFbMC8RScS4xk869RVlwePCbn+mETGYKECK/NVjfP22/glUiAysvq5ndgYir9fEtVvBvovE3+lLqdIiliLqsKBAEEie1lGUwClLFYAs/vP3bp35jd98rdr7ha+4uPTi1o/e8FTHhUsi1HYlyATy1AQ8x1E1oHnfyEgDAh+xjrFUb63AX61VB"
                    "1cfYOWntzYPZVYVzAytaH3MAJLQbubtNIRikZ9hgVC2mPaUtzF91qszjNBWSqjtQDGfC5GHQukZngN6JtzKoI2Fk9+KHnIBfUEUX6wWPibXC+Vv8WNmUMQ3OL0jgH6tJq/dCvRfQH+XC7/UIhZjYSR2WcIqN5CEK0rKqDIEFv/PV/7t1Cte8wYe3POgxubhua5Nw8GTL76o4kGpNiLdcS9mF8xDWR+fOu7ed/8DQF9vA2SAhQUbsIIOwv28rG+fZlbiVzmF/9Bja0h1NhyW3ZoDINUYZf69tG0J0pCgwyYljzQsDsY0OOcdJ5Dsu6EIbB1vrb4IS+xYkElRIsWWvFk8YfPSOCm1wbcaBcTwXXYxkBbfZcdUzOSnRBHDHnvry5zAcw1n9Z96YnsLvIng9dvA/ovg7/RQKjWBJY04yi2jzdGWif8pnlKOj0q8hGjpHR98f/Cnf/3RLrRaWu3YtqRbzVrvrh3BBYMDuxL+SreNtCSZfZIBIAWQ+OSpiXjPwcMxyuUIrXaAeqNpRVcrRVgPa/e/yCkIYrxANuIZlorfXs81UeceEpdjbSrSQwJW2uYfAnCNMJJZDGFULwVQkg5C3fYrZyZ8pMUsiwDWuByg0/9hlT+ImQ6LOuGwhDpNDa12QEvBmGs/bfQpRKMWOZ89k1ZrCaytZtZ2/dhDrC/rSXm+DDGJJCa3tYCbSH39dqDvEvF2evDLTWAxBiMRUciiTTsaJ3zlKh/lcBrhwlv+1zv50Y990kGpHDlbNzdj6jY2DrWf/vSnocvzXSBMWgyZteQXw2WF1ujUVLBw+GgJ6wbaaAed/FUaYT3s6OqznEAZghcYMMoATE90N4D+SMSvEGFF9OxzZWMdAK4wg0fuIHHd4xi0zq6XEIKUTM4tZpi6EWSMAjtAhtKRE0oxzCo8jOb/SDHCssIwrtIqYRpmGh0W05fGLFGzXeGkA5wy3bUUI9Ac6/NeRRHL0teGtJW6fFb5+pKelOerBKy+ok9tbQE3afD6HWDfJfB3eixVmi"
                    "JLMRhZXJ61Ycl7JfEcD6XgOMLJN7ztrfKZT/5DGYP9DdXdXY9n5loYWtfAVU9YuOHyyzb6cFSEODa7xc6D0VVKNYHmnhMn64iiKqIoQm0ZfxU8Ev7q33gSHlQBrL6oJy9vCZ/TAC51iUoJRAjoGtD4Dz3xgETxF3/S3zwqIth4+W48+zd+Gd/4yKfWAOt0pDupjb1MXv9FoUvQjgVg+bqv4I5ilep52p9q131wNg1Aj16EZU47baz6CqIGKRgFWK/Z9kJerqHKOb/inq7YrUnbCeOxEWX9ByflBSay+hInt7TA1yRghb7d8Hf6KFUaIksaOgSg7DmOFmGKsvjKhRc8EMxOvvIt/zO45Z8+W8XwhqaqlOv60NElVMp1bNm46F99ZeP67dsvKQF+CzouQJVkBw9LqqIPzo959957n6C3pwlIiPkFOxV8xIJRoYcfV6a4wMlKHfjZNvBCn9hWAnr7RVUqhDMH3ZgD6hFwmXKdqz7THP3Eiyrbbj+1dx/EVVJoN1oDrGK6E5v4WdvldlodWdIpzU5NNjO2pXiHdNj+5n5qmQi1gyVaxRwWWCDdUdBlWMMZC685p/1YDGRlWe1BrJ6BLBuSIgRyde+Rvf6dE/JTBqy+yIktLeI1MfCUncK+yxKw6moIajF0KBnbmc+GS+G/KiXlwG1/Z+Lw/E2/+z/lrq98vQtbNwdOudSMHzywgOnZGTz7aSGuv3rmsmuvdnZWKgOC2IuoA0m9cplvaTKa1uHY5GR8/8EjISolIogCo3BP9VctKx08Z/7qXziDnzZeXrfpSXca+LWQ/Mle4dBFcPqGxRt0qHwRIIJuTjAe24uoURd5En2//aEjt4+/9oLrR8fv20tvaL2Ek9OPO8g6gw4rfygTfiarqmStolzJHdnyXOAyoGIhwLAmPeM03YaZFmJ1clhppEVoUGsRjULxrlj3ZEccmrJPmQ+8XbdKXZFTQVuqkheuAJerPyX8D56Sn5bhFKw2t4hXR9BP2Q72XQ5/p89SVwNSi6HDbEe"
                    "saDLV4XYZsPr6kfsXfutVr3fu+vLXSti+JXBcrxkfODyH6dkJANN46jXTeOFzll60c2d/FahEjHUuQZZiXpD0YrZOTE0HU/sPlhFTIwxbHWBl9w+e8/KyUQTABHBjQL5gELr/GjhDO6S0swy/H+KoEE7swKsOobR9o3jd8+DEgpJdamjDs9LfPpycTp1PZQ2wCmkeQDpZuV0jM3VPzQIyJaRpjEgp9LxTglYLSqqZST8vSbCWCUvz2Ra0HuDVmxJa/ybiWtuxId0rI/qUPGdMN6fYW0PjdSKWat4WdMhKufOqBvR0fUaPy0/JRgLAl3lqY5N4VQg+ZTvQ+wT4O0ssddeBWgQdWBINpiF9SmJ2SclRcJuf33fXwqtf8yb3vltvV9i1o6FE1eMDh2YxMTUJYApbN0/iuc+avvySK7zrlLtJkSpKGPdMaZqLn0lPRAVA68Hx8TpaTR9xFJvqYCd/lTo0nPOG/7RsBgD8M6c2hMJnV4WDl4s72I/yxhBOuEjO1MHFNnSzTtYcEbeH7sBJOAsjcFqTnrez8qZXDiExsHTO9Pw+jiMsgTZXCcyjgsTAPWs4TB88FtKcdLwhLf0QLfLG/p+ZdVGhV86ciasTtOICYBkQEnQ4NeSa/2zuKiVNF82XZkpbFnmpbCcsoE8hSqyYbfUB1qf1KQDAP+lTUjVFvi/piU1Nymsj4bN2QNY9UfwLSlLqaQC1SHRgInACoq3meWqSnjgqhlv/f/feOv3rr3yd2nf7ncT2LYsSxkt638EpzMyOAzgF4CRe8OyZ7qc+WT8XanA31VYXcKMknUspq2yDNcmSU9XHaxPq9tvvBLqrbYi0MTffMNFV+icFrEe04RH1DQ70zo2Qrj74AxGdoAksEVorUDmEY842XRHl12PiqzqY+m4cx139fRsB9AIoYRUZWa4S0t1gkYRJu1rxxCdt9n3ZlqWmdbYBTVZSXC5fl2WUcxJtrXK/zMwzzI6qioBbfI35PDMWiqIFT3FYIRMNfZhVMjJWz2KuBKts"
                    "p/6Bk/gFGcKnOQFXE89Xw/xyfHKwBvxaCNywHei6Et62EhKwCkQH5gYR635Kbh5NKSvXUSjXP37PLVNvfu2bywv7Dggu2rUkUVznwSOzWKpNAZgBMA7g1IYX/XjvlZXe/ivQWtcv5R4lkAgFOsx8+0Sc7MDVY+OnuO/gIaBSCRDHARaWmh3RVWfDM88dxBtKY+kKBXT1QKCpnBBsk9pMS5SsvhwDWoESHB/tvm1uesYdXt/QR4/1I/fsZwf5z8c1YKXRQwiBK5KO+SIKbsa5AWbSJGgcy6XzQbVLiiu5CxSqaQUjgtWc7hSj0IT9ZmGILC2SvDDWuZNct5SjeRlfchFDZ5EwHwHC1Rdh/ZIM4e84AZ+xvMjZzK9wvGeR+JU2+KNbge6r4G0piZ+AFRkkZlaybBaj1kRFOY6gUv/be26ZeNNNby7XDhz0ceHOmrTDGg8enkG9OW3AagLAKBw184TrnrTpSjhDuzU3+Ap+TGpmrfliiXMTFzMAwcnp6fDE/kM+yqU6gsiWM3TyVw97wyvSVHWgHEKkRRWGQCygUxjdIqDWGp6q6NnZcXzvi1/tgifd0YZ1XbjvwUEDWDEehxY3Z2fgB6tlJC/yJSabkpUKM2OFhIpKdUdJTVFyrXdu2WCP+mIh2LB66LC6ZQ1ZJVVDk1TIDLGyUWipCYXhYvL5zbRLrLTto7Pq67LAKR9EwdW8Rx/nKZRI9SK1WX+NU6VZ6pcEwhduBvuuhre5Qr+vAakHwrY1GY6AMLPK1kBZuS5QqX3s3lsm3nzTm8r1/Ycq2LV9Cc3WEg8enUGzNQ1gGsCkia5Ouj9/o75ysG/95YiG+6GqGiZCZUcrk4F8V0QCoHXg1Kkm6rUKXCdGo9lEsdnZ5q/0w3/gmnTo6CYQjIrMbwQ2V0g3sqZQa01KTM9VXvuu0dG5b33pPx1cflE/Dhyp4/59fQAWkAxybaylhIUzPDvcqc0w1awpXorKOyJPcWhlNETBWiBr67EOk2KgVkyjuOwjqw2waBn4iTmw8"
                    "9mKtOfeJF07lrd9WmVNWwWYTU7I4Uc6pN2m0p9WDJflpqtg3awnUAXVz6mN+tt6Ro0x+rkQ/JlhcOBqeBu7UVrXhGq2wZZkU5fTCUwGujVRVq4jqNRuvueWU29+zZsqrQOHqti1vYZWu4aDR2bQak8ZsJrIuCtg+gW/8bLtw67Xtx5xeVC8PoEoDcawx0FmlrBkySnxZDAjd95zX4xKOYRSIRYXm6eJsB7RTrfowhHqJtG+k9H8BeJtukjcHq0jHWrG0Fr5rut0eX2tO5sTkx/4+CebGBvvQXdXiOMjPoKgimR61EqTkR7npDtzUWRmLyOW+lzy6p+V7NkmcxlBnJH1efmQhXH3AnsiDwuQt5p1WFbzpBHn5K9BjFmTkDoPQO3Xl/2/bLI97JZnS9KQHAgstFgiM6hZNTqsj+op9EiIn5eNGgDGJHxRAP5iPzj4JDjr++Ctb1K1WmBToFFQgmb2XmRZOQ5QqX3knv8ae+Or31huHTjYhZ3b6mi1l3DwyLQFVqdS3sr8Wbjq6iuGu0R6yySqkIoDKJ3ep5J77ic3r4YLX4+PT2DfvoOCSiUA0Mb84nnnrwDgxWpjDMjhsqhwMgriT02O7d3TXBorq65wwOllv9ffCKU6+4Wj986+6W3viu/57Oe70N+rMDkF3LNHY3mF8IfCFvv8pISp1xMSD2zNQq3PHhph9Qeio09HijS8JTPNPFVs4bdFfln3sXQ2+qwe0r04+VkE1MhaIo2GwfDINHY6nUGjLQvNdaYrkfK0hxbmsofVIf14N0/CY4hfkK2GeB97YRt8aQ+w/kmiBtajNNyCEzSgG5KM+lZ5dzjSaqBUxHGJau0j994y9uZXv6kaHTzUg53ba2gFizh0ZAqtdpoGpkA1Yd6fBqAHe7o2l4H+DXAqnsDXoGYawVr3mJHRKAWJxmemo8P7DyoAIaK4Da1X8r86L1OeGzr+VlV5z7zEdbb/w9L8gT3HRsKf6Oq+cJ1mX3vbxEgAACAASURBVF3r9neOHVv4+7/5RG"
                    "Vmz74KNm1sw3Hb2HeojnpjpcEXj6shrmfgsMQAlgOhhs7tfXWW1MhKA4vtgXxI7xG7FYVZk3PH+WCFZQXqerWu2LyqxMAPVGIkafkkIFISCi8fkZY3YNKOTWET78z3l7RYeLFdWVEwbniU1y4ovExtSsBKjz+rDby8BAxdJdK7haXNAZy4Iaxbbh/MuCVJ0uuyOI5Gdekj994y9uZXvbGiDx7qwY5tNbTaizh0dMpEVjNWZDVh3p8DUHvRF/9hsMdxhgfBXh+OGwKxApTpy8w7pczdpQAVALVDU9N1zM11oae7hWar+f0g3G8ncb0IftnZMvrx+MTn+pT/izdeeNG2rwZ7l97yla8dnRk75eDYaB/mF3yIquOyS1oIwggHj8zj+Og0kspgKrVon48U9YeSdKckKtEsgM8myrNzTL3lypDjlxQpKC4z5FthNkNBErCKs0JaHjuaWXSVRY6ZjeryIRvFADSLsjLDGi7bGnuAleSyrdXQGP5xnsLLZCMA4JN6/Oom8N9dcvNVonp2obQtFBdL0DVjsSN2VG4qEvDFUQrV+kfvu/XUm171xgoPHurD9m11tNqLOHxsqoOzGrMiqwUAiwCiH3nadVsg6KkA6IXqFopoMM4KkNZ9qTVZdX2cQo3fve+BEL4XQ0mExWXtOJ2A9bCmaF9nvf0rztZ/+4vm8XpPufSjV1964eDiBduXZhYXa/j27d246771GJ+s4tio4IF9TUzNLBpAnjX/LiHXhT2uTATPcvJzEl3p3OTfIpJXGPMlxTezT58u217e/CsdMLVqe+WKSvesQTwXeNrWg2eEFcl0tzY//FD7ZZkqP2qg9VFO4FdkGADwtxy/rEn8mi/ccRWkZzf8HRE9Z1G4REArdEwDN2DlieN46Gr9457vzP6P1/52D/cfrGLntjraZwSreSRl/joA9FbLOzTYDUpYhlQdgRMkUVYRZBJdJjyU4lNjh9WB+/d6qFYSwejCwvfF/0pE8FUSzzVZyasrO7720k988IHeZ93whN5"
                    "1g/3rZhfdmYWlHoxNTOO2u7qw/5CYn9syoDxlAXQDy33lH+cRFlMuS0Mg1BRqSYcrGBbZfhyl8AxZla3Ocpb1GOY1MDvWKLbvFIMSWV2Alfre5zq1rELYOZfQKot2DGCUXH8k7Bihnp4akrorF2bXd5Y7fsDrbziGX0vBSp/c1QZ/VcALnwjVcwVKOyL43gK4pKFjlQ4DTENx4/HkiXJK6Gp/bt8di7/3ut+tLt6/10/AKljE4WPTZwIrUSqgTkXOsl3A3kEovyTwNZOJFEWPEEkLhOLCiU7NzoZ79h+UpGVWtxFGnYT7edM6PVcEn2KIXxYPAPB3L3/9hAGgPgDrzJ8BAN1IKoE0v8eSia7SCKuFR9AmdDbrj5pH8avlnVgvgg+Gx1Fy3CS3NtbmBQqHkrpTJmdu2lwuyKYYkRrtZgu/033x95d0JxPBlZYEwwyH3En0sWDYZ9IgWU64wOba8+9CWoRoxzwZdg41XD2AJcUUlpKV8zp+XVk+4rMjvaBl2pco4cWqVlhWfyxY9uUdBT9gMP8rjuPXZZOJsk6ub1JeRvDyKyE9T4S/g/BKBqwiSataqf7FqDMcgVtCd/C1I99b/J03vqU0dtc9LnZtryGKazhyfPosIqs2tQ4B4G1z93UrcGM3pNuH60SQ2JGctCJzx07T4qlCoHF0ZraJiYluVCsLiOIGlhv2nRMwfFiPwoGCSFENGmtiUG1GiBAv+cgfy8kHHpRvffBmMWDYAOA4QEygSaAKpVwmZ2BgUsCa+dPAeZJZnG79MUfQBRef1ifxEY7Jr2MTLbfTR/Qz/46Ulxi7VwBwyiXErfZ5SAlpRRFmKoxhgJPUUCzrlLzlBLZTHWX50NRCwpmPXu/8HK2ZMqvWrzyf/KwRC6lyjUJKvhetYoiCN3SGUEkAJXk9osOGx/bWsrURWToogh+gE+WH9Sm80nBWn+J4ZZF8SSB80m6gchX8HQpeZQFYisFQJWCV9kAAgGiSDuFUpTf49ti++Te+8X/4R269w8WObXWQ"
                    "dRw5Po1mywar8ZXAyop+MNy3fpsW9pSZ8lcQo79KT898aIgmK17JmUKTd96/J4Lr6kR/Nd+0UsHWuURYf65PQgnwKtlyVvSn9f1i83OaccLFlQF40NoD4CsRrYEAZOcQ1/OeCv6BPoI+KaMLgpebwwgAfwPAzdFxT4k7CEEZIh4gviZ9CjwNKFAhGb2pddL4gYgigQaCmLoehtHCG0o7Wy8V4UsBfIKLeLn0njVYnRGwYsl3Myk5m8kwlFR+ZA4tscl0SYcrSA5GAtpC1FzekLkm54IHsed9rb6YamXAiplEpOkEFqTCzmxOo+2rmqISC/GVUcNbCWSS/4lVtWDR1DvhzH7AG/ThaAQ3qY3Z+/Pki0Ph03eQPdeIv60kfu8ipBaBoWQ+2GlUnfRGitZu1ekL71g8MfOG336bd+Abt/jYtqUJRzVw5Pg06o1OUWhaDUzBKgAQKcfR2piIuoIdAditIe2qSNWFuG2wrSy9YVp6TgZOlPTE1Kjsvfd+F+VSAMcJML9oR1id/NXp0yc9Ak+I3zSSjr/iWJeCDAvpimQmStSMAXGSeycKJIojhKGWZqMlS4tLamFh0VlcrKkojr2pUxON/3zl753S9UaskuchZjHaO29DMD7MExAKXqNysP0rPaGUhBuFWKchQ03ItgjYFINdMVgmUKGwArBMwjEVIyPZREyRtgJbDtB0oBZcz536cDw6EgbRg//1vr868XLp1U997Stw24duPl9VQm3SnhgCZYSjgs6+P9oj6bksdSlAjtjIVoi0sHzKVyHblFUNWFo0NaG16d2l2QgSeUOO3fEtLDZjWnDfaTKa9mZm42Hy88Hq3PnBcFh/ynHclJ+8+DM9/sJA9I8Pk/3Xi7e5D/6GGtEIRIfKjiVz1YtGFLs97kCwB4uTr33L29Wez3+1jC0bW/D9Bo6OTGMxa2SeRFFnNW/SoSyyGty2mdPHRgEAEfR2Er3roPwSEv1Vhyo1y85JLS7caGJ2Jt67/6AAiEC20Q5W4q/OODB1EA5eY"
                    "SKrv9YndsXk8xuCCwNQmXoMtJG+CONkbrDrgK4DXRKwu0t7Q+vYD7ALKnYh3m4gvO4XbvyvP/yZX/2W/uLX20gEo+dFC5ZdP56AgsJNxvoGAP5Sj3ZB5IqI0WUhZEcIbCEw2ANdGoKUugCvDHge4TgiSkFUOrouvd4xoDWoW9DhAtiuiw5qkEiLglfyjzzlra/6+D+/8w/vu+1DN8Pv7ZFgcYnnAbCMDst0jMQi2ooebM1o5i/QYeS4DHZs9z5KkUjv8L9Ke4hldc8llOxfQ7pnA7IL2jKKLhQMzUTCoke7ebmSTtrJ/zdtcWkWh9pNTN9fRP95EjdwEi4DC6xOPiUAf64XHLwGzroN9IcbooLMeaGjckyAjGLV5/ZHR9Caft1b36Hv+cfP9GJ4fYhKuYFjJ6Yxv5CCVRpZTT5EGpiB1Tt5wiexsUuku0LHS3yOtM4JQaPKNeGt6FgRaIwsLNZboyf74LuLgDRWSAfDs+GvXmEik1fr70kb8nwfeNHFYFcvxItN/SqlUTIXxkKwLNRZ0q/RgmpNQDjvlwfe8M8fGfvTroseMK/7EV/nD/IEFABNhddZQPVhjg8JubstvDoErnaF69aT/jqRcj9UZQBOdx9UpQTllQnPE7gKImmXhxEcpronaiHbZLAENurQwTS4dFDU7JToS1uufsZzv/UvR7/6zJ9dqm4YlGBx6awe8rPSYSEZU5+YNokx8RORfO6n9cgRljKvWJgxDqXMg4tsahNR9OymzvutBUVPkNUpa2A2TBXMxggj47Bo9wSiYzhQtgNi62+tIWqFB58wyl2L5/u+81dXYQIKAW5S202kNba7Df6SDz18NaVvu5S2tunoFnVTpFCFyfcqiqXX7dXjaC289X+9x7n15k/1YKBfo7uridGTM5idmzxNZLVggVW4Ep/UTW6NBL0ViPSJ00UKIjCGNVI7/UsTuuT7zjRavOuBvTGUiuE4kbGT6RSMnlMVbpcMbQ6IXT2C0hV0120Vd1NM0bTG2FGKRrQ2L0ISJV"
                    "Huwah98vbW0t64u1oql0s7ATxQuFsexnq/Pg4RDwKN15rUFQA+xLEhgtc2oa8JBZf2QPfsInp2wh0cgtPXQ1WuiFQIpSKqOILEWsAWEDMxrNRpN4GxNhABRFFECbx+YmCDwNkEXd83Pzl/v4qnN/b2DfY67hCApbDRdJCbw/ARkO55w03yMJK6+NnOb575zqbGDdbgl9Q9ROwUkLaXuVVktJ7WVd0rlZPu6aj6tEpoZqKJ9foLY886JB3LXqWkUdqycQOW6Vb+z/cxAH2fPoESArzegNUH9cnNbfAlCtx5FdBzGUq7CNetC2vm0qmO+gAYa+lxuzCPqP6eD3wA//bXH+1Fd1eI3t4mTo7PYGrWBquJM0VWy25kkR1tsicStLsgVVfEaYOBIlRn0VaT8FCJRxYm5f577nNQ8kK47pn4Kz5kdcqgtAI2xkBVg3UtLgN6jNOHMXfyl8KQEUmrT4k+QEspOlhbnP/igQNLlz35ilp3pLUh4qPkR5z71J738QQAwqWD16kkqvp/PODMoPrUFvicALxsEOzaBenekQDt+i44XREdBCLhItmKBKGWpDBum9uJNSGzOEw4mVkjgPhQzgz82f88fmLhuy5r117RXz98+z0lANXWYk2sw+EhQcs9u5J90laSmtTlgVPKC5uURLLJlOmZYZ1sxSqNNaUqb1Ax108XE55VTbwXdFiE1mK812lZ8KZRpFgzGgtHpXS4UojlfW85zFh7lrcwfX/5vd/XJwEhXi9pZHWyt0X8UgR9+ZVE9UlS3qXELy+QNYrWAlFE8XfWWqPieKoNf/H9//jx2sf+7K8G4DoxBvqaODWR2hrPdJDsUwasajiN5ujDnMVNMggACIltGtI3SOWVBSVm9tsdwl0Ro7/y9OTcbPzgvoMCSASibSxlmg+RDp7xLiQ5rMFeF6IVlASCVgAGkoNTfu3Sa2uuKAn6AnceUe22+dmJE0E7cDTned+DLQA9BkQ7haJn/J3+yIDVm2Vb9rE/4diOMfB5bfCpJejBq4i"
                    "eK8TbvBHuOg9OKYSj58l6IAkHoATKhZIyfBE4cQxEMSTSUDFBOhDlIqyEaHkBdGzTAIm8Q3h4cbpxz9RU4/DW4Vp7bHxx/vNfKwHoi9tB23pND9kbeQYDPzFkpkAxVbpb1XQDVtnfRXtNsnNYYUFgZeuTZJnYncu8nlb7EIrcdZTQgCi7oZtFD2hh1iZCy+KKK4wSyn2csnetSYSpgZ+kjNf5XO+JRiEAftekDx/Sx90a+LNtwbUXEZVrxd/h0+teEtY0dKzS6bG0ZjBqwlfKcdBd/9gd/7nwJ7///i40WwqbNzYwPTuPsYkUrKZWINhTsFqRR0rB6qXcr2JguAR09cDxkPgra+SNq1kuBgGhYwdQjbHFpebC0eO9IBtwdaf+qo2zVZJb6XgErNdA93q4pUGUKhV44oFKZYG0QCNGAy2G0Lkjk+l1c1WJC4sz6uC+/Wp6qK8+PT2zqP7vp4hETFo3RYezirB+k3diFxJR7+8YsPoAR9wY6llN8PkRuXOLsHotnKGLxdteolttQgVLwFIEHYpAPDjKhytEuTGOev3u++9Qh06cjBbb7ajVaEq72RStYxGluHPXzqXnX39tdWd3fw+hRRtAFigASp84dNgfP3jYR9lTJ752i8JXvtktwCDjeN56TQ85pPYMSndtSPck5olTi5nkAWOhf0YKyU6hLpOeK1KkWmxDzbwrUTIG2erSO6u+lkcJsHSWIMdCxMx9+5ZDkOmsFKtNME0EUro9jWBTSlZyPOuY52iVIs5/Z8a79BgUY7zVKnMvwvmJEPzRrWDP0+Bv64O/rgY0I+hIjChFbPcKEq6IU0JP+98P3F5771ve0Y3RE93YurmBxdo8ToydLrKaOxNY2etiVDdpsL+HcHpFVXUSocb5LShMu540Cd9zMY9A3/3g/hDUhKMi1OsNC6hs/uqcUi9HVJdA/HsXFhYO3f3dRWduXrTrRApQAmGsY3b198VPvu7aeHNXXynWgStKsoErDhw932yG+w8eIRYGKthzoFv/6+d7"
                    "AAyadDA6GyB9i55FN+tQIH5HbTMHzmi5Tvm5QPD8CuPeayA9T6K3fb346wM4egZcMNcSLpRTRlkvIKp9ac9d7e/s2dd+cGy8cfepU0uzS4sNNFoOavUKmq0SglChHWhs3zwdbBwaeNPl6y6PoCUWW3qnoiPT0825Bw95aLU34Ou3TgMYIBAbYWx6QMhD8XRnmRIScXIDpg32zIybaPU8ywqTQ7OHSzoMYmgV2SQPH/McictKcatwpUl3LERszHazCWZiG99zOQ/NooCBtgVYJmNYIWU08tLCVJ3zvByJ0WudLu/RJ54eCH9qHdn3VLjDQyhtbEC1Q8RBqsqzY2hNwNF0qk5f+O2Jgwvv+O23V2fvvreKbVvqaAcLGD05Cc0ZqyKYRlZ2c+9pT9t362N4h9oJAPCB7QHYEwravZQuH/ACsG0qrVmxlULoWMN3qvpEfUrdf+c9HlwnhOu2sTDZwHL/q3MWZwq4KJDoKOP24fnJw63JycqC6zamwjBozC5I7fiIM7Rxo/t3T7xixwVdg11L1FpRZT2VCorzrVaw74EHfVTL61FvhJhdGLSql3WzN3I6Ev6dHIHDOjwovMUcOH/E0b5F4Bfa5DMHge6nizN8Ob1dHrzyIlSjSd0mSE/EqaAkbTiNr48fWvz7z32p/bm9e+eWpmdaCKMSdu1o4OIL26jXFWr1NmoNDUCgpITuatP13AEHdENLhS+AaLjB6PFRhfv2DqLeqOHw8V4A/RBpgVw0WCRnKiycZS9hEshqo3fUTLHJ+IRYNimdjXJizaPPWk9yZfdyp+Tk52oib8o7y1T90QUsIHNkZcb7JSRkyqnSgm2umMNJwdaeqTYgI+ml4EVm6ZvkfMoa3s0TUNR4rTmZ/7ceuTSAfnEJWHctnP6tUtragoqaYEtECsKM7HeMY6fbHQz2YGnqLW97T+nwN28rY8umpL1k5MQUgnC6A6ymkLgRLK0QWS0fWCm592REbtWCvgE6TkmkREIz7cnsmAKiCXFQiibnF+L9+w46E"
                    "NEQCVBrNPHQgtGzIt3bOvweIJfs6O+9fPjFLw5m4M8cQGnxKNRc4+gdDr55aznevq1nfW/PRUDoxUo1Et4PFE1XO17j1NJiO953YAiVygJa7bTPcDHB5jOP9xICDmL8nrl+7+XJ9U3wJW3w+iFh9cfgbL4I5V2RODJPzEfUkQJVSRy3hK7wMOcX/vZfPtv++2/dujA5MVnDts0BrrkyxMx8E8dPArffG2FhIUQ7iLC4FGD3RVVcudsb2r6tcnlff5+ZsKU1E+27QDAHd3705Jhgcno9fL+N2bkKgC6QtiGhLCvgnZusIa8SkmCcaozEtssE7eSG9nxKq8K10lxta/y42KYztEj3fIyVrPYqIXVaJSmY6+VyWHu6kB2G5q9OWGy/oV3ayBOuhLgtmCmerxj07RyFhsY7TEXw3Xp0fSD4WQG3PZnSfan4OwlHGtBNe2ib5O3bUITqcnvjMTQX/+fb3+3d/9kvdGN4fSCeV+fx0Sk0WtMGnCZXAKtmh5xgRaB4pykCJIcFN7qQ7j44vqI4sVAvO+kktV7WoqDCicWl9tjhI10IgjYgDavtpdOw75xSwumR8f29w0OfpIovbMbN0vz8AheOn4Semaugv+ciDPSVn71pc0+357shko7tJAggPKVkCTq8f2y8jqX6erTaAeaXPCQjvc7KDvkdPAEB8TZJos/36tGtIfVL2+CVW4WV58DduQOlbS24cY2sm7ZlVRZP+aiEt88fn33Pxz61+M1v3drAQF+AJ+xuY2YO+PevASfGmpiYaWF+oVEoTDz5ietxyYXxridd2bW5v88PEevYajFTcHi4NR4dGz8FKOVgbl6hVlcdr6nzWj8M0l1SAz8FSMyYhXHs+WxjW7IuhYyOxZbcgsdoFlrl47GyPpYOL+DVrMMSOzKkThxHqQt+WJmkFixOk2Dm7W4dKCxIPHJSr6NkIQVAPw8B6Fv1EQg03iU7kkqSnnDnENwYgJdfAZSuQmmHC6+8CNaTl5p198LS5Emv6tUzCBr/+/1/7H"
                    "715k92o6crRle1wdGxaSwspeA0tQJYNc4GrOz123pkgwYHBgGvT1RVJwdrnEsq8hNCg9pzHGcRcXzvkaNtBEEvlNKoNxsdYBXiXBqeRfCvJGoAXiYSAzhk/sBED93YMrQLX/jkEDYP91zZPVh1STcSiTLZEEnXKenJcNo5fPd9VcRawEgwt2AfR7bKfVmV8NVHbgGhMzB/tx5d1xa+JCCevBP0nwP3gq0obW7ACevUDYOVTkU8R6Ha/NzYg9Pv/bO/jB44cKiNSy6oARTccodgz4E6RsfSw6SNfE5jHUPrNZ7/LAc/dkPjCdt37tjsVroixDof0acIOPHInger8+MTMcqlGPVGjHozPRDiczkYznZqDgWglo7ZhFlLnIUnRqme+2RaKY4JnaUQRNN6MCVzFs7NzVf3yLU8wgK0GfhJyf3ZWZwvyA5/BqvIxJzVK+aElg9PJozoLKaKZS7/sNZv8k4IXbzbilxmpf28NvHU7dBdT0FpWxVebw1oaDJWAgUsGxUr3VJWIZzFP/v8Zxc/+ec3D0MpoL+3ifHJGczMTZ4GrBZxDv5Ov8tRvM9UvcqQbSHYE4LtfqjuMuC3iLYSqHRjstmRWqPklOOJ9oy69/a7PTgqhOO0MT23EuF+TiPpX7zy9jtIPNjV4P95a3X24l3ulnI1fgL9Hk/ghNRxrv0hHDjx7NJSfOjocVPb0hoLizZ4xg+Vpvbv3Ib3mOv3YRKnOPoTbfKqXQLvefQv3AR/Uw2q3QCbiWaMqiK+AsqNvzt6z9i7PvDn4fTICeCinW1Mz5Zwxz1NPHBgHmQjA6jkWi3BceqI4wVc88RS5cbn9YWbt5augtPXBd0dElGKGabKFB+dnG7Pzc0TJd/BzFxoiPYIy0eV8eHrsEyVUCOCIJ1sbJfhO473zK+d2WCr3KW80NKVzS3psI7BslCelp/wKpY1xBDE1MYvLMWbLLMVywcrf8EJJcyCD74USuUpa2+N/cp1XcUr+8gAfQBD0HCy99/G41e3wBf0AX1Pgbd"
                    "hiP5QCwhC6FAlx2ahPEIAZfHER7X1T/u/0/rQu9/Xi0ZdYXiohanZOZyamjDglFYEU2GorWI/Z9+pGNwSi/T3QTlloMSOuZZkdonEDJyIZxbG4gcf3K8gEsNx2qjVGw+hv3q4JdjUQycEoHY/4ynd82W/9wJI905xBj2K2wCbCnTSCosDV8/V69HR4yOCOCaUEyII04ivcxBG4Xe6jcRnOZK9f4qjz2oJnjZIeE+Ft3kI/lANqtUAW2IeVF8c5aIcfH7iwOTb/+RD7bmjIwoX7IhwYtzFd+5exPGTc+YgqRmZyZy5XovKcxs6jqe3vfBZW4Y3bxrYCr3hUqh1SBwyDAWd+LcRTnhi9ITC1IzCuoEI7bC9QnEjfuSAlbk1JJ7ksYkgsiqhWHkAU7dM63GSYh6TaeeEzJqOOix2UuFpNikm08KvUtKdeaQVA7a41pKSSXGuasqpFHyTs/gpb1xKBRN2Ec5KqzOo0HhEw57erI+BBP7QVJTeytGtbfJnHHDoGlG9O1naGorSbWpTeRMWCwXJzd+DnvYttdGF33/be3vCA4d7sXm4jnpjDidPTZ4BrM5JQvA+SwAZCTcqoGcQju9RXC2MCEKnytv8PCC0VgpeNLW0FIwcOFxGO9BQThNRvNLAiUdqjEe/u4vBUk0DCLZsHhpcD7Xuaur+HkFVJwxCFiZDa0fDbU0sLAbh2EQ/wiiEYsrndT7Yy/bpvwD8gdphUvuRC1vAC12w72qovq3wNjZFhQ2ymVqCuCKqjO7orsb47Hs/8Bd67uCRErZtCXDkuODO+5cwMTVnrs28KY6kvvmLAGoO2dTAzO7nPH3bOnDoArB/kNJtemnTNh0oQM1DLZycnFRot3sRRdMIgvRg6DRI1I+oSpgWnkhFiJlLmNn/Ju9n406Wp/XCjg75ztAJnSp2ySZc6IJwFMQjTXm+bxFWNlnIkO7JYWm4PuO6LqcRpMtK1SYUei87JyF3Gvhl3n4PU/rxBo4ABN5nSPa38kRXm/rGWHjBVUT1KpR2"
                    "OqK8JbBBJKM0Cr79TG7KXvTq42gvvuvdf+gc+PZtXRha3xSNRZ4YnwI500Gy28LQNs6ywXh5GjvaF0MPrCf8flGVWIQCxrnaptBrD0cpVQeD+0+ebOhaLRn53myu1PB8XhxGg6Va8vXr1rHfcYYI9l0Ip9sHvNgYe2gznMRVomqIwwcmJ2uIol6IaDRbtk1z8FAp8++Yx+P3eLInZPwToXDrFWTpCvG2KzhuDbppAjlRgJRR5VGEk+/5vx9t7fnOnR42DWsZn3D43XuWMD2bXps0fU9T93kAdeV7zTBxtYg3XbC9tw9xdRuc7hLgRcKIzHFBQeFINBEfm5yKUKlotMMArXarA4jP+vo/5LkcSU6+x4llRBrrwbSg5FPxJJ1KkfNPxgHeaHgL8wiTt4WZS7w1UsGamrOq6asOWYOYHSes15hFoWZfzADUjh2yXq9VcEj3Nx84kQ8OM08Tiyruc9yn1/MIhBrvVzlv1YZ+YUv01TuoS9eLv6MCt6cGNmPquHCVk38JaOmRHj0PXfuDD/2Jd8unPzOAwf5IfL/Gk+NTaAd2RXDS0lo9ZDPz6dZvcCx7uwfcGoK9AdgaoOougX5E6qyRODswkrd8t6Tn4mn3e7fd4cNRIUp+gMWanQ6eb0tkAsDLDn6j11foLS7khQAAIABJREFUHQTLA1BlMin7p1lKTA3X9eP5eNY5fM/9ZTgqhusEqNftB3slYC8qfwDEjJ8ZgE/aArrXwt/WRa+3DrZjMk53oiQ+WpDGX/77p9tf//yXBX29VPWG4j176gas6ihOJkpdM+YALJb6exsAajd+69Ol3pK3wYGWPohfFVWKAR2DRrMJChw9sndfdW5svIRqJUQ7CAwQ2xFWfLYcljoDPwAACNN0p1AdTMvwksJRbgyTmtilDkBiM8zMpBCd5X/rWc1D5Yf7NP6AOayEdDcXSoqMeJ7f5REXWRhPyFw8Ym8HOobNmrcLYwmLweq5kSxOgVV+I0ee1oJ+9hDY80zxt62jv64GNhOdDoSim"
                    "V+kRHTVJWXR8Bp/8c3PLX3iLz/WDRFPdVWbPDU5Y3ytOkn2TmHoOUVWXQjs+3NzBPT3iKgKWLZ2uDDMNt1fF148t7Co9+/Z7wKI4Tpt1Op1rDxw4rzdcD293cMEKj0Q6YfqghCxMEY+fgoOXD1fq+nDR0cAEQ3lRKg1GtZDHayUEtp+jm/k8a0t8Lpu6O7r4AxvFH9DSxCETNpsSNIVEQ+l6NaZo/P//A//6qEdlB1A9J4DTZycSMBKxI6G026ERRN1Ndc/cXcAAFufuLuilAzFSfEj4RDTgzlvoI2PTU23Z2bnQ5R8ot0OzMzH4ExR4zkDVorewhjaRFjakhkxf2isB8l8QsCUdKaVSqaCysLUYzGNwtn0Z9GFIaGreCaIXa6JTfMzsosmhddLS2JGa9J1hlLWHqyg5SrsI9LvzzzGPRee7yZ9BBE0PmjU4m/gyMUB+ZNlsP96uP3bUNoYCOIQOsizKsmvMTR98eCju/W5Q7e3/vydfzDIpSVfDfY39cT0PKZmU3Ca7oisbK3VOSvJ/9ToiwAgAjcqoncIbsmHuHFqh2zfb1ap2UE5nK7XguP7DlTQagOaTQRhp3/7eZmQc91fvDd721MyHAFlAmE3EidUjTxoBjVUQriHR4+NCBIH1RDtdhunn9wDp+R33otPCoXbt4moHfSGNZW0wbaYKdcCQQklzkLXbv70v4bTI6MK5bIbHx0JcOxE0g4lMgdyJX1cVhi56GeerwGgXK12xcCGCsTtp5QUoUj7kBBqeO2TJ04KJqbLICLDX7VXAKyz2u8zRFipE4GDGISGpjUY3Y4F7AcsU2EX0xwWUx7LkiJ/YlmcVf8YmFxUFI6ShnjPuvFpAXqaCzJL7ApIz2J8xXzPWAQwK7SyQP3s9+q39DHEUPiw7AIAvEkfG2gDPxULN18L1fNElHYSoppgK4lvJQ97zWt2Rake9IW3B2ML733b7///1L1plGXZVR747XPu9MaYIyNynmoeVFUqlURJqCRZErbADMZgM8uYRggb3K"
                    "a9sL2AdmPDUhsb0UBjQAwGNGCEJSM0oAKqpBJIoua5KrOyqnKIjMyYI9587z3D7h93fC8iszJVKSn7rRUrcoh4793z7tln729/+/vqreePT2BqQtl2dwvnV7IbPZOLyf7e+kq4Vjs9foDPVA1jcozgj0NULBEMw+R+AxgO/g4JEQH6+PJqf7DVCgAGwqg3gl9dKcAdd773B4t9RJhhQrXOJAIiD5nUOCX6crBWMPxotdNR8dlzE4iVgFJ98LaNPRTg3UYtf42f4FO7Yuab6mD/IDtjLkkvJI7TJhAxAIckGJ76zAuPde7/7H0+tA1EGCucOtuBtV0QtcG8hsJObHOUH1eZnbL+xHiyNq7bNECjAnKqJLxssiPlakIA1IboLC6tAP1BHUpFSJQZ4gtkja9IBxCXgs/EECmEz6XahNLShpjLpggpOFVKocobtLyD8zqSRzqDDB61c6erwSz04hhWJt6X4VZFULY8rJ0xtC40sl5DmSiNBveh2tmOBvVLFOJZBcMtrWZI/I6I7Y3XAMFr4R0MICt9YGABywWRF2n7hwmEOprmLOL2L/z8f3WPP/DFGmamQ9KmjbPn12DM+kiwyrTY+/gKh4oB4Fv5WP7nOvMeTTxmwPEky7oP4WnADKXspfVy4dstbIhHv/SQB0DB92N0v3r41W+WOpkRYxrM1Ukiz2U4hmCKfUIshUQPRj2zttaxcZzMNYRhOLKxt2nL21iVP/pbFOHwLIP2Qk4RhIiZVVZ1AgwXHq9AtT/ywT9x4zOLFREEsGcW++j0Em4VeL0UrHZSy7Df9YWP8md/4F9ZAFCEaYfZGWfyHCZHJzruuZKLhMQpu2JOLq3E8D2LOFYIhzqE6nIPiIt3CSlbG5XoNDPYZoqjwLCm+JAY2U6dsBE61dCYYybrR6V8gUvyMny1TuYMKY6apEto0w1TlG/EFx6f2eYyy6VZnVER6ryVWkL18jbtJS/QBFn8Gh0BAPw4n7q7D757nmzlTfD2jcMd7wADxawFQZRdfzL/oxp"
                    "VEEIOfvWDHxAP/I+PT6DZYOHInj21kNlybaY3/gqGjSNeFUY0A7+0bDyvwOMVEKpAIIYGncuD9skfHPLMVmeVjz17TILZwHNidPr9rzZ+9UN8pmHZNKtETo2llziEs034ScnLOI5vN3lTvvjYUwEEmVRMcDDSIRwaFSIpOW53AQDv5ZcnI/DNAVA5AmesDlk1ibK5ze4chxwChP2bl54Kn3ri6QoYHnd7HSyvtQG0QbRV6uaOBqs8gB+57m0pFeZ0ZcA8a4m5CXKrID/RcYdNyOEEgmMWjr1Q2Vg8Z1GtGAzCVwW4v2LAMoXBApiJk04ho1BsGKIZDXX0iEZP/yQyEQ11/5K/j9rbJxQBa/H/h+FnLvGwOO2kDlVQqcZk+VqICv+WsplOWcEPOWmhGBigcsGYzxOm4omXdvIz472Z87A9tT8E3uGDx++CnNgDbyYCKZX6CA7xAhK+FQXkkItq+PHjX+59+P/93RnEUSBmpjr27PkNtLqJ6B7RWoqDZNydHW/+y33cymX8yu4CY2wX3MAnuBrG8DB1hEtiakLCVWv9fnzy2eM1xBHBdfolw4nR+cFXQxgd2WB2lyGuBCDbJKqAQYbYckm+X0DaVm/Lnnz5tAAokbsphrF3JFeyMWX60U0x4Zo9YDpIctph4Q6Yo1Lzihw43AcN/uIv76f+0qqgSoV54VwPgzAB05k3S8Fqp5lOAMAaVggAK+KaAc9EYOuDRBXk26yAYgAkiEHm9OpavLa+SfBcxlY7hjFZs+WyAfdXLAlVuqViCMQE1oVdvbWloFJQFzgv6yzbxAk5/WcLy5aS3jhz3k2zSZnB+c8WMsOjmu5X50MXnVRWxFbBZtduLZgNWzbJZabrgPT/kOvWp9dtk9/Mr91muFi21vk65mYXJZjsEjOsE+n399oXql3wO0M2e24FVW+Bvx8Qog8Tcvr86UxkOh9pWRJQQ1M/gbXW+3/+v9S7pxdqNDke2vXNrVTiuBys1jA80Pyqs5d/JZJL/CZ70ovAk1XAH4Pw"
                    "mME6KQDYMltL1ub3EjELAmJAvbSx1eutrQfQhhDF2cDzToD7qwpUb+GXy52rmRAcaLBugCqSIDSzYcr2B1jAMZu9njp18oyA0gBzjCgKL4D1DL23H+GX6yHhehe2dgiy2YSsacBYYsvpXGtiEyHsC+huPvHgox622g0yJkKr0wXQAVE7PVi20m7gBQfQQwwoOaBtdcA87QKyyfAITJaLezaBLNx48ewiYWk1BdzVToD7ZY1AXTxgZYqjDFYgjhnWJILzhS17GmCScojTkjGzbE+wGFtgU/mG43wzFFLCaUnP5ecf6vlchQ+VBSxiKMAagE22DszWEpeCUqYrmcSl5FpLgT37WbC1pVCU/1ImvYwi8HFG5L6E9fkpZrw/za56kG/uwb5mL7H3Orh7K5CVHtvQpnPbWTBkSg4fgFFFzayCW7/yvl+pvPj5L05ivKkRqzaWVjOAdgPM5VLwioHs4I/mf5wkzIegMU2Ip0nUfcBTYMWUrH0ypJ+3160Lz26hQ4988UHXsNUI/HiEznBFCaOTpW0Vws7EjEoNhArIS5hrbEs8Nlh44UqvE4ULZ8cRxxJaD2A5vpTWvwH2Dpj3+8xmL+SYC+GGsBEXBycLSKvA6i/vu18uvHTKQ+ALu7nVx2DQSbOrrVKwyqgU23DGf80MazUDwADUiAiNCpNTA7lJDGCbHaQERguyffb8EtDrNaBUnChjQGE7EfaS1/wVSkKTljsKFg7HBKvTkz+td+x2Vd8RhYgyyT6DqfJJuJ2rvbQTZW3ugsH4akgAX5mARel3sAKsZthULsSmYdZum8sZ8mrkHPxLJy95CLPKjVWzTnGhEJgaX1ySRPLu73gnvT8dqfkBfvmGDvObJogr98Cb38XeVJc4jImVABGPvgcAHrnSIgh/5/7/FX/qQ386Dc8V5LltPr24jlitl3Cr1QsEq1cVCL4Dr8X/ynFDuysijEmGqREHEhAGbEVJvKeseCHhmrba5ONPP+fCWobrZvjVKOD+aucHAQAfL1EvBsCkJ"
                    "a5MMrkO2DFgY7OPjxguSPag9bMrqz0dxbMgsgijQek9RRfLRGJgVhFPVlmIKoSXHpK21K0iCce2IboP/uXnqzi/UqXJ8ZDPr3RhbMataqXBqjdSug89ltSL+GPvGgaAPmMSgGwSuQ6EVGBtsgY5MQtIOo0Vc/L8SlIOKqVK1zWq6HoZJfbFNmOKTcWU9L9U1rbPjCFHI9KI8Di2iY8OSRWUgPh8rJCR6HEX7dHSyPDVGbCK74rBOm9OJMEqmb2koeunXFs0Va4YtRstj+MQ72T1ldJMShqHFykJyZFoXJNsoh/mE2PrzG+1ZCdvZ9k4St6cAdkIJiZQQlzJPpN013vkUAUT4V+df6r/h7/w/llsbFZpdnrAy2ubKTl0C4Q18AWdbl51mXVNqk2eZC2YNcDYXnKCCuDFxJqZ2NJoMyMR8CHy1Xo4iE4+c6yJMBKo10LE0YW6g1eC4Q4AeBefqiqYegCIBgmXGUjwK87fq4CvN9Gilx57ugKwhePE6LQG2Hk4eOi9vYvPyhDhLgI7s3ADFyQVEqWEbP/JtGt6Aq3OyTNnJ6G0jzDqoB92Ut5VO1X87I6U7tvWwTiCAPAP6pe9AcyMAnMdnkwAd7YmseZEYjrh6IVjz/prZxYlKoFBGMcYhIMLBOFLlmS5aMCKcywrERXISp4kayBYLoHLRdePyg4TQ2LHeZbBPNwF4zxoIUnnKcNQkBllXaUolkovVYM5JrYqTYuLOj47QbgA0nOh65wuWvT5SitXBDCbWquWNLNSy7XRmYCdiqmZd72Fjv/SBywArIO+sQ977U2Afye8vS4Lr0tmYAtz6kKUHUyCCFXU7MvotX/lfb9WX3vm+TFMTyjudLewlpJDidbTUnC9xN0Jr1SwAoBfojoAYBc/LOIUvxpn4VsQK2KdrtDw6gFwCFJDqNObrai9vFyFUhZx3IWxWbAa4DIs6S+vjNXTMdnKOBOPk6hwAiGYsvSKgGO2ehv21EsnZaIrLTR6g8EO3cv8467eeBT9516Ej2gmZM"
                    "x7YD1DVA2YvJissqV1kBAwYPPUQ4/5a6trDiq+5U5vgCjuAuiDuZMGq7KjNu8cD5J/joStKmC2B7Y+mGqMwIKtTbqSEABZkD29uqrX19cJjsOIexG0GWD7/OBlEXTFpWQPMQgKsDGzNcy2BI4nm5MSUCbJKtL6PAdu2TIlaarNN/MQ2FyI3hXYlU2yE8vFhrxaS8JifCkR+WFrkgWwKUZnU5wqD2SWkYPyCThaBtgz6ghbS8Wf03UdXkNKJ/4v0phwpiZo5c/vswDwbfzizR3grnFY/y3s7p4hZ7xPNtScAtYogGqbXkIAFz243d/47d8IHv+zv5hBo24IaGNpbRXGJl2lJFhdcZB9505hY1cIO8YweheoHhC5CqyZS++/dI868GwXHXrkwUecOFYM31foDXqlQDVqSf8qeS7/V5nyMj0AAk3QDUbFIRYarHOjYGYWcExr0NenT56RUIoAUiOA+7aN3bj9Bk471NMh8S6Z4GaBCzgJZJM2w5itgDAKpJ576jkZr647CHyDftiHtVmQ6o6Ughcs31UKOxjieo8w6xE5U0y+IBbDexts4EbnFs8JLK9VUsA9ugiv7JLvEXHxDCsDlA3HzBwDrIvuVbHhks4MW2Y2aRAzXNi2m3wRk69MzSAPeAxrE9FFm4LuNnuenK98lT5UylXTYMTMVjOy4FysUdpfyIM0IevAJWuX4HWlgyAL2Hm3Ll+P/OfB2ZDpRTMsf3KMAPC3tJ+YaoPvsTDjd5EzdhjejAJ0BKuYGEmHx5YCIluHJFxM9D/23BfDj/3Ohyag44AqwYCXVtcxCLPSb60UrNpXErfKHq8vEUYZYjZkHrcMXSfyHcAxQ4E8vdco2bQSrmnZDp944tkAxlr4boRevzeCX10J/SsAwD14d7F/iKcjoBqAuJIz3PMDyiZD7F680u9Hg4XFccRKQus+jL0QfsWiGmD5w59MS2PepcH1WQivDvJNdl+V7j1A2DZk9+Xnj/vY2KoRKEIUJXZhRD0UphYXbTr8PfsiBGs"
                    "LAD3GZMS2Ps7kjhP5FjCGOGcMEBhtuN3Fc0tAp9eE1gqxGozwry6b0vDKGFaaHStmssQcMRtddAJgc2f2UWXkZLItrQCLkogw7CBKaY1DlkoESLaAyLuEl9EF+3o84vR9RZyWhMw2z6AoJdKN0tF2Apwo00i0JXlkpkyDFZnUHyUiWulcJ9JR6x2Dupwco96JU7H4l98jW/XKG2PYo68FVe+Gty8gEbTZ9mz+vFSu0EkSwUOgn8By57f+4y9PRydeHsPu2T5vbK5js72EYUv51a9mdvUgXZ//uQfsigjNQ+wGdSZfwSZuxKkjYuIznM9jkCDPrPW6+viTz9YRhoR6dYBBVAbcr2iH8AFRAO59NlOaKNjFwvMAVxOMSZox6agMiS50/NTi+b7qDRK4KRkXKr+voeDvzkwhOr2IN/KJeg885zBjP2StCvIi5thSMQAhiYgh+Dm1Mjhz6swYmD2O4hVEcepvyN0dgPZt138HH4OFxSfpBvx9PiEHzHsMWM4BfgOiohhaE0xKcCcBiVM4b15aWDRwJaBUjDAsZ7Q7YZuvvksYF6UhWwbHxLbIsMDmQo4itIMAF5XYkbl0eQluTki5WaqbdQlL2/zqDFgRZacpOAKsgmUNaw2JNAtKlbw4b63SkDlOgcUzSuvCZQ/Q4a5qooORZKJgsEQWtMploRCwGy0GYO/8rz9zc5fojllY9x72dk3DGeuSHShiQ4UmIFHJoEdAUgey/we/+lvNs5//4izGmwaxzvhWGygGm3cih17xUjCpsb6MkHiiAlRmIAMQI2LWtqR4kU5FMBhwCELDUWfb3bi1eL4KpQCle7Bm1HBC4QqC7Unp+kwlBNcDJjlB5IEZmtMOYSp2KeHrNlri5KNPVsGW4Tgare4AOys0JPN5lYztz/UY3IzAqg5yA8CPySouPD5ZQJIB+MRDj9Xaa5seKoFCfzBAHCdBitG/QMAeWofH6Hq8kY8TADYwjYjE3pDZjhPJBlNFk7U6LaeT3MWzC8ee99fPLAYIfIMo"
                    "jlLAXV0EcL8SGVaB0xhijpLyDsw5Kc0OhSvC0IQJhrwVmHJd0txtL727hv0ZqITnoKQ4c/WVhQtfyoN6zEAMWEUJ9mYxRAJNM5dcvH17JyL3GMoz0ZK5V2plXvAbYFPbNVsekipz6auB4G4/OvSZ35nt+N4bJLjxejjNI3BnQlg1gI1pSBcxVw1khwR5aOg/e/4hde8HPzYPrX2Mj63j3PIatMkoCxcrBb8qp8stNDkbgscCht5FVPMZbkxWD888FVo7Ei530cMjjz5FgygieI7GIOyCt2VX5koHLMPOlAIHE8SYZFTTbrvmZLwt/eQd01KbduHFky6sJUip0B/0S4F/W/fS2TUNHHsZCvAisA/AiMJ2ii2TLennIQbrhTOLFHV7BM+1aPcGsJy9xk4Nh6E12GufxUGy0Cl6FAJTfeY9dcCdB1U8YhkylCFmkTtbk1lYWTVbq+uAlAylIxj7ilI5rz7D4ixgCShmG4GtIbaGuEwILY/C8ZCJwrAtIucapJSLkVJJZTl/jnTMxdrCj+fqpDXsuxuxPZbhFYjArDN8Lm1C5FloqflGVFjolOYMuZxBDXG1Mol85Iq/mdwP5dMA5fVxJHG3rwC45hvvfE0XfOhOwL+bvfkAwm+R6WddwVIrkrOWtAufT6Pf/eCv/u5E+OLJBmYn+2h1NtHpJbLGQmyAsA4hNiFlh3xvQM26kntmjXfb9XC/8bVU+c53QPiTsOgn8imUm1gD5RibWkYX/1fIhxESx6bzdGOSeTPPROAxS1ANhu8CTsgcU6afTqXqggAJz6yjjRMPP1FjYxi+H2F1MykHiSJIEcFxNCq+ETOT7L3uZri3XU/eXbfCf/PbASg+lzoIfQUtwqk+OIhBqgkKHIII2arMx1EQCJB2q981C6cWKlAacL0Y0Y7ywfkhUPt738CdBx5CDHgDJm+crHABqRlGJw2cbGMlIzRgvbi+adAPkw6kUmF6/SGYd3yN8uOsuAmT9ll6SlzDb7H3Uht8MCIObmEZ7GHZ1MRGA"
                    "yadxKCEcuPGSwtnHSyv1eC5BkpHOwSrr+iAuGjAitLnG4BhiVghAZUNZWMiyd1ly1UfFcQhTjcBUkPQLFsv1Ta5D+tQ5p9oclvDLArQ/SoE3g9PQGUlIVuOARsRWc1ss0aDKUwQ0mun1IUocwsq0z3EEGeNyhYPmflfobOPbJxnWwaaMi2qn/jNA5169dY9sP474M7OQ0z0YELN0KDk5ipckZMPwgUJDT/84Ad/t3LsM/fNoFYBlNnC6voyMlt5a5cALMPYVSi9yWHU5VYnsgvnWf3dk8Bv/Qm2vu/KL3cIM9EnahyGG4wRAsVWGUoIo6UYnxfSRI7ZGvT1i8+fqCOMCa4zQLefaDsx96BNH9oMEEah3WxH4QunOPzwp4Zec5rP0Buwjz91CcP34/wstuimbO9MRkClAnAV5AIMDViROx2wMORFK/1BFC6cn0OsXAiZAe6jHKx8cxvXyfakNyDrzzIJD5CWksZMknNT6rZMrAETrq37GIQeVSsRG1sOiOWAta3hUPv9X0Dvh38WMSX8qy72jnfB12gwDpFTmYCoRYDWVOiQSUC24HQXzy830O01MdbYTAF3dYEgfOVKwijtgIXQAFyOkqHHpIuViGebdCdSYYPDOQN+6CMmzqubLDHLbAmZijqJGDDMMLA21yrF1RmwvEc+D50ueAiICAnGZwhJ1y/tVpXUJmjYG7uQRk4SEJtHdGKmQu0hm3zOtP+IbFaeF5u0QAuNiXH7Dc3+G2652YKbr4eo38Turpis6TFHKUW3GFpPTxNBAgEa8QNnn48//Tsf3o9Wq4G52S2sbW7C2BY8tw/X6ZDn9kS9GoqpCeMd3C1qt93ojb/hdlHbO8veZJPd+WnOVCVsSl2UsBDE4Nzg18IywZIEpdqnnPdodPpvBM0Sc6jo44hpnaIZH6jMsfRBhJBYJR3nIfU0ZiIWzMLCi5cGoWqfOjOGKHTg1HrwXA1tLBpVFvUayfkZt/qaa4PpN99J9dtvNpWjB1gGNWwA9DwdCddoP3/q8Y"
                    "9gauVvsT77poveD1mwAoAe8ZgCgnmG6xFcnY5pZZIBPgkxgNHHlldDNQgdAIwoGsWvtpVq0dnzadVjnBDsCJJwGGTYJh34DCtOWlmsAWM63QBKecx+B9Ymz80c4xUMIHo//LOY5+dxjG6wANAlHByA53eDvMMs6g5BDsCxBlviRNPUg0MLWObTi0sMRwJax4jjwcWyxitWEob59zQcsWWT83QKJnqqpDI0NpIkELawaC9GU5gxVAJRWdaBEp4HG1iGgCUhTEKdvfr0ZeR4AJsifSEg4lxNJumqZsPD201Us/UpBgXsTmY0yGvAnGWaIU25+yTnVXmWflkAJH7kHx+y9cqBvSD3Hni7GpCVNszAIDHhoZIKM4OYCPAhaQXc+/Bvf6jZfvaFMZoaD4UUCtOTDu3fPY6ZSct7d1lzdL9rbjwyZq4/3FHz08oEAYeOhKTkeTIl+iT46uy8opydhHIrwSBTB8uyTWJBxVFl+WnuWQZ7irF3gjiaJ6r5gBMz6zS34rLdeCIF7KKHCE8++byIB6GA71nZqLp063W7MDfNOLq/om+5dtLefsNW6+i+QVwJ2CdKa6mIJLOcVU+vy9XNJ87vfnNvHYD4wM/B/uh/esX7wudnvAFQ9xhymoRPCYdJ2wSWTexF4eku2uLUI0/WYQzBddQOksjbGhi8ayrN4CxFAMfMHAFG5bSCfC04MTEsWWOCbBosLiSety14SNYCgD1qjzfbMK9hGPd17NYPkpyMwTpOeGUlzxRXL5447m+cXqjA9yy0jtOubLxDSXhlQfe8ZQ9iwZZBNukSpm17k2rfFK9p06CVYsVDkr3ExQ0r8tEJlDndpVvagJmFsHCkyUm7O9v/fN0CGZNC5okekRUqlbdLR2dsoQ80Mro02pwodwHZlrS/MstZHjarTTA+GCrh98wEQQRAidtvmLQ3H70WVlfeBTl9BM5Un0wUgzXKuUhp5VxIilAd/OZH/sB9+Df+cA6ttmDX6RmGwtS4wN5ZB0f3O7j+cAPXHpA4uGc"
                    "CuyaV9H2WYJl0fC0xRq8rW6v07KciApfXwJb+zjTcqUm6psQ", 0, QApplication::UnicodeUTF8));
     */
    m_main_tab->setTabText(m_main_tab->indexOf(tab), QApplication::translate("Frame", "About", 0, QApplication::UnicodeUTF8));

} // retranslateUi





void
osc_error(int num, const char *msg, const char *path)
{
    cerr << "Error: liblo server error " << num
	 << " in path \"" << (path ? path : "(null)")
	 << "\": " << msg << endl;
}

int
debug_handler(const char *path, const char *types, lo_arg **argv,
	      int argc, void *data, void *user_data)
{
    int i;

    cerr << "Warning: unhandled OSC message in GUI:" << endl;

    for (i = 0; i < argc; ++i) {
	cerr << "arg " << i << ": type '" << types[i] << "': ";
        lo_arg_pp((lo_type)types[i], argv[i]);
	cerr << endl;
    }

    cerr << "(path is <" << path << ">)" << endl;
    return 1;
}

int
configure_handler(const char *path, const char *types, lo_arg **argv,
		  int argc, void *data, void *user_data)
{
    SamplerGUI *gui = static_cast<SamplerGUI *>(user_data);
    const char *key = (const char *)&argv[0]->s;
    const char *value = (const char *)&argv[1]->s;

    if (!strcmp(key, "load")) {
	gui->setSampleFile(QString::fromLocal8Bit(value));
    } else if (!strcmp(key, DSSI_PROJECT_DIRECTORY_KEY)) {
	//gui->setProjectDirectory(QString::fromLocal8Bit(value));
    }

    return 0;
}

int
rate_handler(const char *path, const char *types, lo_arg **argv,
	     int argc, void *data, void *user_data)
{
    return 0;
}

int
show_handler(const char *path, const char *types, lo_arg **argv,
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

int
hide_handler(const char *path, const char *types, lo_arg **argv,
	     int argc, void *data, void *user_data)
{
    SamplerGUI *gui = static_cast<SamplerGUI *>(user_data);
    gui->hide();
    return 0;
}

int
quit_handler(const char *path, const char *types, lo_arg **argv,
	     int argc, void *data, void *user_data)
{
    SamplerGUI *gui = static_cast<SamplerGUI *>(user_data);
    gui->setHostRequestedQuit(true);
    qApp->quit();
    return 0;
}

int
control_handler(const char *path, const char *types, lo_arg **argv,
		int argc, void *data, void *user_data)
{
    SamplerGUI *gui = static_cast<SamplerGUI *>(user_data);

    if (argc < 2) {
	cerr << "Error: too few arguments to control_handler" << endl;
	return 1;
    }

    const int port = argv[0]->i;
    const float value = argv[1]->f;

    switch (port) {
/*
    case Sampler_RETUNE:
	gui->setRetune(value < 0.001f ? false : true);
	break;

    case Sampler_BASE_PITCH:
	gui->setBasePitch((int)value);
	break;

    case Sampler_SUSTAIN:
	gui->setSustain(value < 0.001f ? true : false);
	break;

    case Sampler_RELEASE:
	gui->setRelease(value < (Sampler_RELEASE_MIN + 0.000001f) ?
			0 : (int)(value * 1000.0 + 0.5));
	break;

    case Sampler_BALANCE:
	gui->setBalance((int)(value * 100.0));
	break;
*/
    default:
	cerr << "Warning: received request to set nonexistent port " << port << endl;
    }

    return 0;
}

int
main(int argc, char **argv)
{
    cerr << "trivial_sampler_qt_gui starting..." << endl;

    QApplication application(argc, argv);

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

#ifdef Q_WS_X11
    XSetErrorHandler(handle_x11_error);
#endif

    char *url = application.argv()[1];

    char *host = lo_url_get_hostname(url);
    char *port = lo_url_get_port(url);
    char *path = lo_url_get_path(url);

    char *label = application.argv()[3];
    bool stereo = false;
    if (QString(label).toLower() == QString(Sampler_Stereo_LABEL).toLower()) {
	stereo = true;
    }

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

    QObject::connect(&application, SIGNAL(aboutToQuit()), &gui, SLOT(aboutToQuit()));

    gui.setReady(true);
    return application.exec();
}

