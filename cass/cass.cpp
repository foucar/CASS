// Copyright (C) 2009 Jochen Küpper

#include <QtGui/QApplication>

#include "cass.h"
#include "Analyzer.h"
#include "EventQueue.h"
#include "FormatConverter.h"
#include "RootTree.h"

using namespace cass;

int main(int argc, char **argv)
{
    // construct Qt application object
    QApplication app(argc, argv, false);

    // create event queue object
    EventQueue *input(new EventQueue);
    // create format converter object
    FormatConverter *conversion(FormatConverter::instance(input));
    // create analysis object
    Analyzer *analysis(new Analyzer);
//    // create database object
//    RootTree *database(new RootTree);

    // connect the objects
    QObject::connect (input, SIGNAL(nextEvent(uint32_t)), conversion, SLOT(processDatagram(uint32_t)));
    QObject::connect (conversion, SIGNAL(nextEvent(CASSEvent*)), analysis, SLOT(nextEvent(Event&)));
//    QObject::connect (analysis, SIGNAL(nextEvent(Event&)), database, SLOT(nextEvent(Event&)));

    // start Qt event loop
    int retval(app.exec());

    // clean up
//    delete database;
    delete analysis;
    delete input;
    FormatConverter::destroy();

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
