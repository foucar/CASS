#include "remi_converter.h"
#include "cass_event.h"
#include "pdsdata/xtc/Xtc.hh"
#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/xtc/DetInfo.hh"
#include "pdsdata/acqiris/ConfigV1.hh"
#include "pdsdata/acqiris/DataDescV1.hh"
#include "pdsdata/xtc/Src.hh"

cass::REMI::Converter::Converter()
{
  //this converter should react on acqiris config and waveform//
  _types.push_back(Pds::TypeId::Id_AcqConfig);
  _types.push_back(Pds::TypeId::Id_AcqWaveform);
}

void cass::REMI::Converter::operator()(const Pds::Xtc* xtc, cass::CASSEvent* cassevent)
{
  //check whether xtc is a configuration or a event//
  switch (xtc->contains.id())
  {
    //if it is a configuration then check what kind of configuration
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
            //clear the stored event//
            _storedEvent.channels().clear();
            _storedEvent.detectors().clear();
            //set the status flags//
            _storedEvent.isConfigured() = true;
            //extract all infos for the event//
            _storedEvent.sampleInterval() = config.horiz().sampInterval();
            std::cout <<"config:"<<std::endl;
            std::cout <<" SampleInterval: "<<config.horiz().sampInterval()<<std::endl;
            std::cout <<" NbrChannels: "<<config.nbrChannels()<<std::endl;

            //go through all channels and extract the infos for them//
            for (size_t i=0; i<config.nbrChannels();++i)
            {
              //create a new Channel//
              cass::REMI::Channel channel;
              //assing the right channel number to this channel//
              channel.channelNbr() = i;
              //extract the fullscale of the channel, convert V -> mV//
              channel.fullscale()  = static_cast<int16_t>(config.vert(i).fullScale()*1000.);
              //extract the offset of the channel, convert V -> mV//
              channel.offset()     = static_cast<int16_t>(config.vert(i).offset()*1000.);
              //add the channel to the event//
              _storedEvent.channels().push_back(channel);

              //debug output//
              std::cout <<" channel: "<<i<<std::endl;
              std::cout <<"  Fullscale: "<<config.vert(i).fullScale()*1000<<std::endl;
              std::cout <<"  Offset: " <<config.vert(i).offset()*1000.<<std::endl;
              std::cout <<"  SizeofVert: " <<sizeof(Pds::Acqiris::VertV1)<<std::endl;
            }
          }
          break;
                    default:
          printf("Unsupported acqiris configuration version %d\n",version);
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
        const Pds::Acqiris::DataDescV1 &datadesc = *reinterpret_cast<const Pds::Acqiris::DataDescV1*>(xtc->payload());
        REMIEvent &remievent = cassevent->REMIEvent();
        //first copy the stored configuration into the incoming remievent//
        remievent = _storedEvent;
        //initialize the rest of the values from the datadescriptor//
        //only if it is already configured//
        if (remievent.isConfigured())
        {
          //change the flag//
          remievent.isFilled() = true;
          //go through all channels//
          const Pds::Acqiris::DataDescV1 *dd = &datadesc;
          for (size_t ic=0;ic<remievent.channels().size();++ic)
          {
            //retrieve a reference instead of a pointer//
            const Pds::Acqiris::DataDescV1 &ddesc = *dd;
            //retrieve a reference to the channel we are working on//
            cass::REMI::Channel &chan = remievent.channels()[ic];
            //extract the vertical gain, that allows one to convert points from ADC Bytes to Volts (milli Volts)
            chan.gain() = ddesc.gain()*1000;
            //extact the horizontal position of the first point with respect to the trigger//
            chan.horpos() = ddesc.timestamp(0).horPos();
            //extact the vertical Offset //
            chan.offset() = static_cast<int16_t>(ddesc.offset()*1000.);
            //extract waveform//
            const short* waveform = ddesc.waveform();
            //we need to shift the pointer so that it looks at the first real point of the waveform//
            waveform += ddesc.indexFirstPoint();
            //resize the waveform vector to hold all the entries of the waveform//
            remievent.channels()[ic].waveform().resize(ddesc.nbrSamplesInSeg());
            //we have to swap the byte order for some reason that still has to be determined//
            for (size_t iw=0;iw<ddesc.nbrSamplesInSeg();++iw)
              remievent.channels()[ic].waveform()[iw] = (waveform[iw]&0x00ff<<8) | (waveform[iw]&0xff00>>8);

            //change to the next Channel//
            dd = dd->nextChannel();
          }
        }
      }
      break;
    }
        default:
    printf("xtc type \"%s\" is not handled by REMIConverter",Pds::TypeId::name(xtc->contains.id()));
    break;
  }
}

