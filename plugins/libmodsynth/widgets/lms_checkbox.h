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
                
        /* This constructor is meant to be used only with LMS_mod_matrix,
         * it does not initialize the layout or label.
         * 
         * LMS_checkbox(
         * QWidget *a_parent, //The parent widget
         * LMS_style_info * a_style_info, //A style_info object to provide information about theming         
         * int a_lms_port) //The port defined in synth.h for this control
         */
        LMS_checkbox(QWidget *a_parent, LMS_style_info * a_style_info, int a_lms_port, QString a_text)
        {            
            lms_checkbox = new QCheckBox(a_parent);
            lms_checkbox->setMaximumWidth(90);
            lms_checkbox->setText(a_text);            
            lms_port = a_lms_port;        
            lms_layout = new QVBoxLayout();
            lms_layout->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
            lms_layout->addWidget(lms_checkbox);
            lms_layout->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
        }
        
        void lms_value_changed(int a_value)
        {
                //Do nothing?
        }        
        
        QCheckBox * lms_checkbox;        
        QVBoxLayout * lms_layout;
        
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

