// Copyright (C) 2010 Lutz Foucar

#include <algorithm>
#include <cassert>
#include <functional>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <math.h>

#include <QtCore/QSettings>
#include <QtCore/QString>

#include "ccd_device.h"
#include "pnccd_device.h"
#include "histogram.h"
#include "cass_event.h"
#include "postprocessor.h"
#include "imaging.h"
#include "acqiris_detectors_helper.h"
#include "tof_detector.h"












// *** postprocessors 160 advanced photon finding ***

cass::pp160::pp160(PostProcessors& pp, cass::PostProcessors::id_t id)
  : PostprocessorBackend(pp, id),
  _idAverage(cass::PostProcessors::FirstPnccdFrontBinnedConditionalRunningAverage),
  _image(0)
{
  loadSettings(0);
}


cass::pp160::~pp160()
{
  _pp.histograms_delete(_id);
  _image = 0;
}

std::list<cass::PostProcessors::id_t> cass::pp160::dependencies()
{
  std::list<PostProcessors::id_t> list;
  list.push_front(_idAverage);
  return list;
}

void cass::pp160::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;

  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString("p") + QString::number(_id));

  _threshold = settings.value("Threshold",1.).toFloat();


  std::string name(settings.value("ConditionDetector","YAGPhotodiode").toString().toStdString());
  if (name=="YAGPhotodiode")
    _conditionDetector = YAGPhotodiode;
  else if (name=="HexDetector")
    _conditionDetector = HexDetector;
  else if (name=="QuadDetector")
    _conditionDetector = QuadDetector;
  else if (name=="VMIMcp")
    _conditionDetector = VMIMcp;
  else if (name=="FELBeamMonitor")
    _conditionDetector = FELBeamMonitor;
  else if (name=="FsPhotodiode")
    _conditionDetector = FsPhotodiode;
  else
    _conditionDetector = InvalidDetector;

  if (_conditionDetector)
    HelperAcqirisDetectors::instance(_conditionDetector)->loadSettings();


  try
  {
    _pp.validate(_idAverage);
  }
  catch (InvalidHistogramError)
  {
    _reinitialize = true;
    return;
  }


  const PostProcessors::histograms_t container (_pp.histograms_checkout());
  PostProcessors::histograms_t::const_iterator it (container.find(_idAverage));
  _pp.histograms_release();

  _pp.histograms_delete(_id);
  _image = new Histogram2DFloat(it->second->axis()[HistogramBackend::xAxis].size(),
                                it->second->axis()[HistogramBackend::yAxis].size());
  _pp.histograms_replace(_id,_image);
}



void cass::pp160::operator()(const CASSEvent& event)
{
  using namespace cass::ACQIRIS;
  using namespace std;

  bool update(true);
  if (_conditionDetector != InvalidDetector)
  {
    TofDetector *det =
        dynamic_cast<TofDetector*>(HelperAcqirisDetectors::instance(_conditionDetector)->detector(event));
    update = det->mcp().peaks().size();
  }

  if (update)
  {
    //check whether detector exists
    if (event.devices().find(CASSEvent::pnCCD)->second->detectors()->size() <= 0)
      throw std::runtime_error(QString("PostProcessor_%1: front pnccd detector does not exist").arg(_id).toStdString());

    const PixelDetector &det((*event.devices().find(CASSEvent::pnCCD)->second->detectors())[0]);
    const PixelDetector::frame_t& frame(det.frame());

    const PostProcessors::histograms_t container (_pp.histograms_checkout());
    PostProcessors::histograms_t::const_iterator f(container.find(_idAverage));
    HistogramFloatBase::storage_t average (dynamic_cast<Histogram2DFloat *>(f->second)->memory());
    _pp.histograms_release();

    f->second->lock.lockForRead();
    const float alpha = accumulate(frame.begin(),frame.end(),0) /  accumulate(average.begin(),average.end(),0);
    std::cout<< "pp160: alpha:"<<alpha<<std::endl;

    _image->lock.lockForWrite();

    PixelDetector::frame_t::const_iterator frIt(frame.begin()) ;
    HistogramFloatBase::storage_t::const_iterator avIt(average.begin());
    HistogramFloatBase::storage_t::iterator imIt(_image->memory().begin());
    for (;avIt != average.end() ;++avIt)
    {
      *imIt++ += ( *frIt++ - alpha * *avIt++ > _threshold);
//      std::cout<< "pp160: val:"<< *frIt++ - alpha * *avIt++<<std::endl;
    }


    _image->lock.unlock();
    f->second->lock.unlock();
  }
}

















