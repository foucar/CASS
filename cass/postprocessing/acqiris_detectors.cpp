//Copyright (C) 2010 Lutz Foucar

/**
 * @file acqiris_detectors.cpp file contains definition of postprocessors that
 *                             extract information of acqiris detectors.
 *
 * @author Lutz Foucar
 */

#include <stdexcept>
#include <cmath>
#include <algorithm>

#include <QtCore/QString>

#include "acqiris_detectors.h"
#include "acqiris_detectors_helper.h"
#include "histogram.h"
#include "cass_event.h"
#include "acqiris_device.h"
#include "cass.h"
#include "convenience_functions.h"
#include "cass_settings.h"


namespace cass
{
  namespace ACQIRIS
  {
    /** load layer from file
     *
     * load the requested layer from .ini file and checks whether it is valid.
     * If it is not valid an invalid_argument exception is thrown
     *
     * @return key containing the layer name
     * @param s CASSSettings object to read the info from
     * @param detector the name of the detector that contains the layer
     * @param layerKey key how the layer value is called in the .ini file
     * @param ppNbr the Postprocessor number of the postprocessor calling this
     *              function
     * @param key the key of the postprocessor calling this function
     *
     * @author Lutz Foucar
     */
    DelaylineDetector::anodelayers_t::key_type loadLayer(CASSSettings &s,
                                                         const HelperAcqirisDetectors::helperinstancesmap_t::key_type &detector,
                                                         const std::string &layerKey,
                                                         int ppNbr,
                                                         const PostProcessors::key_t& key)
    {
      using namespace std;
      HelperAcqirisDetectors *dethelp (HelperAcqirisDetectors::instance(detector));
      DelaylineDetector::anodelayers_t::key_type layer
          (s.value(layerKey.c_str(),"U").toString()[0].toAscii());
      if (layer != 'U' && layer != 'V' && layer != 'W' &&
          layer != 'X' && layer != 'Y')
      {
        stringstream ss;
        ss <<"pp"<<ppNbr<<"::loadSettings()'"<<key<<"': The loaded value of '"<<layerKey<<"' '"<<layer<<"' does not exist. Can only be 'U', 'V', 'W', 'X' or 'Y'";
        throw invalid_argument(ss.str());
      }
      else if (dynamic_cast<const DelaylineDetector*>(dethelp->detector())->isHex())
      {
        if (layer == 'X' || layer == 'Y')
        {
          stringstream ss;
          ss <<"pp"<<ppNbr<<"::loadSettings()'"<<key<<"': Detector '"<<detector<<"' is Hex-detector and cannot have Layer '"<<layer<<"'";
          throw invalid_argument(ss.str());
        }
      }
      else
      {
        if (layer == 'U' || layer == 'V' || layer == 'W')
        {
          stringstream ss;
          ss <<"pp"<<ppNbr<<"::loadSettings()'"<<key<<"': Detector '"<<detector<<"' is Quad-detector and cannot have Layer '"<<layer<<"'";
          throw invalid_argument(ss.str());
        }
      }
      return layer;
    }

    /** load wireend from file
     *
     * load the requested wireend from .ini file. Check whether it is a valid
     * wireend otherwise throw invalid_argument exception.
     *
     * @return key containing the wireend name
     * @param s CASSSettings object to read the info from
     * @param wireendKey key how the wireend value is called in the .ini file
     * @param ppNbr the Postprocessor number of the postprocessor calling this
     *              function
     * @param key the key of the postprocessor calling this function
     *
     * @author Lutz Foucar
     */
    AnodeLayer::wireends_t::key_type loadWireend(CASSSettings &s,
                                                 const std::string & wireendKey,
                                                 int ppNbr,
                                                 const PostProcessors::key_t& key)
    {
      using namespace std;
      AnodeLayer::wireends_t::key_type wireend
          (s.value(wireendKey.c_str(),"1").toString()[0].toAscii());
      if (wireend != '1' && wireend != '2')
      {
        stringstream ss;
        ss <<"pp"<<ppNbr<<"::loadSettings()'"<<key<<"': The loaded value of '"<<wireendKey<<"' '"<<wireend<<"' does not exist. Can only be '1' or '2'";
        throw invalid_argument(ss.str());
      }
      return wireend;
    }
  }
}

//----------------Nbr of Peaks MCP---------------------------------------------
cass::pp150::pp150(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)
{
  loadSettings(0);
}

