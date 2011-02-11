//Copyright (C) 2011 Lutz Foucar

/**
 * @file acqiris_device.h file contains the declaration of the acqiristdc part
 *                        of the CASSEvent
 *
 * @author Lutz Foucar
 */

#ifndef _ACQIRISTDC_DEVICE_H_
#define _ACQIRISTDC_DEVICE_H_

#include <vector>
#include <map>

#include "cass_acqiris.h"
#include "device_backend.h"
#include "serializable.h"

namespace cass
{
  namespace ACQIRISTDC
  {
    /** A Channel of an Acqiris TDC Instrument.
     *
     * contains all information that an Acqiris TDC instrument will provide about
     * a channel and the recorded hits of that channel
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT Channel : public cass::Serializable
    {
    public:
      /** typedef showing the */
      typedef std::vector<double> hits_t;

      /** constructor that will set the serialize version*/
      Channel():Serializable(1)  {}

    public:
      /** will serialize this channel to the serializer*/
      void serialize(cass::SerializerBackend&)const;

      /** deserialize this channel from the serializer*/
      bool deserialize(cass::SerializerBackend&);

      /** getter */
      const hits_t &hits()const {return _hits;}

      /** setter */
      hits_t &hits() {return _hits;}

    private:
      /** the hits of the channel */
      hits_t _hits;
    };

    /** An Acqiris TDC Instrument.
     *
     * An Acqiris Instrument represents the actual Acqiris (Multi)-Instrument,
     * which contains a lot of channels
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT Instrument : public cass::Serializable
    {
    public:
      /** constructor */
      Instrument():Serializable(1)  {}

    public:
      /** a vector of Channels */
      typedef std::vector<Channel> channels_t;

      /** there is a fixed size of channels in this instrument */
      enum {NbrChannels=6};

    public:
      /** will serialize all channels to Serializer*/
      void serialize(cass::SerializerBackend &)const;

      /** will deserialize all channels from the Serializer*/
      bool deserialize(cass::SerializerBackend &);

    public:
      /** @returns the channels of this instrument*/
      const channels_t &channels()const {return _channels;}

      /** @returns a reference, so that one can edit the channels*/
      channels_t &channels()  {return _channels;}

    private:
      /** Container for all Channels */
      channels_t _channels;
    };




    /** The Acqiris TDC device
     *
     * The Acqiris TDC device contains all availabe Acqiris TDC instruments
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT Device : public cass::DeviceBackend
    {
    public:
      /** constructor */
      Device(): cass::DeviceBackend(1) {}

    public:
      /** a map of all instruments available */
      typedef std::map<uint32_t, Instrument> instruments_t;

    public:
      /** instrument setter */
      instruments_t &instruments()  {return _instruments;}

      /** instrument getter */
      const instruments_t &instruments()const  {return _instruments;}

    public:
      /** will serialize all channels to Serializer*/
      void serialize(cass::SerializerBackend &)const;

      /** will deserialize all channels from the Serializer */
      bool deserialize(cass::SerializerBackend &);

    private:
      /** Container for all Instruments */
      instruments_t _instruments;
    };
  }//end namespace acqiris
}//end namespace cass


#endif
