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

#ifndef ADSR_WIDGET_H
#define	ADSR_WIDGET_H

#include "../group_box.h"
#include "../knob_regular.h"

class LMS_adsr_widget
{
public:
    /* LMS_adsr_widget(
     * QWidget * a_parent, 
     * LMS_style_info * a_style, 
     * bool a_sustain_in_db, 
     * int a_attack_port, 
     * int a_decay_port, 
     * int a_sustain_port, 
     * int a_release_port, 
     * QString a_label)
     */
    LMS_adsr_widget(QWidget * a_parent, LMS_style_info * a_style, bool a_sustain_in_db, 
            int a_attack_port, int a_decay_port, int a_sustain_port, int a_release_port,
            QString a_label)
    {
        lms_groupbox_adsr = new LMS_group_box(a_parent, a_label, a_style);        
        lms_attack =  new LMS_knob_regular(QString("Attack"), 10, 100, 1, 1, QString(".01"), lms_groupbox_adsr->lms_groupbox, a_style, lms_kc_log_time, a_attack_port);
        lms_groupbox_adsr->lms_add_h(lms_attack);
        lms_decay =  new LMS_knob_regular(QString("Decay"), 10, 100, 1, 1, QString(".01"), lms_groupbox_adsr->lms_groupbox, a_style, lms_kc_log_time, a_decay_port);
        lms_groupbox_adsr->lms_add_h(lms_decay);
        
        if(a_sustain_in_db)
        {
            lms_sustain = new LMS_knob_regular(QString("Sustain"), -60, 0, 1, -6, QString("-6"), lms_groupbox_adsr->lms_groupbox, a_style, lms_kc_integer, a_sustain_port);
        }
        else
        {
            lms_sustain = new LMS_knob_regular(QString("Sustain"), 0, 100, 1, 50, QString("0.5"), lms_groupbox_adsr->lms_groupbox, a_style, lms_kc_decimal, a_sustain_port);
        }
        
        lms_groupbox_adsr->lms_add_h(lms_sustain);
        lms_release = new LMS_knob_regular(QString("Release"), 10, 200, 1, 50, QString(".5"), lms_groupbox_adsr->lms_groupbox, a_style, lms_kc_log_time, a_release_port);
        lms_groupbox_adsr->lms_add_h(lms_release);
    }
    
    LMS_group_box * lms_groupbox_adsr;    
    LMS_knob_regular *lms_attack;    
    LMS_knob_regular *lms_decay;    
    LMS_knob_regular *lms_sustain;
    LMS_knob_regular *lms_release;
};

#endif	/* ADSR_H */

