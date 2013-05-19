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

#ifndef MASTER_WIDGET_H
#define	MASTER_WIDGET_H

#include "../group_box.h"
#include "../knob_regular.h"
#include "../lms_combobox.h"

class LMS_master_widget
{
public:
    LMS_master_widget(QWidget * a_parent, LMS_style_info * a_style, int a_vol_port, int a_u_voice_port, int a_u_spread_port,
            int a_glide_port, int a_pitchbend_port, QString a_label, bool a_show_unison_controls = TRUE)
    {
        lms_groupbox = new LMS_group_box(a_parent, QString(a_label), a_style);
        
        lms_master_volume =  new LMS_knob_regular(QString("Vol"), -60, 12, 1, -6, QString("0"), lms_groupbox->lms_groupbox, a_style, lms_kc_integer, a_vol_port);
        lms_groupbox->lms_add_h(lms_master_volume);
        if(a_show_unison_controls)
        {
            lms_master_unison_voices  = new LMS_knob_regular(QString("Unison"), 1, 7, 1, 1, QString("1"), lms_groupbox->lms_groupbox, a_style, lms_kc_integer, a_u_voice_port);        
            lms_groupbox->lms_add_h(lms_master_unison_voices);
            lms_master_unison_spread  =  new LMS_knob_regular(QString("Spread"), 10, 100, 1, 20, QString(".2"), lms_groupbox->lms_groupbox, a_style, lms_kc_decimal, a_u_spread_port);
            lms_groupbox->lms_add_h(lms_master_unison_spread);
        }
        lms_master_glide  = new LMS_knob_regular(QString("Glide"), 0, 200, 1, 0, QString("0"), lms_groupbox->lms_groupbox, a_style, lms_kc_decimal, a_glide_port);
        lms_groupbox->lms_add_h(lms_master_glide);    
        lms_master_pitchbend_amt = new LMS_knob_regular(QString("Pitchbend"), 1, 36, 1, 2, QString("2"), lms_groupbox->lms_groupbox, a_style, lms_kc_integer, a_pitchbend_port);
        lms_groupbox->lms_add_h(lms_master_pitchbend_amt);
                
    }
        
    LMS_group_box * lms_groupbox;
    LMS_knob_regular *lms_master_volume;    
    LMS_knob_regular *lms_master_unison_voices;
    LMS_knob_regular *lms_master_unison_spread;
    LMS_knob_regular *lms_master_glide;
    LMS_knob_regular *lms_master_pitchbend_amt;
    
};

#endif	/* MASTER_H */

