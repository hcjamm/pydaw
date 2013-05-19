/*
This file is part of the PyDAW project, Copyright PyDAW Team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef LMS_PRESET_MGR_H
#define	LMS_PRESET_MGR_H

#include <QComboBox>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QTextOStream>
#include <QMessageBox>
#include <QPushButton>
#include <QList>

#include "group_box.h"
#include "lms_control.h"

#define LMS_PRESETS_EXT QString(".pypresets")

class LMS_preset_manager : public LMS_control
{
public:
    /* The overload for running standalone */
    LMS_preset_manager(QString a_plugin_name, QString a_default_presets, int a_lms_port, LMS_style_info * a_style_info, QWidget * a_parent)
    {
        QString f_preset_path = QDir::homePath() + QString("/pydaw3");
        
        instantiate(a_plugin_name, a_default_presets, a_lms_port, a_style_info, a_parent, f_preset_path);
    }
    
    /* The overload for running in LMS Session Manager */
    LMS_preset_manager(QString a_instance_name, QString a_default_presets, int a_lms_port, 
            LMS_style_info * a_style_info, QWidget * a_parent, QString a_preset_directory)
    {
        instantiate(a_instance_name, a_default_presets, a_lms_port, a_style_info, a_parent, a_preset_directory);
    }
    
    QHBoxLayout * lms_layout;
        
    QComboBox * m_program;
    QGroupBox * lms_group_box;
    QPushButton *m_prog_save;
    QStringList presets_tab_delimited;
    QString lms_plugin_name;
    QList<LMS_control*> lms_controls;
    QWidget * lms_parent;
    
    void lms_add_control(LMS_control * a_control)
    {
        lms_controls.append(a_control);
    }
    
    void lms_set_value(int a_value)
    {        
        m_program->setCurrentIndex(a_value);
    }
    
    int lms_get_value()
    {
        return m_program->currentIndex();
    }
    
    QLayout * lms_get_layout()
    {
        return lms_layout;
    }
    
    QWidget * lms_get_widget()
    {
        return m_program;
    }
    
    void lms_value_changed(int a_value)    
    {
        /*We intentionally leave the first preset empty*/
        if(a_value == 0)
            return;
        
        if(a_value < 0 || a_value >= 128)
        {
            QMessageBox::warning(lms_group_box, QString("Warning"), QString("presets.h::lms_value_changed called with invalid a_value: ") + QString::number(a_value));
            return;
        }

        if(presets_tab_delimited[a_value].compare("empty") != 0)
        {
            QStringList f_preset_values = presets_tab_delimited[a_value].split("\t");
            
            for(int f_i = 0; f_i < lms_controls.count(); f_i++)
            {
                if((f_i + 1) >= f_preset_values.count())
                {
                    break;
                }
                
                int f_preset_value_int = f_preset_values.at(f_i + 1).toInt();

                lms_controls.at(f_i)->lms_set_value(f_preset_value_int);
                //v_control_changed(f_i, f_preset_value_int, false);
            }
        }
    }
    
    /* Used as a slot for the save button pressed signal */
    void programSaved()
    {
        int f_compare_text = m_program->currentText().compare("empty");

        if((m_program->currentIndex()) == 0)
        {
            QMessageBox * f_qmess = new QMessageBox(lms_parent);

            QString f_compare_text_string = QString("");
            f_compare_text_string.setNum(f_compare_text);

            f_compare_text_string.append("  \"" + presets_tab_delimited[m_program->currentIndex()] + "\"\n" +
            "The first preset on the list must be empty, please select another preset to save.");

            f_qmess->setText(f_compare_text_string);
            f_qmess->show();
            
            delete f_qmess;
        }
        else if(f_compare_text == 0)
        {
            QMessageBox * f_qmess = new QMessageBox(lms_parent);

            QString f_compare_text_string = QString("");
            f_compare_text_string.setNum(f_compare_text);

            f_compare_text_string.append("  \"" + presets_tab_delimited[m_program->currentIndex()] + "\"\n" +
            "You must change the name of the preset before you can save it.");

            f_qmess->setText(f_compare_text_string);
            f_qmess->show();
            
            delete f_qmess;
        }    
        else
        {
        QString f_result = m_program->currentText();                

        //TODO:  change f_i back to zero when there is something at that index
        for(int f_i = 0; f_i < lms_controls.count(); f_i++)
        {
            QString f_number = QString("");
            f_number.setNum(lms_controls.at(f_i)->lms_get_value()); //i_get_control(f_i));
            f_result.append("\t");  
            f_result.append(f_number);
        }

        presets_tab_delimited[m_program->currentIndex()] = f_result;

        QString f_file_path = QDir::homePath() + "/pydaw3/" + lms_plugin_name + LMS_PRESETS_EXT;

        write_presets_to_file(f_file_path);
        
        m_program->setItemText(m_program->currentIndex(), m_program->currentText());
        }
    }
        
    QString presets_to_string()
    {
        QString f_result2 = QString("");

        for(int f_i = 0; f_i < 128; f_i++)
        {
                f_result2.append(presets_tab_delimited[f_i] + "\n");
        }
                
        return f_result2;
    }
    
    void session_save(QString a_project_dir, QString a_instance_name)
    {
        QString f_result = m_program->currentText();                

        //TODO:  change f_i back to zero when there is something at that index
        for(int f_i = 0; f_i < lms_controls.count(); f_i++)
        {
            QString f_number = QString("");
            f_number.setNum(lms_controls.at(f_i)->lms_get_value()); //i_get_control(f_i));
            f_result.append("\t");  
            f_result.append(f_number);
        }

        presets_tab_delimited[m_program->currentIndex()] = f_result;
        
        write_presets_to_file(a_project_dir + QString("/") + a_instance_name + LMS_PRESETS_EXT);
    }
    
    void write_presets_to_file(QString a_file)
    {
        QString f_result2 = presets_to_string();
        
        QFile * f_preset_file = new QFile(a_file);
        
        f_preset_file->open(QIODevice::WriteOnly);

        f_preset_file->write(f_result2.toUtf8());
        f_preset_file->flush();

        f_preset_file->close();
        
        delete f_preset_file;
    }
        
    void instantiate(QString a_plugin_name, QString a_default_presets, int a_lms_port, LMS_style_info * a_style_info, QWidget * a_parent, 
            QString a_preset_directory)
    {
        lms_plugin_name = a_plugin_name;
        lms_port = a_lms_port;
        m_program = new QComboBox(a_parent);
        m_program->setEditable(TRUE);
        m_program->setMinimumWidth(240);
        m_prog_save = new QPushButton(a_parent);
        m_prog_save->setText(QString("Save"));
        lms_group_box = new QGroupBox(a_parent);
        lms_group_box->setTitle(QString("Presets"));
                
        lms_layout = new QHBoxLayout(lms_group_box);
                
        lms_layout->addWidget(m_program);
        lms_layout->addWidget(m_prog_save);
                
        lms_parent = a_parent;
        
        QStringList f_programs_list;        

        QDir* f_dir = new QDir(a_preset_directory);

        if(!f_dir->exists(a_preset_directory))           
        {
            f_dir->mkpath(a_preset_directory);
        }

        QFile* f_file = new QFile(a_preset_directory + "/" + a_plugin_name + LMS_PRESETS_EXT);

        if(!f_file->open(QIODevice::ReadOnly)) 
        {   
            f_file->open(QIODevice::ReadWrite | QIODevice::Text);
            
            f_file->write(a_default_presets.toUtf8());
            
            f_file->flush();
            f_file->seek(0);         
        }

        QTextStream * in = new QTextStream(f_file);

        int f_count = 0;
        
        while(f_count < 128) 
        {
            if(in->atEnd())
            {
                f_programs_list.append(QString("empty"));
                presets_tab_delimited.append(QString("empty"));
            }
            else
            {
                QString line = in->readLine();
                                
                QStringList fields = line.split("\t");
                f_programs_list.append(fields.at(0));
                presets_tab_delimited.append(line);
            }
            
            f_count++;
        }

        f_file->close();
        
        m_program->insertItems(0, f_programs_list);
                
    }
    
};


#endif	/* LMS_PRESET_MGR_H */

