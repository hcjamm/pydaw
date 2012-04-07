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
#include "synth.h"

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

/*This allows the executable to run standalone for debugging.  This should normally be commented out*/
#define LMS_DEBUG_STANDALONE

using std::endl;

lo_server osc_server = 0;

static QTextStream cerr(stderr);

#define NO_SAMPLE_TEXT "<none loaded>   "

/*This corresponds to knobs having the range of 0-127, since the DSSI engine requires
 the values to be hard-coded, we can't change them when the user switches effect type*/
#define FX_KNOB_MAX 127
#define FX_KNOB_MAX_RECIP (1/FX_KNOB_MAX)
/*Define the range that filters will use*/
#define FX_FILTER_MAX_PITCH 124
#define FX_FILTER_MIN_PITCH 20
#define FX_FILTER_PITCH_RANGE (FX_FILTER_MAX_PITCH - FX_FILTER_MIN_PITCH)

/*These define the index of each column in m_sample_table.  Re-order these if you add or remove columns*/
#define SMP_TB_RADIOBUTTON_INDEX 0
#define SMP_TB_FILE_PATH_INDEX 1
#define SMP_TB_NOTE_INDEX 2
#define SMP_TB_OCTAVE_INDEX 3
#define SMP_TB_LOW_NOTE_INDEX 4
#define SMP_TB_HIGH_NOTE_INDEX 5
#define SMP_TB_VOLUME_INDEX 6
#define SMP_TB_FX_GROUP_INDEX 7
#define SMP_TB_SMP_START_INDEX 8
#define SMP_TB_SMP_END_INDEX 9
#define SMP_TB_LOOP_START_INDEX 10
#define SMP_TB_LOOP_END_INDEX 11

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
#ifndef LMS_DEBUG_STANDALONE
    m_host = lo_address_new(host, port);
