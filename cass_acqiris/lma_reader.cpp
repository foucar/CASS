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
#include "lma_file_header.h"

using namespace cass;
using namespace cass::ACQIRIS;
using namespace std;

LmaReader::LmaReader()
{}

void LmaReader::loadSettings()
{
}

void LmaReader::readHeaderInfo(std::ifstream &file)
{
  vector<char> headerarray(sizeof(lmaFile::GeneralHeader));
  file.read(&headerarray.front(),sizeof(lmaFile::GeneralHeader));
  const lmaFile::GeneralHeader &header
      (*reinterpret_cast<lmaFile::GeneralHeader*>(&headerarray.front()));
  if (header.nbrBits != 16)
  {
    stringstream ss;
    ss<<"LmaReader(): The lma file seems to contain 8-bit wavefroms '"<<header.nbrBits
     <<"'. Currently this is not supported.";
    throw runtime_error(ss.str());
  }
  _instrument.channels().resize(header.nbrChannels);
  _usedChannelBitmask = header.usedChannelBitmask;

  cout <<"LMAReader(): File contains instrument with '"<<header.nbrChannels
       <<"' channels:"<<endl;
  for (int16_t i(0) ; i < header.nbrChannels ;++i)
  {
    Channel &chan(_instrument.channels()[i]);
    chan.sampleInterval() = header.samplingInterval;
    chan.waveform().resize(header.nbrSamples);
    chan.channelNbr() = i;

    cout <<"LMAReader(): Channel '"<<i<<"' is "
         <<((_usedChannelBitmask & (0x1<<i))?"":"not")<<" recorded!"<<endl;

    if (_usedChannelBitmask & (0x1<<i))
    {
      vector<char> chanheaderarray(sizeof(lmaFile::ChannelHeader));
      file.read(&chanheaderarray.front(),sizeof(lmaFile::ChannelHeader));
      const lmaFile::ChannelHeader &chanheader
          (*reinterpret_cast<lmaFile::ChannelHeader*>(&chanheaderarray.front()));

      chan.offset() = chanheader.offset_mV*1e-3;
      chan.gain() = chanheader.gain_mVperLSB*1e-3;
      waveform_t & waveform (chan.waveform());
      fill(waveform.begin(),waveform.end(),chanheader.offset_mV/chanheader.gain_mVperLSB);
    }
  }
}

bool LmaReader::operator ()(ifstream &file, CASSEvent& evt)
{
  /** extract the right device from the cassevent */
  Device *dev(dynamic_cast<Device*>(evt.devices()[CASSEvent::Acqiris]));
  /** extract the right instrument from the acqiris device */
  Instrument &instr(dev->instruments()[Standalone]);
  /** copy the header information to the instrument */
  instr = _instrument;
  Instrument::channels_t &channels(instr.channels());

  /** read the acqiris data from the file */
  evt.id() = (FileStreaming::retrieve<int32_t>(file));
  double horpos(FileStreaming::retrieve<double>(file));

  for (size_t i=0; i<channels.size();++i)
  {
    Channel &chan(instr.channels()[i]);
    chan.horpos() = horpos;
    waveform_t &waveform(chan.waveform());
    if (_usedChannelBitmask & (0x1<<i))
    {
      /** since we zero substracted the wavefrom in the lma file, we need to
       *  create the right wavefrom again. This is done by going through the
       *  wavefrom sinpplets called pulses and put them at the right position in
       *  the wavefrom of the channel.
       */
      int16_t nbrPulses(FileStreaming::retrieve<int16_t>(file));
      for (int16_t i(0); i < nbrPulses; ++i)
      {
        /** read the puls properties from file */
        int32_t wavefromOffset(FileStreaming::retrieve<int32_t>(file));
        int32_t pulslength(FileStreaming::retrieve<int32_t>(file));
        size_t dataSize(pulslength * 2);
        file.read(reinterpret_cast<char*>(&(*(waveform.begin()+wavefromOffset))),
                  dataSize);
      }
    }
  }

  return evt.id();
}
