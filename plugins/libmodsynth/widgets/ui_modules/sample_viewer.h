/* 
 * File:   sample_viewer.h
 * Author: Jeff Hubbard
 * 
 * A sample viewer that can be used to edit sample start, sample end, loop start and loop end
 *
 * Created on April 3, 2012, 10:06 PM
 */

#ifndef SAMPLE_VIEWER_H
#define	SAMPLE_VIEWER_H

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QSlider>
#include <QSpacerItem>
#include <QSizePolicy>
#include "../sample_graph.h"

/*For sliders that calculate as a percentage, like sample start/end or loop start/end*/
#define SLIDER_LENGTH 10000
#define SLIDER_LENGTH_RECIP 1/SLIDER_LENGTH

class LMS_sample_viewer
{
public:
    QVBoxLayout *m_smp_tab_main_verticalLayout;
    
    LMS_sample_graph * lms_sample_graph;
    QSpinBox *m_sample_start_fine;    
    QSpinBox *m_sample_end_fine;    
    QSlider *m_sample_start;    
    QSlider *m_sample_end;    
    QSpinBox *m_loop_start_fine;        
    QSpinBox *m_loop_end_fine;    
    QSlider *m_loop_start;    
    QSlider *m_loop_end;
    
    bool m_handle_control_updates;
    int m_selected_sample_index;
    QList<int> m_sample_counts;
    
