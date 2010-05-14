// Copyright (C) 2010 Lutz Foucar
// Copyright (C) 2010 Jochen Küpper

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
#include "postprocessing/postprocessor.h"
#include "postprocessing/ccd.h"
#include "acqiris_detectors_helper.h"
#include "tof_detector.h"


namespace cass
{


// *** postprocessors 1, 2, 3 -- last images from a CCD ***

pp1::pp1(PostProcessors& pp, cass::PostProcessors::id_t id)
    : PostprocessorBackend(pp, id)
{
    int cols(0);
    int rows(0);
    switch(id)
    {
    case PostProcessors::Pnccd1LastImage:
        _device=CASSEvent::pnCCD; _detector = 0; cols = pnCCD::default_size; rows = pnCCD::default_size;
        break;
    case PostProcessors::Pnccd2LastImage:
        _device=CASSEvent::pnCCD; _detector = 1; cols = pnCCD::default_size; rows = pnCCD::default_size;
        break;
    case PostProcessors::VmiCcdLastImage:
        _device=CASSEvent::CCD; _detector = 0; cols = CCD::opal_default_size; rows = CCD::opal_default_size;
        break;

    default:
        throw std::invalid_argument("class not responsible for requested postprocessor");
    };
    std::cout<<"Postprocessor_"<<_id<<": set up: cols:"<<cols<<" rows:"<<rows<<std::endl;
    // save storage in PostProcessors container
    _image = new Histogram2DFloat(cols, 0, cols-1, rows, 0, rows-1);
    //_image->setMimeType(std::string("application/image"));     // in future, default mime-type of 2d histograms is 2d histogram, not image. Mime type for individual postprocessors can be specialized like this.
    _pp.histograms_replace(_id, _image);
    VERBOSEOUT(std::cout<<"Postprocessor_"<<_id<<"done."<<std::endl);
}


pp1::~pp1()
{
    _pp.histograms_delete(_id);
    _image = 0;
}


void pp1::operator()(const cass::CASSEvent& event)
{
    //check whether detector exists
    // std::cout<<"BLA"<< event.devices().find(_device)->second->detectors()->size() << " "<< _detector <<std::endl;
    if (event.devices().find(_device)->second->detectors()->size() <= _detector)
        throw std::runtime_error(QString("PostProcessor_%1: Detector %2 does not exist in Device %3").arg(_id).arg(_detector).arg(_device).toStdString());

    //get frame and fill image//
    const PixelDetector::frame_t& frame
        ((*(event.devices().find(_device)->second)->detectors())[_detector].frame());
    _image->lock.lockForWrite();
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
    */
    const PixelDetector &det((*event.devices().find(_device)->second->detectors())[_detector]);

    const cass::ROI::ROIiterator_t& ROIiterator_pp(det.ROIiterator_pp());
    /*std::cout<< "cacca " << ROIiterator_pp.size()
      <<std::endl;*/



    std::copy(frame.begin(), frame.end(), _image->memory().begin());
    _image->lock.unlock();
}











// *** postprocessors 101, 103, 105 ***

pp101::pp101(PostProcessors& pp, cass::PostProcessors::id_t id)
    : PostprocessorBackend(pp, id),
      _scale(1.), _binning(std::make_pair(1, 1)), _image(0)
{
    loadSettings(0);
    switch(id)
    {
    case PostProcessors::FirstPnccdFrontBinnedConditionalRunningAverage:
    case PostProcessors::SecondPnccdFrontBinnedConditionalRunningAverage:
        _detector = 0; _device=CASSEvent::pnCCD;
        break;
    case PostProcessors::FirstPnccdBackBinnedConditionalRunningAverage:
    case PostProcessors::SecondPnccdBackBinnedConditionalRunningAverage:
        _detector = 1; _device=CASSEvent::pnCCD;
        break;
    case PostProcessors::FirstCommercialCCDBinnedConditionalRunningAverage:
    case PostProcessors::SecondCommercialCCDBinnedConditionalRunningAverage:
        _detector = 0; _device=CASSEvent::CCD;
        break;
    default:
        throw std::invalid_argument("Impossible postprocessor id for pp101");
        break;
    };
}


cass::pp101::~pp101()
{
    _pp.histograms_delete(_id);
    _image = 0;
}


void cass::pp101::loadSettings(size_t)
{
    using namespace cass::ACQIRIS;
    int cols(0); int rows(0);
    switch(_id)
    {
    case PostProcessors::FirstPnccdFrontBinnedConditionalRunningAverage:
    case PostProcessors::SecondPnccdFrontBinnedConditionalRunningAverage:
    case PostProcessors::FirstPnccdBackBinnedConditionalRunningAverage:
    case PostProcessors::SecondPnccdBackBinnedConditionalRunningAverage:
        cols = pnCCD::default_size; rows = pnCCD::default_size;
        break;
    case PostProcessors::FirstCommercialCCDBinnedConditionalRunningAverage:
    case PostProcessors::SecondCommercialCCDBinnedConditionalRunningAverage:
        cols = CCD::opal_default_size; rows = CCD::opal_default_size;
        break;
    default:
        throw std::invalid_argument("Impossible postprocessor id for pp101");
        break;
    };
    QSettings settings;
    settings.beginGroup("PostProcessor");
    settings.beginGroup(QString("p") + QString::number(_id));
    _average = settings.value("average", 1).toUInt();
    _scale =  2./(_average+1);
    std::pair<unsigned, unsigned> binning(std::make_pair(settings.value("bin_horizontal", 1).toUInt(),
                                                         settings.value("bin_vertical", 1).toUInt()));
    std::string name(settings.value("ConditionDetector","InvalidDetector").toString().toStdString());
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

    _invert = settings.value("Invert",false).toBool();

    std::cout<<"Postprocessor_"<<_id<<": alpha for the averaging:"<<_scale<<" average:"<<_average
        <<" condition on detector:"<<name
        <<" which has id:"<<_conditionDetector
        <<" The Condition will be inverted:"<<std::boolalpha<<_invert
        <<std::endl;

    if (_conditionDetector)
      HelperAcqirisDetectors::instance(_conditionDetector)->loadSettings();

    _pp.histograms_delete(_id);
    _image = new Histogram2DFloat(cols,rows);
    _pp.histograms_replace(_id,_image);

    _firsttime = true;
}



void cass::pp101::operator()(const CASSEvent& event)
{
    using namespace cass::ACQIRIS;

    //check whether detector exists
    if (event.devices().find(_device)->second->detectors()->size() <= _detector)
        throw std::runtime_error(QString("PostProcessor_%1: Detector %2 does not exist in Device %3").arg(_id).arg(_detector).arg(_device).toStdString());

    const PixelDetector &det((*event.devices().find(_device)->second->detectors())[_detector]);
    const PixelDetector::frame_t& frame(det.frame());

    //find out whether we should update//
    bool update(true);
    if (_conditionDetector != InvalidDetector)
    {
      TofDetector *det =
          dynamic_cast<TofDetector*>(HelperAcqirisDetectors::instance(_conditionDetector)->detector(event));
      update = det->mcp().peaks().size();
      update ^= _invert;
    }
    // running average of data:
    _image->lock.lockForWrite();
    if (update)
    {
      float scale = (_firsttime)?1:_scale;
      if (_firsttime) _firsttime = false;

      transform(frame.begin(),frame.end(),
                _image->memory().begin(),
                _image->memory().begin(),
                Average(scale));
    }
    _image->lock.unlock();
}












/** binary function for weighted substracting.
 *
 * @author Lutz Foucar
 */
class weighted_minus : std::binary_function<float, float, float>
{
public:
  /** constructor.
   *
   * @param first_weight the weight value of the first substrant
   * @param second_weight the weight value of the second substrant
   */
  weighted_minus(float first_weight, float second_weight)
    :_first_weight(first_weight),_second_weight(second_weight)
  {}
  /** operator.*/
  float operator() (float first, float second)
  { return first * _first_weight - second * _second_weight;}
protected:
  float _first_weight, _second_weight;
};










// *** postprocessors 106 substract two histograms ***

pp106::pp106(PostProcessors& pp, cass::PostProcessors::id_t id)
    : PostprocessorBackend(pp, id), _image(0)
{
    loadSettings(0);
}


cass::pp106::~pp106()
{
    _pp.histograms_delete(_id);
    _image = 0;
}

std::list<PostProcessors::id_t> pp106::dependencies()
{
    std::list<PostProcessors::id_t> list;
    list.push_front(_idOne);
    list.push_front(_idTwo);
    return list;
}

void cass::pp106::loadSettings(size_t)
{
    QSettings settings;
    settings.beginGroup("PostProcessor");
    settings.beginGroup(QString("p") + QString::number(_id));

    _fOne = settings.value("FactorOne",1.).toFloat();
    _fTwo = settings.value("FactorTwo",1.).toFloat();

    _idOne = static_cast<PostProcessors::id_t>(settings.value("HistOne",0).toInt());
    _idTwo = static_cast<PostProcessors::id_t>(settings.value("HistTwo",0).toInt());

    try
    {
      _pp.validate(_idOne);
    }
    catch (InvalidHistogramError)
    {
      _reinitialize = true;
      return;
    }

    try
    {
      _pp.validate(_idTwo);
    }
    catch (InvalidHistogramError)
    {
      _reinitialize = true;
      return;
    }

    const PostProcessors::histograms_t container (_pp.histograms_checkout());
    PostProcessors::histograms_t::const_iterator it (container.find(_idOne));
    _pp.histograms_release();

    _pp.histograms_delete(_id);
    _image = new Histogram2DFloat(it->second->axis()[HistogramBackend::xAxis].size(),
                                  it->second->axis()[HistogramBackend::yAxis].size());
    _pp.histograms_replace(_id,_image);

    std::cout << "postprocessor_"<<_id<< " will substract hist "<<_idOne
        <<" from hist "<<_idTwo
        <<std::endl;
}



void cass::pp106::operator()(const CASSEvent&)
{
  const PostProcessors::histograms_t container (_pp.histograms_checkout());
  PostProcessors::histograms_t::const_iterator f(container.find(_idOne));
  HistogramFloatBase::storage_t first (dynamic_cast<Histogram2DFloat *>(f->second)->memory());
  PostProcessors::histograms_t::const_iterator s(container.find(_idTwo));
  HistogramFloatBase::storage_t second (dynamic_cast<Histogram2DFloat *>(s->second)->memory());
  _pp.histograms_release();

  f->second->lock.lockForRead();
  s->second->lock.lockForRead();
  _image->lock.lockForWrite();
  std::transform(first.begin(), first.end(),
                 second.begin(),
                 _image->memory().begin(),
                 weighted_minus(_fOne,_fTwo));
  _image->lock.unlock();
  s->second->lock.unlock();
  f->second->lock.unlock();
}

















// *** A Postprocessor that will display the photonhits of ccd detectors***
// ***  used by postprocessors 110-112 ***

pp110::pp110(PostProcessors& pp, cass::PostProcessors::id_t id)
    : PostprocessorBackend(pp, id)
{
    switch(id)
    {
    case PostProcessors::VMIPhotonHits:
        _device=CASSEvent::CCD; _detector = 0;
        break;
    case PostProcessors::PnCCDFrontPhotonHits:
        _device=CASSEvent::pnCCD; _detector = 0;
        break;
    case PostProcessors::PnCCDBackPhotonHits:
        _device=CASSEvent::pnCCD; _detector = 1;
        break;
    default:
        throw std::invalid_argument("Impossible postprocessor id for class pp110");
        break;
    };
    loadSettings(0);
}

cass::pp110::~pp110()
{
    _pp.histograms_delete(_id);
    _image = 0;
}

void cass::pp110::loadSettings(size_t)
{
  std::cout <<std::endl<< "load the parameters of postprocessor "<<_id
      <<" it histograms the Nbr of Mcp Peaks"
      <<" of detector "<<_detector
      <<" of device "<<_device
      <<std::endl;
  //create the histogram
  set2DHist(_image,_id);
  _pp.histograms_replace(_id,_image);
}

void cass::pp110::operator()(const CASSEvent& evt)
{
    //check whether detector exists
    if (evt.devices().find(_device)->second->detectors()->size() <= _detector)
        throw std::runtime_error(QString("PostProcessor_%1: Detector %2 does not exist in Device %3").arg(_id).arg(_detector).arg(_device).toStdString());
    //retrieve the detector's photon hits of the device we are working for.
    const PixelDetector::pixelList_t& pixellist
        ((*(evt.devices().find(_device)->second)->detectors())[_detector].pixellist());
    PixelDetector::pixelList_t::const_iterator it(pixellist.begin());
    _image->lock.lockForWrite();
    for (; it != pixellist.end();++it)
        _image->fill(it->x(),it->y());
    _image->lock.unlock();
}






















// *** A Postprocessor that will display the photonhits of ccd detectors in 1D hist***
// ***  used by postprocessors 113-115 ***

pp113::pp113(PostProcessors& pp, cass::PostProcessors::id_t id)
    : PostprocessorBackend(pp, id), _hist(0)
{
    switch(id)
    {
    case PostProcessors::VMIPhotonHits1d:
        _device=CASSEvent::CCD; _detector = 0;
        break;
    case PostProcessors::PnCCDFrontPhotonHits1d:
        _device=CASSEvent::pnCCD; _detector = 0;
        break;
    case PostProcessors::PnCCDBackPhotonHits1d:
        _device=CASSEvent::pnCCD; _detector = 1;
        break;
    default:
        throw std::invalid_argument("Impossible postprocessor id for pp113");
        break;
    };
    loadSettings(0);
}

cass::pp113::~pp113()
{
    _pp.histograms_delete(_id);
    _hist = 0;
}

void cass::pp113::loadSettings(size_t)
{
  std::cout <<std::endl<< "load the parameters of postprocessor "<<_id
      <<" it histograms the Nbr of Mcp Peaks"
      <<" of detector "<<_detector
      <<" of device "<<_device
      <<std::endl;
  //create the histogram
  _pp.histograms_delete(_id);
  _hist=0;
  set1DHist(_hist,_id);
  _pp.histograms_replace(_id,_hist);
}

void cass::pp113::operator()(const CASSEvent& evt)
{
//    std::cout << "pp113::operator():"<<std::endl;
    //check whether detector exists
    if (evt.devices().find(_device)->second->detectors()->size() <= _detector)
        throw std::runtime_error(QString("PostProcessor_%1: Detector %2 does not exist in Device %3").arg(_id).arg(_detector).arg(_device).toStdString());

    //retrieve the detector's photon hits of the device we are working for.
    const PixelDetector::pixelList_t& pixellist
        ((*(evt.devices().find(_device)->second)->detectors())[_detector].pixellist());
    PixelDetector::pixelList_t::const_iterator it(pixellist.begin());
//    std::cout<<"pp113::operator(): pixellist size:"<<pixellist.size()<<std::endl;
    _hist->lock.lockForWrite();
    for (; it != pixellist.end();++it)
        _hist->fill(it->z());
    _hist->lock.unlock();
//    std::cout << "pp113::operator(): going out"<<std::endl;
}
















// *** A Postprocessor that will display the photonhits of ccd detectors in 1D hist***
// *** energies in eV 
// ***  used by postprocessors 116-118 ***

pp116::pp116(PostProcessors& pp, cass::PostProcessors::id_t id)
    : PostprocessorBackend(pp, id), _hist(0)
{
    switch(id)
    {
    case PostProcessors::VMIPhotonHitseV1d:
        _device=CASSEvent::CCD; _detector = 0;
        break;
    case PostProcessors::PnCCDFrontPhotonHitseV1d:
        _device=CASSEvent::pnCCD; _detector = 0;
        break;
    case PostProcessors::PnCCDBackPhotonHitseV1d:
        _device=CASSEvent::pnCCD; _detector = 1;
        break;
    default:
        throw std::invalid_argument("Impossible postprocessor id for pp113");
        break;
    };
    loadSettings(0);
}

cass::pp116::~pp116()
{
    _pp.histograms_delete(_id);
    _hist = 0;
}

void cass::pp116::loadSettings(size_t)
{
  std::cout <<std::endl<< "load the parameters of postprocessor "<<_id
      <<" it histograms the Nbr of Mcp Peaks"
      <<" of detector "<<_detector
      <<" of device "<<_device
      <<std::endl;
  QSettings param;
  param.beginGroup("PostProcessor");
  param.beginGroup(QString("p") + QString::number(_id));
  //load the condition on the third component//
  adu2eV = param.value("adu2eV",5.).toDouble();
  if(adu2eV<=0.) adu2eV=1.;
  //create the histogram
  _pp.histograms_delete(_id);
  _hist=0;
  if(param.value("adu2eV",5.).toDouble()!=0.)
  {
    std::cerr << "Creating 1D histogram with"
              <<" XNbrBins:"<<param.value("XNbrBins",1).toUInt()
              <<" XLow:"<<param.value("XLow",0).toFloat()
              <<" XUp:"<<16384./param.value("adu2eV",0).toFloat()
              <<std::endl;
    _hist = new cass::Histogram1DFloat(param.value("XNbrBins",1).toUInt(),
                                       param.value("XLow",0).toFloat(),
                                       16384./param.value("adu2eV",0).toFloat());
  }
  else  set1DHist(_hist,_id);
  _pp.histograms_replace(_id,_hist);
}

void cass::pp116::operator()(const CASSEvent& evt)
{
    //check whether detector exists
    if (evt.devices().find(_device)->second->detectors()->size() <= _detector)
        throw std::runtime_error(QString("PostProcessor_%1: Detector %2 does not exist in Device %3"
                                         ).arg(_id).arg(_detector).arg(_device).toStdString());

    //retrieve the detector's photon hits of the device we are working for.
    const PixelDetector::pixelList_t& pixellist
        ((*(evt.devices().find(_device)->second)->detectors())[_detector].pixellist());

    //cass::pnCCD::DetectorParameter &dp = _param._detectorparameters[iDet];
    //const double adu2eV = 15.;//((*(evt.devices().find(_device)->second)->detectors()->detectorparameters)[_detector]._adu2eV());
    PixelDetector::pixelList_t::const_iterator it(pixellist.begin());
    _hist->lock.lockForWrite();
    for (; it != pixellist.end();++it)
        _hist->fill(it->z()/adu2eV);
    _hist->lock.unlock();
}













// *** postprocessor 141 -- integral over last image from VMI CCD ***

pp141::pp141(PostProcessors& pp, cass::PostProcessors::id_t id)
    : PostprocessorBackend(pp, id), _value(new Histogram0DFloat)
{
    _pp.histograms_replace(_id, _value);
}



pp141::~pp141()
{
    _pp.histograms_delete(_id);
}



void pp141::operator()(const cass::CASSEvent&)
{
    HistogramFloatBase *hist(dynamic_cast<HistogramFloatBase *>(_pp.histograms_checkout().find(PostProcessors::VmiCcdLastImage)->second));
    _pp.histograms_release();
    hist->lock.lockForRead();
    const HistogramFloatBase::storage_t& val(hist->memory());
    HistogramFloatBase::value_t sum(0);
    std::accumulate(val.begin(), val.end(), sum);
    hist->lock.unlock();
    _value->lock.lockForWrite();
    *_value = sum;
    _value->lock.unlock();
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



} // end namespace cass



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "Stroustrup"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:
