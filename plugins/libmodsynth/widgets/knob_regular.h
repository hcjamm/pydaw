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

#include "lms_control.h"

enum LMS_KNOB_CONVERSION
{
    lms_kc_integer, lms_kc_decimal, lms_kc_pitch, lms_kc_none
};

class LMS_knob_regular : public LMS_control
{   
    public:
        /* LMS_knob_regular(
         * QString a_label,  //The label to display above the knob
         * int a_min,  //The minimum value of the knob.
         * int a_max, //The maximum value of the knob.
         * int a_step_size, //Step size.  Usually 1
         * int a_value, //The initial value
         * QString a_value_label_value, //The initial value of the label below the knob that displays the converted value
         * QWidget *a_parent, //The parent widget
         * LMS_style_info * a_style_info, //A style_info object to provide information about theming
         * LMS_KNOB_CONVERSION a_conv_type, //This enum specifies how the knob's integer value is converted to the displayed value
         * int a_lms_port) //The port defined in synth.h for this control
         */
        LMS_knob_regular(QString a_label,int a_min, int a_max, int a_step_size, int a_value, 
        QString a_value_label_value, QWidget *a_parent, LMS_style_info * a_style_info, LMS_KNOB_CONVERSION a_conv_type, int a_lms_port)
        {
            lms_layout = new QVBoxLayout();
            lms_label = new QLabel(a_parent);            
            lms_label->setText(a_label);
            lms_label->setAlignment(Qt::AlignCenter);
            lms_knob = new QDial(a_parent);
            lms_knob->setMinimum(a_min);
            lms_knob->setMaximum(a_max);
            lms_knob->setSingleStep(a_step_size);
            lms_knob->setValue(a_value);
            lms_value = new QLabel(a_parent);
            lms_value->setText(a_value_label_value);
            
            if(a_style_info->lms_use_label_style)
            {
                lms_label->setStyleSheet((a_style_info->lms_label_style));
                lms_label->setMinimumWidth(a_style_info->lms_label_width);
            }
            
            if(a_style_info->lms_use_value_style)
            {
                lms_value->setStyleSheet((a_style_info->lms_value_style));
            }
                        
            lms_knob->setMinimumSize((a_style_info->lms_knob_size),(a_style_info->lms_knob_size));
            lms_knob->setMaximumSize((a_style_info->lms_knob_size),(a_style_info->lms_knob_size));
            
            lms_conv_type  = a_conv_type;
                                    
            lms_layout->addWidget(lms_label, -1, Qt::AlignCenter);
            lms_layout->addWidget(lms_knob, -1, Qt::AlignCenter);
            lms_layout->addWidget(lms_value, -1, Qt::AlignCenter);
            
            lms_port = a_lms_port;
            
            lms_value_changed(lms_knob->value());
        }
        
        /* This constructor is meant to be used only with LMS_mod_matrix,
         * it does not initialize the layout or label.
         * 
         * LMS_knob_regular(
         * int a_min,  //The minimum value of the knob.
         * int a_max, //The maximum value of the knob.
         * int a_step_size, //Step size.  Usually 1
         * int a_value, //The initial value
         * QWidget *a_parent, //The parent widget
         * LMS_style_info * a_style_info, //A style_info object to provide information about theming         
         * int a_lms_port) //The port defined in synth.h for this control
         */        
        LMS_knob_regular(int a_min, int a_max, int a_step_size, int a_value, 
        QWidget *a_parent, LMS_style_info * a_style_info, int a_lms_port)
        {
            lms_knob = new QDial(a_parent);
            lms_knob->setMinimum(a_min);
            lms_knob->setMaximum(a_max);
            lms_knob->setSingleStep(a_step_size);
            lms_knob->setValue(a_value);
                                    
            lms_knob->setMinimumSize((a_style_info->lms_knob_size),(a_style_info->lms_knob_size));
            lms_knob->setMaximumSize((a_style_info->lms_knob_size),(a_style_info->lms_knob_size));
            
            lms_conv_type  = lms_kc_none;
                                    
            lms_port = a_lms_port;            
        }
        
        void lms_value_changed(int a_value)
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
                       ((int)(440*pow(2,((float)(a_value-57))*.0833333)))
                        ));
                        break;
                }
        }
        
                
        QVBoxLayout * lms_layout;
        QLabel * lms_label;
        QDial * lms_knob;        
        QLabel * lms_value;
        LMS_KNOB_CONVERSION lms_conv_type;
        
        void lms_set_value(int a_value)
        {
            lms_knob->setValue(a_value);
        }
        
        int lms_get_value()
        {
            return lms_knob->value();
        }
        
        QLayout * lms_get_layout()
        {
            return lms_layout;
        }
        
        QWidget * lms_get_widget()
        {
            return lms_knob;
        }
        
        ~LMS_knob_regular(){};
};

#endif	/* KNOB_REGULAR_H */

