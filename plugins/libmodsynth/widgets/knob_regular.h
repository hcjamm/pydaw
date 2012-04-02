/* 
 * File:   knob_regular.h
 * Author: Jeff Hubbard
 *
 * A plain knob and label nested in a QVBoxLayout
 * 
 * Created on April 1, 2012, 11:46 AM
 */

#ifndef KNOB_REGULAR_H
#define	KNOB_REGULAR_H

#include <QtGui/QApplication>
#include <QVBoxLayout>
#include <QString>
#include <QLabel>
#include <QDial>
#include <math.h>

/*
extern "C"{
#include <lo/lo.h>
}
*/

/* Convert the knob value to a different format for display
 * lmk_integer  Display as it is
 * lmk_pitch    Convert from pitch to Hz
 * lmk_decimal  multiply by .01
 * lmk_off      don't update the label
 */
enum LMS_KNOB_CONVERSION{lmk_integer, lmk_pitch, lmk_decimal, lmk_off};


/* This class passes global information for the plugin to new knobs
 */


class LMS_knob_info
{    
    public:
        LMS_knob_info(){};
    
};


class LMS_knob_regular
{   
    public:
        /* LMS_knob_regular(
         * QString a_label,
         * int a_min, 
         * int a_max, 
         * int a_step_size, 
         * int a_value, 
         * QString a_label_value, 
         * LMS_KNOB_CONVERSION a_conv_type, 
         * QWidget *a_parent, 
         * int a_lms_port, 
         * LMS_knob_info * a_knob_info)
         */
        LMS_knob_regular(QString a_label,int a_min, int a_max, int a_step_size, int a_value, 
        QString a_label_value, LMS_KNOB_CONVERSION a_conv_type, QWidget *a_parent, int a_lms_port, LMS_knob_info * a_knob_info)
        {
            lms_layout = new QVBoxLayout(a_parent);
            lms_label = new QLabel(a_parent);
            lms_label->setText(a_label);
            lms_knob = new QDial(a_parent);
            lms_knob->setMinimum(a_min);
            lms_knob->setMaximum(a_max);
            lms_knob->setSingleStep(a_step_size);
            lms_knob->setValue(a_value);
            lms_value = new QLabel(a_parent);
            lms_value->setText(a_label_value);
            lms_conversion_type = a_conv_type;
            lms_port = a_lms_port;
                                    
            lms_layout->addWidget(lms_label, -1, Qt::AlignCenter);
            lms_layout->addWidget(lms_knob, -1, Qt::AlignCenter);
            lms_layout->addWidget(lms_value, -1, Qt::AlignCenter);
            
            lms_value_changed(lms_knob->value());            
        }
                
        QVBoxLayout * lms_layout;
        QLabel * lms_label;
        QDial * lms_knob;        
        QLabel * lms_value;        
        int lms_port;
        LMS_KNOB_CONVERSION lms_conversion_type;
        
        
        bool * lms_suppressHostUpdate;
        //lo_address lms_host;
        QByteArray lms_controlPath;
        
        ~LMS_knob_regular(){};
    public slots:
        void lms_value_changed(int a_value)
        {
            switch(lms_conversion_type)
            {
                case lmk_integer:
                    lms_value->setText(QString::number(a_value));
                    break;
                case lmk_decimal:
                    lms_value->setText(QString::number(((float)a_value) * .01));
                    break;
                case lmk_pitch:
                    lms_value->setText(QString::number(
                    (440*pow(2,(a_value-57)*.0833333))
                    ));
                    break;
                case lmk_off:
                    //Do nothing
                    break;
            }
                       
        }
};


#endif	/* KNOB_REGULAR_H */

