// Copyright (C) 2009 Jochen Kuepper
// Copyright (C) 2009, 2010, 2011 Lutz Foucar

/**
 * @file format_converter.cpp file contains definition of the container for all
 *                           format converters
 *
 * @author Lutz Foucar
 */

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include <tr1/functional>

#include <QtCore/QMutexLocker>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include "format_converter.h"

#include "cass_event.h"
#include "machine_device.h"
#include "xtciterator.hpp"
#include "pdsdata/xtc/Dgram.hh"
//#include "calibcycle.h"

using namespace std;
using namespace cass;

// ===============define static members====================
FormatConverter::shared_pointer FormatConverter::_instance;
QMutex FormatConverter::_mutex;

tr1::shared_ptr<FormatConverter> FormatConverter::instance()
{
  QMutexLocker locker(&_mutex);
  if(!_instance)
    _instance = tr1::shared_ptr<FormatConverter>(new FormatConverter());
  return _instance;
}
//==========================================================



FormatConverter::FormatConverter()
  :_configseen(false)
{}

void FormatConverter::loadSettings(size_t)
{
  for (int i(Pds::TypeId::Any); i<Pds::TypeId::NumberOf; ++i)
    _usedConverters[static_cast<Pds::TypeId::Type>(i)] =
        ConversionBackend::instance("Blank");

  typedef ConversionBackend::shared_pointer csp_t;
  typedef ConversionBackend::pdstypelist_t pdslist_t;
  CASSSettings s;
  s.beginGroup("Converter");
  QStringList usedConvertersList(s.value("Used").toStringList());

  QStringList::const_iterator convType(usedConvertersList.begin());
  QStringList::const_iterator convEnd(usedConvertersList.end());
  while (convType != convEnd)
  {
    const string convertertype((*convType++).toStdString());
    const csp_t converter(ConversionBackend::instance(convertertype));
    const pdslist_t &pdsTypeList(converter->pdsTypeList());
    pdslist_t::const_iterator pdsType(pdsTypeList.begin());
    pdslist_t::const_iterator pdsEnd(pdsTypeList.end());
    while (pdsType != pdsEnd)
      _usedConverters[(*pdsType++)] = converter;
  }
}

enum {NoGoodData=0,GoodData};
bool FormatConverter::operator()(CASSEvent *cassevent)
{
  /** intialize the return value
   *  (the return value reflects the whether the datagram was a good L1Transition(an event))
   */
  bool retval(NoGoodData);
  /** get the datagram from the cassevent */
  Pds::Dgram *datagram = reinterpret_cast<Pds::Dgram*>(&cassevent->datagrambuffer().front());

  Log::add(Log::DEBUG4,"Dgram Transition '"+ string(Pds::TransitionId::name(datagram->seq.service()))+ "'");
  Log::add(Log::DEBUG4,"DGram Payload size '" + toString(datagram->xtc.sizeofPayload()) + "'");
  Log::add(Log::DEBUG4,"Dgram Damage value '" + toString(datagram->xtc.damage.value()) + "'");
  Log::add(Log::DEBUG4,"DGram Clock seconds '" + toString(datagram->seq.clock().seconds()) + "'");
  Log::add(Log::DEBUG4,"Dgram Fiducials '" + toString(datagram->seq.stamp().fiducials()) + "'");

  /** if datagram is configuration or an event (L1Accept) then we will iterate through it
   *  otherwise we ignore the datagram
   */
  if ((datagram->seq.service() == Pds::TransitionId::Configure) ||
      (datagram->seq.service() == Pds::TransitionId::L1Accept) ||
      (datagram->seq.service() == Pds::TransitionId::BeginCalibCycle))
  {
    /** when it is a configuration transition then set the flag accordingly */
    if (datagram->seq.service() == Pds::TransitionId::Configure)
      _configseen=true;
    /** if the datagram is an event, create the id from time and fiducial */
    if (_configseen && datagram->seq.service() == Pds::TransitionId::L1Accept)
    {
      /** extract the bunchId from the datagram */
      uint64_t bunchId = datagram->seq.clock().seconds();
      bunchId = (bunchId<<32) + static_cast<uint32_t>(datagram->seq.stamp().fiducials()<<8);
      /** put the id into the cassevent */
      cassevent->id() = bunchId;

      /** set the return value to true */
      retval = GoodData;
    }
    /** prepare the cassevent
     *
     *  @note maybe, when more converters need preparing the data, we will do
     *        it for all loaded converters, which will need another list that
     *        conatins the loaded converters
     */
    _usedConverters[Pds::TypeId::Id_Epics]->prepare(cassevent);
    /** now iterate through the datagram and find the wanted information
     *  if the return value of the iterator is false, then the transition
     *  did not contain all information
     */
    XtcIterator iter(&(datagram->xtc),_usedConverters,cassevent,0);
    retval = iter.iterate() && retval;
    /** finalize the epics conversion, so get the epics converter and finalize
     *
     *  @note maybe, when more converters need finalizing the data, we will do it
     *        for all loaded converters, which will need another list that
     *        conatins the loaded converters
     */
    _usedConverters[Pds::TypeId::Id_Epics]->finalize(cassevent);
  }
  return retval;
}
