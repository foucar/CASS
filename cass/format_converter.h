// Copyright (C) 2009 Jochen Kuepper
// Copyright (C) 2009, 2010 Lutz Foucar

/**
 * @file format_converter.h file contains declaration of the container for all
 *                          format converters
 *
 * @author Lutz Foucar
 */

#ifndef CASS_FORMATCONVERTER_H
#define CASS_FORMATCONVERTER_H

#include <map>

#include <QtCore/QMutex>
#include <QtCore/QObject>

#include "pdsdata/xtc/TypeId.hh"

#include "cass.h"
#include "conversion_backend.h"
#include "cass_settings.h"

namespace cass
{
//forward delarations
class CASSEvent;
class ConversionBackend;

/** Format converter container.
   *
   * Only one FormatConvert object must exist, therefore this is implemented
   * as a singleton. It contains all available Format Converters and calls
   * all requested ones.
   *
   * @section converter List of possible converters
   * @cassttng Converter/{Used}\n
   *           comma separated list of Converters that should be active. Default
   *           is "". Possible values are:
   *           - Machine: access to Machine Data (Beamline and Epics data)
   *             (see cass::MachineData::Converter)
   *           - Acqiris: access to Acqiris Digitizer data
   *             (see cass::ACQIRIS::Converter)
   *           - AcqirisTDC: access to Acqiris TDC data
   *             (see cass::ACQIRISTDC::Converter)
   *           - CCD: access to Opal camera data
   *             (see cass::CCD::Converter)
   *           - pnCCD: access to pnCCD data
   *             (see cass::pnCCD::Converter)
   *           - pixeldetector: access to CCD and pnCCD data which will be put
   *                            into the newer container for accessing it
   *                            through the new analysis chain (omitting the
   *                            preanalysis). see cass::pixeldetector::Converter
   *
   * @author Jochen Kuepper
   * @author Lutz Foucar
   */
class FormatConverter
{
public:
  static const unsigned int pvNumMax = 2;

  /** a shared pointer of this type */
  typedef std::tr1::shared_ptr<FormatConverter> shared_pointer;

  /** Return a pointer to the single FormatConverter instance */
  static shared_pointer instance();

  /** function to process a datagram and turn it into a cassevent.
   *
   * this function will iterate through the xtc's contained in the datagram
   * extract the information and put into the right device of the CASSEvent
   *
   * @return returns true if the datagram was a L1Accept (an event) transition
   * @param evt pointer to the CASSEvent which also contains the datagram
   */
  bool operator()(CASSEvent*evt);

  /** function to load  the settings for the format converter
   *
   * initialze all xtc ids with blank converters.
   * Get the list of converters that should be used. Then create the requested
   * converters and retrieve the list of pds ids that the individual converter
   * is responsible for. Add the converter to used converters container for all
   * retrieved ids
   */
  void loadSettings(size_t what);

  /** function to  save the settings for the format converter */
  void saveSettings() {}

protected:
  /** constructor is made protected, should only be called through instance*/
  FormatConverter();

public:
  /** typdef describing the map of used converters for easier readable code */
  typedef std::map<Pds::TypeId::Type, ConversionBackend::shared_pointer> usedConverters_t;

protected:
  /** status whether a configure has already been seen */
  bool _configseen;

  /** map that contains all type id's of all known xtc in a transition.
   *
   * the converter used by the xtc is the second. We add the appropriate
   * converters at the right position
   */
  usedConverters_t _usedConverters;

  /** pointer to the single instance */
  static shared_pointer _instance;

  /** Singleton operation locker in a multi-threaded environment.*/
  static QMutex _mutex;

private:
  /** the number of pv in the calibcycle */
  unsigned int _pvNum;

  /** the values of the pv in the calibcycle */
  double _pvControlValue[pvNumMax];

  /** the names of the pv in the calibcyle */
  std::string _pvControlName[pvNumMax];

  /** the final name of the pv */
  std::stringstream *_pvSS;
};

}//end namespace cass


#endif
