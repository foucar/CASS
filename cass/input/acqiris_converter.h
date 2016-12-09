//Copyright (C) 2009, 2010, 2011 Lutz Foucar

/**
 * @file acqiris_converter.h file contains the declaration of the converter
 *                           for the xtc containing acqiris data.
 *
 * @author Lutz Foucar
 */

#ifndef _ACQIRIS_CONVERTER_H
#define _ACQIRIS_CONVERTER_H

#include <iostream>
#include <map>

#include <QtCore/QMutex>

#include "conversion_backend.h"
#include "acqiris_device.hpp"


namespace cass
{
namespace lclsid
{
class Key;
}
namespace ACQIRIS
{
/** Acqiris Converter
 *
 * this class takes a xtc of type Id_AcqWaveform or Id_AcqConfig and
 * extracts the acqiris channels for all instruments
 *
 * @cassttng Converter/LCLSAcqirisDevices/Detector/{size} \n
 *           Number of user defined detectors to be pulled out of the xtc
 * @cassttng Converter/LCLSPixelDetectors/Detector/\%id\%/{TypeName} \n
 *           The type of the detector. Only the following types are supported:
 *          - Id_AcqConfig : config for Acqiris device
 *          - Id_AcqWaveform : data of the Acqiris device
 * @cassttng Converter/LCLSAcqirisDevices/\%id\%/{DetectorName} \n
 *           Name of the detector. Default is invalid
 * @cassttng Converter/LCLSAcqirisDevices/\%id\%/{DetectorID} \n
 *           the id of the detector. Default is 0.
 * @cassttng Converter/LCLSAcqirisDevices/\%id\%/{DeviceName} \n
 *           Name of the detector device
 * @cassttng Converter/LCLSAcqirisDevices/\%id\%/{DeviceID} \n
 *           Id of the detector device
 * @cassttng Converter/LCLSAcqirisDevices/\%id\%/{CASSID} \n
 *           the Id the Acqiris should get in the CASSEvent. One needs this
 *           number for further processing. Note that the config and the data
 *           part must have the same CASSID.
 *
 * @author Lutz Foucar
 */
class Converter : public ConversionBackend
{
public:
  /** create singleton if doesnt exist already */
  static ConversionBackend::shared_pointer instance();

  /** takes the xtc and copies the data to cassevent */
  void operator()(const Pds::Xtc*, CASSEvent*);

private:
  /** define the map for lcls key to cass id */
  typedef std::map<lclsid::Key,Device::instruments_t::key_type> idmap_t;

  /** define the store of the config information */
  typedef std::map<Device::instruments_t::key_type,size_t> configStore_t;

  /** constructor
   *
   * sets up the pds type ids it is responsible for
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

  /** map lcls id to cass id */
  idmap_t _LCLSToCASSId;

  /** Number of Channels for a device
   *
   * the number of channels for the device is only send with a configure
   * transition we store them in a map for each instrument
   */
  configStore_t _configStore;
};
}//end namespace acqiris
}//end namespace cass

#endif
