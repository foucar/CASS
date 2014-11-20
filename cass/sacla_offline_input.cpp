// Copyright (C) 2014 Lutz Foucar

/**
 * @file sacla_offline_input.cpp file contains definition of sacla offline input
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

#include <SaclaDataAccessUserAPI.h>

#include "sacla_offline_input.h"

#include "cass_settings.h"
#include "log.h"
#include "sacla_converter.h"

using namespace std;
using namespace cass;

namespace cass
{
/** A processor for a tag list
 *
 * processes a list of tags. The list is given by iterators to the first and one
 * beyond the last element that should be processed.
 *
 * @author Lutz Foucar
 */
class TagListProcessor : public QThread
{
public:
  /** typedef the shared pointer of this */
  typedef std::tr1::shared_ptr<TagListProcessor> shared_pointer;

  /** constructor
   *
   * inline set the provided parameters and intialize counter with 0
   *
   * @param liststart iterator to the start of the tag list
   * @param listend iterator to the end of the tag list
   * @param blNbr the beamline number of the experiment
   * @param highTagNbr the high tag number of the experiment
   */
  TagListProcessor(vector<int>::const_iterator liststart,
                   vector<int>::const_iterator listend,
                   int runNbr, int blNbr, int highTagNbr)
    : _liststart(liststart),
      _listend(listend),
      _runNbr(runNbr),
      _blNbr(blNbr),
      _highTagNbr(highTagNbr),
      _counter(0)
  {
  }

  /** process the tags on the list */
  void run()
  {
    /** load the right reader for the file type depending on its extension */
    SACLAConverter convert;
    convert.loadSettings();
    /** read and convert the info for each of the tags */
    vector<int>::const_iterator iter(_liststart);
    string output("TagListProcessor: The following tags will be processed by '" +
                  toString(this) + "' for beamline '" + toString(_blNbr) + "' (size is '" +
                  toString(distance(_liststart,_listend)) + "'):");
    for (; iter != _listend; ++iter)
      output += " '" + toString(*iter) + "',";
    Log::add(Log::VERBOSEINFO,output);

    InputBase::shared_pointer::element_type& input(InputBase::reference());

    iter = _liststart;
    for(;iter != _listend; ++iter)
    {
      /** retrieve a new element from the ringbuffer */
      InputBase::rbItem_t rbItem(InputBase::reference().ringbuffer().nextToFill());
      /** fill the cassevent object with the contents from the file */
      uint64_t datasize = convert(_runNbr,_blNbr,_highTagNbr,*iter,*rbItem->element);
      if (!datasize)
        Log::add(Log::WARNING,"TagListProcessor: Event with id '"+
                 toString(rbItem->element->id()) + "' is bad: skipping Event");
      else
        ++_counter;
      input.newEventAdded(datasize);
      input.ringbuffer().doneFilling(rbItem, datasize);
    }
  }

  /** retrieve the number of events processed by this thread
   *
   *  @return the number of processed events
   */
  uint64_t nEventsProcessed() {return _counter;}

private:
  /** iterator to the start of the list */
  vector<int>::const_iterator _liststart;

  /** iterator to the end of the list */
  vector<int>::const_iterator _listend;

  /** the run number for the experiment */
  int _runNbr;

  /** the beamline number for the experiment */
  int _blNbr;

  /** the first part of the tag (that doesn't change) */
  int _highTagNbr;

  /** a counter to count how many events (tags) have been processed */
  uint64_t _counter;
};

/** retrieve the list of tags and the assiciated high tag number
 *
 * @return false in case of an error, true otherwise
 * @param[out] taglist the taglist for the run and beamline
 * @param[out] highTagNbr the high tag number for the run at beamline
 * @param[in] blNbr the beamline number where the run was taken
 * @param[in] runNbr the run number for which the tags should be returned
 *
 * @author Lutz Foucar
 */
