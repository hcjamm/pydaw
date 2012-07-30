/*
 * File:   main.cpp
 * Author: jeffh
 *
 * Created on July 29, 2012, 9:59 PM
 */

#include <QApplication>
#include <QSplashScreen>


int main(int argc, char *argv[]) {
    // initialize resources, if needed
    // Q_INIT_RESOURCE(resfile);

    QApplication app(argc, argv);

    QPixmap pixmap(argv[1]);
    QSplashScreen splash(pixmap);    
    splash.show();    
    splash.repaint();
    app.processEvents();    
    sleep(6);

    return 0;
}
