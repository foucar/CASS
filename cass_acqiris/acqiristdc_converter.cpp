//Copyright (C) 2011 Lutz Foucar

/**
 * @file acqiristdc_converter.cpp file contains the definition of the converter
 *                             for the xtc containing acqiris tdc data.
 *
 * @author Lutz Foucar
 */

#include <cassert>
#include <sstream>
#include <stdexcept>

#include "pdsdata/xtc/Xtc.hh"
#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/xtc/DetInfo.hh"
#include "pdsdata/xtc/Src.hh"
#include "pdsdata/acqiris/TdcConfigV1.hh"
#include "pdsdata/acqiris/TdcDataV1.hh"

#include "acqiristdc_converter.h"

#include "acqiristdc_device.h"
#include "cass_event.h"

using namespace cass::ACQIRISTDC;
using namespace std;

// =================define static members =================
cass::ConversionBackend::shared_pointer Converter::_instance;
QMutex Converter::_mutex;

cass::ConversionBackend::shared_pointer Converter::instance()
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
  _pdsTypeList.push_back(Pds::TypeId::Id_AcqTdcData);
  _pdsTypeList.push_back(Pds::TypeId::Id_AcqTdcConfig);
}

void Converter::operator()(const Pds::Xtc* xtc, cass::CASSEvent* evt)
{
  //check whether xtc is a configuration or a event//
  switch (xtc->contains.id())
  {
  case (Pds::TypeId::Id_AcqTdcConfig) :
    {
      const Pds::DetInfo& info = *(Pds::DetInfo*)(&xtc->src);
//      assert(static_cast<int>(info.detector()) == static_cast<int>(SXRTdc));
      cass::CASSEvent::devices_t::iterator devIt
          (evt->devices().find(cass::CASSEvent::AcqirisTDC));
      assert(evt->devices().end() != devIt);
      Device *dev (dynamic_cast<Device*>(devIt->second));
      Instrument &instr(dev->instruments()[info.devId()]);
      Instrument::channels_t &channels(instr.channels());
      channels.resize(Instrument::NbrChannels);
      cout << "ACQIRISTDC::Converter: found '"<<Pds::DetInfo::name(info)
          <<"' configuration in datastream."
          <<endl;
    }
    break;

  //if it is a event then extract all information from the event//
  case (Pds::TypeId::Id_AcqTdcData):
    {
      const Pds::DetInfo& info = *(Pds::DetInfo*)(&xtc->src);
      cass::CASSEvent::devices_t::iterator devIt
          (evt->devices().find(cass::CASSEvent::AcqirisTDC));
      assert(evt->devices().end() != devIt);
      Device *dev (dynamic_cast<Device*>(devIt->second));
      Instrument &instr(dev->instruments()[info.devId()]);
//      if (xtc->sizeofPayload()/4 > 2)
//      {
//        uint32_t* word ((uint32_t*) xtc->payload());
//        cout << "start"<<endl;
//        for (int i(0); i < xtc->sizeofPayload()/4 ; ++i)
//          cout << hex<< word[i] <<dec<<endl;
//        cout << "stop"<<endl;
//      }
//      Device::instruments_t::iterator instrIt
//          (dev->instruments().find(info.devId()));
//      if (dev->instruments().end() == instrIt)
//      {
//        stringstream ss;
//        ss<<"ACQIRISTDC::Converter(): The AcqirisTDC in the CASSEvent does"
//            <<" not contain the instrument '"<<Pds::DetInfo::name(info)<<"'";
//        throw runtime_error(ss.str());
//      }
//      Instrument &instr(instrIt->second);
      Instrument::channels_t &channels(instr.channels());
      channels.resize(Instrument::NbrChannels);
      assert(6 == channels.size());
      for (Instrument::channels_t::iterator it(channels.begin()); it != channels.end(); ++it)
        it->hits().clear();
      //extract the data from the xtc//
      const Pds::Acqiris::TdcDataV1 *data
          (reinterpret_cast<const Pds::Acqiris::TdcDataV1*>(xtc->payload()));
      //  Data is terminated with an AuxIOMarker (Memory bank switch)
      while(!(data->source() == Pds::Acqiris::TdcDataV1::AuxIO &&
              static_cast<const Pds::Acqiris::TdcDataV1::Marker*>(data)->type() <
              Pds::Acqiris::TdcDataV1::Marker::AuxIOMarker))
      {
        switch(data->source())
        {
        case Pds::Acqiris::TdcDataV1::Comm:
          break;
        case Pds::Acqiris::TdcDataV1::AuxIO:
          break;
        default:
          {
            const Pds::Acqiris::TdcDataV1::Channel& c
                (*static_cast<const Pds::Acqiris::TdcDataV1::Channel*>(data));
            if (!c.overflow())
            {
              //get data of the right channel and convert s to ns */
              channels[data->source()-1].hits().push_back(c.time()*1e9);
            }
            break;
          }
        }
        data++;
      }
    }
    break;


  default:
    {
      stringstream ss;
      ss<<"ACQIRISTDC::Converter(): Xtc type'"<<Pds::TypeId::name(xtc->contains.id())
          <<"' is not handled by TDCConverter";
      throw logic_error(ss.str());
    }
    break;
  }
}
