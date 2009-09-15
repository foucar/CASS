// Copyright (C) 2009 Jochen Küpper

#include <QtGui/QApplication>

#include "cass.h"
#include "analyzer.h"
#include "event_queue.h"
#include "format_converter.h"
#include "database.h"

//using namespace cass;

int main(int argc, char **argv)
{
    // construct Qt application object
    QApplication app(argc, argv, false);

    // create event queue object
    cass::EventQueue *input(new cass::EventQueue());
    // create format converter object
    cass::FormatConverter *conversion(cass::FormatConverter::instance(input));
    // create analysis object
    cass::Analyzer *analysis(new cass::Analyzer());
    // create database object
    cass::database::Database *database(new cass::database::Database());

    // connect the objects
    QObject::connect (input, SIGNAL(nextEvent(quint32)), conversion, SLOT(processDatagram(quint32)));
    QObject::connect (conversion, SIGNAL(nextEvent(cass::CASSEvent*)), analysis, SLOT(processEvent(cass::CASSEvent*)));
    QObject::connect (analysis, SIGNAL(nextEvent(cass::CASSEvent*)), database, SLOT(add(cass::CASSEvent*)));

    QObject::connect(input, SIGNAL(finished()), input, SLOT(deleteLater()));
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
