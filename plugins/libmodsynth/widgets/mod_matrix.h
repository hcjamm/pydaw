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
#include <QList>
#include <QRadioButton>
#include "lms_control.h"
#include "lms_spinbox.h"
#include "lms_combobox.h"
#include "lms_note_selector.h"

enum LMS_mm_column_type
    {
        no_widget, spinbox, combobox, radiobutton, note_selector
    };

class LMS_mod_matrix_column
{    
public:
    LMS_mod_matrix_column(LMS_mm_column_type a_type, QString a_column_header, int a_min, int a_max, int a_default)
    {
        lms_column_type = a_type;
        lms_column_header = a_column_header;
        min = a_min;
        max = a_max;
        default_value = a_default;
    }
    
    /* LMS_mod_matrix_column(QStringList a_combobox_items)
     * 
     * For the combobox type
     */
    LMS_mod_matrix_column(QStringList a_combobox_items, QString a_column_header)
    {
        lms_column_type = combobox;
        lms_column_header = a_column_header;
        min = 0;
        max = a_combobox_items.count() - 1;
        default_value = 0;
        lms_combobox_items = a_combobox_items;
    }
    
    
    
    QString lms_column_header;
    LMS_mm_column_type lms_column_type;
    int min;
    int max;
    int default_value;
    QList <LMS_control*> controls;
    QStringList lms_combobox_items;
};

class LMS_mod_matrix
{
public:
        
    QTableWidget * lms_mod_matrix;
    QList <LMS_mod_matrix_column*> lms_mm_columns;
    int lms_selected_column;
    
    LMS_mod_matrix(QWidget * a_parent, int a_row_count, QList<LMS_mod_matrix_column*> a_columns, int a_first_port, LMS_style_info * a_style)
    {
        lms_selected_column = 0;
        
        lms_mod_matrix = new QTableWidget(a_parent);        
        lms_mod_matrix->setRowCount(a_row_count);        
        lms_mod_matrix->setColumnCount(a_columns.count());
        lms_mod_matrix->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        lms_mod_matrix->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        
        QStringList f_headers;
        
        for(int f_i = 0; f_i < a_columns.count(); f_i++)
        {
            f_headers << a_columns[f_i]->lms_column_header;
        }
        
        lms_mod_matrix->setHorizontalHeaderLabels(f_headers);
                
        int f_port = a_first_port;
        
        for(int f_i = 0; f_i < a_columns.count(); f_i++)
        {            
            for(int f_i2 = 0; f_i2 < a_row_count; f_i2++)
            {
                switch(a_columns[f_i]->lms_column_type)
                {
                    case combobox:{
                        LMS_combobox * f_cb = new LMS_combobox(a_parent, a_columns[f_i]->lms_combobox_items, f_port, a_style);
                        a_columns[f_i]->controls.append(f_cb);
                        lms_mod_matrix->setCellWidget(f_i2, f_i, f_cb->lms_get_widget());
                    }
                        break;
                    case no_widget:
                        //Do nothing
                        break;
                    case note_selector:{
                        LMS_note_selector * f_ns = new LMS_note_selector(a_parent, f_port, a_style, a_columns[f_i]->default_value);
                        a_columns[f_i]->controls.append(f_ns);
                        lms_mod_matrix->setCellWidget(f_i2, f_i, f_ns->lms_get_widget());
                    }
                        break;
                    case radiobutton:{
                        QRadioButton * f_rb = new QRadioButton(a_parent);
                        if(f_i2 == 0)
                        {
                            f_rb->setChecked(TRUE);
                        }
                        lms_mod_matrix->setCellWidget(f_i2, f_i, f_rb);
                    }
                        break;
                    case spinbox:{
                        LMS_spinbox * f_sb = new LMS_spinbox(a_columns[f_i]->min, a_columns[f_i]->max, 1, a_columns[f_i]->default_value, a_parent, a_style, f_port);
                        a_columns[f_i]->controls.append(f_sb);
                        lms_mod_matrix->setCellWidget(f_i2, f_i, f_sb->lms_get_widget());
                    }
                        break;
                }
                
                f_port++;
            }
        }
        
    }
    
    void find_selected_radio_button(int a_radio_button_column_index)
    {
        for(int f_i = 0; f_i < lms_mod_matrix->rowCount(); f_i++)
        {
            QRadioButton * f_rb = (QRadioButton*)lms_mod_matrix->cellWidget(f_i , a_radio_button_column_index);
            
            if(f_rb->isChecked())
            {
                lms_selected_column = f_i;
                break;
            }
        }
    }
        
    
};


#endif	/* MOD_MATRIX_H */

