/* 
 * File:   lms_control.h
 * Author: Jeff Hubbard
 * 
 * This is the parent class that all controls such as knobs, sliders, etc... should inherit
 *
 * Created on April 2, 2012, 6:30 PM
 */

#ifndef LMS_CONTROL_H
#define	LMS_CONTROL_H

#include <QLabel>
#include <QDial>
#include <QLayout>

class LMS_style_info
{
public:
    LMS_style_info(int a_knob_size)
    {
        lms_knob_size = a_knob_size;
        lms_use_label_style = FALSE;
    };
    
    void LMS_set_label_style(QString a_style, int a_width)
    {
        lms_use_label_style = TRUE;
        lms_label_style = a_style;
        lms_label_width = a_width;
    }
    
    void LMS_set_value_style(QString a_style, int a_width)
    {
        lms_use_label_style = TRUE;
        lms_label_style = a_style;
        lms_value_width = a_width;
    }
    
    void LMS_set_groupbox_style(QString a_style)
    {
        lms_use_groupbox_style = TRUE;
        lms_groupbox_style = a_style;
    }
    
    int lms_knob_size;
    
    int lms_label_width;
    int lms_label_value;    
    QString lms_label_style;
    bool lms_use_label_style;
    
    int lms_value_width;
    QString lms_value_style;
    bool lms_use_value_style;
    
    QString lms_groupbox_style;
    bool lms_use_groupbox_style;
};

class LMS_control
{
public:
    int lms_port;
    virtual void lms_set_value(int) = 0;
    virtual void lms_value_changed(int) = 0;
    virtual int lms_get_value() = 0;
    virtual QLayout * lms_get_layout() = 0;
    virtual QWidget * lms_get_widget() = 0;
};

#endif	/* LMS_CONTROL_H */

