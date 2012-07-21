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
#include <QLineEdit>
#include <QWidget>
#include <QStringList>
#include <QDir>
#include <QFile>
#include <QDragMoveEvent>
#include <QMap>

#define LMS_file_browser_bookmark_delimiter "|||"

class LMS_file_browser
{
public:    
    QVBoxLayout *m_file_browser_verticalLayout;
    QLabel *m_bookmarks_label;
    QListWidget *m_bookmarks_listWidget;
    QLabel *m_folders_label;
    QLineEdit *m_folder_path_label;
    QListWidget *m_folders_listWidget;    
    QPushButton *m_up_pushButton;
    QPushButton *m_bookmark_button;
    QLabel *m_files_label;
    QListWidget *m_files_listWidget;
    QPushButton *m_preview_pushButton;
    QPushButton *m_load_pushButton; 
    
    QHBoxLayout *m_folders_hlayout0;
    QHBoxLayout *m_files_hlayout0;
    
    QLineEdit * folder_path;
    QMap <QString, QString> hashtable;
    QString f_global_config_path;

    LMS_file_browser(QWidget *a_parent)
    {     
        m_file_browser_verticalLayout = new QVBoxLayout();
                
        m_file_browser_verticalLayout->setObjectName(QString::fromUtf8("m_file_browser_verticalLayout"));
        m_file_browser_verticalLayout->setContentsMargins(0, 0, 0, 0);
        m_bookmarks_label = new QLabel(a_parent);
        m_bookmarks_label->setObjectName(QString::fromUtf8("m_bookmarks_label"));
        m_bookmarks_label->setAlignment(Qt::AlignCenter);

        m_file_browser_verticalLayout->addWidget(m_bookmarks_label);
        
        m_folder_path_label = new QLineEdit(a_parent);
        m_folder_path_label->setObjectName(QString::fromUtf8("m_folder_path_label"));
        m_folder_path_label->setMaximumWidth(240);
        m_folder_path_label->setReadOnly(TRUE);
                
        m_bookmarks_listWidget = new QListWidget(a_parent);
        m_bookmarks_listWidget->setObjectName(QString::fromUtf8("m_bookmarks_listWidget"));
        m_bookmarks_listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        m_bookmarks_listWidget->setMaximumWidth(240);
        m_bookmarks_listWidget->setAcceptDrops(TRUE);
        m_bookmarks_listWidget->setDropIndicatorShown(TRUE);
        m_bookmarks_listWidget->setToolTip(QString("Drag and drop folders from the folder list here."));
        m_bookmarks_listWidget->setDragDropMode(QAbstractItemView::DropOnly);

        m_file_browser_verticalLayout->addWidget(m_bookmarks_listWidget);

        m_folders_label = new QLabel(a_parent);
        m_folders_label->setObjectName(QString::fromUtf8("m_folders_label"));
        m_folders_label->setAlignment(Qt::AlignCenter);

        m_file_browser_verticalLayout->addWidget(m_folders_label);
        
        m_file_browser_verticalLayout->addWidget(m_folder_path_label);

        m_folders_listWidget = new QListWidget(a_parent);
        m_folders_listWidget->setObjectName(QString::fromUtf8("m_folders_listWidget"));
        m_folders_listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        m_folders_listWidget->setMaximumWidth(240);
        m_folders_listWidget->setDragDropMode(QAbstractItemView::DragOnly);

        m_file_browser_verticalLayout->addWidget(m_folders_listWidget);
        
        m_folders_hlayout0 = new QHBoxLayout();
        
        m_up_pushButton = new QPushButton(a_parent);
        m_up_pushButton->setObjectName(QString::fromUtf8("m_up_pushButton"));
        m_up_pushButton->setMaximumWidth(120);

        m_folders_hlayout0->addWidget(m_up_pushButton);
        
        m_bookmark_button = new QPushButton(a_parent);
        m_bookmark_button->setObjectName(QString::fromUtf8("m_bookmark_button"));
        m_bookmark_button->setMaximumWidth(120);
        m_bookmark_button->setText(QString("Bookmark"));
        
        m_folders_hlayout0->addWidget(m_bookmark_button);
        
        m_file_browser_verticalLayout->addLayout(m_folders_hlayout0);

        m_files_label = new QLabel(a_parent);
        m_files_label->setObjectName(QString::fromUtf8("m_files_label"));
        m_files_label->setAlignment(Qt::AlignCenter);

        m_file_browser_verticalLayout->addWidget(m_files_label);

        m_files_listWidget = new QListWidget(a_parent);
        m_files_listWidget->setObjectName(QString::fromUtf8("m_files_listWidget"));
        m_files_listWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
        m_files_listWidget->setMaximumWidth(240);
        m_files_listWidget->setToolTip(QString("Select the file(s) you wish to load and click the 'load' button.\nThe samples will be loaded sequentially starting from the currently selected row."));

        m_file_browser_verticalLayout->addWidget(m_files_listWidget);

        m_files_hlayout0 = new QHBoxLayout();
        
        m_load_pushButton = new QPushButton(a_parent);
        m_load_pushButton->setObjectName(QString::fromUtf8("m_load_pushButton"));
        m_load_pushButton->setMaximumWidth(120);

        m_files_hlayout0->addWidget(m_load_pushButton);

        
        m_preview_pushButton = new QPushButton(a_parent);
        m_preview_pushButton->setObjectName(QString::fromUtf8("m_preview_pushButton"));
        m_preview_pushButton->setMaximumWidth(120);

        m_files_hlayout0->addWidget(m_preview_pushButton);
        
        m_file_browser_verticalLayout->addLayout(m_files_hlayout0);

        m_bookmarks_label->setText(QString("Bookmarks"));
        m_folders_label->setText(QString("Folders"));
        m_folder_path_label->setText(QString("/home"));
        m_up_pushButton->setText(QString("Up"));
        m_files_label->setText(QString("Files"));

        m_files_listWidget->setSortingEnabled(TRUE);

        m_preview_pushButton->setText(QString("Preview"));
        m_load_pushButton->setText(QString("Load"));
        
        f_global_config_path = QString(QDir::homePath() + QString("/dssi/lms_file_browser_bookmarks.txt"));
        
        if(QFile::exists(f_global_config_path))
        {
            QFile f_file(f_global_config_path);
            QTextStream f_in(&f_file);
            f_file.open(QIODevice::ReadOnly | QIODevice::Text);
            
            while(TRUE)
            {
                QString f_line = f_in.readLine();
                //TODO:  Also stop after an arbitrary maximum number of bookmarks
                if(f_line.isNull())
                {
                    break;
                }
                
                QStringList f_line_arr = f_line.split(QString(LMS_file_browser_bookmark_delimiter));
                
                //TODO:  Check f_line_arr
                
                hashtable.insert(f_line_arr.at(0), f_line_arr.at(1));
                m_bookmarks_listWidget->addItem(new QListWidgetItem(f_line_arr.at(0), m_bookmarks_listWidget));
            }

            f_file.close();
        }
        
        
        folder_opened(QString("/home/"), FALSE);
    }
    
