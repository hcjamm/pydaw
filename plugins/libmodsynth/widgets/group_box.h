/*
This file is part of the PyDAW project, Copyright PyDAW Team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef GROUP_BOX_H
#define	GROUP_BOX_H

#include <QtGui/QApplication>
#include <QGroupBox>
#include <QGridLayout>
#include "knob_regular.h"
#include "lms_control.h"

class LMS_group_box 
{
    public:
        LMS_group_box(QWidget * a_parent, QString a_title, LMS_style_info * a_style_info)
        {
            lms_groupbox = new QGroupBox(a_parent);
            lms_groupbox->setTitle(a_title);
            lms_groupbox->setObjectName("plugin_groupbox");
            lms_layout = new QGridLayout(lms_groupbox);
            
            if(a_style_info->lms_use_groupbox_style)
            {
                lms_groupbox->setStyleSheet(a_style_info->lms_groupbox_style);
                lms_stylesheet = (a_style_info->lms_groupbox_style);
            }
            
            x_index = 0;
            y_index = 0;
        }
        void lms_add_h(LMS_control * a_ctrl)
        {
            lms_add(a_ctrl);
            x_index++;
        }
        
        void lms_add_v(LMS_control * a_ctrl)        
        {
            /*Don't increment if nothing has been added yet
            if((y_index != 0) && (x_index != 0))*/
            
            y_index++;
            
            x_index = 0;
            
            lms_add(a_ctrl);
            
            x_index++;
        }
        
        QGroupBox * lms_groupbox;
        QGridLayout * lms_layout;
        QString lms_stylesheet;
        
    private:
        int x_index;
        int y_index;
        
        void lms_add(LMS_control * a_ctrl)        
        {            
            lms_layout->addLayout(a_ctrl->lms_get_layout(), y_index, x_index, Qt::AlignCenter);
        }
};

#endif	/* GROUP_BOX_H */

