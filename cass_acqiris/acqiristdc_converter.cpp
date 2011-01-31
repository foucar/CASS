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
      cass::CASSEvent::devices_t::iterator devIt
          (evt->devices().find(cass::CASSEvent::AcqirisTDC));
      assert(evt->devices().end() != devIt);
      Device *dev (dynamic_cast<Device*>(devIt->second));
      Instrument &instr
          (dev->instruments()[static_cast<TDCInstruments>(info.detector())]);
      Instrument::channels_t &channels(instr.channels());
      channels.resize(Instrument::NbrChannels);
//      unsigned version = xtc->contains.version();
//      switch (version)
//      {
//        //if it is the right configuration then initialize the storedevent with the configuration//
//      case 1:
//        {
//          //get the config//
//          const Pds::Acqiris::ConfigV1 &config =
//              *reinterpret_cast<const Pds::Acqiris::ConfigV1*>(xtc->payload());
//        }
//        break;
//      default:
//        std::cout <<"Unsupported acqiris tdc configuration version "<<version<<std::endl;
//        break;
//      }
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
      Instrument &instr
          (dev->instruments()[static_cast<TDCInstruments>(info.detector())]);
      Instrument::channels_t &channels(instr.channels());
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
               //get data of the right channel and convert s to ns */
               channels[data->source()-1].hits().push_back(c.time()*1e9);
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
