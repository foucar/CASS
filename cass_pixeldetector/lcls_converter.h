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
   * in case that xtc is a Id_pnCCDconfig:\n
   * extract the version and create a config object toghether with this. Then
   * copy the information from the xtc config to the config object. Then store
   * the config object in the _pnccdConfigStore map.
   *
   * in case that xtc is a Id_pnCCDframe:\n
   *
   * in case that xtc is a Id_Frame:\n
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
  idmap_t LCLSToCASSId;

  /** store for the pnccd configuration.
   *
   * Will store the version and the configuration itself in a pair
   */
  std::map<int32_t, config_t > _pnccdConfigStore;
};
}//end namespace vmi
}//end namespace cass

#endif
