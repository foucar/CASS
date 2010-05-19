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










////----------------Nbr of Peaks MCP---------------------------------------------
////-----------pp550, pp600, pp650, pp660, pp670, pp680--------------------------
//cass::pp550::pp550(PostProcessors &pp, PostProcessors::id_t id)
//  :cass::PostprocessorBackend(pp,id),
//  _nbrSignals(0)
//{
//  using namespace cass::ACQIRIS;
//  //find out which detector and Signal we should work on
//  switch (_id)
//  {
//  case PostProcessors::HexMCPNbrSignals:
//    _detector = HexDetector;break;
//  case PostProcessors::QuadMCPNbrSignals:
//    _detector = QuadDetector;break;
//  case PostProcessors::VMIMcpNbrSignals:
//    _detector = VMIMcp;break;
//  case PostProcessors::FELBeamMonitorNbrSignals:
//    _detector = FELBeamMonitor;break;
//  case PostProcessors::YAGPhotodiodeNbrSignals:
//    _detector = YAGPhotodiode;break;
//  case PostProcessors::FsPhotodiodeNbrSignals:
//    _detector = FsPhotodiode;break;
//  default:
//    throw std::invalid_argument(QString("postprocessor %1 is not for Nbr MCP Signals").arg(_id).toStdString());
//  }
//  //create the histogram by loading the settings//
//  loadSettings(0);
//}
//
//cass::pp550::~pp550()
//{
//  _pp.histograms_delete(_id);
//  _nbrSignals=0;
//}
//
//void cass::pp550::loadSettings(size_t)
//{
//  using namespace cass::ACQIRIS;
//  std::cout <<std::endl<< "load the parameters of postprocessor "<<_id
//      <<" it histograms the Nbr of Mcp Peaks"
//      <<" of detector "<<_detector
//      <<std::endl;
//
//  //create the histogram
//  set1DHist(_nbrSignals,_id);
//  _pp.histograms_replace(_id,_nbrSignals);
//  //load the detectors settings
//  HelperAcqirisDetectors::instance(_detector)->loadSettings();
//  std::cout << "done loading postprocessor "<<_id<<"'s parameters"<<std::endl;
//}
//
//void cass::pp550::operator()(const cass::CASSEvent &evt)
//{
//  using namespace cass::ACQIRIS;
//  //get right filled detector from the helper
//  TofDetector *det =
//      dynamic_cast<TofDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt));
//  _nbrSignals->lock.lockForWrite();
//  _nbrSignals->fill(det->mcp().peaks().size());
//  _nbrSignals->lock.unlock();
//}
//
//
//
//
//
////----------------Nbr of Peaks Anode-------------------------------------------
////-----------pp551 - pp556 & pp601 - 604---------------------------------------
//cass::pp551::pp551(PostProcessors &pp, PostProcessors::id_t id)
//  :cass::PostprocessorBackend(pp,id),
//  _nbrSignals(0)
//{
//  using namespace cass::ACQIRIS;
//  //find out which detector and Signal we should work on
//  switch (_id)
//  {
//  case PostProcessors::HexU1NbrSignals:
//    _detector = HexDetector; _layer = 'U'; _signal = '1';break;
//  case PostProcessors::HexU2NbrSignals:
//    _detector = HexDetector; _layer = 'U'; _signal = '2';break;
//  case PostProcessors::HexV1NbrSignals:
//    _detector = HexDetector; _layer = 'V'; _signal = '1';break;
//  case PostProcessors::HexV2NbrSignals:
//    _detector = HexDetector; _layer = 'V'; _signal = '2';break;
//  case PostProcessors::HexW1NbrSignals:
//    _detector = HexDetector; _layer = 'W'; _signal = '1';break;
//  case PostProcessors::HexW2NbrSignals:
//    _detector = HexDetector; _layer = 'W'; _signal = '2';break;
//
//  case PostProcessors::QuadX1NbrSignals:
//    _detector = QuadDetector; _layer = 'X'; _signal = '1';break;
//  case PostProcessors::QuadX2NbrSignals:
//    _detector = QuadDetector; _layer = 'X'; _signal = '2';break;
//  case PostProcessors::QuadY1NbrSignals:
//    _detector = QuadDetector; _layer = 'Y'; _signal = '1';break;
//  case PostProcessors::QuadY2NbrSignals:
//    _detector = QuadDetector; _layer = 'Y'; _signal = '2';break;
//
//  default:
//    throw std::invalid_argument(QString("postprocessor %1 is not responsible for Nbr Anode Signals").arg(id).toStdString());
//  }
//  //create the histogram by loading the settings//
//  loadSettings(0);
//}
//
//cass::pp551::~pp551()
//{
//  _pp.histograms_delete(_id);
//  _nbrSignals=0;
//}
//
//void cass::pp551::loadSettings(size_t)
//{
//  using namespace cass::ACQIRIS;
//  std::cout <<std::endl<< "load the parameters of postprocessor "<<_id
//      <<" it histograms the Nbr of Anode Layer Peaks "
//      <<" of detector "<<_detector
//      <<" layer "<<_layer
//      <<" wireend "<<_signal
//      <<std::endl;
//
//  //create the histogram
//  set1DHist(_nbrSignals,_id);
//  _pp.histograms_replace(_id,_nbrSignals);
//  //load the detectors settings
//  HelperAcqirisDetectors::instance(_detector)->loadSettings();
//  std::cout << "done loading postprocessor "<<_id<<"'s parameters"<<std::endl;
//}
//
//void cass::pp551::operator()(const cass::CASSEvent &evt)
//{
//  using namespace cass::ACQIRIS;
//  //get right filled detector from the helper
//  DelaylineDetector *det =
//      dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt));
//  _nbrSignals->lock.lockForWrite();
//  _nbrSignals->fill(det->layers()[_layer].wireend()[_signal].peaks().size());
//  _nbrSignals->lock.unlock();
//}
//
//
//
//
//
//
////----------------Ratio of Layers----------------------------------------------
////-----------pp557, pp560, pp563, pp605, pp608---------------------------------
//cass::pp557::pp557(PostProcessors &pp, PostProcessors::id_t id)
//  :cass::PostprocessorBackend(pp,id),
//  _ratio(0)
//{
//  using namespace cass::ACQIRIS;
//
//  //find out which detector and Signal we should work on
//  switch (_id)
//  {
//  case PostProcessors::HexU1U2Ratio:
//    _detector = HexDetector; _layer = 'U';break;
//  case PostProcessors::HexV1V2Ratio:
//    _detector = HexDetector; _layer = 'V';break;
//  case PostProcessors::HexW1W2Ratio:
//    _detector = HexDetector; _layer = 'W';break;
//
//  case PostProcessors::QuadX1X2Ratio:
//    _detector = QuadDetector; _layer = 'X';break;
//  case PostProcessors::QuadY1Y2Ratio:
//    _detector = QuadDetector; _layer = 'X';break;
//
//  default:
//    throw std::invalid_argument(QString("postprocessor %1 is not responsible for Ratio of Layers").arg(id).toStdString());
//  }
//  //create the histogram by loading the settings//
//  loadSettings(0);
//}
//
//cass::pp557::~pp557()
//{
//  _pp.histograms_delete(_id);
//  _ratio=0;
//}
//
//void cass::pp557::loadSettings(size_t)
//{
//  using namespace cass::ACQIRIS;
//
//  std::cout <<std::endl<< "load the parameters of postprocessor "<<_id
//      <<" it histograms the Ratio of Anode Layer Peaks"
//      <<" of detector "<<_detector
//      <<" layer "<<_layer
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
//void cass::pp557::operator()(const cass::CASSEvent &evt)
//{
//  using namespace cass::ACQIRIS;
//  //get right filled detector from the helper
//  DelaylineDetector *det =
//      dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt));
//  const float one = det->layers()[_layer].wireend()['1'].peaks().size();
//  const float two = det->layers()[_layer].wireend()['2'].peaks().size();
//  _ratio->lock.lockForWrite();
//  _ratio->fill(one/two);
//  _ratio->lock.unlock();
//}
//
//
//
//
//
//
//
////----------------Ratio of Signals vs. MCP-------------------------------------
////-----------pp558-559, pp561-562, pp564-565, pp606-607, pp609-610-------------
//cass::pp558::pp558(PostProcessors &pp, PostProcessors::id_t id)
//  :cass::PostprocessorBackend(pp,id),
//  _ratio(0)
//{
//  using namespace cass::ACQIRIS;
//
//  //find out which detector and Signal we should work on
//  switch (_id)
//  {
//  case PostProcessors::HexU1McpRatio:
//    _detector = HexDetector; _layer = 'U'; _wireend = '1';break;
//  case PostProcessors::HexU2McpRatio:
//    _detector = HexDetector; _layer = 'U'; _wireend = '2';break;
//  case PostProcessors::HexV1McpRatio:
//    _detector = HexDetector; _layer = 'V'; _wireend = '1';break;
//  case PostProcessors::HexV2McpRatio:
//    _detector = HexDetector; _layer = 'V'; _wireend = '2';break;
//  case PostProcessors::HexW1McpRatio:
//    _detector = HexDetector; _layer = 'W'; _wireend = '1';break;
//  case PostProcessors::HexW2McpRatio:
//    _detector = HexDetector; _layer = 'W'; _wireend = '2';break;
//
//  case PostProcessors::QuadX1McpRatio:
//    _detector = QuadDetector; _layer = 'X'; _wireend = '1';break;
//  case PostProcessors::QuadX2McpRatio:
//    _detector = QuadDetector; _layer = 'X'; _wireend = '2';break;
//  case PostProcessors::QuadY1McpRatio:
//    _detector = QuadDetector; _layer = 'Y'; _wireend = '1';break;
//  case PostProcessors::QuadY2McpRatio:
//    _detector = QuadDetector; _layer = 'Y'; _wireend = '2';break;
//
//  default:
//    throw std::invalid_argument(QString("postprocessor %1 is not responsible for Ratio of Signals vs. MCP").arg(id).toStdString());
//  }
//  //create the histogram by loading the settings//
//  loadSettings(0);
//}
//
//cass::pp558::~pp558()
//{
//  _pp.histograms_delete(_id);
//  _ratio=0;
//}
//
//void cass::pp558::loadSettings(size_t)
//{
//  using namespace cass::ACQIRIS;
//
//  std::cout <<std::endl<< "load the parameters of postprocessor "<<_id
//      <<" it histograms the Ratio of Anode Layer wireend Peaks vs Mcp Peaks "<<_layer
//      <<" of detector "<<_detector
//      << "layer "<<_layer
//      <<" wireend "<<_wireend
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
//void cass::pp558::operator()(const cass::CASSEvent &evt)
//{
//  using namespace cass::ACQIRIS;
//  //get right filled detector from the helper
//  DelaylineDetector *det =
//      dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt));
//  const float wireend = det->layers()[_layer].wireend()[_wireend].peaks().size();
//  const float mcp = det->mcp().peaks().size();
//  _ratio->lock.lockForWrite();
//  _ratio->fill(wireend/mcp);
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
////----------------MCP Fwhm vs. height------------------------------------------
////-------------pp581, pp621, pp652, pp662, pp672, pp682------------------------
//cass::pp581::pp581(PostProcessors &pp, PostProcessors::id_t id)
//  :cass::PostprocessorBackend(pp,id),
//  _sigprop(0)
//{
//  using namespace cass::ACQIRIS;
//
//  //find out which detector and Signal we should work on
//  switch (_id)
//  {
//  case PostProcessors::HexHeightvsFwhmMcp:
//    _detector = HexDetector;break;
//  case PostProcessors::QuadHeightvsFwhmMcp:
//    _detector = QuadDetector;break;
//  case PostProcessors::VMIMcpHeightvsFwhmMcp:
//    _detector = VMIMcp;break;
//  case PostProcessors::FELBeamMonitorHeightvsFwhmMcp:
//    _detector = FELBeamMonitor;break;
//  case PostProcessors::YAGPhotodiodeHeightvsFwhmMcp:
//    _detector = YAGPhotodiode;break;
//  case PostProcessors::FsPhotodiodeHeightvsFwhmMcp:
//    _detector = FsPhotodiode;break;
//
//  default:
//    throw std::invalid_argument(QString("postprocessor %1 is not responsible for FWHM vs. Height of MCP").arg(id).toStdString());
//  }
//  //create the histogram by loading the settings//
//  loadSettings(0);
//}
//
//cass::pp581::~pp581()
//{
//  _pp.histograms_delete(_id);
//  _sigprop=0;
//}
//
//void cass::pp581::loadSettings(size_t)
//{
//  using namespace cass::ACQIRIS;
//
//  std::cout <<std::endl<< "load the parameters of postprocessor "<<_id
//      <<" it histograms the FWHM vs the height of the found MCP Peaks"
//      <<" of  detector "<<_detector
//      <<std::endl;
//
//  //create the histogram
//  set2DHist(_sigprop,_id);
//  _pp.histograms_replace(_id,_sigprop);
//  //load the detectors settings
//  HelperAcqirisDetectors::instance(_detector)->loadSettings();
//  std::cout << "done loading postprocessor "<<_id<<"'s parameters"<<std::endl;
//}
//
//void cass::pp581::operator()(const cass::CASSEvent &evt)
//{
//  using namespace cass::ACQIRIS;
//  //get right filled detector from the helper
//  TofDetector *det =
//      dynamic_cast<TofDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt));
//  //reference to all found peaks of the mcp channel//
//  Signal::peaks_t::const_iterator it = det->mcp().peaks().begin();
//  //fill all found peaks into the histogram//
////  std::cout<<det->mcp().peaks().size()<<std::endl;
//  _sigprop->lock.lockForWrite();
//  for (;it != det->mcp().peaks().end(); ++it)
//    _sigprop->fill(it->fwhm(),it->height());
//  _sigprop->lock.unlock();
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
////----------------FWHM vs. Height of Wireend Signals---------------------------
////-----------pp582-587, pp622-625----------------------------------------------
//cass::pp582::pp582(PostProcessors &pp, PostProcessors::id_t id)
//  :cass::PostprocessorBackend(pp,id),
//  _sigprop(0)
//{
//  using namespace cass::ACQIRIS;
//
//  //find out which detector and Signal we should work on
//  switch (_id)
//  {
//  case PostProcessors::HexHeightvsFwhmU1:
//    _detector = HexDetector; _layer = 'U'; _signal = '1';break;
//  case PostProcessors::HexHeightvsFwhmU2:
//    _detector = HexDetector; _layer = 'U'; _signal = '2';break;
//  case PostProcessors::HexHeightvsFwhmV1:
//    _detector = HexDetector; _layer = 'V'; _signal = '1';break;
//  case PostProcessors::HexHeightvsFwhmV2:
//    _detector = HexDetector; _layer = 'V'; _signal = '2';break;
//  case PostProcessors::HexHeightvsFwhmW1:
//    _detector = HexDetector; _layer = 'W'; _signal = '1';break;
//  case PostProcessors::HexHeightvsFwhmW2:
//    _detector = HexDetector; _layer = 'W'; _signal = '2';break;
//
//  case PostProcessors::QuadHeightvsFwhmX1:
//    _detector = QuadDetector; _layer = 'X'; _signal = '1';break;
//  case PostProcessors::QuadHeightvsFwhmX2:
//    _detector = QuadDetector; _layer = 'X'; _signal = '2';break;
//  case PostProcessors::QuadHeightvsFwhmY1:
//    _detector = QuadDetector; _layer = 'Y'; _signal = '1';break;
//  case PostProcessors::QuadHeightvsFwhmY2:
//    _detector = QuadDetector; _layer = 'Y'; _signal = '2';break;
//
//  default:
//    throw std::invalid_argument(QString("postprocessor %1 is not responsible for FWHM vs. Height of Layer Signals").arg(id).toStdString());
//  }
//  //create the histogram by loading the settings//
//  loadSettings(0);
//}
//
//cass::pp582::~pp582()
//{
//  _pp.histograms_delete(_id);
//  _sigprop=0;
//}
//
//void cass::pp582::loadSettings(size_t)
//{
//  using namespace cass::ACQIRIS;
//
//  std::cout <<std::endl<< "load the parameters of postprocessor "<<_id
//      <<" it histograms the FWHM vs the height of layer "<<_layer
//      << " wireend "<<_signal
//      <<" of detector "<<_detector
//      <<std::endl;
//  //create the histogram
//  set2DHist(_sigprop,_id);
//  _pp.histograms_replace(_id,_sigprop);
//    //load the detectors settings
//  HelperAcqirisDetectors::instance(_detector)->loadSettings();
//  std::cout << "done loading postprocessor "<<_id<<"'s parameters"<<std::endl;
//}
//
//void cass::pp582::operator()(const cass::CASSEvent &evt)
//{
//  using namespace cass::ACQIRIS;
//  //get right filled detector from the helper
//  DelaylineDetector *det =
//      dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt));
//  //go through all peaks of the wireend//
//  //reference to all found peaks of the wireend channel//
//  Signal::peaks_t::const_iterator it = det->layers()[_layer].wireend()[_signal].peaks().begin();
//  //fill all found peaks into the histogram//
//  _sigprop->lock.lockForWrite();
//  for (; it != det->layers()[_layer].wireend()[_signal].peaks().end(); ++it)
//    _sigprop->fill(it->fwhm(),it->height());
//  _sigprop->lock.unlock();
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
