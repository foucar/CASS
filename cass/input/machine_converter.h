// Copyright (C) 2009 - 2014 Lutz Foucar

/**
 * @file machine_converter.h contains xtc converter for machine data
 *
 * @author Lutz Foucar
 */


#ifndef MACHINEDATACONVERTER_H
#define MACHINEDATACONVERTER_H

#include <map>
#include <tr1/functional>

#include <QtCore/QMutex>

#include "cass_machine.hpp"
#include "conversion_backend.h"
#include "machine_device.hpp"

namespace Pds
{
class EpicsPvHeader;
}

namespace cass
{
class CASSEvent;

namespace MachineData
{

class EpicsKey;

/** Converter for Beamline-, Cavity-, Epics- and EVR Data
 *
 * Will convert Beamline data, Cavity data, Epics Data and EVR Data
 *
 * @note maybe split this to several converters for the different data types
 *       will this work. ie is only the epics map copied or the whole
 *       event?
 *
 * @author Lutz Foucar
 */
class CASS_MACHINEDATASHARED_EXPORT Converter : public cass::ConversionBackend
{
public:
  /** create singleton if doesnt exist already */
  static ConversionBackend::shared_pointer instance();

  /** called for appropriate xtc part.
   *
   * @param xtc the xtc that contains evr, epics, beamlinedata info
   * @param evt pointer to the event that we will write the data to.
   */
  void operator()(const Pds::Xtc*xtc, cass::CASSEvent*evt);

  /** called before the conversion
   *
   * @param evt pointer to the event that needs to be finalized
   */
  void prepare(CASSEvent *evt);

  /** called at the end of the conversion
   *
   * @param evt pointer to the event that needs to be finalized
   */
  void finalize(CASSEvent *evt);

private:

private:
  /** constructor
   *
   * sets up the pds types that it is responsible for
   */
  Converter();

  /** prevent copy construction */
  Converter(const Converter&);

  /** prevent assignment */
  Converter& operator=(const Converter&);

  /** the singleton container */
  static ConversionBackend::shared_pointer _instance;

  /** singleton locker for mutithreaded requests */
  static QMutex _mutex;

  /** define the conversion map from keys to strings */
  typedef std::map<EpicsKey,std::string> epicsKeyMap_t;

  /** map Epics Keys to strings */
  epicsKeyMap_t _index2name;

  /** define the function to convert epics to cass */
  typedef std::tr1::function<void(const Pds::EpicsPvHeader&,
                                  Device::epicsDataMap_t::iterator,
                                  Device::epicsDataMap_t::iterator)> epicsType2val_t;

  /** define a map to map epics type to function for retrieval of the value */
  typedef std::map<int16_t, epicsType2val_t > epicsType2convFunc_t;

  /** map containing fucntions that convert epics values to cass values */
  epicsType2convFunc_t _epicsType2convFunc;

  /** a container for the epics values
   *
   * @note this is necessary, since not every shot there is info about the
   *       epics values and calibcycle
   */
   Device _store;
};
}//end namespace MachineData
}//end namespace cass

#endif
