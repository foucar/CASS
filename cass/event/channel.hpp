//Copyright (C) 2009,2010, 2015 Lutz Foucar

/**
 * @file channel.hpp file contains the classes that describe an acqiris channel
 *
 * @author Lutz Foucar
 */

#ifndef _CHANNEL_HPP_
#define _CHANNEL_HPP_


#include <stdint.h>
#include <iostream>
#include <vector>

#include "serializer.hpp"
#include "serializable.hpp"

namespace cass
{
namespace ACQIRIS
{
/** A Channel of an Acqiris Instrument.
 *
 * contains all information that an Acqiris instrument will provide about
 * a channel
 *
 * @author Lutz Foucar
 */
class Channel : public Serializable
{
public:
  /** define the waveform */
  typedef std::vector<int16_t> waveform_t;

  /** constructor that will set the serialize version*/
  Channel()
    : Serializable(1),
      _chNbr(0),
      _horpos(0),
      _offset(0),
      _gain(0),
      _sampleInterval(0)
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
    /** output all variables obtained from the acqiris */
    out.add(_horpos);
    out.add(_offset);
    out.add(_gain);
    out.add(_sampleInterval);
    /** write the length of the waveform */
    out.add(static_cast<size_t>(_waveform.size()));
    /** the waveform itselve */
    for (waveform_t::const_iterator it=_waveform.begin();it!=_waveform.end();++it)
      out.add(*it);
  }

  /** deserialize this channel from the serializer
   *
   * @param in the stream to serialze this class from
   */
  bool deserialize(SerializerBackend &in)
  {
    checkVersion(in);
    /** read all variables and then the waveform from the input stream */
    _horpos = in.retrieve<double>();
    _offset = in.retrieve<double>();
    _gain = in.retrieve<double>();
    _sampleInterval = in.retrieve<double>();
    /** read the length of the waveform */
    size_t nSamples(in.retrieve<size_t>());
    /** clear the wavefrom */
    _waveform.clear();
    for (size_t i(0);i<nSamples;++i)
      _waveform.push_back(in.retrieve<waveform_t::value_type>());
    return true;
  }

public:
  //@{
  /** setter */
  double            &horpos()               {return _horpos;}
  double            &offset()               {return _offset;}
  double            &sampleInterval()       {return _sampleInterval;}
  double            &gain()                 {return _gain;}
  waveform_t        &waveform()             {return _waveform;}
  size_t            &channelNbr()           {return _chNbr;}
  //@}
  //@{
  /** getter */
  double             horpos()const          {return _horpos;}
  double             offset()const          {return _offset;}
  double             gain()const            {return _gain;}
  double             sampleInterval()const  {return _sampleInterval;}
  const waveform_t  &waveform()const        {return _waveform;}
  size_t             channelNbr()const      {return _chNbr;}
  //@}

  /** getter
   *
   * will calculate the fullscale from the gain value, provided by the
   * instrument
   */
  double             fullscale()const       {return _gain*0xffff;}

private:
  /** This Channels Number in the Acqiris Instrument*/
  size_t _chNbr;

  /** Horizontal position of first data point with respect to the trigger*/
  double _horpos;

  /** the vertical offset of this channel (in V)*/
  double _offset;

  /** Vertical gain in Volts/LSB. (V = vGain * data - vOffset)*/
  double _gain;

  /** the time between two consecutive datapoints in seconds*/
  double _sampleInterval;

  /** the waveform of this channel*/
  waveform_t  _waveform;
};

}//end namespace ACQIRIS
}//end namespace cass

#endif
