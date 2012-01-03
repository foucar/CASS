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
#include "analyzer.h"
//#include "daemon.h"
#include "input_base.h"
#include "file_input.h"
#include "multifile_input.h"
#include "format_converter.h"
#include "ratemeter.h"
#include "rate_plotter.h"
#include "ringbuffer.h"
#include "sharedmemory_input.h"
#include "tcp_input.h"
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
class CommandlineArgumentParser;

/** command line argument parser
 *
 * object that will parse the command line parameters and set the switches and
 * retrieve the arguments
 *
 * @author Lutz Foucar
 */
class CommandlineArgumentParser
{
public:
  /** a container type for switches */
  typedef map<string,pair<bool*,string> >  switches_t;

  /** a container type for switches */
  typedef map<string,pair<int*,string> >  intarguments_t;

  /** a container type for switches */
  typedef map<string,pair<string*,string> >  stringarguments_t;

  /** output which commandline parameters are available */
  void usage()
  {
    switches_t::iterator boolarg(_switches.begin());
    switches_t::iterator boolargEnd(_switches.end());
    for (;boolarg != boolargEnd; ++boolarg)
    {
      cout << boolarg->first <<":"<<boolarg->second.second<<endl;
    }

    intarguments_t::iterator intarg(_intargs.begin());
    intarguments_t::iterator intargEnd(_intargs.end());
    for (;intarg != intargEnd; ++intarg)
    {
      cout << intarg->first <<":"<<intarg->second.second
           <<" Default value is '"<<*(intarg->second.first)<<"'"<<endl;
    }

    stringarguments_t::iterator strarg(_stringargs.begin());
    stringarguments_t::iterator strargEnd(_stringargs.end());
    for (;strarg != strargEnd; ++strarg)
    {
      cout << strarg->first <<":"<<strarg->second.second
           <<" Default value is '"<<*(strarg->second.first)<<"'"<<endl;
    }
  }

  /** operator to parse the argumetns
   *
   * the arguments are retrieved as a QStringList from Qt. Go through the list
   * and try to find the parameter in the containers. If it is the switches
   * container simply set the switch to true. Otherwise take the next parameter
   * that should be the argumetn of the preceding parameter.
   * Start at the 2nd argument of the list, since the first is just the 
   * program name.
   *
   * @param argumentList the list of arguments
   */
  void operator()(const QStringList& argumentList)
  {
    QStringList::const_iterator argument(argumentList.constBegin()+1);
    for (; argument != argumentList.constEnd(); ++argument)
    {
      switches_t::iterator boolarg(_switches.find(argument->toStdString()));
      if (boolarg != _switches.end())
      {
        *(boolarg->second.first) = true;
        continue;
      }
      intarguments_t::iterator intarg(_intargs.find(argument->toStdString()));
      if (intarg != _intargs.end())
      {
        ++argument;
        *(intarg->second.first) = argument->toInt();
        continue;
      }
      stringarguments_t::iterator stringarg(_stringargs.find(argument->toStdString()));
      if (stringarg != _stringargs.end())
      {
        ++argument;
        *(stringarg->second.first) = argument->toStdString();
        continue;
      }

      cout << "CommandlineArgumentParser(): parameter '" << argument->toStdString()
           << "' is unknown. Possible values for this version of CASS are: "
           << endl;
      usage();
      exit(2);
    }
  }

  /** add a switch to the switches container
   *
   * @param sw the name of the parameter to look for
   * @param desc the description of the parameter
   * @param val a reference to the value that should be changed.
   */
  void add(const string &sw, const string& desc, bool &val)
  {
    _switches[sw] = make_pair(&val,desc);
  }

  /** add a switch to the string container
   *
   * @param sw the name of the parameter to look for
   * @param desc the description of the parameter
   * @param val a reference to the value that should be changed.
   */
  void add(const string &sw, const string& desc, string &val)
  {
    _stringargs[sw] = make_pair(&val,desc);
  }

  /** add a switch to the int container
   *
   * @param sw the name of the parameter to look for
   * @param desc the description of the parameter
   * @param val a reference to the value that should be changed.
   */
  void add(const string &sw, const string& desc, int &val)
  {
    _intargs[sw] = make_pair(&val, desc);
  }

private:
  /** container for the switches */
  switches_t _switches;

