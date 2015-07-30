CASS        {#mainpage}
====
[TOC]

CASS stands for (C)Fel-(A)SG (S)oftware (S)uite. It is a program designed
to analyze and visualize data taken at Free-electron-lasers (FEL). To be easily
adoptable to the various possible different types of experiments, it is based
on a highly modular design. The analysis part of CASS allows the user to set
up a analysis chain that can be tailored for each experimantal needs.
The section [general layout](#layout) describes the details of CASS.

Please find below the links to most important parts of the description

- @ref pplist
- @ref casssetting


Installation {#install}
============

See [INSTALL.md](@ref cassinstall) for short installation instructions.


Getting CASS source code {#get_source}
------------------------

### Downloading a tarball {#download}
You can download a recent tarball version of CASS from the following location

    http://www.mpi-hd.mpg.de/personalhomes/gitasg/Downloads/cass.latest.tar.gz

Older versions of CASS are also available for download from:

    http://www.mpi-hd.mpg.de/personalhomes/gitasg/Downloads/

Note that versions without a numeric version name were created when CASS was
still developed in svn and are therefore outdated. They area only availabe for
historic reasons.

### Retrieving the version controlled source code (git) {#get_git}
CASS is being developed using the distributed version control system 'git'.
One can clone a copy from the server using the following command

    git clone gitasg@lfs1.mpi-hd.mpg.de:cass

This requires that you have set up a ssh key pair and sent Lutz Foucar your
public key.

The recent stable version is contained in the 'master' branch.

#### Anonymous retrieval via GitHub
An anonymous clone of the latest master branch of the repository is accessable
via GitHub

    https://github.com/foucar/CASS

Building CASS {#Building}
-------------
See the [INSTALL.md](@ref cassinstall) file for the prerequeries. Once you
cloned / downloaded the version you want to compile you need to do the following
steps:

    cd /path/to/cass/directory/
    copy cass_defaultconfig.pri to cass_myconfig.pri

Now modify the the cass_myconfig.pri file to include what you need and
whether you want to use the offline and/or online version of CASS. Make sure
that you have set up all the paths correctly. Once you are done with this type:

    qmake

if you want to install the binaries later on in a directory other than
"/usr/local" you need to type

    qmake PREFIX=path/to/desired/location

Once you did this you can type

    make [-j5]

After it is done with compiling you have the choice to either leave the
binary in the ./bin folder and start CASS from there or you might want to
install it into the lockation that you requested with the optional PREFIX
parameter by typing

    make install

Now CASS is ready to run. Please see the @ref using section of the documentation
to see how to configure and run CASS. Parameters for the program start can be
found here: @ref clpar.

By enabling the in the cass_myconfig.pri file one can enable CASS to compile the
additional viewers that ship with CASS.


Documentation {#documentation}
------------

The Software Suite is documented using doxygen. One can create a html version
of the documentation by running doxygen on the "doc" subfolder of the CASS
directory. For this please install doxygen on your computer (see
[INSTALL.md](@ref cassinstall) for details). Then cd into the doc directory and
use doxygen there eg. if you are in the CASS base directory do:

    cd doc
    make doc

This will create a subdirectory of the doc directory called doxygen/html.
Within this directory you will find the html version of the documentation. Just
open index.html with your favorite browser to view the documentation.

An online version of the documentation that shows the latest stable version in
the master branch is located at

<http://www.mpi-hd.mpg.de/personalhomes/gitasg/cass>

In the doc location are also the example .ini files. All of the documentation
is in this file.


Using CASS {#using}
==========
CASS multiple input modes allow running online (get the data from a live
datastream) and offline (get the data from the written files).
Depending on the settings in cass_myconfig.pri one can compile CASS for either
online or offline mode. Please see @ref cassinstall for details.

A complete list of all available commandline parameters for the compiled mode
can be found by starting CASS with the help switch:

    cass --help
    cass -h

To quit CASS in either online or offline mode one can use

    crtl + \

Killing the program with

    crtl + c

has various implications. Please refer to the following sections for details.



Running Online at LCLS {#running_online}
----------------------
To run cass online at LCLS you need shared memory access to the data
provided by LCLS. This access is only available on selected dedicated computers.
For the AMO-Hutch these are

    daq-amo-mon01
    daq-amo-mon02
    daq-amo-mon03
    daq-amo-mon04

Please ask the LCLS DAQ people on which machine you will have access to the
online data stream.

When starting the online CASS program you need to determine where to find the
shared memory that has been preset by the DAQ people. Doing

    ls /dev/shm

gives a list which shared memory tags have been set up. The shared memory tag
can be passed to the program with the

    -p <partition tag>

parameter. If more than one person is running on the same machine trying to get
the data from the online stream it is possible to pass an id to each one with
the

    -c <client id>

parameter.

With the

    -s <server port>

parameter you tell the program on which port the cass histograms are provided to
the viewers. See @ref viewers for details on how to use this parameter in the
viewer.

The

    -f <path/to/.inifile>

parameter lets you choose which .ini file to use. Please see @ref ini_file
for details of the contents of the .ini file. If this parameter is not set CASS
will by default load CASS.ini that resides in the UserScope path. On Unix or Mac
OS X this is $HOME/.config or $HOME/Settings ie.

    ~/.config/CFEL-ASG/CASS.ini

A typical program start command looks like this:

    ./cass_online -p 0_1_cass_AMO -c 0 -f "inifilename"

For your convenience the most recent cass binary compiled by an expert is
located here

    /reg/g/cass/cass/%version%/bin/cass_online

In this location you will also find some convenience startup scripts. As well
as the binary version of older versions of CASS.

To end the program you should use

    crtl + \

Killing the program with `crtl + c` had the potential to eat up communication
buffers for the interface to the shared memory communication with LCLS. This
should be under control but be adviced that one might use up all available
buffer, such that one has to restart the shared memory server in order to be
able to retrieve live data again.


Using Offline at LCLS {#running_offline}
-------------
The offline version of CASS will process the xtc file that were recorded by the
LCLS DAQ. Please put all files that you want to process into a txt file. Then
provide the name of the text file to CASS with the

    -i <filename containing filenames of xtcfiles to process>
parameter.

Usually CASS will not quit after it has finishes processing all the files. It
will keep all histograms in memory to be accessible via the viewers. If you
would like CASS to quit after it has processed all provided files you have to
pass the

    -q

parameter to CASS at the program start.

Sometimes you don't want to have all the rate output, ie. when you run
the program in a batched way on a cluster. You can suppress the output with

    -r

Results of Processors can optionally be saved to either a root or a hdf5 file.
One has to enable these options in cass_myconfiq.pri Refer to @ref cassinstall
for more details. The filename has to be given in the Processor setup in
the ini file.

If you want to quit the program before the file has been fully analyzed but
still want to dump everything that has been processed so far to the file you
can quit the program with

    crtl + \

Using `crtl + c` will result in immediate stop of the program without the data
being written to file.


The Viewers {#viewers}
-----------
There are two options to look at the histograms that are created and filled by
CASS, @ref jocassview and @ref lucassview. Both viewers need to be told where to
find the CASS server and on which port it will listen for requests.


### Jocassview {#jocassview}
This viewer is based on QWT. A library for scientific widgets that are based on
QT.

At LCLS, one can also use the convenience startup script to start the viewer.


### Lucassview {#lucassview}
This viewer is based on ROOT which was developed by CERN.
Please visit <http://root.cern.ch> for details.
In order to run properly one has to provide a startup script to lucassview'er
which is called lucassStartup.C. Apart from the declaration of the appropriate
server and port, there are several options such as an automatic update.
All options in the script can also be typed in the root console manually. A
template of the startup script is located in  the lucassview folder of the
repository you have downloaded:

./cass/lucassview/lucassStartup.C

In order to increase the usability of root there are a collection of macros
available, which need to be loaded to root.

One of such a collection of macros is available at LCLS and can be found at

    /reg/g/cass/macros/MoritzLutzAchimTill.C



How to set up the .ini file {#ini_file}
---------------------------
CASS consists of a lot of "lego" pieces that one can put together to process
the data as you want it. These "lego" pieces are called "Processors".
For better understanding of this concept, here is a short overview of the
program flow:

for each event
- receive an event containing all data from either the file (offline) or the
  the live data stream
- pick out the information we are interested in (Converters)
- process the data using the Processors

So the ini file exist of basically three parts:

The Converter part tells CASS what kind of data you are actually interested in.
To see what options are available please refer to @ref converter.

The processing part is the most advanced part. @ref pplist gives an overview
of all available Processors. The parenthesis refers to the
Processor to look up in the parameter list @ref casssetting to find out which
parameters it takes. In this reference you will also find all available
parameters that one can give in the .ini file. There are some examples given in
@ref examples as a starting point for your .ini file.

The special detector Paramters. Currently there are Acqiris-detectors and Pixel-
detectors that need such special parameters.

### The special Acqiris Detector Parameters {#acq_param}
If you want to use the Processors that deal with the AcqirisDetectors you
need to set up the AcqirisDetectors first. Basically there are two types of
Acqiris Detectors:
- Time of Flight detectors (ToF)
- Delayline detectors (DLD)

where the Delayline detectors are Time of Flight detectors with additional spatial
information. So everything that a Time of Flight detector can do, also a
Delayline detector will do, but more. Basically the concept behind these
detectors is that a ToF detector has a cass::ACQIRIS::SignalProducer which is
called MCP. The SignalProducer contains a SingalExtractor object that knows
where to find the signals in the data and how to extract them from there. The
parameter @c SignalExtractionMethod lets you choose what kind of SignalExtractor
should be used to extract the signals from the data. Please refer to the
specific SignalExtrator for its available parameters.

Next to an MCP SignalProducer the delay line detector (DLD) has also
cass::ACQIRIS::AnodeLayer which
contain wireends which themselves are again cass::ACQIRIS::SignalProducer. So
one can set up how to extract the produced signals from the wireends as
described for the MCP SignalProducer.

To be able to convert or resort the Signals produced by the MCP
and the wireends of a DLD to actual hits on the detector the DLD also contains
a DetectorAnalyzer object. There are several ways to sort the signals into
detectorhits. You can choose which one of these methods you want to use with the
@c AnalysisMethod parameter. Please refer to the description of the specific
DetectorAnalyzer to see what parameters are available.

Once you have set up the DLD to identify hits, these hits can be refered to
certain particles.  There are several ways of identifying which hit
belongs to which particle. You can choose which one to use with the @c ConditionType
parameter of the particle to define which method to use. For the other parameters
available for a particle please refer to the documentation of
cass::ACQIRIS::Particle. In order to calculate the momenta and energy of the
particle each particle contains a cass cass::ACQIRIS::Spectrometer object. You
need to set up this object for each particle individually. Based on the parameters
you provide the particle will decide which method it will use to calculate the
momentum. In this the raw values of the detectorhit are taken and corrected
using a cass::ACQIRIS::HitCorrector object. Please refer to the documentation
of cass::ACQIRIS::HitCorrector to see which parameters are available.


### The special Pixel-detector Parameters {#pix_param}
This section has not been written yet. Please refer to the @ref pplist and the examples
to find out how tho setup these.



CASS developer notes {#devel_notes}
====================

Coding Rules {#rules}
------------

These are some guidelines for the CASS software development.

* Prefer standard language features (including the C++ standard library and TR1)
  over external libraries. If you don't, please document your decision.

* Favor initialization over assignment
  (because initialization will always occur).

* Please document the code using inline comments on implementation
  details and doxygen comments for interfaces and implementation choices:

    /** short description title.
     * detailed description
     * parameters and return values using @param and @return.
     */

  or

    /** short description title
     *
     * detailed description
     *
     * parameters and return values using @param and @return.
     */

  Do not duplicate the code in the comment, augment it!

* Do not bloat the code (implementation) by useless empty lines,
  comments without content (i.e., essentially duplicating the code),
  etc.
  But please do structure/document header files well

* Please keep the indentation style consistent. Generally, indentation
  should be 2 spaces. Indent whenever a curly bracket opens a block.
  Do not use tabs, use spaces instead!
  (Define the indentation style for Emacs users in the Local Variables
  list for automatic indentation.)


Software repository {#repo_rules}
-------------------

* Push the code into descriptive branches, so that they can be reviewed by the
  principle developer to be merged to master.

* One can check in to tmp/branchname. If you need write access to other branches
  ask the principle developer.

* Alternatively you can sent patches to the principle developer.

* Make sure the pushed code compiles.
  - if you made changes to the project, so that it will probably
    not compile for the other users, write a detailed description
    what one has to do in order to compile again to the cass
    mailinglist and into the [INSTALL.md](@ref cassinstall) file.

* Please provide details on your changes in the commit message.

* For further information please read the "How to add a feature to cass using
  git"


General program layout Information {#layout}
----------------------------------
There are two versions of CASS availabe, the online and offline version. Both
differ in the choise of input modules that one can choose from and how the
ringbuffer behaves.
Each version of CASS has two basic parts, an input module and the analyze module.
The responsibility of the input module is to fetch the "raw" data from whichever
source is available and adapt it to CASS conventions. The data is then put into
a cass::CASSEvent.

The purpose of the analysis module is to process the data according to the
processing chain that the user provides with the help of the ini file. There are
multiple processing units, where each one will analyze one of the data in the
CASSEvents.

The linkage between the input threads and the analysis thread is done by
cass::RingBuffer. This is a buffer that contains the amount of cass::CASSEvent
elements that can be chosen by setting cass::RingBufferSize.
It allows to retrieve elements that should be filled with new data and elements
whose contents should be analyzed. The difference of the cass::RingBuffer
between the online and offline version of CASS is, that the online version
allows for elements that haven't been processed yet to be filled with new data.
The offline version ensures that only elements of the cass::RingBuffer that have
been processed by the analysis module can be filled with new data by the input
module.


### Available input modules in CASS {#cass_input}
The input modules will do the following tasks after they retrieved the data from
whichever source that they get the data from:
1. get fillable (empty) cass::CASSEvent element from cass::RingBuffer
2. convert retrieved data to cass::CASSEvent data and fill the later with the
   data.
3. put cass::CASSEvent back into cass::RingBuffer

#### Offline input modules of CASS
The following input modules are available in the offline version:

##### General File input
The input part that will read the data from one of the supported binary or ascii
file types is handled by the class cass::FileInput. This just parses the input
file containing the file names to analyze, put them into string list and then
goes through that list. Please refer to cass::FileInput for all options that
this input module provides to the user.

This input can read data files within the following file types:

- **XTC:**
  This is the data format used to record data at LCLS. To convert the xtc data
  the cass::FormatConverter singleton is used. This will iterate through the
  LCLS datagram and call the right cass::ConversionBackend object that will
  convert the xtc data to cass::CASSEvent data. The user can select what part of
  the data he wants to have converted by setting up which converters he wants to
  use. Please refer to the documentation of cass::FormatConverter and the
  converters for details.

- **lma:**
  This data format is used by AGAT, a standalone application to record data from
  ACQIRIS Digitizers with the possibility to remove zeros. No further options
  are available, when using this file reader.

- **txt:**
  This reader will read regular ascii files where the contents should be in csv
  format. The delimiter between the values can be chosen. For details about all
  user parameters please refer to the ini parameters of cass::TxtReader.

- **sss:**
  These files are generated by Per Johnsons program for VMI data which basically
  contain concatunated raw image data where every image is preceded by the event
  id. No further paramters are available for this reader.

- **frms6:**
  These files are generated by the HLL data acquisition program and contain the
  frame data of the pnCCD detector. No further paramters are available for
  this reader.

##### Multi-File input
The multi-file input allows to combine information that is distributed among
several files of the types that the general file input can read.
Please refer to cass::MultiFileInput to get an overview of all the parameters
the user can choose from.

##### HDF5 File input
As the HDF5 files are no ordinary binary files where the data is just
concatunated and one can parse them by serially reading the contents, a
dedicated input module had to be written to read hdf5 files. It is highly
recommended that one adds all the data related to the experiment into a single
HDF5 file to be read as it is currently not possible to combine information from
other sources with the data read in this input module.

The layout of the data within the HDF5 file should be such, that data from all
detectors for one event is bundled into a group. This input module will
determine all "root" groups and assumes that each root groups contains all the
information for one event. For more details and all available users options,
refer to cass::HDF5FileInput

##### SACLA offline input
At SACLA the data is not stored in files, but in a database. One can use one of
their tools to retrieve the data and let it be written into HDF5 file or one can
use a provided API to access all data. This input modules makes use of the
provided API and allows the user to access the data directly, hence speeding up
the offline analysis chain. One has to provide this input with all data that
one is interested in. For details please refer to cass::SACLAOfflineInput and
cass::SACLAConverter, where the latter will provide information about how to
extract the user requested data.


#### Online input modules of CASS
The following input modules are available in the online version:

##### LCLS shared memory input
This input module will read the xtc like data from the LCLS shared memory.
The corrosponding class is cass::SharedMemoryInput. It derives from the
Pds::XtcMonitorClient class provided by LCLS. This class will do all the
communication between the shared memory and CASS. Once new data is available
it will call the overwritten cass::SharedMemoryInput::processDgram() member,
where all the necessary steps for an input module will be performed. Parsing of
the xtc like data will be done with the same class that also parses the data in
offline mode cass::FormatConverter. For further details please refer to the
documentation of this class.

##### General TCP input
This class connects to a tcp server on port 9090. It will then retrieve data
from this server and parses it.
The data packets need have the following layout: First 32 bits should contain
the size of the following data packet. The following data packet then needs to
be deserialized according to the rules of the program that serves the data.
Please refer to cass::TCPInput for more details and user choosable parameters.
Two types of data sources are currently supported:
- **AGAT3:**
  Data from AGAT3, which is a standalone program to operate with ACQIRIS
  Digitizers.
-**RACOON:**
  This software suite was developed by the Max Planck semiconductor lab in
  Munich. It provides the data in the frms6 data format to the tcp receiver.

##### SACLA online input
At SACLA a specialized API is provided that allows access to the detector data
live. Meaning that as the data is acquired it is also provided to the user via
the API. Other data, that is typically stored in the database is made available
via the offline API, which is quite slow, thus it is currently not recommended
to retrieve additional data about the experiment. However, it is possible to do
so using this input module of CASS. One can retrieve the latest data, wihtin
which it is described what event id the latest data has. Using the event id, all
additional information about the experiment is retrieved.
Please refer to cass::SACLAOnlineInput for all user parameters and details.


### Analysis part of CASS {#cass_analysis}
The analysis part is done by multiple threads. One can set the number of
analysis threads by modifying the cass::NbrOfWorkers variable. Each
analysis thread is a cass::Worker object, which are handled by the cass::Workers
singleton. Each one of the cass::Worker objects will do the following until it
is told to quit:
1. Retrieve an analyzable cass::CASSEvent element from cass::RingBuffer
2. Analyze the cass::CASSEvent element using the user set analysis chain build
   by individual cass::Processors.
3. Put the event back to the ringbuffer to be refilled again.

For further details on how to select what Processors should run please see
section @ref ini_file.


Communicating with CASS {#cass_communication}
-----------------------
CASS is using SOAP to communicate with the viewers. The class that handles these
communication is cass::SoapServer. There are several commands that are
understood by the CASS implementation of the server:

- **quit:**
  This tells CASS to quit itself.
- **readini:**
  This tells CASS to reload the ini file. It can be used to clear all data and
  restart over.
- **getPostprocessorIds:**
  This returns a list of all those Processors that are not marked as hidden.
- **clearHistogram:**
  This command will clear all the histograms of the Processor provided to
  this function.
- **controlDarkcal:**
  This broadcasts the received command to all Processors. Unlike the next
  function that only provides the received command to a specific Processor
- **receiveCommand:**
  This forwards a given command string to the requested Processor.
- **getHistogram:**
  Retrieves either the latest (when eventID is 0) histogram or a histogram for
  a specific eventID. The histogram is serialized and must be deserialized by
  the receiver of the data. One can determin what kind of histogram is is by
  checking the mimetype that is attached to the dime.


The Processors {#processors}
--------------
All Processors are managed by the cass::ProcessorManager singleton. This manager
will parse the ini file and set up the requested Processors and put them in
an internal list. Once all requested Processors have been created they will
be ordered correclty such that all dependencies will be processed before the
Processors that require the information from the Processors they depend
upon.

When processing the events the event data contained in cass::CASSEvent
will be passed to the invidividual Procssors in that order. After the event
has been passed to all Processors by the manager it tells all Processors
that the event with the given id can be released and can be overwritten by the
result of a different event.

Each regular Processor stores a list of pointers to histograms to cache the
events it has processed so far. The list is implemented by cass::CachedList.
This is to ensure that it is not needed to evaluate the same event if a
Processor in a parallel analysis chain, that evluates a different event,
requests the result of its event. Since there is only one instance of each
Processor this is the only way that multiple events can be anlysed in
parallel. When told to process an event a regular Processor will check if the
condition for the requested event is true. If this is the case it will get a new
event from the list and lock the contents for writing. It will then call the
cass::Processor::process function that should have been overwritten by the
individual Processor type. Once the event is processed by the Processor, the
result will be marked as the latest event into the list.

Some of the Processors do not have a separate result for each event,
i.e. the summing up and averaging Processors. These types of Processors
are based upon the cass::AccumulatingProcessor base class. Unlike the regular
Processors, this only holds one result. Thus when told to process an event it
will also check if the condition is true and if so it will lock the single
result for writing before calling the overwritten cass::Processor::process
member of the cass::Processor base class of this accumulating Processor class.


Adding new functionality to CASS {#add_functionality}
--------------------------------

### How to add a new Processor {#add_pp}

A new "normal" Processor needs to inherit from cass::Processor. The most
important function that need to be overwritten are
cass::Processor::loadSettings() and
cass::Processor::process().
In cass::Processor::loadSettings() you need to load all user information,
setup the dependencies you are relying on and most importantly the resulting
histogram. With cass::Processor::setupCondition() you set up that your
Processor has a condition.

cass::Processor::setupGeneral() will setup all the default parameters that
are available for Processors. Optionally you can use the function member
cass::Processor::setupDependency() to set up the dependencies you
rely on.

To initialize the list of cached resulting histograms you need to call the
cass::Processor::createHistList() member. This member needs to have a
shared pointer to the histogram that will work as a result of the Processor
passed.

Most Processors rely on the fact that a histograms shape or size will not be
changed during the processing step. (An exception to this rule are the table
like Processors.) Therefore one needs to ensure that the result Histogram will
have the right size or shape during the load settings step.

The resulting histogram is write locked before calling the
cass::Processor::process() function. There is no need to lock it again in
definition of your Processor. However the Histogram of the Processor you depend
on is not locked. Therefore one needs to readlock it before trying to read
contents from it to ensure its contents are not changed during the time one
reads them. It is recommended to use the locker facility that Qt provides. Eg:

    // retrieve the histogram for this event from the dependency (_pHist) and cast it to an 1d histogram
    const Histogram1DFloat& hist
        (dynamic_cast<const Histogram1DFloat&>(_pHist->result(evt.id())));

    // lock the histogram for read access
    QReadLocker lock(&(hist.lock));

When you are done coding your Processor you need to make it available to
the user. To do this you need to complete the following steps:
- in cass/postprocessing/processor_manager.h add the number and a short
  description in the description part.
- in cass/postprocessing/processor_manager.h add an entry with your id in the
  cass::ProcessorManager::id_t enum.
- in cass/postprocessing/processor_manager.cpp add your id in the
  cass::ProcessorManager::create() member to the switch statement. Just use the
  other entries as example.

Please document what your processor does so that other people now what it
does. When documenting please use doxygen style as then your documentation will
be available on the webserver. Documenting the parameters in cass.ini can be
done using the custom doxygen tag cassttng.

### How to add a new LCLS Converter {#add_converter}
Your converter has to inherit from cass::ConversionBackend and it should be a
singleton. This means that it should have a static member function called
instance that will return a cass::ConversionBackend::converterPtr_t object that
contains the singleton pointer.

In the constructor of the class you need to fill the
cass::ConversionBackend::_pdsTypeList with the type id's from the LCLS that you
want the converter to react on.\n
You need to overwrite cass::ConversionBackend::operator()() with the code that
extracts the desired data from the datagram and put it into the cass::CASSEvent.
Once you have set up your converter, you need to modify
cass::ConversionBackend::instance() and add an `else-if-statment` that returns
converter. Please document in cass/format_converter.h which string will return
your the singleton of your converter.


Update the LCLS library {#update_lcls_library}
-----------------------
The LCLS library is needed if one wants to parse the xtc files or the xtc file
stream. This library get updated very often by LCLS. Historically this library
was provided as a regular shared or static library that was generated from c++
source code. And updating it was just a matter of finding out which files have
been changed and then copy them into the local copy of the LCLS library.
Unfortunately in the beginning of 2013 a decision was made to provide the
library in a metalanguage that allows to export it to different computer
languages. The c++ version of parts of the library is completely different to
what was used before and it would require extensive effort to rewrite CASS to
work with this new implementation. Luckily however, if one wants to only update
the already existing parts, it can be done fairly easy, as the whole xtc part of
the library has not been converted to the new metalanguage.

## Find out which files have changed {#find_changed_files}
The LCLS library is kept within a svn repository. One can get the latest version
of the library as described [here](https://confluence.slac.stanford.edu/display/PSDM/Analysis+Setup)
Once one has a checkout version of the library one can see whether there have
been any changes by doing run the `svn up` command.

## How to update the files {#upate_changed_files}
Typically files that have changed in the xtc subfolder can just be copied. The
same is true for the XtcMonitorClient/Server/Msg files. The tricky part is the
BLD. Here one has to update the file

    LCLS/pdsdata/bld/bldData.hh

by comparing the contents with the contents of

    psddl/bld.ddl.h

In the latter everything is implemented completely in the header, so one has to
extract essential of a new class from this file and add it to the files in the
LCLS library.


How to add a feature to cass using git {#add_to_git}
--------------------------------------
The master branch should be linear, to be able to create a changelog from the
commit history at some point. Optimally all feature branches should be rebased
to a few single commits that would explain in the changelog what was done. For
now we just keep merging every commit to the master branch.

On your local repository ensure that everything is up to date:

    git pull origin

Make sure that the feature branch and the master branch are up to date. One
might have to checkout both branches and then redo the pull command.
Make sure that branch can be fast forward merged into master by rebasing it to
master.

    git checkout %feature branch name%
    git rebase master (only if cannot be fast forwarded)

Now merge the feature branch into master

    git checkout master
    git merge %feature branch name%

Give the new master branch head a new annotated tag following the Tagging rules

    git tag -a major.minor.bugfix -m”version major.minor.bugfix”

Push the tags to the repo server before pushing the changes in master. Otherwise
the tag number will not appear in the automatically generated documentation.

    git push origin --tags
    git push origin

The last command might take a while, because when the master branch has changed
a hook script will automatically create the new documentation and the new
binaries.

Then remove the feature branch from all the repositories (locals and remote)

    git push origin --delete %feature branch name%
    git branch -D %feature branch name%

Make sure that you update the other repositories that you might work on in
different places by doing a
    git pull (--rebase)
    git remote prune origin


Tagging {#taggin}
-------
The tags should follow the versioning scheme

    major.minor.bugfix

    1.2.3
    ^ ^ ^
    | | |
    | | +--- Build number
    | +----- Minor version number
    +------- Major version number

Try to use the RubyGems Rational Versioning policy in which:

- The Major version number is incremented when binary compatibility is broken
- The Minor version number is incremented when new functionality is added
- The Build number changes for bug fixes /typos

When releasing a version (pushing a commit in the master branch to the
repository) you need to give a tag according to the version number described
above first.


Authors {#authors}
=======

Lutz Foucar
-----------
- general leader
- CASS design, infrastructure development
- cass, cass_acqiris, cass_ccd (depreciated), cass_machinedata, cass_pixeldetector implementation
- new Processor framework
- all acqiris related Processors
- new jocassviewer
- new implementation of processing units
- inputs for usage at SACLA

Nicola Coppola
--------------
- cass_database (deprectiated)
- cass_dictionary (depreciated)
- pnCCD analysis (depreciated)
- Region of Interest (ROI) implementation (depreciated)
- CASS testing, debug and development

Uwe Hoppe
---------
- old jocassview - SOAP client/imageviewer (deprecitated)

Stephan Kassemeyer
-----------------
- Jocassview, plotwidget, spectrogramwidget (deprecitated)
- general bug hunting and fixing.

Nils Kimmel
----------
- cass_pnCCD (depreciated)

Jochen Küpper
------------
- CASS design, infrastructure development
- cass framework implementation (depreciated)
- postprocessor setup and synchronization (depreciated)
- TCP (SOAP) server and client

Mirko Scholz
------------
- Added calib cycle retrieval from xtc files
- Enabled readout of Gain/CTE Correction files from semiconductor lab.

Thomas White
------------
- pp1000 (HDF dump) and offline mode paragraph of the online documentation (depreciated)


Licence {#licence}
=======
CASS is delveloped under the terms of the GNU General Public
License, version 3 as of 29 June 2007. See LICENCE for details.

If you use this software for publishable work, please cite and
acknowledge the authors and the CASS collaboration in your
publication. The paper describing CASS is published in 'Computer Physics Communication'.
Please cite it as:
> Lutz Foucar; Barty, A.; Coppola, N.; Hartmann, R.; Holl, P.; Hoppe, U.; Kassemeyer, S.; Kimmel, N.; Küpper, J.; Scholz, M.; Techert, S.; White, T. A.; Strüder, L. & Ullrich, J
> CASS—CFEL-ASG software suite
> Computer Physics Communications, 2012, 183, 2207 - 2213
