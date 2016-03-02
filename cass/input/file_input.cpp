// Copyright (C) 2009-2016 Lutz Foucar

/**
 * @file file_input.cpp file contains definition of xtcfile input
 *
 * @author Lutz Foucar
 */

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <stdexcept>

#include <QtCore/QFileInfo>

#include "file_input.h"

#include "cass_event.h"
#include "cass_settings.h"
#include "log.h"

using namespace std;
using namespace cass;

namespace cass
{
/** process a file
 *
 * @author Lutz Foucar
 */
class FileProcessor : public lmf::PausableThread
{
public:
  /** define the shared pointer of this */
  typedef std::tr1::shared_ptr<FileProcessor> shared_pointer;

  /** constructor
   *
   * set the filename and initializes all parameters for the thread to be
   * able to process the data
   *
   * @param filename The name of the file to process
   */
  FileProcessor(const string &filename)
    : _filename(filename),
      _file(filename.c_str(), ios::binary | ios::in),
      _counter(0)
  {
    /** load the right reader for the file type depending on its extension */
    _read = FileReader::instance(_filename);
    _read->loadSettings();
    Log::add(Log::INFO,"FileProcessor(): processing file '" + _filename +
             "' with file reader type '" + _read->type() + "'");
    _file.seekg (0, ios::end);
    _filesize = _file.tellg();
    _file.seekg (0, ios::beg);
    _read->readHeaderInfo(_file);
  }

  /** process the file */
  void runthis()
  {
    /** get a pointer to the calling thread */
    InputBase::shared_pointer::element_type& input(InputBase::reference());

    /** make a container with all event ids */
    vector<CASSEvent::id_t> ids;

    /** iterate through the file until we've reached the filesize */
    while((!input.shouldQuit()) && (_file.tellg() < _filesize))
    {
      /** retrieve a new element from the ringbuffer */
      InputBase::rbItem_t rbItem(input.getNextFillable());
      if (rbItem == input.ringbuffer().end())
        continue;

      /** fill the cassevent object with the contents from the file */
      bool isGood((*_read)(_file,*rbItem->element));
      if (!isGood)
        Log::add(Log::WARNING,"FileProcessor::run(): Event with id '"+
                 toString(rbItem->element->id()) + "' is bad: skipping Event");
      else
      {
        ++_counter;
        ids.push_back(rbItem->element->id());
      }
      rbItem->element->setFilename(_filename.c_str());
      input.newEventAdded(rbItem->element->datagrambuffer().size());
      input.ringbuffer().doneFilling(rbItem, isGood);
    }
    _file.close();
    /** find out whether all ids are unique within the event */
    sort(ids.begin(),ids.end());
    vector<CASSEvent::id_t>::iterator first(ids.begin());
    while((first = adjacent_find(first,ids.end())) != ids.end())
    {
      string output("File '"+_filename+"' has duplicate id '" + toString(*first)+"'");
      if (_read->type() == "xtc")
      {
        uint32_t seconds(static_cast<uint32_t>((*first & 0xFFFFFFFF00000000) >> 32));
        uint32_t fiducial(static_cast<uint32_t>((*first & 0x00000000FFFFFFFF) >> 8));
        output += ("(seconds '" + toString(seconds) + "', fiducial '" +
                   toString(fiducial) + "')");
      }
      Log::add(Log::ERROR,output);
      ++first;
    }
  }

  /** retrieve the progess within the file
   *
   * @return the current progress
   */
  double progress()
  {
    return static_cast<double>(_file.tellg()) / static_cast<double>(_filesize);
  }

  /** retrieve the number of events processed by this thread
   *
   *  @return the number of processed events
   */
  uint64_t nEventsProcessed() {return _counter;}

private:
  /** the filename to work on */
  string _filename;

  /** shared pointer to the actual reader */
  FileReader::shared_pointer _read;

  /** the file stream */
  ifstream _file;

  /** the size of the file */
  streampos _filesize;

  /** a counter for the events */
  uint64_t _counter;
};//end class FileProcessor

}//end namespace cass

