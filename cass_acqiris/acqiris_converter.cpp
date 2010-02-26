//Copyright (C) 2010 lmf

#include "acqiris_converter.h"
#include "cass_event.h"
#include "pdsdata/xtc/Xtc.hh"
#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/xtc/DetInfo.hh"
#include "pdsdata/acqiris/ConfigV1.hh"
#include "pdsdata/acqiris/DataDescV1.hh"
#include "pdsdata/xtc/Src.hh"


void cass::ACQIRIS::Converter::operator()(const Pds::Xtc* xtc, cass::CASSEvent* cassevent)
{
  //check whether xtc is a configuration or a event//
  switch (xtc->contains.id())
  {
  //if it is a configuration then check for which Acqiris
  case (Pds::TypeId::Id_AcqConfig) :
    {
      //extract the detectorinfo//
      const Pds::DetInfo& info = *(Pds::DetInfo*)(&xtc->src);
      if(info.detector() == Pds::DetInfo::Camp)
      {
        unsigned version = xtc->contains.version();
        switch (version)
        {
          //if it is the right configuration then initialize the storedevent with the configuration//
        case 1:
          {
            //get the config//
            const Pds::Acqiris::ConfigV1 &config = *reinterpret_cast<const Pds::Acqiris::ConfigV1*>(xtc->payload());
            //extract how many channels are in the acqiris device//
            _numberOfChannels = config.nbrChannels();
            std::cout <<"config:"<<std::endl;
            std::cout <<" NbrChannels: "<<config.nbrChannels()<<std::endl;
          }
          break;
        default:
          std::cout <<"Unsupported acqiris configuration version "<<version<<std::endl;
          break;
        }
      }
      break;
    }


    //if it is a event then extract all information from the event//
  case (Pds::TypeId::Id_AcqWaveform):
    {
      //extract the detectorinfo//
      const Pds::DetInfo& info = *(Pds::DetInfo*)(&xtc->src);
      //only extract data if it is from the acqiris that we are using//
      if (info.detector() == Pds::DetInfo::Camp)
      {
        //extract the datadescriptor (waveform etc) from the xtc//
        const Pds::Acqiris::DataDescV1 *datadesc =
            reinterpret_cast<const Pds::Acqiris::DataDescV1*>(xtc->payload());
        //retrieve a pointer to the right acqiris instrument//
        AcqirisDevice *dev =
            dynamic_cast<AcqirisDevice*>(cassevent->devices()[cass::CASSEvent::Acqiris]);
        //resize the channel vector to how many channels are in the device//
        dev->channels().resize(_numberOfChannels);
        //initialize the channel values from the datadescriptor//
        for (size_t iChan=0;iChan<dev->channels().size();++iChan)
        {
          //retrieve a reference instead of a pointer//
          const Pds::Acqiris::DataDescV1 &dd = *datadesc;
          //retrieve a reference to the channel we are working on//
          Channel &chan         = dev->channels()[iChan];
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
    }
  default:
    std::cout<<"xtc type \""<<Pds::TypeId::name(xtc->contains.id())<<"\" is not handled by REMIConverter"<<std::endl;
    break;
  }
}

