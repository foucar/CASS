// Copyright (C) 2011 Lutz Foucar

/**
 * @file lma_reader.cpp contains the class to read lma files
 *
 * @author Lutz Foucar
 */

#include <stdint.h>

#include "lma_reader.h"

#include "cass_event.h"
#include "cass_settings.h"

using namespace cass;
using namespace cass::ACQIRIS;
using namespace std;

namespace cass
{
  namespace lmareader
  {
    /** retrieve a variable from a file stream
     *
     * @return the variable
     * @tparam the type of the variable to retrieve
     * @param file The file stream to retrieve the vairable from
     */
    template <typename T>
    T retrieve(ifstream &file)
    {
      T var;
      file.read(reinterpret_cast<char*>(&var),sizeof(T));
      return var;
    }
  }
}
LmaReader::LmaReader()
  :_newFile(true)
{}

void LmaReader::loadSettings()
{
}

bool LmaReader::operator ()(ifstream &file, CASSEvent& evt)
{
  if (_newFile)
  {
    size_t headersizebytes(lmareader::retrieve<int32_t>(file));
    int16_t nbrChannels(lmareader::retrieve<int16_t>(file));
    int16_t nbrBits(lmareader::retrieve<int16_t>(file));
    double sampInter(lmareader::retrieve<double>(file));
    int32_t nbrSamples(lmareader::retrieve<int32_t>(file));
    double delayTime(lmareader::retrieve<double>(file));
    int16_t triggerChannel(lmareader::retrieve<int16_t>(file));
    double triggerLevel(lmareader::retrieve<double>(file));
    int16_t triggerSlope(lmareader::retrieve<int16_t>(file));
    _usedChannelBitmask = static_cast<uint32_t>(lmareader::retrieve<int32_t>(file));
    int32_t channelCombinationBitmask(lmareader::retrieve<int32_t>(file));
    int16_t nbrConvertersPerChan(lmareader::retrieve<int16_t>(file));

    if (nbrBits != 16)
    {
      stringstream ss;
      ss<<"LmaReader(): The lma file seems to contain 8-bit wavefroms '"<<nbrBits
          <<"'. Currently this is not supported.";
      throw runtime_error(ss.str());
    }
    _instrument.channels().resize(nbrChannels);

    for (int16_t i(0) ; i < nbrChannels ;++i)
    {
      Channel &chan(_instrument.channels()[i]);
      chan.sampleInterval() = sampInter;
      chan.waveform().resize(nbrSamples);
      chan.channelNbr() = i;

      if (_usedChannelBitmask & (0x1<<i))
      {
        int16_t fullscale_mV(lmareader::retrieve<int16_t>(file));
        int16_t offset_mV(lmareader::retrieve<int16_t>(file));
        //the vertical Gain (conversion factor to convert the bits to mV)
        double gain_mVperLSB(lmareader::retrieve<double>(file));
        int16_t baseline(lmareader::retrieve<int16_t>(file));
        int16_t noiseLevel(lmareader::retrieve<int16_t>(file));
        int32_t stepsize(lmareader::retrieve<int32_t>(file));
        int32_t backsize(lmareader::retrieve<int32_t>(file));


        chan.offset() = offset_mV*1e-3;
        chan.gain() = gain_mVperLSB*1e-3;
      }
    }
  }



  Device *dev(dynamic_cast<Device*>(evt.devices()[CASSEvent::Acqiris]));
  Instrument &instr(dev->instruments()[Standalone]);
  instr = _instrument;
  Instrument::channels_t &channels(instr.channels());

  evt.id() = (lmareader::retrieve<int32_t>(file));
  double horpos(lmareader::retrieve<double>(file));

  for (size_t i=0; i<channels.size();++i)
  {
    Channel &chan(instr.channels()[i]);
    chan.horpos() = horpos;
    waveform_t &waveform(chan.waveform());
    if (_usedChannelBitmask & (0x1<<i))
    {
      int16_t nbrPulses(lmareader::retrieve<int16_t>(file));
      for (int16_t i(0); i < nbrPulses; ++i)
      {
        //--read the puls properties from archive--//
        int32_t wavefromOffset(lmareader::retrieve<int32_t>(file));
        int32_t pulslength(lmareader::retrieve<int32_t>(file));
        size_t dataSize(pulslength * 2);
        file.read(reinterpret_cast<char*>(&(*(waveform.begin()+wavefromOffset))),
                  dataSize);
      }
    }
  }

  return true;
}
