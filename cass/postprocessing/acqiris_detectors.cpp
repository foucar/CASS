//Copyright (C) 2010 Lutz Foucar

#include <stdexcept>
#include <cmath>
#include <algorithm>

#include <QtCore/QSettings>
#include <QtCore/QString>

#include "acqiris_detectors.h"
#include "acqiris_detectors_helper.h"
#include "histogram.h"
#include "cass_event.h"
#include "acqiris_device.h"
#include "com.h"
#include "cfd.h"
#include "delayline_detector_analyzer_simple.h"
#include "delayline_detector.h"
#include "tof_analyzer_simple.h"
#include "tof_detector.h"



void cass::set1DHist(cass::Histogram1DFloat*& hist, PostProcessors::key_t key)
{
  //open the settings//
  QSettings param;
  param.beginGroup("PostProcessor/active");
  param.beginGroup(key.c_str());
  //create new histogram using the parameters//
  std::cerr << "Creating 1D histogram with"
      <<" XNbrBins:"<<param.value("XNbrBins",1).toUInt()
      <<" XLow:"<<param.value("XLow",0).toFloat()
      <<" XUp:"<<param.value("XUp",0).toFloat()
      <<std::endl;
  hist = new cass::Histogram1DFloat(param.value("XNbrBins",1).toUInt(),
                                    param.value("XLow",0).toFloat(),
                                    param.value("XUp",0).toFloat());
}

void cass::set2DHist(cass::Histogram2DFloat*& hist, PostProcessors::key_t key)
{
  //open the settings//
  QSettings param;
  param.beginGroup("PostProcessor/active");
  param.beginGroup(key.c_str());
  //create new histogram using the parameters//
  std::cerr << "Creating 2D histogram with"
      <<" XNbrBins:"<<param.value("XNbrBins",1).toUInt()
      <<" XLow:"<<param.value("XLow",0).toFloat()
      <<" XUp:"<<param.value("XUp",0).toFloat()
      <<" YNbrBins:"<<param.value("YNbrBins",1).toUInt()
      <<" YLow:"<<param.value("YLow",0).toFloat()
      <<" YUp:"<<param.value("YUp",0).toFloat()
      <<std::endl;
  hist = new cass::Histogram2DFloat(param.value("XNbrBins",1).toUInt(),
                                    param.value("XLow",0).toFloat(),
                                    param.value("XUp",0).toFloat(),
                                    param.value("YNbrBins",1).toUInt(),
                                    param.value("YLow",0).toFloat(),
                                    param.value("YUp",0).toFloat());
}










//----------------Nbr of Peaks MCP---------------------------------------------
cass::pp150::pp150(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key),
  _nbrSignals(0)
{
  //create the histogram by loading the settings//
  loadSettings(0);
}

cass::pp150::~pp150()
{
  _pp.histograms_delete(_key);
  _nbrSignals=0;
}

void cass::pp150::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector = static_cast<Detectors>(settings.value("Detector",0).toUInt());

  std::cout <<std::endl<< "PostProcessor "<<_key
      <<": retrieves the nbr of mcp signals"
      <<" of detector "<<_detector
      <<std::endl;

  //create the histogram
  _pp.histograms_delete(_key);
  _nbrSignals = new Histogram0DFloat();
  _pp.histograms_replace(_key,_nbrSignals);
  //load the detectors settings
  HelperAcqirisDetectors::instance(_detector)->loadSettings();
}

void cass::pp150::operator()(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  //get right filled detector from the helper
  TofDetector *det =
      dynamic_cast<TofDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt));
  _nbrSignals->lock.lockForWrite();
  _nbrSignals->fill(det->mcp().peaks().size());
  _nbrSignals->lock.unlock();
}










//----------------MCP Hits (Tof)-----------------------------------------------
cass::pp151::pp151(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key),
  _tof(0)
{
  //create the histogram by loading the settings//
  loadSettings(0);
}

cass::pp151::~pp151()
{
  _pp.histograms_delete(_key);
  _tof=0;
}

