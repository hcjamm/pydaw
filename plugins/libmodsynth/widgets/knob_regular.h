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
#include <math.h>

enum LMS_KNOB_CONVERSION
{
    lms_kc_integer, lms_kc_decimal, lms_kc_pitch, lms_kc_none
};

/* This class passes global information for the plugin to new knobs
 */
class LMS_knob_info
{    
    public:
        LMS_knob_info(int a_size)
        {
            lms_knob_size = a_size;
            lms_use_label_style = FALSE;
        };
        
        void LMS_set_label_style(QString a_style, int a_width)
        {
            lms_use_label_style = TRUE;
            lms_label_style = a_style;
            lms_label_width = a_width;
        }
        
        int lms_knob_size;
        int lms_label_width;
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
        QString a_label_value, QWidget *a_parent, LMS_knob_info * a_knob_info, LMS_KNOB_CONVERSION a_conv_type, int a_lms_port)
        {
            lms_layout = new QVBoxLayout(a_parent);
            lms_label = new QLabel(a_parent);
            lms_label->setMinimumWidth(a_knob_info->lms_label_width);
            lms_label->setText(a_label);
            lms_label->setAlignment(Qt::AlignCenter);
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
                //lms_value->setStyleSheet((a_knob_info->lms_label_style));
            }
                        
            lms_knob->setMinimumSize((a_knob_info->lms_knob_size),(a_knob_info->lms_knob_size));
            lms_knob->setMaximumSize((a_knob_info->lms_knob_size),(a_knob_info->lms_knob_size));
            
            lms_conv_type  = a_conv_type;
                                    
            lms_layout->addWidget(lms_label, -1, Qt::AlignCenter);
            lms_layout->addWidget(lms_knob, -1, Qt::AlignCenter);
            lms_layout->addWidget(lms_value, -1, Qt::AlignCenter);
            
            lms_port = a_lms_port;
            
            //valueChanged(lms_knob->value());            
        }
        
        void valueChanged(int a_value)
        {
                switch(lms_conv_type)
                {
                    case lms_kc_decimal:
                        lms_value->setText(QString::number(((float)a_value) * .01 ));
                        break;
                    case lms_kc_integer:
                        lms_value->setText(QString::number(a_value));
                        break;
                    case lms_kc_none:
                        //Do nothing
                        break;
                    case lms_kc_pitch:
                        lms_value->setText(QString::number(
                        (440*pow(2,((float)(a_value-57))*.0833333))
                        ));
                        break;
                }
        }
        
                
        QVBoxLayout * lms_layout;
        QLabel * lms_label;
        QDial * lms_knob;        
        QLabel * lms_value;
        LMS_KNOB_CONVERSION lms_conv_type;
        
        int lms_port;
        
        ~LMS_knob_regular(){};
};

#endif	/* KNOB_REGULAR_H */

