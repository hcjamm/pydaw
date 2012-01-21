/* 
 * File:   key.h
 * Author: vm-user
 *
 * Created on January 15, 2012, 12:54 PM
 */

#ifndef KEY_H
#define	KEY_H

#include <qt4/QtGui/QPushButton>

class _key : QPushButton
{
    public:
        _key(int _x, int _y, int _width, int _height, bool _white_key, unsigned char __note, QWidget * _parent)
        {
            this->setParent(_parent);
            
            this->setGeometry(_x, _y, _width, _height);
            
            if(_white_key)
                this->setStyleSheet("background-color : white;");
            else
                this->setStyleSheet("background-color : black;");
            
            this->_note = __note;
        }
        
        unsigned char _note;
};


#endif	/* KEY_H */

