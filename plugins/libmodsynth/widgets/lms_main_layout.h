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

#ifndef LMS_MAIN_LAYOUT_H
#define	LMS_MAIN_LAYOUT_H

#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QString>
#include <QLabel>
#include <QDial>
#include <QList>
#include <QSpacerItem>

class LMS_main_layout
{
    public:
        QVBoxLayout * lms_layout;
        QList<QHBoxLayout*> lms_h_layouts;
        int lms_row_index;
        
        LMS_main_layout(QWidget * a_parent)
        {
            lms_layout = new QVBoxLayout(a_parent);            
            lms_row_index = -1;
            lms_add_layout();
            
        }
        
        void lms_add_widget(QWidget * a_widget)
        {
            lms_h_layouts.at(lms_row_index)->addWidget(a_widget);
        }
        
        void lms_add_layout()
        {
            QHBoxLayout * f_new = new QHBoxLayout();
            lms_h_layouts.append(f_new);
            lms_layout->addLayout(f_new, -1);
            lms_row_index++;
        }
        
        void lms_add_spacer()
        {
            lms_h_layouts.at(lms_row_index)->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
        }
        
        void lms_add_vertical_spacer()
        {
            lms_h_layouts.at(lms_row_index)->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
        }
};


#endif	/* LMS_MAIN_LAYOUT_H */

