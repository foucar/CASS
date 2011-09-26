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
namespace Id
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
   * @param
   * @param
   */
  void operator()(const Pds::Xtc*, cass::CASSEvent*);

private:
  /** */
  typedef std::map<Id::Key, int32_t>  idmap_t;

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
  std::map<Id::Key, std::pair<uint32_t, Pds::PNCCD::ConfigV2> > _pnccdConfig;
};
}//end namespace vmi
}//end namespace cass

#endif
