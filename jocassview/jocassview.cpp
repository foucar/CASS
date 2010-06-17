// Copyright (C) 2010 Uwe Hoppe
// Copyright (C) 2010 Jochen Küpper
// Copyright (C) 2010 Nicola Coppola

#include <QtCore/QSettings>
#include <QtGui/QApplication>
#include <QDesktopWidget>

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

    QDesktopWidget *my_desktop= app.desktop();
    const QRect displ(my_desktop->availableGeometry(-1));
    const int displ_height= displ.height();
    const int displ_width= displ.width();
    const int min_size = std::min(displ_height,displ_width);
    const int nscreens= my_desktop->screenCount();
    const QRect screen(my_desktop->screenGeometry(-1));
    const int screen_height= screen.height();
    const int screen_width= screen.width();
    std::cout<<"Display dimensions: "<< displ_height << " x " 
             << displ_width << " " << nscreens
             << " "
             << screen_height << " x " 
             << screen_width << " " <<  my_desktop->isVirtualDesktop() <<std::endl;
    window.resize(min_size/2,min_size/2);

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
