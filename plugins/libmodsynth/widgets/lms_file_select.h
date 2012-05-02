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
#include <QFileDialog>
#include <QProcess>

class LMS_file_select
{
public:
    QHBoxLayout * lms_layout;
    QPushButton * lms_open_button;
    QPushButton * lms_clear_button;
    QPushButton * lms_open_in_editor_button;
    QPushButton * lms_reload_button;
    QLineEdit * lms_file_path;
    QString lms_editor_path;
    
    LMS_file_select(QWidget * a_parent)
    {
        lms_layout = new QHBoxLayout();
        lms_open_button = new QPushButton(a_parent);
        lms_open_button->setText(QString("Open"));
        lms_clear_button = new QPushButton(a_parent);
        lms_clear_button->setText(QString("Clear"));
        lms_open_in_editor_button = new QPushButton(a_parent);
        lms_open_in_editor_button->setText("Edit");
        lms_reload_button = new QPushButton(a_parent);
        lms_reload_button->setText(QString("Reload"));
        lms_file_path = new QLineEdit(a_parent);
        lms_file_path->setReadOnly(TRUE);
        
        /*TODO:  Read this from a to-be-created LMS global config file*/
        lms_editor_path = QString("/usr/bin/audacity");
        
        lms_layout->addWidget(lms_file_path);
        lms_layout->addWidget(lms_clear_button);
        lms_layout->addWidget(lms_open_button);
        lms_layout->addWidget(lms_open_in_editor_button);
        lms_layout->addWidget(lms_reload_button);
    }
    
    QString open_button_pressed(QWidget * a_parent)
    {
        QString f_result = QFileDialog::getOpenFileName(a_parent, "Select an audio sample file", ".", "Audio files (*.wav *.aiff)");
        lms_file_path->setText(f_result);
        return lms_file_path->text();
    }
    
    void clear_button_pressed()
    {        
        lms_file_path->setText(QString(""));        
    }
    
    QString lms_get_file()
    {
        return lms_file_path->text();
    }
    
    void lms_set_file(QString a_file)
    {
        lms_file_path->setText(a_file);
    }
    
    void open_in_editor_button_pressed(QWidget * a_parent)
    {
        QStringList commandAndParameters;

        QString f_file_path = lms_file_path->text();
        
	commandAndParameters << f_file_path;

	QProcess * myProcess = new QProcess(a_parent);

	myProcess->startDetached(lms_editor_path, commandAndParameters);
    }        
};

#endif	/* LMS_FILE_SELECT_H */