void cass::pp150::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector = settings.value("Detector","blubb").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);
  HelperAcqirisDetectors::instance(_detector)->loadSettings();
  cout <<endl<< "PostProcessor '"<<_key
      <<"' retrieves the nbr of mcp signals of detector '"<<_detector
      <<"'. Condition is '"<<_condition->key()<<"'"
      <<endl;
}

void cass::pp150::process(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  TofDetector *det
      (dynamic_cast<TofDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt)));
  _result->lock.lockForWrite();
  dynamic_cast<Histogram0DFloat*>(_result)->fill(det->mcp().output().size());
  _result->lock.unlock();
}










//----------------MCP Hits (Tof)-----------------------------------------------
cass::pp151::pp151(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)
{
  loadSettings(0);
}

void cass::pp151::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector = settings.value("Detector","blubb").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  set1DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  HelperAcqirisDetectors::instance(_detector)->loadSettings();
  std::cout <<std::endl<< "PostProcessor "<<_key
      <<": it histograms times of the found mcp signals"
      <<" of detector "<<_detector
      <<". Condition is"<<_condition->key()
      <<std::endl;
}

void cass::pp151::process(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  TofDetector *det
      (dynamic_cast<TofDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt)));
//  cout << " pp151: 1"<<endl;
  SignalProducer::signals_t::const_iterator it (det->mcp().output().begin());
//  cout << " pp151: 2"<<endl;
  _result->clear();
  _result->lock.lockForWrite();
  for (; it != det->mcp().output().end(); ++it)
    dynamic_cast<Histogram1DFloat*>(_result)->fill((*it)["time"]);
  _result->lock.unlock();
//  cout << " pp151: 3"<<endl;
}










//----------------MCP Fwhm vs. height------------------------------------------
cass::pp152::pp152(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)
{
  loadSettings(0);
}

void cass::pp152::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector = settings.value("Detector","blubb").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  set2DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  HelperAcqirisDetectors::instance(_detector)->loadSettings();
  std::cout <<std::endl<< "PostProcessor "<<_key
      <<": histograms the FWHM vs the height of the found mcp signals"
      <<" of detector "<<_detector
      <<". Condition is"<<_condition->key()
      <<std::endl;
}

void cass::pp152::process(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  TofDetector *det
      (dynamic_cast<TofDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt)));
  SignalProducer::signals_t::const_iterator it (det->mcp().output().begin());
  _result->clear();
  _result->lock.lockForWrite();
  for (;it != det->mcp().output().end(); ++it)
    dynamic_cast<Histogram2DFloat*>(_result)->fill((*it)["fwhm"],(*it)["height"]);
  _result->lock.unlock();
}












//----------------Nbr of Peaks Anode-------------------------------------------
cass::pp160::pp160(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)
{
  loadSettings(0);
}

void cass::pp160::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector = loadDelayDet(settings,160,_key);
  _layer = loadLayer(settings,_detector,"Layer",160,_key);
  _signal = loadWireend(settings,"Wireend",160,_key);
  setupGeneral();
  if (!setupCondition())
    return;
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);
  cout <<endl<< "PostProcessor '"<<_key
      <<"' outputs the nbr of signals of layer '"<<_layer
      <<"' wireend '"<<_signal
      <<"' of detector '"<<_detector
      <<"'. Condition is '"<<_condition->key()<<"'"
      <<endl;
}

void cass::pp160::process(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  DelaylineDetector *det
      (dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt)));
  _result->lock.lockForWrite();
  dynamic_cast<Histogram0DFloat*>(_result)->fill(det->layers()[_layer].wireends()[_signal].output().size());
  _result->lock.unlock();
}











//----------------FWHM vs. Height of Wireend Signals---------------------------
cass::pp161::pp161(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)
{
  loadSettings(0);
}

void cass::pp161::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector = loadDelayDet(settings,161,_key);
  _layer = loadLayer(settings,_detector,"Layer",161,_key);
  _signal = loadWireend(settings,"Wireend",161,_key);
  setupGeneral();
  if (!setupCondition())
    return;
  set2DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  cout <<endl<< "PostProcessor '"<<_key
      <<"' histograms the FWHM vs the height from the signals of layer '"<<_layer
      <<"' wireend '"<<_signal
      <<"' of detector '"<<_detector
      <<"'. Condition is '"<<_condition->key()<<"'"
      <<endl;
}

void cass::pp161::process(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  DelaylineDetector *det
      (dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt)));
  SignalProducer::signals_t::const_iterator it (det->layers()[_layer].wireends()[_signal].output().begin());
  _result->clear();
  _result->lock.lockForWrite();
  for (; it != det->layers()[_layer].wireends()[_signal].output().end(); ++it)
    dynamic_cast<Histogram2DFloat*>(_result)->fill((*it)["fwhm"],(*it)["height"]);
  _result->lock.unlock();
}










