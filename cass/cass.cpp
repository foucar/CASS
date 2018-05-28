// Copyright (C) 2009,2010 Jochen Küpper
// Copyright (C) 2009-2017 Lutz Foucar

/**
 * @file cass.cpp file contains the main cass program
 *
 * @author Lutz Foucar
 */

#include <iostream>
#include <signal.h>


#include <QtCore/QStringList>
#include <QtCore/QCoreApplication>

#include "cass.h"
#include "cass_version.h"
#include "log.h"
#include "input_base.h"
#include "ratemeter.h"
#include "rate_plotter.h"
#include "ringbuffer.hpp"
//#include "test_input.h"
#include "tcpserver.h"
#include "processor_manager.h"
#include "worker.h"
#include "cass_settings.h"
#include "cl_parser.hpp"
#ifdef OFFLINE
#include "multifile_input.h"
#include "file_input.h"
#else
#ifdef LCLSLIBRARY
#include "sharedmemory_input.h"
#endif
#include "tcp_input.h"
#endif
#ifdef HTTPSERVER
#include "httpserver.h"
#endif
#ifdef SACLADATA
#ifdef OFFLINE
#include "sacla_offline_input.h"
#else
#include "sacla_online_input.h"
#endif
#endif
#ifdef HDF5
#ifdef OFFLINE
#include "hdf5_file_input.h"
#include "xfel_hdf5_file_input.h"
#endif
#endif
#ifdef ZEROMQ
#include "zmq_input.h"
#endif
#ifdef XFELLIBRARY
#include "xfel_online_input.h"
#endif


using namespace std;
using namespace cass;

namespace cass
{
/** end the input thread
 *
 * @param unused unused parameter needed to register this callback as signal
 *               handler
 *
 * @author Lutz Foucar
 */
void endInputThread(int /*unused*/)
{
  InputBase::reference().end();
}

/** set up the own handler to react on the sigquit (crtl+ \\) signal
 *
 * @author Lutz Foucar
 */
void setSignalHandler()
{
  struct sigaction quit;
  quit.sa_handler = endInputThread;
  sigemptyset(&quit.sa_mask);
  quit.sa_flags = SA_RESTART;

  if (sigaction(SIGQUIT, &quit, 0))
    throw runtime_error("setSignalHandler(): could not set up the quit signal");
}
}//end namespace cass

/** The main program.
 *
 * @section clpar CASS Commandline Parameters
 * - -i filename containing filesnames of xtcfiles to process (offline)
 * - -m enable multifile input (offline)
 * - -q quit after finished with all files (offline)
 * - --hdf5 enable the hdf5 file input module (offline)
 * - -t enable the tcp input (online)
 * - -p partition tag for accessing the shared memory (online)
 * - -c client id for shared memory access (online)
 * - --noSoap disables the soap server
 * - -s TCP port of the soap server (offline / online)
 * - -r suppress the rate output
 * - -o output filename passed to the Processor (offline / online)
 * - -f optional complete path to the cass.ini to use (offline / online)
 * - --sacla Enable sacla input (offline / online)
 * - -h show this help
 * - --version display the version of cass
 *
 * @author Lutz Foucar
 */
