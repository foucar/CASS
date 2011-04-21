// Copyright (C) 2010 Lutz Foucar
// Copyright (C) 2010 Jochen Küpper

#include <algorithm>
#include <cassert>
#include <functional>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <cmath>

#include <QtCore/QString>

#include "ccd.h"
#include "cass.h"
#include "ccd_device.h"
#include "pnccd_device.h"
#include "histogram.h"
#include "cass_event.h"
#include "postprocessor.h"
#include "acqiris_detectors_helper.h"
#include "convenience_functions.h"
#include "cass_settings.h"
#include "pixel_detector_container.h"

// *** postprocessor 100 -- single images from a CCD ***

cass::pp100::pp100(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  :PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp100::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _device = static_cast<CASSEvent::Device>(settings.value("Device",0).toUInt());
  _detector = settings.value("Detector",0).toUInt();
  int cols(0); int rows(0);
  switch(_device)
  {
  case CASSEvent::CCD:
    cols = CCD::opal_default_size; rows = CCD::opal_default_size;
    break;
  case CASSEvent::pnCCD:
    cols = pnCCD::default_size; rows = pnCCD::default_size;
    break;
  default:
    throw std::runtime_error(QString("%1: Device %2 is not an ccd containing device")
                             .arg(_key.c_str())
                             .arg(_device).toStdString());
    break;
  }
  setupGeneral();
  if (!setupCondition())
    return;
  _result = new Histogram2DFloat(cols,rows);
  createHistList(2*cass::NbrOfWorkers);
  cout<<endl<<"Postprocessor '"<<_key
      <<"' will display ccd image of detector '"<<_detector
      <<"' in device '"<<_device
      <<"'. The image has '"<<rows
      <<"' rows and '"<<cols
      <<"' columns. It will use condition '"<<_condition->key()<<"'"
      <<endl;
}

void cass::pp100::process(const cass::CASSEvent& evt)
{
  using namespace std;
  //check whether detector exists
  if (evt.devices().find(_device)->second->detectors()->size() <= _detector)
    throw std::runtime_error(QString("PostProcessor_%1: Detector %2 does not exist in Device %3")
                             .arg(_key.c_str())
                             .arg(_detector)
                             .arg(_device).toStdString());

  //get frame and fill image//
  const PixelDetector &det ((*(evt.devices().find(_device)->second)->detectors())[_detector]);
  const PixelDetector::frame_t& frame (det.frame());
//      ((*(evt.devices().find(_device)->second)->detectors())[_detector].frame());
//  std::cout << frame.size()<<" "<<dynamic_cast<Histogram2DFloat*>(_result)->memory().size()<<std::endl;
  /*
      // the following block is reasonable, if the frames are already rebinned within the Analysis::operator
      if(frame.size()!=_image->shape().first *_image->shape().second)
      {
        size_t cols = _image->shape().first;
        size_t rows = _image->shape().second;
        size_t ratio= cols * rows /frame.size();
        size_t side_ratio = static_cast<size_t>(sqrt( static_cast<double>(ratio) ));
        //std::cout<<"ratio of sizes, ratio of axis are: "<< ratio << " , "<< side_ratio <<std::endl;
        _image = new Histogram2DFloat(cols/side_ratio, 0, cols-1, rows/side_ratio, 0, rows-1);
      }

      const PixelDetector &det((*event.devices().find(_device)->second->detectors())[_detector]);

      const cass::ROI::ROIiterator_t& ROIiterator_pp(det.ROIiterator_pp());
      std::cout<< "cacca " << ROIiterator_pp.size()
        <<std::endl;
  */
  _result->lock.lockForWrite();
  if (_result->axis()[HistogramBackend::xAxis].nbrBins() != det.columns() ||
      _result->axis()[HistogramBackend::yAxis].nbrBins() != det.rows())
  {
    for (histogramList_t::iterator it(_histList.begin()); it != _histList.end(); ++it)
    {
      dynamic_cast<Histogram2DFloat*>(it->second)->resize(det.columns(),0,det.columns()-1,
                                                          det.rows(),0,det.rows()-1);
    }
    PostProcessors::keyList_t dependands (_pp.find_dependant(_key));
    PostProcessors::keyList_t::iterator it (dependands.begin());
    for (; it != dependands.end(); ++it)
      _pp.getPostProcessor(*it).histogramsChanged(_result);
  }
  copy(frame.begin(),
       frame.end(),
       dynamic_cast<Histogram2DFloat*>(_result)->memory().begin());
  _result->nbrOfFills() = 1;
  _result->lock.unlock();
}







