/* 
 * File:   presets.h
 * Author: Jeff Hubbard
 * 
 * This class manages preset functionality
 *
 * Created on April 2, 2012, 5:50 PM
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

class LMS_preset_manager : public LMS_control
{
public:
    LMS_preset_manager(QString a_plugin_name, QString a_default_presets, int a_lms_port, LMS_style_info * a_style_info, QWidget * a_parent)
    {
        lms_plugin_name = a_plugin_name;
        lms_port = a_lms_port;
        m_program = new QComboBox(a_parent);
        m_program->setEditable(TRUE);
        m_prog_save = new QPushButton(a_parent);
        m_prog_save->setText(QString("Save"));
        lms_group_box = new QGroupBox(a_parent);
        lms_group_box->setTitle(QString("Presets"));
                
        lms_layout = new QHBoxLayout(lms_group_box);
                
        lms_layout->addWidget(m_program);
        lms_layout->addWidget(m_prog_save);
                
        lms_parent = a_parent;
        
        QStringList f_programs_list;

        QString f_preset_path = QDir::homePath() + "/dssi";

        QDir* f_dir = new QDir(QDir::homePath());

        if(!f_dir->exists(f_preset_path))
            f_dir->mkpath(f_preset_path);

        QFile* f_file = new QFile(f_preset_path + "/" + lms_plugin_name + "-presets.tsv");

        if(!f_file->open(QIODevice::ReadOnly)) 
        {   
            f_file->open(QIODevice::ReadWrite | QIODevice::Text);
            
            f_file->write(a_default_presets.toUtf8());
            
            f_file->flush();
            f_file->seek(0);         
        }

        QTextStream * in = new QTextStream(f_file);

        bool f_at_end = FALSE;
        int f_count = 0;
        
        while(f_count < 128) 
        {
            if(in->atEnd())
            {      
                f_at_end = TRUE;
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

        if(presets_tab_delimited[a_value].compare("empty") != 0)
        {
            QStringList f_preset_values = presets_tab_delimited[a_value].split("\t");
            //TODO:  change f_i back to zero when there is something at that index
            for(int f_i = 0; f_i < lms_controls.count(); f_i++)
            {
                if(f_i > f_preset_values.count())
                {
                    break;
                }
                /*TODO:  Some error handling here to prevent index-out-of-bounds exceptions from crashing the GUI*/
                int f_preset_value_int = f_preset_values.at(f_i).toInt();

                lms_controls.at(f_i)->lms_set_value(f_preset_value_int);
                //v_control_changed(f_i, f_preset_value_int, false);
            }
        }
    }
    
    void programSaved()
    {
        int f_compare_text = m_program->currentText().compare("empty");

        if((m_program->currentIndex()) == 0)
        {
            QMessageBox * f_qmess = new QMessageBox(lms_parent);

            QString * f_compare_text_string = new QString();
            f_compare_text_string->setNum(f_compare_text);

            f_compare_text_string->append("  \"" + presets_tab_delimited[m_program->currentIndex()] + "\"\n" +
            "The first preset on the list must be empty, please select another preset to save.");

            f_qmess->setText(*f_compare_text_string);
            f_qmess->show();        
        }
        else if(f_compare_text == 0)
        {
            QMessageBox * f_qmess = new QMessageBox(lms_parent);

            QString * f_compare_text_string = new QString();
            f_compare_text_string->setNum(f_compare_text);

            f_compare_text_string->append("  \"" + presets_tab_delimited[m_program->currentIndex()] + "\"\n" +
            "You must change the name of the preset before you can save it.");

            f_qmess->setText(*f_compare_text_string);
            f_qmess->show();
        }    
        else
        {
        QString f_result = m_program->currentText();                

        //TODO:  change f_i back to zero when there is something at that index
        for(int f_i = 0; f_i < lms_controls.count(); f_i++)
        {
            QString * f_number = new QString();
            f_number->setNum(lms_controls.at(f_i)->lms_get_value()); //i_get_control(f_i));
            f_result.append("\t");  
            f_result.append(f_number);
        }

        presets_tab_delimited[m_program->currentIndex()] = f_result;

        QString * f_result2 = new QString();

        for(int f_i = 0; f_i < 128; f_i++)
        {
                f_result2->append(presets_tab_delimited[f_i] + "\n");
        }

        QFile * f_preset_file = new QFile(QDir::homePath() + "/dssi/" + lms_plugin_name + "-presets.tsv");

        f_preset_file->open(QIODevice::WriteOnly);

        f_preset_file->write(f_result2->toUtf8());
        f_preset_file->flush();

        f_preset_file->close();

        m_program->setItemText(m_program->currentIndex(), m_program->currentText());
        }
    }
};


#endif	/* LMS_PRESET_MGR_H */

