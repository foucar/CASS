// Copyright (C) 2009, 2010 Lutz Foucar

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
  _rewind = s.value("Rewind",false).toBool();
  _new = s.value("useNewContainer",false).toBool()? "_new" :"";
}

void FileInput::run()
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

  /** add an eventcounter */
  uint64_t eventcounter(0);

  /** go through the list of files */
  vector<string>::const_iterator filelistIt(filelist.begin());
  vector<string>::const_iterator filelistEnd(filelist.end());
  while (filelistIt != filelistEnd)
  {
    if (_control == _quit)
      break;
    string filename(*filelistIt++);
    QFileInfo info(QString::fromStdString(filename));
    /** if there was such a file then we want to load it */
    if (info.exists())
    {
      ifstream file(filename.c_str(), ios::binary | ios::in);
      /** load the right reader for the file type depending on its extension */
      _read = FileReader::instance(filename + _new);
      _read->loadSettings();
      Log::add(Log::INFO,"FileInput::run(): processing file '" + filename +
               "' with file reader type '" + info.suffix().toStdString() + "'");
      file.seekg (0, ios::end);
      const streampos filesize(file.tellg());
      file.seekg (0, ios::beg);
      _read->readHeaderInfo(file);
      while(file.tellg() < filesize && _control != _quit)
      {
        pausePoint();
        /** rewind if requested */
        if (_rewind)
        {
          /** reset the rewind flag */
          _rewind = false;
          filelistIt = filelist.begin();
          break;
        }
        /** retrieve a new element from the ringbuffer */

        rbItem_t rbItem(_ringbuffer.nextToFill());
        /** fill the cassevent object with the contents from the file */
        bool isGood((*_read)(file,*rbItem->element));
        if (!isGood)
          Log::add(Log::WARNING,"FileInput: Event with id '"+
                   toString(rbItem->element->id()) + "' is bad: skipping Event");
        else
          ++eventcounter;
        rbItem->element->setFilename(filelistIt->c_str());
        _ringbuffer.doneFilling(rbItem, isGood);
        newEventAdded(rbItem->element->datagrambuffer().size());
      }
      file.close();
    }
    else
      Log::add(Log::ERROR,"FileInput::run(): could not open '" + filename + "'");
  }
  Log::add(Log::INFO,"FileInput::run(): Finished with all files.");
  if(!_quitWhenDone)
    while(_control != _quit)
      this->sleep(1);
  Log::add(Log::VERBOSEINFO, "FileInput::run(): closing the input");
  Log::add(Log::INFO,"FileInput::run(): Analysed '" + toString(eventcounter) +
           "' events.");
}