// *** postprocessor 101:  Integral over the Image ***

cass::pp101::pp101(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)
{
  loadSettings(0);
}

void cass::pp101::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _device = static_cast<CASSEvent::Device>(settings.value("Device",0).toUInt());
  _detector = settings.value("Detector",0).toUInt();
  _result = new Histogram0DFloat();
  setupGeneral();
  if (!setupCondition())
    return;
  createHistList(2*cass::NbrOfWorkers);
  std::cout <<std::endl<< "PostProcessor "<<_key
      <<": retrieves the Integral over the whole "<<_detector
      <<" detector"
      <<". Condition is"<<_condition->key()
      <<std::endl;
}

void cass::pp101::process(const cass::CASSEvent &evt)
{
  using namespace std;
  if (evt.devices().find(_device)->second->detectors()->size() <= _detector)
    throw std::runtime_error(QString("PostProcessor_%1: Detector %2 does not exist in Device %3")
                             .arg(_key.c_str())
                             .arg(_detector)
                             .arg(_device).toStdString());
  const float& integral
      (static_cast<float>(
          (*(evt.devices().find(_device)->second)->detectors())[_detector].integral()));
  _result->lock.lockForWrite();
  dynamic_cast<Histogram0DFloat*>(_result)->fill(integral);
  _result->lock.unlock();
}









// *** Integral over the Image for pixel(s) above thres ***

cass::pp102::pp102(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)
{
  loadSettings(0);
}

void cass::pp102::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _device = static_cast<CASSEvent::Device>(settings.value("Device",0).toUInt());
  _detector = settings.value("Detector",0).toUInt();
  _result = new Histogram0DFloat();
  setupGeneral();
  if (!setupCondition())
    return;
  createHistList(2*cass::NbrOfWorkers);
  std::cout <<std::endl<< "PostProcessor "<<_key
      <<": retrieves the Integral over the whole "<<_detector
      <<" detector calculated using pixels over threshold."
      <<". Condition is"<<_condition->key()
      <<std::endl;
}

void cass::pp102::process(const cass::CASSEvent &evt)
{
  using namespace std;

  if (evt.devices().find(_device)->second->detectors()->size() <= _detector)
    throw std::runtime_error(QString("PostProcessor_%1: Detector %2 does not exist in Device %3")
                             .arg(_key.c_str())
                             .arg(_detector)
                             .arg(_device).toStdString());
  const float& integral_overthres
      (static_cast<float>(
          (*(evt.devices().find(_device)->second)->detectors())[_detector].integral_overthres()));
  _result->lock.lockForWrite();
  dynamic_cast<Histogram0DFloat*>(_result)->fill(integral_overthres);
  _result->lock.unlock();
}








// *** A Postprocessor that will display the photonhits of ccd detectors in 1D hist***

cass::pp140::pp140(PostProcessors& pp, const cass::PostProcessors::key_t &key)
    : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}


void cass::pp140::loadSettings(size_t)
{
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _device = static_cast<CASSEvent::Device>(settings.value("Device",0).toUInt());
  _detector = settings.value("Detector",0).toUInt();
  _adu2eV = settings.value("Adu2eV",1).toFloat();
  setupGeneral();
  if (!setupCondition())
    return;
  set1DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  std::cout<<"Postprocessor "<<_key<<":"
      <<" will display ccd spectrum of detector "<<_detector
      <<" in device "<<_device
      <<". Pixelvalues will be converter by factor "<<_adu2eV
      <<". Condition is"<<_condition->key()
      <<std::endl;
}

void cass::pp140::process(const CASSEvent& evt)
{
  if (evt.devices().find(_device)->second->detectors()->size() <= _detector)
    throw std::runtime_error(QString("PostProcessor_%1: Detector %2 does not exist in Device %3")
                             .arg(_key.c_str())
                             .arg(_detector)
                             .arg(_device).toStdString());
  const PixelDetector::pixelList_t& pixellist
      ((*(evt.devices().find(_device)->second)->detectors())[_detector].pixellist());
  PixelDetector::pixelList_t::const_iterator it(pixellist.begin());
  _result->clear();
  _result->lock.lockForWrite();
  for (; it != pixellist.end();++it)
    dynamic_cast<Histogram1DFloat*>(_result)->fill(it->z()*_adu2eV);
  _result->lock.unlock();
}









