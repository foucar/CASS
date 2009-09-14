// Copyright (C) 2009 Jochen Küpper
// 10/9/2009 added call to main_root...

#include <QtGui/QApplication>
#include <QtCore/QTextCodec>
#include <QtCore/QTranslator>
#include "diode.h"
#include "MainWindow.h"
//#include "circ_root.h"

int main(int argc, char**argv)
{
    // create application and event loop
    QApplication app(argc, argv);
    // use translations if available
    // as a side-effect, accept utf-8 strings to tr()
    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));
    QString locale = QLocale::system().name();
    QTranslator translator;
    translator.load(QString("diode_") + locale);
    app.installTranslator(&translator);
    // create MainWindow
    MainWindow mainwindow;
    mainwindow.show();
    // and run application
    return app.exec();
}



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
