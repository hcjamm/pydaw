/* 
 * File:   keyboard.h
 * Author: Jeff Hubbard
 * 
 * This file is not production-ready.  It will likely be deprecated in favor of 
 * using existing Jack-compatible virtual keyboards.
 *
 * Created on January 15, 2012, 12:41 PM
 */

#ifndef KEYBOARD_H
#define	KEYBOARD_H

#include <QFrame>
#include <QDial>
#include <QLabel>
#include <QLayout>
#include <QCheckBox>
#include "keyboard.h"

class _keyboard_widget : QGroupBox
{
    public:
        _keyboard_widget()
        {
            int _x = 0;
                        
            for(int i = 0; i < _key_count; i++)
            {
              
            }
        }
    protected:
        const int _key_count = 36;
        _key _keys [_key_count];
        const int _key_width = 20;
        const int _key_height = 60;
    protected slots:
        
};


#endif	/* KEYBOARD_H */

