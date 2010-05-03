// Copyright (C) 2010 Uwe Hoppe
// Copyright (C) 2010 Jochen Küpper

#include <QtCore/QSettings>
#include <QtGui/QApplication>

#include "imageviewer.h"

using namespace jocassview;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("CFEL-ASG");
    QCoreApplication::setOrganizationDomain("endstation.asg.cfel.de");
    QCoreApplication::setApplicationName("jocassview");
    QSettings::setDefaultFormat(QSettings::IniFormat);
    ImageViewer window;
    window.show();
    return app.exec();
}



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
