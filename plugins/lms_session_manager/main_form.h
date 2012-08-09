/* 
 * File:   main_form.h
 * Author: jeffh
 *
 * Created on July 29, 2012, 11:06 PM
 */

#ifndef MAIN_FORM_H
#define	MAIN_FORM_H

#include <QWidget>
#include <QLabel>
#include <QString>
#include <QMenuBar>
#include <QTableWidget>
#include <QProcess>
#include <QRadioButton>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>

#include "../libmodsynth/widgets/lms_main_layout.h"

extern "C"
{
#include <jack/jack.h>
#include <alsa/asoundlib.h>
}


#define LMS_INSTRUMENT_COUNT 16

class main_form : public QWidget
{
    Q_OBJECT
    
    public:
    main_form()
    {
        QRect f_rect(0, 0, 360, 600);
        this->setGeometry(f_rect);
        //this->setStyleSheet(QString("background-color: white; color:black;"));
        this->setWindowTitle(QString("LMS Session Manager"));
        
        main_layout = new LMS_main_layout(this);
        /*
        QLabel * f_midi_kbd_label = new QLabel(this);
        f_midi_kbd_label->setText(QString("MIDI Keyboard:"));
        midi_keyboard_combobox = new QComboBox(this);
        
        main_layout->lms_add_widget(f_midi_kbd_label);
        main_layout->lms_add_widget(midi_keyboard_combobox);
        
        main_layout->lms_add_layout();
        */
        
        save_button = new QPushButton();        
        save_button->setText(QString("Save"));
        
        main_layout->lms_add_widget(save_button);
        
        open_button = new QPushButton();
        open_button->setText(QString("Open"));
        
        main_layout->lms_add_widget(open_button);
        
        main_layout->lms_add_layout();
        
        table_widget = new QTableWidget(LMS_INSTRUMENT_COUNT, 2, this);
        table_widget->setHorizontalHeaderLabels(QStringList() //<< QString("Keyboard") 
                << QString("Instrument") << QString("Instance Name")); // << QString("MIDI In") << QString("Audio Out"));
        
        main_layout->lms_add_widget(table_widget);
        
        for(int i = 0; i < LMS_INSTRUMENT_COUNT; i++)
        {
            processes[i] = new QProcess(this);
            /*
            select_midi_keyboard[i] = new QRadioButton(table_widget);
            
            table_widget->setCellWidget(i, 0, select_midi_keyboard[i]);
            */
            select_instrument[i] = new QComboBox();
            
            select_instrument[i]->addItems(QStringList() << QString("None") << QString("Euphoria") << QString("Ray-V"));
            
            table_widget->setCellWidget(i, 0, select_instrument[i]);
            
            instance_names[i] = new QLineEdit(table_widget);
            
            instance_names[i]->setMinimumWidth(180);
            
            table_widget->setCellWidget(i, 1, instance_names[i]);
            
            /*
            select_midi_input[i] = new QComboBox();
            
            table_widget->setCellWidget(i, 3, select_midi_input[i]);
            
            select_audio_output[i] = new QComboBox();
            
            table_widget->setCellWidget(i, 4, select_audio_output[i]);
            */
        }
        
        //select_midi_keyboard[0]->setChecked(TRUE);
        
        table_widget->resizeColumnsToContents();
        
        connect(select_instrument[0], SIGNAL(currentIndexChanged(int)), this, SLOT(instrument_index_changed0(int)));
        connect(select_instrument[1], SIGNAL(currentIndexChanged(int)), this, SLOT(instrument_index_changed1(int)));
        connect(select_instrument[2], SIGNAL(currentIndexChanged(int)), this, SLOT(instrument_index_changed2(int)));
        connect(select_instrument[3], SIGNAL(currentIndexChanged(int)), this, SLOT(instrument_index_changed3(int)));
        connect(select_instrument[4], SIGNAL(currentIndexChanged(int)), this, SLOT(instrument_index_changed4(int)));
        connect(select_instrument[5], SIGNAL(currentIndexChanged(int)), this, SLOT(instrument_index_changed5(int)));
        connect(select_instrument[6], SIGNAL(currentIndexChanged(int)), this, SLOT(instrument_index_changed6(int)));
        connect(select_instrument[7], SIGNAL(currentIndexChanged(int)), this, SLOT(instrument_index_changed7(int)));
        connect(select_instrument[8], SIGNAL(currentIndexChanged(int)), this, SLOT(instrument_index_changed8(int)));
        connect(select_instrument[9], SIGNAL(currentIndexChanged(int)), this, SLOT(instrument_index_changed9(int)));
        connect(select_instrument[10], SIGNAL(currentIndexChanged(int)), this, SLOT(instrument_index_changed10(int)));
        connect(select_instrument[11], SIGNAL(currentIndexChanged(int)), this, SLOT(instrument_index_changed11(int)));
        connect(select_instrument[12], SIGNAL(currentIndexChanged(int)), this, SLOT(instrument_index_changed12(int)));
        connect(select_instrument[13], SIGNAL(currentIndexChanged(int)), this, SLOT(instrument_index_changed13(int)));
        connect(select_instrument[14], SIGNAL(currentIndexChanged(int)), this, SLOT(instrument_index_changed14(int)));
        connect(select_instrument[15], SIGNAL(currentIndexChanged(int)), this, SLOT(instrument_index_changed15(int)));
        
        
        
        
        
        QTimer *myTimer = new QTimer(this);
        connect(myTimer, SIGNAL(timeout()), this, SLOT(timer_polling()));
        myTimer->setSingleShot(false);
        myTimer->setInterval(10000);
        myTimer->start();        
    }   
    
