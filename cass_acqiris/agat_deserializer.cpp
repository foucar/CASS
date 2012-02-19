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

#include "cass.h"
#include "cass_event.h"
#include "acqiris_device.h"
#include "agattypes.h"

using namespace cass;
using namespace ACQIRIS;
using namespace std;
using Streaming::operator >>;


size_t AGATStreamer::operator ()(QDataStream& stream, CASSEvent& evt)
{
  size_t nBytesRead(0);
  /** extract the right instrument from the cassevent */
  if (evt.devices().find(CASSEvent::Acqiris) == evt.devices().end())
    throw runtime_error("deserializeNormalAgat(): The Acqiris Device does not exist.");
  Device *dev(dynamic_cast<Device*>(evt.devices()[CASSEvent::Acqiris]));
  Instrument &instr(dev->instruments()[Standalone]);

  /** read the event header and copy the id */
  AGATRemoteHeader::Event evtHead;
  stream >> evtHead;
  nBytesRead += sizeof(AGATRemoteHeader::Event);
  evt.id() = evtHead.id;
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
      AGATRemoteHeader::Channel chanHead;
      stream >> chanHead;
      nBytesRead += sizeof(AGATRemoteHeader::Channel);
      /** copy channel parameters from the header to the cassevent */
      Channel &chan(instr.channels()[iChan]);
      chan.channelNbr() = iChan;
      chan.offset() = chanHead.offset_mV*1e-3;
      chan.gain() = chanHead.gain_mVperLSB*1e-3;
      chan.sampleInterval() = evtHead.samplingInterval;
      /** resize the wavefrom so that all the pulses will fit into it */
      waveform_t& waveform(chan.waveform());
      waveform.resize(evtHead.nbrSamples);
      /** set all value to the basevalue */
      fill(waveform.begin(),waveform.end(),chanHead.offset_mV/chanHead.gain_mVperLSB);
      /** for all pulses within that channel */
      for (size_t iPuls(0); iPuls < chanHead.nbrPulses; ++iPuls)
      {
        /** read the puls header and determine the length of the puls form it*/
        lmaHeader::Puls pulsHead;
        stream >> pulsHead;
        nBytesRead += sizeof(lmaHeader::Puls);
        size_t datasize = pulsHead.length * evtHead.nbrBits/8;
        /** read the puls data into the wavefrom at the right position */
        stream.readRawData(reinterpret_cast<char*>(&waveform[pulsHead.idxPos]),
                           datasize);
        nBytesRead += datasize;
      }
    }
  }
  return true;
}

