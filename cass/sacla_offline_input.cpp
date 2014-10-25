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
}

void SACLAOfflineInput::run()
{
  _status = lmf::PausableThread::running;

  /** load the right reader for the file type depending on its extension */
  SACLAConverter convert;
  convert.loadSettings();

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
    if (nbrs.size() != 2)
    {
      Log::add(Log::ERROR,"SACLAOfflineInput: Could not split information '" +
               runname + "' into a beamline number and runname");
      continue;
    }
    int blNbr(nbrs[0]);
    int runNbr(nbrs[1]);

    /** get the list of tags for the run */
    /** first check if the runstatus is set to 'run ended' */
    int runstatus=0;
    if (ReadRunStatus(runstatus,blNbr,runNbr) != 0)
    {
      Log::add(Log::ERROR,"SACLAOfflineInput: could not retrieve run status of run '" +
               toString(runNbr) + "' at beamline '" + toString(blNbr) + "'");
      continue;
    }
    if (runstatus != 0)
    {
      Log::add(Log::ERROR,"SACLAOfflineInput: run '" + toString(runNbr) +
          "' at beamline '" + toString(blNbr) + "' has not finised yet");
      continue;
    }

    /** get the lowest and highest tag number for the run */
    int highTagNbr,startTagNbr,endTagNbr = 0;
    if (ReadStartTagNumber(highTagNbr,startTagNbr,blNbr,runNbr) != 0)
    {
      Log::add(Log::ERROR,"SACLAOfflineInput: could not retrieve start tag of run '" +
               toString(runNbr) + "' at beamline '" + toString(blNbr) + "'");
      continue;
    }
    if (ReadEndTagNumber(highTagNbr,endTagNbr,blNbr,runNbr) != 0)
    {
      Log::add(Log::ERROR,"SACLAOfflineInput: could not retrieve end tag of run '" +
               toString(runNbr) + "' at beamline '" + toString(blNbr) + "'");
      continue;
    }

    /** get the tag list */
    Log::add(Log::VERBOSEINFO,"SACLAOfflineInput: get Taglist for tags between '" +
             toString(startTagNbr) + "' and '" + toString(endTagNbr) + "' with highTag '" +
             toString(highTagNbr)+ "' for run '" + toString(runNbr) + "' at beamline '" +
             toString(blNbr) + "'");
    vector<int> taglist;
    if (ReadSyncTagList(&taglist,highTagNbr,startTagNbr,endTagNbr) != 0)
    {
      Log::add(Log::ERROR,"SACLAOfflineInput: could not retrieve taglist of run '" +
               toString(runNbr) + "' at beamline '" + toString(blNbr) + "'");
      continue;
    }


    /** read and convert the info for each of the tags */
    vector<int>::const_iterator taglistIter(taglist.begin());
    vector<int>::const_iterator taglistEnd(taglist.end());
    string output("SACLAOfflineInput: The following tags will be processed for run '" +
                  toString(runNbr) + "' at beamline '" + toString(blNbr) + "' (size is '" +
                  toString(taglist.size()) + "'):");
    for (; taglistIter != taglistEnd; ++ taglistIter)
      output += " '" + toString(*taglistIter) + "',";
    Log::add(Log::VERBOSEINFO,output);

    taglistIter = taglist.begin();
    while(taglistIter != taglistEnd && _control != _quit)
    {
      pausePoint();

      /** retrieve a new element from the ringbuffer */
      rbItem_t rbItem(_ringbuffer.nextToFill());
      /** fill the cassevent object with the contents from the file */
      bool isGood = convert(blNbr,highTagNbr,*taglistIter,*rbItem->element);
      if (!isGood)
        Log::add(Log::WARNING,"SACLAOfflineInput: Event with id '"+
                 toString(rbItem->element->id()) + "' is bad: skipping Event");
      else
        ++eventcounter;
      newEventAdded(rbItem->element->datagrambuffer().size());
      _ringbuffer.doneFilling(rbItem, isGood);

      ++taglistIter;
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
