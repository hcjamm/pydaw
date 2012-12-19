/* -*- c-basic-offset: 4 -*-  vi:set ts=8 sts=4 sw=4: */

/* synth_qt_gui.cpp

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

*/

#include "synth_qt_gui.h"

#include <QApplication>
#include <QPushButton>
#include <QTimer>
#include <iostream>
#include <unistd.h>

#include <QGroupBox>
#include <QPixmap>

#include <stdlib.h>

#include "synth.h"
#include "meta.h"

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/SM/SMlib.h>
#include <stdbool.h>

static int handle_x11_error(Display *dpy, XErrorEvent *err)
{
    char errstr[256];
    XGetErrorText(dpy, err->error_code, errstr, 256);
    if (err->error_code != BadWindow) {
	std::cerr << "synth_qt_gui: X Error: "
		  << errstr << " " << err->error_code
		  << "\nin major opcode:  " << err->request_code << std::endl;
    }
    return 0;
}
#endif

using std::endl;

lo_server osc_server = 0;

static QTextStream rayv_cerr(stderr);


rayv_gui::rayv_gui(const char * host, const char * port,
		   QByteArray controlPath, QByteArray midiPath, QByteArray programPath,
		   QByteArray exitingPath, QWidget *w) :
    QFrame(w),
    m_controlPath(controlPath),
    m_midiPath(midiPath),
    m_programPath(programPath),
    m_exitingPath(exitingPath),
    m_suppressHostUpdate(true),
    m_hostRequestedQuit(false),
    m_ready(false)
{    
    m_host = lo_address_new(host, port);
        
    /*Set the CSS style that will "cascade" on the other controls.  Other control's styles can be overridden by running their own setStyleSheet method*/
    this->setStyleSheet("QMessageBox{color:white;background-color:black;}  QDial{background-color:rgb(152, 152, 152);} QTabBar::tab:selected { color:black;background-color:#BBBBBB;} QTableView QTableCornerButton::section {background: black; border: 2px outset white;} QComboBox{color:white; background-color:black;} QTabBar::tab {background-color:black;  border: 2px solid white;  border-bottom-color: #333333; border-top-left-radius: 4px;  border-top-right-radius: 4px;  min-width: 8ex;  padding: 2px; color:white;} QHeaderView::section {background: black; color: white;border:2px solid white;} QPushButton {background-color: black; border-style: outset; border-width: 2px; border-radius: 10px;border-color: white;font: bold 14px; min-width: 60px; padding: 6px; color:white;}  QAbstractItemView {outline: none;} QLabel{color:black;background-color:white;border:solid 2px white;border-radius:2px;} QFrame{background-color:qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0.273, stop:0 rgba(90, 90, 90, 255), stop:1 rgba(60, 60, 60, 255))} QGroupBox {background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #111111, stop: 1 #222222); border: 2px solid white;  border-radius: 10px;  margin-top: 1ex;} QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 3px; color:black; background-color: white; border solid 2px white; border-radius:3px;}");
    
    QStringList f_osc_types = QStringList() 
            << "Off"
            //Saw-like waves
            << "Plain Saw" << "SuperbSaw" << "Viral Saw" << "Soft Saw" << "Mid Saw" << "Lush Saw"
            //Square-like waves
            << "Evil Square" << "Punchy Square" << "Soft Square" 
            //Glitchy and distorted waves
            << "Pink Glitch" << "White Glitch" << "Acid" << "Screetch"
            //Sine and triangle-like waves
            << "Thick Bass" << "Rattler" << "Deep Saw";
        
    QStringList f_lfo_types = QStringList() << "Off" << "Sine" << "Triangle";
        
    QString f_default_presets = QString("empty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\nempty\n");
        
    LMS_style_info * f_info = new LMS_style_info(51);
    f_info->LMS_set_label_style("QLabel{background-color: white; border: 1px solid black;  border-radius: 6px;}", 60);
    f_info->LMS_set_value_style(QString("color : white; background-color: rgba(0,0,0,0);"), 64);
        
    m_tab_widget = new QTabWidget(this);
    
    m_window_layout = new QVBoxLayout();
    this->setLayout(m_window_layout);
    m_window_layout->addWidget(m_tab_widget);
    
    m_osc_tab = new QWidget();
    m_tab_widget->addTab(m_osc_tab, QString("Oscillators"));
    m_poly_fx_tab = new QWidget();    
    m_tab_widget->addTab(m_poly_fx_tab, QString("PolyFX"));

    m_oscillator_layout = new LMS_main_layout(m_osc_tab);
    
    m_program = new LMS_preset_manager(QString(WAYV_PLUGIN_NAME), f_default_presets, -1, f_info, this);
        
    connect(m_program->m_program, SIGNAL(currentIndexChanged(int)), this, SLOT(programChanged(int)));
    connect(m_program->m_prog_save, SIGNAL(pressed()), this, SLOT(programSaved()));
    
    m_oscillator_layout->lms_add_widget(m_program->lms_group_box);
    
    QLabel * f_logo_label = new QLabel("", this);    
    f_logo_label->setTextFormat(Qt::RichText);
        
    QString f_logo_text = "<html><img src=\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAKAAAABOCAYAAACngR3fAAAABmJLR0QA/wD/AP+gvaeTAAAACXBIWXMAAAsTAAALEwEAmpwYAAAAB3RJTUUH3AwOACoI7beo3AAAABl0RVh0Q29tbWVudABDcmVhdGVkIHdpdGggR0lNUFeBDhcAACAASURBVHja7LxJrCVZet/3i3mOO9/37ptzrnliN7tbpNVkUxwFWwtRoGUasLiQAS208sYwrJUNA4a3BgzDWhiwZUCcZbVEk81md5HsrqGrq2vKoTLzZb75vTvfuDHPXmRldlZW1tSkDdKuD3h4ERcRJ8754n++4X++E/CFfCFfyBfyhXyCVFX1hRK+kM8sdV1/5muFT2tIEISHz58AXgJW67qWPu3+L+T/H/IBRnzgVl3X3xFFsbxvuARB+BCGPhMA7yNYEATqurbquv4fgH/2wfkXGv9CPk2uCoLwLwRB+IPHGbJPBGBVVYiieP/4vwH+64ex+YXV+0I+zQPfx4ggCLO6rp8VRfHkYVx9LAAfRmpd1wd1XW9+0CBfAO8L+Zxuua7r+j4Qf1MQhP/jcZbw41zwFGh/YfG+kL96PlIj3EPdY0EoPhr31XX97bqu2/W9H4THZTafJQ78vNd/3uzqJ2nz/2vx6+cdz2fV319Vtw8dCx+Arwb+VV3Xa4/mEcLDrreu678PfPNR8D2UkHzmTj6M8r+O5OWh0OAn6sNfVz/+pmWff93v5HF6/knafuSe+gMwTgRB6D0cD34IgFVVeYIguB9C6AcdffS3zzsz7qfjPwkAHr7v4fb+3+zD30Tg/VV08Th9/HXq+TEgrz+whv9QEITf/0gMWNf1V4FXHrZ+Dw/0cZ173Mx43Ey4f+/DA/68A/skJX2ePvxtB+L/U+/ks+j5k9r+LHr+4PiqKIrP3H+2/FBb//RhUD5uoA93sKqqe+dV9VCaLCCIwoMHPzy4z+suHpedP9yHB8//UL8E7uv6/rNFUfxIHx6n5L+t4HucTn6sXwGEGlEQP/GdPArmT9I199v/MVAe6FgUxY8YmkfaqwVBePreP6EGPgTAn3tcBx4+rqqKsiwpypKyKCiK8sE5Qo0sikiSjCSJyLKCLCuIooAkSZ9oqj+Lq3gY+FVZUVQFeV5QFiVFWVCVJXVdAgKKoiJJEpIsocgKkiQ9UNJPaoX/poKvLEuqqiLPc4qioCxL8jyjFmqEGiTp3vhlWUKSZGRZfqCPR+O1++3eB9yD910U5MU9HedF+eBaSRQetKcoP9bzw7p+BFP3Ff5LwB9/CIB1XZ9/ZNntIwMtioIoTojjhCSKCeOEJC8paxCFAqkOUBQNQ7cxbQtTMzFM6166/UGnHuEaPzEQvi/316KrqqIoCpI0JY4ToighilPCvIAqQyx9qGtEUcVyLEzNxjAMdMNE0zRkWf5Ypf9tA999cGRZRhSnRHFEEnvEkU+alQiSgFDWSKKMpmvouoNpqui6iW5aKB8A53FJQ13XFEVBlmZEUUIYxsRpSpJl5GVBXVVINQiShKqpGKaBbWiYhoauGx/S8+PCL+CJjwDwcS/kYfBleYbvB3jzkNlyznCZMg8y0jKlyk/Qy0MUWYPCxbU7dPs92m6HRrOL6zqomoYkSY91549LcB5dQ7w/y4MgJgh9xouAqReyjCLi+ASSu9RCQbxUMPUmbsemY3RodzdptlycRgtD15Fl+UPj/LS1ysf17bMA9pOyzM8K+Mdd+zD44iTB80MW3pTl4gTPHzI6m5GlBegJWlSiyBs4XQXTWsNu6LSaHdw8x3VcFEV5AJaH2y6KgjiO8eYhvucxmgV4QUSaBoQLlcKISEQNRVZomAIdq6LbMGk3ezhOE9t2UFX1Q+HPIyBs3H+m/IhyP5KA3H/xSy9kPPQ4Pb3D6WTKaZ2iq9dZFh7EAstZhnd2ii2t0O86rI/W6a9cYG3zPFW5SrPdRFEURFH8kImvqpKq+mB2IyBKAqL4YVN+fxJEUcx8NmE4GnG4mBIlt5ilI7IkYTGLmJ/cRa2aNO2aruEwsjYYbD9Jv3+RwUaJ0O5gGOYDi/xJVvgnyfg/jdL4Sdp6nHWqqookTVku54ymZ5wOjzk8PeBw/yqxn6LLY1pEZAEYjXNcf/0cWy/cpdtZI+ltUPRXqauKRqN5z5V+YAnv6zmOYsYTj9FwxsHxnJOzhKUnIExNwvqPkDsO4XqXDeMq87HKeNSmZQ4Y9Dy2Ns59EA/eA/hjClo+lPzKj4730USjLEviOMGbLxmeLLj2/veZTY/gpZfY/eb3UPOSdHZK20y41KwoRIFoYXBz/wrj/pRKKqAWECUJyzYRBIGyKEiSjLzIKMvi3nOKe6AQBRFFkZFlBVVX0TQFQYA0zZhPA46Odtk/2OVMgL3vfws58clnR7StlMuNCkEuKaYl86jBUbxNKs5IwwJBlJFlHVlWPhILfhrwPi7b/jyUxMOhxE9KmTxsocIoZHh2xu6t69x893WC0Vu01RP6sk8R5ZSRTnyosfmbdxGPX+KHf/AN2uc8BpcSzl/OKXIRWZIRXfeBLsqyJE1TFnOf8emM/dszjg9LxPE6W/oWZdXhNe8/oaU/g7P261z91z9ESUuyIGJt+zL5019CqUsUSUBVtY/Ego9mzI8D4EcUUJQF0TJiPlxwdOgxOhsiXX4JbfS7fOPcP2YcRZTdlGC6z+7uVZbhMZ2ORBVfY3kcojkquq6iaNoHYIMoTIjiJVEUEAU5YVyiiRq1ICAgYesyjqtiOzaGYyIrElmaM53OOT454SjIKPLv8I9e+M8YhwmBt+Ds8BrXb7yNF57SaSvoVYa4eIeTZoTyJRHt1MCyNDT9x6HAx1ZoPBI8/1Ut4F+FCH9cSJTlOf5ygT/3iZc1O80XsZU13vjRt9kd7qIYAmWSIoc+5csuL/5Hr+P/6wnHr11iOb9AUdmouoZhaiiq+iCDzfMcfxkyHc2Z3johezPguWsDrJnKjc0Rd9bukiTnOKuexvru/8zPrP8GsuWiI3J89C6779wBbGy7g2W5aJr2WA/z8Ln8cYO+5x4rirwg9kP80wWjo4zGYs7h/gGrZ5f41tlfsjcd4WPSXCpsrV6gbTQ5O7iF3Xap7t5ir9HFabZQxBZpmhH6EdNJTLCoCZYlcVxSZjVhsiAHLEXA0hVWnZruoIG70sRoGWRpzvDUY3FwwCQM+JL2HO8c3uD66Ji7Xkm9N8eyu+RJzvs3h6yu2jh5RnntkLuag3zlHJ3eGk7DQdPUj61Ve/i3h2fw4xT4Sdbz4Vn/cdzap93/uGdWVUUSx0R+xHQ0ZjIcIgz+DuXBFmubKntn/5YfvPsOtSqiZAndoGC61GhWIYax4L3X/5hC/Xu4joVtB5iWg6oqVJVImqb4fsDwbMFwd8b2XZtzYZPr+W2O3r/GD4Yzlv4R/u2QdUFj0fk+Kxub6Oe/xnrys7jaDcLJkul4Qrvbw3bcj8SYj45L/jjO7f5xmmTEaUY4XhCemmTleb7sLfmzacaNgyM0U2O9qaK0JSaJRxGMmccxcq7SnelEb+3SXDmHZazhLXyGJyNu3DpjeWcJuU5d64SnPpprobef5NBucjGIkZ0zipWQ4VMx3RUboS6YnCacDGN+3pW4mTR5//iEazdPEISQ7noD1bSonR7BUcitKOe8Z+KNQrR8jK0dMdi6SKPVBEFEVRVkWfpgkpU/5gxFEUWRkGUZURQ/oBnEDzjGD9xgXVN/ELs+zL3dj1vvW9hH6af715ZVSVVWDyb5PX4NREl80MbDfx9xwXlOGMXMvZTUX2H10CeY3uJqfJU70YzO5SdodXuIZYU3GnLr7RF9YYTm6Mimwg/ffQWnkdLqqjQbJoZxjyGIwpjpzGN2OiU7SWnPzvHNCxKTu6e8fetlPLOiv/MsFy2DOCuYBinCSOZp30cqUyo7ZTKe0F9skWb3wqv7E+3hUqxPtYAfcsFFSR6nhHlF5NWM2zFmlpPfbTDo9jn/9A4d3UVVas78OUfHKoeJj7CvkAcOWVjg7A5xusfINbx/eMrhDwP6gzW8nctMax3NXSK88RYD54Ro3sBeCnTnOwT+jANlSj5dIHUUZqOaYeoxpEnx3irh/jXajsCFF55nxXUJ05CkWqc5WOXtl9/mxCvIM406jXAvjJmPJ1iNBlEYIwgCRQ1VfS8iFqoSSZIRJQVNltA0FV3TUAwNTVWQVQUBKKuSPMso8pI8L6iqe7yYJAlUdY0kS6iqhqEbqJqKLMsfSejSJCVNU/KypMhL6rpCoAZBRJEVVEVB01Q0XUX+gF/7MDFcUkQ19azE3gtpNxJm3T3uvv5DOtvbXHnySeq0pkgy6u3znJ6csv/y91iPp8w7A8pZwcybM5ovGAQhdhSjayphGDKfBcyPpygTjVFbZr++w2LvBxQXVL7ypW0MS8XVdYpMZbYcUtxO2WqGKM0Jr73zHdJWl81nnyNPC+qKT6XZ5E9b+qqpyBHwK4locIB3+9+h6/8la9Kf4PUtBo5FQ1lybW+PIjHod1e5ox0SBKegtBD1AVEAo9GCbHrMyZ0EQVrjpPkCoyggG7+PFy9omksmV19G7484qXoUeosriyavxWdwaGCXJYu6xB+/gRT9C8zw28jaLhtPXKJvlgxP30OsmqSCiOs2UMWSVFaQ3R3k2idOREZnp2SlgCAqVLJDVEv3iOy6Rg1jlCpHdVzaioJhq7iuidKxsHUL3VQRJIEsjUjDkiwIyNKIskhJq4QgTlBlE92ysBsNmg0Xx3GxbAtFVairmiSOCcKQpRcQRCnLKCNOMrKiRM5yVBF026Llmjiuje1YWK71gMP8sfsWEaocQZhh9e5yWt/mt//8VZbLJs881SZLcuLxgjwCQXQZrG4QXX6BxftvYpjrqM02Z2mTo2nEthfQakbUZYm/9JlO5oSjFCdxKaszjjvvMO/K7DyzhtO8SBK8Qk/5RUQmlI5F69ycyvgu//Lbf8FsZnPhxR5lWVHXFTWPJ/s/Uwz48GJxreRk1hnT8W/zlcZ/xeb0kMJVSVUBw4LJcIQqtVmM91CqmkF3hbsHZ5jiRZSiZD4LmU8WLO/OyM4WVJ119uf7pLe/T3oyROxpRGpNXi1ZThJuij0W/XNszzboLBSOnKs4Mx3Pu8uXrX/OYH6bd6UlRkfCbdgk8wOEWCUXUkI/xjYMVFlEUK4gFznL2qfMBA7vnHC8N6QSV1j2n0GJYrLbKeGKiqkWNE6OEVslG1KJ0BJYV3XKCwZmR0HXNCSxok406sWSLPCpWVBpFXsHe8zHCbJhoJkurZVVBusDBmubdHsdNF2jqmqCpcdwOGI88jiJZMJ5RHScEugCqpxj+x5mr8uqq9FaazAYdGhkLZqdBoZuPHBjkiCgaDKKJeI1C+68e4q2NJHikDQMKHSLOAzo9K8QHCZ4BzErl9e5tXuIlHmozkVGdY/56YLpyYxep0lZFiwXPrNxQDrPqdSS6GiCJA+Rz5+iu5t48+9ysfmLGMkMpAyRkp0nLN55/QxhrlNXBbqmoysakiIhCuKnJmfyp2Vwkiih1DIOFk/zKzwzmlGvLFHRkeuIqIypjAox1ciiJYkO7f4Ke4qCJjXw52PkZMxyaZGcHeDHLSpVIzh6l7Y45eKXXUTHQokVasklKwqymcc4OeGGZfDcj0x+sNYlHozpJ0/wU35A3hrj+xnINrpRoygyYm4TDw9IxAKz6lJUYKkui9kx5kpAGiyZnN5mmXawnvtZ/PaXsN++i7kIGfYHXHh3RCKmxNXTPBmPOJ0NmYkpzTshJxcU5KaALsgYUYaaLdB0hW43oOgGnM1G3PiXf47gqlTrPRqDLTbWr3Dxcsa5ixntXpM8zTnZ3+Pu7hHHM5WTeo3qqoQyNdh/coXNO2Pm8oKsoVBoI063RsTnfLYu5ICA2BHRdO3eS1NkZF1BsSwCsUJdb1N8Zw9FLphOjuiv9Vl6Pq0eZHVBOrxLe3Ub3S1ZRBKN1jaUFpODEfOtOQvPpy51Foslk1mKHqkELLlmTuhc81F/pkOvWKMXvEDRmKKWIZ4GaZzi9gSMC6uE/+YaQsvG6Tg4rokiq0iy9IkJ1WMB+GjlhKRIqIqMLchoqsfN1YhuR6ROEmopZTpR2Nx5mtHZ+7jWGvnEQ3ArVLumTGUkxSJe7BMuxiSxgmBt4gkSupiytuHSSL5Ec/cKtRww7ucY0oKi/X2KuOakHiJWDs7QJxQDBK3keF2kUGL8YE5h2Yh5C6fXY748RPAlmrpGHoZIckCW1wiqSp2XSLduElY2RXeNiWyTHVylmh6RJx7db7/DXF4Qrlg0w7skUUjUUlhmAe5Mx1wERI0lgQuZDc8PLBQ7JdnwOT5+nsH588Txv2VFvoR0fcnoh99k9KURkW+SZwXr2x2yKGL3+m1uvz/n1PklYuUKzdEd5HDE5ncPKOUx8w2XVrRO+6Tian5CEt0EoUTWlXuJgiJ/8E5kdENDaxik+RqtzRaob9I3zpMdLkkvJSAnDA8iVs/b3Lq2wN2wMXd28HcVssYGQi4RLmomZ3O80QwKi/HUJ5gXrEYG01bCoXCXwFowMNcYnNQU1iGhJqBnEbOyxrIC/PiX6KxvEPMndLrbrDb7OKaNbmgfSsIeTkQ+Mw94PzMUNZm8qTKyauq9Asms0AwNqciZe3PWojYkAhvP/DK5p+CZ79MWX2Uqx7Qsi0TfIYhzTOtZGmsOJ7Ml3dUciycwva8zbC6IxJBGfY5L0wknzhpn2hCv7JMaITPBJxdjFkVJc96jNAKyskaQNPxRhG21MLQOnUt/l9A8JoyOac8m7FsejUaFpjyFl0NpCwimx8Fb/yeJGmJPItIoolAzKlnAOGpTVe/xsqMhKA1OjAYHtoDQ1ig1DdWb8mze51Cdk3gBe2cBb/77P2LjBRt566u0d/5jXLfFJfGMP/m3/wXv5JfQlJqSlGgZsn9zBpFJmMjErbdQy32OvEMCfU4tCTS8HXaKiJOmSBXL7O5NEJ099L6N0bLRDB1NU5ElCc0wsJw2lHuEM4FzT/wKO1v/mEl0SCzdpqHKHE+XbGsm6uBZFtOQxpZNndRMvQxpxUCYOoyHAePZgoKc2TTHHuXEqkFaBJxkCfIqoGu8QkDbrnDHMrM6YiqJPPHEFu9864TGcxXt9WfZvPIVBttXaHb6GLr+oaW4j6Ni5E/iaO5TCaqu4jQUtIHBDS+jnwYstIiy0aJalOyfvoZaP8+0vs4yOqPjXsLtDDg9mSA0NmiUBqJkcu78VxmP79BPb6IgINrPIGQ5wt7bVOqEhjUmMxIiuUElHJMjkq6A3ZHIlyFpqfCe3WDj5CZnZczAcpAnNUP/XQzxed4c/i6r2x1We88xa61xMxpS9lQ2s4vkqxW3g12me3sYDYl2Y43Wcwam1kERRGpRoEhyYt/nOByTe6fIMwdnx2Rrpc3WpYLZpMtfHkuc39/nXX+Ps7hgejbn5qshUrjFe2/9t6ytNPm1n/3ntJprXD3cp9/R0WyBIhYgW8FYWKwVf8prZxInk1epUg8lKRAVlXhxh32nw9nqeUy7y3IocWdywvpylbVkhTxvIMvSPapIVbGdJq2Wy9Xd11C1r/GXp/8TDVuknT2J3uqSjRaEUYfBuk0RtJDMiFgo2Iluspd8lUzVWAYxy7GHQEk1iRCzgKOuipemhHnK1qqN3JRpN6YocY/9icIgq4jLGHv7Bap4j3d3v41k7iA7EW5LxXGdj6yEfKoLflwljCDcK7fRNRVTc7B1FVXICKyCRVfCEu6VQtVnzxKXErogcn7lefx8Cs020plOoahgNZEdleXmFSaKj3x0cC+gN7cZFbcYN/YpqjvUQobZ2mRp1ihLgbm9ZG214pJi885QQ1IK5CpiJsfEhoKmapRBSnm6zVL3qKhRFZdoepthKaLrBoqiELp9TrMT0HJ+/uf/A85vbyHJGlWtUeQFZVZT5SVZnpKRktcBobfk6GzOMj9EOE24M9WozYDFySlReMSuv2Bn9RzP/doL6KpJVqWMxgMqH27f/FNGtYSEy9nIx+nOqIUOefMZYrfEe+sa6eQ91rZ1zu1cwbFt6qpmNplxfJog2im2FOK7OXI6I14sCfyQsBE9IHYl6R5V5DYdjOR5zvIJRVXwzNY3OB7eIjMUTMtkfFqwfdlhNFpHE4ewPaU4MHGDIXVjAy8fMVl6FELOcJniNXt4bsjsaETWKGi0mqxYOuNphmYJVEVEogmUUolkx0TeksntS8i9iH5/hV5vFd24t8LyuAqoR+UTq2Huu2BZVtENHUsxaRsRan/Bc0nFkWWSJDnR+WP0m88i1Eteu/ZDFFXB6qxhqRZqKiM3FBRTxUEnSj10R0apG+w1t2n7HnJTQ0grKien6mhIKiA0EPMhmWLD4iKucwdvlDBoHDB25qwsZHRFw99coG2X8KPLSMWYIPJ5+50jYvdrGHmKUVjIZguxvsOLT17m0uWL6FIDRIPSstHykrwMicMYz48p8opMdVGcHucbU1LPZHGakcpHJEsBiYJC9XhydZ1Lz76AoogkWo67exGhpTKTT3nt9Ztozk9jZytk0zmLRYhq9ehZA6b+O3hVRrer8+TTl1CVKyhyjST46IZFIU+IOjaSKVANl4RpQLjM8YMFS8+hLAskUSBLM6jBaOloFxO0E5tyss4fvfKnhH7C5Zcu0Gp2ySYStVijWQoyAwztmMhUIZUYra/TORsxXZ5SkrGoJPK2CtqUWJ7TbEvojgxeB8uISP0lTSkkEkReaBQUy4jZ1oLJ2lXOBT9FvzvAtEx0TX/Af36ayJ9Uan0fgKqqYOo6na5Na0PGkEJKrUJatgm7+3S1LQqrwqZN23iaOKkxZZ/tdR1VMTk7mGO0O4TTG4h+iNUMyNWn6U7vMFcE9PbX6K/8FkK9R6nkKAWIZoIcHjMJJBbtm3S8Lu75Of2nI2bf85G1DdTSJNo8plmu4YsiXXeHPD/CrX6ZRlKStN5gtTVgebhE7Tk0XA1ddWk3HGTTpa5FwlyExELSMoxGSidfMo98FqVJ5kpIpsCaHjKfZ6T6BM1oo1QS/U6XbrPH0p1hKDX5oYo2vUCrkbPWcxEmA7rbEJUl3mhMY1UlzfZRoogiOmPz2T6SfIFAfQpbCsjIMKV9rKaKvvoSYn1KHB4QTcHLJiymHWRZQPM1xFqgKiuStKKUKpyWzaiZ0A2exI/XUBWB+GjBxtMlueyQjAs0NyVflBi2ScKI2DjHIE2ZGWtMA49KyKlbX8bs7RBPXkaqagZNi0C18ZQjrLpBc31GEMP0MKO54RMd2ZytHaEfyDQHDVZX1nHdJqZpfmS9/ePW1+VPW4B/EAcaGrpswazDYi2k3Vjl9G6AcDyntrYoN4akfp/+6oDpzQW6EzJrZ+SxRfcJFW8R4Nl9iniOudMizU282VXMekqn86vo5jME1RrL498no4MiFqyoS1IZ3sorNt0x1m2Nw0OXue3QcHsEUoByPUB+UiZZO0abOZThOoOiT/bTVzkciqSawtpLBsE1nTiRoE6JcwVhWVJRU9VQpBVBZhJkNqmwQ13NaGTvMUld0naAKohsNAz8ExNZbeGsg6nbiIpMeW2KfF6m7J0hLNbQ8qfprvwItn0KuUU5sZHkCekiI1ntEI7fIbM7uI5OmvVQijGW47EoLlJwBOoVsvKXKf1XkbIRkjnnLIzQp3cI503qQsMYVFCqLL2CcD4ESqSVAD3u4qZfJzh7h3JlQU5M7bQRdR3dGBP5F5HTGXVrjh4UyEVIszIoI4FIFtFWvkJWdlmMblPFLew1SF2dsyDmgpkRH5bMAwNnI0bY+ArvfuuA/LUTnK11etsOvX4H27JRNfWxCcjnXoq7v74pyzKKomG1dPSGzO57GpP2KX7zDGupUmY1cq4QJTYr2z5zz0PWDEwKgpnM0o2oS/DFPex1mU7fIc4zFP+Iuig4Wn6faPEKvfUIXTugKUrE2lPI0jrOIqDolRzlOi9tLVkeVozuyKw8Z5FIJ9idBnphIIU5uRRi6Fcon9gjEgTKiUztyoRmhW1u4mcLxouQNXOCazUQFRVFgKIWGYgJfpxyOH4fv+ywkJ7D8X5A7G3iG3fp1Cbnnq2IJgqm0WcRHmO7MUrfRIoFFKHAEwVENIxOwSwqUQ9NTMFhpuyjCBK0puQrNd0yRTdaCI0Kq95jWXRZVDm2MMbsiPT6b3Ck9MjzFzGjiMCq2cVntTFmcbvB3XmBaEZE5RI7V5CkFkIuEYkhWjtBNocUhku88GgYEESg6xVqK6QS+jSNM/TmBNXVkPKc/VOHvIh56sqCbPodNPEt9KaAaK1TR2366iEnUsDOekB12yJb2rz1+gEn4vsIlkrbWmXQW8FpuBiWjiR9uBD1cZ/k+FQAPhw4Sh+UXsuGi7Fq05qUiGqTS+qA6ZUp8+OSrJAQMxEEsFpAqdBeEZjP5kh1TS3HBEdHXHqxjW6u4ZVrdPSU08jGjq5ikVIvI3BF+kafWJ5TKiv045jx0qPQl9yoHKTOlPXSwbZtmopNPhBR05JkrqA7AWLdQevscnbmUwkpvnCC4m9hGCbCosNZXyIed5GSDqJkIAgZzXpJJWkMWiO22jI3T64i1udYSG3MfMnEqAmqAEXtIe4s0aIKW3cQ4xLLbKMsY4YjEVkLyKMGqq0iLVMEuUZ0QI0bqLJKvHebqDqmr0sU1hahsoNeHSEIDmRDBCEjlm4j+HMa5TbT7gU69q9TD14ha93mveY621shz7ynMqkcdqyLCJs5xonD3rspVXFIlB1jNHOqWiddWNCdo3hrxDGY6gTPbmBUOpNCRdQvIVgl/fp9Tschlf42E//PSIJjzrltYmcD8hoNkRM1Yi9pIaohlwcWJ5HMdrBN2VZZaa7QbvVwXRtN0xDFjxLQH1f5I3/SDrQHLlgUURUV0zJROi7JisLgOOGgnxFcq0Cuaa36jO5k1IWA2arJMwG3ZaOpY+pKRGwUuF2Tnc0ekTCgU4vYaoPZ1GbNWGGt12CejjjxGqiuSlXp5LJJy3BwFiMK20avFIa5xtq5AYYgMa7nmMsOdVmzsp4ymdVIgYq1uSQ/9CiUiDQ5ZRn0kDYFog0JJYrwPYG1fIbmqjiGiq4IVKJMUC0xRYuG3uNoIVKqbZrRDSI9mb1K4QAAIABJREFUJlcNpKxmck6n3o1w5gUTcYqRNKiCAkuXSWSPYGzg9CUMK2U6DdGnCnXukG2lhAMBDpZYho2grxAyQxAl/KLGlAJ0A/xRj71QRJIXqJ1Txtk1wuQOa5rFiXjI3Qp6zNl4R+ePWwXqX7hsr6/w9JNNXn35BUbht7i4UWMVDbyhSpEvqbKCQFUYbGTIdUleXsSN99mrPGzVRZQ1lIZBVrzLZK9PlKUYTotY1lDrG8Rqir1qI7+fEX7J5HSjxP79mDfmR0iBQbvXo99vYZoGqqp8ovv9zBbw4UREEEU0RcVQDBqGjWqbnAgztv7DhLMXS87+LwWnzLD6U7KojWqLIOW4jTY7PxWSEqKaBUVk0mn1iX2blj2mzHu0lIyL2ztsrriMQgtveYc4arAmw63qCElxecreZBx5qKmCaSlsuNtUUUjj2Qjhbpfx2EMOVIq6RIi7pN4SofYwbVCtHPvpm+x8JebO9yA6XMVugazplNU2USSTaiJN5xikkqxO0JQSxdzD1+ZI8wWDyw2quyKhl6IZNaGdUwgFzlfmxK/LBFJOPc/QezqSICAWFbal4LcO0bZ1JHXG1gs3uPlqgZA6OLaD4CQ05im1ZBLkcLk1BktDiG6hLzJOxx7h3TFp5WEZCpNXDMqkQnBTbiUFyh7YX8oQDJ1r729wUbuAs75JfHuH6fA1Njd6LHo5mZjhPHlK4+kF4u4AOXbI5C62eQNjeYgSrRHpK7jN73CS73Jc7XOxsY5g94iU19iuVZLVGiPSSacJ7X/YIE4yTpdzlItL2skK7X4H13EwjB8X+36S+/1ca8EAoiAiKzKGrmIrFlpqof70IcFJSAeB44FDvF9imwW+l7DW0kmTAlWUuPRMgzBqUVQ5zYZOtVQpgpqmqDFr+rStiM3BBi3LRdE0QkpOGqfooU7LUSlTge10G8MYoQmQ1B1aSptkY8ZiUmAJUG0UxEMVW3aI8mOywwJBENHEiq/8swp57RJLf8jGr17lxu9JnE1XqXWR9updZFvH1AV0zcAUW9RVxryKUC8EdGyd9ATUGwNKa58kjSmXFlUUo6+H5NWYXK4RrT7eLQVRE1Bdj9oTMTo6jnpG7+d+xPD6DsORhLazy8rySWSzSRIZrJjXiTomTnCbvrZGJtg81zMYqWdIhwFV0EOXN9DQkTUBQYKqgtoEsXevrCtZBswrjxs3Rpy/2EAXV5iNfTYGEusNH/Gr+xTXniIVHJzLC9zrFqOih6r26NU/wlvbRVeWyElCcaNNN5ywudqk2ixpaCLGXCM4EtBsn7KZUcsKy/c86ueGJG9XrF7s0VvtYlr2x1abf6aK6Mf56AeEtCggSzKGouNqJv2eyVuvtHEvDUhqnfbeDarekMaTxxSVDZMXqcMJlezRFJ/EaRRkqYItCYxXbqLPNCSlpBj1aNlLHN2lFFUMHZpqB9/TkYWMztShLFKsVZsNVcepC3JRAEHk8LqFoGzjlyrmNCPtx6z/3Ftk5Zu8/r+uUh6XXPh6RZ5+g+Rqzcu/3SSTfVpaxLnmJjt9i35rBds2kVUdJJGsyFmWCbneZGA9w+zqEel8hvvERY5Hd1GnPhthTX2p5Nr/BuvSk4yCgPY4Z/XXX0UuJOyDLzEcGZipyOCKzdu/t4VpqoyPbNRunxe3OyA4RFmEKpv4pytYmYq8qVJqFi0zQ1EcNprbdCyboqopgFxM0bOQUoRCFCl8BUGCPM+YD6ecXP422994ld70Cq//7y7xmcfOL815a3eTdpEw/cEViqcPaFkhRhVSVU26for3johfd1iIHkm8YNVYodnsUwqriKc+tQCKYpLuT3lhI0UZL9l3PF79HQ3H2aLpbtK229gNHUVRP9O+60+lYR5Hy0iShKSpWKZDJ9J5oo7x/rBN5+IE7bfe5PgPvoZ3NKBaP8V2AooLCuG4xLgooyouZVVRiwlZESOJArVQ0+8d4xgt0hyEuiYva3RJRHc1wlGKlsaEmkAmZKyYPQRTRMoqotM9ngqnnAib5Jzh/MweZ9+/wGi+jVeOKC9cJ477tCdXqBs3+MEfigwPfsATOwOeWr+I09xANJuIZs20FqkjnUrISAWZSFCQN1oE10cMf3gduS2g6qfURw4TaclGqvDCv99jHIL1r7YZbJyQPnmMePcCY3NJAwHl0hnpeJ1+scHOTsz8VKPdDOn0N2jbfSaFiVl7OE5GIsfokUJRSWSSQ1xO0FsuTlZi6DqVJCBIAlUukNsxRpSh+Sl1Q8NPVFJ5gbLtkp88weisR6YvaL0UM5mlON/r0Pu1GYu/tDA5QtILVLdESwrKSqbX10mUhLNRTJXHiELN+Y0NtNUe4TzHyAuyho5WzMjjmr1feI6tvTNWfm+PjWWC5v4Ug2aXVs/C0GxkWfrI6sfj+OXPRcM8vINMV1RUw0BZbSA4Sy7+1nexXfiDf/NbNNr/DjcfsDMoSe8UaM0ZUbxFmUdImoMqiUzFMZN3N3HFGEGokec1qloyLxbUlURaGWh5Tu1HxLMavSmRDSOmpsxKs4daKZRSSd6OKH/9GsZbDRqr7/Duj34WxdjlS5ePmM5Lkj+XETY9gsYm4SvfZHQq8sITG2ye71MKm1SFi6oXyIFGq5khGUuqsmIRJCz9GbPTY6K9nDCZow1h5gVUYkIxzTjYdul0bfL33+TcL7yNLBf83v/yG2wZCdsbS6TOGPW4w1nnFCV6kXPKRVy1QpY9Wj0HvBWMEBQ7QctM3HxBLeQs9Ba5bZL4IMcZBCmSpEAmIlj3DIAYSKRNHVmvkG0PtyxJooz5TQuMkr/z7IRSFHjtXYlb9fsE7V+hU5wy6ug0Aw+7FXF6bZ2eFkNLxwwtzq3oaNqMtDDRjXVWmiYeMvgxOAr+vg9uApaBftXlxuUl4iWJdNXn4kCmt2mja60HycenbWd9FJDyJ1EwH4oDxXuf29BVA9tssHf11yiSgLJ9hdVLr+AuBcL9mMPvnKNj+JjnXbwkZRaPkSWHsioJrnd4/saIxeWA5SoUUoYW6pS5S6aGLGURI06R/YJcMciTgMLSOZ0UrLseru2QVAVJXfHyn/3nfNn6EafDJ9GqBc1zM/Lnl7zxj57Fm5yw8U9X8P58xMl8xnrjPP32Jt3Tp9lY+ORXzsi1FpGW4SxaeFJBrhQsSohqFWFSYqxu4folsTrkeE/CKjNKQPQaRFKTV976J6zo36RefY7mE29RXr9IZmks/8yg226itGKiacrFtU0a7Qoh3UAXddRJyI53wO2mid+doe+6zKsUZ6tPburEeYfmScrRYkYULjFdm9JXMHSBghqhFqjLJeOzPh3tAH8pMJkesfFTM4yXQr7zT74M/j50avLMR3RUxIbO8nSJ/bvP8sJCZvl0TN4tSOycfjbAsVoUQoWmKoSCiHCcYtoVaWiRahnMY5pb62SCSrmvMp3BfPINOld2aLgDTFNDUT7b2u/ntoAPW0FZljE1HUHvsqIv+eH1f4DcX+fZF1/Gc4ak15/GbNecVC2apwb9lbuM0vpeFbGWIv7UCdG1Pj4ZCCpVWhDEC7akbfZOU+aGx9Kb45QlWWQyQ8Jey6hKiePhiCgIyWSRd+tLNJKYo2KN0VzFDd5CevY22a5Dalcslw7nj/8uRuOPuDs12enZbPzKGMF/mdFfPoWrQKIkVHmIgoajNskrkTILCeZTRqOAcG4gSRso+iGrVsLZKAdBoDyDbxZ/j2Z1zJ+98nU6T32Fy0/9MYtlRXj9Crlb4UkG+nAHQzsiF7qsthpUYkGapqhBwdAtEEWBwkoIhBKlMliPnmCe3kXw5xjFNmOpw+l0jKkcoVc1flST1RL1CCLdRA0KjsR1SkXh8tf/lM1ffZ/4ZBNPjjgRm0gjhd6zN4hNl4u/+CYzqSQdStTLNcqyQEw0pGZGfprSarYBkaRImYcZpAKSVjENTLTGkCzMUe0WfgST63NuHq/zlbUWTuP8vVI43XzsHuBPyiseGLbPujn6/m4vVVFp2xpuW8WuID/qspiaOL2CzDpDkSO2nrhKoQwZaxdRdkJOR9fYv+mx/yMVrz0jijxyYpbTkLpKEZI56cKnJzeRixpvMaeIchIUooOAXGwxmoscjE6YRSGD9iEIRyzSHswNkjSgeWnOWRTQe/4W7V7K9tde4WaxRVtrYBsVR9/uM3y1jytWLKIxoRfQKFdQdZUqyxAKFVXSWGnYtFoy/ugUvzylPBWZTROyVEOoQQhFnl95n7qfUc0mlMcr+PMGzrO7zMciq08OsVZu0rASxsYakTohXsYkC5jMfGolZCFVoFWkVYofLnFFFTmKML0A3fc4uzvGH9ZI/Q3c4lfRL/w0VevLKGtP0frq81z4Byv4X/v7iGv/KbXymxSDgkUe86N3JlT91zGkY1q9mN10B7cjo76+4OzNFZazkqSVkFY+6bygtFLi0MPzPIJwSRhXGEhUWUYYiqRSiDgrEBvrhHGL5d6Y8GTOlhWyutZipdfFMi009cP0y+fZS/2JWfCj30+RJBFN01DlBq5qocgz0jTHX/w8+Ru3SAa3GO6u4/zcMQSwou1ysT5GWpM58jZZnDR431MYJCYTDohPZS5v/gZCquOP/pCLF7dQlCa3kwOU2iOcXCDVjnEWGepzK+y9d4HFWZ/6RkCn/B6ZY6Ple9SiyuzNnCP5LpPlKitfF6jkmvOrbzB59wlW+0OSckaUXOZsMScNCjrndeqsokpXESKZtFgg9kxkYlpdnbWZwNFIRm/kEIPWFylPVN6+eQH5eoN8+S6kIUEaEy5/gSz8H8n0Q0YzA/f8Xeqrq1y4+AbiUmTuXSGvbYTM51qcUwka1doJ5bsKWSyx1vgG6XSMKPscvH+Lsjwg9jeYnZhMBhrGLEe8EDN8f43xq22UYg1dfQdFkykTnXnwFNXvvMH+YhdOV7k8WMU2Wijum1hvntAKRfy+wmSywq5m0PFk/PUJzjUbI/0qcXiLQgzobp4j90NqeZdyeZ4qXBKpHrb1BNWqz/4tjf2bKs9ubLPa6dNoOJiG8RHu79MA+LH1gI9+v+/ROFAQxHulVpZGq2HTNiVGwQHy8nlG82eJbh9QuxF772Ss//Qb+N/fQdxIGU2nJKOrnM5eIm31qLwVln+RIBQ2qthkXu7iFxGTmxHR0EX0VkgauzzlXmR3+hRH8i7RHz+J0DZxycnqYwrlPEWcIyxv4W6d5+4Pvsz4B2/hxybmLydMhesImCiSRpYtEeMblJxjsXKGMu6jZCqpnFNFGbV0RqHEpGcqqS+itkw2WjU9LE5ylUBrszx2cKSQTdOmkkTmiUFKSZ29j7x8nvH0RfK7d9GXP0MkDnE77+CfNXji0iEHwwVlmBHTZ9GR0SYDxHcy4rjCkrZw+iaLScnBe6fsH5whlUB9l6pWOdpVKQYQfddBcvcwBIiBRVVRVzlpHqIIE9KoS0uzufLU05zfuIig5njBmDIfMVumuMWbjIvn8TSLZtVAXpRM5iGb/RVWOw2SKiYYihzefY9SMOkigzpn5DmUF7Y5+XaG71+lr2dsbwxY6XdxbftD5PNn/ezdZy7Jf1x5viLL6LqG2zBotHT0kzmns2fYWfs5Ds7+e4LFguxb0DmvYLfv8Dv/XUwpyphWh/+7vXOJjSvN7vvvPuu+6/0mWXxJlNTdavV7jEE6MxNP4EyyyQtxgATJznAWWSQbAwG8CbLyKosAsZPASAZJgCALB05iw55O257M9LRmerrVrRYpiaJEskhWsYr1rrpVdV9ZUKTZFPXuCcbTOgABgix8de93//ec75zzff+/YtcoJDvctASidVj6hsTWdp9Ov8febpcZTUWapvCGLp3oNvWWwetFi/95b4eD6H2Mesh+UiVp6jilS7h3Vgn7LRrR32AxJVCX/hivk6H7J1NGhTjBdB+3M6GnysTENLLpEvWruMkssi+BPqDR6iKLErJo4k8DvAnEJBF/miHqVHBSq/hNh2Gsj+oLHMQOkBWdwBkhtxSizgbVxrsszX6Lrdpv0Tt4GemqhvntBq16yLX/ukGulCQmC4ReleH+HGE5gX3LYxDvYoffYf2uR702pN/oYxga8bRNGAqMR2NMr0dn7KObfRRgyiGjg6iAFAnIMRHDSpLP/FXyOZNycZ5UKsNg2sKb7rH6v4ZkczqGUSRpbFEfJKnNSlysW1QrNT5fu0XcTBFOXBQE2t0WjniFnUGXnt3gsv02N2+ITPwDlN4WZjwiW8iQzSaxLOOB1tvjGMSeKQn5Yj1QRIvFcAyDhKljagdUpYjbvEUpq3Ew3mQymqV6vUfi+hjMHHtNjYQ6wy8vjPDWfsxHzKILETlzgaZ4l77Xoh82uVftYU9MxkOZURiwE66R3v46KT3PXut9JnQJYlms8t9EWN1g1NtECiOC5AKfjjximoKih+DL9OoSkx+LqLEJgZJG1t9GUkY4rSnbSoTvD7DEDDG7T7cd4E5GKIFOJqsjCOD1DPqdAMG20EIJQQiIxIihLpNzZeT9IUMkFDr0owm3gjeZLcl0dq4Rm1ym026R2L1NT5insyeC8hpJ/VNmunfZya7gC10mHRlzRmIcjNibbtLsdZlbTrN8sYJjO0QIBFOPkTdiEoZIkkAgRojhYd1UlGU0NYZmGcTjFpm4TDyeQhJV9tt7tHZ2iaUvMRFdEolfocJ7BPV1antpxsUOS5k5/m//x+y2dMb+EHVqkxMv0HIDeuI6njTF9r+Bpl5n0tshCA7IFzKU8zkS9lH4lb+Q/R6xpD7JJpcnrgOeDMeiKB1u0bdMnJSDkrUQohvMDv8tclxjdGeIHMgMtyckXJ+MdJFxYZd8fo5O5ibXtMvIfR/R0kiRRc98xvZuE98bMBEGyBFEukgivMxAuslQKtHyddL5rxP0f59l8x2i1JTV3R6a0MXNXELPbJK7/U9ohDpRZBAIA7wDFxOB/lDCz+tMBJ8xe2jFb6J7fcZ0sIMUsws2xZmIqWcjCiLe1KdRTRLzZmlJNXTVpe3GEEIfQQ1Y2Y4YzB6wlzSRQxNTmRIJ1yj2vosW1+mttzAFDzYC0oMWk/4VGnKV4XiTvdSQ/MoVJo0DupMpxVSaVyobXFuPaDa2UXWf4kyRxeUFCoUcqqoSRQFTPzykHhZCwiAC+fC4rCCI91lgJQxNQtd0FEUjCHwQQw5yBWobK6jOKqPIoy5PaJdmme53aatxzJrM2yvwvY8+pTvsct75e8j+IlX3D9HT22TDbxGzR2w1R3iDFkltzEI6SzabwHIsYvf3/T0tld1TUXOcVQ9UVQVdV8mlQuYS60T7f8xUz7Fzo4c31UEMEbcleq7HbKmPV73MN3+pR213Gc2/jhcKXCzGsNrrtHoDxn6AogpMxzWy+jl2ejKhruLJu/ygs43nzDCbXCbBvyDyrtJcew93WCfr71G5fJ2M+O+55aXY3RySNmyEqIddHYEACWNIYyckeb6OoyYRVBFFKaMEfaaNe8jTJGEsgzeOEXRMvL7K7laXwfRTtFKNTujQ9kUscQjjGNP+dSw7Iop1yVofYYoVjNFVJlaa+moff2IzUfZRqjadqYfttGivpQjTNf76lTfpey63dmt03IiXnCmhu4obmAzdGsVshvJMgUIxT6l8eLJMkiVEUThkGwgPeWlE8ZCbWRAiRFFAkdX7dMiHYPA8j+l0TDY3Q7pY49MPErzx7lXK5hUG9pBb0x6NzS5J4zNKs1m+/Ze+xuaWg7PzDvd6H9MabJFKH6DqZf6g1aE5GRC4dZKpNIVsikwygWHox/w5z0N5/MRrwKMvEEURWZIwdA0zlsKMSgzvzBO1P2Y+LTAx2rQmKuNhkc52hyj3Y7xWyG//qwv40qeI+gQtY9Bdn+Mzz8bnDsNhG0OPMxVGeK19LiQTbAw1RuJHKLG76LWIaNBhoiRZ7f4GQ88hVbmMN5Hp/iTBQTeg37hHPNJJOSZMVIKaj9fVKH9jnf4HCTarc8QjkIUDjKSAky2iJk1E43ABLYsSkeEjBj1Kcy77Bx6jQQ5vGCfZ3sbMeISyxTj4d4T7ReLJS7SqMaxSDPeghLT+GZWkwFTrMJoOGI9fp7Vvkbx0g8LGa9T6l6n+sMB250OaQx9P1WiPV5DGLfbrexAEZMtpctkUmXSSZDKBrmvHxEhP0qk6MkmSsC2HZCJLoZJma22bP/3vFZbnIlSthzDW2J4M6KsmtXEbWb/HoLHCrfof0t27Tm45Qo4uMh7PIQvbpA96DMUpc+U5CiUHxzYwT2S/P1MAnkVnK8kyeixGMu4wkyvSLizTGI6obW6AKKFqLn1NR3IqVG/XUDMfcNBYQ5JnKczOI6sqr/61edqdba79OKLZETl/Pok/CdjqD3ilPeLN0ZRd9wK381exhQmtRsSGNKXv2yRz81jSmJVChTfnFtiqbvPB7hhXFHHsDC4B23frOK6L17T55t//Kd//Hz7D9Ay2tsAwiogFA2R3Qugn8CMZQ5GJxJDuWIC9gGR1jDZwifn38BIuY6OHFl5Ey84zUioY4w8omZeQIpGCkUbgIq2dm4zDKT6g5MELTAbVPku/8jHj91zWVwX2pVlI73D55TmSKwF3VhU+XR2Ry1rkCxlS+RS2baJpsWPi7ycJb6d5tzXdIJFMkUoUWXh9nV7jKtWNRbKVdxGzHTLBT3DHIzabKnc376DRAEnAqqhEaovvtP42mrDHRv8WH3U+I3NFpVwpkEnnsOz4MdvtkyYfz+0BT4fhWEzDNC0S2RzGzCJ60+SvLP4ym+s/ZLdxGydq4lpJxFgeUfEovaRAIDPuNrmU+Q1uvJ+l1vmQ9Z0WhbksqUyKXjekJ++wFp7jTWnMt4TzpA56XHNuI6cD0qJMKrqEr8aYiZm8cfFrVKdD1s1FkHucWzQol8r02iaD/Q6d5nXaN6c08hLLqTa7d+PU7evk5ucZDuP4noEsiaiKyDgaMPG8Q+ozc4pUahNGY6KoS+D5ZA/KWLPfod39PXZSIoXaeexEFiuVZOrMkK9rrFwR+IP3v8u1zRsUChKSk6K33mQoGGjGkF5tlf1Bh5ncCsHOr/Jp1WW7sYoq1EhkTPLFDImEg2EZx/zNJ6UPntQOj2zGsGybTLpIvT5LvLTLwf49Vq/d5dzFf4ZtX8YL/g8jtigvWERBQBhNkCOJmeDXiGMie9sE3TWkWIv54gWKxTypdPr40NFJmuPHJR/PlQWfFYYVRcE0LVLJFKVkhmGsRrXRJpVM8nnNR1Z9ijtbKPjkz+eYjiOCoMzOxOZPPvltkqFBP2ozjbVJJC6Ry6aIKVP2Gy2iO7cIZQvVSzE3/+u8EUzxqp/x4cUlZP13eafk4KYL7HZ99ncDWjfuIgQj8sUl5ioV+ukkg6nLrWaHm59WEf0Cmt8mJjhE2zI/Wt+hbPWwsyqaLSAQEosbRNhMRRmrL5N2U8z143Syr3IhKmDXd/m9hEF/mCPX2CS1fJ75uSuo2iKxfpzUqM7ewV2QFVp+yFIqRTQe09gWMI0ZxF6P9qRHbr5Iqznk5up/QzJkasO7iFaXXPaXyGRSWLaJqioPdBaehF315OdlWcY0bVKpLLnUApn0DnWzTTh0+bPP/w0LsUXSiQpO7BUmwzZ2PgHaS0jpRZaaCoPwExrtO/zAbFE4l6ayWKZQyOE4iUOipsewzH6pHvDMbfqShG7opBJJysUsg/Np1n56lT+6+mcIlk14TyTomARTn3vVFmnNR0/J3DEsvElA3/LpSR7LK/MsXVxirlJmmJ4wGHnsVO9yr5dASCp0tQ3SqyGaViYxuMCdTJq61KLzk6/R2d9gr7rKILrH8psLzM2XyeZzpHNZglAgHIR8NlX53odVcjGBfEZhL5ElHKpYxSRC4QINLY3WHOO9d53KWxfo+ud4u+2yKKrYmsXVbJzgB7e4Tp36VpPr0ohLBZPF4O8w+WmTXncNVe+ybexy8/Z13l/9mJVXXyedyFK9UaOvzqGrOeqbVeRMFjVpEuyIOHNjDgyVbsPjQrlAoZgjm01gmvoXGOYfphrwOAchSRKaruEkEmTzZUpzy9RrXW53PsfrdRGKB7iLBaZ2EaOTovnJKrNXitT2BWqNHTbdbbamd2BWYHn5FSrLC2RzKUzL+sKZ32dRvXriVtyjwvChm9dw4jaFmTSNdgp1J4Oem+P2zi7qQY+mpOPoeRJSgv1RG2F0k45RxPUFRkKcpfMLXHntVc5fukgpl8IduHi+gDQace/aPdqtGs39Neqcg8imyU9oXP0d0P8u/v5/YH+6i6v7lFcqzK/MU5qbYW5+AQSBmKqhSCqK6XDnxi12bm/Qqm0QjsE0FDb1NGMcpLHISJHJF9Ksr31IlG6z0fep4hMfhvzQmvBJqDNVVJqJfeLjG7yb/C2qn/8+n7Q+phd2GfkdtlsNkC1efvNrvPbWW9RubdMcByx//W1Gu33QSyTnM3R7ArlkiW7hJbYGNVTdoLQwT2l2hng8jmE8uLh/kvB2eg12FKVsxyGXL7Bw7gKj0YieF3F7bYt7AQQdF3XQYBpGJE2fg2t/xFRP0Ok0GIRNcvNxXn7jFc6/+hKFygzxhHMsdfG82e9jW3FPUpSWZQnLsUl5WZaWVwgDAV0zSa3dYmdnm0HLxwt9alMXOSYSChaq6TObn2F+5QKXVs6xsrxMpTJDIuHgT6bIagxdEtGdGPtrW+yPN7im3CTwA4a317GFGW43riKZPs5SgqXFBRaXllhcPMf8wuKx9IAqy+i6SSKRIZ8rsDU3x+a9W+w3m3S9BrXqDdStNaJGgJ/WGYkTYvV96N6lP/WYiGPSY9ibTlAMC8uxQPyMd5f+Oe3PP+Jq5ypSJcFsbgFJFHhdM0hlciiCyObGFu3WAS9duYxu29T2e5SWLxDqLsrGASR0aqEKtbuUUwpz5RL5Yh7Lth7Q13ia2tppQByu1WOkcxmCMCIiQjYskpkcd+o9Duo3Ge67eDGRNlPkaR9JMXFKGucqi7y0fI6FV89khTITAAAIt0lEQVSxuDRHtpDEcR7u/Z5V7Ed+Gk2M0zcoSRKqqhJPJInCCEVRiacSzC5W2Nur0+uMGfUHTKZDJFEkpsnopk22OEthZpaZfI5iIU8ynUA3YgRegKzEsHUdJ21zMD/DUq3BgdulN2iDf4FYGBKzFEzHJpNPUSiXyBUK5NJ54qkklmUhCAKGaRDTdGzbJp2JMztXZLleYbe2T7s3pjv2cds9PGvCSHaRpwFiRQJ5iCKLxFQFVYrxkpolNBV83+fr5m9Svt3gg+kNtNksL7/zBumERRT59OodRgf77I+7iAS8/NYVFNHh+p/ew8x46LM23Q+uM3VtwpzFqHkPM+hxbukSc3NlspkUpmUee5fTD/hJgXjyGR1l0IZpkMmlkSQB07bIl2ZZqrdo7tUYdPv0vCmi56HKAoblkEvGyZdyzJbKFMtFktkU8fih+MxZpZen8YKP7IQ8iZs/+YVHE6VpGsl0GkVVcRyHUqFEp9ej3xszcSdE4fiYANywHOKpNI5jE7ctbNvCMPVjHuSYFkPXDqW2+qUCo96AUX+IN50SEhCJoKoSumViOTa2Hcd2bHTdwDTNY/IeWZZRZBlD13Eci0IhR6dTodXqM5x49Icu496Q8ci9rz/nEUkgyTEMSQBFQiQidH36O21S6yKX3B1ulbbo1ceki2WSjkkqlUHWRMKBh6XrlNQKHgGbO7s01qcYcQ2zbDP49AZRe8pEyzEQfMYHG1yqpFlcmmO2kCNu22hnHOp5Wg9zFrePqqjYto0kimi6TjKbY25+SK/dZ9gf4PsBcCjvpesmccvASljEHQfbtjEtAyWmfkFz7yECNE8tXfFMWfADhen7b62iKJiWRSKZJD8e4/sBYRQRBoetJOGYZUE9JAE/Ues66VUVRca0TBLjxCEhuO8TRocihaIgIkoCsiShxFRU5VD74+Q4x7yGooisKOiGju04pLNZSuMx47HPeDoh9AKCwCfwfaL7IlFCJCCJEBIxnU44aBywNp3w8UGNPXeM1o9QTJNQkgiIGLQC4plDrj6JIu5gwnBvFXdvG7t8CVkNqP4URutJAqdErKhyZ+cec3M+Fy4ss7A0Tyabwrof3s7yfs+jLXI03lFoj2kapmUzSU/xiveFgqKIKIwQJRFJEFFUGUVViWkxYqp6LEj4CO23Z8LQUychZ4kYCqfkOo9qUPZ9CYIwCO8/XM6UIDjpSb+w60ZR0HTtWMrgSM7gSBL25BgnJ+d0onTMbaOqmJZJ4PsEQYgfBviefygTFob3VUgPW1thGBKEIcPBkBCwcnWE6Rbr4xhK2+LlME4tGFFvtEjaEdPuPn4nzihcZeL1iOVyGOIc9f0mYrXIea/IdHmeenjAxtYq5azPxXNvcf7SEuVykWQy8YD3e14hxbO4fk4qW5qhfmJuIyD6wot7NK8PUxn9MsTHnykJOV14fADRX2Bzjx4rRPgwNaGT4z+JoOHDhJOPQvvRhEb39cu+cI1Ef66SF0FEROAHyJLMwB2QzzpUt0VGwToT4SI4MvoFnXNzOTb+c4OeFcczmwz8bZYKl9ADgbUf/Igd/x1yqQQb4QDPUrnb6CIXJV69tMhrL59nYalCtpDGdqwHCrvPmlmeXped9IQnNTseNben5/L0dT3Pi/E4Dxg9svH4iHB88iafZE/Yo46DniXr+qTjPE7R+3Gi2Ud/8wUfw9SJxx3STp6kU+T2WhPZ/5yDVIxKW8Za8THyBu35KsUP/xEbtf/CsPp9uo0BPfVlIEu7fpepEdDc2yVuhaxcWuK1y5dZOb9MoVggHnceWD48r/c7C4SnH/yT6uT9rJXmHxArfB40n76wh7EinbXz+lGgeJpxTo/1sLf7YV7m6POyLB+2syybTD7N8nKFkdvk7r1P2G/UiToqTW+Bz7LXyCTiBJ27yH6WvW0NN3SJRiGwTmO8hz8cUVmZ4cL5BS6+/BIry/OUZspkUnF0/cutq50FwtMe71mfz5cIPu+sEDyMosh81hFPP9wnbRmd/vzDJJ2eZpzTtbDHeb1HtbNs08JNJZmdm2fqTokChY3tz/ms3WK09n0m4Q4HFQs3aGL2Zul0B0zcOkJ2hBqXKDhxZoqLLC4ts7y8zHylTL6YJ5VKHIPvLE3dL8MeBrinndfnqfM9ZOzqWR7wJ8BfftabfFZ1yYfxhjzrOI8TEnwUV93p5QRwWMhNpgmjkJgqoxs2+dwM1a0qu8ktBvsa0URAliDKNMmVFGQlTjJTIeVkKZWKFGfyFPNFCqU8iUQcx7HQ7ncUTkoZ/CxC3MNezGd5Pl/CdUVRFAmCIHz4QMiNouifAv/6adaBv4h2cr10rBjq+4zHLr1Bn16nS6vVpNXt0Gt1GU0GTPoT+sMQQZDR9Ri6IWLaDrbhEE/ESaXiOHEH0zQxTf14p8vDsstf5Lm9f4/CmWu+6L59lQF4Opk5+t33fTzPw524uCOXieviumMm4wlhFBIEEVEkIokisgqqqhOLaRi6dlhc17XjZOMs+YKflff7ecKfIAhCFEW/I4rirx2D8dSC9btRFP2Dw89FwlcdgCd/P1K7PPKIvn+o9O4HPoEf3FdDFw6PrirSIZmTJB2H2ZN1z9PA+5IX+D+3ALzv2AxRFN2HZr3hYVXyK21ngfCkNzwJyAfraHCkLXzWz+l+7fM28/8igU8QhH8pCMJvhmH457JdZ6Tt3wLeexGKvxiKTwPxUYnUWR7u/0dN7eccfFuCIFROgu8BD3j0zyiK/jHwu0fFaeEXeWX8mOzvLCA+bUnn9HjPupnzL+Y0RoIgCC1BENInMfaoEHwEwiNPeIRivqoe8Um7KI8D4Vck3J5MOBAE4XuCIHz7LPA9FFCn2mD/CfiHD5m0r1SYfl5v9QsKvIc5pz7wq6Io/u8wDB9aYhIeV7O5j1xBEIRfB/4W8A5gfQXe4hf29GvmdUEQvgf8R0EQfvSlLDNeAO2FPYsdbZ97YS/shb2wF/bCXtjPpf0/XKu+YjxI26EAAAAASUVORK5CYII=\"/></html>";
    
    f_logo_label->setText(f_logo_text);
    f_logo_label->setMinimumSize(90, 30);   
    f_logo_label->show();
    m_oscillator_layout->lms_add_spacer();
    m_oscillator_layout->lms_add_widget(f_logo_label);
    
        
    m_oscillator_layout->lms_add_layout();
    
    m_osc1 = new LMS_oscillator_widget(f_info, this, QString("Oscillator 1") , WAYV_OSC1_PITCH, WAYV_OSC1_TUNE, WAYV_OSC1_VOLUME, WAYV_OSC1_TYPE, f_osc_types);
    m_osc1->lms_vol_knob->lms_knob->setMinimum(-30);
    
    m_osc1_uni_voices = new LMS_knob_regular(QString("Unison"), 1, 7, 1, 30, QString(""), m_osc1->lms_groupbox->lms_groupbox, f_info, lms_kc_integer, WAYV_OSC1_UNISON_VOICES);
    m_osc1->lms_groupbox->lms_add_h(m_osc1_uni_voices);
    connect(m_osc1_uni_voices->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT( osc1UnisonVoicesChanged(int)));    
    
    m_osc1_uni_spread = new LMS_knob_regular(QString("Spread"), 0, 100, 1, 30, QString(""), m_osc1->lms_groupbox->lms_groupbox, f_info, lms_kc_decimal, WAYV_OSC1_UNISON_SPREAD);
    m_osc1->lms_groupbox->lms_add_h(m_osc1_uni_spread);
    connect(m_osc1_uni_spread->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(osc1UnisonSpreadChanged(int)));
    
    m_oscillator_layout->lms_add_widget(m_osc1->lms_groupbox->lms_groupbox);
    
    connect(m_osc1->lms_pitch_knob->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(osc1PitchChanged(int)));
    connect(m_osc1->lms_fine_knob->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(osc1TuneChanged(int)));        
    connect(m_osc1->lms_vol_knob->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(osc1VolumeChanged(int)));        
    connect(m_osc1->lms_osc_type_box->lms_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(osc1TypeChanged(int)));

    m_adsr_amp1 = new LMS_adsr_widget(this, f_info, TRUE, WAYV_ATTACK1, WAYV_DECAY1, WAYV_SUSTAIN1, WAYV_RELEASE1, QString("ADSR Osc1"));    
    m_adsr_amp1->lms_sustain->lms_knob->setMinimum(-30);
    m_oscillator_layout->lms_add_widget(m_adsr_amp1->lms_groupbox_adsr->lms_groupbox);
    
    connect(m_adsr_amp1->lms_attack->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(attack1Changed(int)));    
    connect(m_adsr_amp1->lms_decay->lms_knob,   SIGNAL(valueChanged(int)), this, SLOT(decay1Changed(int)));        
    connect(m_adsr_amp1->lms_sustain->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(sustain1Changed(int)));        
    connect(m_adsr_amp1->lms_release->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(release1Changed(int)));    
    
    m_adsr_amp1_checkbox = new LMS_checkbox(this, f_info, WAYV_ADSR1_CHECKBOX, QString("On"));    
    connect(m_adsr_amp1_checkbox->lms_checkbox, SIGNAL(toggled(bool)), this, SLOT(adsr1checkChanged(bool)));
    m_adsr_amp1->lms_groupbox_adsr->lms_add_h(m_adsr_amp1_checkbox);
    m_oscillator_layout->lms_add_spacer();
    
    m_oscillator_layout->lms_add_layout();    
    
    m_osc2 = new LMS_oscillator_widget(f_info, this, QString("Oscillator 2"), WAYV_OSC2_PITCH, WAYV_OSC2_TUNE, WAYV_OSC2_VOLUME, WAYV_OSC2_TYPE, f_osc_types);    
    m_osc2->lms_vol_knob->lms_knob->setMinimum(-30);
    
    m_osc2_uni_voices = new LMS_knob_regular(QString("Unison"), 1, 7, 1, 30, QString(""), m_osc1->lms_groupbox->lms_groupbox, f_info, lms_kc_integer, WAYV_OSC2_UNISON_VOICES);
    m_osc2->lms_groupbox->lms_add_h(m_osc2_uni_voices);
    connect(m_osc2_uni_voices->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT( osc2UnisonVoicesChanged(int)));    
    
    m_osc2_uni_spread = new LMS_knob_regular(QString("Spread"), 0, 100, 1, 30, QString(""), m_osc1->lms_groupbox->lms_groupbox, f_info, lms_kc_decimal, WAYV_OSC2_UNISON_SPREAD);
    m_osc2->lms_groupbox->lms_add_h(m_osc2_uni_spread);
    connect(m_osc2_uni_spread->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(osc2UnisonSpreadChanged(int)));
    
    
    m_oscillator_layout->lms_add_widget(m_osc2->lms_groupbox->lms_groupbox);
    connect(m_osc2->lms_pitch_knob->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(osc2PitchChanged(int)));
    connect(m_osc2->lms_fine_knob->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(osc2TuneChanged(int)));
    connect(m_osc2->lms_vol_knob->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(osc2VolumeChanged(int)));    
    connect(m_osc2->lms_osc_type_box->lms_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(osc2TypeChanged(int)));
        
    m_adsr_amp2 = new LMS_adsr_widget(this, f_info, TRUE, WAYV_ATTACK2, WAYV_DECAY2, WAYV_SUSTAIN2, WAYV_RELEASE2, QString("ADSR Osc2"));   
    m_adsr_amp2->lms_sustain->lms_knob->setMinimum(-30);
    m_oscillator_layout->lms_add_widget(m_adsr_amp2->lms_groupbox_adsr->lms_groupbox);
    m_oscillator_layout->lms_add_spacer();
    
    connect(m_adsr_amp2->lms_attack->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(attack2Changed(int)));    
    connect(m_adsr_amp2->lms_decay->lms_knob,   SIGNAL(valueChanged(int)), this, SLOT(decay2Changed(int)));        
    connect(m_adsr_amp2->lms_sustain->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(sustain2Changed(int)));        
    connect(m_adsr_amp2->lms_release->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(release2Changed(int)));    
    
    m_adsr_amp2_checkbox = new LMS_checkbox(this, f_info, WAYV_ADSR2_CHECKBOX, QString("On"));    
    connect(m_adsr_amp2_checkbox->lms_checkbox, SIGNAL(toggled(bool)), this, SLOT(adsr2checkChanged(bool)));
    m_adsr_amp2->lms_groupbox_adsr->lms_add_h(m_adsr_amp2_checkbox);
    
    m_oscillator_layout->lms_add_layout();
    
    m_master = new LMS_master_widget(this, f_info, WAYV_MASTER_VOLUME, -1, 
            -1, WAYV_MASTER_GLIDE, WAYV_MASTER_PITCHBEND_AMT, QString("Master"), FALSE);
    m_master->lms_master_volume->lms_knob->setMinimum(-30);
    m_oscillator_layout->lms_add_widget(m_master->lms_groupbox->lms_groupbox);    
    
    connect(m_master->lms_master_volume->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(masterVolumeChanged(int)));
    connect(m_master->lms_master_glide->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(masterGlideChanged(int)));    
    connect(m_master->lms_master_pitchbend_amt->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(masterPitchbendAmtChanged(int)));
    
    m_adsr_amp_main = new LMS_adsr_widget(this, f_info, TRUE, WAYV_ATTACK_MAIN, WAYV_DECAY_MAIN, WAYV_SUSTAIN_MAIN, WAYV_RELEASE_MAIN, QString("ADSR Master"));    
    m_adsr_amp_main->lms_sustain->lms_knob->setMinimum(-30);
    m_oscillator_layout->lms_add_widget(m_adsr_amp_main->lms_groupbox_adsr->lms_groupbox);
    
    connect(m_adsr_amp_main->lms_attack->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(attackMainChanged(int)));    
    connect(m_adsr_amp_main->lms_decay->lms_knob,   SIGNAL(valueChanged(int)), this, SLOT(decayMainChanged(int)));        
    connect(m_adsr_amp_main->lms_sustain->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(sustainMainChanged(int)));        
    connect(m_adsr_amp_main->lms_release->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(releaseMainChanged(int)));    
    
    
    m_groupbox_noise = new LMS_group_box(this, QString("Noise"), f_info);
    m_oscillator_layout->lms_add_widget(m_groupbox_noise->lms_groupbox);

    m_noise_amp = new LMS_knob_regular(QString("Vol"), -60, 0, 1, 30, QString(""), m_groupbox_noise->lms_groupbox, f_info, lms_kc_integer, WAYV_NOISE_AMP);
    m_groupbox_noise->lms_add_h(m_noise_amp);
    connect(m_noise_amp->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(noiseAmpChanged(int)));

    m_noise_type = new LMS_combobox(QString("Type"), this, QStringList() << QString("Off") << QString("White") << QString("Pink"), LMS_NOISE_TYPE, f_info);
    m_noise_type->lms_combobox->setMinimumWidth(64);
    m_groupbox_noise->lms_add_h(m_noise_type);
    connect(m_noise_type->lms_combobox,  SIGNAL(currentIndexChanged(int)), this, SLOT(noise_typeChanged(int)));

    m_oscillator_layout->lms_add_spacer();
    m_oscillator_layout->lms_add_layout();
    m_oscillator_layout->lms_add_vertical_spacer();
    
    m_main_layout = new LMS_main_layout(m_poly_fx_tab);
        
    //From Modulex

    m_fx0 = new LMS_multieffect(this, QString("FX1"), f_info, WAYV_FX0_KNOB0, WAYV_FX0_KNOB1, WAYV_FX0_KNOB2, WAYV_FX0_COMBOBOX);
    connect(m_fx0->lms_knob1->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx0knob0Changed(int)));
    connect(m_fx0->lms_knob2->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx0knob1Changed(int)));
    connect(m_fx0->lms_knob3->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx0knob2Changed(int)));
    connect(m_fx0->lms_combobox->lms_combobox,  SIGNAL(currentIndexChanged(int)), this, SLOT(fx0comboboxChanged(int)));

    m_main_layout->lms_add_widget(m_fx0->lms_groupbox->lms_groupbox);

    m_fx1 = new LMS_multieffect(this, QString("FX2"), f_info, WAYV_FX1_KNOB0, WAYV_FX1_KNOB1, WAYV_FX1_KNOB2, WAYV_FX1_COMBOBOX);
    connect(m_fx1->lms_knob1->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx1knob0Changed(int)));
    connect(m_fx1->lms_knob2->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx1knob1Changed(int)));
    connect(m_fx1->lms_knob3->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx1knob2Changed(int)));
    connect(m_fx1->lms_combobox->lms_combobox,  SIGNAL(currentIndexChanged(int)), this, SLOT(fx1comboboxChanged(int)));

    m_main_layout->lms_add_widget(m_fx1->lms_groupbox->lms_groupbox);

    m_main_layout->lms_add_layout();    

    m_fx2 = new LMS_multieffect(this, QString("FX3"), f_info, WAYV_FX2_KNOB0, WAYV_FX2_KNOB1, WAYV_FX2_KNOB2, WAYV_FX2_COMBOBOX);
    connect(m_fx2->lms_knob1->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx2knob0Changed(int)));
    connect(m_fx2->lms_knob2->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx2knob1Changed(int)));
    connect(m_fx2->lms_knob3->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx2knob2Changed(int)));
    connect(m_fx2->lms_combobox->lms_combobox,  SIGNAL(currentIndexChanged(int)), this, SLOT(fx2comboboxChanged(int)));

    m_main_layout->lms_add_widget(m_fx2->lms_groupbox->lms_groupbox);

    m_fx3 = new LMS_multieffect(this, QString("FX4"), f_info, WAYV_FX3_KNOB0, WAYV_FX3_KNOB1, WAYV_FX3_KNOB2, WAYV_FX3_COMBOBOX);
    connect(m_fx3->lms_knob1->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx3knob0Changed(int)));
    connect(m_fx3->lms_knob2->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx3knob1Changed(int)));
    connect(m_fx3->lms_knob3->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(fx3knob2Changed(int)));
    connect(m_fx3->lms_combobox->lms_combobox,  SIGNAL(currentIndexChanged(int)), this, SLOT(fx3comboboxChanged(int)));

    m_main_layout->lms_add_widget(m_fx3->lms_groupbox->lms_groupbox);

    m_main_layout->lms_add_layout();  


    //New mod matrix

    QList <LMS_mod_matrix_column*> f_mod_matrix_columns;

    f_mod_matrix_columns << new LMS_mod_matrix_column(spinbox, QString("FX1\nCtrl1"), -100, 100, 0); 
    f_mod_matrix_columns << new LMS_mod_matrix_column(spinbox, QString("FX1\nCtrl2"), -100, 100, 0); 
    f_mod_matrix_columns << new LMS_mod_matrix_column(spinbox, QString("FX1\nCtrl3"), -100, 100, 0); 

    f_mod_matrix_columns << new LMS_mod_matrix_column(spinbox, QString("FX2\nCtrl1"), -100, 100, 0); 
    f_mod_matrix_columns << new LMS_mod_matrix_column(spinbox, QString("FX2\nCtrl2"), -100, 100, 0); 
    f_mod_matrix_columns << new LMS_mod_matrix_column(spinbox, QString("FX2\nCtrl3"), -100, 100, 0);  

    f_mod_matrix_columns << new LMS_mod_matrix_column(spinbox, QString("FX3\nCtrl1"), -100, 100, 0);  
    f_mod_matrix_columns << new LMS_mod_matrix_column(spinbox, QString("FX3\nCtrl2"), -100, 100, 0);  
    f_mod_matrix_columns << new LMS_mod_matrix_column(spinbox, QString("FX3\nCtrl3"), -100, 100, 0);  

    f_mod_matrix_columns << new LMS_mod_matrix_column(spinbox, QString("FX4\nCtrl1"), -100, 100, 0);  
    f_mod_matrix_columns << new LMS_mod_matrix_column(spinbox, QString("FX4\nCtrl2"), -100, 100, 0);  
    f_mod_matrix_columns << new LMS_mod_matrix_column(spinbox, QString("FX4\nCtrl3"), -100, 100, 0);  

    m_polyfx_mod_matrix[0] = new LMS_mod_matrix(this, WAYV_MODULATOR_COUNT, f_mod_matrix_columns, LMS_PFXMATRIX_FIRST_PORT, f_info);
    m_polyfx_mod_matrix[0]->lms_mod_matrix->setMinimumHeight(165);
    m_polyfx_mod_matrix[0]->lms_mod_matrix->setMaximumHeight(165);

    m_polyfx_mod_matrix[0]->lms_mod_matrix->setVerticalHeaderLabels(QStringList() << QString("ADSR 1") << QString("ADSR 2") << QString("Ramp Env") << QString("LFO"));

    m_main_layout->lms_add_widget(m_polyfx_mod_matrix[0]->lms_mod_matrix);

    m_polyfx_mod_matrix[0]->lms_mod_matrix->resizeColumnsToContents();

    m_main_layout->lms_add_layout();  

    //Connect all ports from PolyFX mod matrix

    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[0]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst0src0ctrl0Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[1]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst0src0ctrl1Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[2]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst0src0ctrl2Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[0]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst0src1ctrl0Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[1]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst0src1ctrl1Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[2]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst0src1ctrl2Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[0]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst0src2ctrl0Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[1]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst0src2ctrl1Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[2]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst0src2ctrl2Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[0]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst0src3ctrl0Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[1]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst0src3ctrl1Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[2]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst0src3ctrl2Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[3]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst1src0ctrl0Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[4]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst1src0ctrl1Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[5]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst1src0ctrl2Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[3]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst1src1ctrl0Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[4]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst1src1ctrl1Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[5]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst1src1ctrl2Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[3]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst1src2ctrl0Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[4]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst1src2ctrl1Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[5]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst1src2ctrl2Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[3]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst1src3ctrl0Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[4]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst1src3ctrl1Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[5]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst1src3ctrl2Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[6]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst2src0ctrl0Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[7]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst2src0ctrl1Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[8]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst2src0ctrl2Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[6]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst2src1ctrl0Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[7]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst2src1ctrl1Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[8]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst2src1ctrl2Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[6]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst2src2ctrl0Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[7]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst2src2ctrl1Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[8]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst2src2ctrl2Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[6]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst2src3ctrl0Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[7]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst2src3ctrl1Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[8]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst2src3ctrl2Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[9]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst3src0ctrl0Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[10]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst3src0ctrl1Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[11]->controls[0]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst3src0ctrl2Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[9]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst3src1ctrl0Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[10]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst3src1ctrl1Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[11]->controls[1]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst3src1ctrl2Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[9]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst3src2ctrl0Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[10]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst3src2ctrl1Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[11]->controls[2]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst3src2ctrl2Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[9]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst3src3ctrl0Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[10]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst3src3ctrl1Changed(int)));
    connect((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[11]->controls[3]->lms_get_widget()), SIGNAL(valueChanged(int)), this, SLOT(pfxmatrix_grp0dst3src3ctrl2Changed(int)));

    //End new mod matrix



    //End from Modulex

    m_adsr_amp = new LMS_adsr_widget(this, f_info, TRUE, WAYV_ATTACK_PFX1, WAYV_DECAY_PFX1, WAYV_SUSTAIN_PFX1, WAYV_RELEASE_PFX1, QString("ADSR 1"));

    m_adsr_amp->lms_release->lms_knob->setMinimum(5);  //overriding the default for this, because we want a low minimum default that won't click
    m_adsr_amp->lms_sustain->lms_knob->setMinimum(-30);
    
    m_main_layout->lms_add_widget(m_adsr_amp->lms_groupbox_adsr->lms_groupbox);

    connect(m_adsr_amp->lms_attack->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(attackChanged(int)));    
    connect(m_adsr_amp->lms_decay->lms_knob,   SIGNAL(valueChanged(int)), this, SLOT(decayChanged(int)));        
    connect(m_adsr_amp->lms_sustain->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(sustainChanged(int)));        
    connect(m_adsr_amp->lms_release->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(releaseChanged(int)));    

    m_adsr_filter = new LMS_adsr_widget(this, f_info, FALSE, WAYV_ATTACK_PFX2, WAYV_DECAY_PFX2, WAYV_SUSTAIN_PFX2, WAYV_RELEASE_PFX2, QString("ADSR 2"));

    m_main_layout->lms_add_widget(m_adsr_filter->lms_groupbox_adsr->lms_groupbox);

    connect(m_adsr_filter->lms_attack->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(filterAttackChanged(int)));
    connect(m_adsr_filter->lms_decay->lms_knob,   SIGNAL(valueChanged(int)), this, SLOT(filterDecayChanged(int))); 
    connect(m_adsr_filter->lms_sustain->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(filterSustainChanged(int)));
    connect(m_adsr_filter->lms_release->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(filterReleaseChanged(int)));

    m_pitch_env = new LMS_ramp_env(this, f_info, WAYV_RAMP_ENV_TIME, RAYV_PITCH_ENV_AMT, -1, FALSE, QString("Ramp Env"), TRUE);
    m_pitch_env->lms_amt_knob->lms_label->setText(QString("Pitch"));
    m_main_layout->lms_add_widget(m_pitch_env->lms_groupbox->lms_groupbox);

    connect(m_pitch_env->lms_time_knob->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(pitchEnvTimeChanged(int)));
    connect(m_pitch_env->lms_amt_knob->lms_knob, SIGNAL(valueChanged(int)), this, SLOT(pitchEnvAmtChanged(int)));

    m_lfo = new LMS_lfo_widget(this, f_info, WAYV_LFO_FREQ, WAYV_LFO_TYPE, f_lfo_types, QString("LFO"));
    m_main_layout->lms_add_widget(m_lfo->lms_groupbox->lms_groupbox);

    //Overriding the default so we can have a faster LFO
    m_lfo->lms_freq_knob->lms_knob->setMaximum(1600);

    connect(m_lfo->lms_freq_knob->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(LFOfreqChanged(int)));
    connect(m_lfo->lms_type_combobox->lms_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(LFOtypeChanged(int)));
    
    m_lfo_amount  = new LMS_knob_regular(QString("Amount"), 0, 100, 1, 0, QString("0"), m_lfo->lms_groupbox->lms_groupbox, f_info, lms_kc_decimal, WAYV_LFO_AMOUNT);
    m_lfo->lms_groupbox->lms_add_h(m_lfo_amount);
    connect(m_lfo_amount->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(LFOamountChanged(int)));
    
    m_lfo_amp  = new LMS_knob_regular(QString("Amp"), -24, 24, 1, 0, QString("0"), m_lfo->lms_groupbox->lms_groupbox, f_info, lms_kc_integer, WAYV_LFO_AMP);
    m_lfo->lms_groupbox->lms_add_h(m_lfo_amp);
    connect(m_lfo_amp->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(LFOampChanged(int)));
    
    m_lfo_pitch = new LMS_knob_regular(QString("Pitch"), -36, 36, 1, 0, QString("0"), m_lfo->lms_groupbox->lms_groupbox, f_info, lms_kc_integer, WAYV_LFO_PITCH);
    m_lfo->lms_groupbox->lms_add_h(m_lfo_pitch);
    connect(m_lfo_pitch->lms_knob,  SIGNAL(valueChanged(int)), this, SLOT(LFOpitchChanged(int)));
    
    
    
    /*Add the knobs to the preset module*/
    
    m_program->lms_add_control(m_adsr_amp_main->lms_attack);
    m_program->lms_add_control(m_adsr_amp_main->lms_decay);
    m_program->lms_add_control(m_adsr_amp_main->lms_sustain);
    m_program->lms_add_control(m_adsr_amp_main->lms_release);    
    m_program->lms_add_control(m_adsr_amp->lms_attack);
    m_program->lms_add_control(m_adsr_amp->lms_decay);
    m_program->lms_add_control(m_adsr_amp->lms_sustain);
    m_program->lms_add_control(m_adsr_amp->lms_release);
    m_program->lms_add_control(m_adsr_filter->lms_attack);
    m_program->lms_add_control(m_adsr_filter->lms_decay);
    m_program->lms_add_control(m_adsr_filter->lms_sustain);
    m_program->lms_add_control(m_adsr_filter->lms_release);
    m_program->lms_add_control(m_adsr_amp1->lms_attack);
    m_program->lms_add_control(m_adsr_amp1->lms_decay);
    m_program->lms_add_control(m_adsr_amp1->lms_sustain);
    m_program->lms_add_control(m_adsr_amp1->lms_release);
    m_program->lms_add_control(m_adsr_amp2->lms_attack);
    m_program->lms_add_control(m_adsr_amp2->lms_decay);
    m_program->lms_add_control(m_adsr_amp2->lms_sustain);
    m_program->lms_add_control(m_adsr_amp2->lms_release);
    m_program->lms_add_control(m_lfo->lms_freq_knob);
    m_program->lms_add_control(m_lfo->lms_type_combobox);
    
    m_program->lms_add_control(m_noise_amp);
    m_program->lms_add_control(m_noise_type);
    m_program->lms_add_control(m_pitch_env->lms_time_knob);
    m_program->lms_add_control(m_osc1->lms_osc_type_box);
    m_program->lms_add_control(m_osc1->lms_pitch_knob);
    m_program->lms_add_control(m_osc1->lms_fine_knob);
    m_program->lms_add_control(m_osc1->lms_vol_knob);    
    m_program->lms_add_control(m_osc2->lms_osc_type_box);
    m_program->lms_add_control(m_osc2->lms_pitch_knob);
    m_program->lms_add_control(m_osc2->lms_fine_knob);
    m_program->lms_add_control(m_osc2->lms_vol_knob);    
    m_program->lms_add_control(m_master->lms_master_volume);
    m_program->lms_add_control(m_master->lms_master_glide);
    m_program->lms_add_control(m_master->lms_master_pitchbend_amt);
    
    m_program->lms_add_control(m_adsr_amp1_checkbox);
    m_program->lms_add_control(m_adsr_amp2_checkbox);
    
    m_program->lms_add_control(m_fx0->lms_knob1);
    m_program->lms_add_control(m_fx0->lms_knob2);
    m_program->lms_add_control(m_fx0->lms_knob3);
    m_program->lms_add_control(m_fx0->lms_combobox);
    m_program->lms_add_control(m_fx1->lms_knob1);
    m_program->lms_add_control(m_fx1->lms_knob2);
    m_program->lms_add_control(m_fx1->lms_knob3);
    m_program->lms_add_control(m_fx1->lms_combobox);
    m_program->lms_add_control(m_fx2->lms_knob1);
    m_program->lms_add_control(m_fx2->lms_knob2);
    m_program->lms_add_control(m_fx2->lms_knob3);
    m_program->lms_add_control(m_fx2->lms_combobox);
    m_program->lms_add_control(m_fx3->lms_knob1);
    m_program->lms_add_control(m_fx3->lms_knob2);
    m_program->lms_add_control(m_fx3->lms_knob3);
    m_program->lms_add_control(m_fx3->lms_combobox);
        
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[0]->controls[0]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[1]->controls[0]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[2]->controls[0]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[0]->controls[1]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[1]->controls[1]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[2]->controls[1]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[0]->controls[2]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[1]->controls[2]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[2]->controls[2]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[0]->controls[3]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[1]->controls[3]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[2]->controls[3]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[3]->controls[0]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[4]->controls[0]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[5]->controls[0]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[3]->controls[1]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[4]->controls[1]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[5]->controls[1]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[3]->controls[2]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[4]->controls[2]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[5]->controls[2]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[3]->controls[3]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[4]->controls[3]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[5]->controls[3]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[6]->controls[0]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[7]->controls[0]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[8]->controls[0]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[6]->controls[1]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[7]->controls[1]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[8]->controls[1]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[6]->controls[2]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[7]->controls[2]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[8]->controls[2]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[6]->controls[3]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[7]->controls[3]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[8]->controls[3]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[9]->controls[0]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[10]->controls[0]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[11]->controls[0]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[9]->controls[1]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[10]->controls[1]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[11]->controls[1]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[9]->controls[2]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[10]->controls[2]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[11]->controls[2]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[9]->controls[3]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[10]->controls[3]);
    m_program->lms_add_control(m_polyfx_mod_matrix[0]->lms_mm_columns[11]->controls[3]);
    
    m_program->lms_add_control(m_lfo_amp);
    m_program->lms_add_control(m_lfo_pitch);    
    m_program->lms_add_control(m_pitch_env->lms_amt_knob);
    
    m_program->lms_add_control(m_osc1_uni_voices);
    m_program->lms_add_control(m_osc1_uni_spread);
    
    m_program->lms_add_control(m_osc2_uni_voices);
    m_program->lms_add_control(m_osc2_uni_spread);
    m_program->lms_add_control(m_lfo_amount);
    
    QTimer *myTimer = new QTimer(this);
    connect(myTimer, SIGNAL(timeout()), this, SLOT(oscRecv()));
    myTimer->setSingleShot(false);
    myTimer->start(0);
    
    m_suppressHostUpdate = false;
}





