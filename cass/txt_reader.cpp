// Copyright (C) 2011 Lutz Foucar

/**
 * @file txt_reader.cpp contains class to read txt ascii files
 *
 * @author Lutz Foucar
 */

#include <algorithm>

#include "txt_reader.h"

#include "cass_event.h"
#include "cass_settings.h"
#include "machine_device.h"
#include "cass.h"

using namespace cass;
using namespace MachineData;
using namespace std;

TxtReader::TxtReader()
  :_delim('\t')
{}

void TxtReader::loadSettings()
{
  CASSSettings s;
  s.beginGroup("TxtReader");
  _delim = s.value("Deliminator",'\t').toInt();
  _eventIdhead = s.value("EventIdHeader","Event ID 24 bits").toString().toStdString();
  s.endGroup();
}

bool TxtReader::operator ()(ifstream &file, CASSEvent& event)
{
  Splitter split;
  if(_newFile)
  {
    _newFile = false;
    string headerline;
    while (true)
    {
      getline(file, headerline);
      _headers.clear();
      split(headerline,_headers,_delim);
      if (!_headers.empty() &&
          !QString::fromStdString(headerline).contains("GMD	(GMD_DATA)	-	GMD main vlaues"))
        break;
    }
    cout <<"TextReader: the txt file contains the following variables:";
    vector<string>::const_iterator h(_headers.begin());
    for (; h != _headers.end();++h)
      cout <<"'"<<*h<<"',";
    cout <<endl;
  }

  string line;
  vector<double> values;
  while(true)
  {
    getline(file, line);
    values.clear();
    split(line,values,_delim);
    if (!values.empty())
      break;
  }
  if(_headers.size() != values.size())
    throw runtime_error("TextReader():There are not enough values for the amount of values suggested by the header");

  if (event.devices().find(CASSEvent::MachineData) == event.devices().end())
    throw runtime_error("TextReader():The CASSEvent does not contain a Machine Data Device");

  MachineDataDevice &md
    (*dynamic_cast<MachineDataDevice*>(event.devices()[CASSEvent::MachineData]));

  vector<double>::const_iterator value(values.begin());
  vector<string>::const_iterator head(_headers.begin());
  for (;value != values.end(); ++value, ++head)
  {
    md.BeamlineData()[*head] = *value;
//    cout << "'"<<*head<<"'="<<*value<<", ";
  }
//  cout <<endl;

  event.id() = md.BeamlineData()[_eventIdhead];

//  cout << "EventID '"<<_eventIdhead<<"'= "<<event.id()<<endl;
  if(!event.id())
    cout << "TxtReader: EventId is bad '"<<event.id()<<"': skipping Event"<<endl;
  return (event.id());
}