#endif    
    this->setStyleSheet("QGroupBox {background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #E0E0E0, stop: 1 #FFFFFF); border: 2px solid gray;  border-radius: 10px;  margin-top: 1ex; } QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 3px; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #FFOECE, stop: 1 #FFFFFF); }");
    
    m_handle_control_updates = true;
    
    LMS_style_info * a_style = new LMS_style_info(64);
    //a_style->LMS_set_value_style("")
    
    QStringList f_notes_list = QStringList() << "C" << "C#" << "D" << "D#" << "E" << "F" << "F#" << "G" << "G#" << "A" << "A#" << "B";
    QStringList f_fx_group_list = QStringList() << "1" << "2" << "3" << "4" << "None";
    QStringList f_mode_list = QStringList() << "Play Once" << "Loop" << "Reverse";
    
    
    QList <LMS_mod_matrix_column*> f_sample_table_columns;
    
    f_sample_table_columns << new LMS_mod_matrix_column(radiobutton, QString(""), 0, 1, 0);  //Selected row
    f_sample_table_columns << new LMS_mod_matrix_column(no_widget, QString("Path"), 0, 1, 0);  //File path
    f_sample_table_columns << new LMS_mod_matrix_column(f_notes_list,QString("Note"));  //Note
    f_sample_table_columns << new LMS_mod_matrix_column(spinbox, QString("Octave"), -2, 8, 3);  //Octave
    f_sample_table_columns << new LMS_mod_matrix_column(spinbox, QString("Low Note"), 0, 127, 0);  //Low Note
    f_sample_table_columns << new LMS_mod_matrix_column(spinbox, QString("High Note"), 0, 127, 127);  //High Note    
    f_sample_table_columns << new LMS_mod_matrix_column(spinbox, QString("Volume"), -50, 36, 0);  //Volume
    f_sample_table_columns << new LMS_mod_matrix_column(f_fx_group_list, QString("FX Group"));  //FX Group
    f_sample_table_columns << new LMS_mod_matrix_column(no_widget, QString("Sample Start"), 0, 1, 0);  //Sample Start
    f_sample_table_columns << new LMS_mod_matrix_column(no_widget, QString("Sample End"), 0, 1, 0);  //Sample End
    f_sample_table_columns << new LMS_mod_matrix_column(no_widget, QString("Loop Start"), 0, 1, 0);  //Loop Start
    f_sample_table_columns << new LMS_mod_matrix_column(no_widget, QString("Loop End"), 0, 1, 0);  //Loop End
    
    
    m_sample_table = new LMS_mod_matrix(this, LMS_MAX_SAMPLE_COUNT, f_sample_table_columns, 20, a_style);
    
    m_file_selector = new LMS_file_select(this);
    
        /*Set all of the array variables that are per-sample*/
        for(int i = 0; i < LMS_MAX_SAMPLE_COUNT; i++)        
        {            
            //TODO:  Re-implement this for each radiobutton in the LMS_mod_matrix
            //connect(m_selected_sample[i], SIGNAL(clicked()), this, SLOT(selectionChanged()));
            //m_sample_table->setCellWidget(i, 0, m_selected_sample[i]);
            
            QPixmap pmap(m_previewWidth, m_previewHeight);
            pmap.fill();
            
            //m_sample_viewer->lms_sample_graph->lms_graphs[i] = pmap;
            
            m_note_indexes[i] = 0;
            m_sample_counts[i] = 0;
            m_sample_fx_group_indexes[i] = 0;
            m_sample_mode_indexes[i] = 0;
        }
    
        m_sample_viewer = new LMS_sample_viewer(this, Sampler_FRAMES_MAX);
    
        /*Code generated by Qt4 Designer*/
    
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
        
        horizontalLayout_5 = new QHBoxLayout(this);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
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

        //m_smp_tab_main_verticalLayout->addLayout(m_loop_start_end_Layout);
        m_smp_tab_main_verticalLayout->addWidget(m_sample_table->lms_mod_matrix, Qt::AlignCenter); 
        m_smp_tab_main_verticalLayout->addLayout(m_file_selector->lms_layout);
        m_smp_tab_main_verticalLayout->addLayout(m_sample_viewer->m_smp_tab_main_verticalLayout);

        horizontalLayout->addLayout(m_smp_tab_main_verticalLayout);

        m_smp_tab_scrollArea->setWidget(m_smp_tab_scrollAreaWidgetContents);

        horizontalLayout_2->addWidget(m_smp_tab_scrollArea);

        m_main_tab->addTab(m_sample_tab, QString());
        m_poly_fx_tab = new QWidget();
        m_poly_fx_tab->setObjectName(QString::fromUtf8("m_poly_fx_tab"));
        horizontalLayout_4 = new QHBoxLayout(m_poly_fx_tab);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        m_poly_fx_vertical_layout = new QVBoxLayout();
        m_poly_fx_vertical_layout->setObjectName(QString::fromUtf8("m_poly_fx_vertical_layout"));
        m_poly_fx_Layout = new QHBoxLayout();
        m_poly_fx_Layout->setObjectName(QString::fromUtf8("m_poly_fx_Layout"));

        m_poly_fx_vertical_layout->addLayout(m_poly_fx_Layout);

        horizontalLayout_4->addLayout(m_poly_fx_vertical_layout);

        m_main_tab->addTab(m_poly_fx_tab, QString());

        horizontalLayout_5->addWidget(m_main_tab);
        
        this->setWindowTitle(QApplication::translate("Frame", "Euphoria - Powered by LibModSynth", 0, QApplication::UnicodeUTF8));
        m_main_tab->setTabText(m_main_tab->indexOf(m_sample_tab), QApplication::translate("Frame", "Samples", 0, QApplication::UnicodeUTF8));
        m_main_tab->setTabText(m_main_tab->indexOf(m_poly_fx_tab), QApplication::translate("Frame", "Poly FX", 0, QApplication::UnicodeUTF8));

        m_main_tab->setCurrentIndex(0);

        m_sample_table->lms_mod_matrix->resizeColumnsToContents();
        m_sample_table->lms_mod_matrix->resizeRowsToContents();
        
        QMetaObject::connectSlotsByName(this);
    
        /*Connect slots manually*/
        connect(m_file_selector->lms_open_button, SIGNAL(pressed()), this, SLOT(fileSelect()));    
        /*        
        connect(m_update_sample, SIGNAL(pressed()), this, SLOT(updateSampleTable()));
        
        connect(m_sample_start, SIGNAL(valueChanged(int)), this, SLOT(sampleStartChanged(int)));
        connect(m_sample_start_fine, SIGNAL(valueChanged(int)), this, SLOT(sampleStartFineChanged(int)));
        connect(m_sample_end, SIGNAL(valueChanged(int)), this, SLOT(sampleEndChanged(int)));
        connect(m_sample_end_fine, SIGNAL(valueChanged(int)), this, SLOT(sampleEndFineChanged(int)));
        
        connect(m_loop_start, SIGNAL(valueChanged(int)), this, SLOT(loopStartChanged(int)));
        connect(m_loop_start_fine, SIGNAL(valueChanged(int)), this, SLOT(loopStartFineChanged(int)));
        connect(m_loop_end, SIGNAL(valueChanged(int)), this, SLOT(loopEndChanged(int)));
        connect(m_loop_end_fine, SIGNAL(valueChanged(int)), this, SLOT(loopEndFineChanged(int)));
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
    
    /*
    m_sample_viewer->lms_sample_graph->lms_graphs[m_sample_table->lms_selected_column].fill();
    
    printf("set sample index\n");
        
    info.format = 0;
    file = sf_open(path.toLocal8Bit(), SFM_READ, &info);
    printf("Opened SNDFILE\n");
    if (file && info.frames > 0) {

	float binSize = (float)info.frames / m_previewWidth;
	float peak[2] = { 0.0f, 0.0f }, mean[2] = { 0.0f, 0.0f };
	float *frame = (float *)malloc(info.channels * sizeof(float));
	int bin = 0;
        
	QPainter paint(&(m_sample_viewer->lms_sample_graph->lms_graphs[m_sample_table->lms_selected_column]));

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
        
        m_sample_counts[m_sample_table->lms_selected_column] = info.frames;
        
     * */
     
        /*
        m_sample_start_fine->setMaximum(info.frames);
        m_sample_end_fine->setMaximum(info.frames);
        m_loop_start_fine->setMaximum(info.frames);
        m_loop_end_fine->setMaximum(info.frames);
        */
        
        /*Set seconds*/
        /*
        QTableWidgetItem *f_set_seconds = new QTableWidgetItem;
        QString * f_seconds = new QString();                
        f_seconds->setNum((float(info.frames) / float(info.samplerate)));
        f_set_seconds->setText(*f_seconds);
         * */
        //m_sample_table->setItem(m_selected_sample_index, 11, f_set_seconds);
        
        /*Set samples*/
        /*
        QTableWidgetItem *f_set_samples = new QTableWidgetItem;
        QString * f_samples = new QString();                
        f_samples->setNum((info.frames));
        f_set_samples->setText(*f_samples);
        //m_sample_table->setItem(m_selected_sample_index, 12, f_set_samples);
        */
    
        /*Trigger start/end changes to update m_sample_table*/
        
        /*
        sampleStartChanged(m_sample_start->value());
        sampleEndChanged(m_sample_start->value());
        loopStartChanged(m_sample_start->value());
        loopEndChanged(m_sample_start->value());
        */
        
	//std::cout << "duration " << duration << std::endl;
	
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

    //} else {
        /*
	m_duration->setText("0.00 sec");
	m_sampleRate->setText("");
	m_channels->setText("");
         */
    //}
    
    //if (file) sf_close(file);

    //m_preview->setPixmap(pmap);
    //m_sample_graph->setPixmap(m_sample_graphs[m_selected_sample_index]);
    
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
    
    //m_file_path->setText(file);
    m_file = file;
    
    updateSampleTable();
    
    generatePreview(file);
    
    m_sample_table->lms_mod_matrix->resizeColumnsToContents();
    
    m_suppressHostUpdate = false;
}

