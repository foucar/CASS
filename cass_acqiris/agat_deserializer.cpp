// Copyright (C) 2011 Lutz Foucar

/**
 * @file agat_deserializer.cpp contains functions to deserialize the data stream
 *                             sent by agat.
 *
 * @author Lutz Foucar
 */

#include <algorithm>

#include <QtCore/QDataStream>

#include "agat_deserializer.h"

#include "cass_event.h"
#include "acqiris_device.h"

using namespace cass;
using namespace ACQIRIS;
using namespace std;

namespace cass
{
namespace AGATHeader
{
#pragma pack(2)
/** the general stream header
 *
 * @author Lutz Foucar
 */
struct Event
{
  /** the event id */
  uint32_t headersize;

  /** horpos value from acqiris */
  double horpos;

  /** the size of the entries in the waveform in bits */
  int16_t nbrBits;

  /** the sampling interval (the time between two datapoints in s */
  double samplingInterval;

  /** number of samples in the waveform */
  int32_t nbrSamples;

  /** time between trigger and first sample */
  double delayTime;

  /** the triggering channel */
  int16_t triggerChannel;

  /** triggering level of the trigger in V */
  double triggerLevel;

  /** on which slope the instruments triggers on */
  int16_t triggerSlope;

  /** how many converters are being used per channel */
  int16_t nbrConvertersPerChan;

  /** bitmask describing which channels are combined */
  int32_t channelCombinationBitmask;

  /** bitmask describing which channels are recorded */
  uint32_t usedChannelBitmask;

  /** number of channels present in the acqiris instrument */
  uint16_t nbrChannels;
};

/** the header of an recorded channel
 *
 * @author Lutz Foucar
 */
struct Channel
{
  /** the full scale of the channel in mV */
  int16_t fullscale_mV;

  /** the offset of the channel in mV */
  int16_t offset_mV;

  /** the vertical Gain (conversion factor to convert the bits to mV) */
  double gain_mVperLSB;

  /** baseline that was used for zero substraction in digitizer units */
  int16_t baseline;

  /** noiselevel for zero substraction in digitizer units
   *
   * the zero substaction will check whether a value of the recorded waveform
   * is outside the noiselevel. Mathematically:
   * \f \left| value_Du - baseline \right| > noiselevel \f
   */
  int16_t noiseLevel;

  /** stepsize in sample interval units
   *
   * after finding a waveform value thats outside the noiselevel this is the
   * amount of values skipped before checking whether the wavefrom is in the
   * noiselevel again.
   */
  int32_t stepsize;

  /** backsize of the zerosubstraction in sample interval units
   *
   * how many steps we should go back after a value is outside the noiselevel
   * to start recording the wavefrom values
   *
   * puls of waveform outside noise := puls;
   * waveform index first value outside noise := puls[i]
   * puls.begin = puls[i-backsize]
   */
  int32_t backsize;

  /** the number of pulses contained in this channel */
  uint16_t nbrPulses;
};

/** the header of a puls
 *
 * @author Lutz Foucar
 */
struct Puls
{
  /** the start index position of this puls in the original waveform */
  uint32_t idxPos;

  /** how many points of the wavefrom are in the puls */
  uint32_t length;
};

/** reading the Header parts from the QDataStream
 *
 * @tparam T the type that should be read from the stream
 * @return reference to the stream
 * @param stream the stream to read from
 * @param evt the header to read to
 *
 * @author Lutz Foucar
 */
template<typename T>
QDataStream &operator>>(QDataStream& stream, T& evt)
{
  if(stream.readRawData(reinterpret_cast<char*>(&evt),sizeof(T)) != sizeof(T))
    throw runtime_error("operator>>(QDdataStream&,T&): could not retrieve the right size");
  return stream;
}

} //end namespace AGATStreamHeader
#pragma pack()
} //end namespace cass


bool deserializeNormalAgat::operator ()(QDataStream& stream, CASSEvent& evt)
{
  /** extract the right device from the cassevent */
  if (evt.devices().find(CASSEvent::Acqiris) == evt.devices().end())
    throw runtime_error("...");
  Device *dev(dynamic_cast<Device*>(evt.devices()[CASSEvent::Acqiris]));
  /** extract the right instrument from the acqiris device */
  Instrument &instr(dev->instruments()[Standalone]);
  /** read the event header */
  AGATHeader::Event evtHead;
  stream >> evtHead;
  /** resize the channel container to fit the right number of channels */
  instr.channels().resize(evtHead.nbrChannels);
  /** for each channel */
  for (size_t iChan(0); iChan < evtHead.nbrChannels; ++iChan)
  {
    /** check whether it is contained in the stream */
    if (evtHead.usedChannelBitmask & (0x1 << iChan))
    {
      /** if the channel is contained in the stream */
      /** read the channel header */
      AGATHeader::Channel chanHead;
      stream >> chanHead;
      /** copy channel parameters from the header to the cassevent */
      Channel &chan(instr.channels()[iChan]);
      chan.channelNbr() = iChan;
      chan.offset() = chanHead.offset_mV*1e-3;
      chan.gain() = chanHead.gain_mVperLSB*1e-3;
      /** resize the wavefrom so that all the pulses will fit into it */
      waveform_t& waveform(chan.waveform());
      waveform.resize(evtHead.nbrSamples);
      /** set all value to the basevalue */
      fill(waveform.begin(),waveform.end(),chanHead.offset_mV/chanHead.gain_mVperLSB);
      /** for all pulses within that channel */
      for (size_t iPuls(0); iPuls < chanHead.nbrPulses; ++iPuls)
      {
        /** read the puls header and determine the length of the puls form it*/
        AGATHeader::Puls pulsHead;
        stream >> pulsHead;
        size_t datasize = pulsHead.length * evtHead.nbrBits/8;
        /** read the puls data into the wavefrom at the right position */
        stream.readRawData(reinterpret_cast<char*>(&waveform[pulsHead.idxPos]),
                           datasize);
      }
    }
  }
  return true;
}