void rayv_gui::pfxmatrix_Changed(int a_port, int a_fx_group, int a_dst, int a_ctrl, int a_src)
{
    //m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[a_control_index]->lms_value_changed(0);    
#ifndef LMS_DEBUG_STANDALONE
    if (!m_suppressHostUpdate) {        
	lo_send(m_host, m_controlPath, "if",
                a_port,
                //(float)(m_sample_table->lms_mm_columns[SMP_TB_VEL_HIGH_INDEX]->controls[a_control_index]->lms_get_value())
                (float)(m_polyfx_mod_matrix[a_fx_group]->lms_mm_columns[((a_dst * WAYV_CONTROLS_PER_MOD_EFFECT) + a_ctrl)]->controls[a_src]->lms_get_value())
                );
    }
#endif
}





void rayv_gui::fx0knob0Changed(int value){ lms_value_changed(value, m_fx0->lms_knob1); }
void rayv_gui::fx0knob1Changed(int value){ lms_value_changed(value, m_fx0->lms_knob2); }
void rayv_gui::fx0knob2Changed(int value){ lms_value_changed(value, m_fx0->lms_knob3); }
void rayv_gui::fx0comboboxChanged(int value){ lms_value_changed(value, m_fx0->lms_combobox); m_fx0->lms_combobox_changed(); }

