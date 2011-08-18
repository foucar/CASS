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

using namespace cass;
using namespace cass::ACQIRIS;
using namespace std;

namespace cass
{
namespace lmaFile
{
#pragma pack(2)
/** the general file header
 *
 * @author Lutz Foucar
 */
struct GeneralHeader
{
  /** size of the Header in bytes */
  int32_t headersize;

  /** nbr of channels in the instrument */
  int16_t nbrChannels;

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

  /** bitmask describing which channels are recorded */
  int32_t usedChannelBitmask;

  /** bitmask describing which channels are combined */
  int32_t channelCombinationBitmask;

  /** how many converters are being used per channel */
  int16_t nbrConvertersPerChan;
};

/** the header of an recorded channel
 *
 * @author Lutz Foucar
 */
struct ChannelHeader
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
};
} //end namespace lmafile
#pragma pack()
} //end namespace cass

LmaReader::LmaReader()
  :_newFile(true)
{}

void LmaReader::loadSettings()
{
}

bool LmaReader::operator ()(ifstream &file, CASSEvent& evt)
{
  /** if it is a new file read the file header first */
  if (_newFile)
  {
    _newFile = false;
    vector<char> headerarray(sizeof(lmaFile::GeneralHeader));
    file.read(&headerarray.front(),sizeof(lmaFile::GeneralHeader));
    const lmaFile::GeneralHeader &header
        (*reinterpret_cast<lmaFile::GeneralHeader*>(&headerarray.front()));
    //    size_t headersizebytes(FileStreaming::retrieve<int32_t>(file));
    //    int16_t nbrChannels(FileStreaming::retrieve<int16_t>(file));
    //    int16_t nbrBits(FileStreaming::retrieve<int16_t>(file));
    //    double sampInter(FileStreaming::retrieve<double>(file));
    //    int32_t nbrSamples(FileStreaming::retrieve<int32_t>(file));
    //    double delayTime(FileStreaming::retrieve<double>(file));
    //    int16_t triggerChannel(FileStreaming::retrieve<int16_t>(file));
    //    double triggerLevel(FileStreaming::retrieve<double>(file));
    //    int16_t triggerSlope(FileStreaming::retrieve<int16_t>(file));
    //    _usedChannelBitmask = static_cast<uint32_t>(FileStreaming::retrieve<int32_t>(file));
    //    int32_t channelCombinationBitmask(FileStreaming::retrieve<int32_t>(file));
    //    int16_t nbrConvertersPerChan(FileStreaming::retrieve<int16_t>(file));

    if (header.nbrBits != 16)
    {
      stringstream ss;
      ss<<"LmaReader(): The lma file seems to contain 8-bit wavefroms '"<<header.nbrBits
       <<"'. Currently this is not supported.";
      throw runtime_error(ss.str());
    }
    _instrument.channels().resize(header.nbrChannels);
    _usedChannelBitmask = header.usedChannelBitmask;

    for (int16_t i(0) ; i < header.nbrChannels ;++i)
    {
      Channel &chan(_instrument.channels()[i]);
      chan.sampleInterval() = header.samplingInterval;
      chan.waveform().resize(header.nbrSamples);
      chan.channelNbr() = i;

      if (_usedChannelBitmask & (0x1<<i))
      {
        vector<char> chanheaderarray(sizeof(lmaFile::ChannelHeader));
        file.read(&chanheaderarray.front(),sizeof(lmaFile::ChannelHeader));
        const lmaFile::ChannelHeader &chanheader
            (*reinterpret_cast<lmaFile::ChannelHeader*>(&chanheaderarray.front()));

        //        int16_t fullscale_mV(FileStreaming::retrieve<int16_t>(file));
        //        int16_t offset_mV(FileStreaming::retrieve<int16_t>(file));
        //        //the vertical Gain (conversion factor to convert the bits to mV)
        //        double gain_mVperLSB(FileStreaming::retrieve<double>(file));
        //        int16_t baseline(FileStreaming::retrieve<int16_t>(file));
        //        int16_t noiseLevel(FileStreaming::retrieve<int16_t>(file));
        //        int32_t stepsize(FileStreaming::retrieve<int32_t>(file));
        //        int32_t backsize(FileStreaming::retrieve<int32_t>(file));

        chan.offset() = chanheader.offset_mV*1e-3;
        chan.gain() = chanheader.gain_mVperLSB*1e-3;
        waveform_t & waveform (chan.waveform());
        fill(waveform.begin(),waveform.end(),chanheader.offset_mV/chanheader.gain_mVperLSB);
      }
    }
  }

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

  return true;
}
