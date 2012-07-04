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
        QStringList f_types = QStringList() << QString("Off") << QString("LP2") << QString("LP4") << QString("HP2") << QString("HP4") << QString("BP2") << QString("BP4")
                 << QString("EQ") << QString("Distortion");
        
        lms_groupbox = new LMS_group_box(a_parent, a_title, a_style);
        
        lms_knob1 = new LMS_knob_regular(QString(""), 0, 127, 1, 65, QString("65"), a_parent, a_style, lms_kc_none, a_knob1_port);
        lms_knob2 = new LMS_knob_regular(QString(""), 0, 127, 1, 65, QString("65"), a_parent, a_style, lms_kc_none, a_knob2_port);
        lms_knob3 = new LMS_knob_regular(QString(""), 0, 127, 1, 65, QString("65"), a_parent, a_style, lms_kc_none, a_knob3_port);
        lms_combobox = new LMS_combobox(QString("Type"), a_parent, f_types, a_combobox_port, a_style);
        
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
                lms_knob1->lms_label->setText(QString("Off"));
                lms_knob2->lms_label->setText(QString("Off"));
                lms_knob3->lms_label->setText(QString("Off"));
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
            case 7: //EQ
                lms_knob1->lms_label->setText(QString("Cutoff"));
                lms_knob2->lms_label->setText(QString("Res"));
                lms_knob3->lms_label->setText(QString("Gain"));
                lms_knob1->lms_conv_type = lms_kc_127_pitch;
                lms_knob2->lms_conv_type = lms_kc_127_zero_to_x;
                lms_knob2->lms_set_127_min_max(-30.0f, 0.0f);
                lms_knob3->lms_conv_type = lms_kc_127_zero_to_x;
                lms_knob3->lms_set_127_min_max(-24.0f, 24.0f);
                break;
            case 8: //Distortion
                lms_knob1->lms_label->setText(QString("Gain"));
                lms_knob2->lms_label->setText(QString("Dry/Wet"));
                lms_knob3->lms_label->setText(QString("unused"));
                lms_knob1->lms_conv_type = lms_kc_127_zero_to_x;
                lms_knob1->lms_set_127_min_max(0.0f, 36.0f);
                lms_knob2->lms_conv_type = lms_kc_none;
                lms_knob2->lms_value->setText(QString(""));
                lms_knob3->lms_conv_type = lms_kc_none;
                lms_knob3->lms_value->setText(QString(""));
                break;
        }
    }
};

#endif	/* MULTIEFFECT_BASIC_H */

