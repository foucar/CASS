//Copyright (C) 2009,2010 lmf

#ifndef _ACQIRIS_DEVICE_H_
#define _ACQIRIS_DEVICE_H_

#include <vector>
#include <map>

#include "cass_acqiris.h"
#include "device_backend.h"
#include "channel.h"
#include "serializable.h"

namespace cass
{
  namespace ACQIRIS
  {
    /*! An Acqiris Instrument

    An Acqiris Instrument represents the actual Acqiris (Multi)-Instrument,
    which contains a lot of channels

    @author lmf
    */
    class CASS_ACQIRISSHARED_EXPORT Instrument : public cass::Serializable
    {
    public:
      Instrument()
        :Serializable(1)
      {}

    public:
      /** @typedef a vector of Channels for more readable code*/
      typedef std::vector<Channel> channels_t;

    public:
      /** will serialize all channels to Serializer*/
      virtual void serialize(cass::Serializer &) const;
      /** will deserialize all channels from the Serializer*/
      virtual void deserialize(cass::Serializer &);

    public:
      /** @returns the channels of this instrument*/
      const channels_t &channels()const {return _channels;}
      /** @returns a reference, so that one can edit the channels*/
      channels_t &channels()  {return _channels;}

    private:
      /** Container for all Channels */
      channels_t _channels;
    };




    /*! The Acqiris device

    The Acqiris device contains all availabe Acqiris instruments

    @author lmf
    */
    class CASS_ACQIRISSHARED_EXPORT Device : public cass::DeviceBackend
    {
    public:
      /** default constructor creates all instruments*/
      Device();

    public:
      /** @typedef instruments_t a map of all instruments available*/
      typedef std::map<Instruments, Instrument> instruments_t;

    public:
      /** instrument setter*/
      instruments_t &instruments()  {return _instruments;}
      /** instrument getter*/
      const instruments_t &instruments()const  {return _instruments;}

    public:
      /** will serialize all channels to Serializer*/
      virtual void serialize(cass::Serializer &) const;
      /** will deserialize all channels from the Serializer*/
      virtual void deserialize(cass::Serializer &);

    private:
      /** Container for all Instruments*/
      instruments_t _instruments;
    };
  }//end namespace acqiris
}//end namespace cass


#endif