bool getCompleteTagList(vector<int> &taglist, int &highTagNbr, int blNbr, int runNbr)
{
  /** get the lowest and highest tag number for the run */
  int funcstatus,startTagNbr,endTagNbr = 0;
  funcstatus = ReadStartTagNumber(highTagNbr,startTagNbr,blNbr,runNbr);
  if (funcstatus)
  {
    Log::add(Log::ERROR,"getCompleteTagList: could not retrieve start tag of run '" +
             toString(runNbr) + "' at beamline '" + toString(blNbr) +
             "' Errorcode is '" + toString(funcstatus) + "'");
    return false;
  }
  funcstatus = ReadEndTagNumber(highTagNbr,endTagNbr,blNbr,runNbr);
  if (funcstatus)
  {
    Log::add(Log::ERROR,"getCompleteTagList: could not retrieve end tag of run '" +
             toString(runNbr) + "' at beamline '" + toString(blNbr) +
             "' Errorcode is '" + toString(funcstatus) + "'");
    return false;
  }

  /** get the tag list */
  Log::add(Log::VERBOSEINFO,"getCompleteTagList: get Taglist for tags between '" +
           toString(startTagNbr) + "' and '" + toString(endTagNbr) + "' with highTag '" +
           toString(highTagNbr)+ "' for run '" + toString(runNbr) + "' at beamline '" +
           toString(blNbr) + "'");
  funcstatus = ReadTagListInRange(&taglist,highTagNbr,startTagNbr,endTagNbr);
  if (funcstatus)
  {
    Log::add(Log::ERROR,"getCompleteTagList: could not retrieve taglist of run '" +
             toString(runNbr) + "' at beamline '" + toString(blNbr) +
             "'Errorcode is '" + toString(funcstatus) + "'");
    return false;
  }
  return true;
}
}//end namespace cass

void SACLAOfflineInput::instance(string runlistname,
                                 RingBuffer<CASSEvent> &ringbuffer,
                                 Ratemeter &ratemeter, Ratemeter &loadmeter,
                                 bool quitWhenDone,
                                 QObject *parent)
{
  if(_instance)
    throw logic_error("SACLAOfflineInput::instance(): The instance of the base class is already initialized");
  _instance = shared_pointer(new SACLAOfflineInput(runlistname,ringbuffer,ratemeter,loadmeter,quitWhenDone,parent));
}

SACLAOfflineInput::SACLAOfflineInput(string runlistname,
                                     RingBuffer<CASSEvent> &ringbuffer,
                                     Ratemeter &ratemeter, Ratemeter &loadmeter,
                                     bool quitWhenDone,
                                     QObject *parent)
        : InputBase(ringbuffer,ratemeter,loadmeter,parent),
          _quitWhenDone(quitWhenDone),
          _runlistname(runlistname)
{
  Log::add(Log::VERBOSEINFO, "SACLAOfflineInput::SACLAOFflineInput: constructed");
  load();
}

void SACLAOfflineInput::load()
{
  CASSSettings s;
  s.beginGroup("SACLAOfflineInput");
  _rewind = s.value("Rewind",false).toBool();
  _chunks = s.value("NbrThreads",1).toInt();
}

