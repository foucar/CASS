// Copyright (C) 2011 Lutz Foucar

/**
 * @file multifile_input.cpp file contains definition of file input reading
 *                           multiple files in parallel.
 *
 * @author Lutz Foucar
 */

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <tr1/functional>

#include <QStringList>

#include "multifile_input.h"

#include "cass_event.h"
#include "cass_settings.h"
#include "file_parser.h"

using namespace std;
using namespace cass;

namespace cass
{
}//end namespace cass

void MultiFileInput::instance(const string& filelistname,
                              RingBuffer<CASSEvent,RingBufferSize> &ringbuffer,
                              Ratemeter &ratemeter,
                              bool quitWhenDone,
                              QObject *parent)
{
  if(_instance)
    throw logic_error("MultiFileInput::instance(): The instance of the base class is already initialized");
  _instance = shared_pointer(new MultiFileInput(filelistname,ringbuffer,ratemeter,quitWhenDone,parent));
}

MultiFileInput::MultiFileInput(const string& filelistname,
                               RingBuffer<CASSEvent,RingBufferSize> &ringbuffer,
                               Ratemeter &ratemeter,
                               bool quitWhenDone,
                               QObject *parent)
  :InputBase(ringbuffer,ratemeter,parent),
    _quitWhenDone(quitWhenDone),
    _filelistname(filelistname),
    _rewind(false)
{
  VERBOSEOUT(cout<< "FileInput::FileInput: constructed" <<endl);
//  loadSettings(0);
  load();
}

MultiFileInput::~MultiFileInput()
{
  VERBOSEOUT(cout<< "MultiFileInput is closed" <<endl);
}

void MultiFileInput::load()
{
  CASSSettings s;
  s.beginGroup("MultiFileInput");
  _rewind = s.value("Rewind",false).toBool();
  _new = s.value("UseNewContainer",false).toBool()? "_new" :"";
  _nbrDifferentSources = s.value("NbrDifferentSources",2).toUInt();
}

void MultiFileInput::readEventData(event2positionreaders_t::iterator &eventIt)
{
  CASSEvent *cassevent(0);
  _ringbuffer.nextToFill(cassevent);
  cassevent->id() = eventIt->first;
  bool isGood(true);
  positionreaders_t &posreaders(eventIt->second);
  positionreaders_t::iterator fileposread(posreaders.begin());
  positionreaders_t::const_iterator posreadEnd(posreaders.end());
  for (; fileposread != posreadEnd; ++fileposread)
  {
    FilePointer &filepointer(fileposread->second);
    FileReader &read(*(fileposread->first));
    ifstream &filestream(filepointer.getStream());
    isGood = read(filestream,*cassevent) && isGood;
  }
  _ringbuffer.doneFilling(cassevent, isGood);
  newEventAdded();
}

void MultiFileInput::run()
{
  _status = lmf::PausableThread::running;

  /** create the resource and a lock for it that contains the pointers to the
   *  places in the files where one finds the data that corresponds to a given
   *  eventID. And create a container for all the file parser that we create in
   *  the next step. Create helpers to split the extension from the filename
   *  and to tokenize the list of filenames.
   */
  SplitExtension extension;
  Tokenizer tokenize;
  event2positionreaders_t event2posreaders;
  QReadWriteLock lock;
  vector<FileParser::shared_pointer> parsercontainer;

  /** open the file containing the files to process, convert the contents to a
   *  vector of filenames. Iterate through the vector of filenames and for each
   *  filename create a file parser that will parse this file. Then put the
   *  fileparser in the container. Also create a reader for the file and put it
   *  in the pair that contains the filepointer and the reader for it. Then read
   *  the header information into the file reader. Rewind the file to the
   *  beginning and
   */
  ifstream filelistfile(_filelistname.c_str());
  if (!filelistfile.is_open())
    throw invalid_argument("MultiFileInput::run(): filelist '"+_filelistname +
                           "' could not be opened");

  vector<string> filelist(tokenize(filelistfile));
  vector<string>::const_iterator filelistIt(filelist.begin());
  vector<string>::const_iterator filelistEnd(filelist.end());
  while (filelistIt != filelistEnd)
  {
    if (_control == _quit)
      break;
    string filename(*filelistIt++);
    cout <<"MultiFileInput::run(): parsing file '"<<filename<<"'"<<endl;
    FilePointer fp;
    fp._filestream =
        FilePointer::filestream_t(new ifstream(filename.c_str(), std::ios::binary | std::ios::in));
    if (!fp._filestream->is_open())
    {
      cout << "MultiFileInput::run(): could not open File '"<<filename<<"'"<<endl;
      continue;
    }
    fp._pos = fp._filestream->tellg();
    filereaderpointerpair_t readerpointer
        (make_pair(FileReader::instance(extension(filename)+_new),fp));
    readerpointer.first->setFilename(filename);
    readerpointer.first->loadSettings();
    readerpointer.first->readHeaderInfo(*fp._filestream);
    fp._filestream->seekg(0,ios::beg);
    fp._pos = fp._filestream->tellg();
    FileParser::shared_pointer fileparser
        (FileParser::instance(extension(filename),readerpointer,event2posreaders,lock));
    fileparser->start();
    parsercontainer.push_back(fileparser);
  }

  /** wait until all files are parsed */
  vector<FileParser::shared_pointer>::iterator fileparseIt(parsercontainer.begin());
  vector<FileParser::shared_pointer>::const_iterator fileparseEnd(parsercontainer.end());
  for (;fileparseIt!=fileparseEnd;++fileparseIt)
    (*fileparseIt)->wait();

  /** Then iterate through the eventlist, read the contents of each file and
   *  put it into the cassvent. For each entry in the eventlist, check whether
   *  all requested infos are present.
   *  If the data for this eventid is complete, retrieve a CASSEvent from the
   *  ringbuffer and use the file readers to retrieve the data from the file
   *  and convert them into the CASSEvent.
   */
  event2positionreaders_t::iterator eventIt(event2posreaders.begin());
  event2positionreaders_t::const_iterator eventEnd(event2posreaders.end());
  while (eventIt != eventEnd)
  {
    pausePoint();
    if (_rewind)
    {
      eventIt = event2posreaders.begin();
      continue;
    }

    /** check whether the event contains information from all files */
    if (eventIt->second.size() != _nbrDifferentSources)
    {
      cout <<"MultiFileInput:run(): Event '"<<eventIt->first
           <<"' is incomplete, skipping event."<<endl;
      ++eventIt;
      continue;
    }
    readEventData(eventIt);
    ++eventIt;
  }

  cout << "MultiFileInput::run(): Finished with all files." <<endl;
  if(!_quitWhenDone)
    while(_control != _quit)
      this->sleep(1);
  cout << "MultiFileInput::run(): closing the input"<<endl;
}


// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
