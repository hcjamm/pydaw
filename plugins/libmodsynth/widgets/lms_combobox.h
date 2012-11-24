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
        /* LMS_combobox(
         * QString a_label, //The text to be used for the control's label
         * QWidget *a_parent, //the parent widget
         * QStringList a_items, //The items to show in the dropdown
         * int a_lms_port, //The LMS port for this control
         * LMS_style_info * a_style_info) //The style_info object that controls theming
         */
        LMS_combobox(QString a_label, QWidget *a_parent, QStringList a_items, int a_lms_port, LMS_style_info * a_style_info)
        {
            lms_layout = new QVBoxLayout();
            lms_label = new QLabel(a_parent);
            lms_label->setText(a_label);
            lms_label->setAlignment(Qt::AlignCenter);
                        
            if(a_style_info->lms_use_label_style)
            {
                lms_label->setStyleSheet((a_style_info->lms_label_style));
                lms_label->setMinimumWidth(a_style_info->lms_label_width);
            }
            
            lms_combobox = new QComboBox(a_parent);
            lms_combobox->insertItems(0, a_items);
                                    
            lms_value = new QLabel(a_parent);
            //lms_value->setText(a_label_value);
              
            lms_combobox->setMinimumWidth((a_style_info->lms_knob_size));
            lms_combobox->setMaximumWidth((a_style_info->lms_knob_size));
                        
            lms_layout->addWidget(lms_label, 0, Qt::AlignTop);
            lms_layout->addWidget(lms_combobox, -1, Qt::AlignCenter);
            lms_layout->addWidget(lms_value, 1, Qt::AlignCenter);
                                    
            lms_port = a_lms_port;   
        }
        
        /* This constructor is meant to be used only with LMS_mod_matrix,
         * it does not initialize the layout or label.
         * 
         * LMS_combobox(         
         * QWidget *a_parent, //the parent widget
         * QStringList a_items, //The items to show in the dropdown
         * int a_lms_port, //The LMS port for this control
         * LMS_style_info * a_style_info) //The style_info object that controls theming
         */
        LMS_combobox(QWidget *a_parent, QStringList a_items, int a_lms_port, LMS_style_info * a_style_info)
        {   
            lms_combobox = new QComboBox(a_parent);
            lms_combobox->insertItems(0, a_items);
            /*
            lms_combobox->setMinimumSize((a_style_info->lms_knob_size),(a_style_info->lms_knob_size));
            lms_combobox->setMaximumSize((a_style_info->lms_knob_size),(a_style_info->lms_knob_size));
            */
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

        
        void lms_set_value(int a_value)
        {
            lms_combobox->setCurrentIndex(a_value);
        }
        
        int lms_get_value()
        {
            return lms_combobox->currentIndex();
        }
        
        QLayout * lms_get_layout()
        {
            return lms_layout;
        }
        QWidget * lms_get_widget()
        {
            return lms_combobox;
        }
        
        ~LMS_combobox(){};
};


#endif	/* LMS_COMBOBOX_H */