void rayv_gui::fx1knob0Changed(int value){ lms_value_changed(value, m_fx1->lms_knob1); }
void rayv_gui::fx1knob1Changed(int value){ lms_value_changed(value, m_fx1->lms_knob2); }
void rayv_gui::fx1knob2Changed(int value){ lms_value_changed(value, m_fx1->lms_knob3); }
void rayv_gui::fx1comboboxChanged(int value){ lms_value_changed(value, m_fx1->lms_combobox); m_fx1->lms_combobox_changed(); }

void rayv_gui::fx2knob0Changed(int value){ lms_value_changed(value, m_fx2->lms_knob1); }
void rayv_gui::fx2knob1Changed(int value){ lms_value_changed(value, m_fx2->lms_knob2); }
void rayv_gui::fx2knob2Changed(int value){ lms_value_changed(value, m_fx2->lms_knob3); }
void rayv_gui::fx2comboboxChanged(int value){ lms_value_changed(value, m_fx2->lms_combobox); m_fx2->lms_combobox_changed(); }

void rayv_gui::fx3knob0Changed(int value){ lms_value_changed(value, m_fx3->lms_knob1); }
void rayv_gui::fx3knob1Changed(int value){ lms_value_changed(value, m_fx3->lms_knob2); }
void rayv_gui::fx3knob2Changed(int value){ lms_value_changed(value, m_fx3->lms_knob3); }
void rayv_gui::fx3comboboxChanged(int value){ lms_value_changed(value, m_fx3->lms_combobox); m_fx3->lms_combobox_changed(); }

