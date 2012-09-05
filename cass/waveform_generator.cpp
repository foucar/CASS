//Copyright (C) 2012 Lutz Foucar

/**
 * @file waveform_generator.cpp file contains a class for waveform generation
 *
 * @author Lutz Foucar
 */

#include <algorithm>

#include "waveform_generator.h"

#include "cass_event.h"
#include "acqiris_device.h"
#include "cass_settings.h"
#include "channel.h"

using namespace std;
using namespace cass;
using namespace ACQIRIS;


DataGeneratorRegister<WaveformGenerator> WaveformGenerator::reg("Waveform");

WaveformGenerator::WaveformGenerator()
{
}

void WaveformGenerator::load()
{
  CASSSettings s;
  s.beginGroup("WaveformGenerator");
  _instrID = s.value("InstrumentId",8).toInt();
  Instrument::channels_t &channels = _instrument.channels();
  const size_t nSamples = s.value("NbrOfSamples",20000).toUInt();
  const double sampleInterval = s.value("SampleInterval",1.e-9).toDouble();
  const int size = s.beginReadArray("Channel");
  channels.resize(size);
  for (int i = 0; i < size; ++i)
  {
    s.setArrayIndex(i);
    Channel &chan = channels[i];
    chan.channelNbr()     = 99;
    chan.horpos()         = 0;
    chan.offset()         = s.value("Offset",0).toDouble();
    chan.sampleInterval() = sampleInterval;
    chan.gain()           = 0xFFFF / s.value("FullScale",0.5).toDouble();
    chan.waveform().resize(nSamples);
  }

}

void WaveformGenerator::fill(CASSEvent& evt)
{
  /** generate random waveform */
  Instrument::channels_t &channels = _instrument.channels();
  for (Instrument::channels_t::iterator it(channels.begin()); it != channels.end(); ++it)
  {
    Channel &chan = *it;
    waveform_t &waveform (chan.waveform());
    generate(waveform.begin(),waveform.end(),qrand);
  }

  /** retrieve acqiris device */
  Device &dev(*dynamic_cast<Device*>(evt.devices()[CASSEvent::Acqiris]));
  /** retrieve a reference to the requested instrument */
  Instrument & instrument(dev.instruments()[static_cast<Instruments>(_instrID)]);
  /** copy contents of the instrument to the instrument stored in the cassevent */
  instrument = _instrument;
}