void cass::pp151::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector = static_cast<Detectors>(settings.value("Detector",0).toUInt());

  //create the histogram
  set1DHist(_tof,_key);
  _pp.histograms_replace(_key,_tof);

  //load the detectors settings
  HelperAcqirisDetectors::instance(_detector)->loadSettings();

  std::cout <<std::endl<< "PostProcessor "<<_key
      <<": it histograms times of the found mcp signals"
      <<" of detector "<<_detector
      <<std::endl;
}

void cass::pp151::operator()(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  //get right filled detector from the helper
  TofDetector *det
      (dynamic_cast<TofDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt)));
  //reference to all found peaks of the mcp channel//
  Signal::peaks_t::const_iterator it (det->mcp().peaks().begin());
  //clear histo and fill all found peaks into the histogram//
  _tof->lock.lockForWrite();
  fill(_tof->memory().begin(),_tof->memory().end(),0.f);
  for (; it != det->mcp().peaks().end(); ++it)
    _tof->fill(it->time());
  _tof->lock.unlock();
}










//----------------MCP Fwhm vs. height------------------------------------------
cass::pp152::pp152(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key),
  _sigprop(0)
{
  loadSettings(0);
}

cass::pp152::~pp152()
{
  _pp.histograms_delete(_key);
  _sigprop=0;
}

void cass::pp152::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector = static_cast<Detectors>(settings.value("Detector",0).toUInt());


  std::cout <<std::endl<< "PostProcessor "<<_key
      <<": histograms the FWHM vs the height of the found mcp signals"
      <<" of  detector "<<_detector
      <<std::endl;

  //create the histogram
  set2DHist(_sigprop,_key);
  _pp.histograms_replace(_key,_sigprop);
  //load the detectors settings
  HelperAcqirisDetectors::instance(_detector)->loadSettings();
}

void cass::pp152::operator()(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  //get right filled detector from the helper
  TofDetector *det
      (dynamic_cast<TofDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt)));
  //reference to all found peaks of the mcp channel//
  Signal::peaks_t::const_iterator it (det->mcp().peaks().begin());
  //fill all found peaks into the histogram//
  _sigprop->lock.lockForWrite();
  fill(_sigprop->memory().begin(),_sigprop->memory().end(),0.f);
  for (;it != det->mcp().peaks().end(); ++it)
    _sigprop->fill(it->fwhm(),it->height());
  _sigprop->lock.unlock();
}












//----------------Nbr of Peaks Anode-------------------------------------------
cass::pp160::pp160(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key),
  _nbrSignals(0)
{
  loadSettings(0);
}

cass::pp160::~pp160()
{
  _pp.histograms_delete(_key);
  _nbrSignals=0;
}

void cass::pp160::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector = static_cast<Detectors>(settings.value("Detector",0).toUInt());
  _layer = settings.value("Layer",'U').toChar().toAscii();
  _signal = settings.value("Wireend",'1').toChar().toAscii();

  std::cout <<std::endl<< "PostProcessor "<<_key
      <<": histograms the nbr of signals in"
      <<" detector "<<_detector
      <<" layer "<<_layer
      <<" wireend "<<_signal
      <<std::endl;

  //create the histogram
  _pp.histograms_delete(_key);
  _nbrSignals = new Histogram0DFloat();
  _pp.histograms_replace(_key,_nbrSignals);
  //load the detectors settings
  HelperAcqirisDetectors::instance(_detector)->loadSettings();
}

void cass::pp160::operator()(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  //get right filled detector from the helper
  DelaylineDetector *det
      (dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt)));
  _nbrSignals->lock.lockForWrite();
  _nbrSignals->fill(det->layers()[_layer].wireend()[_signal].peaks().size());
  _nbrSignals->lock.unlock();
}











//----------------FWHM vs. Height of Wireend Signals---------------------------
cass::pp161::pp161(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key),
  _sigprop(0)
{
  loadSettings(0);
}

cass::pp161::~pp161()
{
  _pp.histograms_delete(_key);
  _sigprop=0;
}

