/* -*- c-basic-offset: 4 -*-  vi:set ts=8 sts=4 sw=4: */

/* trivial_sampler_qt_gui.cpp

   DSSI Soft Synth Interface
   Constructed by Chris Cannam, Steve Harris and Sean Bolton

   A straightforward DSSI plugin sampler: Qt GUI.

   This example file is in the public domain.
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

SamplerGUI::SamplerGUI(bool stereo, const char * host, const char * port,
		       QByteArray controlPath, QByteArray midiPath, QByteArray configurePath,
		       QByteArray exitingPath, QWidget *w) :
    QFrame(w),
    m_controlPath(controlPath),
    m_midiPath(midiPath),
    m_configurePath(configurePath),
    m_exitingPath(exitingPath),
    m_previewWidth(200),
    m_previewHeight(40),
    m_suppressHostUpdate(true),
    m_hostRequestedQuit(false),
    m_ready(false)
{
    /*
     *  Example of how to add widgets to the QTableWidget
     * 
        QTableWidget* table = new QTableWidget( this );
        table->setColumnCount( 2 );
        table->setRowCount( 1 );
        table->setCellWidget ( 0, 0, new QComboBox( table ) );
        table->setCellWidget ( 0, 1, new QSpinBox( table ) );
        table->horizontalHeader()->setResizeMode( 0, QHeaderView::Stretch );
        table->horizontalHeader()->setResizeMode( 1, QHeaderView::ResizeToContents );
     */
    
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
        if (m_sample_table->rowCount() < 32)
            m_sample_table->setRowCount(32);
        m_sample_table->setObjectName(QString::fromUtf8("m_sample_table"));
        m_sample_table->setMinimumSize(QSize(0, 300));
        m_sample_table->setMaximumSize(QSize(1920, 800));
        m_sample_table->setRowCount(32);
        m_sample_table->horizontalHeader()->setCascadingSectionResizes(true);
        m_sample_table->horizontalHeader()->setDefaultSectionSize(60);
        m_sample_table->horizontalHeader()->setMinimumSectionSize(42);
        m_sample_table->horizontalHeader()->setStretchLastSection(true);

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


        m_main_tab->setCurrentIndex(0);
        m_note->setCurrentIndex(0);
        m_play_mode->setCurrentIndex(0);
        m_sample_fx_group->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(this);
    
    

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

void
SamplerGUI::generatePreview(QString path)
{
    SF_INFO info;
    SNDFILE *file;
    QPixmap pmap(m_previewWidth, m_previewHeight);
    pmap.fill();
    
    info.format = 0;
    file = sf_open(path.toLocal8Bit(), SFM_READ, &info);

    if (file && info.frames > 0) {

	float binSize = (float)info.frames / m_previewWidth;
	float peak[2] = { 0.0f, 0.0f }, mean[2] = { 0.0f, 0.0f };
	float *frame = (float *)malloc(info.channels * sizeof(float));
	int bin = 0;

	QPainter paint(&pmap);

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
    m_sample_graph->setPixmap(pmap);

}
/*
void
SamplerGUI::setProjectDirectory(QString dir)
{
    QFileInfo info(dir);
    if (info.exists() && info.isDir() && info.isReadable()) {
	m_projectDir = dir;
    }
}
*/

void
SamplerGUI::setSampleFile(QString file)
{
    m_suppressHostUpdate = true;
    //m_sampleFile->setText(QFileInfo(file).fileName());
    m_file_path->setText(QFileInfo(file).fileName());
    m_file = file;
    generatePreview(file);
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

