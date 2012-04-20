/* 
 * File:   euphoria_poly_effects.h
 * Author: Jeff Hubbard
 * 
 * A class containing a polyphonic effects section used by Euphoria Sampler
 *
 * Created on April 17, 2012, 6:30 PM
 */

#ifndef EUPHORIA_POLY_EFFECTS_H
#define	EUPHORIA_POLY_EFFECTS_H

#include "adsr.h"
#include "filter.h"
#include "lfo.h"
#include "parametric_eq.h"
#include "../group_box.h"
#include "../knob_regular.h"
#include "../lms_main_layout.h"
#include <QTabWidget>

class LMS_euphoria_poly_effects
{
public:
    /* LMS_euphoria_poly_effects()
     *      
     */
    LMS_euphoria_poly_effects(QWidget * a_parent, LMS_style_info * a_style, 
            int a_filter1_ports [3], int a_filter2_ports[3],  //cutoff, res, type
            int a_adsr1_ports [4], int a_adsr2_ports[4],  //a, d, s, r
            int a_lfo_ports[2], //freq, type???
            int a_lfo_to_filter1_cutoff_port,
            int a_lfo_to_filter1_res_port,
            int a_lfo_to_filter2_cutoff_port,
            int a_lfo_to_filter2_res_port,
            int a_adsr1_to_filter1_cutoff_port,
            int a_adsr1_to_filter1_res_port,
            int a_adsr1_to_filter2_cutoff_port,
            int a_adsr1_to_filter2_res_port,
            int a_adsr2_to_filter1_cutoff_port,
            int a_adsr2_to_filter1_res_port,
            int a_adsr2_to_filter2_cutoff_port,
            int a_adsr2_to_filter2_res_port)
    {
        
    }
    
    LMS_main_layout * lms_layout;
    LMS_filter_widget * lms_filter1;
    LMS_filter_widget * lms_filter2;
    LMS_lfo_widget * lms_lfo;    
    LMS_adsr_widget * lms_adsr1;
    LMS_adsr_widget * lms_adsr2;
    
    LMS_knob_regular * lms_lfo_to_filter1_cutoff;
    LMS_knob_regular * lms_lfo_to_filter1_res;
    LMS_knob_regular * lms_lfo_to_filter2_cutoff;
    LMS_knob_regular * lms_lfo_to_filter2_res;
    LMS_knob_regular * lms_adsr1_to_filter1_cutoff;
    LMS_knob_regular * lms_adsr1_to_filter1_res;
    LMS_knob_regular * lms_adsr1_to_filter2_cutoff;
    LMS_knob_regular * lms_adsr1_to_filter2_res;
    LMS_knob_regular * lms_adsr2_to_filter1_cutoff;
    LMS_knob_regular * lms_adsr2_to_filter1_res;
    LMS_knob_regular * lms_adsr2_to_filter2_cutoff;
    LMS_knob_regular * lms_adsr2_to_filter2_res;
};

#endif	/* EUPHORIA_POLY_EFFECTS_H */

