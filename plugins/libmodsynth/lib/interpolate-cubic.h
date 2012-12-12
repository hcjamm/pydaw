/* 
 * File:   interpolate-cubic.h
 * 
 *  
 */

#ifndef INTERPOLATE_CUBIC_H
#define	INTERPOLATE_CUBIC_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct 
{
    float a0,a1,a2,a3,mu,mu2;
    int int_pos, int_pos_plus1, int_pos_minus1, int_pos_minus2;
}t_cubic_interpolater;

t_cubic_interpolater * g_cubic_get();

t_cubic_interpolater * g_cubic_get()
{
    t_cubic_interpolater * f_result;
    
    if(posix_memalign((void**)&f_result, 16, sizeof(t_cubic_interpolater)) != 0)
    {
        return 0;
    }
    
    f_result->a0 = 0.0f;
    f_result->a1 = 0.0f;
    f_result->a2 = 0.0f;
    f_result->a3 = 0.0f;
    f_result->mu = 0.0f;
    f_result->mu2 = 0.0f;
    
    f_result->int_pos = 0;
    f_result->int_pos_minus1 = 0;
    f_result->int_pos_minus2 = 0;
    f_result->int_pos_plus1 = 0;
    
    return f_result;
}

//inline float f_cubic_interpolate(float, float, float);
inline float f_cubic_interpolate_ptr_wrap(float*,int,float, t_cubic_interpolater*);
inline float f_cubic_interpolate_ptr(float*, float, t_cubic_interpolater*);

/* inline float f_cubic_interpolate(
 * float a_a, //item 0
 * float a_b, //item 1
 * float a_position)  //position between the 2, range:  0 to 1
 */
/*
inline float f_cubic_interpolate(float a_a, float a_b, float a_position)
{    
    return (((a_a - a_b) * a_position) + a_a);
}
*/


/* inline float f_cubic_interpolate_ptr_wrap(
 * float * a_table, 
 * int a_table_size, 
 * float a_ptr, 
 * t_cubic_interpolater * a_lin)
 * 
 * This method uses a pointer instead of an array the float* must be malloc'd to (sizeof(float) * a_table_size)
 */
inline float f_cubic_interpolate_ptr_wrap(float * a_table, int a_table_size, float a_ptr, t_cubic_interpolater * a_cubic)
{        
    a_cubic->int_pos = (int)a_ptr;
    a_cubic->int_pos_plus1 = (a_cubic->int_pos) + 1;
    a_cubic->int_pos_minus1 = (a_cubic->int_pos) - 1;
    a_cubic->int_pos_minus2 = (a_cubic->int_pos) - 2;
    
    if(a_cubic->int_pos_plus1 >= a_table_size)
    {
        a_cubic->int_pos_plus1 = (a_cubic->int_pos_plus1) - a_table_size;
    }
    
    
    if(a_cubic->int_pos_minus1 < 0)
    {
        a_cubic->int_pos_minus1 = (a_cubic->int_pos_minus1) + a_table_size;
    }
        
    if(a_cubic->int_pos_minus2 < 0)
    {
        a_cubic->int_pos_minus2 = (a_cubic->int_pos_minus2) + a_table_size;
    }
    
    a_cubic->mu = a_ptr - (a_cubic->int_pos);
    
    a_cubic->mu2 = (a_cubic->mu) * (a_cubic->mu);
    a_cubic->a0 = a_table[a_cubic->int_pos_plus1] - a_table[a_cubic->int_pos] - a_table[a_cubic->int_pos_minus2] + a_table[a_cubic->int_pos_minus1];
    a_cubic->a1 = a_table[a_cubic->int_pos_minus2] - a_table[a_cubic->int_pos_minus1] - a_cubic->a0;
    a_cubic->a2 = a_table[a_cubic->int_pos] - a_table[a_cubic->int_pos_minus2];
    a_cubic->a3 = a_table[a_cubic->int_pos_minus1];

    return(a_cubic->a0*a_cubic->mu*a_cubic->mu2+a_cubic->a1*a_cubic->mu2+a_cubic->a2*a_cubic->mu+a_cubic->a3);        
}

/* inline float f_cubic_interpolate_ptr_wrap(
 * float * a_table, 
 * float a_ptr, 
 * t_cubic_interpolater * a_lin)
 * 
 * This method uses a pointer instead of an array the float* must be malloc'd to (sizeof(float) * a_table_size)
 * 
 * THIS DOES NOT CHECK THAT YOU PROVIDED A VALID POSITION
 */

/*
inline float f_cubic_interpolate_ptr(float * a_table, float a_ptr, t_cubic_interpolater * a_lin)
{        
    a_lin->int_pos = (int)a_ptr;
    a_lin->int_pos_plus_1 = (a_lin->int_pos) + 1;
        
    a_lin->pos = a_ptr - (a_lin->int_pos);
    
    return (((a_table[(a_lin->int_pos)]) - (a_table[(a_lin->int_pos_plus_1)])) * (a_lin->pos)) + (a_table[(a_lin->int_pos_plus_1)]);
}
*/

#ifdef	__cplusplus
}
#endif

#endif	/* INTERPOLATE_CUBIC_H */

