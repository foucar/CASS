// Copyright (C) 2009 Jochen Küpper,lmf

#include <iostream>
#include <QtGui/QApplication>

#include "cass.h"
#include "analyzer.h"
#include "event_queue.h"
#include "event_manager.h"
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
  QApplication app(argc, argv);

  // create event queue object
  cass::EventQueue *input(new cass::EventQueue());
  // create event manager object
  cass::EventManager *eventmanager(new cass::EventManager());
  // create format converter object
  cass::FormatConverter *conversion(cass::FormatConverter::instance(input,eventmanager));
  // create analysis object
  cass::Analyzer *analysis(new cass::Analyzer());
  // create database object
  cass::database::Database *database(new cass::database::Database());
  // create a ratemeter object
  cass::Ratemeter *ratemeter(new cass::Ratemeter());
  // create a dialog object
  cass::Window * window(new cass::Window());

  // connect the objects
  QObject::connect (input, SIGNAL(nextEvent(quint32)), conversion, SLOT(processDatagram(quint32)));
  QObject::connect (conversion, SIGNAL(nextEvent(cass::CASSEvent*)), analysis, SLOT(processEvent(cass::CASSEvent*)));
  QObject::connect (analysis, SIGNAL(nextEvent(cass::CASSEvent*)), database, SLOT(add(cass::CASSEvent*)));
  QObject::connect (database, SIGNAL(nextEvent()), ratemeter, SLOT(nextEvent()));

  // connect controls
  QObject::connect (window, SIGNAL (load()), analysis, SLOT(loadSettings()));
  QObject::connect (window, SIGNAL (save()), analysis, SLOT(saveSettings()));
  QObject::connect (window, SIGNAL (start()), input, SLOT(start()));
  QObject::connect (window, SIGNAL (quit()), input, SLOT(end()));

  // when the thread has finished, we want to close this application
  QObject::connect(input, SIGNAL(finished()), window, SLOT(close()));

  //show dialog//
  window->show();

  // start input thread
  input->start();

  // start Qt event loop
  int retval(app.exec());

  // clean up
  delete window;
  delete ratemeter;
  delete database;
  delete analysis;
  cass::FormatConverter::destroy();
  delete eventmanager;
  delete input;

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
