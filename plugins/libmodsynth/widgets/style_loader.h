/*
This file is part of the PyDAW project, Copyright PyDAW Team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef STYLE_LOADER_H
#define	STYLE_LOADER_H

#include <QString>
#include <QFile>

QString pydaw_load_style()
{
    QString f_user_style(QDir::homePath() + QString("/pydaw3/default-style.txt"));
    QString f_real_style_location("/usr/lib/pydaw3/themes/default/style.txt");
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