void cass::pp161::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;

  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector = static_cast<Detectors>(settings.value("Detector",0).toUInt());
  _layer = settings.value("Layer",'U').toChar().toAscii();
  _signal = settings.value("Wireend",'1').toChar().toAscii();

  std::cout <<std::endl<< "PostProcessor "<<_key
      <<": histograms the FWHM vs the height of layer "<<_layer
      << " wireend "<<_signal
      <<" of detector "<<_detector
      <<std::endl;
  //create the histogram
  set2DHist(_sigprop,_key);
  _pp.histograms_replace(_key,_sigprop);
  //load the detectors settings
  HelperAcqirisDetectors::instance(_detector)->loadSettings();
}

void cass::pp161::operator()(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  DelaylineDetector *det
      (dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt)));
  Signal::peaks_t::const_iterator it (det->layers()[_layer].wireend()[_signal].peaks().begin());
  _sigprop->lock.lockForWrite();
  fill(_sigprop->memory().begin(),_sigprop->memory().end(),0.f);
  for (; it != det->layers()[_layer].wireend()[_signal].peaks().end(); ++it)
    _sigprop->fill(it->fwhm(),it->height());
  _sigprop->lock.unlock();
}
















////----------------Ratio of rec. Hits vs. MCP Hits------------------------------
////------------------------------pp566, pp611-----------------------------------
//cass::pp566::pp566(PostProcessors &pp, PostProcessors::id_t id)
//  :cass::PostprocessorBackend(pp,id),
//  _ratio(0)
//{
//  using namespace cass::ACQIRIS;
//
//  //find out which detector and Signal we should work on
//  switch (_id)
//  {
//  case PostProcessors::HexRekMcpRatio:
//    _detector = HexDetector;break;
//  case PostProcessors::QuadRekMcpRatio:
//    _detector = QuadDetector;break;
//
//  default:
//    throw std::invalid_argument(QString("postprocessor %1 is not responsible for Ratio of reconstructed Hits vs MCP Hits").arg(id).toStdString());
//  }
//  //create the histogram by loading the settings//
//  loadSettings(0);
//}
//
//cass::pp566::~pp566()
//{
//  _pp.histograms_delete(_id);
//  _ratio=0;
//}
//
//void cass::pp566::loadSettings(size_t)
//{
//  using namespace cass::ACQIRIS;
//
//  std::cout <<std::endl<< "load the parameters of postprocessor "<<_id
//      <<" it histograms the ratio of reconstructed hits vs. Mcp peaks"
//      <<" of detector "<<_detector
//      <<std::endl;
//
//  //create the histogram
//  set1DHist(_ratio,_id);
//  _pp.histograms_replace(_id,_ratio);
//  //load the detectors settings
//  HelperAcqirisDetectors::instance(_detector)->loadSettings();
//  std::cout << "done loading postprocessor "<<_id<<"'s parameters"<<std::endl;
//}
//
//void cass::pp566::operator()(const cass::CASSEvent &evt)
//{
//  using namespace cass::ACQIRIS;
//  //get right filled detector from the helper
//  DelaylineDetector *det =
//      dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt));
//  const float rek = det->hits().size();
//  const float mcp = det->mcp().peaks().size();
//  _ratio->lock.lockForWrite();
//  _ratio->fill(rek/mcp);
//  _ratio->lock.unlock();
//}
//
//
//
//
//
//
//
//
//
////----------------MCP Hits (Tof)-----------------------------------------------
////-------------pp567, pp612, pp651, pp661, pp671, pp681------------------------
//cass::pp567::pp567(PostProcessors &pp, PostProcessors::id_t id)
//  :cass::PostprocessorBackend(pp,id),
//  _tof(0)
//{
//  using namespace cass::ACQIRIS;
//
//  //find out which detector and Signal we should work on
//  switch (_id)
//  {
//  case PostProcessors::HexAllMcp:
//    _detector = HexDetector;break;
//  case PostProcessors::QuadAllMcp:
//    _detector = QuadDetector;break;
//  case PostProcessors::VMIMcpAllMcp:
//    _detector = VMIMcp;break;
//  case PostProcessors::FELBeamMonitorAllMcp:
//    _detector = FELBeamMonitor;break;
//  case PostProcessors::YAGPhotodiodeAllMcp:
//    _detector = YAGPhotodiode;break;
//  case PostProcessors::FsPhotodiodeAllMcp:
//    _detector = FsPhotodiode;break;
//
//  default:
//    throw std::invalid_argument(QString("postprocessor %1 is not responsible for All MCP Hits").arg(id).toStdString());
//  }
//  //create the histogram by loading the settings//
//  loadSettings(0);
//}
//
//cass::pp567::~pp567()
//{
//  _pp.histograms_delete(_id);
//  _tof=0;
//}
//
//void cass::pp567::loadSettings(size_t)
//{
//  using namespace cass::ACQIRIS;
//
//  std::cout <<std::endl<< "load the parameters of postprocessor "<<_id
//      <<" it histograms times of the found MCP Hits"
//      <<" of detector "<<_detector
//      <<std::endl;
//
//  //create the histogram
//  set1DHist(_tof,_id);
//  _pp.histograms_replace(_id,_tof);
//  //load the detectors settings
//  HelperAcqirisDetectors::instance(_detector)->loadSettings();
//  std::cout << "done loading postprocessor "<<_id<<"'s parameters"<<std::endl;
//}
//
//void cass::pp567::operator()(const cass::CASSEvent &evt)
//{
//  using namespace cass::ACQIRIS;
//  //get right filled detector from the helper
//  TofDetector *det =
//      dynamic_cast<TofDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt));
//  //reference to all found peaks of the mcp channel//
//  Signal::peaks_t::const_iterator it = det->mcp().peaks().begin();
//  //fill all found peaks into the histogram//
//  _tof->lock.lockForWrite();
//  for (; it != det->mcp().peaks().end(); ++it)
//    _tof->fill(it->time());
//  _tof->lock.unlock();
//}
//
//
//
//
//
//
//
//
////----------------Timesum for the layers---------------------------------------
////-----------pp568-570, pp613-614----------------------------------------------
//cass::pp568::pp568(PostProcessors &pp, PostProcessors::id_t id)
//  :cass::PostprocessorBackend(pp,id),
//  _timesum(0)
//{
//  using namespace cass::ACQIRIS;
//
//  //find out which detector and Signal we should work on
//  switch (_id)
//  {
//  case PostProcessors::HexTimesumU:
//    _detector = HexDetector; _layer = 'U'; break;
//  case PostProcessors::HexTimesumV:
//    _detector = HexDetector; _layer = 'V'; break;
//  case PostProcessors::HexTimesumW:
//    _detector = HexDetector; _layer = 'W'; break;
//
//  case PostProcessors::QuadTimesumX:
//    _detector = QuadDetector; _layer = 'X'; break;
//  case PostProcessors::QuadTimesumY:
//    _detector = QuadDetector; _layer = 'Y'; break;
//
//  default:
//    throw std::invalid_argument(QString("postprocessor %1 is not responsible for Timesum").arg(id).toStdString());
//  }
//  //create the histogram by loading the settings//
//  loadSettings(0);
//}
//
//cass::pp568::~pp568()
//{
//  _pp.histograms_delete(_id);
//  _timesum=0;
//}
//
//void cass::pp568::loadSettings(size_t)
//{
//  using namespace cass::ACQIRIS;
//
//  std::cout <<std::endl<< "load the parameters of postprocessor "<<_id
//      <<" it histograms the timesum of layer "<<_layer
//      <<" of detector "<<_detector
//      <<std::endl;
//
//  //create the histogram
//  set1DHist(_timesum,_id);
//  _pp.histograms_replace(_id,_timesum);
//  //load the detectors settings
//  HelperAcqirisDetectors::instance(_detector)->loadSettings();
//  std::cout << "done loading postprocessor "<<_id<<"'s parameters"<<std::endl;
//}
//
//void cass::pp568::operator()(const cass::CASSEvent &evt)
//{
//  using namespace cass::ACQIRIS;
//  //get right filled detector from the helper
//  DelaylineDetector *det =
//      dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt));
//  _timesum->lock.lockForWrite();
//  _timesum->fill(det->timesum(_layer));
//  _timesum->lock.unlock();
//}
//
//
//
//
//
//
//
//
//
////----------------Timesum vs Postition for the layers--------------------------
////-----------pp571-573, pp615-616----------------------------------------------
//cass::pp571::pp571(PostProcessors &pp, PostProcessors::id_t id)
//  :cass::PostprocessorBackend(pp,id),
//  _timesumvsPos(0)
//{
//  using namespace cass::ACQIRIS;
//
//  //find out which detector and Signal we should work on
//  switch (_id)
//  {
//  case PostProcessors::HexTimesumUvsU:
//    _detector = HexDetector; _layer = 'U'; break;
//  case PostProcessors::HexTimesumVvsV:
//    _detector = HexDetector; _layer = 'V'; break;
//  case PostProcessors::HexTimesumWvsW:
//    _detector = HexDetector; _layer = 'W'; break;
//
//  case PostProcessors::QuadTimesumXvsX:
//    _detector = QuadDetector; _layer = 'X'; break;
//  case PostProcessors::QuadTimesumYvsY:
//    _detector = QuadDetector; _layer = 'Y'; break;
//
//  default:
//    throw std::invalid_argument(QString("postprocessor %1 is not responsible for Timesum vs. Pos").arg(id).toStdString());
//  }
//  //create the histogram by loading the settings//
//  loadSettings(0);
//}
//
//cass::pp571::~pp571()
//{
//  _pp.histograms_delete(_id);
//  _timesumvsPos=0;
//}
//
//void cass::pp571::loadSettings(size_t)
//{
//  using namespace cass::ACQIRIS;
//
//  std::cout <<std::endl<< "load the parameters of postprocessor "<<_id
//      <<" it histograms the timesum vs Postion on layer "<<_layer
//      <<" of detector "<<_detector
//      <<std::endl;
//
//  //create the histogram
//  set2DHist(_timesumvsPos,_id);
//  _pp.histograms_replace(_id,_timesumvsPos);
//  //load the detectors settings
//  HelperAcqirisDetectors::instance(_detector)->loadSettings();
//  std::cout << "done loading postprocessor "<<_id<<"'s parameters"<<std::endl;
//}
//
//void cass::pp571::operator()(const cass::CASSEvent &evt)
//{
//  using namespace cass::ACQIRIS;
//  //get right filled detector from the helper
//  DelaylineDetector *det =
//      dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt));
//  _timesumvsPos->lock.lockForWrite();
//  _timesumvsPos->fill(det->position(_layer),det->timesum(_layer));
//  _timesumvsPos->lock.unlock();
//}
//
//
//
//
//
//
//
//
//
//
////----------------Detector First Hit-------------------------------------------
////-----------pp574-577, pp617--------------------------------------------------
//cass::pp574::pp574(PostProcessors &pp, PostProcessors::id_t id)
//  :cass::PostprocessorBackend(pp,id),
//  _pos(0)
//{
//  using namespace cass::ACQIRIS;
//
//  //find out which detector and Signal we should work on
//  switch (_id)
//  {
//  case PostProcessors::HexFirstUV:
//    _detector = HexDetector; _first = 'U'; _second = 'V'; break;
//  case PostProcessors::HexFirstUW:
//    _detector = HexDetector; _first = 'U'; _second = 'W'; break;
//  case PostProcessors::HexFirstVW:
//    _detector = HexDetector; _first = 'V'; _second = 'W'; break;
//
//  case PostProcessors::QuadFirstXY:
//    _detector = QuadDetector; _first = 'X'; _second = 'Y'; break;
//
//  default:
//    throw std::invalid_argument(QString("postprocessor %1 is not responsible for Detector Picture of First Hit").arg(id).toStdString());
//  }
//  //create the histogram by loading the settings//
//  loadSettings(0);
//}
//
//cass::pp574::~pp574()
//{
//  _pp.histograms_delete(_id);
//  _pos=0;
//}
//
//void cass::pp574::loadSettings(size_t)
//{
//  using namespace cass::ACQIRIS;
//
//  std::cout <<std::endl<< "load the parameters of postprocessor "<<_id
//      <<" it histograms a detector picture of the first Hit on the detector created"
//      <<" from  Layers "<<_first
//      << " and "<<_second
//      <<" of detector "<<_detector
//      <<std::endl;
//
//  //create the histogram
//  set2DHist(_pos,_id);
//  _pp.histograms_replace(_id,_pos);
//  //load the detectors settings
//  HelperAcqirisDetectors::instance(_detector)->loadSettings();
//  std::cout << "done loading postprocessor "<<_id<<"'s parameters"<<std::endl;
//}
//
//void cass::pp574::operator()(const cass::CASSEvent &evt)
//{
//  using namespace cass::ACQIRIS;
//  //get right filled detector from the helper
//  DelaylineDetector *det =
//      dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt));
//  //get the requested layers//
//  AnodeLayer &f = det->layers()[_first];
//  AnodeLayer &s = det->layers()[_second];
//  //get the timesums for the layers//
//  const double tsf = det->timesum(_first);
//  const double tss = det->timesum(_second);
//  //check timesum//
//  const bool csf = (f.tsLow() < tsf && tsf < f.tsHigh());
//  const bool css = (s.tsLow() < tss && tss < s.tsHigh());
//  //only fill when timesum is fullfilled
//  _pos->lock.lockForWrite();
//  if (csf && css)
//    _pos->fill(f.position(),s.position());
//  _pos->lock.unlock();
//}
//
//
//
//
//
//
//
//
//
//
//
////----------------Detector Values----------------------------------------------
////-----------pp578-580, pp61-620-----------------------------------------------
//cass::pp578::pp578(PostProcessors &pp, PostProcessors::id_t id)
//  :cass::PostprocessorBackend(pp,id),
//  _hist(0)
//
//{
//  using namespace cass::ACQIRIS;
//
//  //find out which detector and Signal we should work on
//  switch (_id)
//  {
//  case PostProcessors::HexXY:
//    _detector = HexDetector; _first = 'x'; _second = 'y'; _third = 't'; break;
//  case PostProcessors::HexXT:
//    _detector = HexDetector; _first = 't'; _second = 'x'; _third = 'y'; break;
//  case PostProcessors::HexYT:
//    _detector = HexDetector; _first = 't'; _second = 'y'; _third = 'x'; break;
//
//  case PostProcessors::QuadXY:
//    _detector = QuadDetector; _first = 'x'; _second = 'y'; _third = 't'; break;
//  case PostProcessors::QuadXT:
//    _detector = QuadDetector; _first = 't'; _second = 'x'; _third = 'y'; break;
//  case PostProcessors::QuadYT:
//    _detector = QuadDetector; _first = 't'; _second = 'y'; _third = 'x'; break;
//
//  default:
//    throw std::invalid_argument(QString("postprocessor %1 is not responsible for Detector Values").arg(id).toStdString());
//  }
//  //create the histogram by loading the settings//
//  loadSettings(0);
//}
//
//cass::pp578::~pp578()
//{
//  _pp.histograms_delete(_id);
//  _hist=0;
//}
//
//void cass::pp578::loadSettings(size_t)
//{
//  using namespace cass::ACQIRIS;
//
//  QSettings param;
//  param.beginGroup("PostProcessor");
//  param.beginGroup(QString("p") + QString::number(_id));
//  //load the condition on the third component//
//  float f = param.value("ConditionLow",-50000.).toFloat();
//  float s = param.value("ConditionHigh",50000.).toFloat();
//  param.endGroup();
//  //make sure that the first value of the condition is the lower and second the higher value//
//  _condition.first = (f<=s)?f:s;
//  _condition.second = (s>f)?s:f;
//  //tell the user what is loaded//
//
//  std::cout <<std::endl<< "load the parameters of postprocessor "<<_id
//      <<" it histograms the Property "<<_second
//      <<" vs. "<<_first
//      <<" of the reconstructed Detectorhits of detector "<<_detector
//      <<" condition Low "<<_condition.first
//      <<" High "<<_condition.second
//      <<" on Property "<< _third
//      <<std::endl;
//  //create the histogram
//  set2DHist(_hist,_id);
//  _pp.histograms_replace(_id,_hist);
//  //load the detectors settings
//  HelperAcqirisDetectors::instance(_detector)->loadSettings();
//  std::cout << "done loading postprocessor "<<_id<<"'s parameters"<<std::endl;
//}
//
//void cass::pp578::operator()(const cass::CASSEvent &evt)
//{
//  using namespace cass::ACQIRIS;
//  //get right filled detector from the helper
//  DelaylineDetector *det =
//      dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt));
//  //get iterator to the hits//
//  DelaylineDetector::dethits_t::iterator it = det->hits().begin();
////  std::cout << det->hits().size()<<std::endl;
//  //go through all hits of the detector//
//  _hist->lock.lockForWrite();
//  for (; it != det->hits().end(); ++it)
//  {
////    std::cout
////        <<" "<<_first<<":"<<it->values()[_first]
////        <<" "<<_second<<":"<<it->values()[_second]
////        <<" "<<_third<<":"<<it->values()[_third]
////        <<std::endl;
//    if (_condition.first < it->values()[_third] && it->values()[_third] < _condition.second)
//      _hist->fill(it->values()[_first],it->values()[_second]);
//  }
//  _hist->lock.unlock();
//}
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
////----------------PIPICO-------------------------------------------------------
////-----------pp700-701---------------------------------------------------------
//cass::pp700::pp700(PostProcessors &pp, PostProcessors::id_t id)
//  :cass::PostprocessorBackend(pp,id),
//  _pipico(0)
//{
//  using namespace cass::ACQIRIS;
//
//  //find out which detector and Signal we should work on
//  switch (_id)
//  {
//  case PostProcessors::HexPIPICO:
//    _detector01 = HexDetector; _detector02 = HexDetector; break;
//  case PostProcessors::HexQuadPIPICO:
//    _detector01 = HexDetector; _detector02 = QuadDetector; break;
//
//  default:
//    throw std::invalid_argument(QString("postprocessor %1 is not responsible for FWHM vs. Height of Layer Signals").arg(id).toStdString());
//  }
//  //create the histogram by loading the settings//
//  loadSettings(0);
//}
//
//cass::pp700::~pp700()
//{
//  _pp.histograms_delete(_id);
//  _pipico=0;
//}
//
//void cass::pp700::loadSettings(size_t)
//{
//  using namespace cass::ACQIRIS;
//
//  std::cout <<std::endl<< "load the parameters of postprocessor "<<_id
//      <<" it create a PIPICO Histogram"
//      <<" of detectors "<<_detector01
//      <<" and "<<_detector02
//      <<std::endl;
//
//  //create the histogram
//  set2DHist(_pipico,_id);
//  _pp.histograms_replace(_id,_pipico);
//    //load the detectors settings
//  HelperAcqirisDetectors::instance(_detector01)->loadSettings();
//  HelperAcqirisDetectors::instance(_detector02)->loadSettings();
//  std::cout << "done loading postprocessor "<<_id<<"'s parameters"<<std::endl;
//}
//
//void cass::pp700::operator()(const cass::CASSEvent &evt)
//{
//  using namespace cass::ACQIRIS;
//  //get first detector from the helper
//  TofDetector *det01 =
//      dynamic_cast<TofDetector*>(HelperAcqirisDetectors::instance(_detector01)->detector(evt));
//  //get second detector from the helper
//  TofDetector *det02 =
//      dynamic_cast<TofDetector*>(HelperAcqirisDetectors::instance(_detector02)->detector(evt));
//  //get iterator of the peaks in the first detector//
//  Signal::peaks_t::const_iterator it01(det01->mcp().peaks().begin());
//  //draw all found hits vs another//
//  _pipico->lock.lockForWrite();
//  for (; it01 != det01->mcp().peaks().end();++it01)
//  {
//    //if both detectors are the same, then the second iterator should start
//    //i+1, otherwise we will just draw all hits vs. all hits
//    Signal::peaks_t::const_iterator it02((_detector01==_detector02) ?
//                                         it01+1 :
//                                         det02->mcp().peaks().begin());
//    for (; it02 != det02->mcp().peaks().end(); ++it02)
//    {
//      _pipico->fill(it01->time(),it02->time());
//    }
//  }
//  _pipico->lock.unlock();
//}
