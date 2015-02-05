CASS        {#mainpage}
====
[TOC]

CASS stands for (C)Fel-(A)SG (S)oftware (S)uite. It is a program designed
to visualize data taken at Free-electron-lasers (FEL). To be easily adoptable
to the various possible experiments it is designed to be consitsting of
'puzzle' pieces which can be stiched together to make CASS visualize the
data of interest for a given experiment.

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

Older versions of CASS are also available for download from the same location:

    http://www.mpi-hd.mpg.de/personalhomes/gitasg/Downloads/

Note that version without numeric version name were created when CASS was still
developed in svn and are therefore outdated and only availabe for historic
reasons.

### Retrieving the version controlled source code (git) {#get_git}
CASS is being developed using the distributed version control system 'git'.
One can clone a copy from the server using the following command

    git clone gitasg@lfs1.mpi-hd.mpg.de:cass

This requires that you have set up a ssh key pair and sent Lutz Foucar your
public key.

The recent stable version is contained in the 'master' branch.


Building CASS {#Building}
-------------
See the [INSTALL.md](@ref cassinstall) file for the prerequeries. Once you cloned / downloaded
the version you want to compile you need to do the following steps:

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

    make [-j]

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
directory. For this please install doxygen on your computer (see [INSTALL.md](@ref cassinstall)
for details). Then cd into the doc directory and use doxygen there
eg. if you are in the CASS base directory do:

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
CASS multiple input modes allow running online (get the data from a live datastream) and
offline (get the data from the written files).
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

gives a list which shared memory tags have been set up. The shared memory tag can
be passed to the program with the

    -p <partition tag>

parameter. If more than one person is running on the same machine trying to get
the data from the online stream it is possible to pass an id to each one with the

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

    /reg/g/cass/cass/devel/bin/cass_online

In this location you will also find some convenience startup scripts. As well
as the binary version of older versions of CASS.

To end the program you should use

    crtl + \

Killing the program with `crtl + c` had the potential to eat up communication
buffers for the interface to the shared memory communicatoin with LCLS. This
should be under control but be adviced that one might use up all available
buffer, such that one has to restart the shared memory server in order to be
able to retrieve live data again.


Using Offline {#running_offline}
-------------
The offline version of CASS will process the xtc file that were recorded by the
LCLS DAQ. Please put all files that you want to process into a txt file. Then
provide the name of the text file to CASS with the

    -i <filename containing filenames of xtcfiles to process>
parameter.

Usually CASS will not quit after it has finishes processing all the files. It
will keep all histograms in memory to be accessible via the viewers. If you would
like CASS to quit after it has processed all provided files you have to pass the

    -q

parameter to CASS at the program start.

Sometimes you don't want to have all the rate output, ie. when you run
the program in a batched way on a cluster. You can suppress the output with

    -r

Results of PostProcessors can optionally be saved to either a root or a hdf5 file.
One has to enable these options in cass_myconfiq.pri Refer to @ref cassinstall
for more details. The filename has to be given in the PostProcessor setup in
the ini file.

If you want to quit the program before the file has been fully analyzed but
still want to dump everything that has been processed so far to the file you
can quit the program with

    crtl + \

Using `crtl + c` will result in immediate stop of the program without the data
being written to file.


The Viewers {#viewers}
-----------
There are two options to look at the histograms that are created and filled by CASS,
@ref jocassview and @ref lucassview. Both viewers need to be told where to find
the CASS server and on which port it will listen for requests.


### Jocassview {#jocassview}
This viewer is based on QWT. A library for scientific widgets that are based on
QT.

At LCLS, one can also use the convenience startup script to start the viewer.


### Lucassview {#lucassview}
This viewer is based on ROOT which was developed by CERN.
Please visit <http://root.cern.ch> for details.
In order to run properly one has to provide a startup script to lucassview'er
which is called lucassStartup.C. Apart from the declaration of the appropriate server and
port, there are several options such as an automatic update.
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
the data as you want it. These "lego" pieces are called "PostProcessors".
For better understanding of this concept, here is a short overview of the
program flow:

for each event
- receive an event containing all data from either the file (offline) or the
  the live data stream
- pick out the information we are interested in (Converters)
- process the data using the PostProcessors

So the ini file exist of basically three parts:

The Converter part tells CASS what kind of data you are actually interested in.
To see what options are available please refer to @ref converter.

The processing part is the most advanced part. @ref pplist gives an overview
of all available postprocessors. The parenthesis refers to the
postprocessor to look up in the parameter list @ref casssetting to find out which
parameters it takes. In this reference you will also find all available
parameters that one can give in the .ini file. There are some examples given in
@ref examples as a starting point for your .ini file.

The special detector Paramters. Currently there are Acqiris-detectors and Pixel-
detectors that need such special parameters.

### The special Acqiris Detector Parameters {#acq_param}
If you want to use the PostProcessors that deal with the AcqirisDetectors you
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

* One can check into tmp/branchname. If you need write access to other branches
  ask the principle developer.

* Make sure the pushed code compiles.
  - if you made changes to the project, so that it will probably
    not compile for the other users, write a detailed description
    what one has to do in order to compile again to the cass
    mailinglist and into the [INSTALL.md](@ref cassinstall) file.

* Please provide details on your changes in the commit message.

* For further information please read the "How to add a feature to cass using git"


General program layout Information {#layout}
----------------------------------
CASS has two basic parts the input and analyze. Where the input part gets the data
from either file or shared memory and the analyze part will analyze the data.
Both of them are threaded, where the input is always just one thread and the
analyze part consists of more threads. The linkage between the input threads
and the analysis thread is done by cass::RingBuffer. This is a buffer that
contains the amount of cass::CASSEvent elements that can be chosen by setting
cass::RingBufferSize. It allows to retrieve elements that should be filled with
new data and elements whose contents should be analyzed.


### Input part of CASS {#cass_input}
Both inputs will do the following tasks after they retrieved the data from LCLS:
- get fillable (empty) cass::CASSEvent element from cass::RingBuffer
- convert LCLS data to cass::CASSEvent data and fill the later with the data
- put cass::CASSEvent back into cass::RingBuffer

To convert the data the cass::FormatConverter singleton is used. This will
iterate through the LCLS datagram and call the right cass::ConversionBackend
object that will convert the xtc data to cass::CASSEvent data. The user can
select what part of the data he wants to have converted by setting up which
converters he wants to use. Please refer to the documentation of
cass::FormatConverter and the converters for details.

#### Offline input of CASS {#cass_offline_input}
The input part that will read the data from file in offline mode is handled by
the class cass::FileInput. This just parses the input file containing the file
names to analyze, put them into string list and then goes through that list.
For each file it will retrieve an event and convert the data to cass::CASSEvent
object. The later was retrieved from the cass::RingBuffer and then later put back
into it.


#### Online input of CASS {#cass_online_input}
In the online mode the input part will read the data from the LCLS shared memory.
The corrosponding class is cass::SharedMemoryInput. It derives from the
Pds::XtcMonitorClient class provided by LCLS. This class will do all the
communication between the shared memory and CASS. Once it retrieved new data
(in LCLS terms this is called datagram) it will call the overwritten
cass::SharedMemoryInput::processDgram() member. Here the datagram will be
converted to a cass::CASSEvent which is retrieved from the cass::RingBuffer
before. After that it is put back into the cass::RingBuffer.



### Analysis part of CASS {#cass_analysis}
The analysis part is done by multiple threads. One can set the number of analysis
threads by modifying the cass::NbrOfWorkers variable. Each
analysis thread is a cass::Worker object, which are handled by the cass::Workers
object. Each one of the cass::Worker objects will do the following until it is
told to quit:
- retrieve an analyzable (non empty) cass::CASSEvent element from cass::RingBuffer
- pre analyze the cass::CASSEvent element using the cass::Analyzer singleton
- post analyze the cass::CASSEvent element using the cass::PostProcessors
  singleton.
- put the event back to the ringbuffer to be refilled again.

The cass::Analyzer singleton will pass the cass event to all user selected pre
analyzers to pre process the data. The cass::PostProcessor will pass the event
to all user defined postprocessors to be postprocessed. For further details on
how to select what postprocessors should run please see section @ref ini_file.
A list of analyzers can be found in cass::Analyzer in which you can also find
links to specific documentation of each analyzer.


Communicating with CASS {#cass_communication}
-----------------------
CASS is using SOAP to communicate with the viewers. The class that handles these
communication is cass::SoapServer. It uses a cass::HistogramGetter object to
retrieve a requested histogram from the available postprocessors.



The post processors {#postprocessors}
-------------------
Each post post processor has a list of pointers to histograms that it has
processed yet. This is to ensure that it will not evaluate the same event
over and over again. When it has already processed one event it will return the
resulting histogram associated with this event.

Some of the post processors do not have a separate result for each event,
i.e. the summing up and averaging post processors. They are based upon the
accumulating postprocessor base class.

To make sure that in the cass::PostProcessor::process() function one will
always use the right histograms a reference to it will be passed to this function.
The following happens in each post processor when cass::PostProcessors::process()
will call the cass::PostProcessor::processEvent() member of each normal
post processor:
- search in the histogram list whether there is a an entry that has the
  cass::CASSEvent::_id as first part of the pair.
- if so then just return the second part of the pair.
- if not, check if the condition is true and in this case evaluate it.

Adding new functionality to CASS {#add_functionality}
--------------------------------

### How to add a new PostProcessor {#add_pp}

A new "normal" PostProcessor needs to inherit from cass::PostProcessor. The most
important function that need to be overwritten are
cass::PostProcessor::loadSettings() and
cass::PostProcessor::process().
In cass::PostProcessor::loadSettings() you need to setup the
dependencies you are relying on, the general available parameters
and most importantly the resulting histogram. With
cass::PostProcessor::setupCondition() you set up that your PostProcessor has a
condition.

cass::PostProcessor::setupGeneral() will setup all the default parameters that
are available for post processors. Optionally you can use the function member
cass::PostProcessor::setupDependency() to set up the dependencies you
rely on.

To initialize the list of cached resulting histograms you need to call the
cass::PostProcessor::createHistList() member. This member needs to have a
shared pointer to the histogram that will work as a result of the PostProcessor
passed.

Most PostProcessors rely on the fact that a histograms shape or size will not be
changed during the processing step. (An exception to this rule are the table like
PostProcessors.) Therefore one needs to ensure that the result Histogram will
have the right size or shape during the load settings step.

The resulting histogram is write locked before calling the cass::PostProcessor::process()
function. There is no need to lock it again in definition of you PostProcessor.
However the Histogram of the PostProcessor you depend on is not locked. Therefore
one needs to readlock it before trying to read contents from it to ensure its
contents are not changed during the time one reads them. It is recommended to
use the locker facility that Qt provides. Eg:

    // retrieve the histogram for this event from the dependency (_pHist) and cast it to an 1d histogram
    const Histogram1DFloat& one
        (dynamic_cast<const Histogram1DFloat&>(_pHist->result(evt.id())));

    // lock the histogram for read access
    QReadLocker lock(&one.lock);

When you are done coding your postprocessor you need to make it available to
the user. To do this you need to complete the following steps:
- in cass/postprocessing/processor_manager.h add the number and a short description
  in the description part.
- in cass/postprocessing/processor_manager.h add an entry with your id in the
  cass::PostProcessors::id_t enum.
- in cass/postprocessing/processor_manager.cpp add your id in the
  cass::PostProcessors::create() member to the switch statement. Just use the
  other entries as example.

Please document what your postprocessor does so that other people now what it
does. When documenting please use doxygen style as then your documentation will
be available on the webserver. Documenting the parameters in cass.ini can be done
using the custom doxygen tag cassttng.

### How to add a new Converter {#add_converter}
Your converter has to inherit from cass::ConversionBackend and it should be a
singleton. This means that it should have a static member function called
instance that will return a cass::ConversionBackend::converterPtr_t object that
contains the singleton pointer.

In the constructor of the class you need to fill the
cass::ConversionBackend::_pdsTypeList with the type id's from the LCLS that you
want the converter to react on.\n
You need to overwrite cass::ConversionBackend::operator()() with the code that
extracts the desired data from the datagram and put it into the cass::CASSEvent.\n
Once you have set up your converter, you need to modify
cass::ConversionBackend::instance() and add an @c else-if-statment that returns
converter. Please document in cass/format_converter.h which string will return
your the singleton of your converter.


Update the LCLS library {#update_lcls_library}
-----------------------
The LCLS library is needed if one wants to parse the xtc files or the xtc file
stream. This library get updated very often by LCLS. Historically this library
was provided as a regular shared or static library that was generated from c++
source code. And updating it was just a matter of finding out which files have
been changed and then copy them into the local copy of the LCLS library.
Unfortunately in the beginning of 2013 a decision was made to provide the library
in a metalanguage that allows to export it to different computer languages. The
c++ version of parts of the library is completely different to what was used
before and it would require extensive effort to rewrite CASS to work with this
new implementation. Luckily however, if one wants to only update the already
existing parts, it can be done fairly easy, as the whole xtc part of the library
has not been converted to the new metalanguage.

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

When releasing a version (pushing a commit in the master branch to the repository)
you need to give a tag according to the version number described above first.


Authors {#authors}
=======

Lutz Foucar
-----------
- general leader
- CASS design, infrastructure development
- cass, cass_acqiris, cass_ccd (depreciated), cass_machinedata, cass_pixeldetector implementation
- new postprocessor framework
- all acqiris related postprocessors
- new jocassviewer
- new implementation of processing units

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
