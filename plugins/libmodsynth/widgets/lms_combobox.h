/* 
 * File:   lms_combobox.h
 * Author: Jeff Hubbard
 *
 * Created on April 2, 2012, 6:14 PM
 */

#ifndef LMS_COMBOBOX_H
#define	LMS_COMBOBOX_H

#include <QtGui/QApplication>
#include <QVBoxLayout>
#include <QString>
#include <QLabel>
#include <QComboBox>
#include <QStringList>

#include "lms_control.h"


class LMS_combobox : public LMS_control
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
        LMS_combobox(QString a_label, QWidget *a_parent, QStringList a_items, int a_lms_port, LMS_style_info * a_style_info)
        {
            lms_layout = new QVBoxLayout(a_parent);
            lms_label = new QLabel(a_parent);
            //lms_label->setMinimumWidth(a_knob_info->lms_label_width);
            lms_label->setText(a_label);
            lms_label->setAlignment(Qt::AlignCenter);
            
            lms_combobox = new QComboBox(a_parent);
            lms_combobox->insertItems(0, a_items);
            
            lms_value = new QLabel(a_parent);
            //lms_value->setText(a_label_value);
            /*
            if(a_knob_info->lms_use_label_style)
            {
                lms_label->setStyleSheet((a_knob_info->lms_label_style));
                //lms_value->setStyleSheet((a_knob_info->lms_label_style));
            }
                        
            lms_knob->setMinimumSize((a_knob_info->lms_knob_size),(a_knob_info->lms_knob_size));
            lms_knob->setMaximumSize((a_knob_info->lms_knob_size),(a_knob_info->lms_knob_size));
            
            lms_conv_type  = a_conv_type;
            */                      
            lms_layout->addWidget(lms_label, -1, Qt::AlignCenter);
            lms_layout->addWidget(lms_combobox, -1, Qt::AlignCenter);
            lms_layout->addWidget(lms_value, -1, Qt::AlignCenter);
            
            lms_port = a_lms_port;
            
        }
        
        void lms_value_changed(int a_value)
        {
                //Do nothing
        }
        
                
        QVBoxLayout * lms_layout;
        QLabel * lms_label;
        QComboBox * lms_combobox;
        QLabel * lms_value;
        LMS_KNOB_CONVERSION lms_conv_type;
        
        void lms_set_value(int a_value)
        {
            lms_combobox->setCurrentIndex(a_value);
        }
        
        int lms_get_value()
        {
            return lms_combobox->currentIndex();
        }
        
        virtual QLayout * lms_get_layout()
        {
            return lms_layout;
        }
        virtual QWidget * lms_get_widget()
        {
            return lms_combobox;
        }
        
        ~LMS_combobox(){};
};


#endif	/* LMS_COMBOBOX_H */

