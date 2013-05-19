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

#ifndef LMS_NOTE_SELECTOR_H
#define	LMS_NOTE_SELECTOR_H

#include "lms_control.h"
#include <QSpinBox>
#include <QComboBox>
#include <QHBoxLayout>

class LMS_note_selector : public LMS_control
{
    public:
        LMS_note_selector(QWidget * a_parent, int a_lms_port, LMS_style_info * a_style, int a_default_octave)
        {
            lms_port = a_lms_port;
            
            lms_selected_note = 48;
                        
            lms_widget = new QWidget(a_parent);
            lms_widget->setMinimumSize(10, 10);
            
            lms_layout = new QHBoxLayout(lms_widget);
            lms_layout->setSpacing(0);
            lms_layout->setMargin(0);
            
            lms_octave = new QSpinBox(lms_widget);
            lms_octave->setMaximum(8);
            lms_octave->setMinimum(-2);
            lms_octave->setValue(a_default_octave);
            
            QStringList f_notes_list = QStringList() << "C" << "C#" << "D" << "D#" << "E" << "F" << "F#" << "G" << "G#" << "A" << "A#" << "B";
            
            lms_note = new QComboBox(lms_widget);
            lms_note->addItems(f_notes_list);
            lms_note->setMinimumWidth(75);
                        
            lms_layout->addWidget(lms_note, -1);
            lms_layout->addWidget(lms_octave, -1);
            
            lms_value_changed(-1);
        }
            
        int lms_selected_note;
        QWidget * lms_widget;
        QHBoxLayout * lms_layout;
        QSpinBox * lms_octave;
        QComboBox * lms_note;
        
        void lms_set_value(int a_value)
        {
            lms_note->setCurrentIndex(a_value % 12);
            lms_octave->setValue(((int)(a_value / 12)) - 2);
        }
        void lms_value_changed(int a_value)
        {            
            lms_selected_note = (lms_note->currentIndex()) + (((lms_octave->value()) + 2) * 12);
        }
        
        int lms_get_value()
        {
            return lms_selected_note;
        }
        
        QLayout * lms_get_layout()
        {
            return lms_layout;
        }
        
        QWidget * lms_get_widget()
        {
            return lms_widget;
        }
};


#endif	/* LMS_NOTE_SELECTOR_H */