//----------------Timesum for the layers---------------------------------------
cass::pp162::pp162(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)
{
  loadSettings(0);
}

void cass::pp162::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector = loadDelayDet(settings,162,_key);
  _layer = loadLayer(settings,_detector,"Layer",162,_key);
  _range = make_pair(settings.value("TimeRangeLow",0).toDouble(),
                     settings.value("TimeRangeHigh",20000).toDouble());
  setupGeneral();
  if (!setupCondition())
    return;
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);
  cout <<endl<< "PostProcessor '"<<_key
      <<"' calculates the timesum of layer '"<<_layer
      <<"' of detector '"<<_detector
      <<"'. It will use the first signals that appeared in the ToF range from '"<<_range.first
      <<"' ns to '"<<_range.second
      <<"' ns. Condition is '"<<_condition->key()<<"'"
      <<endl;
}

void cass::pp162::process(const cass::CASSEvent &evt)
{
  using namespace std;
  using namespace cass::ACQIRIS;
//  static int counter;
//  cout <<counter++<< " pp162"<<endl;
  DetectorBackend *rawdet
      (HelperAcqirisDetectors::instance(_detector)->detector(evt));
  DelaylineDetector *det (dynamic_cast<DelaylineDetector*>(rawdet));
//  cout << "pp162 1"<<endl;
  const double one (det->layers()[_layer].wireends()['1'].firstGood(_range));
//  cout << "pp162 6"<<endl;
  const double two (det->layers()[_layer].wireends()['2'].firstGood(_range));
//  cout << "pp162 7"<<endl;
  const double mcp (det->mcp().firstGood(_range));
//  cout << "pp162 mcp '"<<mcp<<"' one '"<<one<<"' two '"<<two<<"'"<<endl;
  _result->lock.lockForWrite();
  dynamic_cast<Histogram0DFloat*>(_result)->fill( one + two - 2.*mcp);
  _result->lock.unlock();
}










//----------------Timesum vs Postition for the layers--------------------------
cass::pp163::pp163(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)
{
  loadSettings(0);
}

void cass::pp163::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector = loadDelayDet(settings,163,_key);
  _layer = loadLayer(settings,_detector,"Layer",163,_key);
  _range = make_pair(settings.value("TimeRangeLow",0).toDouble(),
                     settings.value("TimeRangeHigh",20000).toDouble());
  setupGeneral();
  if (!setupCondition())
    return;
  set2DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  cout<<endl<< "PostProcessor '"<<_key
      <<"' histograms the timesum vs Postion on layer '"<<_layer
      <<"' of detector '"<<_detector
      <<"'. Condition is '"<<_condition->key()<<"'"
      <<endl;
}

void cass::pp163::process(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  DelaylineDetector *det
      (dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt)));
  const double one (det->layers()[_layer].wireends()['1'].firstGood(_range));
  const double two (det->layers()[_layer].wireends()['2'].firstGood(_range));
  const double mcp (det->mcp().firstGood(_range));
  const double timesum (one + two - 2.*mcp);
  const double position (one - two);
  _result->clear();
  _result->lock.lockForWrite();
  dynamic_cast<Histogram2DFloat*>(_result)->fill(position,timesum);
  _result->lock.unlock();
}











//----------------Detector First Hit-------------------------------------------
cass::pp164::pp164(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)
{
  loadSettings(0);
}

void cass::pp164::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector = loadDelayDet(settings,164,_key);
  _first = loadLayer(settings,_detector,"FirstLayer",164,_key);
  _second = loadLayer(settings,_detector,"SecondLayer",164,_key);
  _range = make_pair(settings.value("TimeRangeLow",0).toDouble(),
                     settings.value("TimeRangeHigh",20000).toDouble());
  _tsrange = make_pair(make_pair(settings.value("TimesumFirstLayerLow",20).toDouble(),
                                 settings.value("TimesumFirstLayerHigh",200).toDouble()),
                       make_pair(settings.value("TimesumSecondLayerLow",20).toDouble(),
                                 settings.value("TimesumSecondLayerHigh",200).toDouble()));
  setupGeneral();
  if (!setupCondition())
    return;
  set2DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  cout <<endl<< "PostProcessor '"<<_key
      <<"' creates a detector picture of the first Hit on the detector created"
      <<" from  Layers '"<<_first
      <<"' and '"<<_second
      <<"' of detector '"<<_detector
      <<"'. The signals from wich the frist hit is calculated have to be in the"
      <<" range from '"<<_range.first
      <<"' ns to '"<<_range.second
      <<"' ns. The Timesum range of the first layer goes from '"<<_tsrange.first.first
      <<"' to '"<<_tsrange.first.second
      <<"'. The Timesum range of the second layer goes from '"<<_tsrange.second.first
      <<"' to '"<<_tsrange.second.second
      <<"'. Condition is '"<<_condition->key()<<"'"
      <<std::endl;
}

