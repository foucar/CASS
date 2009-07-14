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
    EventQueue input;
    // create format converter object
    FormatConverter conversion;
    // create analysis object
    Analyzer analysis;
    // create database object
    RootTree database;

    // connect the objects
    QObject::connect (&input, SIGNAL(nextEvent(Event&)), &conversion, SLOT(nextEvent(Event&)));
    QObject::connect (&conversion, SIGNAL(nextEvent(Event&)), &analysis, SLOT(nextEvent(Event&)));
    QObject::connect (&analysis, SIGNAL(nextEvent(Event&)), &database, SLOT(nextEvent(Event&)));

    // start Qt event loop
    return app.exec();
}




// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
