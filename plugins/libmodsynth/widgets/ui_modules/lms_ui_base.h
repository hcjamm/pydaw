/* 
 * File:   lms_ui_base.h
 * Author: Jeff Hubbard
 *
 * This class provides base functionality for all UI modules
 * 
 * Created on April 2, 2012, 10:43 PM
 */

#ifndef LMS_UI_BASE_H
#define	LMS_UI_BASE_H

#include <QList>
#include "../presets.h"

class LMS_ui_module
{
public:
    QList<LMS_control*> lms_control_list;
    
    void lms_add_to_presets(LMS_preset_manager * a_presets)
    {
        for(int f_i = 0; f_i < lms_control_list.count(); f_i++)
        {
            a_presets->lms_add_control(lms_control_list[f_i]);
        }
    }
};


#endif	/* LMS_UI_BASE_H */

