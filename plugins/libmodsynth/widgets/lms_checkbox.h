/* 
 * File:   lms_checkbox.h
 * Author: Jeff Hubbard
 *
 * 
 */

#ifndef LMS_CHECKBOX_H
#define	LMS_CHECKBOX_H

#include <QLabel>
#include <QCheckBox>

class LMS_checkbox : public LMS_control
{   
    public:
        /* LMS_checkbox(
         * QString a_label,  //The label to display above the knob
         * int a_min,  //The minimum value of the knob.
         * int a_max, //The maximum value of the knob.
         * int a_step_size, //Step size.  Usually 1
         * int a_value, //The initial value        
         * QWidget *a_parent, //The parent widget
         * LMS_style_info * a_style_info, //A style_info object to provide information about theming         
         * int a_lms_port) //The port defined in synth.h for this control
         */
        LMS_checkbox(QString a_label,int a_min, int a_max, int a_step_size, int a_value, 
        QWidget *a_parent, LMS_style_info * a_style_info, int a_lms_port)
        {
            lms_layout = new QVBoxLayout();
            lms_label = new QLabel(a_parent);
            lms_label->setMinimumWidth(a_style_info->lms_label_width);
            lms_label->setText(a_label);
            lms_label->setAlignment(Qt::AlignCenter);
            lms_checkbox = new  QCheckBox(a_parent);
                        
            if(a_style_info->lms_use_label_style)
            {
                lms_label->setStyleSheet((a_style_info->lms_label_style));
            }

            lms_layout->addWidget(lms_label, -1, Qt::AlignCenter);
            lms_layout->addWidget(lms_checkbox, -1, Qt::AlignCenter);
                        
            lms_port = a_lms_port;            
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
        LMS_checkbox(int a_min, int a_max, int a_step_size, int a_value, 
        QWidget *a_parent, LMS_style_info * a_style_info, int a_lms_port)
        {            
            lms_checkbox = new QSpinBox(a_parent);            
            /*
            lms_spinbox->setMinimumSize((a_style_info->lms_knob_size),(a_style_info->lms_knob_size));
            lms_spinbox->setMaximumSize((a_style_info->lms_knob_size),(a_style_info->lms_knob_size));
            */
            lms_port = a_lms_port;            
        }
        
        void lms_value_changed(int a_value)
        {
                //Do nothing?
        }        
                
        QVBoxLayout * lms_layout;
        QLabel * lms_label;
        QCheckBox * lms_checkbox;        
        
        void lms_set_value(int a_value)
        {
            if(a_value == 0)
            {
                lms_checkbox->setChecked(FALSE);                
            }
            else
            {
                lms_checkbox->setChecked(TRUE);
            }            
        }
        
        int lms_get_value()
        {
            if(lms_checkbox->isChecked())
            {
                return 1;
            }
            else
            {
                return 0;
            }            
        }
        
        QLayout * lms_get_layout()
        {
            return lms_layout;
        }
        
        QWidget * lms_get_widget()
        {            
            return lms_checkbox;
        }
        
        ~LMS_checkbox(){};
};

#endif	/* LMS_CHECKBOX_H */

