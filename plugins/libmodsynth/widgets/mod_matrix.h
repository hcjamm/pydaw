/* 
 * File:   mod_matrix.h
 * Author: Jeff Hubbard
 * 
 *  A QTableWidget that automatically creates rows based on column definitions
 *
 * Created on April 1, 2012,10:41 AM
 */

#ifndef MOD_MATRIX_H
#define	MOD_MATRIX_H

#include <QApplication>
#include <QTableWidget>
#include <QList>
#include "lms_control.h"

/*Flags for defining special control variants*/
#define LMS_MM_NO_FLAG 0
#define LMS_MM_LINEEDIT_ENTRY 1
#define LMS_MM_LINEEDIT_FS 2

enum lms_mm_widget_type
{
    lms_mm_spinbox, lms_mm_combobox, lms_mm_knob, lms_mm_text, lms_mm_lineedit
};

class LMS_mm_column_info
{
public:
    LMS_mm_column_info(){}
    
    lms_mm_widget_type lms_type;
    int min_value;
    int max_value;
    int lms_flag;
    QString lms_name;
};

class LMS_mod_matrix : public LMS_control
{
    public:
        LMS_mod_matrix(int a_row_count, QWidget * a_parent)
        {
            lms_matrix = new QTableWidget(a_parent);
            lms_matrix->setRowCount(a_row_count);
            row_count = a_row_count;
        }
        
        void add_column(LMS_mm_column_info a_column_info)
        {
            lms_columns.append(a_column_info);
        }
        
        /* void setup_matrix()
         * 
         * Call this once you've added all of your columns.  It should only  be called once.
         */
        void setup_matrix()
        {
            for(int f_i = 0; f_i < lms_columns; f_i++)
            {
                for(int f_i2 = 0; f_i2 < row_count; f_i2++)
                {
                    QTableWidgetItem * f_item = new QTableWidgetItem(0);
                    
                    switch(lms_columns[f_i]->lms_type)
                    {
                        case lms_mm_combobox:                            
                            break;
                        case lms_mm_knob:
                            break;
                        case lms_mm_lineedit:
                            break;
                        case lms_mm_spinbox:
                            break;
                        case lms_mm_text:
                            break;
                    }
                }
            }
        }
        
        QTableWidget * lms_matrix;
        QList <LMS_mm_column_info*> lms_columns;
        int row_count;
};


#endif	/* MOD_MATRIX_H */

