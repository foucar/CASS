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

#include "multifile_input.h"

#include "cass_event.h"
#include "cass_settings.h"

using namespace std;
using namespace cass;


namespace cass
{
/** tokenize the file containing the files that we want to process
 *
 * will return a list containing all non empty lines of the file. Before
 * returning the list strip the 'new line' and 'line feed' from the line.
 * Also skip all lines that contain either a '#' or a ';'.
 *
 * @return vector of string  containing all non empty lines of the file
 * @param file the filestream to tokenize
 *
 * @author Lutz Foucar
 */
vector<string> tokenize(std::ifstream &file)
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
    /* don't read lines containing ';' or '#' */
    if(line.find(';') != string::npos || line.find('#') != string::npos)
    {
      continue;
    }
    lines.push_back(line);
    VERBOSEOUT(cout <<"tokenize(): adding '"
               <<line.c_str()
               <<"' to list"
               <<endl);
  }
  return lines;
}

/** retrieve the extension of a filename
 *
 * find the last occurence of '.' after which hopefully the extension comes
 *
 * @return string containing the extension
 * @param string containing the filename
 *
 * @author Lutz Foucar
 */
string extension(const string &filename)
{
  if(filename.find_last_of(".") != std::string::npos)
    return filename.substr(filename.find_last_of(".")+1);
  else
  {
    stringstream ss;
    ss <<"extension: the filename '"<<filename<<"' does not have a file extension.";
    throw invalid_argument(ss.str());
  }
}
}

MultiFileInput::MultiFileInput(const string& filelistname,
                               RingBuffer<CASSEvent,RingBufferSize> &ringbuffer,
                               bool quitWhenDone,
                               QObject *parent)
  :PausableThread(lmf::PausableThread::_run,parent),
    _ringbuffer(ringbuffer),
    _quit(false),
    _quitWhenDone(quitWhenDone),
    _filelistname(filelistname)
{
  VERBOSEOUT(cout<< "FileInput::FileInput: constructed" <<endl);
  loadSettings(0);
}

MultiFileInput::~MultiFileInput()
{
  VERBOSEOUT(cout<< "input is closed" <<endl);
}

void MultiFileInput::loadSettings(size_t /*what*/)
{
//  /** pause the thread if it is running */
//  VERBOSEOUT(cout << "File Input: Load Settings: suspend when we are running"
//             <<" before laoding settings"
//             <<endl);
//  if(isRunning())
//    suspend();
//  VERBOSEOUT(cout << "File Input: Load Settings: suspended. Now loading Settings"
//      <<endl);
//  CASSSettings s;
//  s.beginGroup("FileInput");
//  /** load the rewind info */
//  _rewind = s.value("Rewind",false).toBool();
//  /** load the right reader */
//  _read = FileReader::instance(s.value("FileType","xtc").toString().toStdString());
//  /** and load its settings */
//  _read->loadSettings();
//  /** then resume the thread */
//  VERBOSEOUT(cout << "File Input: Load Settings: Done loading Settings. Now Resuming Thread"
//      <<endl);
//  resume();
//  VERBOSEOUT(cout << "File Input: Load Settings: thread is resumed"<<endl);
}

void MultiFileInput::end()
{
  VERBOSEOUT(cout << "MultiFileInput::end(): received end signal" <<endl);
  _quit=true;
}


void MultiFileInput::run()
{
  VERBOSEOUT(cout<<"MultiFileInput::run(): try to open filelist '"
             <<_filelistname<<"'"<<endl);
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
//      cout <<"FileInput::run(): processing file '"<<filename<<"'"<<endl;
//      _read->newFile();
//      while(!file.eof() && !_quit)
//      {
//        pausePoint();
//        /** rewind if requested */
//        if (_rewind)
//        {
//          /** reset the rewind flag */
//          _rewind = false;
//          filelistIt = filelist.begin();
//          break;
//        }
//        /** retrieve a new element from the ringbuffer */
//        CASSEvent *cassevent(0);
//        _ringbuffer.nextToFill(cassevent);
//        /** fill the cassevent object with the contents from the file */
//        const bool isGood((*_read)(file,*cassevent));
//        cassevent->setFilename(filelistIt->c_str());
//        _ringbuffer.doneFilling(cassevent, isGood);
//        emit newEventAdded();
//      }
//      file.close();
    }
    else
      cout <<"FileInput::run(): could not open '"<<filename<<"'"<<endl;
  }
//  cout << "FileInput::run(): Finished with all files." <<endl;
//  if(!_quitWhenDone)
//    while(!_quit)
//      this->sleep(1);
//  cout << "FileInput::run(): closing the input"<<endl;
}


// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
