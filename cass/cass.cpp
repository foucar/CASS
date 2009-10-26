// Copyright (C) 2009 Jochen Küpper

#include <QtGui/QApplication>

#include "cass.h"
#include "analyzer.h"
#include "event_queue.h"
#include "format_converter.h"
#include "database.h"
#include "ratemeter.h"
#include "dialog.h"

/*#include "main_window.h"
#include "TGraph.h"
#include "TQtWidget.h"
#include "TCanvas.h"
#include "TDatime.h"
#include "TAxis.h"
#include <QLabel>*/


int main(int argc, char **argv)
{
    // construct Qt application object
    QApplication app(argc, argv, true);

    // create event queue object
    cass::EventQueue *input(new cass::EventQueue());
    // create format converter object
    cass::FormatConverter *conversion(cass::FormatConverter::instance(input));
    // create analysis object
    cass::Analyzer *analysis(new cass::Analyzer());
    // create database object
    cass::database::Database *database(new cass::database::Database());
    // create a ratemeter object
    cass::Ratemeter *ratemeter(new cass::Ratemeter());
    // create a dialog object
    cass::Dialog * dialog(new cass::Dialog());

    // connect the objects
    QObject::connect (input, SIGNAL(nextEvent(quint32)), conversion, SLOT(processDatagram(quint32)));
    QObject::connect (conversion, SIGNAL(nextEvent(cass::CASSEvent*)), analysis, SLOT(processEvent(cass::CASSEvent*)));
    QObject::connect (analysis, SIGNAL(nextEvent(cass::CASSEvent*)), database, SLOT(add(cass::CASSEvent*)));
    QObject::connect (database, SIGNAL(nextEvent()), ratemeter, SLOT(nextEvent()));

    // connect controls
    QObject::connect (dialog, SIGNAL(quit()), &app, SLOT(quit()));
    QObject::connect (dialog, SIGNAL (load()), analysis, SLOT(loadSettings()));
    QObject::connect (dialog, SIGNAL (save()), analysis, SLOT(saveSettings()));

    // connect deletion of thread
    QObject::connect(input, SIGNAL(finished()), input, SLOT(deleteLater()));

    //show dialog//
    dialog->show();

    // start input thread
    input->start();

    // start Qt event loop
    int retval(app.exec());

    // clean up
    delete database;
    delete analysis;
    delete input;
    cass::FormatConverter::destroy();

    // finish
    return retval;
}




// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
