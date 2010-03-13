// Copyright (C) 2009,2010 Jochen Küpper
// Copyright (C) 2009,2010 lmf
//
#include <iostream>
#include <QtGui/QApplication>

#include "cass.h"
#include "analyzer.h"
#include "sharedmemory_input.h"
#include "ringbuffer.h"
#include "format_converter.h"
#include "ratemeter.h"
#include "tcpserver.h"
#include "worker.h"


int main(int argc, char **argv)
{
  // construct Qt application object
  QApplication app(argc, argv,false);

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
  cass::RingBuffer<cass::CASSEvent,cass::RingBufferSize> ringbuffer;
  ringbuffer.behaviour(cass::RingBuffer<cass::CASSEvent,cass::RingBufferSize>::nonblocking);
  // create shared memory input object //
  cass::SharedMemoryInput *input(new cass::SharedMemoryInput(partitionTag,ringbuffer,qApp));
  // create a worker//
  //cass::Worker *worker(new cass::Worker(ringbuffer));
  cass::Workers *workers(new cass::Workers(ringbuffer,qApp));
  //create a ratemeter object for the input//
  cass::Ratemeter *inputrate(new cass::Ratemeter());
  // create a ratemeter object for the worker//
  cass::Ratemeter *workerrate(new cass::Ratemeter());

  //connect ratemeters//
  QObject::connect(workers, SIGNAL(processedEvent()), workerrate, SLOT(count()));
  QObject::connect(input,   SIGNAL(newEventAdded()),  inputrate,  SLOT(count()));


  // when the thread has finished, we want to close this application
  QObject::connect(input, SIGNAL(finished()), workers, SLOT(end()));
  QObject::connect(workers, SIGNAL(finished()), qApp, SLOT(quit()));

  // start input and worker threads
  input->start();
  workers->start();

  // start TCP server
#warning fix setup of TCP server parameters
  //tell the server how to get an id or histogram//
  cass::TCP::GetEvent get_event;
  cass::TCP::GetHistogram get_histogram;
  cass::TCP::Server server(get_event, get_histogram,qApp);
  //setup the connections//
  QObject::connect(&server, SIGNAL(quit()), input, SLOT(end()));
  QObject::connect(&server, SIGNAL(readini(size_t)), input, SLOT(loadSettings(size_t)));
  QObject::connect(&server, SIGNAL(readini(size_t)), workers, SLOT(loadSettings(size_t)));
  //let the server listen to port 54321//
  if(! server.listen(QHostAddress::Any, 54321)) {
      std::cerr << "Failed to bind to TCP port" << std::endl;
      return 1;
  }

  // start Qt event loop
  int retval(app.exec());

  // clean up
  delete workerrate;
  delete inputrate;
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
