// Copyright (C) 2011 Lutz Foucar

/**
 * @file lma_reader.cpp contains the class to read lma files
 *
 * @author Lutz Foucar
 */

#include <stdint.h>
#include <algorithm>
#include <vector>

#include "lma_reader.h"

#include "cass_event.h"
#include "cass_settings.h"
#include "agattypes.h"
#include "log.h"

using namespace cass;
using namespace cass::ACQIRIS;
using namespace std;
using Streaming::operator >>;

LmaReader::LmaReader()
{}

void LmaReader::loadSettings()
{
}

void LmaReader::readHeaderInfo(std::ifstream &file)
{
  lmaHeader::General header;
  file >> header;

  if (header.nbrBits != 16)
    throw runtime_error("LMAParser():run: The lma file seems to contain 8-bit wavefroms '"
                        + toString(header.nbrBits) + "'. Currently this is not supported.");

  _instrument.channels().resize(header.nbrChannels);
  _usedChannelBitmask = header.usedChannelBitmask;

  Log::add(Log::VERBOSEINFO,"LMAReader(): File contains instrument with '" +
           toString(header.nbrChannels)  +"' channels:");
  for (int16_t i(0) ; i < header.nbrChannels ;++i)
  {
    Channel &chan(_instrument.channels()[i]);
    chan.sampleInterval() = header.samplingInterval;
    chan.waveform().resize(header.nbrSamples);
    chan.channelNbr() = i;

    Log::add(Log::VERBOSEINFO,"LMAReader(): Channel '" + toString(i) + "' is " +
             ((_usedChannelBitmask & (0x1<<i))?"":"not") + " recorded!");

    if (_usedChannelBitmask & (0x1<<i))
    {
      lmaHeader::Channel chanheader;
      file >> chanheader;
      chan.offset() = chanheader.offset_mV*1e-3;
      chan.gain() = chanheader.gain_mVperLSB*1e-3;
      waveform_t & waveform (chan.waveform());
      fill(waveform.begin(),waveform.end(),chanheader.offset_mV/chanheader.gain_mVperLSB);
    }
  }
}

bool LmaReader::operator ()(ifstream &file, CASSEvent& evt)
{
  /** extract the right instrument from the cassevent */
  Device *dev(dynamic_cast<Device*>(evt.devices()[CASSEvent::Acqiris]));
  Instrument &instr(dev->instruments()[Standalone]);
  /** copy the header information to the instrument */
  instr = _instrument;
  Instrument::channels_t &channels(instr.channels());

  /** read the event information data from the file */
  lmaHeader::Event evtHead;
  file >> evtHead;
  evt.id() = evtHead.id;

  for (size_t i=0; i<channels.size();++i)
  {
    Channel &chan(instr.channels()[i]);
    chan.horpos() = evtHead.horpos;
    waveform_t &waveform(chan.waveform());
    if (_usedChannelBitmask & (0x1<<i))
    {
      /** since we zero substracted the wavefrom in the lma file, we need to
       *  create the right wavefrom again. This is done by going through the
       *  wavefrom sinpplets called pulses and put them at the right position in
       *  the wavefrom of the channel.
       */
      int16_t nbrPulses(Streaming::retrieve<int16_t>(file));
      for (int16_t i(0); i < nbrPulses; ++i)
      {
        /** read the puls properties from file */
        lmaHeader::Puls pulsHead;
        file >> pulsHead;
        size_t dataSize(pulsHead.length * 2);
        file.read(reinterpret_cast<char*>(&waveform[pulsHead.idxPos]),
                  dataSize);
      }
    }
  }

  return evt.id();
}