    LMS_sample_viewer(QWidget* a_parent)
    {
        QString f_stylesheet = QString("QSlider::groove:horizontal { border: 1px solid #bbb; background: white; height: 10px; border-radius: 4px; }  QSlider::sub-page:horizontal { background: qlineargradient(x1: 0, y1: 0,    x2: 0, y2: 1,     stop: 0 #66e, stop: 1 #bbf); background: qlineargradient(x1: 0, y1: 0.2, x2: 1, y2: 1,     stop: 0 #bbf, stop: 1 #55f); border: 1px solid #777; height: 10px; border-radius: 4px; }  QSlider::add-page:horizontal { background: #fff; border: 1px solid #777; height: 10px; border-radius: 4px; }  QSlider::handle:horizontal { background: qlineargradient(x1:0, y1:0, x2:1, y2:1,     stop:0 #eee, stop:1 #ccc); border: 1px solid #777; width: 13px; margin-top: -2px; margin-bottom: -2px; border-radius: 4px; }  QSlider::handle:horizontal:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:1,     stop:0 #fff, stop:1 #ddd); border: 1px solid #444; border-radius: 4px; }");
                    
        m_handle_control_updates = FALSE;
        m_selected_sample_index = 0;
        
        //TODO:  set elements for m_sample_counts
        
        m_smp_tab_main_verticalLayout = new QVBoxLayout();
        m_smp_tab_main_verticalLayout->setObjectName(QString::fromUtf8("m_smp_tab_main_verticalLayout"));
        
        /*TODO:  User configurable values*/
        lms_sample_graph = new LMS_sample_graph(32, 400, 800, a_parent);
        m_smp_tab_main_verticalLayout->addWidget(lms_sample_graph->m_sample_graph);
        
        m_smp_start_end_Layout = new QGridLayout();
        m_smp_start_end_Layout->setObjectName(QString::fromUtf8("m_smp_start_end_Layout"));
        m_smp_fine_Layout = new QHBoxLayout();
        m_smp_fine_Layout->setObjectName(QString::fromUtf8("m_smp_fine_Layout"));
        m_smp_start_fine_Label = new QLabel(a_parent);
        m_smp_start_fine_Label->setObjectName(QString::fromUtf8("m_smp_start_fine_Label"));

        m_smp_fine_Layout->addWidget(m_smp_start_fine_Label);

        m_sample_start_fine = new QSpinBox(a_parent);
        m_sample_start_fine->setObjectName(QString::fromUtf8("m_sample_start_fine"));
        m_sample_start_fine->setMaximumSize(QSize(100, 16777215));
        m_sample_start_fine->setMaximum(100000);

        m_smp_fine_Layout->addWidget(m_sample_start_fine);

        m_smp_end_fine_Label = new QLabel(a_parent);
        m_smp_end_fine_Label->setObjectName(QString::fromUtf8("m_smp_end_fine_Label"));

        m_smp_fine_Layout->addWidget(m_smp_end_fine_Label);

        m_sample_end_fine = new QSpinBox(a_parent);
        m_sample_end_fine->setObjectName(QString::fromUtf8("m_sample_end_fine"));
        m_sample_end_fine->setMaximumSize(QSize(100, 16777215));
        m_sample_end_fine->setMaximum(100000);

        m_smp_fine_Layout->addWidget(m_sample_end_fine);

        m_smp_start_end_hSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        m_smp_fine_Layout->addItem(m_smp_start_end_hSpacer);

        m_smp_start_end_Layout->addLayout(m_smp_fine_Layout, 3, 0, 1, 1);

        m_sample_start = new QSlider(a_parent);
        m_sample_start->setObjectName(QString::fromUtf8("m_sample_start"));
        m_sample_start->setMaximum(10000);
        m_sample_start->setOrientation(Qt::Horizontal);
        m_sample_start->setStyleSheet(f_stylesheet);

        m_smp_start_end_Layout->addWidget(m_sample_start, 1, 0, 1, 1);

        m_sm_start_end_Label = new QLabel(a_parent);
        m_sm_start_end_Label->setObjectName(QString::fromUtf8("m_sm_start_end_Label"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(m_sm_start_end_Label->sizePolicy().hasHeightForWidth());
        m_sm_start_end_Label->setSizePolicy(sizePolicy1);

        m_smp_start_end_Layout->addWidget(m_sm_start_end_Label, 0, 0, 1, 1);

        m_sample_end = new QSlider(a_parent);
        m_sample_end->setObjectName(QString::fromUtf8("m_sample_end"));
        m_sample_end->setMaximum(10000);
        m_sample_end->setSliderPosition(10000);
        m_sample_end->setOrientation(Qt::Horizontal);
        m_sample_end->setStyleSheet(f_stylesheet);

        m_smp_start_end_Layout->addWidget(m_sample_end, 2, 0, 1, 1);

        m_smp_tab_main_verticalLayout->addLayout(m_smp_start_end_Layout);

        m_loop_start_end_Layout = new QGridLayout();
        m_loop_start_end_Layout->setObjectName(QString::fromUtf8("m_loop_start_end_Layout"));
        m_loop_start_end_Layout->setSizeConstraint(QLayout::SetDefaultConstraint);
        m_loop_fine_Layout = new QHBoxLayout();
        m_loop_fine_Layout->setObjectName(QString::fromUtf8("m_loop_fine_Layout"));
        m_loop_start_fine_Label = new QLabel(a_parent);
        m_loop_start_fine_Label->setObjectName(QString::fromUtf8("m_loop_start_fine_Label"));

        m_loop_fine_Layout->addWidget(m_loop_start_fine_Label);

        m_loop_start_fine = new QSpinBox(a_parent);
        m_loop_start_fine->setObjectName(QString::fromUtf8("m_loop_start_fine"));
        m_loop_start_fine->setMaximumSize(QSize(100, 16777215));
        m_loop_start_fine->setMaximum(100000);

        m_loop_fine_Layout->addWidget(m_loop_start_fine);

        m_loop_end_fine_Label = new QLabel(a_parent);
        m_loop_end_fine_Label->setObjectName(QString::fromUtf8("m_loop_end_fine_Label"));

        m_loop_fine_Layout->addWidget(m_loop_end_fine_Label);

        m_loop_end_fine = new QSpinBox(a_parent);
        m_loop_end_fine->setObjectName(QString::fromUtf8("m_loop_end_fine"));
        m_loop_end_fine->setMaximumSize(QSize(100, 16777215));
        m_loop_end_fine->setMaximum(100000);

        m_loop_fine_Layout->addWidget(m_loop_end_fine);

        m_loop_start_end_hSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        m_loop_fine_Layout->addItem(m_loop_start_end_hSpacer);


        m_loop_start_end_Layout->addLayout(m_loop_fine_Layout, 3, 0, 1, 1);

        m_loop_start = new QSlider(a_parent);
        m_loop_start->setObjectName(QString::fromUtf8("m_loop_start"));
        m_loop_start->setMaximum(10000);
        m_loop_start->setOrientation(Qt::Horizontal);
        m_loop_start->setStyleSheet(f_stylesheet);

        m_loop_start_end_Layout->addWidget(m_loop_start, 1, 0, 1, 1);

        m_loop_start_end_Label = new QLabel(a_parent);
        m_loop_start_end_Label->setObjectName(QString::fromUtf8("m_loop_start_end_Label"));
        sizePolicy1.setHeightForWidth(m_loop_start_end_Label->sizePolicy().hasHeightForWidth());
        m_loop_start_end_Label->setSizePolicy(sizePolicy1);

        m_loop_start_end_Layout->addWidget(m_loop_start_end_Label, 0, 0, 1, 1);

        m_loop_end = new QSlider(a_parent);
        m_loop_end->setObjectName(QString::fromUtf8("m_loop_end"));
        m_loop_end->setMaximum(10000);
        m_loop_end->setSliderPosition(10000);
        m_loop_end->setOrientation(Qt::Horizontal);
        m_loop_end->setStyleSheet(f_stylesheet);

        m_loop_start_end_Layout->addWidget(m_loop_end, 2, 0, 1, 1);
        
        m_smp_tab_main_verticalLayout->addLayout(m_loop_start_end_Layout);

        m_handle_control_updates = TRUE;
        
        //Was retranslateUI();
        m_smp_start_fine_Label->setText(QApplication::translate("Frame", "Start", 0, QApplication::UnicodeUTF8));
        m_smp_end_fine_Label->setText(QApplication::translate("Frame", "End", 0, QApplication::UnicodeUTF8));
        m_sm_start_end_Label->setText(QApplication::translate("Frame", "Sample Start/End", 0, QApplication::UnicodeUTF8));
        m_loop_start_fine_Label->setText(QApplication::translate("Frame", "Start", 0, QApplication::UnicodeUTF8));
        m_loop_end_fine_Label->setText(QApplication::translate("Frame", "End", 0, QApplication::UnicodeUTF8));
        m_loop_start_end_Label->setText(QApplication::translate("Frame", "Loop Start/End", 0, QApplication::UnicodeUTF8));
    }
    
    
    void smp_start_Changed()
    {
        if(m_handle_control_updates)
        {            
            m_handle_control_updates = false;

            if(m_sample_counts[m_selected_sample_index] > 0)
            {
                int f_value = ((int)((m_sample_counts[m_selected_sample_index]) * SLIDER_LENGTH_RECIP * (m_sample_start->value())));

                QTableWidgetItem * f_widget = new QTableWidgetItem;
                f_widget->setText(QString::number(f_value));
                //setSampleStartFine(f_value);
                //m_sample_table->setItem(m_selected_sample_index, 13, f_widget);
            }

            m_handle_control_updates = true;
        }
    }    
    
    void smp_end_Changed()
    {
        if(m_handle_control_updates)
        {
            m_handle_control_updates = false;

            if(m_sample_counts[m_selected_sample_index] > 0)
            {
                int f_value = ((int)((m_sample_counts[m_selected_sample_index]) * SLIDER_LENGTH_RECIP * (m_sample_end->value())));

                QTableWidgetItem * f_widget = new QTableWidgetItem;
                f_widget->setText(QString::number(f_value));
                //setSampleEndFine(f_value);
                //m_sample_table->setItem(m_selected_sample_index, 14, f_widget);
            }
            m_handle_control_updates = true;
        }
    }
    
    void smp_start_fine_Changed()
    {
        if(m_handle_control_updates)
        {
            m_handle_control_updates = false;

            if(m_sample_counts[m_selected_sample_index] > 0)
            {
                int f_value = (int)(((float)(m_sample_start_fine->value())/(float)(m_sample_counts[m_selected_sample_index])) * SLIDER_LENGTH);
                
                QTableWidgetItem * f_widget = new QTableWidgetItem;
                f_widget->setText(QString::number((m_sample_start_fine->value())));
                //setSampleStart(f_value);
                //m_sample_table->setItem(m_selected_sample_index, 15, f_widget);
            }
            m_handle_control_updates = true;
        }
    }
    
    
    void smp_end_fine_Changed()
    {        
        if(m_handle_control_updates)
        {
            m_handle_control_updates = false;

            if(m_sample_counts[m_selected_sample_index] > 0)
            {
                int f_value = (int)(((float)(m_sample_end_fine->value())/(float)(m_sample_counts[m_selected_sample_index])) * SLIDER_LENGTH);
                
                QTableWidgetItem * f_widget = new QTableWidgetItem;
                f_widget->setText(QString::number((m_sample_end_fine->value())));
                //setSampleEnd(f_value);
                //m_sample_table->setItem(m_selected_sample_index, 15, f_widget);
            }
            m_handle_control_updates = true;
        }
    }
    
    void loop_start_Changed()
    {
        if(m_handle_control_updates)
        {
            m_handle_control_updates = false;

            if(m_sample_counts[m_selected_sample_index] > 0)
            {
                int f_value = ((int)((m_sample_counts[m_selected_sample_index]) * SLIDER_LENGTH_RECIP * (m_loop_start->value())));
                
                QTableWidgetItem * f_widget = new QTableWidgetItem;
                f_widget->setText(QString::number(f_value));
                //setLoopStartFine(f_value);
                //m_sample_table->setItem(m_selected_sample_index, 15, f_widget);
            }

            m_handle_control_updates = true;
        }
    }
    
    void loop_end_Changed()
    {
        if(m_handle_control_updates)
        {
            m_handle_control_updates = false;

            if(m_sample_counts[m_selected_sample_index] > 0)
            {
                int f_value = ((int)((m_sample_counts[m_selected_sample_index]) * SLIDER_LENGTH_RECIP * (m_loop_end->value())));
                
                QTableWidgetItem * f_widget = new QTableWidgetItem;
                f_widget->setText(QString::number(f_value));
                //setLoopEndFine(f_value);
                //m_sample_table->setItem(m_selected_sample_index, 16, f_widget);
            }

            m_handle_control_updates = true;
        }
    }
    
    
    void loop_start_fine_Changed()
    {
        if(m_handle_control_updates)
        {
            m_handle_control_updates = false;

            if(m_sample_counts[m_selected_sample_index] > 0)
            {
                int f_value = (int)(((float)(m_loop_start_fine->value())/(float)(m_sample_counts[m_selected_sample_index])) * SLIDER_LENGTH);
                
                QTableWidgetItem * f_widget = new QTableWidgetItem;
                f_widget->setText(QString::number((m_loop_start_fine->value())));
                //setLoopStart(f_value);
                //m_sample_table->setItem(m_selected_sample_index, 15, f_widget);
            }
            m_handle_control_updates = true;
        }
    }
    
    void loop_end_fine_Changed()
    {
        if(m_handle_control_updates)
        {
            m_handle_control_updates = false;

            if(m_sample_counts[m_selected_sample_index] > 0)
            {
                int f_value = (int)(((float)(m_loop_end_fine->value())/(float)(m_sample_counts[m_selected_sample_index])) * SLIDER_LENGTH);
                
                QTableWidgetItem * f_widget = new QTableWidgetItem;
                f_widget->setText(QString::number((m_loop_end_fine->value())));
                //setLoopEnd(f_value);
                //m_sample_table->setItem(m_selected_sample_index, 15, f_widget);
            }
            m_handle_control_updates = true;
        }
    }
    
private:
    QGridLayout *m_smp_start_end_Layout;
    QHBoxLayout *m_smp_fine_Layout;
    QLabel *m_smp_start_fine_Label;
    QLabel *m_smp_end_fine_Label;
    QSpacerItem *m_smp_start_end_hSpacer;
    QLabel *m_sm_start_end_Label;
    QGridLayout *m_loop_start_end_Layout;
    QHBoxLayout *m_loop_fine_Layout;
    QLabel *m_loop_start_fine_Label;
    QLabel *m_loop_end_fine_Label;
    QSpacerItem *m_loop_start_end_hSpacer;
    QLabel *m_loop_start_end_Label;
};

#endif	/* SAMPLE_VIEWER_H */

