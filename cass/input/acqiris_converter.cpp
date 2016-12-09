//Copyright (C) 2009, 2010, 2011 Lutz Foucar

/**
 * @file acqiris_converter.cpp file contains the definition of the converter
 *                             for the xtc containing acqiris data.
 *
 * @author Lutz Foucar
 */

#include <cassert>

#include "acqiris_converter.h"
#include "cass_event.h"
#include "log.h"
#include "cass_settings.h"
#include "lcls_key.hpp"

#include "pdsdata/xtc/Xtc.hh"
#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/xtc/DetInfo.hh"
#include "pdsdata/acqiris/ConfigV1.hh"
#include "pdsdata/acqiris/DataDescV1.hh"
#include "pdsdata/xtc/Src.hh"

using namespace cass;
using namespace ACQIRIS;
using namespace std;
using namespace Pds;
using namespace lclsid;

// =================define static members =================
ConversionBackend::shared_pointer Converter::_instance;
QMutex Converter::_mutex;

ConversionBackend::shared_pointer Converter::instance()
{
  QMutexLocker locker(&_mutex);
  if(!_instance)
  {
    _instance = ConversionBackend::shared_pointer(new Converter());
  }
  return _instance;
}
// ========================================================

Converter::Converter()
{
  _pdsTypeList.push_back(TypeId::Id_AcqConfig);
  _pdsTypeList.push_back(TypeId::Id_AcqWaveform);

  CASSSettings s;
  s.beginGroup("Converter");

  int size = s.beginReadArray("LCLSAcqirisDevices");
  for (int i = 0; i < size; ++i)
  {
    s.setArrayIndex(i);
    string type(s.value("TypeName","Invalid").toString().toStdString());
    TypeId::Type typeID(TypeId::NumberOf);
    for (int i(0); i < TypeId::NumberOf; ++i)
      if (TypeId::name(static_cast<TypeId::Type>(i)) ==  type)
      {
        typeID = static_cast<TypeId::Type>(i);
        break;
      }

    uint32_t detID(s.value("DetectorID",0).toUInt());
    string detname(s.value("DetectorName","Invalid").toString().toStdString());
    DetInfo::Detector detnameID(DetInfo::NumDetector);
    for (int i(0); i < DetInfo::NumDetector; ++i)
      if (DetInfo::name(static_cast<DetInfo::Detector>(i)) ==  detname)
      {
        detnameID = static_cast<DetInfo::Detector>(i);
        break;
      }

    uint32_t devID(s.value("DeviceID",0).toUInt());
    string devname(s.value("DeviceName","Invalid").toString().toStdString());
    DetInfo::Device devnameID(DetInfo::NumDevice);
    for (int i(0); i < DetInfo::NumDevice; ++i)
      if (DetInfo::name(static_cast<DetInfo::Device>(i)) ==  devname)
      {
        devnameID = static_cast<DetInfo::Device>(i);
        break;
      }

    /** skip if the either name has not been set or not correctly set */
    if (typeID == TypeId::NumberOf ||
        detnameID == DetInfo::NumDetector ||
        devnameID == DetInfo::NumDevice)
      continue;

    Key key(typeID, detnameID, detID, devnameID, devID);
    _LCLSToCASSId[key] = s.value("CASSID",0).toInt();
  }
  s.endArray();
}

