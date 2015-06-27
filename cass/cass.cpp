// Copyright (C) 2009,2010 Jochen Küpper
// Copyright (C) 2009,2010 Lutz Foucar

/**
 * @file cass.cpp file contains the main cass program
 *
 * @author Lutz Foucar
 */

#include <iostream>
#include <signal.h>


#include <QtCore/QStringList>
#include <QtGui/QApplication>

#include "cass.h"
#include "cass_version.h"
#include "log.h"
#include "input_base.h"
#include "file_input.h"
#include "multifile_input.h"
#include "format_converter.h"
#include "ratemeter.h"
#include "rate_plotter.h"
#include "ringbuffer.h"
#include "sharedmemory_input.h"
#include "tcp_input.h"
//#include "test_input.h"
#include "tcpserver.h"
#include "processor_manager.h"
#include "worker.h"
#include "cass_settings.h"
#include "cl_parser.hpp"
#ifdef HTTPSERVER
#include "httpserver.h"
#endif
#ifdef SACLADATA
#include "sacla_offline_input.h"
#include "sacla_online_input.h"
#endif
#ifdef HDF5
#include "hdf5_file_input.h"
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
 * - i filename containing filesnames of xtcfiles to process (offline)
 * - c client id for shared memory access (online)
 * - s TCP port of the soap server (offline / online)
 * - p partition tag for accessing the shared memory (online)
 * - o output filename passed to the postprocessor (offline / online)
 * - q quit after finished with all files (offline)
 * - f optional complete path to the cass.ini to use (offline / online)
 * - r suppress the rate output
 * - m enable multifile input
 * - --noSoap disables the soap server
 *
 * @author Lutz Foucar
 */
int main(int argc, char **argv)
{
  try
  {
     /** construct Qt application object to hold the run loop */
    QApplication app(argc, argv,false);

    /** register used types as Qt meta type */
    qRegisterMetaType< std::string >("std::string");
    qRegisterMetaType<cass::PostProcessors::key_t>("cass::PostProcessors::key_t");
    qRegisterMetaType<size_t>("size_t");

    /** set up details for QSettings and Co.*/
    QCoreApplication::setOrganizationName("CFEL-ASG");
    QCoreApplication::setOrganizationDomain("endstation.asg.cfel.de");
    QCoreApplication::setApplicationName("CASS");
    QSettings::setDefaultFormat(QSettings::IniFormat);

    /** create a qsettings object from which one can retrieve the default cass.ini.
     *  After parsing one can then set the CASSSettings default object to the user
     *  requested .ini file or the keep the default .ini file to use
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
    bool hdf5file(false);
    parser.add("--hdf5","use the special hdf5 file parser",hdf5file);
#else
    bool tcp(false);
    parser.add("-t","enable the tcp input",tcp);
    string partitionTag("0_1_cass_AMO");
    parser.add("-p","partition tag for accessing the shared memory",partitionTag);
    int index(0);
    parser.add("-c","client id for shared memory access",index);
#endif
    bool noSoap(false);
    parser.add("--noSoap","Disable the Soap Server",noSoap);
    int soap_port(12321);
    parser.add("-s","TCP port of the soap server ",soap_port);
    bool useDatagenerator(false);
    parser.add("-d","Use generated fake data as input",useDatagenerator);
    bool suppressrate(false);
    parser.add("-r","suppress the rate output",suppressrate);
    string outputfilename("output.ext");
    parser.add("-o","output filename passed to the postprocessor",outputfilename);
    string settingsfilename(settings.fileName().toStdString());
    parser.add("-f","complete path to the cass.ini to use",settingsfilename);
    bool sacladata(false);
    parser.add("--sacla","Enable SACLA Input",sacladata);
    bool showUsage(false);
    parser.add("-h","show this help",showUsage);
    bool showVersion(false);
    parser.add("--version","display the version of cass",showVersion);

    parser(QCoreApplication::arguments());


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
     *  done processing the events. Also create the postprocessor singleton used
     *  by the worker to post process the events.
     */
    RingBuffer<CASSEvent> ringbuffer(RingBufferSize);
    PostProcessors::instance(outputfilename);
    Workers::instance(ringbuffer, workerrate);
#ifdef OFFLINE
    if (multifile)
      MultiFileInput::instance(filelistname, ringbuffer, inputrate, inputload, quitwhendone);
//    else if (useDatagenerator)
//      TestInput::instance(ringbuffer,inputrate, inputload);
    else if (sacladata)
#ifdef SACLADATA
      SACLAOfflineInput::instance(filelistname,ringbuffer,inputrate,inputload,quitwhendone);
#else
      throw runtime_error("SACLA support has not been compiled into this version of CASS");
#endif
    else if (hdf5file)
#ifdef HDF5
      HDF5FileInput::instance(filelistname,ringbuffer,inputrate, inputload,quitwhendone);
#else
      throw runtime_error("HDF5 support has not been compiled into this version of CASS");
#endif
    else
      FileInput::instance(filelistname, ringbuffer, inputrate, inputload, quitwhendone);
#else
    if (tcp)
      TCPInput::instance(ringbuffer,inputrate, inputload);
//    else if (useDatagenerator)
//      TestInput::instance(ringbuffer,inputrate, inputload);
    else if (sacladata)
#ifdef SACLADATA
      SACLAOnlineInput::instance(ringbuffer,inputrate,inputload);
#else
      throw runtime_error("SACLA support has not been compiled into this version of CASS");
#endif
    else
      SharedMemoryInput::instance(partitionTag, index, ringbuffer, inputrate, inputload);
#endif

    /** connect a own signal handler that acts on when sigquit is sent by linux
     *  it will call the end member of the input.
     */
    setSignalHandler();

    /** set up the TCP/SOAP server and connect its provided signals to the
     *  appropriate slots fo the input and the workers
     */
    SoapServer::shared_pointer server;
    if(!noSoap)
    {
      server = SoapServer::instance(soap_port);
    }

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
    if(!suppressrate)
      plotter.start();
    InputBase::reference().start();
    InputBase::reference().wait();

    /** now stop the other threads */
    Workers::reference().end();

    /** @todo find out how to stop the other threads */
  }
  catch (const invalid_argument &error)
  {
    Log::add(Log::ERROR,string("User input is wrong: ") + error.what());
    cout << "User input is wrong: Please review the log file '"
         <<Log::filename()<<"'"<<endl;
  }
  catch (const runtime_error &error)
  {
    Log::add(Log::ERROR,string("Runtime error: ") + error.what());
    cout << "Bad error: Please review the log file '"
         <<Log::filename()<<"'"<<endl;
  }
  catch (const out_of_range &error)
  {
    Log::add(Log::ERROR,string("Out of range error: ") + error.what());
    cout << "Bad error: Please review the log file '"
         <<Log::filename()<<"'"<<endl;
  }
  catch (...)
  {
    Log::add(Log::ERROR,"main(): something bad happend, quitting the program.");
    cout << "Bad error: Please review the log file '"
         <<Log::filename()<<"'"<<endl;
  }
  Log::add(Log::INFO,"Quitting CASS");
  cout << endl;
}
