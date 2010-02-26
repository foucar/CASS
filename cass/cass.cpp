// Copyright (C) 2009 Jochen Küpper,lmf
//
#include <iostream>
#include <QtGui/QApplication>

#include "cass.h"
#include "analyzer.h"
#include "sharedmemory_input.h"
#include "ringbuffer.h"
#include "format_converter.h"
#include "ratemeter.h"
#include "dialog.h"
#include "worker.h"


int main(int argc, char **argv)
{
  // construct Qt application object
  QApplication app(argc, argv);

  //create a container for the partition tag
  int c;
  char partitionTag[128];

  //get the partition string
  while ((c = getopt(argc, argv, "p:")) != -1)
  {
    switch (c) 
    {
      case 'p':
        strcpy(partitionTag, optarg);
        break;
      default:
        std::cout << "please give me a partition tag" <<std::endl;
        break;
    }
  }


  //a nonblocking ringbuffer for the cassevents//
  lmf::RingBuffer<cass::CASSEvent,cass::RingBufferSize> ringbuffer;
  ringbuffer.behaviour(lmf::RingBuffer<cass::CASSEvent,cass::RingBufferSize>::nonblocking);
  // create shared memory input object //
  cass::SharedMemoryInput *input(new cass::SharedMemoryInput(partitionTag,ringbuffer),qApp);
  // create a worker//
  //cass::Worker *worker(new cass::Worker(ringbuffer));
  cass::Workers *workers(new cass::Workers(ringbuffer,qApp));
  //create a ratemeter object for the input//
  //cass::Ratemeter *inputrate(new cass::Ratemeter());
  // create a ratemeter object for the worker//
  //cass::Ratemeter *workerrate(new cass::Ratemeter());
  // create a dialog object //
  //cass::Window * window(new cass::Window());

  //conncet ratemeters//
//  QObject::connect(worker,     SIGNAL(processedEvent()), workerrate, SLOT(count()));
//  QObject::connect(input,      SIGNAL(newEventAdded()),  inputrate,  SLOT(count()));
//  QObject::connect(workerrate, SIGNAL(rate(double)),     window,     SLOT(updateProcessRate(double)));
//  QObject::connect(inputrate,  SIGNAL(rate(double)),     window,     SLOT(updateInputRate(double)));


  // connect controls
//  QObject::connect (window, SIGNAL (load()), worker, SLOT(loadSettings()));
//  QObject::connect (window, SIGNAL (save()), worker, SLOT(saveSettings()));
//  QObject::connect (window, SIGNAL (quit()), input, SLOT(end()));

  // when the thread has finished, we want to close this application
  QObject::connect(input, SIGNAL(finished()), worker, SLOT(end()));
  QObject::connect(workers, SIGNAL(finished()), qApp, SLOT(quit()));
//  QObject::connect(worker, SIGNAL(finished()), window, SLOT(close()));

  //show dialog//
//  window->show();

  // start input and worker threads
  input->start();
  worker->start();

  // start Qt event loop
  int retval(app.exec());

  // clean up
//  delete window;
//  delete workerrate;
//  delete inputrate;
  delete workers;
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