void rayv_gui::adsr1checkChanged(bool a_value)
{
    int value = 0;
    if(a_value)
    {
        value = 1;
    }
    lms_value_changed(value, m_adsr_amp1_checkbox);
}

void rayv_gui::adsr2checkChanged(bool a_value)
{
    int value = 0;
    if(a_value)
    {
        value = 1;
    }
    lms_value_changed(value, m_adsr_amp2_checkbox);
}

void rayv_gui::pfxmatrix_grp0dst0src0ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST0SRC0CTRL0, 0, 0, 0, 0);}
void rayv_gui::pfxmatrix_grp0dst0src0ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST0SRC0CTRL1, 0, 0, 1, 0);}
void rayv_gui::pfxmatrix_grp0dst0src0ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST0SRC0CTRL2, 0, 0, 2, 0);}
void rayv_gui::pfxmatrix_grp0dst0src1ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST0SRC1CTRL0, 0, 0, 0, 1);}
void rayv_gui::pfxmatrix_grp0dst0src1ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST0SRC1CTRL1, 0, 0, 1, 1);}
void rayv_gui::pfxmatrix_grp0dst0src1ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST0SRC1CTRL2, 0, 0, 2, 1);}
void rayv_gui::pfxmatrix_grp0dst0src2ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST0SRC2CTRL0, 0, 0, 0, 2);}
void rayv_gui::pfxmatrix_grp0dst0src2ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST0SRC2CTRL1, 0, 0, 1, 2);}
void rayv_gui::pfxmatrix_grp0dst0src2ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST0SRC2CTRL2, 0, 0, 2, 2);}
void rayv_gui::pfxmatrix_grp0dst0src3ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST0SRC3CTRL0, 0, 0, 0, 3);}
void rayv_gui::pfxmatrix_grp0dst0src3ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST0SRC3CTRL1, 0, 0, 1, 3);}
void rayv_gui::pfxmatrix_grp0dst0src3ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST0SRC3CTRL2, 0, 0, 2, 3);}
void rayv_gui::pfxmatrix_grp0dst1src0ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST1SRC0CTRL0, 0, 1, 0, 0);}
void rayv_gui::pfxmatrix_grp0dst1src0ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST1SRC0CTRL1, 0, 1, 1, 0);}
void rayv_gui::pfxmatrix_grp0dst1src0ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST1SRC0CTRL2, 0, 1, 2, 0);}
void rayv_gui::pfxmatrix_grp0dst1src1ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST1SRC1CTRL0, 0, 1, 0, 1);}
void rayv_gui::pfxmatrix_grp0dst1src1ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST1SRC1CTRL1, 0, 1, 1, 1);}
void rayv_gui::pfxmatrix_grp0dst1src1ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST1SRC1CTRL2, 0, 1, 2, 1);}
void rayv_gui::pfxmatrix_grp0dst1src2ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST1SRC2CTRL0, 0, 1, 0, 2);}
void rayv_gui::pfxmatrix_grp0dst1src2ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST1SRC2CTRL1, 0, 1, 1, 2);}
void rayv_gui::pfxmatrix_grp0dst1src2ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST1SRC2CTRL2, 0, 1, 2, 2);}
void rayv_gui::pfxmatrix_grp0dst1src3ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST1SRC3CTRL0, 0, 1, 0, 3);}
void rayv_gui::pfxmatrix_grp0dst1src3ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST1SRC3CTRL1, 0, 1, 1, 3);}
void rayv_gui::pfxmatrix_grp0dst1src3ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST1SRC3CTRL2, 0, 1, 2, 3);}
void rayv_gui::pfxmatrix_grp0dst2src0ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST2SRC0CTRL0, 0, 2, 0, 0);}
void rayv_gui::pfxmatrix_grp0dst2src0ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST2SRC0CTRL1, 0, 2, 1, 0);}
void rayv_gui::pfxmatrix_grp0dst2src0ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST2SRC0CTRL2, 0, 2, 2, 0);}
void rayv_gui::pfxmatrix_grp0dst2src1ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST2SRC1CTRL0, 0, 2, 0, 1);}
void rayv_gui::pfxmatrix_grp0dst2src1ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST2SRC1CTRL1, 0, 2, 1, 1);}
void rayv_gui::pfxmatrix_grp0dst2src1ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST2SRC1CTRL2, 0, 2, 2, 1);}
void rayv_gui::pfxmatrix_grp0dst2src2ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST2SRC2CTRL0, 0, 2, 0, 2);}
void rayv_gui::pfxmatrix_grp0dst2src2ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST2SRC2CTRL1, 0, 2, 1, 2);}
void rayv_gui::pfxmatrix_grp0dst2src2ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST2SRC2CTRL2, 0, 2, 2, 2);}
void rayv_gui::pfxmatrix_grp0dst2src3ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST2SRC3CTRL0, 0, 2, 0, 3);}
void rayv_gui::pfxmatrix_grp0dst2src3ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST2SRC3CTRL1, 0, 2, 1, 3);}
void rayv_gui::pfxmatrix_grp0dst2src3ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST2SRC3CTRL2, 0, 2, 2, 3);}
void rayv_gui::pfxmatrix_grp0dst3src0ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST3SRC0CTRL0, 0, 3, 0, 0);}
void rayv_gui::pfxmatrix_grp0dst3src0ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST3SRC0CTRL1, 0, 3, 1, 0);}
void rayv_gui::pfxmatrix_grp0dst3src0ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST3SRC0CTRL2, 0, 3, 2, 0);}
void rayv_gui::pfxmatrix_grp0dst3src1ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST3SRC1CTRL0, 0, 3, 0, 1);}
void rayv_gui::pfxmatrix_grp0dst3src1ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST3SRC1CTRL1, 0, 3, 1, 1);}
void rayv_gui::pfxmatrix_grp0dst3src1ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST3SRC1CTRL2, 0, 3, 2, 1);}
void rayv_gui::pfxmatrix_grp0dst3src2ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST3SRC2CTRL0, 0, 3, 0, 2);}
void rayv_gui::pfxmatrix_grp0dst3src2ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST3SRC2CTRL1, 0, 3, 1, 2);}
void rayv_gui::pfxmatrix_grp0dst3src2ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST3SRC2CTRL2, 0, 3, 2, 2);}
void rayv_gui::pfxmatrix_grp0dst3src3ctrl0Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST3SRC3CTRL0, 0, 3, 0, 3);}
void rayv_gui::pfxmatrix_grp0dst3src3ctrl1Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST3SRC3CTRL1, 0, 3, 1, 3);}
void rayv_gui::pfxmatrix_grp0dst3src3ctrl2Changed(int a_value){pfxmatrix_Changed(LMS_PFXMATRIX_GRP0DST3SRC3CTRL2, 0, 3, 2, 3);}

