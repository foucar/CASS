// Copyright (C) 2009,2010 Jochen Küpper
// Copyright (C) 2009,2010 Lutz Foucar

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
#include "worker.h"


/** @mainpage CASS (CFEL ASG Software Suite)
 *
 * @section toc Table of Contents
 * -# @ref licence
 * -# @ref download
 * -# @ref desc
 * -# @ref inst
 * -# @ref run
 * -# @ref add_pp
 * -# @ref pplist
 * -# @ref cred
 *
 * @section licence License
 * CASS is delveloped under the terms of the GNU General Public
 * License, version 3 as of 29 June 2007. See @ref casslicense for details.
 *
 * If you use this software for publishable work, please cite and
 * acknowledge the authors and the CASS collaboration in your
 * publication. The suggested reference is:\n
 * CFEL-ASG Software System (CASS), developed by the CASS collaboration, 2009-2010.
 *
 * @section download Getting CASS
 * You can access cass via svn from the following Repository:\n
 * https://www.mpi-hd.mpg.de/repos/lutz/diode \n
 * The version we are working with right now is to be found in\n
 * /trunk\n
 * an "anonymous" readonly access for checking out can be done using\n
 * username: asg\n
 * password: cass\n
 * (please contact Lutz Foucar for write access)
 *
 * @section desc Brief description about program flow
 * There are two types of threads running. They are both accessing a shared entity
 * the ringbuffer. The producer thread is the shared memory input. The are several
 * worker threads that will work on the created cassevents. Here is a short
 * describtion of what the two thread types will do:
 * -# Shared Memory Thread (single thread)
 *   -# get lcls data
 *   -# takes out a cassevent from ringbuffer
 *   -# converter converts lcls -> cassevent
 *   -# puts it back to ringbuffer
 * -# Worker Thread (optional multiple threads)
 *   -# takes cassevent out of ringbuffer
 *   -# puts it to
 *    -# analyzer
 *      -# puts it to the preanalyzers of different devices
 *    -# postanalyzers
 *      -# list of userdefined analyzers that extract info from cassevent and
 *         put results it in histograms.
 *   -# puts worked on cassevent back to ringbuffer to be filled again
 *
 * - Program control is done via a tcpip interface
 * - Accesss histograms created by postprocessors via tcpip interface
 * - Parameters are loaded using qt's qsettings. @see @ref run
 *
 * @section inst Installing CASS
 * See the @ref cassinstall file as the primary installation documentation.
 *
 * @section run Running CASS
 * In Order to run the cass you need to start it with a parameter, the partition
 * tag. When running at LCLS in the daq state you need to provide the partition
 * tag of the shared memory server you want to connect to. \n
 * User settable parameters are to be found in "CASS.ini". The latter can be found
 * in the user scope path. From the QSettings documentary: "The default UserScope
 * paths on Unix and Mac OS X ($HOME/.config or $HOME/Settings) can be overridden
 * by the user by setting the XDG_CONFIG_HOME environment variable."
 * @see http://doc.trolltech.com/4.6/qsettings.html#setPath
 *
 * @subsection testing Testing CASS in offline mode (DAQ simulation)
 * For testing CASS you can create the shared memory using xtcmonserver located
 * in the build directory of the LCLS subfolder. This will take a xtc file and
 * put the contents into the shared memory. You have to give it serval commands
 * which are documented when you run it without any command. A typical start
 * command will look like this:\n
 *
 * LCLS/build/pdsdata/bin/x86_64-linux/xtcmonserver -f xtcfile.xtc -n 4
 * -s 0x1000000 -p test -r 120 -l\n
 * - f: filename of file that you want to create the shared memory with
 * - n: number of datagrams to be stored in the buffer
 * - s: size of the buffer for that stores one datagram
 * - p: the name of the partition tag
 * - r: the rate that you want to simulate
 * - l: loop. If the end of the file has been reached start from the beginning
 * - v: verbose output. Includes the spare time, which can be used to
 *         calculate the time in ns it took to read the event from file and put
 *         it into the shared memory (buisy time).\n
 *         sparetime = 1e9 / rate - buisy time.
 *
 * @subsection Running CASS with file input ("offlinecass")
 * If CASS has been compiled in offline mode (see the @ref cassinstall file),
 * then it will read data from XTC files on the filesystem rather than from
 * shared memory.  First, create a file called "filesToProcess.txt" in the
 * working directory containing the names of the XTC files to use.  Then
 * simply run CASS as normal, without the "-p" option.  All postprocessors
 * and options can be used as normal, and you can additionally enable
 * postprocessor #1001 for HDF5 file output.
 *
 * Running CASS in this offline mode is similar to using "xtcmonserver", but
 * there is one important difference:  The event buffer will be made to block
 * if events cannot be processed quickly enough, instead of skipping events.
 *
 * If you get weird HDF5 error messages when using pp1001, make sure your
 * HDF5 library was compiled with thread safety enabled.
 *
 *
 * @section cred Credits
 * @par Authors:
 *
 * Nicola Coppola
 * - depreciated cass_database
 * - depreciated cass_dictionary
 * - new pnCCD analysis
 * - Region of Interest (ROI) implementation
 * - CASS testing, debug and development
 *
 * Nils Kimmel
 * - original pnCCD analysis
 *
 * Jochen Kuepper
 * - CASS design, infrastructure development
 * - cass framework implementation
 * - postprocessor setup
 * - TCP/SOAP server
 *
 * @par Project admin:
 *
 * Lutz Foucar
 * - cass, cass_acqiris, cass_ccd, cass_machinedata implementation
 * - CASS design, infrastructure development
 *
 * @see @ref authors file in distribution for details
 *
 * @date 2009-2010
 *
 * @todo only retrieve the last ccd / waveform when requested
 * @todo describe how to use Nicolas ROI.
 * @todo create another acqiris detector, that will just measure the voltage
 *       on a given channel
 * @todo make detector evaluation lazy by having the getters calc all values.
 * @todo right now we cannot define a postprocessors dependency on the
 *       converter that it needs. We need to include this dependency.
 *       Maybe using cass.ini?
 * @todo create pp: cummulative average commercial ccd image with condition on
 *       tof and pos on delayline detector
 * @todo move main documentation to own file.
 * @todo next to the cass executable create a cass library so that the viewer
 *       can use the histogram part for it.
 * @todo find out how one can use xrefitem so that it will list consecutive
 *       items under one title, just like todo does.
 * @todo create pp that will convert all active pp histograms to root histo and
 *       store it in .root file.
 * @todo add possibilty in cassview to write 2d histograms as csv file
 * @todo add gui's to modify the postprocessors settings in cassviewer.
 * @todo make postprocessors is's to have pair<size_t,size_t> to be able to
 *       have userdefined number of instances of on pp
 * @todo create pp for beam energy
 * @todo create boolean AND and boolean OR 0d pp
 * @todo create boolean AND and boolean OR 0d pp
 * @todo create comparison 0d pp less than constant
 * @todo create comparison 0d pp greater than constant
 * @todo create comparison 0d pp equal to constant
 * @todo create pp that will histogram a 0d pp value
 * @todo create pp that will create an running average of a histogram
 * @todo create boolean xor 0d pp
 * @todo make a file with all convenience helpers
 */

