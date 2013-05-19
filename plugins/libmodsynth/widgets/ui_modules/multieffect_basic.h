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
        QStringList f_types = QStringList() << QString("Off") << QString("LP2") 
                << QString("LP4") << QString("HP2") << QString("HP4") << QString("BP2") << QString("BP4")
                 << QString("Notch2") << QString("Notch4") << QString("EQ") 
                << QString("Distortion") << QString("Comb Filter") << QString("Amp/Pan") << QString("Limiter")
                << QString("Saturator") << QString("Formant") << QString("Chorus") << QString("Glitch")
                << QString("RingMod") << QString("LoFi") << QString("S/H");
        
        lms_groupbox = new LMS_group_box(a_parent, a_title, a_style);
        
        lms_knob1 = new LMS_knob_regular(QString(""), 0, 127, 1, 65, QString(""), a_parent, a_style, lms_kc_none, a_knob1_port);
        lms_knob2 = new LMS_knob_regular(QString(""), 0, 127, 1, 65, QString(""), a_parent, a_style, lms_kc_none, a_knob2_port);
        lms_knob3 = new LMS_knob_regular(QString(""), 0, 127, 1, 65, QString(""), a_parent, a_style, lms_kc_none, a_knob3_port);
        lms_combobox = new LMS_combobox(QString("Type"), a_parent, f_types, a_combobox_port, a_style);
        lms_combobox->lms_combobox->setMinimumWidth(132);
        
        lms_groupbox->lms_add_h(lms_knob1);
        lms_groupbox->lms_add_h(lms_knob2);
        lms_groupbox->lms_add_h(lms_knob3);
        lms_groupbox->lms_add_h(lms_combobox);        
    }
        
    LMS_group_box * lms_groupbox;
    LMS_knob_regular * lms_knob1;
    LMS_knob_regular * lms_knob2;
    LMS_knob_regular * lms_knob3;
    LMS_combobox * lms_combobox;
    
    void lms_combobox_changed()
    {
        switch(lms_combobox->lms_get_value())
        {
            case 0: //Off
                lms_knob1->lms_label->setText(QString(""));
                lms_knob2->lms_label->setText(QString(""));
                lms_knob3->lms_label->setText(QString(""));
                lms_knob1->lms_conv_type = lms_kc_none;
                lms_knob2->lms_conv_type = lms_kc_none;
                lms_knob3->lms_conv_type = lms_kc_none;
                lms_knob1->lms_value->setText(QString(""));
                lms_knob2->lms_value->setText(QString(""));
                lms_knob3->lms_value->setText(QString(""));
                break;
            case 1: //LP2
                lms_knob1->lms_label->setText(QString("Cutoff"));
                lms_knob2->lms_label->setText(QString("Res"));
                lms_knob3->lms_label->setText(QString("unused"));
                lms_knob1->lms_conv_type = lms_kc_127_pitch;
                lms_knob2->lms_conv_type = lms_kc_127_zero_to_x;
                lms_knob2->lms_set_127_min_max(-30.0f, 0.0f);
                lms_knob3->lms_conv_type = lms_kc_none;
                lms_knob3->lms_value->setText(QString(""));
                break;
            case 2: //LP4
                lms_knob1->lms_label->setText(QString("Cutoff"));
                lms_knob2->lms_label->setText(QString("Res"));
                lms_knob3->lms_label->setText(QString("unused"));
                lms_knob1->lms_conv_type = lms_kc_127_pitch;
                lms_knob2->lms_conv_type = lms_kc_127_zero_to_x;
                lms_knob2->lms_set_127_min_max(-30.0f, 0.0f);
                lms_knob3->lms_conv_type = lms_kc_none;
                lms_knob3->lms_value->setText(QString(""));
                break;
            case 3: //HP2
                lms_knob1->lms_label->setText(QString("Cutoff"));
                lms_knob2->lms_label->setText(QString("Res"));
                lms_knob3->lms_label->setText(QString("unused"));
                lms_knob1->lms_conv_type = lms_kc_127_pitch;
                lms_knob2->lms_conv_type = lms_kc_127_zero_to_x;
                lms_knob2->lms_set_127_min_max(-30.0f, 0.0f);
                lms_knob3->lms_conv_type = lms_kc_none;
                lms_knob3->lms_value->setText(QString(""));
                break;
            case 4: //HP4
                lms_knob1->lms_label->setText(QString("Cutoff"));
                lms_knob2->lms_label->setText(QString("Res"));
                lms_knob3->lms_label->setText(QString("unused"));
                lms_knob1->lms_conv_type = lms_kc_127_pitch;
                lms_knob2->lms_conv_type = lms_kc_127_zero_to_x;
                lms_knob2->lms_set_127_min_max(-30.0f, 0.0f);
                lms_knob3->lms_conv_type = lms_kc_none;
                lms_knob3->lms_value->setText(QString(""));
                break;
            case 5: //BP2
                lms_knob1->lms_label->setText(QString("Cutoff"));
                lms_knob2->lms_label->setText(QString("Res"));
                lms_knob3->lms_label->setText(QString("unused"));
                lms_knob1->lms_conv_type = lms_kc_127_pitch;
                lms_knob2->lms_conv_type = lms_kc_127_zero_to_x;
                lms_knob2->lms_set_127_min_max(-30.0f, 0.0f);
                lms_knob3->lms_conv_type = lms_kc_none;
                lms_knob3->lms_value->setText(QString(""));
                break;
            case 6: //BP4
                lms_knob1->lms_label->setText(QString("Cutoff"));
                lms_knob2->lms_label->setText(QString("Res"));
                lms_knob3->lms_label->setText(QString("unused"));
                lms_knob1->lms_conv_type = lms_kc_127_pitch;
                lms_knob2->lms_conv_type = lms_kc_127_zero_to_x;
                lms_knob2->lms_set_127_min_max(-30.0f, 0.0f);
                lms_knob3->lms_conv_type = lms_kc_none;
                lms_knob3->lms_value->setText(QString(""));
                break;
            case 7: //Notch2
                lms_knob1->lms_label->setText(QString("Cutoff"));
                lms_knob2->lms_label->setText(QString("Res"));
                lms_knob3->lms_label->setText(QString("unused"));
                lms_knob1->lms_conv_type = lms_kc_127_pitch;
                lms_knob2->lms_conv_type = lms_kc_127_zero_to_x;
                lms_knob2->lms_set_127_min_max(-30.0f, 0.0f);
                lms_knob3->lms_conv_type = lms_kc_none;
                lms_knob3->lms_value->setText(QString(""));                
                break;
            case 8: //Notch4
                lms_knob1->lms_label->setText(QString("Cutoff"));
                lms_knob2->lms_label->setText(QString("Res"));
                lms_knob3->lms_label->setText(QString("unused"));
                lms_knob1->lms_conv_type = lms_kc_127_pitch;
                lms_knob2->lms_conv_type = lms_kc_127_zero_to_x;
                lms_knob2->lms_set_127_min_max(-30.0f, 0.0f);
                lms_knob3->lms_conv_type = lms_kc_none;
                lms_knob3->lms_value->setText(QString(""));                
                break;
            case 9: //EQ
                lms_knob1->lms_label->setText(QString("Cutoff"));
                lms_knob2->lms_label->setText(QString("Q"));
                lms_knob3->lms_label->setText(QString("Gain"));
                lms_knob1->lms_conv_type = lms_kc_127_pitch;
                lms_knob2->lms_conv_type = lms_kc_none;
                lms_knob3->lms_conv_type = lms_kc_127_zero_to_x;
                lms_knob3->lms_set_127_min_max(-24.0f, 24.0f);
                lms_knob2->lms_value->setText(QString(""));
                break;
            case 10: //Distortion
                lms_knob1->lms_label->setText(QString("Gain"));
                lms_knob2->lms_label->setText(QString("Dry/Wet"));
                lms_knob3->lms_label->setText(QString("Out Gain"));
                lms_knob1->lms_conv_type = lms_kc_127_zero_to_x;
                lms_knob1->lms_set_127_min_max(0.0f, 36.0f);
                lms_knob2->lms_conv_type = lms_kc_none;
                lms_knob2->lms_value->setText(QString(""));
                lms_knob3->lms_conv_type = lms_kc_127_zero_to_x;
                lms_knob3->lms_set_127_min_max(-12.0f, 0.0f);
                break;
            case 11: //Comb Filter
                lms_knob1->lms_label->setText(QString("Cutoff"));
                lms_knob2->lms_label->setText(QString("Amt"));
                lms_knob3->lms_label->setText(QString("unused"));
                lms_knob1->lms_conv_type = lms_kc_none;
                lms_knob2->lms_conv_type = lms_kc_none;                
                lms_knob3->lms_conv_type = lms_kc_none;
                lms_knob1->lms_value->setText(QString(""));
                lms_knob2->lms_value->setText(QString(""));
                lms_knob3->lms_value->setText(QString(""));
                break;
            case 12: //Amp/Panner
                lms_knob1->lms_label->setText(QString("Pan"));
                lms_knob2->lms_label->setText(QString("Amp"));
                lms_knob3->lms_label->setText(QString("unused"));
                lms_knob1->lms_conv_type = lms_kc_none;
                lms_knob2->lms_conv_type = lms_kc_127_zero_to_x;
                lms_knob2->lms_set_127_min_max(-30.0f, 6.0f);
                lms_knob3->lms_conv_type = lms_kc_none;
                lms_knob1->lms_value->setText(QString(""));
                lms_knob2->lms_value->setText(QString(""));
                lms_knob3->lms_value->setText(QString(""));
                break;
            case 13: //Limiter
                lms_knob1->lms_label->setText(QString("Thresh"));
                lms_knob2->lms_label->setText(QString("Ceiling"));
                lms_knob3->lms_label->setText(QString("Release"));
                lms_knob1->lms_conv_type = lms_kc_127_zero_to_x;
                lms_knob1->lms_set_127_min_max(-30.0f, 0.0f);
                lms_knob2->lms_conv_type = lms_kc_127_zero_to_x;
                lms_knob2->lms_set_127_min_max(-12.0f, -0.1f);
                lms_knob3->lms_conv_type = lms_kc_127_zero_to_x_int;
                lms_knob3->lms_set_127_min_max(150.0f, 400.0f);
                break;
            case 14: //Saturator
                lms_knob1->lms_label->setText(QString("InGain"));
                lms_knob2->lms_label->setText(QString("Amt"));
                lms_knob3->lms_label->setText(QString("OutGain"));
                lms_knob1->lms_conv_type = lms_kc_127_zero_to_x;
                lms_knob1->lms_set_127_min_max(-12.0f, 12.0f);
                lms_knob2->lms_conv_type = lms_kc_none;
                lms_knob2->lms_value->setText(QString(""));
                lms_knob3->lms_conv_type = lms_kc_127_zero_to_x;
                lms_knob3->lms_set_127_min_max(-12.0f, 12.0f);                
            case 15: //Formant Filter
                lms_knob1->lms_label->setText(QString("Vowel"));
                lms_knob2->lms_label->setText(QString("Wet"));
                lms_knob3->lms_label->setText(QString("unused"));
                lms_knob1->lms_conv_type = lms_kc_none;
                lms_knob1->lms_value->setText(QString(""));
                lms_knob2->lms_conv_type = lms_kc_none;
                lms_knob2->lms_value->setText(QString(""));    
                lms_knob3->lms_conv_type = lms_kc_none;
                lms_knob3->lms_value->setText(QString(""));                
                break;
            case 16: //Chorus
                lms_knob1->lms_label->setText(QString("Rate"));
                lms_knob2->lms_label->setText(QString("Wet"));
                lms_knob3->lms_label->setText(QString("unused"));
                lms_knob1->lms_conv_type = lms_kc_127_zero_to_x;
                lms_knob1->lms_set_127_min_max(0.3f, 6.0f);
                lms_knob1->lms_value->setText(QString(""));
                lms_knob2->lms_conv_type = lms_kc_none;
                lms_knob2->lms_value->setText(QString(""));    
                lms_knob3->lms_conv_type = lms_kc_none;
                lms_knob3->lms_value->setText(QString(""));                
                break;                
            case 17: //Glitch
                lms_knob1->lms_label->setText(QString("Pitch"));
                lms_knob2->lms_label->setText(QString("Glitch"));
                lms_knob3->lms_label->setText(QString("Wet"));
                lms_knob1->lms_conv_type = lms_kc_none;                
                lms_knob1->lms_value->setText(QString(""));
                lms_knob2->lms_conv_type = lms_kc_none;
                lms_knob2->lms_value->setText(QString(""));    
                lms_knob3->lms_conv_type = lms_kc_none;
                lms_knob3->lms_value->setText(QString(""));                
                break;                
            case 18: //RingMod
                lms_knob1->lms_label->setText(QString("Pitch"));
                lms_knob2->lms_label->setText(QString("Wet"));
                lms_knob3->lms_label->setText(QString("unused"));
                lms_knob1->lms_conv_type = lms_kc_none;                
                lms_knob1->lms_value->setText(QString(""));
                lms_knob2->lms_conv_type = lms_kc_none;
                lms_knob2->lms_value->setText(QString(""));    
                lms_knob3->lms_conv_type = lms_kc_none;
                lms_knob3->lms_value->setText(QString(""));                
                break;        
            case 19: //LoFi
                lms_knob1->lms_label->setText(QString("Bits"));
                lms_knob2->lms_label->setText(QString("unused"));
                lms_knob3->lms_label->setText(QString("unused"));
                lms_knob1->lms_conv_type = lms_kc_127_zero_to_x;                
                lms_knob1->lms_set_127_min_max(4.0f, 16.0f);
                lms_knob2->lms_conv_type = lms_kc_none;
                lms_knob2->lms_value->setText(QString(""));    
                lms_knob3->lms_conv_type = lms_kc_none;
                lms_knob3->lms_value->setText(QString(""));                
                break;                
            case 20: //Sample and Hold
                lms_knob1->lms_label->setText(QString("Pitch"));
                lms_knob2->lms_label->setText(QString("Wet"));
                lms_knob3->lms_label->setText(QString("unused"));
                lms_knob1->lms_conv_type = lms_kc_none;                
                lms_knob1->lms_value->setText(QString(""));
                lms_knob2->lms_conv_type = lms_kc_none;
                lms_knob2->lms_value->setText(QString(""));    
                lms_knob3->lms_conv_type = lms_kc_none;
                lms_knob3->lms_value->setText(QString(""));                
                break;
        }
        
        lms_knob1->lms_value_changed(lms_knob1->lms_get_value());
        lms_knob2->lms_value_changed(lms_knob2->lms_get_value());
        lms_knob3->lms_value_changed(lms_knob3->lms_get_value());
    }
};

#endif	/* MULTIEFFECT_BASIC_H */

