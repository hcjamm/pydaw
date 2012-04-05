/********************************************************************************
** Form generated from reading UI file 'mainv2w26266.ui'
**
** Created: Wed Apr 4 14:06:17 2012
**      by: Qt User Interface Compiler version 4.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef MAINV2W26266_H
#define MAINV2W26266_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QScrollArea>
#include <QtGui/QSlider>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QTabWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Frame
{
public:
    QHBoxLayout *horizontalLayout_5;
    QTabWidget *m_main_tab;
    QWidget *m_sample_tab;
    QHBoxLayout *horizontalLayout_2;
    QScrollArea *m_smp_tab_scrollArea;
    QWidget *m_smp_tab_scrollAreaWidgetContents;
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *m_smp_tab_main_verticalLayout;
    QGridLayout *m_smp_start_end_Layout;
    QHBoxLayout *m_smp_fine_Layout;
    QLabel *m_smp_start_fine_Label;
    QSpinBox *m_sample_start_fine;
    QLabel *m_smp_end_fine_Label;
    QSpinBox *m_sample_end_fine;
    QSpacerItem *m_smp_start_end_hSpacer;
    QSlider *m_sample_start;
    QLabel *m_sm_start_end_Label;
    QSlider *m_sample_end;
    QGridLayout *m_loop_start_end_Layout;
    QHBoxLayout *m_loop_fine_Layout;
    QLabel *m_loop_start_fine_Label;
    QSpinBox *m_loop_start_fine;
    QLabel *m_loop_end_fine_Label;
    QSpinBox *m_loop_end_fine;
    QSpacerItem *m_loop_start_end_hSpacer;
    QSlider *m_loop_start;
    QLabel *m_loop_start_end_Label;
    QSlider *m_loop_end;
    QWidget *m_poly_fx_tab;
    QHBoxLayout *horizontalLayout_4;
    QVBoxLayout *m_poly_fx_vertical_layout;
    QHBoxLayout *m_poly_fx_Layout;

    void setupUi(QFrame *Frame)
    {
        if (Frame->objectName().isEmpty())
            Frame->setObjectName(QString::fromUtf8("Frame"));
        Frame->resize(1023, 801);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(Frame->sizePolicy().hasHeightForWidth());
        Frame->setSizePolicy(sizePolicy);
        Frame->setFrameShape(QFrame::StyledPanel);
        Frame->setFrameShadow(QFrame::Raised);
        horizontalLayout_5 = new QHBoxLayout(Frame);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        m_main_tab = new QTabWidget(Frame);
        m_main_tab->setObjectName(QString::fromUtf8("m_main_tab"));
        m_main_tab->setStyleSheet(QString::fromUtf8(""));
        m_sample_tab = new QWidget();
        m_sample_tab->setObjectName(QString::fromUtf8("m_sample_tab"));
        horizontalLayout_2 = new QHBoxLayout(m_sample_tab);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        m_smp_tab_scrollArea = new QScrollArea(m_sample_tab);
        m_smp_tab_scrollArea->setObjectName(QString::fromUtf8("m_smp_tab_scrollArea"));
        m_smp_tab_scrollArea->setStyleSheet(QString::fromUtf8(""));
        m_smp_tab_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        m_smp_tab_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        m_smp_tab_scrollArea->setWidgetResizable(true);
        m_smp_tab_scrollAreaWidgetContents = new QWidget();
        m_smp_tab_scrollAreaWidgetContents->setObjectName(QString::fromUtf8("m_smp_tab_scrollAreaWidgetContents"));
        m_smp_tab_scrollAreaWidgetContents->setGeometry(QRect(0, 0, 966, 728));
        horizontalLayout = new QHBoxLayout(m_smp_tab_scrollAreaWidgetContents);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        m_smp_tab_main_verticalLayout = new QVBoxLayout();
        m_smp_tab_main_verticalLayout->setObjectName(QString::fromUtf8("m_smp_tab_main_verticalLayout"));
        m_smp_start_end_Layout = new QGridLayout();
        m_smp_start_end_Layout->setObjectName(QString::fromUtf8("m_smp_start_end_Layout"));
        m_smp_fine_Layout = new QHBoxLayout();
        m_smp_fine_Layout->setObjectName(QString::fromUtf8("m_smp_fine_Layout"));
        m_smp_start_fine_Label = new QLabel(m_smp_tab_scrollAreaWidgetContents);
        m_smp_start_fine_Label->setObjectName(QString::fromUtf8("m_smp_start_fine_Label"));

        m_smp_fine_Layout->addWidget(m_smp_start_fine_Label);

        m_sample_start_fine = new QSpinBox(m_smp_tab_scrollAreaWidgetContents);
        m_sample_start_fine->setObjectName(QString::fromUtf8("m_sample_start_fine"));
        m_sample_start_fine->setMaximumSize(QSize(100, 16777215));
        m_sample_start_fine->setMaximum(100000);

        m_smp_fine_Layout->addWidget(m_sample_start_fine);

        m_smp_end_fine_Label = new QLabel(m_smp_tab_scrollAreaWidgetContents);
        m_smp_end_fine_Label->setObjectName(QString::fromUtf8("m_smp_end_fine_Label"));

        m_smp_fine_Layout->addWidget(m_smp_end_fine_Label);

        m_sample_end_fine = new QSpinBox(m_smp_tab_scrollAreaWidgetContents);
        m_sample_end_fine->setObjectName(QString::fromUtf8("m_sample_end_fine"));
        m_sample_end_fine->setMaximumSize(QSize(100, 16777215));
        m_sample_end_fine->setMaximum(100000);

        m_smp_fine_Layout->addWidget(m_sample_end_fine);

        m_smp_start_end_hSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        m_smp_fine_Layout->addItem(m_smp_start_end_hSpacer);


        m_smp_start_end_Layout->addLayout(m_smp_fine_Layout, 3, 0, 1, 1);

        m_sample_start = new QSlider(m_smp_tab_scrollAreaWidgetContents);
        m_sample_start->setObjectName(QString::fromUtf8("m_sample_start"));
        m_sample_start->setMaximum(10000);
        m_sample_start->setOrientation(Qt::Horizontal);

        m_smp_start_end_Layout->addWidget(m_sample_start, 1, 0, 1, 1);

        m_sm_start_end_Label = new QLabel(m_smp_tab_scrollAreaWidgetContents);
        m_sm_start_end_Label->setObjectName(QString::fromUtf8("m_sm_start_end_Label"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(m_sm_start_end_Label->sizePolicy().hasHeightForWidth());
        m_sm_start_end_Label->setSizePolicy(sizePolicy1);

        m_smp_start_end_Layout->addWidget(m_sm_start_end_Label, 0, 0, 1, 1);

        m_sample_end = new QSlider(m_smp_tab_scrollAreaWidgetContents);
        m_sample_end->setObjectName(QString::fromUtf8("m_sample_end"));
        m_sample_end->setMaximum(10000);
        m_sample_end->setSliderPosition(10000);
        m_sample_end->setOrientation(Qt::Horizontal);

        m_smp_start_end_Layout->addWidget(m_sample_end, 2, 0, 1, 1);


        m_smp_tab_main_verticalLayout->addLayout(m_smp_start_end_Layout);

        m_loop_start_end_Layout = new QGridLayout();
        m_loop_start_end_Layout->setObjectName(QString::fromUtf8("m_loop_start_end_Layout"));
        m_loop_start_end_Layout->setSizeConstraint(QLayout::SetDefaultConstraint);
        m_loop_fine_Layout = new QHBoxLayout();
        m_loop_fine_Layout->setObjectName(QString::fromUtf8("m_loop_fine_Layout"));
        m_loop_start_fine_Label = new QLabel(m_smp_tab_scrollAreaWidgetContents);
        m_loop_start_fine_Label->setObjectName(QString::fromUtf8("m_loop_start_fine_Label"));

        m_loop_fine_Layout->addWidget(m_loop_start_fine_Label);

        m_loop_start_fine = new QSpinBox(m_smp_tab_scrollAreaWidgetContents);
        m_loop_start_fine->setObjectName(QString::fromUtf8("m_loop_start_fine"));
        m_loop_start_fine->setMaximumSize(QSize(100, 16777215));
        m_loop_start_fine->setMaximum(100000);

        m_loop_fine_Layout->addWidget(m_loop_start_fine);

        m_loop_end_fine_Label = new QLabel(m_smp_tab_scrollAreaWidgetContents);
        m_loop_end_fine_Label->setObjectName(QString::fromUtf8("m_loop_end_fine_Label"));

        m_loop_fine_Layout->addWidget(m_loop_end_fine_Label);

        m_loop_end_fine = new QSpinBox(m_smp_tab_scrollAreaWidgetContents);
        m_loop_end_fine->setObjectName(QString::fromUtf8("m_loop_end_fine"));
        m_loop_end_fine->setMaximumSize(QSize(100, 16777215));
        m_loop_end_fine->setMaximum(100000);

        m_loop_fine_Layout->addWidget(m_loop_end_fine);

        m_loop_start_end_hSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        m_loop_fine_Layout->addItem(m_loop_start_end_hSpacer);


        m_loop_start_end_Layout->addLayout(m_loop_fine_Layout, 3, 0, 1, 1);

        m_loop_start = new QSlider(m_smp_tab_scrollAreaWidgetContents);
        m_loop_start->setObjectName(QString::fromUtf8("m_loop_start"));
        m_loop_start->setMaximum(10000);
        m_loop_start->setOrientation(Qt::Horizontal);

        m_loop_start_end_Layout->addWidget(m_loop_start, 1, 0, 1, 1);

        m_loop_start_end_Label = new QLabel(m_smp_tab_scrollAreaWidgetContents);
        m_loop_start_end_Label->setObjectName(QString::fromUtf8("m_loop_start_end_Label"));
        sizePolicy1.setHeightForWidth(m_loop_start_end_Label->sizePolicy().hasHeightForWidth());
        m_loop_start_end_Label->setSizePolicy(sizePolicy1);

        m_loop_start_end_Layout->addWidget(m_loop_start_end_Label, 0, 0, 1, 1);

        m_loop_end = new QSlider(m_smp_tab_scrollAreaWidgetContents);
        m_loop_end->setObjectName(QString::fromUtf8("m_loop_end"));
        m_loop_end->setMaximum(10000);
        m_loop_end->setSliderPosition(10000);
        m_loop_end->setOrientation(Qt::Horizontal);

        m_loop_start_end_Layout->addWidget(m_loop_end, 2, 0, 1, 1);


        m_smp_tab_main_verticalLayout->addLayout(m_loop_start_end_Layout);


        horizontalLayout->addLayout(m_smp_tab_main_verticalLayout);

        m_smp_tab_scrollArea->setWidget(m_smp_tab_scrollAreaWidgetContents);

        horizontalLayout_2->addWidget(m_smp_tab_scrollArea);

        m_main_tab->addTab(m_sample_tab, QString());
        m_poly_fx_tab = new QWidget();
        m_poly_fx_tab->setObjectName(QString::fromUtf8("m_poly_fx_tab"));
        horizontalLayout_4 = new QHBoxLayout(m_poly_fx_tab);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        m_poly_fx_vertical_layout = new QVBoxLayout();
        m_poly_fx_vertical_layout->setObjectName(QString::fromUtf8("m_poly_fx_vertical_layout"));
        m_poly_fx_Layout = new QHBoxLayout();
        m_poly_fx_Layout->setObjectName(QString::fromUtf8("m_poly_fx_Layout"));

        m_poly_fx_vertical_layout->addLayout(m_poly_fx_Layout);


        horizontalLayout_4->addLayout(m_poly_fx_vertical_layout);

        m_main_tab->addTab(m_poly_fx_tab, QString());

        horizontalLayout_5->addWidget(m_main_tab);


        retranslateUi(Frame);

        m_main_tab->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(Frame);
    } // setupUi

    void retranslateUi(QFrame *Frame)
    {
        Frame->setWindowTitle(QApplication::translate("Frame", "Euphoria - Powered by LibModSynth", 0, QApplication::UnicodeUTF8));
        m_smp_start_fine_Label->setText(QApplication::translate("Frame", "Start", 0, QApplication::UnicodeUTF8));
        m_smp_end_fine_Label->setText(QApplication::translate("Frame", "End", 0, QApplication::UnicodeUTF8));
        m_sm_start_end_Label->setText(QApplication::translate("Frame", "Sample Start/End", 0, QApplication::UnicodeUTF8));
        m_loop_start_fine_Label->setText(QApplication::translate("Frame", "Start", 0, QApplication::UnicodeUTF8));
        m_loop_end_fine_Label->setText(QApplication::translate("Frame", "End", 0, QApplication::UnicodeUTF8));
        m_loop_start_end_Label->setText(QApplication::translate("Frame", "Loop Start/End", 0, QApplication::UnicodeUTF8));
        m_main_tab->setTabText(m_main_tab->indexOf(m_sample_tab), QApplication::translate("Frame", "Samples", 0, QApplication::UnicodeUTF8));
        m_main_tab->setTabText(m_main_tab->indexOf(m_poly_fx_tab), QApplication::translate("Frame", "Poly FX", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Frame: public Ui_Frame {};
} // namespace Ui

QT_END_NAMESPACE

#endif // MAINV2W26266_H

