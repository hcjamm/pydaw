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

