/* 
 * File:   lms_slider.h
 * Author: Jeff Hubbard
 * 
 * A horizontal or vertical slider
 *
 * Created on April 11, 2012, 12:10 PM
 */

#ifndef LMS_SLIDER_H
#define	LMS_SLIDER_H

#include <QtGui/QApplication>
#include <QVBoxLayout>
#include <QString>
#include <QLabel>
#include <QSlider>
#include <math.h>

#include "lms_control.h"

class LMS_slider : public LMS_control
{   
    public:        
        /* LMS_knob_regular(
         * QString a_label,  //The label to display above the slider
         * int a_min,  //The minimum value of the knob.
         * int a_max, //The maximum value of the knob.
         * int a_step_size, //Step size.  Usually 1
         * int a_value, //The initial value
         * QString a_value_label_value, //The initial value of the label below the knob that displays the converted value
         * QWidget *a_parent, //The parent widget
         * LMS_style_info * a_style_info, //A style_info object to provide information about theming         
         * int a_lms_port, //The port defined in synth.h for this control
         * bool a_vertical) //True for a vertical slider, false for a horizontal
         */
        LMS_slider(QString a_label,int a_min, int a_max, int a_step_size, int a_value, 
        QString a_value_label_value, QWidget *a_parent, LMS_style_info * a_style_info, int a_lms_port, bool a_vertical)
        {
            lms_layout = new QVBoxLayout();
            lms_label = new QLabel(a_parent);            
            lms_label->setText(a_label);
            lms_label->setAlignment(Qt::AlignCenter);
            lms_slider = new QSlider(a_parent);
            lms_slider->setMinimum(a_min);
            lms_slider->setMaximum(a_max);
            lms_slider->setSingleStep(a_step_size);
            lms_slider->setValue(a_value);
            
            if(a_vertical)
            {
                lms_slider->setOrientation(Vertical);
            }
            else
            {
                lms_slider->setOrientation(Horizontal);
            }
            
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
                        
            lms_slider->setMinimumSize((a_style_info->lms_knob_size),(a_style_info->lms_knob_size));
            lms_slider->setMaximumSize((a_style_info->lms_knob_size),(a_style_info->lms_knob_size));
                                                
            lms_layout->addWidget(lms_label, -1, Qt::AlignCenter);
            lms_layout->addWidget(lms_slider, -1, Qt::AlignCenter);
            lms_layout->addWidget(lms_value, -1, Qt::AlignCenter);
            
            lms_port = a_lms_port;
            
            lms_value_changed(lms_slider->value());
        }
                        
        void lms_value_changed(int a_value)
        {
            lms_value->setText(QString::number(a_value));
        }
        
                
        QVBoxLayout * lms_layout;
        QLabel * lms_label;
        QSlider * lms_slider;        
        QLabel * lms_value;
        LMS_KNOB_CONVERSION lms_conv_type;
        
        void lms_set_value(int a_value)
        {
            lms_slider->setValue(a_value);
        }
        
        int lms_get_value()
        {
            return lms_slider->value();
        }
        
        QLayout * lms_get_layout()
        {
            return lms_layout;
        }
        
        QWidget * lms_get_widget()
        {
            return lms_slider;
        }
        
        ~LMS_slider(){};
};

#endif	/* LMS_SLIDER_H */

