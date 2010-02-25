// Copyright (C) 2010 lmf

#include "acqiris_analysis.h"
#include "acqiris_device.h"
#include "cass_event.h"
#include "com.h"
#include "cfd.h"
#include "delayline_detector_analyzer_simple.h"

namespace cass
{
  namespace ACQIRIS
  {
    void loadSignalParameter(Signal& s, const char * groupName, Parameter* p)
    {
      p->beginGroup(groupName);
      s.channelNbr()    = p->value("ChannelNumber",0).toInt();
      s.trLow()         = p->value("LowerTimeRangeLimit",0.).toDouble();
      s.trHigh()        = p->value("UpperTimeRangeLimit",20000.).toDouble();
      s.polarity()      = static_cast<Polarity>(p->value("Polarity",Negative).toInt());
      s.threshold()     = p->value("Threshold",50.).toInt();
      s.delay()         = p->value("Delay",5).toInt();
      s.fraction()      = p->value("Fraction",0.6).toDouble();
      s.walk()          = p->value("Walk",0.).toDouble();
      s.analyzerType()  = static_cast<WaveformAnalyzers>(p->value("WaveformAnalysisMethod",CoM16Bit).toInt());
      p->endGroup();
    }

    void saveSignalParameter(const Signal& s, const char * groupName, Parameter* p)
    {
      p->beginGroup(groupName);
      p->setValue("ChannelNumber",static_cast<int>(s.chanNbr()));
      p->setValue("LowerTimeRangeLimit",s.trLow());
      p->setValue("UpperTimeRangeLimit",s.trHigh());
      p->setValue("Polarity",static_cast<int>(s.polarity()));
      p->setValue("Threshold",s.threshold());
      p->setValue("Delay",s.delay());
      p->setValue("Fraction",s.fraction());
      p->setValue("Walk",s.walk());
      p->setValue("WaveformAnalysisMethod",static_cast<int>(s.analyzerType()));
      p->endGroup();
    }

    void loadAnodeParameter(AnodeLayer& a, const char * groupName, const Parameter* p)
    {
      p->beginGroup(groupName);
      a.tsLow()   = p->value("LowerTimeSumLimit",0.).toDouble();
      a.tsHigh()  = p->value("UpperTimeSumLimit",20000.).toDouble();
      a.sf()      = p->value("Scalefactor",0.5).toDouble();
      loadSignalParameter(a.one(),"One",p);
      loadSignalParameter(a.two(),"Two",p);
      p->endGroup();
    }

    void saveAnodeParameter(const AnodeLayer& a, const char * groupName, Parameter* p)
    {
      p->beginGroup(groupName);
      p->setValue("LowerTimeSumLimit",a.tsLow());
      p->setValue("UpperTimeSumLimit",a.tsHigh());
      p->setValue("Scalefactor",a.sf());
      saveSignalParameter(a.one(),"One",p);
      saveSignalParameter(a.two(),"Two",p);
      p->endGroup();
    }
  }
}




void cass::ACQIRIS::Parameter::load()
{
  //string for the container index//
  QString s;
  //sync before loading//
  sync();

  //the detector parameters//
  beginGroup("DetectorContainer");
  //delete the previous detector parameters//
  _detectors.clear();
  for (size_t i = 0; i < value("size",1).toUInt();++i)
  {
    beginGroup(s.setNum(static_cast<uint32_t>(i)));
    //create a new detector//
    _detectors.push_back(cass::REMI::Detector());
    //load the parameters of the detector//
    _detectors[i].runtime()       = value("Runtime",150).toDouble();
    _detectors[i].wLayerOffset()  = value("WLayerOffset",0.).toDouble();
    _detectors[i].mcpRadius()     = value("McpRadius",66.).toDouble();
    _detectors[i].deadTimeMCP()   = value("DeadTimeMcp",10.).toDouble();
    _detectors[i].deadTimeAnode() = value("DeadTimeAnode",10.).toDouble();
    _detectors[i].sorterType()    = value("SortingMethod",DetectorHitSorter::Simple).toInt();
    _detectors[i].LayersToUse()   = value("LayersToUse",DetectorHitSorterSimple::UV).toInt();
    _detectors[i].isHexAnode()    = value("isHex",true).toBool();
    _detectors[i].name()          = value("Name","IonDetector").toString().toStdString();
    loadSignalParameter(_detectors[i].mcp(),"McpSignal",this);
    loadAnodeParameter(_detectors[i].u(),"ULayer",this);
    loadAnodeParameter(_detectors[i].v(),"VLayer",this);
    loadAnodeParameter(_detectors[i].w(),"WLayer",this);
    endGroup(); //QString(i)
  }
  endGroup();//detectorcontainer
}

void cass::ACQIRIS::Parameter::save()
{
  //string for the container index//
  QString s;
  //the detector parameters//
  beginGroup("DetectorContainer");
  setValue("size",static_cast<uint32_t>(_detectors.size()));
  for (size_t i = 0; i < _detectors.size();++i)
  {
    beginGroup(s.setNum(static_cast<uint32_t>(i)));
    setValue("Runtime",_detectors[i].runtime());
    setValue("WLayerOffset",_detectors[i].wLayerOffset());
    setValue("McpRadius",_detectors[i].mcpRadius());
    setValue("DeadTimeMcp",_detectors[i].deadTimeMCP());
    setValue("DeadTimeAnode",_detectors[i].deadTimeAnode());
    setValue("SortingMethod",_detectors[i].sorterType());
    setValue("LayersToUse",_detectors[i].LayersToUse());
    setValue("isHex",_detectors[i].isHexAnode());
    setValue("Name",_detectors[i].name().c_str());
    saveSignalParameter(_detectors[i].mcp(),"McpSignal",this);
    saveAnodeParameter(_detectors[i].u(),"ULayer",this);
    saveAnodeParameter(_detectors[i].v(),"VLayer",this);
    saveAnodeParameter(_detectors[i].w(),"WLayer",this);
    endGroup();
  }
  endGroup();//detectorcontainer
}











cass::ACQIRIS::Analysis::Analysis()
{
  //create the map with the waveform analyzers//
  _waveformanalyzer[CFD8Bit]  = new CFD8Bit();
  _waveformanalyzer[CFD16Bit] = new CFD16Bit();
  _waveformanalyzer[CoM8Bit]  = new CoM8Bit();
  _waveformanalyzer[CoM16Bit] = new CoM16Bit();

  //create the map with the detector analyzers//
  _detectoranalyzer[DelaylineDetectorSimple] = new DetectorHitSorterSimple(&_waveformanalyzer);

  //load the settings//
  loadSettings();
}


void cass::ACQIRIS::Analysis::operator()(cass::CASSEvent* cassevent)
{
  //get the remievent from the cassevent//
  AcqirisDevice* dev = cassevent->REMIEvent();

  //ignore event if it is not initialized//
  if (!dev->channels().emtpy())
  {
    //copy the parameters to the event//
    dev->detectors() = _parameter._detectors;

    //analyze the detectors//
    //this has to be done for each detektor individually//
    for (size_t i=0; i<dev->detectors().size();++i)
    {
      //retrieve reference to the detector//
      DetectorBackend &det = dev->detectors()[i];
      cass::REMI::DetectorHitSorter::SorterTypes type =
          static_cast<cass::REMI::DetectorHitSorter::SorterTypes>(remievent.detectors()[i].sorterType());
      _detectoranalyzer[det.analyzerType()]->analyze(det, dev->channels());
    }
  }
}



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
