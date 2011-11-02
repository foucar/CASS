// Copyright (C) 2009,2010 Jochen Küpper
// Copyright (C) 2009,2010 Lutz Foucar

/**
 * @file cass.cpp file contains the main cass program
 *
 * @author Lutz Foucar
 */

#include <iostream>
#include <QtCore/QStringList>
#include <QtGui/QApplication>

#include "cass.h"
#include "analyzer.h"
#include "daemon.h"
#include "file_input.h"
#include "format_converter.h"
#include "ratemeter.h"
#include "rate_plotter.h"
#include "ringbuffer.h"
#include "sharedmemory_input.h"
#include "tcpserver.h"
#ifdef HTTPSERVER
#include "httpserver.h"
#endif
#include "worker.h"
#include "cass_settings.h"


using namespace std;

/** The main program.
 *
 * @section clpar CASS Commandline Parameters
 * - i filename containing filesnames of xtcfiles to process (offline)
 * - c client id for shared memory access (online)
 * - s TCP port of the soap server (offline / online)
 * - p partition tag for accessing the shared memory (online)
 * - o output filename passed to the postprocessor (offline / online)
 * - q quit after finished with all files (offline)
 * - f optional complete path to the cass.ini to use (offline / online)
 * - r suppress the rate output
 *
 * @author Lutz Foucar
 */