  /** container for the string arguments */
  stringarguments_t _stringargs;

  /** container for the int arguments */
  intarguments_t _intargs;
};

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
#else
    bool tcp(false);
    parser.add("-t","enable the tcp input",tcp);
    string partitionTag("0_1_cass_AMO");
    parser.add("-p","partition tag for accessing the shared memory",partitionTag);
    int index(0);
    parser.add("-c","client id for shared memory access",index);
#endif
#ifdef SOAPSERVER
    int soap_port(12321);
    parser.add("-s","TCP port of the soap server ",soap_port);
#endif
    bool suppressrate(false);
    parser.add("-r","suppress the rate output",suppressrate);
    string outputfilename("output.ext");
    parser.add("-o","output filename passed to the postprocessor",outputfilename);
    string settingsfilename(settings.fileName().toStdString());
    parser.add("-f","complete path to the cass.ini to use",settingsfilename);
    bool showUsage(false);
    parser.add("-h","show this help",showUsage);

    parser(QCoreApplication::arguments());

    //  //parse command line options
    //  int c;
    //  while((c = getopt(argc, argv, "rmqp:s:c:i:o:f:")) != -1)
    //  {
    //    switch (c)
    //    {
    //#ifdef OFFLINE
    //    case 'q':
    //      quitwhendone = true;
    //      break;
    //    case 'i':
    //      filelistname = optarg;
    //      break;
    //    case 'm':
    //      multifile = true;
    //      break;
    //#else
    //    case 'p':
    //      partitionTag = optarg;
    //      break;
    //    case 'c':
    //      index = strtol(optarg, 0, 0);
    //      break;
    //#endif
    //#ifdef SOAPSERVER
    //    case 's':
    //      soap_port = strtol(optarg, 0, 0);
    //      break;
    //#endif
    //    case 'r':
    //      suppressrate = true;
    //      break;
    //    case 'f':
    //      settingsfilename = optarg;
    //      break;
    //    case 'o':
    //      outputfilename = optarg;
    //      break;
    //    default:
    //      throw invalid_argument("CASS: Parameter '" + string(1,c) +
    //                             "' with argument '" + optarg +
    //                             "' does not exist in current CASS configuration.");
    //      break;
    //    }
    //  }

    /** show help and exit if requested */
    if (showUsage)
    {
      parser.usage();
      exit(0);
    }

    /** set the user requested .ini file name */
    CASSSettings::setFilename(settingsfilename);

    /** create a ratemeter objects for input and worker and the plotter to plot
     *  the rates that are calculated.
     */
    Ratemeter inputrate;
    Ratemeter workerrate;
    RatePlotter plotter(inputrate,workerrate);

    /** create workers and requested inputs which need a ringbuffer for passing the
     *  the events from one to the other. Once created connect their terminated
     *  and finished signals such that they notify each other about that they are
     *  done processing the events. Also create the postprocessor singleton used
     *  by the worker to post process the events.
     */
    RingBuffer<CASSEvent,RingBufferSize> ringbuffer;
    PostProcessors::instance(outputfilename);
    Workers::instance(ringbuffer, workerrate);
#ifdef OFFLINE
    if (multifile)
      MultiFileInput::instance(filelistname, ringbuffer, inputrate, quitwhendone);
    else
      FileInput::instance(filelistname, ringbuffer, inputrate, quitwhendone);
#else
    if (tcp)
      TCPInput::instance(ringbuffer,inputrate);
    else
      SharedMemoryInput::instance(partitionTag, index, ringbuffer, inputrate);
#endif

    /** connect a own signal handler that acts on when sigquit is sent by linux
     *  it will call the end member of the input.
     */
    setSignalHandler();

    /** set up the TCP/SOAP server and connect its provided signals to the
     *  appropriate slots fo the input and the workers
     */
#ifdef SOAPSERVER
    EventGetter get_event(ringbuffer);
    HistogramGetter get_histogram;
    SoapServer::shared_pointer server(SoapServer::instance(get_event, get_histogram, soap_port));
#endif

    /** set up the optional http server */
#ifdef HTTPSERVER
    httpServer http_server(get_histogram);
#endif

    /** start worker and server threads, then start input and wait until input
     *  is done
     */
    Workers::reference().start();
#ifdef SOAPSERVER
    server->start();
#endif
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
  }
}



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "gnu"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:
