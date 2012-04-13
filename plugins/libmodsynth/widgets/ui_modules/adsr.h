/* 
 * File:   adsr.h
 * Author: Jeff Hubbard
 *
 * Created on April 4, 2012, 10:39 PM
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
        lms_attack =  new LMS_knob_regular(QString("Attack"), 1, 100, 1, 1, QString(".01"), lms_groupbox_adsr->lms_groupbox, a_style, lms_kc_decimal, a_attack_port);
        lms_groupbox_adsr->lms_add_h(lms_attack);
        lms_decay =  new LMS_knob_regular(QString("Decay"), 1, 100, 1, 1, QString(".01"), lms_groupbox_adsr->lms_groupbox, a_style, lms_kc_decimal, a_decay_port);
        lms_groupbox_adsr->lms_add_h(lms_decay);    
        lms_sustain = new LMS_knob_regular(QString("Sustain"), -60, 0, 1, -6, QString("-6"), lms_groupbox_adsr->lms_groupbox, a_style, lms_kc_integer, a_sustain_port);
        lms_groupbox_adsr->lms_add_h(lms_sustain);
        lms_release = new LMS_knob_regular(QString("Release"), 1, 400, 1, 50, QString(".5"), lms_groupbox_adsr->lms_groupbox, a_style, lms_kc_decimal, a_release_port);
        lms_groupbox_adsr->lms_add_h(lms_release);
    }
    
    LMS_group_box * lms_groupbox_adsr;    
    LMS_knob_regular *lms_attack;    
    LMS_knob_regular *lms_decay;    
    LMS_knob_regular *lms_sustain;
    LMS_knob_regular *lms_release;
};

#endif	/* ADSR_H */