/** \page authors Authors
 * @include "AUTHORS"
 */

/** \page casslicense License
 * @include "License"
 * @include "License.GPLv3"
 */

/** \page cassinstall INSTALL
 * @include "INSTALL"
 */


/** The main program.
 * @param i filename containing filesnames of xtcfiles to process
 * @param c client id for shared memory access
 * @param s soap port of the soap server
 * @param p partition tag for accessing the shared memory
 * @param o output filename passed to the postprocessor
 */
int main(int argc, char **argv)
{
  // construct Qt application object
  QApplication app(argc, argv,false);
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

  // register size_t as Qt meta type
  qRegisterMetaType<size_t>("size_t");

  //create a container for the partition tag
  int c;
  char partitionTag[128];
  // filename containing XTC filenames
  std::string filelistname("filesToProcess.txt");
  // filename of the output filename
  std::string outputfilename("output.root");
  // SOAP server port (default: 12321)
  size_t soap_port(12321);
  //the sharememory client index
  int index(0);
  //check if at least 1 param is given

  //get the partition string
  while((c = getopt(argc, argv, "p:s:c:i:o:")) != -1)
  {
    switch (c)
    {
    case 'p':
      strcpy(partitionTag, optarg);
      break;
    case 's':
      soap_port = strtol(optarg, 0, 0);
      break;
    case 'c':
      index = strtol(optarg, 0, 0);
      break;
    case 'i':
      filelistname = optarg;
      break;
    case 'o':
      outputfilename = optarg;
      break;
    default:
      std::cout << "please give me a partition tag" <<std::endl;
      return 2;
      break;
    }
  }


  //a ringbuffer for the cassevents//
  cass::RingBuffer<cass::CASSEvent,cass::RingBufferSize> ringbuffer;

#ifndef OFFLINE
  // create shared memory input object //
  cass::SharedMemoryInput *input(new cass::SharedMemoryInput(partitionTag,
                                                             index,
                                                             ringbuffer));
#else
  // create file input object
  cass::FileInput *input(new cass::FileInput(filelistname.c_str(),ringbuffer));
#endif

  //create workers//
  cass::Workers *workers(new cass::Workers(ringbuffer, outputfilename, qApp));
  //create a ratemeter object for the input//
  cass::Ratemeter *inputrate(new cass::Ratemeter(1,qApp));
  //create a ratemeter object for the worker//
  cass::Ratemeter *workerrate(new cass::Ratemeter(1,qApp));
  //create a rate plotter that will plot the rate of the worker and input//
  cass::RatePlotter *rateplotter(new cass::RatePlotter(*inputrate,*workerrate,qApp));
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

  //start input and worker threads
  workers->start();
  input->start();

  // TCP/SOAP server
  cass::EventGetter get_event(ringbuffer);
  cass::HistogramGetter get_histogram;
  cass::SoapServer *server(cass::SoapServer::instance(get_event, get_histogram, soap_port));
  server->start();
  QObject::connect(server, SIGNAL(quit()), input, SLOT(end()));
  QObject::connect(server, SIGNAL(readini(size_t)), input, SLOT(loadSettings(size_t)));
  QObject::connect(server, SIGNAL(readini(size_t)), workers, SLOT(loadSettings(size_t)));
  QObject::connect(server, SIGNAL(writeini(size_t)), workers, SLOT(saveSettings()));
  QObject::connect(server, SIGNAL(clearHistogram(size_t)), workers, SLOT(clearHistogram(size_t)));

  //start Qt event loop
  int retval(app.exec());

  //clean up
  server->destroy();
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



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "gnu"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:
