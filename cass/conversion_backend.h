// Copyright (C) 2009 Jochen Kuepper
// Copyright (C) 2009,2010,2011,2013 Lutz Foucar

/**
 * @file conversion_backend.h file contains base class for all format converters
 *
 * @author Lutz Foucar
 */

#ifndef CASS_CONVERSIONBACKEND_H
#define CASS_CONVERSIONBACKEND_H

#include <list>
#include <string>
#include <tr1/memory>

#include "pdsdata/xtc/TypeId.hh"

#include "cass.h"

namespace Pds
{
//forward declaration
class Xtc;
}

namespace cass
{
class CASSEvent;

/** Base class for Converters
 *
 * Inherit from this class if you would like to add a new Converter
 *
 * @author Lutz Foucar
 * @author Jochen Kuepper
 */
class CASSSHARED_EXPORT ConversionBackend
{
public:
  /** typedef */
  typedef std::tr1::shared_ptr<ConversionBackend> shared_pointer;

  /** typedef */
  typedef std::list<Pds::TypeId::Type> pdstypelist_t;

public:
  /** virtual destructor to make clear this is a base class */
  virtual ~ConversionBackend() {}

  /** pure virtual operator.
   *
   * call this to convert the xtc to the cass event
   *
   * @param xtc the xtc that contains the data to convert
   * @param evt the event where the converted data should be stored
   */
  virtual void operator()(const Pds::Xtc *xtc, cass::CASSEvent *evt);

  /** return the list of pds type ids the converter is responsible for */
  const pdstypelist_t &pdsTypeList()const {return _pdsTypeList;}

  /** return the requested converter type
   *
   * @return shared pointer to the requested converter
   * @param type the type of the requested converter
   */
  static shared_pointer instance(const std::string& type);

protected:
  /** the list of pds types that the converter is responsible for */
  pdstypelist_t _pdsTypeList;
};
}//end namespace cass

#endif