void rayv_gui::LFOamountChanged(int a_value){lms_value_changed(a_value, m_lfo_amount);}
void rayv_gui::LFOampChanged(int a_value){lms_value_changed(a_value, m_lfo_amp);}
void rayv_gui::LFOpitchChanged(int a_value){lms_value_changed(a_value, m_lfo_pitch);}



void rayv_gui::lms_set_value(float val, LMS_control * a_ctrl )
{    
    m_suppressHostUpdate = true;
    a_ctrl->lms_set_value(int(val));
    m_suppressHostUpdate = false;     
}

void rayv_gui::setAttackMain(float a_value){ lms_set_value(a_value, m_adsr_amp_main->lms_attack);}
void rayv_gui::setDecayMain(float a_value){ lms_set_value(a_value, m_adsr_amp_main->lms_decay); }
void rayv_gui::setSustainMain(float a_value){lms_set_value(a_value, m_adsr_amp_main->lms_sustain);}
void rayv_gui::setReleaseMain(float a_value){lms_set_value(a_value, m_adsr_amp_main->lms_release);}

void rayv_gui::setAttack1(float a_value){ lms_set_value(a_value, m_adsr_amp1->lms_attack);}
void rayv_gui::setDecay1(float a_value){ lms_set_value(a_value, m_adsr_amp1->lms_decay); }
void rayv_gui::setSustain1(float a_value){lms_set_value(a_value, m_adsr_amp1->lms_sustain);}
void rayv_gui::setRelease1(float a_value){lms_set_value(a_value, m_adsr_amp1->lms_release);}