void SamplerGUI::updateSampleTable()
{     
    
}


void SamplerGUI::setSelection(int a_value)
{
    m_suppressHostUpdate = true;
    //m_selected_sample[a_value]->setChecked(true);
    m_suppressHostUpdate = false;
}

void SamplerGUI::setSampleStart(int a_value)
{
    m_suppressHostUpdate = true;
    //m_sample_start->setValue(a_value);
    m_suppressHostUpdate = false;
}

void SamplerGUI::setSampleEnd(int a_value)
{
    m_suppressHostUpdate = true;
    //m_sample_end->setValue(a_value);
    m_suppressHostUpdate = false;
}

void SamplerGUI::setSampleStartFine(int a_value)
{
    m_suppressHostUpdate = true;
    //m_sample_start_fine->setValue(a_value);
    m_suppressHostUpdate = false;
}

void SamplerGUI::setSampleEndFine(int a_value)
{
    m_suppressHostUpdate = true;
    //m_sample_end_fine->setValue(a_value);
    m_suppressHostUpdate = false;
}

void SamplerGUI::setLoopStart(int a_value)
{
    m_suppressHostUpdate = true;
    //m_loop_start->setValue(a_value);
    m_suppressHostUpdate = false;
}

void SamplerGUI::setLoopEnd(int a_value)
{
    m_suppressHostUpdate = true;
    //m_loop_end->setValue(a_value);
    m_suppressHostUpdate = false;
}

