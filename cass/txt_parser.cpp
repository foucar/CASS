// Copyright (C) 2011 Lutz Foucar

/**
 * @file txt_parser.cpp contains class to parse txt ascii files
 *
 * @author Lutz Foucar
 */

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>

#include "txt_parser.h"

#include "cass_settings.h"

using namespace cass;
using namespace std;
using namespace std::tr1;

void TxtParser::run()
{
  CASSSettings s;
  s.beginGroup("TxtParser");
  char delim = s.value("Deliminator",'\t').toInt();
  string eventIdhead = s.value("EventIdHeader","Event ID 24 bits").toString().toStdString();
  s.endGroup();
  ifstream &file(*(_readerpointerpair.second._filestream));
  file.seekg (0, ios::end);
  const streampos filesize(file.tellg());
  file.seekg (0, ios::beg);

  Splitter split;
  string headerline;
  vector<string> headers;
  while (true)
  {
    getline(file, headerline);
    headers.clear();
    split(headerline,headers,delim);
    if (!headers.empty() &&
        !QString::fromStdString(headerline).contains("GMD	(GMD_DATA)	-	GMD main vlaues"))
      break;
  }
  cout <<"TxtParser: the txt file contains the following variables:";
  vector<string>::const_iterator h(headers.begin());
  for (; h != headers.end();++h)
    cout <<"'"<<*h<<"',";
  cout <<endl;

  string line;
  vector<double> values;
  map<string,double> head2value;
  streampos eventstartpos;
  while(file.tellg()<filesize)
  {
    while(true)
    {
      eventstartpos = file.tellg();
      getline(file, line);
      values.clear();
      split(line,values,delim);
      if (!values.empty())
        break;
    }
    if(headers.size() != values.size())
      throw runtime_error("TxtParser():There are not enough values for the amount of values suggested by the header");

    vector<double>::const_iterator value(values.begin());
    vector<string>::const_iterator head(headers.begin());
    for (;value != values.end(); ++value, ++head)
    {
      head2value[*head] = *value;
//      cout << "'"<<*head<<"'="<<*value<<", ";
    }
//    cout <<endl;
    const uint64_t eventId(head2value[eventIdhead]);
    const streampos currentpos(file.tellg());
    file.seekg(eventstartpos);
    savePos(eventId);
    file.seekg(currentpos);
  }
}
