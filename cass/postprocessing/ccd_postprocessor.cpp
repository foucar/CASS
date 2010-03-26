// Copyright (C) 2010 lmf
// Copyright (C) 2010 Jochen Küpper

#include <utility>
#include <algorithm>
#include <cassert>
#include <stdexcept>

#include <QtCore/QSettings>
#include <QtCore/QString>

#include "ccd_postprocessor.h"
#include "pnccd_device.h"
#include "histogram.h"
#include "post_processor.h"
#include "cass_event.h"



cass::PostprocessorPnccdLastImage::PostprocessorPnccdLastImage(cass::PostProcessors::histograms_t& hist, cass::PostProcessors::id_t id)
: PostprocessorBackend(hist, id),
_image(new Histogram2DFloat(1024, 0, 1023, 1024, 0, 1023))
{
  switch(id) 
  {
  case PostProcessors::Pnccd1LastImage:
    _detector = 0;
    break;
  case PostProcessors::Pnccd2LastImage:
    _detector = 1;
    break;
  default:
    throw std::invalid_argument("Impossible postprocessor id for PostprocessorPnccdLastImage");
  };
  // save storage in PostProcessors container
  assert(hist == _histograms);
  _histograms[_id] = _image;

}


cass::PostprocessorPnccdLastImage::~PostprocessorPnccdLastImage()
{ 
  delete _image; 
  _image = 0; 
}


void cass::PostprocessorPnccdLastImage::operator()(const cass::CASSEvent& event)
{
  using namespace cass::pnCCD;
  const pnCCDDevice *dev(dynamic_cast<const pnCCDDevice *>(event.devices().find(cass::CASSEvent::pnCCD)->second));
  const CCDDetector::frame_t& frame(dev->detectors()[_detector].frame());
  std::copy(frame.begin(), frame.end(), _image->memory().begin());
}





cass::PostprocessorPnccdBinnedRunningAverage::PostprocessorPnccdBinnedRunningAverage(
  cass::PostProcessors::histograms_t& hist, cass::PostProcessors::id_t id)
  : cass::PostprocessorBackend(hist, id)
{
  loadSettings(0);
  // _image(new Histogram2DFloat(1024, 0, 1023, 1024, 0, 1023))
  //     {
  //         switch(id) {
  //         case PostProcessors::PnccdLastImage1:
  //             _detector = 0;
  //             break;
  //         case PostProcessors::PnccdLastImage2:
  //             _detector = 1;
  //             break;
  //         };
  //         // save storage in PostProcessors container
  //         assert(hist == _histograms);
  //         _histograms[_id] = _image;
  //     };
}


cass::PostprocessorPnccdBinnedRunningAverage::~PostprocessorPnccdBinnedRunningAverage()
{
  delete _image; 
  _image = 0; 
}


void cass::PostprocessorPnccdBinnedRunningAverage::loadSettings(size_t)
{
  QSettings settings;
  settings.beginGroup("postprocessors");
  settings.beginGroup(QString("processor_") + QString::number(_id));
  _binning = std::make_pair(settings.value("bin_vertical").toUInt(), settings.value("bin_vertical").toUInt());
}



void cass::PostprocessorPnccdBinnedRunningAverage::operator()(const CASSEvent& event)
{
  using namespace cass::pnCCD;
  const pnCCDDevice *dev(dynamic_cast<const pnCCDDevice *>(event.devices().find(cass::CASSEvent::pnCCD)->second));
  const CCDDetector::frame_t& frame(dev->detectors()[_detector].frame());
  std::copy(frame.begin(), frame.end(), _image->memory().begin());
}




// void PostprocessorAveragePnCCD::operator()(const CASSEvent&)
// {
// /*
// pnCCD::pnCCDDevice &dev(*(dynamic_cast<pnCCDDevice *>(event->devices()[cass::CASSEvent::pnCCD])));
// CCDDetector::frame_t &frame(dev.detectors()[0].frame());
// for(HistogramFloatBasehisto_t::iterator h=_backend->memory().begin(),
// CCDDetector::frame_t::iterator f=frame.begin(); f != frame.end(); ++f, ++h)
// *h = 0.5 * *h + 0.5 * *f;
// */
// }
