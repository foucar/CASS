#include "remi_analysis.h"
#include "remi_event.h"
#include "cass_event.h"
#include "com.h"
#include "cfd.h"
#include "detektorhitsorter_simple.h"

void loadSignalParameter(cass::REMI::Signal& s, const char * groupName, cass::REMI::Parameter* p)
{
  p->beginGroup(groupName);
  s.polarity()    = static_cast<cass::REMI::Peak::Polarity>(p->value("Polarity",cass::REMI::Peak::Negative).toInt());
  s.chanNbr()     = p->value("ChannelNumber",0).toInt();
  s.trLow()       = p->value("LowerTimeRangeLimit",0.).toDouble();
  s.trHigh()      = p->value("UpperTimeRangeLimit",20000.).toDouble();
  p->endGroup();
}

void saveSignalParameter(const cass::REMI::Signal& s, const char * groupName, cass::REMI::Parameter* p)
{
  p->beginGroup(groupName);
  p->setValue("Polarity",static_cast<int>(s.polarity()));
  p->setValue("ChannelNumber",static_cast<int>(s.chanNbr()));
  p->setValue("LowerTimeRangeLimit",s.trLow());
  p->setValue("UpperTimeRangeLimit",s.trHigh());
  p->endGroup();
}

void loadAnodeParameter(cass::REMI::AnodeLayer& a, const char * groupName, cass::REMI::Parameter* p)
{
  p->beginGroup(groupName);
  a.tsLow()   = p->value("LowerTimeSumLimit",0.).toDouble();
  a.tsHigh()  = p->value("UpperTimeSumLimit",20000.).toDouble();
  a.sf()      = p->value("Scalefactor",0.5).toDouble();
  loadSignalParameter(a.one(),"One",p);
  loadSignalParameter(a.two(),"Two",p);
  p->endGroup();
}

void saveAnodeParameter(const cass::REMI::AnodeLayer& a, const char * groupName, cass::REMI::Parameter* p)
{
  p->beginGroup(groupName);
  p->setValue("LowerTimeSumLimit",a.tsLow());
  p->setValue("UpperTimeSumLimit",a.tsHigh());
  p->setValue("Scalefactor",a.sf());
  saveSignalParameter(a.one(),"One",p);
  saveSignalParameter(a.two(),"Two",p);
  p->endGroup();
}

void cass::REMI::Parameter::load()
{
  //string for the container index//
  QString s;
  //sync before loading//
  sync();
  //the channel parameters//
  beginGroup("ChannelContainer");
  //delete the previous channel parameters//
  _channelParameters.clear();
  for (size_t i = 0; i < value("size",4).toUInt();++i)
  {
    beginGroup(s.setNum(static_cast<uint32_t>(i)));
    //add a channel//
    _channelParameters.push_back(ChannelParameter());
    //load the parameters of the channel//
    _channelParameters[i]._threshold    = value("Threshold",50.).toInt();
    _channelParameters[i]._delay        = value("Delay",5).toInt();
    _channelParameters[i]._fraction     = value("Fraction",0.6).toDouble();
    _channelParameters[i]._walk         = value("Walk",0.).toDouble();
    _channelParameters[i]._analyzetyp   =
        static_cast<WaveformAnalyzer::WaveformAnalyzerTypes>(value("Method",WaveformAnalyzer::CoM16Bit).toInt());
    endGroup(); //QString
  }
  endGroup();//channelparameter

  //the detektor parameters//
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
    // add detector to detector vector//

    endGroup(); //QString(i)
  }
  endGroup();//detectorcontainer
}

void cass::REMI::Parameter::save()
{
  //string for the container index//
  QString s;
  //the channel parameters//
  beginGroup("ChannelContainer");
  setValue("size",static_cast<uint32_t>(_channelParameters.size()));
  for (size_t i = 0; i < _channelParameters.size();++i)
  {
    beginGroup(s.setNum(static_cast<uint32_t>(i)));
    setValue("Threshold",_channelParameters[i]._threshold);
    setValue("Delay",_channelParameters[i]._delay);
    setValue("Fraction",_channelParameters[i]._fraction);
    setValue("Walk",_channelParameters[i]._walk);
    setValue("Method",_channelParameters[i]._analyzetyp);

    endGroup(); //QString
  }
  endGroup();//channelparameter

  //the detektor parameters//
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
    endGroup(); //QString(i)
  }
  endGroup();//detectorcontainer
}