void cass::pp164::process(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  DelaylineDetector *det
      (dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt)));
  const double f1 (det->layers()[_first].wireends()['1'].firstGood(_range));
  const double f2 (det->layers()[_first].wireends()['2'].firstGood(_range));
  const double s1 (det->layers()[_second].wireends()['1'].firstGood(_range));
  const double s2 (det->layers()[_second].wireends()['2'].firstGood(_range));
  const double mcp (det->mcp().firstGood(_range));
  const double tsf (f1 + f2 - 2.*mcp);
  const double tss (s1 + s2 - 2.*mcp);
  const double f (f1-f2);
  const double s (s1-s2);
  const bool csf = (_tsrange.first.first < tsf && tsf < _tsrange.first.second);
  const bool css = (_tsrange.second.first < tss && tss < _tsrange.second.second);
  _result->clear();
  _result->lock.lockForWrite();
  if (csf && css)
    dynamic_cast<Histogram2DFloat*>(_result)->fill(f,s);
  _result->lock.unlock();
}


















//----------------Nbr of rec. Hits --------------------------------------------
cass::pp165::pp165(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)
{
  loadSettings(0);
}

void cass::pp165::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector = loadDelayDet(settings,165,_key);
  setupGeneral();
  if (!setupCondition())
    return;
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);
  cout<<endl<< "PostProcessor '"<<_key
      <<"' outputs the number of reconstructed hits of detector '"<<_detector
      <<"'. Condition is '"<<_condition->key()<<"'"
      <<endl;
}

void cass::pp165::process(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  using namespace std;
//  static int counter;
//  cout << counter++ <<" pp165 1"<<endl;
  DelaylineDetector *det
      (dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt)));
//  cout << "pp165 2"<<endl;
  _result->lock.lockForWrite();
//  cout << "pp165 3"<<endl;
  dynamic_cast<Histogram0DFloat*>(_result)->fill(det->hits().size());
//  cout << "pp165 4"<<endl;
  _result->lock.unlock();
//  cout << "pp165 5"<<endl;
}
















//----------------Detector Values----------------------------------------------
cass::pp166::pp166(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)

{
  loadSettings(0);
}

void cass::pp166::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector = loadDelayDet(settings,166,_key);
  _first = settings.value("XInput",'x').toString().toStdString();
  _second = settings.value("YInput",'y').toString().toStdString();
  _third =  settings.value("ConditionInput",'t').toString().toStdString();
  _cond = make_pair(min(settings.value("ConditionLow",-50000.).toFloat(),
                        settings.value("ConditionHigh",50000.).toFloat()),
                    max(settings.value("ConditionLow",-50000.).toFloat(),
                        settings.value("ConditionHigh",50000.).toFloat()));
  setupGeneral();
  if (!setupCondition())
    return;
  set2DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  cout<<endl<< "PostProcessor '"<<_key
      <<"' histograms the Property '"<<_second
      <<"' vs. '"<<_first
      <<"' of the reconstructed detectorhits of detector '"<<_detector
      <<"'. It puts a condition from '"<<_cond.first
      <<"' to '"<<_cond.second
      <<"' on Property '"<< _third
      <<"'. Condition is '"<<_condition->key()<<"'"
      <<endl;
}

void cass::pp166::process(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  DelaylineDetector *det
      (dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt)));
  detectorHits_t::iterator it (det->hits().begin());
  _result->clear();
  _result->lock.lockForWrite();
  for (; it != det->hits().end(); ++it)
  {
    if (_cond.first < (*it)[_third] && (*it)[_third] < _cond.second)
      dynamic_cast<Histogram2DFloat*>(_result)->fill((*it)[_first],(*it)[_second]);
  }
  _result->lock.unlock();
}









//----------------PIPICO-------------------------------------------------------
cass::pp220::pp220(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)
{
  loadSettings(0);
}

