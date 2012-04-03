/* 
 * File:   lms_spinbox.h
 * Author: Jeff Hubbard
 *
 * Created on April 2, 2012, 6:09 PM
 */

#ifndef LMS_SPINBOX_H
#define	LMS_SPINBOX_H

class LMS_spinbox : public LMS_control
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
        LMS_spinbox(QString a_label,int a_min, int a_max, int a_step_size, int a_value, 
        QString a_value_label_value, QWidget *a_parent, LMS_style_info * a_style_info, LMS_KNOB_CONVERSION a_conv_type, int a_lms_port)
        {
            lms_layout = new QVBoxLayout(a_parent);
            lms_label = new QLabel(a_parent);
            lms_label->setMinimumWidth(a_style_info->lms_label_width);
            lms_label->setText(a_label);
            lms_label->setAlignment(Qt::AlignCenter);
            lms_spinbox = new QDial(a_parent);
            lms_spinbox->setMinimum(a_min);
            lms_spinbox->setMaximum(a_max);
            lms_spinbox->setSingleStep(a_step_size);
            lms_spinbox->setValue(a_value);
            
            
            if(a_style_info->lms_use_label_style)
            {
                lms_label->setStyleSheet((a_style_info->lms_label_style));
            }
                        
            lms_spinbox->setMinimumSize((a_style_info->lms_knob_size),(a_style_info->lms_knob_size));
            lms_spinbox->setMaximumSize((a_style_info->lms_knob_size),(a_style_info->lms_knob_size));
            
            lms_conv_type  = a_conv_type;
                                    
            lms_layout->addWidget(lms_label, -1, Qt::AlignCenter);
            lms_layout->addWidget(lms_spinbox, -1, Qt::AlignCenter);
                        
            lms_port = a_lms_port;            
        }
        
        void lms_value_changed(int a_value)
        {
                //Do nothing?
        }        
                
        QVBoxLayout * lms_layout;
        QLabel * lms_label;
        QSpinBox * lms_spinbox;        
        LMS_KNOB_CONVERSION lms_conv_type;
        
        void lms_set_value(int a_value)
        {
            lms_spinbox->setValue(a_value);
        }
        
        int lms_get_value()
        {
            return lms_spinbox->value();
        }
        
        QLayout * lms_get_layout()
        {
            return lms_layout;
        }
        
        QWidget * lms_get_widget()
        {            
            return lms_spinbox;
        }
        
        ~LMS_spinbox(){};
};

#endif	/* LMS_SPINBOX_H */

