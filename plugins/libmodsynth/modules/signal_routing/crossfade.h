/* 
 * File:   crossfade.h
 * Author: Jeff Hubbard
 * 
 * Purpose:  This file provides crossfader_1d, for crossfading between 2 signals
 *
 * Created on January 8, 2012, 1:55 PM
 */

#ifndef CROSSFADE_H
#define	CROSSFADE_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct _crossfader_1d
{
    float position_x;
}crossfader_1d;
    
/*TODO:  eventually allow for different curves, etc... hence this isn't calling up interpolate-linear from the library, 
 * even though the code is currently identical*/
float _xfd_crossfade_1d(crossfader_1d *, float, float);
    
float _xfd_crossfade_1d(crossfader_1d _xfd, float value_x1, float value_x2)
{
    return (((value_x1 - value_x2)*(_xfd->position_x))+value_x1);    
}

/*
float crossfade_2d(float position_x, float value_x1, float value_x2,
float position_y, float value_y1, float value_y2)
{
    
}

float crossfade_3d()
{}

*/

#ifdef	__cplusplus
}
#endif

#endif	/* CROSSFADE_1D_H */