    void folder_opened(QString a_folder, bool a_relative_path)
    {
        if(a_relative_path)
        {
            if(m_folder_path_label->text().compare(QString("/")) == 0)
            {
                enumerate_folders_and_files(QString("/") + a_folder);
            }
            else
            {
                enumerate_folders_and_files(m_folder_path_label->text() + QString("/") + a_folder);
            }
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
        
        QFileInfoList f_list = f_dir.entryInfoList();
        m_files_listWidget->clear();
        m_folders_listWidget->clear();
        
        m_folder_path_label->setText(f_dir.path());
        
        foreach(QFileInfo f_info, f_list)
        {
            
            if((f_info.fileName().compare(QString(".")) == 0) ||
                (f_info.fileName().compare(QString("..")) == 0))
            {
                continue;
            }
            
            if(f_info.isDir())
            {
                m_folders_listWidget->addItem(f_info.fileName());
            }
            else if((f_info.fileName().endsWith(".wav", Qt::CaseInsensitive) ||
                    f_info.fileName().endsWith(".ogg", Qt::CaseInsensitive) ||
                    f_info.fileName().endsWith(".aiff", Qt::CaseInsensitive)))
            {
                m_files_listWidget->addItem(f_info.fileName());
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
    
    void bookmark_button_pushed()
    {
                QFile f_file(f_global_config_path);
                f_file.open(QIODevice::WriteOnly | QIODevice::Text);

                QTextStream f_out(&f_file);

                for(int f_i = 0; f_i < m_bookmarks_listWidget->count(); f_i++)
                {
                    if((!hashtable.contains(m_bookmarks_listWidget->item(f_i)->text())))
                    {
                        hashtable.insert(m_bookmarks_listWidget->item(f_i)->text(), folder_path->text());
                    }
                }
                                
                foreach(const QString &key, hashtable.keys())
                {
                    f_out << key << LMS_file_browser_bookmark_delimiter << hashtable.value(key) << "\n";
                }

                f_out.flush();
                f_file.close();
    }
};

#endif	/* LMS_FILE_BROWSER_H */