void SamplerGUI::setLoopStartFine(int a_value)
{
    m_suppressHostUpdate = true;
    //m_loop_start_fine->setValue(a_value);
    m_suppressHostUpdate = false;
}

void SamplerGUI::setLoopEndFine(int a_value)
{
    m_suppressHostUpdate = true;
    //m_loop_end_fine->setValue(a_value);
    m_suppressHostUpdate = false;
}



void SamplerGUI::fileSelect()
{
    QString orig = m_file;
    if (orig.isEmpty()) {
	if (!m_projectDir.isEmpty()) {
	    orig = m_projectDir;
	} else {
	    orig = ".";
	}
    }
    
    QString path = m_file_selector->button_pressed(this);
    
    if(!path.isEmpty())
    {
        m_sample_table->find_selected_radio_button(SMP_TB_RADIOBUTTON_INDEX);    
        m_sample_viewer->generatePreview(path, m_sample_table->lms_selected_column);
    }

}

void SamplerGUI::selectionChanged()
{
    m_sample_table->find_selected_radio_button(SMP_TB_RADIOBUTTON_INDEX);
#ifndef LMS_DEBUG_STANDALONE        
    if (!m_suppressHostUpdate) {        
	lo_send(m_host, m_controlPath, "if", Sampler_SELECTED_SAMPLE, (float)(m_sample_table->lms_selected_column));
    }
#endif
    /*
    //These 2 will never be null, and should be set regardless of whether a sample is loaded
    m_sample_graph->setPixmap(m_sample_graphs[m_selected_sample_index]);
    m_sample_num->setValue(m_selected_sample_index);
    
    QTableWidgetItem * f_item_file = m_sample_table->item(m_selected_sample_index, 1);    
    
    //If no file loaded, then the selection is empty.  Clear the file path and don't bother setting any other widgets
    if(!f_item_file)
    {
        m_file_path->setText("");
        return;
    }
    
    m_file_path->setText((f_item_file->text()));
        
     * */
    /* TODO:  Start, End, Loop Start, Loop End
     */
}

void SamplerGUI::sampleStartChanged(int a_value)
{   

}

void SamplerGUI::sampleEndChanged(int a_value)
{
    
}

void SamplerGUI::sampleStartFineChanged(int a_value)
{
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

    if (!strcmp(key, "load")) {
	gui->setSampleFile(QString::fromLocal8Bit(value));
    } else if (!strcmp(key, DSSI_PROJECT_DIRECTORY_KEY)) {
	//gui->setProjectDirectory(QString::fromLocal8Bit(value));
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

int main(int argc, char **argv)
{
    cerr << "trivial_sampler_qt_gui starting..." << endl;

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

