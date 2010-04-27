#include <QApplication>
#include <QSettings>

#include "imageviewer.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("CFEL-ASG");
    QCoreApplication::setOrganizationDomain("endstation.asg.cfel.de");
    QCoreApplication::setApplicationName("IMAGEVIEWER");
    QSettings::setDefaultFormat(QSettings::IniFormat);

    ImageViewer window;
    window.show();
    return app.exec();
}