void cass::pp220::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector01 = settings.value("FirstDetector","blubb").toString().toStdString();
  _detector02 = settings.value("SecondDetector","blubb").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  set2DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  HelperAcqirisDetectors::instance(_detector01)->loadSettings();
  HelperAcqirisDetectors::instance(_detector02)->loadSettings();
  cout<<endl<< "PostProcessor '"<<_key
      <<"' create a PIPICO Histogram of detectors '"<<_detector01
      <<"' and '"<<_detector02
      <<"'. Condition is "<<_condition->key()<<"'"
      <<endl;
}

void cass::pp220::process(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  TofDetector *det01
      (dynamic_cast<TofDetector*>(HelperAcqirisDetectors::instance(_detector01)->detector(evt)));
  TofDetector *det02
      (dynamic_cast<TofDetector*>(HelperAcqirisDetectors::instance(_detector02)->detector(evt)));
  SignalProducer::signals_t::const_iterator it01(det01->mcp().output().begin());
  _result->clear();
  _result->lock.lockForWrite();
  for (; it01 != det01->mcp().output().end();++it01)
  {
    //if both detectors are the same, then the second iterator should start
    //i+1, otherwise we will just draw all hits vs. all hits
    SignalProducer::signals_t::const_iterator it02((_detector01==_detector02) ?
                                                   it01+1 :
                                                   det02->mcp().output().begin());
    for (; it02 != det02->mcp().output().end(); ++it02)
      dynamic_cast<Histogram2DFloat*>(_result)->fill((*it01)["time"],(*it02)["time"]);
  }
  _result->lock.unlock();
}





//----------------Particle Value----------------------------------------------
cass::pp250::pp250(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)

{
  loadSettings(0);
}

void cass::pp250::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector = loadDelayDet(settings,250,_key);
  _particle = settings.value("Particle","NeP").toString().toStdString();
  _property = settings.value("Property","px").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  set1DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  cout<<endl<< "PostProcessor '"<<_key
      <<"' histograms the Property '"<<_property
      <<"' of the particle '"<<_particle
      <<"' of detector '"<<_detector
      <<"'. Condition is '"<<_condition->key()<<"'"
      <<endl;
}

void cass::pp250::process(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  DelaylineDetector *det
      (dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt)));
  Particle & particle (det->particles()[_particle]);
//  cout << "pp250 size"<<particle.hits().size()<<endl;
  particleHits_t::iterator it (particle.hits().begin());
  _result->clear();
  _result->lock.lockForWrite();
  for (; it != particle.hits().end(); ++it)
  {
    dynamic_cast<Histogram1DFloat*>(_result)->fill((*it)[_property]);
  }
  _result->lock.unlock();
}






//----------------Particle Values----------------------------------------------
cass::pp251::pp251(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)

{
  loadSettings(0);
}

void cass::pp251::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector = loadDelayDet(settings,251,_key);
  _particle = settings.value("Particle","NeP").toString().toStdString();
  _property01 = settings.value("FirstProperty","px").toString().toStdString();
  _property02 = settings.value("SecondProperty","py").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  set2DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  cout<<endl<< "PostProcessor '"<<_key
      <<"' histograms the Property '"<<_property02
      <<"' vs. '"<<_property01
      <<"' of the particle '"<<_particle
      <<"' of detector '"<<_detector
      <<"'. Condition is '"<<_condition->key()<<"'"
      <<endl;
}

void cass::pp251::process(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  DelaylineDetector *det
      (dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt)));
  Particle & particle (det->particles()[_particle]);
  particleHits_t::iterator it (particle.hits().begin());
  _result->clear();
  _result->lock.lockForWrite();
  for (; it != particle.hits().end(); ++it)
  {
    dynamic_cast<Histogram2DFloat*>(_result)->fill((*it)[_property01],(*it)[_property02]);
  }
  _result->lock.unlock();
}




//----------------Number of Particles---------------------------------------------
cass::pp252::pp252(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)

{
  loadSettings(0);
}

void cass::pp252::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector = loadDelayDet(settings,252,_key);
  _particle = settings.value("Particle","NeP").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);
  cout<<endl<< "PostProcessor '"<<_key
      <<"' outputs how many particles were found for '"<<_particle
      <<"' of detector '"<<_detector
      <<"'. Condition is '"<<_condition->key()<<"'"
      <<endl;
}

void cass::pp252::process(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  DelaylineDetector *det
      (dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt)));
  Particle & particle (det->particles()[_particle]);
  _result->lock.lockForWrite();
  dynamic_cast<Histogram0DFloat*>(_result)->fill(particle.hits().size());
  _result->lock.unlock();
}

