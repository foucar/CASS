//Copyright (C) 2010 Lutz Foucar

/**
 * @file acqiristdc_device.cpp file contains the definition of the acqiris tdc part
 *                             of the CASSEvent
 *
 * @author Lutz Foucar
 */

#include <iostream>

#include "acqiristdc_device.h"
#include "serializer.h"

using namespace std;
using namespace cass::ACQIRISTDC;

//-- the device--
void Device::serialize(cass::SerializerBackend &out)const
{
  //the version
  out.addUint16(_version);
  //the instruments
  instruments_t::const_iterator it(_instruments.begin());
  for(; it != _instruments.end(); ++it)
    it->second.serialize(out);
}

bool Device::deserialize(cass::SerializerBackend &in)
{
  //check whether the version fits//
  uint16_t ver = in.retrieveUint16();
  if(ver!=_version)
  {
    std::cerr<<"version conflict in acqiris: "<<ver<<" "<<_version<<std::endl;
    return false;
  }
  //the instruments//
  instruments_t::iterator it=_instruments.begin();
  for(; it != _instruments.end(); ++it)
    it->second.deserialize(in);
  return true;
}




//--the instrument--

void Instrument::serialize(cass::SerializerBackend &out)const
{
  //the version//
  out.addUint16(_version);
  //serializ the size of the channels and then all channels//
  size_t nChannels = _channels.size();
  out.addSizet(nChannels);
  channels_t::const_iterator it(_channels.begin());
  for(; it != _channels.end(); ++it)
    it->serialize(out);
}

bool Instrument::deserialize(cass::SerializerBackend &in)
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
  channels_t::iterator it (_channels.begin());
  for(; it != _channels.end(); ++it)
    it->deserialize(in);
  return true;
}


//-- the tdc channel --

void Channel::serialize(cass::SerializerBackend &out)const
{
  //the version//
  out.addUint16(_version);
  //the number of hits//
  const size_t nHits = _hits.size();
  out.addSizet(nHits);
  //the waveform itselve//
  for (hits_t::const_iterator it(_hits.begin()); it!=_hits.end(); ++it)
    out.addDouble(*it);
}

bool Channel::deserialize(cass::SerializerBackend &in)
{
  //check whether the version fits//
  uint16_t ver = in.retrieveUint16();
  if(ver!=_version)
  {
    std::cerr<<"ACQIRISTDC::Channel::deserialize():version conflict in tdc channel: "<<ver<<" "<<_version<<std::endl;
    return false;
  }
  //read the length of the waveform//
  size_t nHits = in.retrieveSizet();
  //make sure the waveform is long enough//
  _hits.resize(nHits);
  for (hits_t::iterator it(_hits.begin()); it!=_hits.end(); ++it)
    *it = in.retrieveDouble();
  return true;
}

