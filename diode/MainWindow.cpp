// Copyright (C) 2009 Jochen Küpper

#include <QtGui/QCloseEvent>
#include <QtGui/QMessageBox>
#include "MainWindow.h"



MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
{
    setupUi(this);
    // connect Actions
    QObject::connect(this->actionAbout_Diode, SIGNAL(triggered()), this, SLOT(about()));
    QObject::connect(this->actionAbout_Qt, SIGNAL(triggered()), this, SLOT(aboutQt()));
    QObject::connect(this->actionQuit_Diode, SIGNAL(triggered()), this, SLOT(quit()));
    QObject::connect(this->actionStart_ROOTb, SIGNAL(triggered()), this, SLOT(startrOotb()));
    // the following is not an action....
    QObject::connect(this->actionStart_ROOT, SIGNAL(triggered()), this, SLOT(startrOot(int argc, char**argv)));
}



void MainWindow::about()
{
    QMessageBox::about(this, tr("About Diode"),
                       tr("Diffractive Imaging of Dilute Ensembles (Diode)\n\n"
                          "Copyright (C) 2009 The CASS team\n"
                          "Copyright (C) 2009 Jochen Küpper\n"));
}



void MainWindow::aboutQt()
{
    QMessageBox::aboutQt(this, tr("Diode: About At"));
}



void MainWindow::closeEvent(QCloseEvent *event)
{
    event->accept();
}


void MainWindow::quit()
{
    close();
}


void MainWindow::startrOotb()
{
    int argc=0;
    char**argv=NULL;
    // start root windows
    main_root(argc,argv);
}
void MainWindow::startrOot(int argc, char**argv)
{
    // start root windows
    main_root(argc,argv);
}


// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
