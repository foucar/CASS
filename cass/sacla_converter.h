// Copyright (C) 2014 Lutz Foucar

/**
 * @file sacla_converter.h contains class to convert sacla data to cassevent
 *
 * @author Lutz Foucar
 */

#ifndef _SACLACONVERTER_
#define _SACLACONVERTER_

#include "cass.h"

namespace cass
{
class CASSEvent;

/** class for reading txt files
 *
 * first line has to give the names of the variables, which are then ordered
 * line by line.
 *
 * @cassttng TxtReader/\%filename\%/{Deliminator}\n
 *           The deliminator that is used to separate the values. Default is '\\t',
 *           which is a tab.
 * @cassttng TxtReader/\%filename\%/{EventIdHeader}\n
 *           The name of the Header under which the Event Id is stored. Default
 *           is "".
 * @cassttng TxtReader/\%filename\%/{LinesToSkip}\n
 *           How many lines do we have to skip before the line appears that
 *           contains the headers. Default is 3
 *
 * @author Lutz Foucar
 */
class SACLAConverter
{
public:
  /** constructor */
  SACLAConverter();

  /** read the frms6 file contents put them into cassevent
   *
   * @return true when the workers should work on the filled cassevent,
   *         false if not.
   * @param blNbr the beamline number
   * @param highTagNumber first part of the tag
   * @param tagNbr the acutal Tag
   * @param event the CASSEvent where the data will be put into
   */
  bool operator()(const int blNbr, const int highTagNbr,
                  const int tagNbr, CASSEvent& event);

  /** load the settings of the reader */
  void loadSettings();

private:
  /** the list of requested machine values */
  std::vector<std::string> _machineVals;

  /** the list of requested octal detectors */
  std::map<int32_t,std::string> _octalDetectors;
};
}//end namespace cass
#endif
