/* 
 * File:   multieffect_basic.h
 * Author: jeffh
 * 
 * This class encapsulates multiple effect types that can be switched with the control's combobox
 *
 * Created on April 4, 2012, 1:23 PM
 */

#ifndef MULTIEFFECT_BASIC_H
#define	MULTIEFFECT_BASIC_H

#include "../group_box.h"
#include "../knob_regular.h"
#include "../lms_combobox.h"

class LMS_multieffect
{
public:
    LMS_multieffect(QWidget * a_parent, QString a_title, LMS_style_info * a_style, int a_knob1_port, int a_knob2_port, int a_knob3_port, int a_combobox_port)
    {
        lms_groupbox = new LMS_group_box(a_parent, a_title, a_style);
        
    }
        
    LMS_group_box * lms_groupbox;
    LMS_knob_regular * lms_knob1;
    LMS_knob_regular * lms_knob2;
    LMS_knob_regular * lms_knob3;
    LMS_combobox * lms_combobox;

    void lms_knob1Changed()
    {
        
    }
    
    void lms_knob2Changed()
    {
        
    }
    
    void lms_knob3Changed()
    {
        
    }
    
    void lms_comboboxChanged()
    {
        
    }
};

#endif	/* MULTIEFFECT_BASIC_H */