void FileInput::instance(string filelistname,
                         RingBuffer<CASSEvent> &ringbuffer,
                         Ratemeter &ratemeter, Ratemeter &loadmeter,
                         bool quitWhenDone,
                         QObject *parent)
{
  if(_instance)
    throw logic_error("FileInput::instance(): The instance of the base class is already initialized");
  _instance = shared_pointer(new FileInput(filelistname,ringbuffer,ratemeter,loadmeter,quitWhenDone,parent));
}

FileInput::FileInput(string filelistname,
                     RingBuffer<CASSEvent> &ringbuffer,
                     Ratemeter &ratemeter, Ratemeter &loadmeter,
                     bool quitWhenDone,
                     QObject *parent)
  : InputBase(ringbuffer,ratemeter,loadmeter,parent),
    _quitWhenDone(quitWhenDone),
    _filelistname(filelistname)
{
  Log::add(Log::VERBOSEINFO, "FileInput::FileInput: constructed");
  load();
}

void FileInput::load()
{
  CASSSettings s;
  s.beginGroup("FileInput");
  _parallelize = s.value("Parallelize",false).toBool();
}

void FileInput::runthis()
{
  _status = lmf::PausableThread::running;
  Tokenizer tokenize;

  /** retrieve all files in a list from the file */
  Log::add(Log::VERBOSEINFO,"FileInput::run(): try to open filelist '" +
           _filelistname + "'");
  ifstream filelistfile(_filelistname.c_str());
  if (!filelistfile.is_open())
    throw invalid_argument("FileInput::run(): filelist '" + _filelistname +
                           "' could not be opened");
  vector<string> filelist(tokenize(filelistfile));
  filelistfile.close();

  /** go through the list of files and create a processor for each file and
   *  add them to the list of processors
   */
  vector<string>::const_iterator filelistIt(filelist.begin());
  vector<string>::const_iterator filelistEnd(filelist.end());
  for (;filelistIt != filelistEnd; ++filelistIt)
  {
    string filename(*filelistIt);
    QFileInfo info(QString::fromStdString(filename));
    /** if there was such a file then we want to load it */
    if (info.exists())
    {
      FileProcessor::shared_pointer fProc(new FileProcessor(filename));
      _fProcs.push_back(fProc);
    }
    else
      Log::add(Log::ERROR,"FileInput::run(): could not open '" + filename + "'");
  }
  /** process the files using the processors
   *
   * @note we don't need to check for quitting at this point and the loop below
   *       because the threads themselves will check if the input should quit
   *       and if it does they will shut down graciously, thus wait will only
   *       wait until all the threads are finished, no need to shortcut by not
   *       starting the threads. Also because using the shortcuts in these
   *       loops will prevent gathering the correct information about how many
   *       events have been analyzed so far.
   */
  fileProcessors_t::iterator pIt(_fProcs.begin());
  fileProcessors_t::iterator pEnd(_fProcs.end());
  for (;(!shouldQuit()) && (pIt != pEnd); ++pIt)
  {
    (*pIt)->start();
    if (!_parallelize)
      (*pIt)->wait();
  }

  /** wait until the processors are done and gather information about the
   *  number of events they processed.
   */
  uint64_t eventcounter(0);
  pIt = _fProcs.begin();
  for (; pIt != pEnd; ++pIt)
  {
    (*pIt)->wait();
    (*pIt)->rethrowException();
    eventcounter += (*pIt)->nEventsProcessed();
  }

  Log::add(Log::INFO,"FileInput::run(): Finished with all files.");
  if(!_quitWhenDone)
    while(!shouldQuit())
      this->sleep(1);
  Log::add(Log::VERBOSEINFO, "FileInput::run(): closing the input");
  Log::add(Log::INFO,"FileInput::run(): Analysed '" + toString(eventcounter) +
           "' events.");
}

double FileInput::progress()
{
  double progressSum(0.);
  for (fileProcessors_t::const_iterator it(_fProcs.begin()); it != _fProcs.end(); ++it)
    progressSum += (*it)->progress();
  return (progressSum / _fProcs.size());
}

uint64_t FileInput::eventcounter()
{
  uint64_t counter(0);
  for (fileProcessors_t::const_iterator it(_fProcs.begin()); it != _fProcs.end(); ++it)
    counter += (*it)->nEventsProcessed();
  return counter;
}
