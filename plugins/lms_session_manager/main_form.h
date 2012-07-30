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

class main_form : public QWidget
{
    public:
    main_form()
    {
        QRect f_rect(0, 0, 800, 600);
        this->setGeometry(f_rect);
        this->setStyleSheet(QString("background-color: white; color:black;"));
    }   
    
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

