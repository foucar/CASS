// Copyright (C) 2009 Jochen Küpper
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
//#include <fstream>

#include <QtCore/QMutexLocker>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include "format_converter.h"

#include "cass_event.h"
#include "machine_device.h"
#include "xtciterator.h"
#include "pdsdata/xtc/Dgram.hh"
#include "calibcycle.h"

using namespace std;
using namespace cass;

namespace cass
{
  /** functor to activate a certain converter
   *
   * puts the converter in the used converter list for the right pds ids.
   *
   * @author Lutz Foucar
   */
  struct activate
  {
    /** constructor
     *
     * @param usedConv reference to the used converters container
     */
    activate(FormatConverter::usedConverters_t &usedConv)
      :_usedConverters(usedConv)
    {}

    /** the operator
     *
     * retrieves the list of pds ids that the converter type is responsible for
     * then adds the converter to used converters container for all retrieved ids
     *
     * @param type the type of converter that should be activated
     */
    void operator()(const QString& type)
    {
      const string convertertype(type.toStdString());
      const ConversionBackend::shared_pointer converter =
          ConversionBackend::instance(convertertype);
      const ConversionBackend::pdstypelist_t &pdsTypeList(converter->pdsTypeList());
      ConversionBackend::pdstypelist_t::const_iterator idIt(pdsTypeList.begin());
      for (;idIt != pdsTypeList.end();++idIt)
        _usedConverters[(*idIt)] = converter;
    }

    /** reference to the container with the used converters */
    FormatConverter::usedConverters_t &_usedConverters;
  };
}



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
  :_configseen(false), _pvSS(NULL)
{}

void FormatConverter::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Converter");
  QStringList usedConvertersList(s.value("Used").toStringList());

  for_each(usedConvertersList.begin(), usedConvertersList.end(), activate(_usedConverters));
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


  Log::add(Log::DEBUG4,"transition '"+ string(Pds::TransitionId::name(datagram->seq.service()))+ "'");
  Log::add(Log::DEBUG4,"payload size '" + toString(datagram->xtc.sizeofPayload()) + "'");
  Log::add(Log::DEBUG4,"damage value '" + toString(datagram->xtc.damage.value()) + "'");
  Log::add(Log::DEBUG4,"clock seconds '" + toString(datagram->seq.clock().seconds()) + "'");
  Log::add(Log::DEBUG4,"fiducials '" + toString(datagram->seq.stamp().fiducials()) + "'");


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
      cassevent->pvControl = _pvSS->str();

      /** set the return value to true */
      retval = GoodData;
      /** clear the beamline data */
      dynamic_cast<MachineData::MachineDataDevice*>
          (cassevent->devices()[CASSEvent::MachineData])->clear();
    }
    else if (_configseen && datagram->seq.service() == Pds::TransitionId::BeginCalibCycle) 
    {
      CalibCycleIterator iter(&(datagram->xtc), _pvNum, _pvControlValue, _pvControlName);
      retval = iter.iterate() && retval;
      if (_pvSS) delete _pvSS;
      _pvSS = new stringstream;
      for (unsigned int i=0; i < _pvNum; i++) 
      {
        *_pvSS << _pvControlName[i] << "=" << _pvControlValue[i];
        if (!(i+1 == _pvNum)) *_pvSS << ",";
      }
      Log::add(Log::INFO, "BeginCalibCycle " +  _pvSS->str());
    }

    /** now iterate through the datagram and find the wanted information
     *  if the return value of the iterateor is false, then the transition
     *  did not contain all information
     */
    XtcIterator iter(&(datagram->xtc),_usedConverters,cassevent,0);
    retval = iter.iterate() && retval;
  }
  return retval;
}
