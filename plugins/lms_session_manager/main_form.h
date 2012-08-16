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
#include <QTextStream>
#include <QTableWidget>
#include <QProcess>
#include <QRadioButton>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>

#include "../libmodsynth/widgets/lms_main_layout.h"

extern "C"
{
//#include <jack/jack.h>
#include <alsa/asoundlib.h>
}

static QTextStream cerr(stderr);

#define LMS_NOTIFY_DIRECTORY QString("/notify/")

#define LMS_DELIMITER QString("|")

#define LMS_INSTRUMENT_COUNT 16

class main_form : public QWidget
{
    Q_OBJECT
    
    public:
        
    //TODO:  Ensure that a2jmidid is running at startup
        
    main_form(QString a_project_path)
    {
        first_quit = TRUE;
        
        full_project_file_path = a_project_path;
        
        QStringList f_project_path_arr = a_project_path.split(QString("/"));
        
        project_directory = QString("");
        
        for(int i = 1; i < f_project_path_arr.count(); i++)            
        {
            if(i == (f_project_path_arr.count() - 1))
            {
                project_name = f_project_path_arr.at(i).split(QString(".pss")).at(0);
            }
            else
            {
                project_directory.append(QString("/"));
                project_directory.append(f_project_path_arr.at(i));
            }
            
        }
        
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
        connect(save_button, SIGNAL(pressed()), this, SLOT(save_button_pressed()));
        
        main_layout->lms_add_widget(save_button);
                
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
            instance_names[i]->setText(QString("instrument") + QString::number(i));
            
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

        
        QString f_notify_dir = project_directory + LMS_NOTIFY_DIRECTORY;

        if(!QFile::exists(f_notify_dir))
        {
            //cerr << f_file_info.dir().absolutePath() + QString("/.notify/");
            QDir f_dir(project_directory);

            f_dir.mkpath(f_notify_dir);
        }

        supress_instrument_change = FALSE;
                
        if(QFile::exists(full_project_file_path))
        {            
            QFile file(full_project_file_path);

            //QFileInfo f_file_info(full_project_file_path);

            //project_directory = f_file_info.dir().absolutePath();

            if(!file.open(QIODevice::ReadOnly)) {
                QMessageBox::information(0, "error", file.errorString());
                return;
            }

            QTextStream in(&file);

            int f_count = 0;

            while((!in.atEnd()) && (f_count < LMS_INSTRUMENT_COUNT))
            {
                QString line = in.readLine();

                QStringList f_line_list = line.split(LMS_DELIMITER);

                instance_names[f_count]->setText(f_line_list.at(1));  //This must be set before select_instrument
                select_instrument[f_count]->setCurrentIndex(f_line_list.at(0).toInt());                

                f_count++;
            }
        }
        else
        {
            //Do nothing, or maybe create a new default file?
        }
                
        QTimer *myTimer = new QTimer(this);
        connect(myTimer, SIGNAL(timeout()), this, SLOT(timer_polling()));
        myTimer->setSingleShot(false);
        myTimer->setInterval(10000);
        myTimer->start();        
    }   
    
    QString project_directory;
    QString project_name;
    QString full_project_file_path;
    
    QProcess * processes[LMS_INSTRUMENT_COUNT];
    
    QPushButton * save_button;
    
    QComboBox * midi_keyboard_combobox;
    QTableWidget * table_widget;
    QComboBox * select_instrument[LMS_INSTRUMENT_COUNT];
    QLineEdit * instance_names[LMS_INSTRUMENT_COUNT];
    QComboBox * select_midi_input[LMS_INSTRUMENT_COUNT];
    QComboBox * select_audio_output[LMS_INSTRUMENT_COUNT];
    QRadioButton * select_midi_keyboard[LMS_INSTRUMENT_COUNT];
    LMS_main_layout * main_layout;
    
    bool first_quit;    
    bool supress_instrument_change;
            
    void instrument_index_changed(int a_instrument_index, int a_index)
    {
        if(supress_instrument_change)
        {
            return;
        }
        
        if(instance_names[a_instrument_index]->text().isEmpty())
        {
            QMessageBox::warning(this, QString("Error"), QString("The instance name must not be empty"));
            return;
        }
        
        //TODO:  Figure out how best to handle attempts at going between different instruments...  Invoke the quit handler, etc..
        
        instance_names[a_instrument_index]->setText(instance_names[a_instrument_index]->text().trimmed());
                        
        for(int i = 0; i < LMS_INSTRUMENT_COUNT; i++)
        {
            if((i != a_instrument_index) && (select_instrument[i]->currentIndex() > 0))
            {
                if(!(instance_names[a_instrument_index]->text()).compare(instance_names[i]->text(), Qt::CaseInsensitive))
                {
                    QMessageBox::warning(this, QString("Error"), QString("Instance names must be unique."));
                    supress_instrument_change = TRUE;
                    select_instrument[a_instrument_index]->setCurrentIndex(0);
                    supress_instrument_change = FALSE;
                    return;
                }
            }
        }
        
        QStringList f_args = QStringList() << QString("-a") <<  QString("-p") << project_directory << QString("-c") << project_name + QString("-") + instance_names[a_instrument_index]->text();
        
        switch(a_index)
        {
            case 0:
                //TODO:  Send the quit signal
                instance_names[a_instrument_index]->setEnabled(TRUE);
                break;
            case 1:
                f_args << QString("euphoria.so");
                processes[a_instrument_index]->startDetached(QString("lms-jack-dssi-host"), f_args);
                instance_names[a_instrument_index]->setEnabled(FALSE);
                break;
            case 2:
                f_args << QString("ray_v.so");
                processes[a_instrument_index]->startDetached(QString("lms-jack-dssi-host"), f_args);
                instance_names[a_instrument_index]->setEnabled(FALSE);
                break;
        }
    }
    
