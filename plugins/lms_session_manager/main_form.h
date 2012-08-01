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
#include "../libmodsynth/widgets/lms_main_layout.h"
#include "../libmodsynth/widgets/mod_matrix.h"

#define LMS_INSTRUMENT_COUNT 16

class main_form : public QWidget
{
    public:
    main_form()
    {
        QRect f_rect(0, 0, 450, 600);
        this->setGeometry(f_rect);
        //this->setStyleSheet(QString("background-color: white; color:black;"));
        this->setWindowTitle(QString("LMS Session Manager"));
                
        
        QList<LMS_mod_matrix_column*> f_columns;
        f_columns << new LMS_mod_matrix_column(radiobutton, QString("MIDI Keyboard"), 0, 1,0);
        f_columns << new LMS_mod_matrix_column(QStringList() << QString("None") << QString("Euphoria") << QString("Ray-V"), QString("Plugin"));
        f_columns << new LMS_mod_matrix_column(QStringList(), QString("MIDI Input"));
        f_columns << new LMS_mod_matrix_column(QStringList(), QString("Audio Output"));
        
        LMS_style_info * f_style = new LMS_style_info(45);
        
        main_layout = new LMS_main_layout(this);
        
        QLabel * f_midi_kbd_label = new QLabel(this);
        f_midi_kbd_label->setText(QString("MIDI Keyboard:"));
        midi_keyboard_combobox = new QComboBox(this);
        
        main_layout->lms_add_widget(f_midi_kbd_label);
        main_layout->lms_add_widget(midi_keyboard_combobox);
        
        main_layout->lms_add_layout();
        
        mod_matrix = new LMS_mod_matrix(this, LMS_INSTRUMENT_COUNT, f_columns, 0, f_style);        
        main_layout->lms_add_widget(mod_matrix->lms_mod_matrix);
                
    }   
    
    QComboBox * midi_keyboard_combobox;
    LMS_mod_matrix * mod_matrix;
    LMS_main_layout * main_layout;
    
    void open_session_file()
    {
        
    }
    
    void save_session_file()
    {
        
    }
    
    void launch_application()
    {
        
    }
    
};

#endif	/* MAIN_FORM_H */

