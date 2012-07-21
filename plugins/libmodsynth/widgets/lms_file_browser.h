/* 
 * File:   lms_file_browser.h
 * Author: jeffh
 * 
 * A full-featured file browser
 *
 * Created on July 20, 2012, 5:43 PM
 */

#ifndef LMS_FILE_BROWSER_H
#define	LMS_FILE_BROWSER_H

#include <QVariant>
#include <QAction>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QStringList>
#include <QDir>
#include <QFile>

class LMS_file_browser
{
public:    
    QVBoxLayout *m_file_browser_verticalLayout;
    QLabel *m_bookmarks_label;
    QListWidget *m_bookmarks_listWidget;
    QLabel *m_folders_label;
    QLabel *m_folder_path_label;
    QListWidget *m_folders_listWidget;    
    QPushButton *m_up_pushButton;
    QLabel *m_files_label;
    QListWidget *m_files_listWidget;
    QPushButton *m_preview_pushButton;
    QPushButton *m_load_pushButton; 

    LMS_file_browser(QWidget *a_parent)
    {     
        m_file_browser_verticalLayout = new QVBoxLayout();
                
        m_file_browser_verticalLayout->setObjectName(QString::fromUtf8("m_file_browser_verticalLayout"));
        m_file_browser_verticalLayout->setContentsMargins(0, 0, 0, 0);
        m_bookmarks_label = new QLabel(a_parent);
        m_bookmarks_label->setObjectName(QString::fromUtf8("m_bookmarks_label"));
        m_bookmarks_label->setAlignment(Qt::AlignCenter);

        m_file_browser_verticalLayout->addWidget(m_bookmarks_label);

        m_bookmarks_listWidget = new QListWidget(a_parent);
        m_bookmarks_listWidget->setObjectName(QString::fromUtf8("m_bookmarks_listWidget"));
        m_bookmarks_listWidget->setSelectionMode(QAbstractItemView::NoSelection);
        m_bookmarks_listWidget->setMaximumWidth(240);

        m_file_browser_verticalLayout->addWidget(m_bookmarks_listWidget);

        m_folders_label = new QLabel(a_parent);
        m_folders_label->setObjectName(QString::fromUtf8("m_folders_label"));
        m_folders_label->setAlignment(Qt::AlignCenter);

        m_file_browser_verticalLayout->addWidget(m_folders_label);

        m_folder_path_label = new QLabel(a_parent);
        m_folder_path_label->setObjectName(QString::fromUtf8("m_folder_path_label"));

        m_file_browser_verticalLayout->addWidget(m_folder_path_label);

        m_folders_listWidget = new QListWidget(a_parent);
        m_folders_listWidget->setObjectName(QString::fromUtf8("m_folders_listWidget"));
        m_folders_listWidget->setSelectionMode(QAbstractItemView::NoSelection);
        m_folders_listWidget->setMaximumWidth(240);

        m_file_browser_verticalLayout->addWidget(m_folders_listWidget);
        
        m_up_pushButton = new QPushButton(a_parent);
        m_up_pushButton->setObjectName(QString::fromUtf8("m_up_pushButton"));
        m_up_pushButton->setMaximumWidth(240);

        m_file_browser_verticalLayout->addWidget(m_up_pushButton);

        m_files_label = new QLabel(a_parent);
        m_files_label->setObjectName(QString::fromUtf8("m_files_label"));
        m_files_label->setAlignment(Qt::AlignCenter);

        m_file_browser_verticalLayout->addWidget(m_files_label);

        m_files_listWidget = new QListWidget(a_parent);
        m_files_listWidget->setObjectName(QString::fromUtf8("m_files_listWidget"));
        m_files_listWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
        m_files_listWidget->setMaximumWidth(240);

        m_file_browser_verticalLayout->addWidget(m_files_listWidget);
        
        m_load_pushButton = new QPushButton(a_parent);
        m_load_pushButton->setObjectName(QString::fromUtf8("m_load_pushButton"));
        m_load_pushButton->setMaximumWidth(240);

        m_file_browser_verticalLayout->addWidget(m_load_pushButton);

        
        m_preview_pushButton = new QPushButton(a_parent);
        m_preview_pushButton->setObjectName(QString::fromUtf8("m_preview_pushButton"));
        m_preview_pushButton->setMaximumWidth(240);

        m_file_browser_verticalLayout->addWidget(m_preview_pushButton);

        m_bookmarks_label->setText(QString("Bookmarks"));
        m_folders_label->setText(QString("Folders"));
        m_folder_path_label->setText(QString("/home"));
        m_up_pushButton->setText(QString("Up"));
        m_files_label->setText(QString("Files"));

        m_files_listWidget->setSortingEnabled(TRUE);

        m_preview_pushButton->setText(QString("Preview"));
        m_load_pushButton->setText(QString("Load"));
        
        folder_opened(QString("/home/"), FALSE);
    }
    
    void folder_opened(QString a_folder, bool a_relative_path)
    {
        if(a_relative_path)
        {
            enumerate_folders_and_files(m_folder_path_label->text() + "/" + a_folder);
        }
        else
        {
            enumerate_folders_and_files(a_folder);
        }
    }
    
    void enumerate_folders_and_files(QString a_path)
    {   
        QDir f_dir(a_path);        
        
        if(!f_dir.exists())
        {
            //TODO:  notify?
            return;
        }
        
        QStringList f_list = f_dir.entryList();
        m_files_listWidget->clear();
        m_folders_listWidget->clear();
        
        m_folder_path_label->setText(f_dir.path());
        
        QDir f_test_dir;
        
        foreach(QString f_entry, f_list)
        {
            QFileInfo f_info(f_entry);
            
            if(f_info.fileName().endsWith(".wav", Qt::CaseInsensitive) ||
                    f_info.fileName().endsWith(".ogg", Qt::CaseInsensitive) ||
                    f_info.fileName().endsWith(".aiff", Qt::CaseInsensitive))
            {
                m_files_listWidget->addItem(f_info.fileName());
            }
            else if(f_test_dir.exists(f_info.absolutePath()))
            {
                m_folders_listWidget->addItem(f_info.fileName());
            }            
        }
    }
    
    /* Returns a QStringList of the files that should be opened*/
    QStringList files_opened()
    {
        QStringList f_result;
                
        QList<QListWidgetItem*> f_widget_items = m_files_listWidget->selectedItems();
        
        for(int f_i = 0; f_i < f_widget_items.count(); f_i++)
        {
            f_result << f_widget_items[f_i]->text();
        }
        
        return f_result;
    }
    
    void up_one_folder()
    {
        //TODO:  Some checks before we just fire this off...
        QFileInfo f_current_folder(m_folder_path_label->text());
        
        enumerate_folders_and_files(f_current_folder.absoluteDir().absolutePath());
    }

};



#endif	/* LMS_FILE_BROWSER_H */

