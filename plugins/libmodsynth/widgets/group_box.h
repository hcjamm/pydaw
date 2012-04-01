/* 
 * File:   group_box.h
 * Author: Jeff Hubbard
 * 
 * A QGroupBox for storing other controls
 *
 * Created on April 1, 2012, 2:06 PM
 */

#ifndef GROUP_BOX_H
#define	GROUP_BOX_H

#include <QtGui/QApplication>
#include <QGroupBox>
#include <QGridLayout>
#include "knob_regular.h"

class LMS_group_box 
{
    public:
        LMS_group_box(QWidget * a_parent, QString a_title, QString a_stylesheet)
        {
            lms_groupbox = new QGroupBox(a_parent);
            lms_groupbox->setTitle(a_title);
            lms_layout = new QGridLayout(lms_groupbox);
            lms_groupbox->setStyleSheet(a_stylesheet);
            lms_stylesheet = a_stylesheet;
            x_index = 0;
            y_index = 0;
        }
        void lms_add_h(LMS_knob_regular * a_knob)        
        {
            lms_add(a_knob);
            x_index++;
        }
        
        void lms_add_v(LMS_knob_regular * a_knob)        
        {
            /*Don't increment if nothing has been added yet
            if((y_index != 0) && (x_index != 0))*/
            
            y_index++;
            
            x_index = 0;
            
            lms_add(a_knob);
            
            x_index++;
        }
        
        QGroupBox * lms_groupbox;
        QGridLayout * lms_layout;
        QString lms_stylesheet;
        
    private:
        int x_index;
        int y_index;
        
        void lms_add(LMS_knob_regular * a_knob)        
        {            
            lms_layout->addLayout(a_knob->lms_layout, y_index, x_index, Qt::AlignCenter);
        }
};

#endif	/* GROUP_BOX_H */