void rayv_gui::setAttack2(float a_value){ lms_set_value(a_value, m_adsr_amp2->lms_attack);}
void rayv_gui::setDecay2(float a_value){ lms_set_value(a_value, m_adsr_amp2->lms_decay); }
void rayv_gui::setSustain2(float a_value){lms_set_value(a_value, m_adsr_amp2->lms_sustain);}
void rayv_gui::setRelease2(float a_value){lms_set_value(a_value, m_adsr_amp2->lms_release);}

void rayv_gui::setOsc1Type(float a_value){lms_set_value(a_value, m_osc1->lms_osc_type_box);}
void rayv_gui::setOsc1Pitch(float a_value){lms_set_value(a_value, m_osc1->lms_pitch_knob);}
void rayv_gui::setOsc1Tune(float a_value){lms_set_value(a_value, m_osc1->lms_fine_knob);}
void rayv_gui::setOsc1Volume(float a_value){lms_set_value(a_value, m_osc1->lms_vol_knob);}
void rayv_gui::setOsc2Type(float a_value){lms_set_value(a_value, m_osc2->lms_osc_type_box);}
void rayv_gui::setOsc2Pitch(float a_value){lms_set_value(a_value, m_osc2->lms_pitch_knob);}
void rayv_gui::setOsc2Tune(float a_value){lms_set_value(a_value, m_osc2->lms_fine_knob);}
void rayv_gui::setOsc2Volume(float a_value){lms_set_value(a_value, m_osc2->lms_vol_knob);}
void rayv_gui::setProgram(float a_value){lms_set_value(a_value, m_program);}


void rayv_gui::setADSR1checked(float a_value){lms_set_value(a_value, m_adsr_amp1_checkbox);}
void rayv_gui::setADSR2checked(float a_value){lms_set_value(a_value, m_adsr_amp2_checkbox);}


void rayv_gui::setFX0knob0(float val){ lms_set_value(val, m_fx0->lms_knob1); }
void rayv_gui::setFX0knob1(float val){ lms_set_value(val, m_fx0->lms_knob2); }
void rayv_gui::setFX0knob2(float val){ lms_set_value(val, m_fx0->lms_knob3); }
void rayv_gui::setFX0combobox(float val){ lms_set_value(val, m_fx0->lms_combobox); }

void rayv_gui::setFX1knob0(float val){ lms_set_value(val, m_fx1->lms_knob1); }
void rayv_gui::setFX1knob1(float val){ lms_set_value(val, m_fx1->lms_knob2); }
void rayv_gui::setFX1knob2(float val){ lms_set_value(val, m_fx1->lms_knob3); }
void rayv_gui::setFX1combobox(float val){ lms_set_value(val, m_fx1->lms_combobox); }

void rayv_gui::setFX2knob0(float val){ lms_set_value(val, m_fx2->lms_knob1); }
void rayv_gui::setFX2knob1(float val){ lms_set_value(val, m_fx2->lms_knob2); }
void rayv_gui::setFX2knob2(float val){ lms_set_value(val, m_fx2->lms_knob3); }
void rayv_gui::setFX2combobox(float val){ lms_set_value(val, m_fx2->lms_combobox); }

void rayv_gui::setFX3knob0(float val){ lms_set_value(val, m_fx3->lms_knob1); }
void rayv_gui::setFX3knob1(float val){ lms_set_value(val, m_fx3->lms_knob2); }
void rayv_gui::setFX3knob2(float val){ lms_set_value(val, m_fx3->lms_knob3); }
void rayv_gui::setFX3combobox(float val){ lms_set_value(val, m_fx3->lms_combobox); }




void rayv_gui::setAttack(float a_value){ lms_set_value(a_value, m_adsr_amp->lms_attack);}
void rayv_gui::setDecay(float a_value){ lms_set_value(a_value, m_adsr_amp->lms_decay); }
void rayv_gui::setSustain(float a_value){lms_set_value(a_value, m_adsr_amp->lms_sustain);}
void rayv_gui::setRelease(float a_value){lms_set_value(a_value, m_adsr_amp->lms_release);}
void rayv_gui::setFilterAttack (float a_value){lms_set_value(a_value, m_adsr_filter->lms_attack);}
void rayv_gui::setFilterDecay  (float a_value){lms_set_value(a_value, m_adsr_filter->lms_decay);}
void rayv_gui::setFilterSustain(float a_value){lms_set_value(a_value, m_adsr_filter->lms_sustain);}
void rayv_gui::setFilterRelease(float a_value){lms_set_value(a_value, m_adsr_filter->lms_release);}
void rayv_gui::setNoiseAmp(float a_value){lms_set_value(a_value, m_noise_amp);}
void rayv_gui::setNoiseType(float a_value){lms_set_value(a_value, m_noise_type);}
void rayv_gui::setMasterVolume(float a_value){lms_set_value(a_value, m_master->lms_master_volume);}
void rayv_gui::setOsc1UnisonVoices(float a_value){lms_set_value(a_value, m_osc1_uni_voices);}
void rayv_gui::setOsc1UnisonSpread(float a_value){lms_set_value(a_value, m_osc1_uni_spread);}
void rayv_gui::setOsc2UnisonVoices(float a_value){lms_set_value(a_value, m_osc2_uni_voices);}
void rayv_gui::setOsc2UnisonSpread(float a_value){lms_set_value(a_value, m_osc2_uni_spread);}
void rayv_gui::setMasterGlide(float a_value){lms_set_value(a_value, m_master->lms_master_glide);}
void rayv_gui::setMasterPitchbendAmt(float a_value){lms_set_value(a_value, m_master->lms_master_pitchbend_amt);}
void rayv_gui::setPitchEnvTime(float a_value){lms_set_value(a_value, m_pitch_env->lms_time_knob);}
void rayv_gui::setPitchEnvAmt(float a_value){lms_set_value(a_value, m_pitch_env->lms_amt_knob);}
void rayv_gui::setLFOfreq(float a_value){lms_set_value(a_value, m_lfo->lms_freq_knob);}
void rayv_gui::setLFOtype(float a_value){lms_set_value(a_value, m_lfo->lms_type_combobox);}

void rayv_gui::setLFOamount(float a_value){lms_set_value(a_value, m_lfo_amount);}
void rayv_gui::setLFOamp(float a_value){lms_set_value(a_value, m_lfo_amp);}
void rayv_gui::setLFOpitch(float a_value){lms_set_value(a_value, m_lfo_pitch);}


void rayv_gui::lms_value_changed(int a_value, LMS_control * a_ctrl)
{    
    a_ctrl->lms_value_changed(a_value);

    if (!m_suppressHostUpdate) {
        lo_send(m_host, m_controlPath, "if", (a_ctrl->lms_port), float(a_value));
    }    
}

void rayv_gui::attackMainChanged(int a_value){lms_value_changed(a_value, m_adsr_amp_main->lms_attack);}
void rayv_gui::decayMainChanged(int a_value){lms_value_changed(a_value, m_adsr_amp_main->lms_decay);}
void rayv_gui::sustainMainChanged(int a_value){lms_value_changed(a_value, m_adsr_amp_main->lms_sustain);}
void rayv_gui::releaseMainChanged(int a_value){lms_value_changed(a_value, m_adsr_amp_main->lms_release);}

void rayv_gui::attack1Changed(int a_value){lms_value_changed(a_value, m_adsr_amp1->lms_attack);}
void rayv_gui::decay1Changed(int a_value){lms_value_changed(a_value, m_adsr_amp1->lms_decay);}
void rayv_gui::sustain1Changed(int a_value){lms_value_changed(a_value, m_adsr_amp1->lms_sustain);}
void rayv_gui::release1Changed(int a_value){lms_value_changed(a_value, m_adsr_amp1->lms_release);}