// *** A Postprocessor that will display the photonhits of ccd detectors ***

cass::pp141::pp141(PostProcessors& pp, const cass::PostProcessors::key_t &key)
    : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp141::loadSettings(size_t)
{
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _device = static_cast<CASSEvent::Device>(settings.value("Device",0).toUInt());
  _detector = settings.value("Detector",0).toUInt();
  setupGeneral();
  if (!setupCondition())
    return;
  set2DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  std::cout<<"Postprocessor "<<_key<<":"
      <<" will display ccd image of detector "<<_detector
      <<" in device "<<_device
      <<". Condition is"<<_condition->key()
      <<std::endl;
}

void cass::pp141::process(const CASSEvent& evt)
{
  using namespace std;
  if (evt.devices().find(_device)->second->detectors()->size() <= _detector)
    throw std::runtime_error(QString("PostProcessor_%1: Detector %2 does not exist in Device %3")
                             .arg(_key.c_str())
                             .arg(_detector)
                             .arg(_device).toStdString());
  const PixelDetector::pixelList_t& pixellist
      ((*(evt.devices().find(_device)->second)->detectors())[_detector].pixellist());
  PixelDetector::pixelList_t::const_iterator it(pixellist.begin());
  _result->clear();
  _result->lock.lockForWrite();
  for (; it != pixellist.end();++it)
    dynamic_cast<Histogram2DFloat*>(_result)->fill(it->x(),it->y());
  _result->lock.unlock();
}













// *** A Postprocessor retrieve the number of the photonhits of ccd detectors ***

cass::pp142::pp142(PostProcessors& pp, const cass::PostProcessors::key_t &key)
    : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}


void cass::pp142::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _device = static_cast<CASSEvent::Device>(settings.value("Device",0).toUInt());
  _detector = settings.value("Detector",0).toUInt();
  setupGeneral();
  if (!setupCondition())
    return;
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);
  cout<<endl<<"Postprocessor "<<_key<<":"
      <<" will retrieve the number of photon hits of detector '"<<_detector
      <<"' in device '"<<_device
      <<"'. Condition is '"<<_condition->key()<<"'"
      <<endl;
}

void cass::pp142::process(const CASSEvent& evt)
{
  if (evt.devices().find(_device)->second->detectors()->size() <= _detector)
    throw std::runtime_error(QString("PostProcessor_%1: Detector %2 does not exist in Device %3")
                             .arg(_key.c_str())
                             .arg(_detector)
                             .arg(_device).toStdString());
  const PixelDetector::pixelList_t& pixellist
      ((*(evt.devices().find(_device)->second)->detectors())[_detector].pixellist());
  _result->lock.lockForWrite();
  dynamic_cast<Histogram0DFloat*>(_result)->fill(pixellist.size());
  _result->lock.unlock();
}











// *** A Postprocessor that will display the coalesced photonhits spectrum ***

cass::pp143::pp143(PostProcessors& pp, const cass::PostProcessors::key_t &key)
    : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp143::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString::fromStdString(_key));
  _detector = settings.value("Detector","blubb").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  _splitLevelRange = make_pair(settings.value("SplitLevelLowerLimit",0).toUInt(),
                               settings.value("SplitLevelUpperLimit",2).toUInt());
  set1DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  HelperPixelDetectors::instance(_detector)->loadSettings();
  cout<<endl<<"Postprocessor '"<<_key
      <<"' will display the spectrum of detector '"<<_detector
      <<"' only when the the split level is between '"<<_splitLevelRange.first
      <<"' and '"<<_splitLevelRange.second
      <<"'. Condition is '"<<_condition->key()<<"'"
      <<endl;
}

void cass::pp143::process(const CASSEvent& evt)
{
  HelperPixelDetectors::PixDetContainer_sptr det
      (HelperPixelDetectors::instance(_detector)->detector(evt));
  PixelDetectorContainer::hitlist_t::const_iterator hit(det->hits().begin());
  _result->lock.lockForWrite();
  _result->clear();
  for (; hit != det->hits().end(); ++hit)
  {
    if (_splitLevelRange.first < hit->nbrPixels() && hit->nbrPixels() < _splitLevelRange.second)
      dynamic_cast<Histogram1DFloat*>(_result)->fill(hit->z());
  }
  _result->lock.unlock();
}











