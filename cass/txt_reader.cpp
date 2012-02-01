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
{}

void TxtReader::loadSettings()
{
  ExtractFilename filenameFromPath;
  CASSSettings s;
  s.beginGroup("TxtReader");
  s.beginGroup(QString::fromStdString(filenameFromPath(_filename)));
  _delim = s.value("Deliminator",'\t').toInt();
  _eventIdhead = s.value("EventIdHeader","Event ID 24 bits").toString().toStdString();
  _linesToSkip = s.value("LinesToSkip",3).toUInt();
  s.endGroup();
  s.endGroup();
}

void TxtReader::readHeaderInfo(std::ifstream &file)
{
  string headerline;
  string tmp;
  for (size_t i=0;i<_linesToSkip;++i)
    getline(file, tmp);
  _headers.clear();
  getline(file, headerline);
  _split(headerline,_headers,_delim);
  cout <<"TextReader: the txt file contains the following variables:";
  vector<string>::const_iterator h(_headers.begin());
  for (; h != _headers.end();++h)
    cout <<"'"<<*h<<"',";
  cout <<endl;
}

bool TxtReader::operator ()(ifstream &file, CASSEvent& event)
{
  string line;
  vector<double> values;
  while(true)
  {
    getline(file, line);
    values.clear();
    _split(line,values,_delim);
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
    md.BeamlineData()[*head] = *value;

  event.id() = md.BeamlineData()[_eventIdhead];

  return (event.id());
}
