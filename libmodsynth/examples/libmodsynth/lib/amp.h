/* 
 * File:   amp.h
 * Author: vm-user
 *
 * Created on January 8, 2012, 11:36 AM
 */

#ifndef AMP_H
#define	AMP_H

#ifdef	__cplusplus
extern "C" {
#endif

float _db_to_linear(float _db)
{
    float _result = pow ( 10.0, (0.05 * _db) );
    return _result;
}

float _linear_to_db(float _linear)
{
    float _result = 20.0 * log10 ( _linear );
    return _result;
}



#ifdef	__cplusplus
}
#endif

#endif	/* AMP_H */

