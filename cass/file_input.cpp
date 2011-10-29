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

#include "file_input.h"

#include "cass_event.h"
#include "cass_settings.h"

using namespace std;
using namespace cass;

FileInput::FileInput(string filelistname,
                     RingBuffer<CASSEvent,RingBufferSize> &ringbuffer,
                     bool quitWhenDone,
                     QObject *parent)
  :QThread(parent),
  _ringbuffer(ringbuffer),
  _quit(false),
  _quitWhenDone(quitWhenDone),
  _filelistname(filelistname),
  _pause(false),
  _paused(false),
  _rewind(false)
{
  VERBOSEOUT(cout<< "FileInput::FileInput: constructed" <<endl);
  loadSettings(0);
}

cass::FileInput::~FileInput()
{
  VERBOSEOUT(cout<< "input is closed" <<endl);
}

void cass::FileInput::loadSettings(size_t /*what*/)
{
  /** pause the thread if it is running */
  VERBOSEOUT(cout << "File Input: Load Settings: suspend when we are running"
             <<" before laoding settings"
             <<endl);
  if(isRunning())
    suspend();
  VERBOSEOUT(cout << "File Input: Load Settings: suspended. Now loading Settings"
      <<endl);
  CASSSettings s;
  s.beginGroup("FileInput");
  /** load the rewind info */
  _rewind = s.value("Rewind",false).toBool();
  /** load the right reader */
  _read = FileReader::instance(s.value("FileType","xtc").toString().toStdString());
  /** and load its settings */
  _read->loadSettings();
  /** then resume the thread */
  VERBOSEOUT(cout << "File Input: Load Settings: Done loading Settings. Now Resuming Thread"
      <<endl);
  resume();
  VERBOSEOUT(cout << "File Input: Load Settings: thread is resumed"<<endl);
}

void cass::FileInput::suspend()
{
  /** set the pause flag */
  _pause=true;
  /** then wait until the thread suspended with the help of waitUntilSuspended */
  waitUntilSuspended();
}

void FileInput::pausePoint()
{
  if (_pause)
  {
    /** lock the mutex to prevent that more than one thread is calling pause */
    QMutexLocker locker(&_pauseMutex);
    /** set the status flag to paused */
    _paused=true;
    /** tell the wait until paused condtion that we are now pausing */
    _waitUntilpausedCondition.wakeOne();
    /** wait until the condition is called again */
    _pauseCondition.wait(&_pauseMutex);
    /** reset the paused status flag */
    _paused=false;
  }
}

void FileInput::resume()
{
  /** check if the thread has not been paused if so  return immidiately */
  if(!_pause)
    return;
  /** reset the pause flag */
  _pause=false;
  /** and tell run to resume by waking the one waiting on the _pauseCondition */
  _pauseCondition.wakeOne();
}


void FileInput::waitUntilSuspended()
{
  /** check if it is already paused, if so  retrun imidiatly */
  if(_paused)
    return;
  /** otherwise wait until the _waitUntilpausedCondition is waked by the pausePoint */
  QMutex mutex;
  QMutexLocker lock(&mutex);
  _waitUntilpausedCondition.wait(&mutex);
}

void FileInput::end()
{
  VERBOSEOUT(std::cout << "input got signal that it should close"
             <<std::endl);
  _quit=true;
}

vector<string> FileInput::tokenize(std::ifstream &file)
{
  vector<string> lines;
  while (!file.eof())
  {
    string line;
    getline(file,line);
    /* remove newline */
    if(line[line.length()-1] == '\n')
    {
      line.resize(line.length()-1);
    }
    /* remove line feed */
    if(line[line.length()-1] == '\r')
    {
      line.resize(line.length()-1);
    }
    /* dont read newlines */
    if(line.empty() || line[0] == '\n')
    {
      continue;
    }
    lines.push_back(line);
    VERBOSEOUT(cout <<"FileInput::tokenize(): adding '"
               <<line.c_str()
               <<"' to list"
               <<endl);
  }
  return lines;
}

void FileInput::run()
{
  VERBOSEOUT(cout<<"FileInput::run(): try to open filelist '"
             <<_filelistname<<"'"
             <<endl);
  ifstream filelistfile(_filelistname.c_str());
  if (!filelistfile.is_open())
  {
    stringstream ss;
    ss <<"FileInput::run(): filelist '"<<_filelistname<<"' could not be opened";
    throw invalid_argument(ss.str());
  }
  vector<string> filelist(tokenize(filelistfile));
  //go through all files in the list
  vector<string>::const_iterator filelistIt(filelist.begin());
  while (filelistIt != filelist.end())
  {
    if (_quit)
      break;
    string filename(*filelistIt++);
    VERBOSEOUT(cout<< "FileInput::run(): trying to open '"<<filename<<"'"<<endl);
    ifstream file(filename.c_str(), std::ios::binary | std::ios::in);
    //if there was such a file then we want to load it
    if (file.is_open())
    {
      cout <<"FileInput::run(): processing file '"<<filename<<"'"<<endl;
      file.seekg (0, ios::end);
      const streampos filesize(file.tellg());
      file.seekg (0, ios::beg);
      _read->readHeaderInfo(file);
      while(file.tellg() < filesize && !_quit)
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
        CASSEvent *cassevent(0);
        _ringbuffer.nextToFill(cassevent);
        /** fill the cassevent object with the contents from the file */
        bool isGood((*_read)(file,*cassevent));
        if (!cassevent->id())
        {
          isGood = false;
          cout << "FileInput: EventId is bad '"<<cassevent->id()<<"': skipping Event"<<endl;
        }
        cassevent->setFilename(filelistIt->c_str());
        _ringbuffer.doneFilling(cassevent, isGood);
        emit newEventAdded();
      }
      file.close();
    }
    else
      cout <<"FileInput::run(): could not open '"<<filename<<"'"<<endl;
  }
  cout << "FileInput::run(): Finished with all files." <<endl;
  if(!_quitWhenDone)
    while(!_quit)
      this->sleep(1);
  cout << "FileInput::run(): closing the input"<<endl;
}


// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
