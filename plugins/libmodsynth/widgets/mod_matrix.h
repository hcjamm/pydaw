/* 
 * File:   mod_matrix.h
 * Author: Jeff Hubbard
 * 
 * Controls for use in a QTableWidget for a modulation matrix
 *
 * Created on April 1, 2012, 10:41 AM
 */

#ifndef MOD_MATRIX_H
#define	MOD_MATRIX_H

#include <QtGui/QApplication>
#include <QtGui/QSpinBox>
#include <QtGui/QComboBox>
#include <QStringList>

/* A QSpinBox with the following fields added:
 * 
 * int lms_column, lms_row, lms_group;
 * 
 */
class MMSpinBox : public QSpinBox
{    
    public:
        void lms_setup(int,int,int,int,int,int);
        int lms_column, lms_row, lms_group;         
};

/* void MMSpinBox::lms_setup(
 * int a_column,  //The zero-based column index of the parent QTableWidget
 * int a_row,     //The zero-based row index of the parent QTableWidget
 * int a_group,   //An integer uniquely identifying controls that this one may be grouped with.  Enter something random if you don't intend to use this
 * int a_min,     //The minimum value of the SpinBox
 * int a_max,     //The maximum value of the SpinBox 
 * int a_value)   //The initial value of the Spinbox
 */
void MMSpinBox::lms_setup(int a_column, int a_row, int a_group, int a_min, int a_max, int a_value)
{   
    lms_column = a_column;
    lms_group = a_group;
    lms_row = a_row;
    setMaximum(a_max);
    setMinimum(a_min);
    setValue(a_value); 
}


/* A QComboBox with the following fields added:
 * 
 * int lms_column, lms_row, lms_group;
 * 
 */
class MMComboBox : public QComboBox
{    
    public:
        void lms_setup(int,int,int,QStringList);
        int lms_column, lms_row, lms_group;
};

/* void MMComboBox::lms_setup(
 * int a_column,  //The zero-based column index of the parent QTableWidget
 * int a_row,     //The zero-based row index of the parent QTableWidget
 * int a_group,   //An integer uniquely identifying controls that this one may be grouped with.  Enter something random if you don't intend to use this
 * QStringList a_items) //The items to populate the list with
 */
void MMComboBox::lms_setup(int a_column, int a_row, int a_group, QStringList a_items)
{
    lms_column = a_column;
    lms_group = a_group;
    lms_row = a_row;
    insertItems(0, a_items); 
}


#endif	/* MOD_MATRIX_H */

