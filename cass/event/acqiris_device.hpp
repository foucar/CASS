//Copyright (C) 2009, 2010, 2015 Lutz Foucar

/**
 * @file acqiris_device.hpp file contains the declaration of the acqiris part
 *                          of the CASSEvent
 *
 * @author Lutz Foucar
 */

#ifndef _ACQIRIS_DEVICE_HPP_
#define _ACQIRIS_DEVICE_HPP_

#include <vector>
#include <map>

#include "device_backend.hpp"
#include "channel.hpp"
#include "serializable.hpp"

namespace cass
{
namespace ACQIRIS
{
/** An Acqiris Instrument.
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

public:
  /** will serialize all channels to Serializer
   *
   * @param out the stream to serialze this class to
   */
  virtual void serialize(SerializerBackend &out)const
  {
    writeVersion(out);
    /** copy the size of the channels and then all channels */
    out.add(static_cast<size_t>(_channels.size()));
    for(channels_t::const_iterator it=_channels.begin(); it != _channels.end(); ++it)
      it->serialize(out);
  }

  /** will deserialize all channels from the Serializer
   *
   * @return true when this class was deserialized from the stream sucessfully
   * @param in the stream to serialize this class from
   */
  virtual bool deserialize(SerializerBackend &in)
  {
    checkVersion(in);
    /** read how many channels */
    size_t nChannels(in.retrieve<size_t>());
    /** clear the channels container and deserialize the channels from the stream */
    _channels.clear();
    for(size_t i(0); i < nChannels; ++i)
      _channels.push_back(Channel(in));
    return true;
  }

public:
  /** @returns the channels of this instrument*/
  const channels_t &channels()const {return _channels;}

  /** @returns a reference, so that one can edit the channels*/
  channels_t &channels()  {return _channels;}

  /** @returns the event id */
  uint64_t id()const {return _eventID;}

  /** @returns reference to the eventid */
  uint64_t &id() {return _eventID;}

private:
  /** Container for all Channels */
  channels_t _channels;

  /** the eventid that this detector belongs to (can be used for crosschecks */
  uint64_t _eventID;
};




/** The Acqiris device
 *
 * The Acqiris device contains all availabe Acqiris instruments
 * All availabe instruments will be added when they are detected
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
  /** a map of all instruments available*/
  typedef std::map<uint32_t, Instrument> instruments_t;

public:
  /** instrument setter*/
  instruments_t &instruments()  {return _instruments;}

  /** instrument getter*/
  const instruments_t &instruments()const  {return _instruments;}

public:
  /** will serialize all channels to Serializer
   *
   * @param out the stream to serialze this class to
   */
  virtual void serialize(SerializerBackend &out)const
  {
    writeVersion(out);
    /** write the size of the container */
    out.add(static_cast<size_t>(_instruments.size()));
    /** for each instrument in the map write the key and then the Instrument */
    for (instruments_t::const_iterator it = _instruments.begin (); it != _instruments.end (); ++it)
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
  virtual bool deserialize(SerializerBackend &in)
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
  /** Container for all Instruments*/
  instruments_t _instruments;
};
}//end namespace acqiris
}//end namespace cass


#endif
