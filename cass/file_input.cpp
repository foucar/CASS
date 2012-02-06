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

using namespace std;
using namespace cass;

void FileInput::instance(string filelistname,
                         RingBuffer<CASSEvent,RingBufferSize> &ringbuffer,
                         Ratemeter &ratemeter,
                         bool quitWhenDone,
                         QObject *parent)
{
  if(_instance)
    throw logic_error("FileInput::instance(): The instance of the base class is already initialized");
  _instance = shared_pointer(new FileInput(filelistname,ringbuffer,ratemeter,quitWhenDone,parent));
}

FileInput::FileInput(string filelistname,
                     RingBuffer<CASSEvent,RingBufferSize> &ringbuffer,
                     Ratemeter &ratemeter,
                     bool quitWhenDone,
                     QObject *parent)
  :InputBase(ringbuffer,ratemeter,parent),
  _quitWhenDone(quitWhenDone),
  _filelistname(filelistname)
{
  VERBOSEOUT(cout<< "FileInput::FileInput: constructed" <<endl);
//  loadSettings(0);
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

  VERBOSEOUT(cout<<"FileInput::run(): try to open filelist '"
             <<_filelistname<<"'"
             <<endl);
  ifstream filelistfile(_filelistname.c_str());
  if (!filelistfile.is_open())
    throw invalid_argument("FileInput::run(): filelist '" + _filelistname +
                           "' could not be opened");

  /** get a list of all filenames and go trhough that list */
  vector<string> filelist(tokenize(filelistfile));
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
      _read = FileReader::instance(info.suffix().toStdString()+_new);
      _read->setFilename(filename);
      _read->loadSettings();
      cout <<"FileInput::run(): processing file '"<<filename
           <<"' with file reader type '"<<info.suffix().toStdString()<<"'"<<endl;
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
        CASSEvent *cassevent(0);
        _ringbuffer.nextToFill(cassevent);
        /** fill the cassevent object with the contents from the file */
        bool isGood((*_read)(file,*cassevent));
        if (!isGood)
          cout << "FileInput: Event with id '"<<cassevent->id()<<"' is bad: skipping Event"<<endl;
        cassevent->setFilename(filelistIt->c_str());
        _ringbuffer.doneFilling(cassevent, isGood);
        newEventAdded();
      }
      file.close();
    }
    else
      cout <<"FileInput::run(): could not open '"<<filename<<"'"<<endl;
  }
  cout << "FileInput::run(): Finished with all files." <<endl;
  if(!_quitWhenDone)
    while(_control != _quit)
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