void SACLAOfflineInput::run()
{
  _status = lmf::PausableThread::running;
  Tokenizer tokenize;

  /** get a list of all runs to process */
  Log::add(Log::VERBOSEINFO,"SACLAOfflineInput::run(): try to open filelist '" +
           _runlistname + "'");
  ifstream runlistfile(_runlistname.c_str());
  if (!runlistfile.is_open())
    throw invalid_argument("SACLAOfflineInput::run(): filelist '" + _runlistname +
                           "' could not be opened");
  vector<string> runlist(tokenize(runlistfile));
  runlistfile.close();

  /** add an eventcounter */
  uint64_t eventcounter(0);

  /** iterate through the list of runs */
  vector<string>::const_iterator runlistIt(runlist.begin());
  vector<string>::const_iterator runlistEnd(runlist.end());
  while (runlistIt != runlistEnd)
  {
    if (_control == _quit)
      break;
    /** split the runname into the run and beamline combination */
    string runname(*runlistIt++);
    stringstream ss(runname);
    string str;
    vector<int> nbrs;
    while(getline(ss,str,','))
    {
      stringstream ssvalue(str);
      int value;
      ssvalue >> value;
      nbrs.push_back(value);
    }
    if (nbrs.size() < 2)
    {
      Log::add(Log::ERROR,"SACLAOfflineInput: Could not split information '" +
               runname + "' into a beamline number and runname");
      continue;
    }
    int blNbr(nbrs[0]);
    int runNbr(nbrs[1]);

    /** check if the runstatus is set to 'run ended' and thus the
     *  data is available to read */
    int runstatus(0);
    int funcstatus(0);
    funcstatus = ReadRunStatus(runstatus,blNbr,runNbr);
    if (funcstatus)
    {
      Log::add(Log::ERROR,"SACLAOfflineInput: could not retrieve run status of run '" +
               toString(runNbr) + "' at beamline '" + toString(blNbr) +
               "'. Errorcode is '" + toString(funcstatus) + "'");
      continue;
    }
    if (runstatus != 0)
    {
      Log::add(Log::ERROR,"SACLAOfflineInput: run '" + toString(runNbr) +
          "' at beamline '" + toString(blNbr) + "' has not finised yet");
      continue;
    }

    /** the rest of the line is a separated list of tags, add them to the id list */
    vector<int> taglist(nbrs.begin()+2,nbrs.end());
    int highTagNbr(0);

    /** if the user did not provide a tag list, get the tag list from the API */
    if (taglist.empty())
    {
      if (!getCompleteTagList(taglist,highTagNbr,blNbr,runNbr))
        continue;
    }
    /** otherwise check if the provided tags are part of the provided run */
    else
    {
      vector<int> completeTagList;
      if (!getCompleteTagList(completeTagList,highTagNbr,blNbr,runNbr))
        continue;
      vector<int>::const_iterator tagIter(taglist.begin());
      vector<int>::const_iterator taglistEnd(taglist.end());
      bool didnotfind(false);
      for (;tagIter != taglistEnd; ++tagIter)
      {
        if (find(completeTagList.begin(),completeTagList.end(),*tagIter) == completeTagList.end())
        {
          didnotfind = true;
          break;
        }
      }
      if (didnotfind)
      {
        Log::add(Log::ERROR,"SACLAOfflineInput: the provided tag '" +
                 toString(*tagIter) + "' is not part of run '" + toString(runNbr) +
                 "' at beamline '" + toString(blNbr) + "'");
        continue;
      }
    }

    /** split the taglist into the user requested amount of chunks
     *  and process the splittet tag list in separate threads
     */
    int chunksize = taglist.size() / _chunks;
    vector<TagListProcessor::shared_pointer> processors;
    for (int chunk(0); chunk < _chunks-1; ++chunk)
    {
      vector<int>::const_iterator chunkstart(taglist.begin() + (chunk*chunksize));
      vector<int>::const_iterator chunkend(taglist.begin() + (chunk+1)*chunksize);
      TagListProcessor::shared_pointer
          processor(new TagListProcessor(chunkstart,chunkend,runNbr,blNbr,highTagNbr));
      processor->start();
      processors.push_back(processor);
    }
    vector<int>::const_iterator chunkstart(taglist.begin() + ((_chunks-1)*chunksize));
    TagListProcessor::shared_pointer
        processor(new TagListProcessor(chunkstart,taglist.end(),runNbr,blNbr,highTagNbr));
    processor->start();
    processors.push_back(processor);

    /** wait until all threads are finished and sum up the total events */
    vector<TagListProcessor::shared_pointer>::iterator processorsIt(processors.begin());
    vector<TagListProcessor::shared_pointer>::iterator processorsEnd(processors.end());
    for (; processorsIt != processorsEnd; ++processorsIt)
    {
      (*processorsIt)->wait();
      eventcounter += (*processorsIt)->nEventsProcessed();
    }
  }

  Log::add(Log::INFO,"SACLAOfflineInput::run(): Finished with all runs.");
  if(!_quitWhenDone)
    while(_control != _quit)
      this->sleep(1);
  Log::add(Log::VERBOSEINFO, "SACLAOfflineInput::run(): closing the input");
  Log::add(Log::INFO,"SACLAOfflineInput::run(): Analysed '" + toString(eventcounter) +
           "' events.");
}
