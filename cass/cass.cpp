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
#include "input_base.h"
#include "file_input.h"
#include "multifile_input.h"
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
using namespace cass;

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
 * - m enable multifile input
 *
 * @author Lutz Foucar
 */
int main(int argc, char **argv)
{
  // construct Qt application object
  QApplication app(argc, argv,false);
  // register used types as Qt meta type
  qRegisterMetaType< string >("std::string");
  qRegisterMetaType<PostProcessors::key_t>("PostProcessors::key_t");
  qRegisterMetaType<size_t>("size_t");
  // set up details for QSettings and Co.
  QCoreApplication::setOrganizationName("CFEL-ASG");
  QCoreApplication::setOrganizationDomain("endstation.asg.cfel.de");
  QCoreApplication::setApplicationName("CASS");
  QSettings::setDefaultFormat(QSettings::IniFormat);
  QSettings settings;
  settings.sync();
  // setup cass settings. when one wants a user settable cass.ini, just use
  // CASSSettings instead of QSettings
  CASSSettings::setFilename(settings.fileName().toStdString());

#ifdef OFFLINE
  // filename containing XTC filenames
  string filelistname("filesToProcess.txt");
  // the flag whether to use multifile input default is false
  bool multifile(false);
  //flag to tell to quit when program has finished executing all files
  bool quitwhendone(false);
#else
  //create a container for the partition tag
  string partitionTag;
  //the sharememory client index
  int index(0);
#endif
#ifdef SOAPSERVER
  // SOAP server port (default: 12321)
  size_t soap_port(12321);
#endif
  //flag to suppress the rate output
  bool suppressrate(false);
  // filename of the output filename
  string outputfilename("output.ext");

  //parse command line options
  int c;
  while((c = getopt(argc, argv, "rmqp:s:c:i:o:f:")) != -1)
  {
    switch (c)
    {
#ifdef OFFLINE
    case 'q':
      quitwhendone = true;
      break;
    case 'i':
      filelistname = optarg;
      break;
    case 'm':
      multifile = true;
      break;
#else
    case 'p':
      partitionTag = optarg;
      break;
    case 'c':
      index = strtol(optarg, 0, 0);
      break;
#endif
#ifdef SOAPSERVER
    case 's':
      soap_port = strtol(optarg, 0, 0);
      break;
#endif
    case 'r':
      suppressrate = true;
      break;
    case 'f':
      CASSSettings::setFilename(optarg);
      break;
    case 'o':
      outputfilename = optarg;
      break;
    default:
#ifdef OFFLINE
      cout << "please give me at least an filename that contains the"
           <<" xtcfilenames you want to process"<<endl;
#else
      cout << "please give me at least a partition tag" <<endl;
#endif
      return 2;
      break;
    }
  }

  //a ringbuffer for the cassevents//
  RingBuffer<CASSEvent,RingBufferSize> ringbuffer;


  //create a ratemeters objects and the plotter for them//
  Ratemeter inputrate(1,qApp);
  Ratemeter workerrate(1,qApp);
  RatePlotter *rateplotter(0);
  if (!suppressrate)
    rateplotter = new RatePlotter(inputrate,workerrate,qApp);

  //create workers and inputs//
  Workers *workers(new Workers(ringbuffer, outputfilename, qApp));
#ifdef OFFLINE
  InputBase::shared_pointer input;
  if (multifile)
    input = InputBase::shared_pointer(new MultiFileInput(filelistname,
                                                         ringbuffer,
                                                         inputrate,
                                                         quitwhendone));
  else
    input = InputBase::shared_pointer(new FileInput(filelistname,
                                                    ringbuffer,
                                                    inputrate,
                                                    quitwhendone));
#else
  // create shared memory input object //
  InputBase::shared_pointer input(new SharedMemoryInput(partitionTag,
                                                        index,
                                                        ringbuffer,
                                                        inputrate));
#endif

  //create deamon to capture UNIX signals//
  setup_unix_signal_handlers();
  UnixSignalDaemon *signaldaemon(new UnixSignalDaemon(qApp));


  //connect ratemeters//
  QObject::connect(workers, SIGNAL(processedEvent()), &workerrate, SLOT(count()));


  //when the thread has finished, we want to close this application
  QObject::connect(input.get(), SIGNAL(finished()), workers, SLOT(end()));
  QObject::connect(input.get(), SIGNAL(terminated()), workers, SLOT(end()));
  QObject::connect(workers, SIGNAL(finished()), qApp, SLOT(quit()));

  //close the programm when sigquit or sigterm were received//
  QObject::connect(signaldaemon, SIGNAL(QuitSignal()), input.get(), SLOT(end()));
  QObject::connect(signaldaemon, SIGNAL(TermSignal()), input.get(), SLOT(end()));

  // TCP/SOAP server
#ifdef SOAPSERVER
  EventGetter get_event(ringbuffer);
  HistogramGetter get_histogram;
  SoapServer *server(SoapServer::instance(get_event, get_histogram, soap_port));
  QObject::connect(server, SIGNAL(quit()), input.get(), SLOT(end()));
  QObject::connect(server, SIGNAL(readini(size_t)), input.get(), SLOT(loadSettings(size_t)));
  QObject::connect(server, SIGNAL(readini(size_t)), workers, SLOT(loadSettings(size_t)));
  QObject::connect(server, SIGNAL(writeini(size_t)), workers, SLOT(saveSettings()));
  QObject::connect(server, SIGNAL(clearHistogram(PostProcessors::key_t)), workers, SLOT(clearHistogram(PostProcessors::key_t)));
  QObject::connect(server, SIGNAL(receiveCommand(PostProcessors::key_t, string)), workers, SLOT(receiveCommand(PostProcessors::key_t, string)));
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
#ifdef HTTPSERVER
    http_server.stop();
#endif
    delete rateplotter;
    delete workers;

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
    delete workers;
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
