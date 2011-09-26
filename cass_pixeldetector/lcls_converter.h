// Copyright (C) 2009, 2010, 2011 Lutz Foucar

/**
 * @file lcls_converter.cpp contains the converters to convert ccd and pnccd data
 *                          to CASSEvent
 *
 * @author Lutz Foucar
 */

#ifndef _LCLSCONVERTER_H_
#define _LCLSCONVERTER_H_

#include <map>

#include <QtCore/QMutex>

#include "conversion_backend.h"

namespace Pds
{
namespace PNCCD
{
class ConfigV1;
class ConfigV2;
}
}

namespace cass
{
class CASSEvent;

namespace pixeldetector
{
namespace lclsid
{
class Key;
}

/** Converter for pnCCD and commercial CCD Data.
 *
 * @author Lutz Foucar
 */
class Converter : public cass::ConversionBackend
{
public:
  /** create singleton if doesnt exist already */
  static ConversionBackend::shared_pointer instance();

  /** operator to convert the LCLS Data to CASSEvent
   *
   * retrieve the corresponding CASS key from the lcls xtc that are contained in
   * the TypeId and Src (DetInfo) parts of the Xtc. Use the_LCLSToCASSId to do
   * this.
   *
   * in case that xtc is a Id_pnCCDconfig:\n
   * extract the version and create a config object toghether with this. Then
   * copy the information from the xtc config to the config object. Then store
   * the config object in the _pnccdConfigStore map.
   *
   * in case that xtc is a Id_pnCCDframe:\n
   * the conversion will only be performed if there is an config present for
   * the data. The config of version 1 does not contain information about the
   * frame size, therefore one needs to set it manually. In version 2 these
   * values are present, but sometimes there is a problem and one gets wrong
   * data. To prevent this, a check for consistency is implemented that will
   * output the wrong values and set them to the default values.
   *
   *
   * in case that xtc is a Id_Frame:\n
   * the frame data of the lcls is just copied to the detectors frame. The
   * alignment is already like it is within CASS as a linearised array of pixels
   * with x being the fast increasing axis. One has to substract the offset from
   * all pixels, which is done during the copying. The first 8 pixels of the
   * frame contain status information and are therefore set to the same value
   * that the ninth pixel has.
   *
   * @param xtc the part of the datagram that this converter is responsible for
   * @param evt The CASSEvent that should store the information from the xtc.
   */
  void operator()(const Pds::Xtc* xtc, cass::CASSEvent* evt);

private:
  /** map that will map the LCLS key to the CASS key */
  typedef std::map<lclsid::Key, int32_t>  idmap_t;

  /** pair the version of the config with a shared pointer of the config */
  typedef std::pair<uint32_t, std::tr1::shared_ptr<Pds::PNCCD::ConfigV2> > config_t;

  /** constructor
   *
   * set up the pds type ids that it is responsible for
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

  /** store for the pnccd configuration.
   *
   * Will store the version and the configuration itself in a pair
   */
  std::map<int32_t, config_t > _pnccdConfigStore;
};
}//end namespace vmi
}//end namespace cass

#endif
