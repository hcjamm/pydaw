/* 
 * File:   mod_matrix.h
 * Author: Jeff Hubbard
 * 
 *  A QTableWidget for use as a modulation matrix.  
 * It will contain widgets that control plugin parameters
 *
 * Created on April 1, 2012,10:41 AM
 */

#ifndef MOD_MATRIX_H
#define	MOD_MATRIX_H

#include <QApplication>
#include <QTableWidget>
#include <QStringList>

class LMS_mod_matrix
{
public:
    LMS_mod_matrix(QWidget * a_parent, QStringList a_columns, int a_row_count)
    {
        current_row = 0;
        current_column = 0;
        
        lms_mod_matrix = new QTableWidget(a_parent);
        lms_mod_matrix->setColumnCount(a_columns.count());   
        lms_mod_matrix->setRowCount(a_row_count);
        lms_mod_matrix->setHorizontalHeaderLabels(a_columns);
        lms_mod_matrix->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        lms_mod_matrix->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    }
    
    void lms_add_widget(QWidget * a_widget)
    {
        lms_mod_matrix->setCellWidget(current_row, current_column, a_widget);
        current_column++;
    }
    
    void lms_increment_row()
    {
        current_row++;
        current_column = 0;
    }
    
    QTableWidget * lms_mod_matrix;
    
private:
    int current_row;
    int current_column;
};


#endif	/* MOD_MATRIX_H */