// *** A Postprocessor that will display the coalesced photonhits of ccd detectors ***

cass::pp144::pp144(PostProcessors& pp, const cass::PostProcessors::key_t &key)
    : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp144::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString::fromStdString(_key));
  _detector = settings.value("Detector","blubb").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  set2DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  _range = make_pair(settings.value("SpectralLowerLimit",0.).toFloat(),
                     settings.value("SpectralUpperLimit",0.).toFloat());
  _splitLevelRange = make_pair(settings.value("SplitLevelLowerLimit",0).toUInt(),
                               settings.value("SplitLevelUpperLimit",2).toUInt());
  HelperPixelDetectors::instance(_detector)->loadSettings();
  cout<<endl<<"Postprocessor '"<<_key
      <<"' will display ccd image of detector '"<<_detector
      <<"' only when the spectral component is between '"<<_range.first
      <<"' and '"<<_range.second
      <<"', and the the split level is between '"<<_splitLevelRange.first
      <<"' and '"<<_splitLevelRange.second
      <<"'. Condition is '"<<_condition->key()<<"'"
      <<endl;
}

void cass::pp144::process(const CASSEvent& evt)
{
  HelperPixelDetectors::PixDetContainer_sptr det
      (HelperPixelDetectors::instance(_detector)->detector(evt));
  PixelDetectorContainer::hitlist_t::const_iterator hit(det->hits().begin());
  _result->lock.lockForWrite();
  _result->clear();
  for (; hit != det->hits().end(); ++hit)
  {
    if (_splitLevelRange.first < hit->nbrPixels() && hit->nbrPixels() < _splitLevelRange.second)
      if (_range.first < hit->z() && hit->z() < _range.second)
        dynamic_cast<Histogram2DFloat*>(_result)->fill(hit->x(),hit->y(),hit->z());
  }
  _result->lock.unlock();
}










// *** A Postprocessor that will retrieve the number of coalesced hits ***

cass::pp145::pp145(PostProcessors& pp, const cass::PostProcessors::key_t &key)
    : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp145::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString::fromStdString(_key));
  _detector = settings.value("Detector","blubb").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);
  HelperPixelDetectors::instance(_detector)->loadSettings();
  cout<<endl<<"Postprocessor '"<<_key
      <<"' will retrieve the number of coalesced photonhits of detector '"<<_detector
      <<"'. Condition is '"<<_condition->key()<<"'"
      <<endl;
}

void cass::pp145::process(const CASSEvent& evt)
{
  HelperPixelDetectors::PixDetContainer_sptr det
      (HelperPixelDetectors::instance(_detector)->detector(evt));
  const PixelDetectorContainer::hitlist_t& hits(det->hits());
  _result->lock.lockForWrite();
  dynamic_cast<Histogram0DFloat*>(_result)->fill(hits.size());
  _result->lock.unlock();
}








// *** A Postprocessor that will output the split level of the coalesced pixels ***

cass::pp146::pp146(PostProcessors& pp, const cass::PostProcessors::key_t &key)
    : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp146::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString::fromStdString(_key));
  _detector = settings.value("Detector","blubb").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  set1DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  HelperPixelDetectors::instance(_detector)->loadSettings();
  cout<<endl<<"Postprocessor '"<<_key
      <<"' will retrieve the number of coalesced photonhits of detector '"<<_detector
      <<"'. Condition is '"<<_condition->key()<<"'"
      <<endl;
}

void cass::pp146::process(const CASSEvent& evt)
{
  HelperPixelDetectors::PixDetContainer_sptr det
      (HelperPixelDetectors::instance(_detector)->detector(evt));
  PixelDetectorContainer::hitlist_t::const_iterator hit(det->hits().begin());
  _result->lock.lockForWrite();
  _result->clear();
  for (; hit != det->hits().end(); ++hit)
  {
      dynamic_cast<Histogram1DFloat*>(_result)->fill(hit->nbrPixels());
  }
  _result->lock.unlock();
}



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "Stroustrup"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:
