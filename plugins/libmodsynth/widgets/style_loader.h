/* 
 * File:   style_loader.h
 * Author: JeffH
 *
 * Load the default CSS and pixmap theme
 * 
 * Created on February 3, 2013, 8:34 PM
 */

#ifndef STYLE_LOADER_H
#define	STYLE_LOADER_H

#include <QString>
#include <QFile>

QString pydaw_load_style()
{
    QString f_user_style(QDir::homePath() + QString("/pydaw2/default-style.txt"));
    QString f_real_style_location("/usr/lib/pydaw2/themes/default/style.txt");
    if(QFile::exists(f_user_style))
    {
        QFile f_user_style_file(f_user_style);
        f_user_style_file.open(QIODevice::ReadOnly | QIODevice::Text);
        QString f_user_style_text(f_user_style_file.readAll());
        if(QFile::exists(f_user_style_text))
        {
            f_real_style_location = f_user_style_text;
        }
        f_user_style_file.close();
    }
    
    QFile f_stylesheet_file(f_real_style_location);
    f_stylesheet_file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString f_result(f_stylesheet_file.readAll());
    f_stylesheet_file.close();
    return f_result;
}

#endif	/* STYLE_LOADER_H */

