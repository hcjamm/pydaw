/* 
 * File:   lfo.h
 * Author: Jeff Hubbard
 * 
 * A UI module for an LFO
 *
 * Created on April 3, 2012, 8:20 PM
 */

#ifndef LFO_WIDGET_H
#define	LFO_WIDGET_H

#include "../group_box.h"
#include "../knob_regular.h"
#include "../lms_combobox.h"

class LMS_lfo_widget
{
public:
    LMS_lfo_widget(QWidget * a_parent, LMS_style_info * a_style, int a_freq_port, int a_type_port, QStringList a_types, QString a_label)
    {
        lms_groupbox = new LMS_group_box(a_parent, QString(a_label), a_style);
                
        lms_freq_knob = new LMS_knob_regular(QString("Freq"), 10, 400, 1, 200, QString("0"), lms_groupbox->lms_groupbox, a_style, lms_kc_decimal, a_freq_port);
        lms_groupbox->lms_add_h(lms_freq_knob);
        lms_type_combobox = new LMS_combobox(QString("Type"), lms_groupbox->lms_groupbox, a_types, a_type_port, a_style);        
        lms_type_combobox->lms_combobox->setMinimumWidth(96);
        lms_groupbox->lms_add_h(lms_type_combobox);        
    }
    
    LMS_group_box * lms_groupbox;
    LMS_knob_regular * lms_freq_knob;
    LMS_combobox * lms_type_combobox;
    
};

#endif	/* LFO_WIDGET_H */

