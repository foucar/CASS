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



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
