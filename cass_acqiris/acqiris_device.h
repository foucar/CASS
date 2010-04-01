//Copyright (C) 2009,2010 lmf

#ifndef _ACQIRIS_DEVICE_H_
#define _ACQIRIS_DEVICE_H_

#include <vector>
#include "cass_acqiris.h"
#include "detector_backend.h"
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
        :_version(1)
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
      /** version for de/serializing*/
      uint16_t _version;
    };




    /*! The Acqiris device

    The Acqiris device contains all availabe Acqiris instruments

    @author lmf
    */
    class CASS_ACQIRISSHARED_EXPORT Device : public cass::DeviceBackend
    {
    public:
      /** default constructor creates all detectors*/
      Device();

      /** destructor deletes all detectors*/
      ~Device();

    public:
      /** @typedef a map of available detectors,
      that can be attached to an acqiris channel*/
      typedef std::map<cass::ACQIRIS::Detectors,DetectorBackend*> detectors_t;
      /** @typedef a map of all instruments available*/
      typedef std::map<cass::ACQIRIS::Instruments, Instrument> instruments_t;

    public:
      /** will serialize all channels to Serializer*/
      virtual void serialize(cass::Serializer &) const;
      /** will deserialize all channels from the Serializer*/
      virtual void deserialize(cass::Serializer &);

    public:
      /** @returns the detectors attached to the Acqiris instruments*/
      const detectors_t &detectors()const {return _detectors;}
      /** @overload @returns the detectors attached to the Acqiris instruments to modify them*/
      detectors_t &detectors()  {return _detectors;}

    private:
      /** Container for all Instruments*/
      instruments_t _instruments;
      /** Container for all Detektors attached to the instruments*/
      detectors_t _detectors;
    };
  }//end namespace acqiris
}//end namespace cass


#endif