void rayv_gui::attack2Changed(int a_value){lms_value_changed(a_value, m_adsr_amp2->lms_attack);}
void rayv_gui::decay2Changed(int a_value){lms_value_changed(a_value, m_adsr_amp2->lms_decay);}
void rayv_gui::sustain2Changed(int a_value){lms_value_changed(a_value, m_adsr_amp2->lms_sustain);}
void rayv_gui::release2Changed(int a_value){lms_value_changed(a_value, m_adsr_amp2->lms_release);}

void rayv_gui::osc1TypeChanged(int a_value){lms_value_changed(a_value, m_osc1->lms_osc_type_box);}
void rayv_gui::osc1PitchChanged(int a_value){lms_value_changed(a_value, m_osc1->lms_pitch_knob);}
void rayv_gui::osc1TuneChanged(int a_value){lms_value_changed(a_value, m_osc1->lms_fine_knob);}
void rayv_gui::osc1VolumeChanged(int a_value){lms_value_changed(a_value, m_osc1->lms_vol_knob);}
void rayv_gui::osc2TypeChanged(int a_value){lms_value_changed(a_value, m_osc2->lms_osc_type_box);}
void rayv_gui::osc2PitchChanged(int a_value){lms_value_changed(a_value, m_osc2->lms_pitch_knob);}
void rayv_gui::osc2TuneChanged(int a_value){lms_value_changed(a_value, m_osc2->lms_fine_knob);}
void rayv_gui::osc2VolumeChanged(int a_value){lms_value_changed(a_value, m_osc2->lms_vol_knob);}
void rayv_gui::programChanged(int a_value){ m_program->lms_value_changed(a_value);}
void rayv_gui::programSaved(){ m_program->programSaved(); }


void rayv_gui::attackChanged(int a_value){lms_value_changed(a_value, m_adsr_amp->lms_attack);}
void rayv_gui::decayChanged(int a_value){lms_value_changed(a_value, m_adsr_amp->lms_decay);}
void rayv_gui::sustainChanged(int a_value){lms_value_changed(a_value, m_adsr_amp->lms_sustain);}
void rayv_gui::releaseChanged(int a_value){lms_value_changed(a_value, m_adsr_amp->lms_release);}
void rayv_gui::filterAttackChanged(int a_value){lms_value_changed(a_value, m_adsr_filter->lms_attack);}
void rayv_gui::filterDecayChanged(int a_value){lms_value_changed(a_value, m_adsr_filter->lms_decay);}
void rayv_gui::filterSustainChanged(int a_value){lms_value_changed(a_value, m_adsr_filter->lms_sustain);}
void rayv_gui::filterReleaseChanged(int a_value){lms_value_changed(a_value, m_adsr_filter->lms_release);}
void rayv_gui::noiseAmpChanged(int a_value){lms_value_changed(a_value, m_noise_amp);}
void rayv_gui::noise_typeChanged(int a_value){lms_value_changed(a_value, m_noise_type);}
void rayv_gui::masterVolumeChanged(int a_value){lms_value_changed(a_value, m_master->lms_master_volume);}
void rayv_gui::osc1UnisonVoicesChanged(int a_value){lms_value_changed(a_value, m_osc1_uni_voices);}
void rayv_gui::osc1UnisonSpreadChanged(int a_value){lms_value_changed(a_value, m_osc1_uni_spread);}
void rayv_gui::osc2UnisonVoicesChanged(int a_value){lms_value_changed(a_value, m_osc2_uni_voices);}
void rayv_gui::osc2UnisonSpreadChanged(int a_value){lms_value_changed(a_value, m_osc2_uni_spread);}
void rayv_gui::masterGlideChanged(int a_value){lms_value_changed(a_value, m_master->lms_master_glide);}
void rayv_gui::masterPitchbendAmtChanged(int a_value){lms_value_changed(a_value, m_master->lms_master_pitchbend_amt);}
void rayv_gui::pitchEnvTimeChanged(int a_value){lms_value_changed(a_value, m_pitch_env->lms_time_knob);}
void rayv_gui::pitchEnvAmtChanged(int a_value){lms_value_changed(a_value, m_pitch_env->lms_amt_knob);}
void rayv_gui::LFOfreqChanged(int a_value){lms_value_changed(a_value, m_lfo->lms_freq_knob);}
void rayv_gui::LFOtypeChanged(int a_value){lms_value_changed(a_value, m_lfo->lms_type_combobox);}







void rayv_gui::v_print_port_name_to_cerr(int a_port)
{
#ifdef LMS_DEBUG_MODE_QT
    switch (a_port) {
    case WAYV_ATTACK_MAIN: rayv_cerr << "LMS_ATTACK"; break;
    case WAYV_DECAY_MAIN: rayv_cerr << "LMS_DECAY"; break;
    case WAYV_SUSTAIN_MAIN: rayv_cerr << "LMS_SUSTAIN"; break;
    case WAYV_RELEASE_MAIN: rayv_cerr << "LMS_RELEASE"; break;
    case WAYV_TIMBRE: rayv_cerr << "LMS_TIMBRE"; break;
    case WAYV_RES: rayv_cerr << "LMS_RES"; break;        
    case WAYV_DIST: rayv_cerr << "LMS_DIST"; break;
    case WAYV_FILTER_ATTACK: rayv_cerr << "LMS_FILTER_ATTACK"; break;
    case WAYV_FILTER_DECAY: rayv_cerr << "LMS_FILTER_DECAY"; break;
    case WAYV_FILTER_SUSTAIN: rayv_cerr << "LMS_FILTER_SUSTAIN"; break;
    case WAYV_FILTER_RELEASE: rayv_cerr << "LMS_FILTER_RELEASE"; break;
    case WAYV_NOISE_AMP: rayv_cerr << "LMS_NOISE_AMP"; break;    
    case WAYV_DIST_WET: rayv_cerr << "LMS_DIST_WET"; break;            
    case WAYV_FILTER_ENV_AMT: rayv_cerr << "LMS_FILTER_ENV_AMT"; break;    
    case WAYV_OSC1_TYPE: rayv_cerr << "LMS_OSC1_TYPE"; break;            
    case WAYV_OSC1_PITCH: rayv_cerr << "LMS_OSC1_PITCH"; break;    
    case WAYV_OSC1_TUNE: rayv_cerr << "LMS_OSC1_TUNE"; break;    
    case WAYV_OSC1_VOLUME: rayv_cerr << "LMS_OSC1_VOLUME"; break;        
    case WAYV_OSC2_TYPE: rayv_cerr << "LMS_OSC2_TYPE"; break;            
    case WAYV_OSC2_PITCH: rayv_cerr << "LMS_OSC2_PITCH"; break;    
    case WAYV_OSC2_TUNE: rayv_cerr << "LMS_OSC2_TUNE";  break;    
    case WAYV_OSC2_VOLUME: rayv_cerr << "LMS_OSC2_VOLUME"; break;        
    case WAYV_MASTER_VOLUME: rayv_cerr << "LMS_MASTER_VOLUME"; break;
    case WAYV_OSC1_UNISON_VOICES: rayv_cerr << "LMS_MASTER_UNISON_VOICES"; break;
    case WAYV_OSC1_UNISON_SPREAD: rayv_cerr << "LMS_MASTER_UNISON_SPREAD"; break;
    case WAYV_MASTER_GLIDE: rayv_cerr << "LMS_MASTER_GLIDE"; break;
    case WAYV_MASTER_PITCHBEND_AMT: rayv_cerr << "LMS_MASTER_PITCHBEND_AMT"; break;
    case WAYV_PITCH_ENV_AMT: rayv_cerr << "LMS_PITCH_ENV_AMT "; break;
    case WAYV_PITCH_ENV_TIME: rayv_cerr << "LMS_PITCH_ENV_TIME ";  break;        
    case WAYV_PROGRAM_CHANGE: rayv_cerr << "LMS_PROGRAM_CHANGE "; break;
    default: rayv_cerr << "Warning: received request to set nonexistent port " << a_port ; break;
    }
#endif
}

void rayv_gui::v_set_control(int a_port, float a_value)
{

#ifdef LMS_DEBUG_MODE_QT    
    rayv_cerr << "v_set_control called.  ";  
    v_print_port_name_to_cerr(a_port);
    rayv_cerr << "  value: " << a_value << endl;
#endif
        
    switch (a_port) {
        case WAYV_ATTACK_MAIN: setAttackMain(a_value); break;
        case WAYV_DECAY_MAIN: setDecayMain(a_value); break;
        case WAYV_SUSTAIN_MAIN: setSustainMain(a_value); break;
        case WAYV_RELEASE_MAIN: setReleaseMain(a_value); break;
        
        case WAYV_ATTACK1: setAttack1(a_value); break;
        case WAYV_DECAY1: setDecay1(a_value); break;
        case WAYV_SUSTAIN1: setSustain1(a_value); break;
        case WAYV_RELEASE1: setRelease1(a_value); break;
        
        case WAYV_ATTACK2: setAttack2(a_value); break;
        case WAYV_DECAY2: setDecay2(a_value); break;
        case WAYV_SUSTAIN2: setSustain2(a_value); break;
        case WAYV_RELEASE2: setRelease2(a_value); break;
        
        case WAYV_NOISE_AMP: setNoiseAmp(a_value); break;
        case WAYV_OSC1_TYPE: setOsc1Type(a_value); break;            
        case WAYV_OSC1_PITCH: setOsc1Pitch(a_value);  break;    
        case WAYV_OSC1_TUNE: setOsc1Tune(a_value); break;    
        case WAYV_OSC1_VOLUME: setOsc1Volume(a_value); break;        
        case WAYV_OSC2_TYPE: setOsc2Type(a_value); break;            
        case WAYV_OSC2_PITCH: setOsc2Pitch(a_value); break;    
        case WAYV_OSC2_TUNE: setOsc2Tune(a_value); break;    
        case WAYV_OSC2_VOLUME: setOsc2Volume(a_value); break;        
        case WAYV_MASTER_VOLUME: setMasterVolume(a_value); break;    
        case WAYV_OSC1_UNISON_VOICES: setOsc1UnisonVoices(a_value); break;
        case WAYV_OSC1_UNISON_SPREAD: setOsc1UnisonSpread(a_value); break;
        case WAYV_OSC2_UNISON_VOICES: setOsc2UnisonVoices(a_value); break;
        case WAYV_OSC2_UNISON_SPREAD: setOsc2UnisonSpread(a_value); break;
        case WAYV_MASTER_GLIDE: setMasterGlide(a_value); break;
        case WAYV_MASTER_PITCHBEND_AMT: setMasterPitchbendAmt(a_value); break;
        
        case WAYV_ATTACK_PFX1: setAttack(a_value); break;
        case WAYV_DECAY_PFX1: setDecay(a_value); break;
        case WAYV_SUSTAIN_PFX1: setSustain(a_value); break;
        case WAYV_RELEASE_PFX1: setRelease(a_value); break;
        case WAYV_ATTACK_PFX2: setFilterAttack(a_value); break;
        case WAYV_DECAY_PFX2: setFilterDecay(a_value); break;
        case WAYV_SUSTAIN_PFX2: setFilterSustain(a_value); break;
        case WAYV_RELEASE_PFX2: setFilterRelease(a_value); break;        
        case LMS_NOISE_TYPE: setNoiseType(a_value); break;       
        case WAYV_RAMP_ENV_TIME: setPitchEnvTime(a_value); break;                
        case WAYV_LFO_FREQ: setLFOfreq(a_value); break;            
        case WAYV_LFO_TYPE:  setLFOtype(a_value);  break;
        //From Modulex            
        case WAYV_FX0_KNOB0:	setFX0knob0(a_value); break;
        case WAYV_FX0_KNOB1:	setFX0knob1(a_value); break;        
        case WAYV_FX0_KNOB2:	setFX0knob2(a_value); break;        
        case WAYV_FX0_COMBOBOX: setFX0combobox(a_value); break;

        case WAYV_FX1_KNOB0:	setFX1knob0(a_value); break;
        case WAYV_FX1_KNOB1:	setFX1knob1(a_value); break;        
        case WAYV_FX1_KNOB2:	setFX1knob2(a_value); break;        
        case WAYV_FX1_COMBOBOX: setFX1combobox(a_value); break;

        case WAYV_FX2_KNOB0:	setFX2knob0(a_value); break;
        case WAYV_FX2_KNOB1:	setFX2knob1(a_value); break;        
        case WAYV_FX2_KNOB2:	setFX2knob2(a_value); break;        
        case WAYV_FX2_COMBOBOX: setFX2combobox(a_value); break;

        case WAYV_FX3_KNOB0:	setFX3knob0(a_value); break;
        case WAYV_FX3_KNOB1:	setFX3knob1(a_value); break;        
        case WAYV_FX3_KNOB2:	setFX3knob2(a_value); break;        
        case WAYV_FX3_COMBOBOX: setFX3combobox(a_value); break;
        //End from Modulex            
        //From PolyFX mod matrix
        case LMS_PFXMATRIX_GRP0DST0SRC0CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[0]->controls[0]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST0SRC0CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[1]->controls[0]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST0SRC0CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[2]->controls[0]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST0SRC1CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[0]->controls[1]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST0SRC1CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[1]->controls[1]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST0SRC1CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[2]->controls[1]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST0SRC2CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[0]->controls[2]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST0SRC2CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[1]->controls[2]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST0SRC2CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[2]->controls[2]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST0SRC3CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[0]->controls[3]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST0SRC3CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[1]->controls[3]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST0SRC3CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[2]->controls[3]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST1SRC0CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[3]->controls[0]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST1SRC0CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[4]->controls[0]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST1SRC0CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[5]->controls[0]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST1SRC1CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[3]->controls[1]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST1SRC1CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[4]->controls[1]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST1SRC1CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[5]->controls[1]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST1SRC2CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[3]->controls[2]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST1SRC2CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[4]->controls[2]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST1SRC2CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[5]->controls[2]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST1SRC3CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[3]->controls[3]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST1SRC3CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[4]->controls[3]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST1SRC3CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[5]->controls[3]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST2SRC0CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[6]->controls[0]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST2SRC0CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[7]->controls[0]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST2SRC0CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[8]->controls[0]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST2SRC1CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[6]->controls[1]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST2SRC1CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[7]->controls[1]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST2SRC1CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[8]->controls[1]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST2SRC2CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[6]->controls[2]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST2SRC2CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[7]->controls[2]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST2SRC2CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[8]->controls[2]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST2SRC3CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[6]->controls[3]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST2SRC3CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[7]->controls[3]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST2SRC3CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[8]->controls[3]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST3SRC0CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[9]->controls[0]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST3SRC0CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[10]->controls[0]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST3SRC0CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[11]->controls[0]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST3SRC1CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[9]->controls[1]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST3SRC1CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[10]->controls[1]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST3SRC1CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[11]->controls[1]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST3SRC2CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[9]->controls[2]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST3SRC2CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[10]->controls[2]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST3SRC2CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[11]->controls[2]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST3SRC3CTRL0: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[9]->controls[3]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST3SRC3CTRL1: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[10]->controls[3]->lms_get_widget()))->setValue(a_value); break;
        case LMS_PFXMATRIX_GRP0DST3SRC3CTRL2: ((QSpinBox*)(m_polyfx_mod_matrix[0]->lms_mm_columns[11]->controls[3]->lms_get_widget()))->setValue(a_value); break;

        case WAYV_LFO_AMOUNT: setLFOamount(a_value); break;            
        case WAYV_LFO_AMP: setLFOamp(a_value); break;            
        case WAYV_LFO_PITCH: setLFOpitch(a_value); break;      
        
        case RAYV_PITCH_ENV_AMT: setPitchEnvAmt(a_value); break;
    }
    
}

