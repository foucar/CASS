//Copyright (C) 2011, 2015 Lutz Foucar

/**
 * @file acqiristdc_device.hpp file contains the declaration of the acqiristdc
 *                             part of the CASSEvent
 *
 * @author Lutz Foucar
 */

#ifndef _ACQIRISTDC_DEVICE_HPP_
#define _ACQIRISTDC_DEVICE_HPP_

#include <vector>
#include <map>

#include "device_backend.hpp"
#include "serializable.hpp"

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
class Channel : public Serializable
{
public:
  /** define the hits */
  typedef std::vector<double> hits_t;

  /** constructor that will set the serialize version*/
  Channel()
    : Serializable(1)
  {}

  /** construct this from the stream
   *
   * @param in the input stream to read this class from
   */
  Channel(SerializerBackend &in)
    : Serializable(1)
  {
    deserialize(in);
  }

public:
  /** will serialize this channel to the serializer
   *
   * @param out the stream to serialze this class to
   */
  void serialize(SerializerBackend &out)const
  {
    writeVersion(out);
    /** write the number of hits */
    const size_t nHits = _hits.size();
    out.add(nHits);
    /** write the hits */
    for (hits_t::const_iterator it(_hits.begin()); it!=_hits.end(); ++it)
      out.add(*it);
  }

  /** deserialize this channel from the stream
   *
   * @return true when this class was deserialized from the stream sucessfully
   * @param in the stream to serialize this class from
   */
  bool deserialize(SerializerBackend &in)
  {
    checkVersion(in);
    /** read the number of hits */
    size_t nHits(in.retrieve<size_t>());
    /** clear the hit container and read the hits */
    _hits.clear();
    for (size_t i(0); i < nHits; ++i)
      _hits.push_back(in.retrieve<double>());
    return true;
  }

  /** retireve hits */
  const hits_t &hits()const {return _hits;}

  /** retrieve hits*/
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
class Instrument : public Serializable
{
public:
  /** constructor */
  Instrument()
    : Serializable(1)
  {}

  /** constuct class from stream
   *
   * @param in the stream to construct this class from
   */
  Instrument(SerializerBackend &in)
    : Serializable(1)
  {
    deserialize(in);
  }

public:
  /** a vector of Channels */
  typedef std::vector<Channel> channels_t;

  /** there is a fixed size of channels in this instrument */
  enum {NbrChannels=6};

public:
  /** will serialize all channels to Serializer
   *
   * @param out the stream to serialze this class to
   */
  void serialize(SerializerBackend &out)const
  {
    writeVersion(out);
    /** serialize the size of the channels and then all channels */
    size_t nChannels = _channels.size();
    out.add(nChannels);
    channels_t::const_iterator it(_channels.begin());
    for(; it != _channels.end(); ++it)
      it->serialize(out);
  }

  /** will deserialize all channels from the Serializer
   *
   * @return true when this class was deserialized from the stream sucessfully
   * @param in the stream to serialize this class from
   */
  bool deserialize(SerializerBackend &in)
  {
    checkVersion(in);
    /** read how many channels */
    size_t nChannels(in.retrieve<size_t>());
    /** clear the channel container and read the channels from the stream */
    _channels.clear();
    for(size_t i(0); i < nChannels ; ++i)
      _channels.push_back(Channel(in));
    return true;
  }

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
class Device : public DeviceBackend
{
public:
  /** constructor */
  Device()
    : DeviceBackend(1)
  {}

public:
  /** define the instruments container */
  typedef std::map<uint32_t, Instrument> instruments_t;

public:
  /** instrument setter */
  instruments_t &instruments()  {return _instruments;}

  /** instrument getter */
  const instruments_t &instruments()const  {return _instruments;}

public:
  /** will serialize all channels to Serializer
   *
   * @param out the stream to serialze this class to
   */
  void serialize(SerializerBackend &out)const
  {
    writeVersion(out);
    /** write the size of the instruments container */
    out.add(static_cast<size_t>(_instruments.size()));
    /** now write each instrument but first the key of the instrument */
    instruments_t::const_iterator it(_instruments.begin());
    for(; it != _instruments.end(); ++it)
    {
      out.add(it->first);
      it->second.serialize(out);
    }
  }

  /** will deserialize all channels from the Serializer
   *
   * @return true when this class was deserialized from the stream sucessfully
   * @param in the stream to serialize this class from
   */
  bool deserialize(SerializerBackend &in)
  {
    checkVersion(in);
    /** read the number of instruments */
    size_t nInstr(in.retrieve<size_t>());
    /** read the key of the instrument and add the deserialized instrument */
    for(size_t i(0); i < nInstr; ++i)
    {
      const instruments_t::key_type key(in.retrieve<instruments_t::key_type>());
      _instruments[key] = Instrument(in);
    }
    return true;
  }

private:
  /** Container for all Instruments */
  instruments_t _instruments;
};
}//end namespace acqiris
}//end namespace cass


#endif
