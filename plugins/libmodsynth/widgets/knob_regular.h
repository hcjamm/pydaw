/* 
 * File:   knob_regular.h
 * Author: Jeff Hubbard
 *
 * A plain knob and label nested in a QVBoxLayout
 * 
 * Created on April 1, 2012, 11:46 AM
 */

#ifndef KNOB_REGULAR_H
#define	KNOB_REGULAR_H

#include <QtGui/QApplication>
#include <QVBoxLayout>
#include <QString>
#include <QLabel>
#include <QDial>

/* This class passes global information for the plugin to new knobs
 */
class LMS_knob_info
{    
    public:
        LMS_knob_info(int a_size)
        {
            lms_size = a_size;
            lms_use_label_style = FALSE;
        };
        
        void LMS_set_label_style(QString a_style)
        {
            lms_use_label_style = TRUE;
            lms_label_style = a_style;
        }
        
        int lms_size;
        QString lms_label_style;
        bool lms_use_label_style;
};


class LMS_knob_regular
{   
    public:
        /* LMS_knob_regular(
         * QString a_label,
         * int a_min, 
         * int a_max, 
         * int a_step_size, 
         * int a_value, 
         * QString a_label_value, 
         * LMS_KNOB_CONVERSION a_conv_type, 
         * QWidget *a_parent, 
         * int a_lms_port, 
         * LMS_knob_info * a_knob_info)
         */
        LMS_knob_regular(QString a_label,int a_min, int a_max, int a_step_size, int a_value, 
        QString a_label_value, QWidget *a_parent, LMS_knob_info * a_knob_info)
        {
            lms_layout = new QVBoxLayout(a_parent);
            lms_label = new QLabel(a_parent);
            lms_label->setText(a_label);
            lms_knob = new QDial(a_parent);
            lms_knob->setMinimum(a_min);
            lms_knob->setMaximum(a_max);
            lms_knob->setSingleStep(a_step_size);
            lms_knob->setValue(a_value);
            lms_value = new QLabel(a_parent);
            lms_value->setText(a_label_value);
            
            if(a_knob_info->lms_use_label_style)
            {
                lms_label->setStyleSheet((a_knob_info->lms_label_style));
                lms_value->setStyleSheet((a_knob_info->lms_label_style));
            }
                        
            lms_knob->setMinimumSize((a_knob_info->lms_size),(a_knob_info->lms_size));
            lms_knob->setMaximumSize((a_knob_info->lms_size),(a_knob_info->lms_size));
                                    
            lms_layout->addWidget(lms_label, -1, Qt::AlignCenter);
            lms_layout->addWidget(lms_knob, -1, Qt::AlignCenter);
            lms_layout->addWidget(lms_value, -1, Qt::AlignCenter);
            
            //lms_value_changed(lms_knob->value());            
        }
                
        QVBoxLayout * lms_layout;
        QLabel * lms_label;
        QDial * lms_knob;        
        QLabel * lms_value;        
                        
        bool * lms_suppressHostUpdate;
        //lo_address lms_host;
        QByteArray lms_controlPath;
        
        ~LMS_knob_regular(){};
};

#endif	/* KNOB_REGULAR_H */