    QString project_directory;
    QString project_name;
    
    QProcess * processes[LMS_INSTRUMENT_COUNT];
    
    QPushButton * open_button;
    QPushButton * save_button;
    
    QComboBox * midi_keyboard_combobox;
    QTableWidget * table_widget;
    QComboBox * select_instrument[LMS_INSTRUMENT_COUNT];
    QLineEdit * instance_names[LMS_INSTRUMENT_COUNT];
    QComboBox * select_midi_input[LMS_INSTRUMENT_COUNT];
    QComboBox * select_audio_output[LMS_INSTRUMENT_COUNT];
    QRadioButton * select_midi_keyboard[LMS_INSTRUMENT_COUNT];
    LMS_main_layout * main_layout;
            
    void instrument_index_changed(int a_instrument_index, int a_index)
    {
        QStringList f_args = QStringList() << QString("-a") << QString("-c") << QString("project-name-") + instance_names[a_instrument_index]->text();
        
        switch(a_index)
        {
            case 0:
                processes[a_instrument_index]->kill();
                break;
            case 1:
                f_args << QString("euphoria.so");
                processes[a_instrument_index]->start(QString("lms-jack-dssi-host"), f_args);
                break;
            case 2:
                f_args << QString("ray_v.so");
                processes[a_instrument_index]->startDetached(QString("lms-jack-dssi-host"), f_args);
                break;
        }
    }
    
    void open_session_file()
    {
        
    }
    
    void save_session_file()
    {
        
    }
    
public slots:
    void save_button_pressed()
    {
        save_session_file();
    }
    
    void open_button_pressed()
    {
        open_session_file();
    }
    
    void instrument_index_changed0(int a_index){instrument_index_changed(0, a_index);};
    void instrument_index_changed1(int a_index){instrument_index_changed(1, a_index);};
    void instrument_index_changed2(int a_index){instrument_index_changed(2, a_index);};
    void instrument_index_changed3(int a_index){instrument_index_changed(3, a_index);};
    void instrument_index_changed4(int a_index){instrument_index_changed(4, a_index);};
    void instrument_index_changed5(int a_index){instrument_index_changed(5, a_index);};
    void instrument_index_changed6(int a_index){instrument_index_changed(6, a_index);};
    void instrument_index_changed7(int a_index){instrument_index_changed(7, a_index);};
    void instrument_index_changed8(int a_index){instrument_index_changed(8, a_index);};
    void instrument_index_changed9(int a_index){instrument_index_changed(9, a_index);};
    void instrument_index_changed10(int a_index){instrument_index_changed(10, a_index);};
    void instrument_index_changed11(int a_index){instrument_index_changed(11, a_index);};
    void instrument_index_changed12(int a_index){instrument_index_changed(12, a_index);};
    void instrument_index_changed13(int a_index){instrument_index_changed(13, a_index);};
    void instrument_index_changed14(int a_index){instrument_index_changed(14, a_index);};
    void instrument_index_changed15(int a_index){instrument_index_changed(15, a_index);};
        
    
    
    void timer_polling()
    {
        
    }
};

#endif	/* MAIN_FORM_H */

