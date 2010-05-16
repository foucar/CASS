//Copyright (C) 2010 Lutz Foucar

#include <iostream>

#include "acqiris_device.h"
#include "serializer.h"


cass::ACQIRIS::Device::Device()
  :cass::DeviceBackend(1)
{
  //create all instruments
//  _instruments[Camp1] = Instrument();
}

void cass::ACQIRIS::Device::serialize(cass::SerializerBackend &out)
{
  //the version
  out.addUint16(_version);
  //the instruments
  for(instruments_t::iterator it=_instruments.begin(); it != _instruments.end(); ++it)
      it->second.serialize(out);
}

bool cass::ACQIRIS::Device::deserialize(cass::SerializerBackend &in)
{
  //check whether the version fits//
  uint16_t ver = in.retrieveUint16();
  if(ver!=_version)
  {
    std::cerr<<"version conflict in acqiris: "<<ver<<" "<<_version<<std::endl;
    return false;
  }
  //the instruments//
  for(instruments_t::iterator it=_instruments.begin();
      it != _instruments.end();
      ++it)
    it->second.deserialize(in);
  return true;
}




//--the instruments--

void cass::ACQIRIS::Instrument::serialize(cass::SerializerBackend &out)
{
  //the version//
  out.addUint16(_version);
  //copy the size of the channels and then all channels//
  size_t nChannels = _channels.size();
  out.addSizet(nChannels);
  for(channels_t::iterator it=_channels.begin(); it != _channels.end(); ++it)
      it->serialize(out);
}

bool cass::ACQIRIS::Instrument::deserialize(cass::SerializerBackend &in)
{
  //check whether the version fits//
  uint16_t ver = in.retrieveUint16();
  if(ver!=_version)
  {
    std::cerr<<"version conflict in acqiris: "<<ver<<" "<<_version<<std::endl;
    return false;
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
  return true;
}