int main(int argc, char **argv)
{
  try
  {
     /** construct Qt application object to hold the run loop */
    QCoreApplication app(argc, argv);

    /** register used types as Qt meta type */
    qRegisterMetaType< std::string >("std::string");
    qRegisterMetaType<cass::ProcessorManager::key_t>("cass::ProcessorManager::key_t");
    qRegisterMetaType<size_t>("size_t");

    /** set up details for QSettings and Co.*/
    QCoreApplication::setOrganizationName("CFEL-ASG");
    QCoreApplication::setOrganizationDomain("endstation.asg.cfel.de");
    QCoreApplication::setApplicationName("CASS");
    QSettings::setDefaultFormat(QSettings::IniFormat);

    /** create a qsettings object from which one can retrieve the default
     *  cass.ini. After parsing one can then set the CASSSettings default
     *  object to the user requested .ini file or the keep the default .ini
     *  file to use
     */
    QSettings settings;
    settings.sync();

    /** set up parameters to be retrieved from the command line and parse them
     *  with the help of the command line parser object
     */
    CommandlineArgumentParser parser;
#ifdef OFFLINE
    string filelistname("filesToProcess.txt");
    parser.add("-i","filename of file containing filesnames of files to process",filelistname);
    bool multifile(false);
    parser.add("-m","enable the multifile input",multifile);
    bool quitwhendone(false);
    parser.add("-q","quit after finished with all files",quitwhendone);
#ifdef HDF5
    bool hdf5file(false);
    parser.add("--hdf5","use the special hdf5 file parser",hdf5file);
    bool xfelhdf5file(false);
    parser.add("--xfelhdf5","use the xfel hdf5 file parser",xfelhdf5file);
#endif
#else
    bool tcp(false);
    parser.add("-t","enable the tcp input",tcp);
#ifdef LCLSLIBRARY
    string partitionTag("0_1_cass_AMO");
    parser.add("-p","partition tag for accessing the shared memory",partitionTag);
    int index(0);
    parser.add("-c","client id for shared memory access",index);
#endif
#endif
    bool noSoap(false);
    parser.add("--noSoap","Disable the Soap Server",noSoap);
    int soap_port(12321);
    parser.add("-s","TCP port of the soap server ",soap_port);
//    bool useDatagenerator(false);
//    parser.add("-d","Use generated fake data as input",useDatagenerator);
    string outputfilename("output.ext");
    parser.add("-o","output filename passed to the Processor",outputfilename);
    string settingsfilename(settings.fileName().toStdString());
    parser.add("-f","complete path to the cass.ini to use",settingsfilename);
#ifdef SACLADATA
    bool sacladata(false);
    parser.add("--sacla","Enable SACLA Input",sacladata);
#endif
#ifdef ZEROMQ
    bool zmq(false);
    parser.add("--zmq","Enable the ZeroMQ Input",zmq);
#endif
#ifdef XFELLIBRARY
    bool xfelonlineinput(false);
    parser.add("--xfelonline","Enable the XFEL-Online Input",xfelonlineinput);
#endif
    bool showUsage(false);
    parser.add("-h","show this help",showUsage);
    bool showVersion(false);
    parser.add("--version","display the version of cass",showVersion);

    parser(app.arguments());


    /** show help and exit if requested */
    if (showUsage)
    {
      parser.usage();
      exit(0);
    }

    /** show version and exit if requested */
    if (showVersion)
    {
      cout <<VERSION <<endl;
      exit(0);
    }

    /** set the user requested .ini file name */
    CASSSettings::setFilename(settingsfilename);

    /** since the settings for the log are now loaded one can now add things to
     *  the log
     */
    Log::add(Log::INFO,"Start CASS");


    /** create a ratemeter objects for input and worker and the plotter to plot
     *  the rates that are calculated.
     */
    Ratemeter inputrate;
    Ratemeter inputload;
    Ratemeter workerrate;
    RatePlotter plotter(inputrate,inputload,workerrate);

    /** create workers and requested inputs which need a ringbuffer for passing the
     *  the events from one to the other. Once created connect their terminated
     *  and finished signals such that they notify each other about that they are
     *  done processing the events. Also create the processor singleton used
     *  by the worker to  process the events.
     */
    RingBuffer<CASSEvent> ringbuffer(RingBufferSize);
    ProcessorManager::instance(outputfilename);
    Workers::instance(ringbuffer, workerrate);
#ifdef OFFLINE
    if (multifile)
      MultiFileInput::instance(filelistname, ringbuffer, inputrate, inputload, quitwhendone);
//    else if (useDatagenerator)
//      TestInput::instance(ringbuffer,inputrate, inputload);
#ifdef SACLADATA
    else if (sacladata)
      SACLAOfflineInput::instance(filelistname,ringbuffer,inputrate,inputload,quitwhendone);
#endif
#ifdef HDF5
    else if (hdf5file)
      HDF5FileInput::instance(filelistname,ringbuffer,inputrate, inputload,quitwhendone);
    else if (xfelhdf5file)
      XFELHDF5FileInput::instance(filelistname,ringbuffer,inputrate, inputload,quitwhendone);
#endif
#ifdef ZEROMQ
    else if (zmq)
     ZMQInput::instance(ringbuffer, inputrate, inputload, quitwhendone);
#endif
#ifdef XFELLIBRARY
    else if (xfelonlineinput)
     XFELOnlineInput::instance(ringbuffer, inputrate, inputload, quitwhendone);
#endif
    else
      FileInput::instance(filelistname, ringbuffer, inputrate, inputload, quitwhendone);
#else
    if (tcp)
      TCPInput::instance(ringbuffer,inputrate, inputload);
//    else if (useDatagenerator)
//      TestInput::instance(ringbuffer,inputrate, inputload);
#ifdef SACLADATA
    else if (sacladata)
      SACLAOnlineInput::instance(ringbuffer,inputrate,inputload);
#endif
#ifdef ZEROMQ
    else if (zmq)
     ZMQInput::instance(ringbuffer, inputrate, inputload);
#endif
#ifdef LCLSLIBRARY
    else
      SharedMemoryInput::instance(partitionTag, index, ringbuffer, inputrate, inputload);
#endif
#endif

    /** connect a own signal handler that acts on when sigquit is sent by linux
     *  it will call the end member of the input.
     */
    setSignalHandler();

    /** set up the TCP/SOAP server */
    SoapServer::shared_pointer server;
    if(!noSoap)
      server = SoapServer::instance(soap_port);

    /** set up the optional http server */
#ifdef HTTPSERVER
    httpServer http_server(get_histogram);
#endif

    /** start worker and server threads, then start input and wait until input
     *  is done
     */
    Workers::reference().start();
    if (!noSoap)
      server->start();
#ifdef HTTPSERVER
    http_server.start();
#endif
    plotter.start();
    InputBase::reference().start();

    /** periodically check if the Workers are still running, while the input
     *  is still running. If not all workers are still running quit the input
     *  at this point and wait until it is quitted. Then rethrow the reason
     *  why the worker have quitted running.
     *  @note wait will return false if the input is still running
     */
    while (!InputBase::reference().wait(500))
    {
      if (!Workers::reference().running())
      {
        Log::add(Log::DEBUG4,"main(): One of the workers seem to not be working, quit all workers and then quit the input");
        InputBase::reference().end();
        InputBase::reference().wait();
        Workers::reference().end();
        Workers::reference().rethrowException();
      }
    }

    /** when the input was shut down gratiously, wait until the ringbuffer is
     *  empty (thus all events have been processed by the workers)
     */
    if (InputBase::reference().exceptionThrown() == InputBase::NO_EXCEPTION)
      InputBase::reference().ringbuffer().waitUntilEmpty();

    /** now stop the worker threads */
    Workers::reference().end();

    /** rethrow the exceptions if any where thrown inside the Input threads */
    InputBase::reference().rethrowException();

    /** @todo find out how to stop the other threads */
  }
  catch (const invalid_argument &error)
  {
    Log::add(Log::ERROR,string("User input is wrong: ") + error.what());
    cout <<endl<<endl<< "User input is wrong: Please review the log file '"
         <<Log::filename()<<"'"<<endl;
  }
  catch (exception &error)
  {
    Log::add(Log::ERROR,string("Exception happened: ") + error.what());
    cout <<endl<<endl<< "An exception happened: Please review the log file '"
         <<Log::filename()<<"'"<<endl;
  }
  catch (...)
  {
    Log::add(Log::ERROR,"main(): something bad happend, quitting the program.");
    cout <<endl<<endl<< "Bad error: Please review the log file '"
         <<Log::filename()<<"'"<<endl;
  }
  Log::add(Log::INFO,"Quitting CASS");
  cout << endl;
}
