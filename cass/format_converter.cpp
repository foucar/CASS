// Copyright (C) 2009 Jochen Küpper
// Copyright (C) 2009,2010, 2011 Lutz Foucar

/**
 * @file format_converter.cpp file contains definition of the container for all
 *                           format converters
 *
 * @author Lutz Foucar
 */

#include <iostream>
#include <iomanip>
#include <algorithm>
//#include <fstream>

#include <QtCore/QMutexLocker>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include "format_converter.h"

#include "cass_event.h"
#include "machine_device.h"
#include "xtciterator.h"
#include "pdsdata/xtc/Dgram.hh"

using namespace std;

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
     * @param availConv reference to the available converters container
     * @param usedConv reference to the used converters container
     */
    activate(const FormatConverter::availableConverters_t &availConv,
             FormatConverter::usedConverters_t &usedConv)
      :_availableConverters(availConv),
       _usedConverters(usedConv)
    {}

    /** the operator
     *
     * retrieves the list of pds ids that the converter type is responsible for
     * then adds the converter to used converters container for all retrieved ids
     *
     * @param qtype the type of converter that should be activated
     */
    void operator()(const QString& type)
    {
      string convertertype(type.toStdString());
      FormatConverter::availableConverters_t::const_iterator converterIt
          (_availableConverters.find(convertertype));
      if (_availableConverters.end() == converterIt)
      {
        stringstream ss;
        ss <<"activate(): Converter type '"<<convertertype<<"' is unknown";
        throw invalid_argument(ss.str());
      }
      const ConversionBackend::converterPtr_t converter = converterIt->second;
      const ConversionBackend::pdstypelist_t &pdsTypeList(converter->pdsTypeList());
      ConversionBackend::pdstypelist_t::const_iterator idIt(pdsTypeList.begin());
      for (;idIt != pdsTypeList.end();++idIt)
        _usedConverters[(*idIt)] = converter;
    }

    /** reference to the container with all available converters */
    const FormatConverter::availableConverters_t &_availableConverters;

    /** reference to the container with the used converters */
    FormatConverter::usedConverters_t &_usedConverters;
  };
}



// ===============define static members====================
cass::FormatConverter *cass::FormatConverter::_instance(0);
QMutex cass::FormatConverter::_mutex;

void cass::FormatConverter::destroy()
{
  QMutexLocker locker(&_mutex);
  delete _instance;
  _instance = 0;
}
cass::FormatConverter *cass::FormatConverter::instance()
{
  QMutexLocker locker(&_mutex);
  if(0 == _instance)
    _instance = new FormatConverter();
  return _instance;
}
//==========================================================



cass::FormatConverter::FormatConverter()
  :_configseen(false)
{
  // create all the necessary individual format converters
  list<string>::const_iterator it(ConversionBackend::availableConverters.begin());
  for (;it!=ConversionBackend::availableConverters.end();++it)
    _availableConverters[*it] = ConversionBackend::instance(*it);

  //now load the converters, that the user want to use//
  loadSettings(0);
}

cass::FormatConverter::~FormatConverter()
{
}

void cass::FormatConverter::loadSettings(size_t)
{
  // initialze all xtc ids with blank converters to be on the save side//
  for (int i(Pds::TypeId::Any); i<Pds::TypeId::NumberOf; ++i)
    _usedConverters[static_cast<Pds::TypeId::Type>(i)] = _availableConverters["Blank"];

  CASSSettings s;
  s.beginGroup("Converter");
  QStringList usedConvertersList(s.value("Used","").toStringList());
  for_each(usedConvertersList.begin(),
           usedConvertersList.end(),
           activate(_availableConverters,_usedConverters));
}

enum {NoGoodData=0,GoodData};
bool cass::FormatConverter::processDatagram(cass::CASSEvent *cassevent)
{
  //intialize the return value//
  // the return value reflects the whether the datagram was a L1Transition(an event)
  bool retval(NoGoodData);
  //get the datagram from the cassevent//
  Pds::Dgram *datagram = reinterpret_cast<Pds::Dgram*>(cassevent->datagrambuffer());

/*
  std::cout << "transition \""<< Pds::TransitionId::name(datagram->seq.service())<< "\" ";
  std::cout << "0x"<< std::hex<< datagram->xtc.sizeofPayload()<<std::dec<<"  ";
  std::cout << "0x"<< std::hex<<datagram->xtc.damage.value()<<std::dec<<" ";
  std::cout << "0x"<< std::hex<< datagram->seq.clock().seconds()<<std::dec<<" ";
  std::cout << "0x"<< std::hex<< static_cast<uint32_t>(datagram->seq.stamp().fiducials())<<std::dec<<" ";
  std::cout << std::dec <<std::endl;
*/

  //if datagram is configuration or an event (L1Accept) then we will iterate through it//
  //otherwise we ignore the datagram//
  if ((datagram->seq.service() == Pds::TransitionId::Configure) ||
      (datagram->seq.service() == Pds::TransitionId::L1Accept))
  {
    //when it is a configuration transition then set the flag accordingly//
    if (datagram->seq.service() == Pds::TransitionId::Configure)
      _configseen=true;
    //if the datagram is an event, create the id from time and fiducial//
    if (_configseen && datagram->seq.service() == Pds::TransitionId::L1Accept)
    {
      //extract the bunchId from the datagram//
      uint64_t bunchId = datagram->seq.clock().seconds();
      bunchId = (bunchId<<32) + static_cast<uint32_t>(datagram->seq.stamp().fiducials()<<8);
      //put the id into the cassevent
      cassevent->id() = bunchId;
      //when the datagram was an event we need to tell the caller//
      retval = GoodData;
      //clear the beamline data//
      dynamic_cast<MachineData::MachineDataDevice*>
          (cassevent->devices()[CASSEvent::MachineData])->clear();
    }

    //iterate through the datagram and find the wanted information//
    //if the return value of the iterateor is false, then the transition
    //did not contain all information//
    XtcIterator iter(&(datagram->xtc),_usedConverters,cassevent,0);
    retval = iter.iterate() && retval;
  }
//  std::cout<< std::boolalpha<<retval<<std::endl;
  return retval;
}




// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