    void save_session_file()
    {   
        QFile file( full_project_file_path );
        if ( file.open(QIODevice::ReadWrite) )
        {
            QTextStream stream( &file );
         
            for(int f_i = 0; f_i < LMS_INSTRUMENT_COUNT; f_i++)
            {
                stream <<
                QString::number(select_instrument[f_i]->currentIndex()) << LMS_DELIMITER <<
                instance_names[f_i]->text() <<  QString("\n");
            }            
            
            file.close();            
        }        
        
        QFileInfo f_file_info(full_project_file_path);
        
        project_directory = f_file_info.dir().absolutePath();
        
        QString f_notify_dir = project_directory + LMS_NOTIFY_DIRECTORY;
        
        if(!QFile::exists(f_notify_dir))
        {
            //cerr << f_file_info.dir().absolutePath() + QString("/.notify/");
            QDir f_dir(project_directory);
            
            f_dir.mkpath(f_notify_dir);
        }
        
        for(int f_i = 0; f_i < LMS_INSTRUMENT_COUNT; f_i++)
        {
            if(select_instrument[f_i]->currentIndex() > 0)
            {
                QFile f_save_file( f_notify_dir + project_name + QString("-") + instance_names[f_i]->text() + QString(".save"));
                if ( f_save_file.open(QIODevice::WriteOnly | QIODevice::Text) )
                {
                    QTextStream stream( &f_save_file );
                    stream << "Created by LMS Session Manager\n";
                    stream.flush();
                }
                else
                {
                    cerr << "Failed to open file.\n";
                }
                
                f_save_file.close();

            }
        }        
    }
    
public slots:
    void save_button_pressed()
    {
        save_session_file();
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
    
    void quitHandler()
    {
        /*
        if(first_quit)
        {
            first_quit = FALSE;
            return;
        }
        */
        
        cerr << QString("Quitting...\n");
        
        QString f_notify_dir = project_directory + LMS_NOTIFY_DIRECTORY;
        
        for(int f_i = 0; f_i < LMS_INSTRUMENT_COUNT; f_i++)
        {
            if(select_instrument[f_i]->currentIndex() > 0)
            {
                QString f_quit_file_path = f_notify_dir + project_name + QString("-") + instance_names[f_i]->text() + QString(".quit");
                
                QFile f_quit_file(f_quit_file_path);
                if ( f_quit_file.open(QIODevice::WriteOnly | QIODevice::Text) )
                {
                    QTextStream stream( &f_quit_file );
                    stream << "Created by LMS Session Manager\n";
                    stream.flush();
                    
                    f_quit_file.close();
                }
                else
                {
                    cerr << "quitHandler failed to open file " << f_quit_file_path << "\n";
                }                
            }
        }
    }
};

class first_window : public QWidget
{
    Q_OBJECT
    
public:
    first_window(QApplication * a_qapp)
    {
        qapp = a_qapp;
        
        main_layout = new LMS_main_layout(this);
        
        text_label = new QLabel(this);
        
        text_label->setText(QString("Open an existing .pss file, or create a new project."));
        
        main_layout->lms_add_widget(text_label);
        
        main_layout->lms_add_layout();
        
        new_button = new QPushButton();        
        new_button->setText(QString("New"));
        connect(new_button, SIGNAL(pressed()), this, SLOT(new_button_pressed()));
        
        main_layout->lms_add_widget(new_button);
        
        open_button = new QPushButton();
        open_button->setText(QString("Open"));
        connect(open_button, SIGNAL(pressed()), this, SLOT(open_button_pressed()));
        
        main_layout->lms_add_widget(open_button);
    }
    
    ~first_window()
    {
        delete new_button;
        delete new_button;
        delete text_label;
        delete main_layout;
    }
    
    LMS_main_layout * main_layout;
    QPushButton * new_button;
    QPushButton * open_button;
    QLabel * text_label;
    QApplication * qapp;
            
public slots:
    void open_button_pressed()
    {
        QString f_selected_path = QFileDialog::getOpenFileName(this, "Select a session file to open...", ".", "Protractor Session Files (*.pss)");  
                
        if(f_selected_path.isEmpty())
        {
            return;
        }
        
        main_form * f_form = new main_form(f_selected_path);
        QObject::connect(qapp, SIGNAL(aboutToQuit()), f_form, SLOT(quitHandler()));
        f_form->show();
        
        this->close();
    }
    
    void new_button_pressed()
    {
        QString f_selected_path = QFileDialog::getSaveFileName(this, "Select an file to save the session to...", ".", "Protractor Session Files (*.pss)");
                
        if(f_selected_path.isEmpty())
        {
            return;
        }
        
        if(!f_selected_path.endsWith(QString(".pss"), Qt::CaseSensitive))
        {
            f_selected_path.append(QString(".pss"));
        }
                
        main_form * f_form = new main_form(f_selected_path);
        QObject::connect(qapp, SIGNAL(aboutToQuit()), f_form, SLOT(quitHandler()));    
        f_form->show();
        
        this->close();
    }
};

#endif	/* MAIN_FORM_H */

