/* 
 * File:   lms_main_layout.h
 * Author: Jeff Hubbard
 * 
 * The main layout to be used in a plugin, either in the main QFrame, or in a tabpage
 *
 * Created on April 1, 2012, 9:47 PM
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

