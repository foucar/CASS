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
using tr1::bind;
using tr1::placeholders::_1;

MultiFileInput::MultiFileInput(const string& filelistname,
                               RingBuffer<CASSEvent,RingBufferSize> &ringbuffer,
                               bool quitWhenDone,
                               QObject *parent)
  :InputBase(ringbuffer,parent),
    _quitWhenDone(quitWhenDone),
    _filelistname(filelistname),
    _rewind(false)
{
  VERBOSEOUT(cout<< "FileInput::FileInput: constructed" <<endl);
  loadSettings(0);
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
}

void MultiFileInput::run()
{
  _status = lmf::PausableThread::running;

  Splitter extension;
  /** open the file containing the files to process, convert the contents to a
   *  vector of filenames.
   */
  VERBOSEOUT(cout<<"MultiFileInput::run(): try to open filelist '"
             <<_filelistname<<"'"<<endl);
  ifstream filelistfile(_filelistname.c_str());
  if (!filelistfile.is_open())
    throw invalid_argument("MultiFileInput::run(): filelist '"+_filelistname +
                           "' could not be opened");
  Tokenizer tokenize;
  vector<string> filelist(tokenize(filelistfile));

  /** create the resource and a lock for it that contains the pointers to the
   *  places in the files where one finds the data that corresponds to a given
   *  eventID.
   */
  event2positionreaders_t event2posreaders;
  QReadWriteLock lock;

  /** iterate through the vector of filenames and for each filename create a
   * file parser that will parse this file. Then put the fileparser in the
   * container.
   */
  vector<FileParser::shared_pointer> parsercontainer;
  vector<string>::const_iterator filelistIt(filelist.begin());
  while (filelistIt != filelist.end())
  {
    if (_control == _quit)
      break;
    string filename(*filelistIt++);
    cout <<"MultiFileInput::run(): parsing file '"<<filename<<"'"<<endl;
    FilePointer fp;
    fp._filestream =
        FilePointer::filestream_t(new ifstream(filename.c_str(), std::ios::binary | std::ios::in));
    fp._pos = fp._filestream->tellg();
    filereaderpointerpair_t readerpointer
        (make_pair(FileReader::instance(extension(filename)),fp));
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
  for (;fileparseIt!=parsercontainer.end();++fileparseIt)
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
    if (eventIt->second.size() != parsercontainer.size())
    {
      cout <<"MultiFileInput:run(): Event '"<<eventIt->first
           <<"' is incomplete, skipping event."<<endl;
      ++eventIt;
      continue;
    }
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
    emit newEventAdded();
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