void rayv_gui::v_control_changed(int a_port, int a_value, bool a_suppress_host_update)
{
    
#ifdef LMS_DEBUG_MODE_QT    
    rayv_cerr << "v_control_changed called.  ";  
    v_print_port_name_to_cerr(a_port);
    rayv_cerr << "  value: " << a_value << endl;
#endif
    
    if(a_suppress_host_update)
        m_suppressHostUpdate = true;      
    
    switch (a_port) {
    case WAYV_ATTACK_MAIN: attackMainChanged(a_value); break;
    case WAYV_DECAY_MAIN: decayMainChanged(a_value); break;
    case WAYV_SUSTAIN_MAIN: sustainMainChanged(a_value); break;
    case WAYV_RELEASE_MAIN: releaseMainChanged(a_value); break;
    
    case WAYV_ATTACK1: attack1Changed(a_value); break;
    case WAYV_DECAY1: decay1Changed(a_value); break;
    case WAYV_SUSTAIN1: sustain1Changed(a_value); break;
    case WAYV_RELEASE1: release1Changed(a_value); break;
    
    case WAYV_ATTACK2: attack2Changed(a_value); break;
    case WAYV_DECAY2: decay2Changed(a_value); break;
    case WAYV_SUSTAIN2: sustain2Changed(a_value); break;
    case WAYV_RELEASE2: release2Changed(a_value); break;
    
    case WAYV_NOISE_AMP: noiseAmpChanged(a_value); break;    
    case WAYV_OSC1_TYPE: osc1TypeChanged(a_value);  break;            
    case WAYV_OSC1_PITCH:  osc1PitchChanged(a_value);  break;    
    case WAYV_OSC1_TUNE: osc1TuneChanged(a_value); break;    
    case WAYV_OSC1_VOLUME: osc1VolumeChanged(a_value); break;
    case WAYV_OSC2_TYPE: osc2TypeChanged(a_value); break;            
    case WAYV_OSC2_PITCH: osc2PitchChanged(a_value); break;    
    case WAYV_OSC2_TUNE: osc2TuneChanged(a_value); break;    
    case WAYV_OSC2_VOLUME: osc2VolumeChanged(a_value); break;
    case WAYV_MASTER_VOLUME: masterVolumeChanged(a_value); break;
    case WAYV_OSC1_UNISON_VOICES: osc1UnisonVoicesChanged(a_value); break;
    case WAYV_OSC1_UNISON_SPREAD: osc1UnisonSpreadChanged(a_value); break;
    case WAYV_OSC2_UNISON_VOICES: osc2UnisonVoicesChanged(a_value); break;
    case WAYV_OSC2_UNISON_SPREAD: osc2UnisonSpreadChanged(a_value); break;
    case WAYV_MASTER_GLIDE: masterGlideChanged(a_value); break;
    case WAYV_MASTER_PITCHBEND_AMT: masterPitchbendAmtChanged(a_value); break;
    
    case WAYV_ATTACK_PFX1: attackChanged(a_value); break;
    case WAYV_DECAY_PFX1: decayChanged(a_value); break;
    case WAYV_SUSTAIN_PFX1: sustainChanged(a_value); break;
    case WAYV_RELEASE_PFX1: releaseChanged(a_value); break;
    case WAYV_ATTACK_PFX2: filterAttackChanged(a_value); break;
    case WAYV_DECAY_PFX2: filterDecayChanged(a_value); break;
    case WAYV_SUSTAIN_PFX2: filterSustainChanged(a_value); break;
    case WAYV_RELEASE_PFX2: filterReleaseChanged(a_value); break;    
    case LMS_NOISE_TYPE: noise_typeChanged(a_value); break;             
    case WAYV_RAMP_ENV_TIME: pitchEnvTimeChanged(a_value); break;
    case WAYV_LFO_FREQ: LFOfreqChanged(a_value); break;
    case WAYV_LFO_TYPE: LFOtypeChanged(a_value); break;
    //From Modulex            
    case WAYV_FX0_KNOB0:	fx0knob0Changed(a_value); break;
    case WAYV_FX0_KNOB1:	fx0knob1Changed(a_value); break;
    case WAYV_FX0_KNOB2:	fx0knob2Changed(a_value); break;  
    case WAYV_FX0_COMBOBOX:  fx0comboboxChanged(a_value); break;

    case WAYV_FX1_KNOB0:	fx1knob0Changed(a_value); break;
    case WAYV_FX1_KNOB1:	fx1knob1Changed(a_value); break;
    case WAYV_FX1_KNOB2:	fx1knob2Changed(a_value); break;  
    case WAYV_FX1_COMBOBOX:  fx1comboboxChanged(a_value); break;

    case WAYV_FX2_KNOB0:	fx2knob0Changed(a_value); break;
    case WAYV_FX2_KNOB1:	fx2knob1Changed(a_value); break;
    case WAYV_FX2_KNOB2:	fx2knob2Changed(a_value); break;  
    case WAYV_FX2_COMBOBOX:  fx2comboboxChanged(a_value); break;

    case WAYV_FX3_KNOB0:	fx3knob0Changed(a_value); break;
    case WAYV_FX3_KNOB1:	fx3knob1Changed(a_value); break;
    case WAYV_FX3_KNOB2:	fx3knob2Changed(a_value); break;  
    case WAYV_FX3_COMBOBOX:  fx3comboboxChanged(a_value); break;
    //End from Modulex
    //From PolyFX mod matrix
    case LMS_PFXMATRIX_GRP0DST0SRC0CTRL0:  pfxmatrix_grp0dst0src0ctrl0Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST0SRC0CTRL1:  pfxmatrix_grp0dst0src0ctrl1Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST0SRC0CTRL2:  pfxmatrix_grp0dst0src0ctrl2Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST0SRC1CTRL0:  pfxmatrix_grp0dst0src1ctrl0Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST0SRC1CTRL1:  pfxmatrix_grp0dst0src1ctrl1Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST0SRC1CTRL2:  pfxmatrix_grp0dst0src1ctrl2Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST0SRC2CTRL0:  pfxmatrix_grp0dst0src2ctrl0Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST0SRC2CTRL1:  pfxmatrix_grp0dst0src2ctrl1Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST0SRC2CTRL2:  pfxmatrix_grp0dst0src2ctrl2Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST0SRC3CTRL0:  pfxmatrix_grp0dst0src3ctrl0Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST0SRC3CTRL1:  pfxmatrix_grp0dst0src3ctrl1Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST0SRC3CTRL2:  pfxmatrix_grp0dst0src3ctrl2Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST1SRC0CTRL0:  pfxmatrix_grp0dst1src0ctrl0Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST1SRC0CTRL1:  pfxmatrix_grp0dst1src0ctrl1Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST1SRC0CTRL2:  pfxmatrix_grp0dst1src0ctrl2Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST1SRC1CTRL0:  pfxmatrix_grp0dst1src1ctrl0Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST1SRC1CTRL1:  pfxmatrix_grp0dst1src1ctrl1Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST1SRC1CTRL2:  pfxmatrix_grp0dst1src1ctrl2Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST1SRC2CTRL0:  pfxmatrix_grp0dst1src2ctrl0Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST1SRC2CTRL1:  pfxmatrix_grp0dst1src2ctrl1Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST1SRC2CTRL2:  pfxmatrix_grp0dst1src2ctrl2Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST1SRC3CTRL0:  pfxmatrix_grp0dst1src3ctrl0Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST1SRC3CTRL1:  pfxmatrix_grp0dst1src3ctrl1Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST1SRC3CTRL2:  pfxmatrix_grp0dst1src3ctrl2Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST2SRC0CTRL0:  pfxmatrix_grp0dst2src0ctrl0Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST2SRC0CTRL1:  pfxmatrix_grp0dst2src0ctrl1Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST2SRC0CTRL2:  pfxmatrix_grp0dst2src0ctrl2Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST2SRC1CTRL0:  pfxmatrix_grp0dst2src1ctrl0Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST2SRC1CTRL1:  pfxmatrix_grp0dst2src1ctrl1Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST2SRC1CTRL2:  pfxmatrix_grp0dst2src1ctrl2Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST2SRC2CTRL0:  pfxmatrix_grp0dst2src2ctrl0Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST2SRC2CTRL1:  pfxmatrix_grp0dst2src2ctrl1Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST2SRC2CTRL2:  pfxmatrix_grp0dst2src2ctrl2Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST2SRC3CTRL0:  pfxmatrix_grp0dst2src3ctrl0Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST2SRC3CTRL1:  pfxmatrix_grp0dst2src3ctrl1Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST2SRC3CTRL2:  pfxmatrix_grp0dst2src3ctrl2Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST3SRC0CTRL0:  pfxmatrix_grp0dst3src0ctrl0Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST3SRC0CTRL1:  pfxmatrix_grp0dst3src0ctrl1Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST3SRC0CTRL2:  pfxmatrix_grp0dst3src0ctrl2Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST3SRC1CTRL0:  pfxmatrix_grp0dst3src1ctrl0Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST3SRC1CTRL1:  pfxmatrix_grp0dst3src1ctrl1Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST3SRC1CTRL2:  pfxmatrix_grp0dst3src1ctrl2Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST3SRC2CTRL0:  pfxmatrix_grp0dst3src2ctrl0Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST3SRC2CTRL1:  pfxmatrix_grp0dst3src2ctrl1Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST3SRC2CTRL2:  pfxmatrix_grp0dst3src2ctrl2Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST3SRC3CTRL0:  pfxmatrix_grp0dst3src3ctrl0Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST3SRC3CTRL1:  pfxmatrix_grp0dst3src3ctrl1Changed(a_value); break;
    case LMS_PFXMATRIX_GRP0DST3SRC3CTRL2:  pfxmatrix_grp0dst3src3ctrl2Changed(a_value); break;

    case WAYV_LFO_AMOUNT: LFOamountChanged(a_value); break;
    case WAYV_LFO_AMP: LFOampChanged(a_value); break;
    case WAYV_LFO_PITCH: LFOpitchChanged(a_value); break;    
    case RAYV_PITCH_ENV_AMT: pitchEnvAmtChanged(a_value); break;
    
    default:
#ifdef LMS_DEBUG_MODE_QT
	rayv_cerr << "Warning: received request to set nonexistent port " << a_port << endl;
#endif
        break;
    }
    
    if(a_suppress_host_update)
        m_suppressHostUpdate = false;
    
}

/*TODO:  For the forseeable future, this will only be used for getting the values to write back to 
 the presets.tsv file;  It should probably return a string that can be re-interpreted into other values for
 complex controls that could have multiple ints, or string values, etc...*/
int rayv_gui::i_get_control(int a_port)
{
        /*Add the controls you created to the control handler*/
    
    switch (a_port) {
    case WAYV_ATTACK_MAIN: return  m_adsr_amp_main->lms_attack->lms_get_value();
    case WAYV_DECAY_MAIN:  return m_adsr_amp_main->lms_decay->lms_get_value();
    case WAYV_SUSTAIN_MAIN: return m_adsr_amp_main->lms_sustain->lms_get_value();
    case WAYV_RELEASE_MAIN: return m_adsr_amp_main->lms_release->lms_get_value();
    case WAYV_NOISE_AMP: return m_noise_amp->lms_get_value();
    case WAYV_OSC1_TYPE: return m_osc1->lms_osc_type_box->lms_get_value();
    case WAYV_OSC1_PITCH: return m_osc1->lms_pitch_knob->lms_get_value();
    case WAYV_OSC1_TUNE: return  m_osc1->lms_fine_knob->lms_get_value();
    case WAYV_OSC1_VOLUME: return m_osc1->lms_vol_knob->lms_get_value();
    case WAYV_OSC2_TYPE:  return m_osc2->lms_osc_type_box->lms_get_value();
    case WAYV_OSC2_PITCH: return m_osc2->lms_pitch_knob->lms_get_value();
    case WAYV_OSC2_TUNE: return m_osc2->lms_fine_knob->lms_get_value();
    case WAYV_OSC2_VOLUME: return m_osc2->lms_vol_knob->lms_get_value();
    case WAYV_MASTER_VOLUME: return m_master->lms_master_volume->lms_get_value();
    case WAYV_OSC1_UNISON_VOICES: return m_osc1_uni_voices->lms_get_value();
    case WAYV_OSC1_UNISON_SPREAD: return m_osc1_uni_spread->lms_get_value();
    case WAYV_OSC2_UNISON_VOICES: return m_osc2_uni_voices->lms_get_value();
    case WAYV_OSC2_UNISON_SPREAD: return m_osc2_uni_spread->lms_get_value();
    case WAYV_MASTER_GLIDE: return m_master->lms_master_glide->lms_get_value();
    case WAYV_MASTER_PITCHBEND_AMT: return m_master->lms_master_pitchbend_amt->lms_get_value();
    
    case WAYV_LFO_AMOUNT: return m_lfo_amount->lms_get_value();
    case WAYV_LFO_AMP: return m_lfo_amp->lms_get_value();
    case WAYV_LFO_PITCH: return m_lfo_pitch->lms_get_value();
    case RAYV_PITCH_ENV_AMT: return m_pitch_env->lms_amt_knob->lms_get_value();
    //case LMS_PROGRAM_CHANGE:
        //return m_program->currentIndex();
    default:
#ifdef LMS_DEBUG_MODE_QT
	rayv_cerr << "Warning: received request to get nonexistent port " << a_port << endl;
#endif
        return 0;
        break;
    }    
}

void rayv_gui::oscRecv()
{
    if (osc_server) {
	lo_server_recv_noblock(osc_server, 1);
    }
}

void rayv_gui::aboutToQuit()
{
    if (!m_hostRequestedQuit) lo_send(m_host, m_exitingPath, "");
}

rayv_gui::~rayv_gui()
{
    lo_address_free(m_host);
}


void rayv_osc_error(int num, const char *msg, const char *path)
{
#ifdef LMS_DEBUG_MODE_QT
    rayv_cerr << "Error: liblo server error " << num
	 << " in path \"" << (path ? path : "(null)")
	 << "\": " << msg << endl;
#endif
}

int rayv_debug_handler(const char *path, const char *types, lo_arg **argv,
	      int argc, void *data, void *user_data)
{
    int i;
#ifdef LMS_DEBUG_MODE_QT
      rayv_cerr << "Warning: unhandled OSC message in GUI:" << endl;
#endif
    

    for (i = 0; i < argc; ++i) {
#ifdef LMS_DEBUG_MODE_QT
	rayv_cerr << "arg " << i << ": type '" << types[i] << "': ";
#endif
        lo_arg_pp((lo_type)types[i], argv[i]);
#ifdef LMS_DEBUG_MODE_QT
	rayv_cerr << endl;
#endif
    }
#ifdef LMS_DEBUG_MODE_QT
    rayv_cerr << "(path is <" << path << ">)" << endl;
#endif
    return 1;
}

int rayv_program_handler(const char *path, const char *types, lo_arg **argv,
	       int argc, void *data, void *user_data)
{
    rayv_gui *gui = static_cast<rayv_gui *>(user_data);
    
    //int bank = 0;
    int program = 0;

    /*
    if (argc < 2) {
        //GDB_MESSAGE(GDB_OSC, " error: too few arguments to osc_program_handler\n");
        return 1;
    }

    
    bank = argv[0]->i;
    program = argv[1]->i;

    if (bank || program < 0 || program >= 128) {        
        return 0;
    }
    */
#ifdef LMS_DEBUG_MODE_QT
    rayv_cerr << "Bank:  " << bank << ", Program:  " << program << endl;
#endif
    gui->setProgram(program);

    return 0;
}

int rayv_configure_handler(const char *path, const char *types, lo_arg **argv,
		  int argc, void *data, void *user_data)
{
    //rayv_gui *gui = static_cast<rayv_gui *>(user_data);
    const char *key = (const char *)&argv[0]->s;
    const char *value = (const char *)&argv[1]->s;

    rayv_cerr << "GUI configure_handler:  Key:  " << QString::fromLocal8Bit(key) << " , Value:" << QString::fromLocal8Bit(value);
    
    return 0;
}

int rayv_rate_handler(const char *path, const char *types, lo_arg **argv,
	     int argc, void *data, void *user_data)
{
    return 0; /* ignore it */
}

int rayv_show_handler(const char *path, const char *types, lo_arg **argv,
	     int argc, void *data, void *user_data)
{
    rayv_gui *gui = static_cast<rayv_gui *>(user_data);
    while (!gui->ready()) sleep(1);
    if (gui->isVisible()) gui->raise();
    else gui->show();
    return 0;
}

int rayv_hide_handler(const char *path, const char *types, lo_arg **argv,
	     int argc, void *data, void *user_data)
{
    rayv_gui *gui = static_cast<rayv_gui *>(user_data);
    gui->hide();
    return 0;
}

int rayv_quit_handler(const char *path, const char *types, lo_arg **argv,
	     int argc, void *data, void *user_data)
{
    rayv_gui *gui = static_cast<rayv_gui *>(user_data);
    gui->setHostRequestedQuit(true);
    qApp->quit();
    return 0;
}

int rayv_control_handler(const char *path, const char *types, lo_arg **argv,
		int argc, void *data, void *user_data)
{
    rayv_gui *gui = static_cast<rayv_gui *>(user_data);

    if (argc < 2) {
        
#ifdef LMS_DEBUG_MODE_QT
	rayv_cerr << "Error: too few arguments to control_handler" << endl;
#endif
	return 1;
    }

    const int port = argv[0]->i;
    const float value = argv[1]->f;

#ifdef LMS_DEBUG_MODE_QT
    rayv_cerr << "control_handler called.  port:  " << port << " , value " << value << endl;
#endif

    gui->v_set_control(port, value);  
     
    //gui->v_control_changed(port, value, true);

    return 0;
}

int main(int argc, char **argv)
{
    for(int i = 0; i < argc; i++)
    {
        rayv_cerr << "argv[" << i << "] == " << argv[i] << "\n";
    }

    rayv_cerr << "Qt GUI main() called..." << endl;

    
    QApplication application(argc, argv);

    if (application.argc() < 5)
    {

	rayv_cerr << "usage: "
	     << application.argv()[0] 
	     << " <osc url>"
	     << " <plugin dllname>"
	     << " <plugin label>"
	     << " <user-friendly id>"
	     << endl;

	return 2;
    }

#ifdef Q_WS_X11
    XSetErrorHandler(handle_x11_error);
#endif

    char *url = application.argv()[1];

    char *host = lo_url_get_hostname(url);
    char *port = lo_url_get_port(url);
    char *path = lo_url_get_path(url);
    
    rayv_cerr << "host: " << host << " port: " << port << " path: " << path << "\n";
        
    rayv_cerr << QString("argc==") << QString::number(argc) << QString("\n");
    

    rayv_gui gui(host, port,
		 QByteArray(path) + "/control",
		 QByteArray(path) + "/midi",
		 QByteArray(path) + "/program",
		 QByteArray(path) + "/exiting",
		 0);
 
    QByteArray myControlPath = QByteArray(path) + "/control";
    QByteArray myProgramPath = QByteArray(path) + "/program";
    QByteArray myConfigurePath = QByteArray(path) + "/configure";
    QByteArray myRatePath = QByteArray(path) + "/sample-rate";
    QByteArray myShowPath = QByteArray(path) + "/show";
    QByteArray myHidePath = QByteArray(path) + "/hide";
    QByteArray myQuitPath = QByteArray(path) + "/quit";

    rayv_cerr << "Adding lo server methods" << endl;

    osc_server = lo_server_new(NULL, rayv_osc_error);
    lo_server_add_method(osc_server, myControlPath, "if", rayv_control_handler, &gui);
    lo_server_add_method(osc_server, myProgramPath, "ii", rayv_program_handler, &gui);
    lo_server_add_method(osc_server, myConfigurePath, "ss", rayv_configure_handler, &gui);
    lo_server_add_method(osc_server, myRatePath, "i", rayv_rate_handler, &gui);
    lo_server_add_method(osc_server, myShowPath, "", rayv_show_handler, &gui);
    lo_server_add_method(osc_server, myHidePath, "", rayv_hide_handler, &gui);
    lo_server_add_method(osc_server, myQuitPath, "", rayv_quit_handler, &gui);
    lo_server_add_method(osc_server, NULL, NULL, rayv_debug_handler, &gui);

    lo_address hostaddr = lo_address_new(host, port);
    lo_send(hostaddr,
	    QByteArray(path) + "/update",
	    "s",
	    (QByteArray(lo_server_get_url(osc_server))+QByteArray(path+1)).data());

    QObject::connect(&application, SIGNAL(aboutToQuit()), &gui, SLOT(aboutToQuit()));

    gui.setWindowTitle(QString("PyDAW - Way-V - ") + application.argv()[3]);
    gui.setReady(true);
    
    rayv_cerr << "Starting GUI now..." << endl;
    
    return application.exec();
}

