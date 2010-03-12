#ifndef _ACQIRIS_DEVICE_H_
#define _ACQIRIS_DEVICE_H_

#include <vector>
#include "cass_acqiris.h"
#include "detector_backend.h"
#include "device_backend.h"
#include "channel.h"
#include "serializer.h"

namespace cass
{
  namespace ACQIRIS
  {
    class CASS_ACQIRISSHARED_EXPORT AcqirisDevice : public cass::DeviceBackend
    {
    public:
      AcqirisDevice()   {_version=1;}
      ~AcqirisDevice()  {}

    public:
      typedef std::vector<Channel> channels_t;
      typedef std::vector<DetectorBackend*> detectors_t;

    public:
      void serialize(cass::Serializer&)const;
      void deserialize(cass::Serializer&);

    public:
      const channels_t   &channels()const         {return _channels;}
      channels_t         &channels()              {return _channels;}
      const detectors_t  &detectors()const        {return _dets;}
      detectors_t        &detectors()             {return _dets;}

    private:
      //containers for acqiris and delayline data//
      channels_t          _channels;              //Container for all Channels
      detectors_t         _dets;                  //Container for all Detektors
    };
  }//end namespace acqiris
}//end namespace cass

inline void cass::ACQIRIS::AcqirisDevice::serialize(cass::Serializer &out) const
{
  //the version//
  out.addUint16(_version);
  //copy the size of the channels and then all channels//
  size_t nChannels = _channels.size();
  out.addSizet(nChannels);
  for(channels_t::const_iterator it=_channels.begin(); it != _channels.end(); ++it)
    it->serialize(out);
}
inline void cass::ACQIRIS::AcqirisDevice::deserialize(cass::Serializer &in)
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
  for(channels_t::iterator it=_channels.begin(); it != _channels.end(); ++it)
    it->deserialize(in);
}

#endif
