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

#ifndef RAMP_ENV_WIDGET_H
#define	RAMP_ENV_WIDGET_H

#include "../group_box.h"
#include "../knob_regular.h"

class LMS_ramp_env
{
public:
    LMS_ramp_env(QWidget * a_parent, LMS_style_info * a_style, int a_time_port, int a_amt_port, int a_curve_port, bool a_show_curve_knob, QString a_label, bool a_show_amt_knob)
    {
        lms_groupbox = new LMS_group_box(a_parent, a_label, a_style);
        
        if(a_show_amt_knob)
        {
            lms_amt_knob = new LMS_knob_regular(QString("Amt"), -36, 36, 1, 0, QString(""), lms_groupbox->lms_groupbox, a_style, lms_kc_integer, a_amt_port);
            lms_groupbox->lms_add_h(lms_amt_knob);
        }
        
        lms_time_knob = new LMS_knob_regular(QString("Time"), 1, 200, 1, 1, QString("0"), lms_groupbox->lms_groupbox, a_style, lms_kc_decimal, a_time_port);
        lms_groupbox->lms_add_h(lms_time_knob);        
        if(a_show_curve_knob)
        {
            lms_curve_knob = new LMS_knob_regular(QString("Curve"), 1, 200, 1, 1, QString("0"), lms_groupbox->lms_groupbox, a_style, lms_kc_decimal, a_time_port);
            lms_groupbox->lms_add_h(lms_curve_knob);
        }
    }
    
    LMS_group_box * lms_groupbox;
    LMS_knob_regular * lms_time_knob;
    LMS_knob_regular * lms_amt_knob;
    LMS_knob_regular * lms_curve_knob;
};


#endif	/* RAMP_ENV_WIDGET_H */

