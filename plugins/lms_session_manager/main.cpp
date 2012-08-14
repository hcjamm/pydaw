/*
 * File:   main.cpp
 * Author: jeffh
 * 
 * A session manager UI for saving connections to/from the LMS Suite and Protractor
 *
 * Created on July 29, 2012, 10:30 PM
 */

#include <QApplication>
#include <QWidget>
#include "main_form.h"

int main(int argc, char *argv[]) {
    // initialize resources, if needed
    // Q_INIT_RESOURCE(resfile);

    QApplication * app = new QApplication(argc, argv);
    
    first_window * form = new first_window(app);
    
    form->show();
    
    return app->exec();
}
