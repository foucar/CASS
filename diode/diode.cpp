// Copyright (C) 2009 Jochen Küpper

#include <QtGui/QApplication>
#include "diode.h"
#include "MainWindow.h"

int main(int argc, char**argv)
{
    QApplication app(argc, argv);
    MainWindow mainwindow;
    mainwindow.show();
    return app.exec();
}



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
