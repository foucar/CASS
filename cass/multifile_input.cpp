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
#include "file_parser.h"

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

}

MultiFileInput::MultiFileInput(const string& filelistname,
                               RingBuffer<CASSEvent,RingBufferSize> &ringbuffer,
                               bool quitWhenDone,
                               QObject *parent)
  :PausableThread(lmf::PausableThread::_run,parent),
    _ringbuffer(ringbuffer),
    _quit(false),
    _quitWhenDone(quitWhenDone),
    _filelistname(filelistname),
    _rewind(false)
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
  /** @todo set up the map of file readers that the user requests.
   */
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
  /** open the file containing the files to process, convert the contents to a
   *  vector of filenames.
   */
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

  /** create the resource and a lock for it that contains the pointers to the
   *  places in the files where one finds the data that corresponds to a given
   *  eventID.
   */
  eventmap_t eventmap;
  QReadWriteLock lock;

  /** iterate through the vector of filenames and for each filename create a
   * file parser that will parse this file. Then put the fileparser in the
   * container.
   */
  vector<FileParser::shared_pointer> parsercontainer;
  vector<string>::const_iterator filelistIt(filelist.begin());
  while (filelistIt != filelist.end())
  {
    if (_quit)
      break;

    string filename(*filelistIt++);
    FileParser::shared_pointer fileparser(FileParser::instance(filename,eventmap,lock));
    fileparser->start();
    parsercontainer.push_back(fileparser);
  }

  /** wait until all files are parsed */
  vector<FileParser::shared_pointer>::iterator fileparseIt(parsercontainer.begin());
  for (;fileparseIt!=parsercontainer.end();++fileparseIt)
    (*fileparseIt)->wait();

  /** Then iterator through the eventlist, read the contents of each file and
   *  put it intothe cassvent. For each entry in the eventlist, check whether
   *  all requested infos are present.
   *  If the data for this eventid is complete, retrieve a CASSEvent from the
   *  ringbuffer and use the file readers to retrieve the data from the file
   *  and convert them into the CASSEvent.
   */
  eventmap_t::iterator eventmapIt(eventmap.begin());
  while (eventmapIt != eventmap.end())
  {
    pausePoint();
    if (_rewind)
    {
      eventmapIt = eventmap.begin();
      continue;
    }

    uint64_t eventId(eventmapIt->first);
    filetypes_t &filetypes(eventmapIt->second);
    if (filetypes.size() != _filereaders.size())
      continue;

    CASSEvent *cassevent(0);
    _ringbuffer.nextToFill(cassevent);
    cassevent->id() = eventId;
    cassevent->setFilename(filelistIt->c_str());
    bool isGood(true);
    filetypes_t::iterator filetypesIt(filetypes.begin());
    for (;filetypesIt != filetypes.end();++filetypesIt)
    {
      FileReader &reader(*(_filereaders[filetypesIt->first]));
      ifstream &filestream(filetypesIt->second.getStream());
      isGood = reader(filestream,*cassevent) && isGood;
    }
    _ringbuffer.doneFilling(cassevent, isGood);
    emit newEventAdded();
  }

  cout << "MultiFileInput::run(): Finished with all files." <<endl;
  if(!_quitWhenDone)
    while(!_quit)
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
