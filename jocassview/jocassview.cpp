#include <QApplication>
#include <QSettings>

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