cass::REMI::Analysis::Analysis()
{
  //create the map with the waveform Analyzers//
  _waveformanalyzer[WaveformAnalyzer::CFD8Bit]  = new cass::REMI::CFD8Bit();
  _waveformanalyzer[WaveformAnalyzer::CFD16Bit] = new cass::REMI::CFD16Bit();
  _waveformanalyzer[WaveformAnalyzer::CoM8Bit]  = new cass::REMI::CoM8Bit();
  _waveformanalyzer[WaveformAnalyzer::CoM16Bit] = new cass::REMI::CoM16Bit();
  _waveformanalyzer[WaveformAnalyzer::DoNothing]= new cass::REMI::WaveformAnalyzerDoNothing();

  //create the map with the DetectorHitSorter//
  _sorter[DetectorHitSorter::Simple] = new cass::REMI::DetectorHitSorterSimple();

  //load the settings//
  loadSettings();
}


void cass::REMI::Analysis::loadSettings()
{
  //we need to fill the parameters with some life first//
  _parameter.load();
}

void cass::REMI::Analysis::operator()(cass::CASSEvent* cassevent)
{
  //get the remievent from the cassevent//
  cass::REMI::REMIEvent& remievent = cassevent->REMIEvent();

  //ignore event if it is not initialized//
  if (remievent.isFilled() && remievent.isConfigured())
  {
    //clear the arrays that contain the actual data//
    for (size_t i=0;i<remievent.channels().size();++i)
      remievent.channels()[i].peaks().clear();

    //copy the parameters to the event//
    remievent.detectors() = _parameter._detectors;
    //if we didn't allocate enough channelparameters//
    //resize it here//
    if(remievent.channels().size() > _parameter._channelParameters.size())
    {
      //retrieve a reference to the channelparameters//
      cass::REMI::Parameter::chanparameters_t &chanparam = _parameter._channelParameters;
      //remember how many parameters we already have//
      const size_t nbrParametersBefore = chanparam.size();
      //resize the channel parameters to fit all channels//
      chanparam.resize(remievent.channels().size());
      //set the default parameters for this channel//
      for (size_t iNewParam = nbrParametersBefore; iNewParam < chanparam.size();++iNewParam)
      {
        chanparam[iNewParam]._threshold   = 50;
        chanparam[iNewParam]._delay       = 5;
        chanparam[iNewParam]._fraction    = 0.6;
        chanparam[iNewParam]._walk        = 0.;
        chanparam[iNewParam]._analyzetyp  = WaveformAnalyzer::CoM16Bit;
      }
      //save the new settings//
//      _parameter.save();
    }
    //go through all channels//
    for (size_t i=0; i<remievent.channels().size() ;++i)
    {
      remievent.channels()[i].threshold() = _parameter._channelParameters[i]._threshold;
      remievent.channels()[i].delay()     = _parameter._channelParameters[i]._delay;
      remievent.channels()[i].walk()      = _parameter._channelParameters[i]._walk;
      remievent.channels()[i].fraction()  = _parameter._channelParameters[i]._fraction;
      remievent.channels()[i].type()      = _parameter._channelParameters[i]._analyzetyp;
    }


    //find the Signals (peaks) of all waveforms in the channels//
    for (size_t i=0; i<remievent.channels().size();++i)
    {
      cass::REMI::WaveformAnalyzer::WaveformAnalyzerTypes type =
          static_cast<cass::REMI::WaveformAnalyzer::WaveformAnalyzerTypes>(remievent.channels()[i].type());
      _waveformanalyzer[type]->analyze(remievent.channels()[i], remievent.sampleInterval());
    }

    //extract the peaks for the layers//
    //and sort the peaks for detektor hits//
    //fill the results in the Cass Event//
    //this has to be done for each detektor individually//
    for (size_t i=0; i<remievent.detectors().size();++i)
    {
      cass::REMI::DetectorHitSorter::SorterTypes type =
          static_cast<cass::REMI::DetectorHitSorter::SorterTypes>(remievent.detectors()[i].sorterType());
      _sorter[type]->sort(remievent, remievent.detectors()[i]);
    }

    //        for (size_t i=0;i<remievent.nbrOfChannels();++i)
    //        {
    //            std::cout << "channel "<<i<<std::endl;
    //            for (size_t j=0;j<remievent.channel(i).nbrPeaks();++j)
    //            {
    //                std::cout << " Peak "<<j<<std::endl;
    //                std::cout << "   time "<<remievent.channel(i).peak(j).time()<<std::endl;
    //                std::cout << "   integral "<<remievent.channel(i).peak(j).integral()<<std::endl;
    //                std::cout << "   width "<<remievent.channel(i).peak(j).width()<<std::endl;
    //                std::cout << "   fwhm "<<remievent.channel(i).peak(j).fwhm()<<std::endl;
    //                std::cout << "   maximum "<<remievent.channel(i).peak(j).maximum()<<std::endl;
    //                std::cout << "   polarity "<<remievent.channel(i).peak(j).polarity()<<std::endl;
    //            }
    //        }
//    std::cout  << remievent.channels().size()<<std::endl;
  }
}



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