int main(int argc, char **argv)
{
  // construct Qt application object
  QApplication app(argc, argv,false);
  // register used types as Qt meta type
  qRegisterMetaType< std::string >("std::string");
  qRegisterMetaType<cass::PostProcessors::key_t>("cass::PostProcessors::key_t");
  qRegisterMetaType<size_t>("size_t");
  // set up details for QSettings and Co.
  // (So we can simply use QSettings settings; everywhere else.)
  QCoreApplication::setOrganizationName("CFEL-ASG");
  QCoreApplication::setOrganizationDomain("endstation.asg.cfel.de");
  QCoreApplication::setApplicationName("CASS");
  QSettings::setDefaultFormat(QSettings::IniFormat);
  QSettings settings;
  // QStringList keys(settings.allKeys());
  // for(QStringList::iterator iter = keys.begin(); iter != keys.end(); ++iter)
  //     std::cout << "   cass.ini keys: " << iter->toStdString() << std::endl;
  settings.sync();
  // setup cass settings. when one wants a user settable cass.ini, just use
  // CASSSettings instead of QSettings
  cass::CASSSettings::setFilename(settings.fileName().toStdString());

  //create a container for the partition tag
  int c;
  char partitionTag[128];
  partitionTag[0]='\0';
  // filename containing XTC filenames
  std::string filelistname("filesToProcess.txt");
  // filename of the output filename
  std::string outputfilename("output.ext");
#ifdef SOAPSERVER
  // SOAP server port (default: 12321)
  size_t soap_port(12321);
#endif
#ifndef OFFLINE
  //the sharememory client index
  int index(0);
#endif
  //flag to tell to quit when program has finished executing all files
  bool quitwhendone(false);
  //flag to suppress the rate output
  bool suppressrate(false);

  //get the partition string
  while((c = getopt(argc, argv, "rqp:s:c:i:o:f:")) != -1)
  {
    switch (c)
    {
    case 'p':
#ifdef OFFLINE
      std::cout<<"WARNING: partition tag for shm: '"<<optarg
          <<"' will be ignored in offline mode."<<std::endl;
#endif
      strcpy(partitionTag, optarg);
      break;
    case 's':
#ifdef SOAPSERVER
      soap_port = strtol(optarg, 0, 0);
#endif
      break;
    case 'c':
#ifdef OFFLINE
      std::cout<<"WARNING: client id for shm: '"<<optarg
          <<"' will be ignored in offline mode."<<std::endl;
#else
      index = strtol(optarg, 0, 0);
#endif
      break;
    case 'q':
#ifndef OFFLINE
      std::cout<<"WARNING: 'quit when done' has no effect in online mode.";
#endif
      quitwhendone = true;
      break;
    case 'r':
      suppressrate = true;
      break;
    case 'i':
#ifndef OFFLINE
      std::cout<<"WARNING: file '"<<optarg
          <<"' containing all filenames will be ignored in online mode."
          <<std::endl;
#endif
      filelistname = optarg;
      break;
    case 'f':
      cass::CASSSettings::setFilename(optarg);
      break;
    case 'o':
      outputfilename = optarg;
      break;
    default:
#ifndef OFFLINE
      std::cout << "please give me at least a partition tag" <<std::endl;
#else
      std::cout << "please give me at least an filename that contains the"
          <<" xtcfilenames you want to process"
          <<std::endl;
#endif
      return 2;
      break;
    }
  }

  //a ringbuffer for the cassevents//
  cass::RingBuffer<cass::CASSEvent,cass::RingBufferSize> ringbuffer;
  //create workers//
  cass::Workers *workers(new cass::Workers(ringbuffer, outputfilename, qApp));
#ifndef OFFLINE
  // create shared memory input object //
  cass::SharedMemoryInput *input(new cass::SharedMemoryInput(partitionTag,
                                                             index,
                                                             ringbuffer));
#else
  // create file input object
  cass::FileInput *input(new cass::FileInput(filelistname.c_str(),ringbuffer,quitwhendone));
#endif

  //create a ratemeter object for the input//
  cass::Ratemeter *inputrate(new cass::Ratemeter(1,qApp));
  //create a ratemeter object for the worker//
  cass::Ratemeter *workerrate(new cass::Ratemeter(1,qApp));
  //create a rate plotter that will plot the rate of the worker and input//
  //only if user has not disabled it//
  cass::RatePlotter *rateplotter(0);
  if (!suppressrate)
    rateplotter = new cass::RatePlotter(*inputrate,*workerrate,qApp);
  //create deamon to capture UNIX signals//
  cass::setup_unix_signal_handlers();
  cass::UnixSignalDaemon *signaldaemon(new cass::UnixSignalDaemon(qApp));


  //connect ratemeters//
  QObject::connect(workers, SIGNAL(processedEvent()), workerrate, SLOT(count()));
  QObject::connect(input,   SIGNAL(newEventAdded()),  inputrate,  SLOT(count()));


  //when the thread has finished, we want to close this application
  QObject::connect(input, SIGNAL(finished()), workers, SLOT(end()));
  QObject::connect(input, SIGNAL(terminated()), workers, SLOT(end()));
  QObject::connect(workers, SIGNAL(finished()), qApp, SLOT(quit()));

  //close the programm when sigquit or sigterm were received//
  QObject::connect(signaldaemon, SIGNAL(QuitSignal()), input, SLOT(end()));
  QObject::connect(signaldaemon, SIGNAL(TermSignal()), input, SLOT(end()));

  // TCP/SOAP server
#ifdef SOAPSERVER
  cass::EventGetter get_event(ringbuffer);
  cass::HistogramGetter get_histogram;
  cass::SoapServer *server(cass::SoapServer::instance(get_event, get_histogram, soap_port));
  QObject::connect(server, SIGNAL(quit()), input, SLOT(end()));
  QObject::connect(server, SIGNAL(readini(size_t)), input, SLOT(loadSettings(size_t)));
  QObject::connect(server, SIGNAL(readini(size_t)), workers, SLOT(loadSettings(size_t)));
  QObject::connect(server, SIGNAL(writeini(size_t)), workers, SLOT(saveSettings()));
  QObject::connect(server, SIGNAL(clearHistogram(cass::PostProcessors::key_t)), workers, SLOT(clearHistogram(cass::PostProcessors::key_t)));
  QObject::connect(server, SIGNAL(receiveCommand(cass::PostProcessors::key_t, std::string)), workers, SLOT(receiveCommand(cass::PostProcessors::key_t, std::string)));
#endif

#ifdef HTTPSERVER
  // http server
  httpServer http_server(get_histogram);
#endif

  int retval(0);
  try
  {
    //start input and worker threads
    workers->start();
    input->start();
#ifdef SOAPSERVER
    server->start();
#endif
#ifdef HTTPSERVER
    http_server.start();
#endif

    //start Qt event loop
    retval = app.exec();

    //clean up
#ifdef SOAPSERVER
    server->destroy();
#endif
    delete rateplotter;
    delete workerrate;
    delete inputrate;
    delete workers;
    delete input;

    // one last sync of settings file
    settings.sync();

    //finish
    return retval;
  }
  catch (const invalid_argument &error)
  {
    cout << "User input is wrong: "<<error.what() <<endl;
    throw;
  }
  catch (const runtime_error &error)
  {
    cout << "Runtime error: "<<error.what() <<endl;
    throw;
  }
  catch (...)
  {
    cout<<"main(): something bad happend, quitting the program."<<endl;
    //stop threads//
    input->end();
    input->wait();
    workers->end();

    //clean up
#ifdef SOAPSERVER
    server->destroy();
#endif
#ifdef HTTPSERVER
    http_server.stop();
#endif
    delete rateplotter;
    delete workerrate;
    delete inputrate;
    delete workers;
    delete input;
  }
  return retval;
}



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "gnu"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:
