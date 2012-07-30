/*
 * File:   main.cpp
 * Author: jeffh
 *
 * Created on July 29, 2012, 9:59 PM
 */

#include <QApplication>
#include <QSplashScreen>

/* Example invocation:
 * 
 * ./lms_splashscreen "/usr/share/pixmap/euphoria_splash.png" 6
 */

int main(int argc, char *argv[]) {
    // initialize resources, if needed
    // Q_INIT_RESOURCE(resfile);

    QApplication app(argc, argv);

    QPixmap pixmap(argv[1]);
    QSplashScreen splash(pixmap);    
    splash.show();    
    splash.repaint();
    app.processEvents();    
    sleep(QString::fromLocal8Bit(argv[2]).toInt());

    return 0;
}
