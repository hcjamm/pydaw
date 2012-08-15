/* 
 * File:   lms_session_manager.h
 * Author: jeffh
 * 
 * Not technically a UI widget that displays anything, but it is implemented in the UI
 *
 * Created on August 4, 2012, 10:17 AM
 */

#ifndef LMS_SESSION_MANAGER_H
#define	LMS_SESSION_MANAGER_H

#include <QString>
#include <QFile>
#include <QDir>

#define LMS_SESSION_NOTIFICATION_DIRECTORY QString("/notify/")
#define LMS_SESSION_NOTIFY_OPEN QString(".open")
#define LMS_SESSION_NOTIFY_SAVE QString(".save")
#define LMS_SESSION_NOTIFY_QUIT QString(".quit")

#define LMS_SESSION_INSTRUMENT_FILE QString(".lis")

/* These are called from the UI on the event handler of a timer.  They poll the project directory for files created by the session manager
 * notifying when to perform certain actions
*/
class lms_session_manager
{
public:
    
    static bool is_saving(QString a_project_directory, QString a_instance_name)
    {        
        QString f_file_name = a_project_directory + LMS_SESSION_NOTIFICATION_DIRECTORY + a_instance_name + LMS_SESSION_NOTIFY_SAVE;
        
        return check_notification_file_exists(f_file_name);
    }
    
    static bool is_opening(QString a_project_directory, QString a_instance_name)
    {
        QString f_file_name = a_project_directory + LMS_SESSION_NOTIFICATION_DIRECTORY + a_instance_name + LMS_SESSION_NOTIFY_OPEN;
        
        return check_notification_file_exists(f_file_name);
    }        

    static bool is_quitting(QString a_project_directory, QString a_instance_name)
    {
        QString f_file_name = a_project_directory + LMS_SESSION_NOTIFICATION_DIRECTORY + a_instance_name + LMS_SESSION_NOTIFY_QUIT;
        
        return check_notification_file_exists(f_file_name);
    } 
    
private:
    
    static bool check_notification_file_exists(QString a_file_name)
    {
        if(QFile::exists(a_file_name))        
        {
            QFile::remove(a_file_name);
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
};

#endif	/* LMS_SESSION_MANAGER_H */

