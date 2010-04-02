//Copyright (C) 2010 lmf

#include <iostream>

#include "acqiris_device.h"
#include "serializer.h"


cass::ACQIRIS::Device::Device()
  :cass::DeviceBackend(1)
{
  //create all detectors that will be attached to the instruments//
//  _detectors[HexDetector] = new DelaylineDetector();
//  _detectors[QuadDetector] = new DelaylineDetector();
}

cass::ACQIRIS::Device::~Device()
{
 /* //delete all detectors
  for (detectors_t::iterator it=_devices.begin() ; it != _devices.end(); ++it )
    delete (it->second);
*/
}

void cass::ACQIRIS::Device::serialize(cass::Serializer &out) const
{
  //the version//
  out.addUint16(_version);

  //the instruments
  for(instruments_t::const_iterator it=_instruments.begin();
      it != _instruments.end();
      ++it)
    it->second.serialize(out);
}

void cass::ACQIRIS::Device::deserialize(cass::Serializer &in)
{
  //check whether the version fits//
  uint16_t ver = in.retrieveUint16();
  if(ver!=_version)
  {
    std::cerr<<"version conflict in acqiris: "<<ver<<" "<<_version<<std::endl;
    return;
  }
  //the instruments//
  for(instruments_t::iterator it=_instruments.begin();
      it != _instruments.end();
      ++it)
    it->second.deserialize(in);
}




//--the instruments--

void cass::ACQIRIS::Instrument::serialize(cass::Serializer &out) const
{
  //the version//
  out.addUint16(_version);
  //copy the size of the channels and then all channels//
  size_t nChannels = _channels.size();
  out.addSizet(nChannels);
  for(channels_t::const_iterator it=_channels.begin();
      it != _channels.end();
      ++it)
    it->serialize(out);
}

void cass::ACQIRIS::Instrument::deserialize(cass::Serializer &in)
{
  //check whether the version fits//
  uint16_t ver = in.retrieveUint16();
  if(ver!=_version)
  {
    std::cerr<<"version conflict in acqiris: "<<ver<<" "<<_version<<std::endl;
    return;
  }
  //read how many channels//
  size_t nChannels= in.retrieveSizet();
  //make the channels container big enough//
  _channels.resize(nChannels);
  //deserialize all channels//
  for(channels_t::iterator it=_channels.begin();
      it != _channels.end();
      ++it)
    it->deserialize(in);
}
