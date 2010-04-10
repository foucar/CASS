// Copyright (C) 2009,2010 Jochen Küpper
// Copyright (C) 2009,2010 Lutz Foucar
//
#include <iostream>
#include <QtGui/QApplication>

#include "cass.h"
#include "analyzer.h"
#include "sharedmemory_input.h"
#include "ringbuffer.h"
#include "format_converter.h"
#include "ratemeter.h"
#include "rate_plotter.h"
#include "tcpserver.h"
#include "worker.h"



/*!
@mainpage CASS (CFEL ASG Software Suite)
@section download Getting CASS
You can access cass via svn from the following Repository:\n
https://www.mpi-hd.mpg.de/repos/lutz/diode \n
The version we are working with right now is to be found in /branches/lutz
(please contact Lutz Foucar for access)
@section desc Brief description about program flow
<ul>
<li>Shared mem
  <ul>
  <li>get lcls data
  <li>takes out a cassevent from ringbuffer
  <li>converter covnerts lcls -> cassevent
  <li>puts it back to ringbuffer
  </ul>
<li>worker (mulitple)
  <ul>
  <li>takes cassevent out of ringbuffer
  <li>puts it to
    <ul>
    <li>analyzer
      <ul>
      <li>puts it to preanalyzers of different devices
      </ul>
    <li>postanalyzers
      <ul>
      <li>list of userdefined analyzers that extract info from cassevent and put results it in histograms
      </ul>
    </ul>
  <li>puts it back to ringbuffer
  </ul>
</ul>
program control is done via a tcpip interface\n
accesss histograms vi tcpip interface\n
parameters are loaded using qt's qsettings.
@section inst Installing CASS
@subsection pre Prerequisites
The following software packages need to be installed and available for building and running CASS:\n
  - Qt version 4.5.x or 4.6.x
@subsection env Needed Environment Variables
LCLSSYSINCLUDE: needs to point to the root folder of LCLS Stuff that contains the pdsdata folder\n
LCLSSYSLIB: needs to point to the folder the build libraries are in.\n
QTDIR:   needs to point where Qt is installed\n
@subsection build Building CASS
@subsubsection build_LCLS Compile the LCLS libraries
cd ${CASS top-level directory}/LCLS \n
make x86_64-linux
@subsubsection build_CASS Compile CASS
Go back to the CASS top-level directory and use qmake with appropriate options to create Makefiles, i. e.,\n
  cd ${CASS top-level directory}\n
  qmake -r\n
then run\n
  make\n
  (optional)sudo make install\n
to build and install the software.
@section run Running CASS
In Order to run the cass you need to start it with a parameter, the partition tag. When running
at LCLS in the daq state you need to provide the partition tag of the shared memory server you
want to connect to. \n
User settable parameters are to be found in "cass.ini". The later has to be in the folder where
you start CASS from.
@subsection testing Testing CASS in offline modus
For testing CASS you can create the shared memory using xtcmonserver located in the
build directory of the LCLS subfolder. This will take a xtc file and put the contents into the
shared memory. You have to give it serval commands which are documented when you run it without
any command. A typical start command will look like this:\n

LCLS/build/pdsdata/bin/x86_64-linux/xtcmonserver -f /lfs/l3/lcls/amo02809/e17-r0193-s00-c00.xtc -n 4 -s 0x1000000 -p test -r 120 -l\n
<ul>
<li>f: filename of file that you want to create the shared memory with
<li>n: number of datagrams to be stored in the buffer
<li>s: size of the buffer for that stores one datagram
<li>p: the name of the partition tag
<li>r: the rate that you want to simulate
<li>l: loop. If the end of the file has been reached start from the beginning
</ul>

@author Nicola Coppola \n
- depreciated cass_database\n
- depreciated cass_dictionary\n
- new pnCCD analysis\n
- Region of Interest (ROI) implementation\n
- CASS testing, debug and development\n

@author Lutz Foucar \n
- general leader \n
- cass, cass_acqiris, cass_ccd, cass_machinedata implementation \n
- CASS design, infrastructure development\n

@author Nils Kimmel \n
- original pnCCD analysis\n

@author Jochen Kuepper \n
- CASS design, infrastructure development\n
- cass framework implementation\n
- TCP server\n

@date 2009-2010

@todo make detector evaluation lazy by having the getters calc
      all values.
@todo look whether pnccd offsetcorrection works
@todo look whether photonit extraction works
@todo copy todo list from google to here
@todo ask Matt about an updated version of xtcmonserver.cc,
      that is capable of having the new send configure transition
      first capability included
@todo find out why there are the warning messages about const
      iterators (size_t) when compiling at SLAC
@todo document the parameters by the analyze functions of Acqiris
@todo Talk to Chris what range of event sequencing codes we will be
      assinged. Make sure that the range will be consequtive.
*/


/** the main program*/
int main(int argc, char **argv)
{
    // construct Qt application object
    QApplication app(argc, argv,false);
    // set up details for QSettings and Co. (So we can simply use QSettings settings; everywhere else.)
    QCoreApplication::setOrganizationName("CFEL-ASG");
    QCoreApplication::setOrganizationDomain("endstation.asg.cfel.de");
    QCoreApplication::setApplicationName("CASS");
    QSettings::setDefaultFormat(QSettings::IniFormat);

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
  // create workers//
  cass::Workers *workers(new cass::Workers(ringbuffer,qApp));
  //create a ratemeter object for the input//
  cass::Ratemeter *inputrate(new cass::Ratemeter(1,qApp));
  // create a ratemeter object for the worker//
  cass::Ratemeter *workerrate(new cass::Ratemeter(1,qApp));
  // create a rate plotter that will plot the rate of the worker and input//
  cass::RatePlotter *rateplotter(new cass::RatePlotter(*inputrate,*workerrate,qApp));

  //connect ratemeters//
  QObject::connect(workers, SIGNAL(processedEvent()), workerrate, SLOT(count()));
  QObject::connect(input,   SIGNAL(newEventAdded()),  inputrate,  SLOT(count()));


  // when the thread has finished, we want to close this application
  QObject::connect(input, SIGNAL(finished()), workers, SLOT(end()));
  QObject::connect(workers, SIGNAL(finished()), qApp, SLOT(quit()));

  // start input and worker threads
  input->start();
  workers->start();

  // TCP server
  // tell the server how to get an id or histogram
  cass::EventGetter get_event(ringbuffer);
  cass::HistogramGetter get_histogram(workers->histograms());
  cass::TCP::Server server(get_event, get_histogram, qApp);
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
  delete rateplotter;
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
