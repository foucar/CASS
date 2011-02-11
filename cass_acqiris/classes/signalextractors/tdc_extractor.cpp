//Copyright (C) 2011 Lutz Foucar

/**
 * @file tdc_extractor.cpp file contains class that extracts the right hits from
 *                         the tdc data
 *
 * @author Lutz Foucar
 */

#include <sstream>

#include "tdc_extractor.h"

#include "cass_event.h"
#include "cass_settings.h"
#include "acqiristdc_device.h"

using namespace std;
using namespace cass;

ACQIRIS::SignalProducer::signals_t& ACQIRISTDC::TDCExtractor::operator()(ACQIRIS::SignalProducer::signals_t& sig)
{
  const ACQIRISTDC::Channel::hits_t &hits(_chan->hits());
  ACQIRISTDC::Channel::hits_t::const_iterator it(hits.begin());
  for (; it != hits.end();++it)
  {
    bool add(false);
    vector<pair<double,double> >::const_iterator tit(_timeranges.begin());
    for (; tit != _timeranges.end();++tit)
    {
      if ((*tit).first < (*it) && (*it) < (*tit).second)
      {
        add = true;
        break;
      }
    }
    if (add)
    {
      ACQIRIS::SignalProducer::signal_t signal;
      signal["isUsed"] = false;
      signal["time"] = *it;
      sig.push_back(signal);
    }
  }
  return sig;
}

void ACQIRISTDC::TDCExtractor::loadSettings(CASSSettings &s)
{
  s.beginGroup("TDCExtraction");
  _instrument   = s.value("TDCInstrument").toUInt();
  _channelNumber= s.value("ChannelNumber",0).toUInt();
  int size = s.beginReadArray("Timeranges");
  for (int i = 0; i < size; ++i)
  {
    s.setArrayIndex(i);
    _timeranges.push_back(std::make_pair(s.value("LowerLimit",0.).toDouble(),
                                           s.value("UpperLimit",1000).toDouble()));
  }
  s.endArray();
  s.endGroup();
}

void ACQIRISTDC::TDCExtractor::associate(const CASSEvent &evt)
{
  const Device &device
      (*(dynamic_cast<const ACQIRISTDC::Device*>(evt.devices().find(CASSEvent::AcqirisTDC)->second)));
  ACQIRISTDC::Device::instruments_t::const_iterator instrumentIt
      (device.instruments().find(_instrument));
  if (instrumentIt == device.instruments().end())
  {
    stringstream ss;
    ss << "TDCExtractor::associate(): The requested Instrument '"<<_instrument
        <<"' is not in the datastream";
    throw invalid_argument(ss.str());
  }
  const ACQIRISTDC::Instrument::channels_t &tdcChannels
      (instrumentIt->second.channels());
  if ((_channelNumber >= tdcChannels.size()))
  {
    stringstream ss;
    ss<< "TDCExtractor::associate(): The requested channel '"<<_channelNumber
        <<"' does not exist in Instrument '"<<_instrument<<"'";
    throw std::invalid_argument(ss.str());
  }
  _chan = &(tdcChannels[_channelNumber]);
}
