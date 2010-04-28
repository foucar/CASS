//Copyright (C) 2009, 2010 lmf

#include <cassert>

#include "acqiris_converter.h"
#include "cass_event.h"
#include "pdsdata/xtc/Xtc.hh"
#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/xtc/DetInfo.hh"
#include "pdsdata/acqiris/ConfigV1.hh"
#include "pdsdata/acqiris/DataDescV1.hh"
#include "pdsdata/xtc/Src.hh"


void cass::ACQIRIS::Converter::operator()(const Pds::Xtc* xtc, cass::CASSEvent* evt)
{
  //check whether xtc is a configuration or a event//
  switch (xtc->contains.id())
  {
    //if it is a configuration then check for which Acqiris
  case (Pds::TypeId::Id_AcqConfig) :
    {
      //extract the detectorinfo//
      const Pds::DetInfo& info = *(Pds::DetInfo*)(&xtc->src);
      //we need to make sure that only instruments we know of are present//
      assert(static_cast<int>(info.detector()) == static_cast<int>(Camp1) ||
             static_cast<int>(info.detector()) == static_cast<int>(Camp2) ||
             static_cast<int>(info.detector()) == static_cast<int>(Camp3));
      //retrieve a reference to the nbr of Channels for this instrument//
      size_t &nbrChannels = _numberOfChannels[static_cast<Instruments>(info.detector())];
      //make sure the number is smaller than 20, which is the maximum nbr of channels//
      assert(nbrChannels <= 20);
      unsigned version = xtc->contains.version();
      switch (version)
      {
        //if it is the right configuration then initialize the storedevent with the configuration//
      case 1:
        {
          //get the config//
          const Pds::Acqiris::ConfigV1 &config =
              *reinterpret_cast<const Pds::Acqiris::ConfigV1*>(xtc->payload());
          //extract how many channels are in the acqiris device//
          nbrChannels = config.nbrChannels();
          std::cout <<"AcqirisConverter::ConfigXtc: Instrument "
              <<info.detector() << " has "
              <<config.nbrChannels()
              <<" Channels"<<std::endl;
        }
        break;
      default:
        std::cout <<"Unsupported acqiris configuration version "<<version<<std::endl;
        break;
      }
    }
    break;


    //if it is a event then extract all information from the event//
  case (Pds::TypeId::Id_AcqWaveform):
    {
      //extract the detectorinfo//
      const Pds::DetInfo& info = *(Pds::DetInfo*)(&xtc->src);
      //we need to make sure that only instruments we know of are present//
      assert(static_cast<int>(info.detector()) == static_cast<int>(Camp1) ||
             static_cast<int>(info.detector()) == static_cast<int>(Camp2) ||
             static_cast<int>(info.detector()) == static_cast<int>(Camp3));
      //retrieve  the nbr of Channels for this instrument//
      const size_t nbrChannels = _numberOfChannels[static_cast<Instruments>(info.detector())];
      //make sure the number is smaller than 20
      assert(nbrChannels <= 20);
      //extract the datadescriptor (waveform etc) from the xtc//
      const Pds::Acqiris::DataDescV1 *datadesc =
          reinterpret_cast<const Pds::Acqiris::DataDescV1*>(xtc->payload());
      //retrieve a pointer to the right acqiris instrument//
      Device *dev =
          dynamic_cast<Device*>(evt->devices()[cass::CASSEvent::Acqiris]);
      //retrieve a reference to the right instrument//
      Instrument & instr = dev->instruments()[static_cast<Instruments>(info.detector())];
      //retrieve a reference to the channel container of the instrument//
      Instrument::channels_t &channels = instr.channels();
      //resize the channel vector to how many channels are in the device//
      channels.resize(nbrChannels);
      //initialize the channel values from the datadescriptor//
      for (Instrument::channels_t::iterator it=channels.begin();
          it != channels.end();
          ++it)
      {
        //retrieve a reference instead of a pointer//
        const Pds::Acqiris::DataDescV1 &dd = *datadesc;
        //retrieve a reference to the channel we are working on//
        Channel &chan         = *it;
        //extract the infos from the datadesc//
        chan.horpos()         = dd.timestamp(0).horPos();
        chan.offset()         = dd.offset();
        chan.gain()           = dd.gain();
        chan.sampleInterval() = dd.sampleInterval();
        //extract waveform from the datadescriptor//
        const short* waveform = dd.waveform();
        //we need to shift the pointer so that it looks at the first real point of the waveform//
        waveform += dd.indexFirstPoint();
        //retrieve a reference to our waveform//
        Channel::waveform_t mywaveform = chan.waveform();
        //resize our waveform vector to hold all the entries of the waveform//
        mywaveform.resize(dd.nbrSamplesInSeg());
        //copy the datapoints of the waveform//
        //we have to swap the byte order for some reason that still has to be determined//
        for (size_t iWave=0;iWave<dd.nbrSamplesInSeg();++iWave)
          mywaveform[iWave] = (waveform[iWave]&0x00ff<<8) | (waveform[iWave]&0xff00>>8);

        //change to the next Channel//
        datadesc = datadesc->nextChannel();
      }
    }
    break;
  default:
    std::cout<<"xtc type \""<<Pds::TypeId::name(xtc->contains.id())<<"\" is not handled by REMIConverter"<<std::endl;
    break;
  }
}
