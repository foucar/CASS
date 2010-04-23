#include <QApplication>

#include "imageviewer.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    ImageViewer window;
    window.show();
    return app.exec();
}