void Converter::operator()(const Pds::Xtc* xtc, CASSEvent* evt)
{
  /** skip if there is no corresponding cass key for that xtc */
  idmap_t::key_type lclskey(xtc->contains.id(), xtc->src.phy());
  idmap_t::iterator lclsmapIt(_LCLSToCASSId.find(lclskey));
  if (lclsmapIt == _LCLSToCASSId.end())
  {
    Log::add(Log::DEBUG0, string("Acqiris::Converter::operator(): There is no corresponding cass key for : '") +
             TypeId::name(xtc->contains.id()) + "'(" + toString(xtc->contains.id()) +
             "), '" + DetInfo::name(reinterpret_cast<const DetInfo*>(&xtc->src)->detector()) +
             "'(" + toString(reinterpret_cast<const DetInfo*>(&xtc->src)->detId()) +
             "), '" + DetInfo::name(reinterpret_cast<const DetInfo*>(&xtc->src)->device()) +
             "'(" + toString(reinterpret_cast<const DetInfo*>(&xtc->src)->devId()) +
             ")");
    return;
  }
  const idmap_t::mapped_type &casskey(lclsmapIt->second);

  /** check whether xtc is a configuration or a event **/
  switch (xtc->contains.id())
  {

  case (Pds::TypeId::Id_AcqConfig) :
  {
    /** use the right version to extract the info */
    unsigned version = xtc->contains.version();
    switch (version)
    {
    case 1:
    {
      /** get the config **/
      const Pds::Acqiris::ConfigV1 &config =
          *reinterpret_cast<const Pds::Acqiris::ConfigV1*>(xtc->payload());
      //extract how many channels are in the acqiris device//
      _configStore[casskey] = config.nbrChannels();
      Log::add(Log::INFO, string("AcqirisConverter: Instrument ") +
               TypeId::name(xtc->contains.id()) + "'(" + toString(xtc->contains.id()) +
               "), '" + DetInfo::name(reinterpret_cast<const DetInfo*>(&xtc->src)->detector()) +
               "'(" + toString(reinterpret_cast<const DetInfo*>(&xtc->src)->detId()) +
               "), '" + DetInfo::name(reinterpret_cast<const DetInfo*>(&xtc->src)->device()) +
               "'(" + toString(reinterpret_cast<const DetInfo*>(&xtc->src)->devId()) +
               "); SampleInterval '" + toString(config.horiz().sampInterval()) +
               "' NbrSamples '" + toString(config.horiz().nbrSamples()) +
               "' NbrSegments '" + toString(config.horiz().nbrSegments()) +
               "' DelayTime '" + toString(config.horiz().delayTime()) +
               "' TrigCoupling '" + toString(config.trig().coupling()) +
               "' TrigInput '" + toString(config.trig().input()) +
               "' TrigSlope '" + toString(config.trig().slope()) +
               "' TrigLevel '" + toString(config.trig().level()) +
               "' NbrChannels '" + toString(config.nbrChannels()) +
               "'");
      for (size_t i(0); i<_configStore[casskey]; ++i)
      {
        Log::add(Log::INFO, string("AcqirisConverter: Instrument ") +
                 TypeId::name(xtc->contains.id()) + "'(" + toString(xtc->contains.id()) +
                 "), '" + DetInfo::name(reinterpret_cast<const DetInfo*>(&xtc->src)->detector()) +
                 "'(" + toString(reinterpret_cast<const DetInfo*>(&xtc->src)->detId()) +
                 "), '" + DetInfo::name(reinterpret_cast<const DetInfo*>(&xtc->src)->device()) +
                 "'(" + toString(reinterpret_cast<const DetInfo*>(&xtc->src)->devId()) +
                 "); Channel '" + toString(i) +
                 "': Gain '" + toString(config.vert(i).slope()) +
                 "' Offset '" + toString(config.vert(i).offset()) +
                 "' FullScale '" + toString(config.vert(i).fullScale()) +
                 "' Bandwidth '" + toString(config.vert(i).bandwidth()) +
                 "' Coupling '" + toString(config.vert(i).coupling()) +
                 "'");
      }
      break;
    }
    default:
      throw runtime_error("Unsupported acqiris configuration version '" +
                          toString(version) + "'");
      break;
    }
    break;
  }

  /** if it is a event then extract all information from the event **/
  case (TypeId::Id_AcqWaveform):
  {
    /** extract the datadescriptor (waveform etc) from the xtc **/
    const Acqiris::DataDescV1 *datadesc =
        reinterpret_cast<const Acqiris::DataDescV1*>(xtc->payload());
    /** retrieve reference to the right acqiris instrument **/
    Device &dev(dynamic_cast<Device&>(*(evt->devices()[CASSEvent::Acqiris])));
    Instrument &instr(dev.instruments()[casskey]);
    /** retrieve a reference to the channel container of the instrument **/
    Instrument::channels_t &channels(instr.channels());
    /** resize the channel vector to how many channels are in the device **/
    const size_t nChans(_configStore[casskey]);
    channels.resize(nChans);
    /** copy the channel values from the datadescriptor **/
    for (size_t i(0); i < nChans; ++i)
    {
      /** get a reference instead of a pointer for easier writing **/
      const Acqiris::DataDescV1 &dd(*datadesc);
      /** retrieve a reference to the channel we are working on **/
      Channel &chan(channels[i]);
      /** extract the infos from the datadesc **/
      chan.channelNbr()     = i;
      chan.horpos()         = dd.timestamp(0).horPos();
      chan.offset()         = dd.offset();
      chan.gain()           = dd.gain();
      chan.sampleInterval() = dd.sampleInterval();
      /** get pointer to waveform in the datadescriptor **/
      const short* wf = dd.waveform();
      /** need to shift the pointer so that it looks at the first real point of
       *  the waveform
       */
      wf += dd.indexFirstPoint();
      /** retrieve a reference to waveform within the cassevent channel **/
      Channel::waveform_t &waveform = chan.waveform();
      /** resize cassevent waveform container to the correct size **/
      const size_t nSamples(dd.nbrSamplesInSeg());
      waveform.resize(nSamples);
      //Log::add(Log::DEBUG4, string("AcqirisConverter: Instrument ") +
      //         TypeId::name(xtc->contains.id()) + "'(" + toString(xtc->contains.id()) +
      //         "), '" + DetInfo::name(reinterpret_cast<const DetInfo*>(&xtc->src)->detector()) +
      //         "'(" + toString(reinterpret_cast<const DetInfo*>(&xtc->src)->detId()) +
      //         "), '" + DetInfo::name(reinterpret_cast<const DetInfo*>(&xtc->src)->device()) +
      //         "'(" + toString(reinterpret_cast<const DetInfo*>(&xtc->src)->devId()) +
      //         "); Channel '" + toString(i) +
      //         "': Gain '" + toString(dd.gain()) +
      //         "' Offset '" + toString(dd.offset()) +
      //         "' idxFirstPoint '" + toString(dd.indexFirstPoint()) +
      //         "' SampleInterval '" + toString(dd.sampleInterval()) +
      //         "' NbrSamplesInSeg '" + toString(nSamples) +
      //         "'");
      /** copy the datapoints of the waveform
       *  the byte order has to be swapped for some reason that still has
       *  to be determined
       */
      for (size_t iWave=0;iWave<nSamples;++iWave)
        waveform[iWave] = (wf[iWave]&0x00ff<<8) | (wf[iWave]&0xff00>>8);

      /** iterate to next channel */
      datadesc = datadesc->nextChannel();
    }
    break;
  }
  default:
    throw logic_error(string("ACQIRIS::Converter(): xtc type '") +
                      Pds::TypeId::name(xtc->contains.id()) +
                      "' is not handled by Acqiris Converter");
    break;
  }
}
