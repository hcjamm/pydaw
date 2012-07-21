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
#include <QFileInfo>
#include <QStringList>
#include <QString>
#include <QTextStream>
#include <QMessageBox>

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
    QString lms_last_directory;
    
    LMS_file_select(QWidget * a_parent)
    {
        lms_layout = new QHBoxLayout();
        lms_open_button = new QPushButton(a_parent);
        lms_open_button->setText(QString("Open"));
        lms_open_button->setMaximumWidth(60);
        lms_clear_button = new QPushButton(a_parent);
        lms_clear_button->setText(QString("Clear"));
        lms_clear_button->setMaximumWidth(60);
        lms_open_in_editor_button = new QPushButton(a_parent);
        lms_open_in_editor_button->setText("Edit");
        lms_open_in_editor_button->setMaximumWidth(60);
        lms_reload_button = new QPushButton(a_parent);
        lms_reload_button->setText(QString("Reload"));
        lms_reload_button->setMaximumWidth(60);
        lms_file_path = new QLineEdit(a_parent);
        lms_file_path->setReadOnly(TRUE);
        lms_file_path->setMinimumWidth(360);
        lms_last_directory = QString("");
        
        lms_editor_path = QString("/usr/bin/audacity");
        
        QString f_global_config_path(QDir::homePath() + QString("/dssi/lms_global_wave_editor.txt"));

        //QByteArray ba = f_global_config_path.toLocal8Bit();
        
        if(QFile::exists(f_global_config_path))
        {
            QFile f_file(f_global_config_path);
            QTextStream f_in(&f_file);
            f_file.open(QIODevice::ReadOnly | QIODevice::Text);
            lms_editor_path = f_in.readLine();
            
            f_file.close();
            
            if((!QFile::exists(lms_editor_path)) && (QFile::exists(QString("/usr/bin/audacity"))))
            {
                //TODO:  notify?
                //Revert to Audacity
                lms_editor_path = QString("/usr/bin/audacity");
            }
        }
        else
        {
            QFile f_file(f_global_config_path);
            f_file.open(QIODevice::WriteOnly | QIODevice::Text);
            QTextStream f_out(&f_file);
            f_out << "/usr/bin/audacity";
            f_out.flush();
            f_file.close(); 
        }
        
        
        lms_layout->addWidget(lms_file_path);
        lms_layout->addWidget(lms_clear_button);
        lms_layout->addWidget(lms_open_button);
        lms_layout->addWidget(lms_open_in_editor_button);
        lms_layout->addWidget(lms_reload_button);
    }
    
    QString open_button_pressed(QWidget * a_parent)
    {
        QString f_result = QFileDialog::getOpenFileName(a_parent, "Select an audio sample file", lms_last_directory, "Audio files (*.wav *.aiff)");
        if(!f_result.isEmpty())
        {
            lms_file_path->setText(f_result);
            
            QFileInfo f_fi(f_result);
            lms_last_directory = f_fi.absolutePath();
        }
        
        return lms_file_path->text();
    }
    
    /* Return multiple files
     */
    QStringList open_button_pressed_multiple(QWidget * a_parent)
    {
        QStringList f_result = QFileDialog::getOpenFileNames(a_parent, "Select one or more audio sample files", lms_last_directory, "Audio files (*.wav *.aiff *.ogg);;All files (*)");
        if(!f_result.isEmpty())
        {
            lms_file_path->setText(f_result[(f_result.count() - 1)]);
            
            QFileInfo f_fi(f_result[0]);
            lms_last_directory = f_fi.absolutePath();
        }
        
        return f_result;
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
        if(!QFile::exists(lms_editor_path))
        {
            QMessageBox::warning(lms_file_path, QString("No Wave Editor Found"), QString("Could not locate ") + lms_editor_path +
            QString(" or another suitable wave editor.  Please edit ~/dssi/lms_global_wave_editor.txt with your wave editor of choice, or install Audacity."));
            return;
        }
        
        QStringList commandAndParameters;

        QString f_file_path = lms_file_path->text();
        
        if(f_file_path.isEmpty())
        {
            return;
        }
        
	commandAndParameters << f_file_path;

	QProcess myProcess;

	myProcess.startDetached(lms_editor_path, commandAndParameters);
    }        
};

#endif	/* LMS_FILE_SELECT_H */

