// Copyright (C) 2009 LMF

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>

#include "file_input.h"
#include "pdsdata/xtc/Dgram.hh"
#include "cass_event.h"
#include "format_converter.h"

cass::FileInput::FileInput(std::string filelistname,
                           cass::RingBuffer<cass::CASSEvent,cass::RingBufferSize> &ringbuffer,
                           QObject *parent)
                             :QThread(parent),
                             _ringbuffer(ringbuffer),
                             _quit(false),
                             _filelistname(filelistname),
                             _converter(cass::FormatConverter::instance())
{
}

cass::FileInput::~FileInput()
{
  _converter->destroy();
  VERBOSEOUT(std::cout<< "input is closed" <<std::endl);
}

void cass::FileInput::loadSettings(size_t what)
{
  //pause yourselve//
  VERBOSEOUT(std::cout << "Shared Memory Input: Load Settings: suspend before laoding settings"
      <<std::endl);
  suspend();
  //load settings//
  VERBOSEOUT(std::cout << "Shared Memory Input: Load Settings: suspended. Now loading Settings"
      <<std::endl);
  _converter->loadSettings(what);
  //resume yourselve//
  VERBOSEOUT(std::cout << "Shared Memory Input: Load Settings: Done loading Settings. Now Resuming Thread"
      <<std::endl);
  resume();
  VERBOSEOUT(std::cout << "Shared Memory Input: Load Settings: thread is resumed"
      <<std::endl);
}

void cass::FileInput::suspend()
{
  _pause=true;
  //wait until you are paused//
  waitUntilSuspended();
}

void cass::FileInput::resume()
{
  //if the thread has not been paused return here//
  if(!_pause)
    return;
  //reset the pause flag;
  _pause=false;
  //tell run to resume via the waitcondition//
  _pauseCondition.wakeOne();
}

void cass::FileInput::waitUntilSuspended()
{
  //if it is already paused then retrun imidiatly//
  if(_paused)
    return;
  //otherwise wait until the conditions has been called//
  QMutex mutex;
  QMutexLocker lock(&mutex);
  _waitUntilpausedCondition.wait(&mutex);
}

void cass::FileInput::run()
{
  //create a list where all files that should be processed are in
  std::vector<std::string> filelist;

  //open the file with the filenames in it
  std::ifstream filelistfile;
  filelistfile.open(_filelistname.c_str());
  //put the names into a list of things that we want to process
  if (filelistfile.is_open())
  {
    std::cout <<"filelist \""<<_filelistname<<"\" is open"<<std::endl;
    //go through whole file
    while (!filelistfile.eof())
    {
      //read a line and put it into the file list
      std::string line;
      getline(filelistfile,line);
      /* remove newline */
      if(line[line.length()-1] == '\n')
      {
        line.resize(line.length()-1);
      }
      /* dont read newlines */
      if(line.empty() || line[0] == '\n')
      {
        continue;
      }
      filelist.push_back(line);
      std::cout <<"file \""<<line<<"\" added to processing list"<<std::endl;
    }
  }

  //make a pointer to a buffer
  cass::CASSEvent *cassevent;
  //go through all files in the list
  for (std::vector<std::string>::iterator filelistiterator = filelist.begin();
       filelistiterator != filelist.end();
       ++filelistiterator)
  {
    //quit if requested//
    if (_quit) break;

    //open the file
    std::ifstream xtcfile;
    xtcfile.open(filelistiterator->c_str(), std::ios::binary | std::ios::in);
    //if there was such a file then we want to load it
    if (xtcfile.is_open())
    {
      std::cout <<"processing file \""<<filelistiterator->c_str()<<"\""<<std::endl;
      //read until we are finished with the file
      while(!xtcfile.eof() && !_quit)
      {
        //pause execution if suspend has been called//
        if (_pause)
        {
          //lock the mutex to prevent that more than one thread is calling pause//
          _pauseMutex.lock();
          //set the status flag to paused//
          _paused=true;
          //tell the wait until paused condtion that we are now pausing//
          _waitUntilpausedCondition.wakeOne();
          //wait until the condition is called again
          _pauseCondition.wait(&_pauseMutex);
          //set the status flag//
          _paused=false;
          //unlock the mutex, such that others can work again//
          _pauseMutex.unlock();
        }
        //reset the cassevent pointer//
        cassevent=0;
        //retrieve a new element from the ringbuffer
        _ringbuffer.nextToFill(cassevent);
        //read the datagram from the file in the ringbuffer
        Pds::Dgram& dg = *reinterpret_cast<Pds::Dgram*>(cassevent->datagrambuffer());
        xtcfile.read(cassevent->datagrambuffer(),sizeof(dg));
        xtcfile.read(dg.xtc.payload(), dg.xtc.sizeofPayload());
        const bool isGood = _converter->processDatagram(cassevent);
        cassevent->setFilename(filelistiterator->c_str());
        //tell the buffer that we are done
        _ringbuffer.doneFilling(cassevent, isGood);
        emit newEventAdded();
      }
      //done reading.. close file
      xtcfile.close();
    }
    else
      std::cout <<"file \""<<filelistiterator->c_str()<<"\" could not be opened"<<std::endl;
  }
  while(!_quit)
    this->sleep(1);
  std::cout << "closing the input"<<std::endl;
}

void cass::FileInput::end()
{
  std::cout << "input got signal that it should close"<<std::endl;
  _quit=true;
}




// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
