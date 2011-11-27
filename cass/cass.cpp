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

namespace cass
{
/** command line argument parser
 *
 * object that will parse the command line parameters and set the switches and
 * retrieve the arguments
 *
 * @author Lutz Foucar
 */
class CliParser
{
  /** a container type for switches */
  typedef map<string,bool*>  switches_t;

  /** a container type for switches */
  typedef map<string,int*>  intarguments_t;

  /** a container type for switches */
  typedef map<string,string*>  stringarguments_t;

  /** operator to parse the argumetns
   *
   * the arguments are retrieved as a QStringList from Qt. Go through the list
   * and try to find the parameter in the containers. If it is the switches
   * container simply set the switch to true. Otherwise take the next parameter
   * that should be the argumetn of the preceding parameter.
   *
   * @param list the list of arguments
   */
  void operator()(const QStringList& list)
  {
    QStringList::const_iterator constIterator;
    for (constIterator = list.constBegin(); constIterator != list.constEnd(); ++constIterator)
    {
      bool foundParam(false);
      switches_t::iterator swIt(_switches.find((*constIterator).toStdString()));
      if (swIt != _switches.end())
      {
        *(swIt->second) = true;
        foundParam = true;
      }
      if (!foundParam)
      {
        intarguments_t::iterator iaIt(_intargs.find((*constIterator).toStdString()));
        if (iaIt != _intargs.end())
        {
          ++constIterator;
          *(iaIt->second) = (*constIterator).toInt();
          foundParam = true;
        }
      }
      if (!foundParam)
      {
        stringarguments_t::iterator saIt(_stringargs.find((*constIterator).toStdString()));
        if (saIt != _stringargs.end())
        {
          ++constIterator;
          *(saIt->second) = (*constIterator).toStdString();
          foundParam = true;
        }
      }
    }
  }

  /** add a switch to the switches container
   *
   * @param sw the name of the parameter to look for
   * @param val a reference to the value that should be changed.
   */
  void add(const string &sw, bool &val ) {_switches[sw] = &val;}

  /** add a switch to the string container
   *
   * @param sw the name of the parameter to look for
   * @param val a reference to the value that should be changed.
   */
  void add(const string &sw, string &val ) {_stringargs[sw] = &val;}

  /** add a switch to the int container
   *
   * @param sw the name of the parameter to look for
   * @param val a reference to the value that should be changed.
   */
  void add(const string &sw, int &val ) {_intargs[sw] = &val;}

private:
  /** container for the switches */
  switches_t _switches;

  /** container for the string arguments */
  stringarguments_t _stringargs;

  /** container for the int arguments */
  intarguments_t _intargs;
};
}

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
 * @todo make the workers a shared pointer, to make the whole main program within
 *       a try - catch environment
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
  string partitionTag("0_1_cass_AMO");
  //the sharememory client index
  int index(0);
#endif
#ifdef SOAPSERVER
  // SOAP server port (default: 12321)
  size_t soap_port(12321);
#endif
  //flag to tell whether the rate should be plotted
  bool plotrate(true);
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
      plotrate = false;
      break;
    case 'f':
      CASSSettings::setFilename(optarg);
      break;
    case 'o':
      outputfilename = optarg;
      break;
    default:
      throw invalid_argument("CASS: Parameter '" + string(1,c) +
                             "' with argument '" + optarg +
                             "' does not exist in current CASS configuration.");
      break;
    }
  }

  //a ringbuffer for the cassevents//
  RingBuffer<CASSEvent,RingBufferSize> ringbuffer;

  //create a ratemeters objects and the plotter for them//
  Ratemeter inputrate(1,qApp);
  Ratemeter workerrate(1,qApp);
  RatePlotter rateplotter(inputrate,workerrate,plotrate,qApp);

  //create workers and inputs//
  Workers workers(ringbuffer, workerrate, outputfilename, qApp);
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

  //create deamon to capture UNIX signals and connect the quit signal//
  setup_unix_signal_handlers();
  UnixSignalDaemon signaldaemon(qApp);
  QObject::connect(&signaldaemon, SIGNAL(QuitSignal()), input.get(), SLOT(end()));
  QObject::connect(&signaldaemon, SIGNAL(TermSignal()), input.get(), SLOT(end()));

  //when the thread has finished, we want to close this application
  QObject::connect(input.get(), SIGNAL(finished()), &workers, SLOT(end()));
  QObject::connect(input.get(), SIGNAL(terminated()), &workers, SLOT(end()));
  QObject::connect(&workers, SIGNAL(finished()), qApp, SLOT(quit()));


  // TCP/SOAP server
#ifdef SOAPSERVER
  EventGetter get_event(ringbuffer);
  HistogramGetter get_histogram;
  SoapServer *server(SoapServer::instance(get_event, get_histogram, soap_port));
  QObject::connect(server, SIGNAL(quit()), input.get(), SLOT(end()));
  QObject::connect(server, SIGNAL(readini(size_t)), input.get(), SLOT(loadSettings(size_t)));
  QObject::connect(server, SIGNAL(readini(size_t)), &workers, SLOT(loadSettings(size_t)));
  QObject::connect(server, SIGNAL(writeini(size_t)), &workers, SLOT(saveSettings()));
  QObject::connect(server, SIGNAL(clearHistogram(cass::PostProcessors::key_t)), &workers, SLOT(clearHistogram(cass::PostProcessors::key_t)));
  QObject::connect(server, SIGNAL(receiveCommand(cass::PostProcessors::key_t, std::string)), &workers, SLOT(receiveCommand(cass::PostProcessors::key_t, std::string)));
#endif

#ifdef HTTPSERVER
  // http server
  httpServer http_server(get_histogram);
#endif

  int retval(0);
  try
  {
    //start input and worker threads
    workers.start();
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
    workers.end();

    //clean up
#ifdef SOAPSERVER
    server->destroy();
#endif
#ifdef HTTPSERVER
    http_server.stop();
#endif
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
