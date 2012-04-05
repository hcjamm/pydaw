/* 
 * File:   lms_file_select.h
 * Author: Jeff Hubbard
 * 
 * A button for opening a file dialog, with a read-only lineedit for viewing the path
 *
 * Created on April 4, 2012, 11:31 PM
 */

#ifndef LMS_FILE_SELECT_H
#define	LMS_FILE_SELECT_H

#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>

class LMS_file_select
{
public:
    QHBoxLayout * lms_layout;
    QPushButton * lms_open_button;
    QLineEdit * lms_file_path;
    
    LMS_file_select(QWidget * a_parent)
    {
        lms_layout = new QHBoxLayout();
        lms_open_button = new QPushButton(a_parent);
        lms_open_button->setText(QString("Open"));
        lms_file_path = new QLineEdit(a_parent);
        
        lms_layout->addWidget(lms_file_path);
        lms_layout->addWidget(lms_open_button);
    }
    
    QString lms_get_file()
    {
        return lms_file_path->text();
    }
    
    void lms_set_file(QString a_file)
    {
        lms_file_path->setText(a_file);
    }
    
};

#endif	/* LMS_FILE_SELECT_H */

