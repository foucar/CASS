// Copyright (C) 2013 Lutz Foucar

#include "pixel_detector_calibration.h"

#include "histogram.h"
#include "cass_settings.h"
#include "log.h"


using namespace cass;
using namespace std;


pp330::pp330(const name_t &name)
  : AccumulatingPostProcessor(name)
{
  loadSettings(0);
}

void pp330::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  _image = setupDependency("RawImage");
  setupGeneral();
  bool ret (setupCondition());
  if (!(_image && ret))
    return;

  _filename = s.value("Filename","out.cal").toString().toStdString();
  _write = s.value("WriteCal",true).toBool();
  _train = s.value("Train",true).toBool();
  _minTrainImages = s.value("NbrTrainingImages",200).toUInt();
  _snr = s.value("SNR",4).toFloat();

  _nTrainImages = 0;
  _trainstorage.clear();

  pair<size_t,size_t> shape(dynamic_cast<const Histogram2DFloat&>(_image->result()).shape());
  createHistList(
        tr1::shared_ptr<Histogram2DFloat>
        (new Histogram2DFloat(shape.first,3*shape.second)));
  loadCalibration();
  Log::add(Log::INFO,"Postprocessor " + name() +
           ": generates the calibration data from images contained in '" +
           _image->name() + "'. Condition is '" + _condition->name() + "'");
}

void pp330::loadCalibration()
{

}

void pp330::writeCalibration()
{
  ofstream out(_filename.c_str(), ios::binary);
  if (!out.is_open())
    throw invalid_argument("pp330::writeCalibration(): Error opening file '" +
                           _filename + "'");

  const Histogram2DFloat::storage_t &result
      (dynamic_cast<const Histogram2DFloat*>(_result.get())->memory());
  const Histogram2DFloat &res(dynamic_cast<const Histogram2DFloat&>(*_result));
  const size_t sizeOfImage(res.shape().first*res.shape().second/3.);

  vector<double> offsets(sizeOfImage);
  Histogram2DFloat::storage_t::const_iterator meanbegin(result.begin());
  Histogram2DFloat::storage_t::const_iterator meanend(result.begin()+sizeOfImage);
  copy(meanbegin,meanend,offsets.begin());
  out.write(reinterpret_cast<char*>(&offsets[0]), offsets.size()*sizeof(double));

  vector<double> noises(sizeOfImage);
  Histogram2DFloat::storage_t::const_iterator stdvbegin(result.begin() + sizeOfImage);
  Histogram2DFloat::storage_t::const_iterator stdvend(result.begin() + 2*sizeOfImage);
  copy(stdvbegin,stdvend,noises.begin());
  out.write(reinterpret_cast<char*>(&noises[0]), noises.size()*sizeof(double));
}

void pp330::aboutToQuit()
{
  if (_write)
    writeCalibration();
}

void pp330::process(const CASSEvent &evt, HistogramBackend &res)
{
  const Histogram2DFloat &image
      (dynamic_cast<const Histogram2DFloat&>(_image->result(evt.id())));
  const size_t sizeOfImage = image.shape().first * image.shape().second;

  Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(res));
  Histogram2DFloat::storage_t::iterator meanAr(result.memory().begin());
  Histogram2DFloat::storage_t::iterator stdvAr(result.memory().begin()+sizeOfImage);
  Histogram2DFloat::storage_t::iterator nValsAr(result.memory().begin()+2*sizeOfImage);

  QReadLocker lock(&image.lock);

  if (_train)
  {
    if (_nTrainImages < _minTrainImages)
    {
      _trainstorage.push_back(image.memory());
      ++_nTrainImages;
    }
    else
    {
      //generate the initial calibration
      for (size_t iPix=0; iPix < sizeOfImage; ++iPix)
      {
        vector<float> pixdistribution;
        for (size_t iStore=0; iStore < _trainstorage.size(); ++iStore)
          pixdistribution.push_back(_trainstorage[iStore][iPix]);
        sort(pixdistribution.begin(),pixdistribution.end());

        vector<float>::iterator lowPos(pixdistribution.begin());
        vector<float>::iterator upPos(pixdistribution.end());

        CummulativeStatisticsCalculator<float> stat;
        bool outliersdetected(false);
        do
        {
          stat.reset();
          stat.addDistribution(lowPos,upPos);

          const float lowBound(stat.mean() - _snr * stat.stdv());
          const float upBound(stat.mean() + _snr * stat.stdv());
          vector<float>::iterator newLowPos(lower_bound(pixdistribution.begin(), pixdistribution.end(), lowBound));
          vector<float>::iterator newUpPos(upper_bound (pixdistribution.begin(), pixdistribution.end(), upBound));

          /** outliers have been detected when the low and up iterators have changed */
          outliersdetected = ( newLowPos != lowPos || newUpPos != upPos);

          lowPos = newLowPos;
          upPos = newUpPos;
        }
        while (outliersdetected);

        meanAr[iPix] = stat.mean();
        stdvAr[iPix] = stat.stdv();
        nValsAr[iPix] = distance(lowPos,upPos);
      }
      _train = false;
      _nTrainImages = 0;
      _trainstorage.clear();
    }
  }
  else
  {
    //add current image to statistics of the calibration
    for(size_t iPix=0; iPix < sizeOfImage; ++iPix)
    {
      const float mean(meanAr[iPix]);
      const float stdv(stdvAr[iPix]);
      const float pix(image.memory()[iPix]);
      /** if pixel is outlier skip pixel */
      if(_snr * stdv < pix - mean)
        continue;
      const float nVals(nValsAr[iPix] + 1);
      const float alpha (1./nVals);
      const float delta(pix - mean);
      const float newmean(mean + (delta * alpha));
      const float tmp(stdv*stdv / alpha);
      const float newtmp(tmp + delta * (pix - mean) );
      const float newstdv(sqrt(newtmp*alpha));

      meanAr[iPix] = newmean;
      stdvAr[iPix] = newstdv;
      nValsAr[iPix] = nVals;
    }
  }
}





//********** gain calibrations ******************

pp331::pp331(const name_t &name)
  : AccumulatingPostProcessor(name)
{
  loadSettings(0);
}

void pp331::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  _image = setupDependency("Image");
  setupGeneral();
  bool ret (setupCondition());
  if (!(_image && ret))
    return;

  _counter = 0;
  _nFrames = s.value("NbrOfFrames",-1).toInt();
  _filename = s.value("Filename","out.cal").toString().toStdString();
  _write = s.value("WriteCal",true).toBool();
  _aduRange = make_pair(s.value("ADURangeLow",0).toFloat(),
                        s.value("ADURangeUp",0).toFloat());
  _minPhotonCount = s.value("MinimumNbrPhotons",200).toUInt();
  _constGain = s.value("DefaultGainValue",1).toFloat();

  const Histogram2DFloat &image(dynamic_cast<const Histogram2DFloat&>(_image->result()));
  pair<size_t,size_t> shape(image.shape());
  createHistList(
        tr1::shared_ptr<Histogram2DFloat>
        (new Histogram2DFloat(shape.first,3*shape.second)));
  loadCalibration();
  Log::add(Log::INFO,"Postprocessor " + name() +
           ": generates the gain calibration from images contained in '" +
           _image->name() + "'. Condition is '" + _condition->name() + "'");
}

void pp331::loadCalibration()
{

}

void pp331::writeCalibration()
{
  ofstream out(_filename.c_str(), ios::binary);
  if (!out.is_open())
    throw invalid_argument("pp331::writeCalibration(): Error opening file '" +
                           _filename + "'");

  const Histogram2DFloat &image(dynamic_cast<const Histogram2DFloat&>(*_result));
  const size_t sizeOfImage(image.shape().first*image.shape().second/3);

  const Histogram2DFloat::storage_t &gains(image.memory());
  out.write(reinterpret_cast<const char*>(&gains.front()), sizeOfImage*sizeof(float));
}

void pp331::aboutToQuit()
{
  calculateGainMap(dynamic_cast<Histogram2DFloat&>(*_result));
  if (_write)
    writeCalibration();
}

void pp331::calculateGainMap(Histogram2DFloat& gainmap)
{
  /** calculate the average of the average pixelvalues, disregarding pixels
   *  that have not seen enough photons in the right ADU range.
   */
  int counter(0);
  double average(0);
  const size_t sizeOfImage(gainmap.shape().first*gainmap.shape().second/3);

  Histogram2DFloat::storage_t::iterator gain(gainmap.memory().begin());
  Histogram2DFloat::storage_t::const_iterator count(gainmap.memory().begin()+1*sizeOfImage);
  Histogram2DFloat::storage_t::const_iterator ave(gainmap.memory().begin()+2*sizeOfImage);
  for (size_t i(0); i < sizeOfImage; ++i, ++count, ++ave)
  {
    if (*count < _minPhotonCount)
      continue;
    ++counter;
    average += (*ave - average)/counter;
  }

  /** assining the gain value for each pixel that has seen enough statistics.
   *  gain is calculated by formula
   *  \f$ gain = frac{average_average_pixelvalue}{average_pixelvalue} \f$
   *  If not enough photons are in the pixel, set the predefined user value
   */
  count = gainmap.memory().begin()+1*sizeOfImage;
  ave = gainmap.memory().begin()+2*sizeOfImage;
  for (size_t i(0); i < sizeOfImage; ++i, ++gain, ++count, ++ave)
  {
    *gain = (*count < _minPhotonCount) ? _constGain : average/(*ave);
  }
}

void pp331::process(const CASSEvent &evt, HistogramBackend &res)
{
  const Histogram2DFloat &image
      (dynamic_cast<const Histogram2DFloat&>(_image->result(evt.id())));

  Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(res));

  QReadLocker lock(&image.lock);
  Histogram2DFloat::storage_t::const_iterator pixel(image.memory().begin());
  Histogram2DFloat::storage_t::const_iterator ImageEnd(image.memory().end());
  const size_t sizeOfImage(image.shape().first * image.shape().second);

  Histogram2DFloat::storage_t::iterator gain(result.memory().begin());
  Histogram2DFloat::storage_t::iterator count(result.memory().begin()+1*sizeOfImage);
  Histogram2DFloat::storage_t::iterator ave(result.memory().begin()+2*sizeOfImage);

  /** go though all pixels of image*/
  for (; pixel != ImageEnd; ++pixel, ++gain, ++count, ++ave)
  {
    /** check if pixel is within the 1 photon adu range */
    if (_aduRange.first < *pixel && *pixel < _aduRange.second)
    {
      /** calculate the mean pixel value */
      *count += 1;
      *ave += ((*pixel - *ave) / *count);
    }
  }

  /** if we have reached the requested nbr of frames calculate the gain map */
  ++_counter;
  if (_counter % _nFrames == 0)
    calculateGainMap(result);

}




//********** hot pixel detection ******************

pp332::pp332(const name_t &name)
  : AccumulatingPostProcessor(name)
{
  loadSettings(0);
}

void pp332::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  _image = setupDependency("Image");
  setupGeneral();
  bool ret (setupCondition());
  if (!(_image && ret))
    return;

  _counter = 0;
  _nFrames = s.value("NbrOfFrames",-1).toInt();
  _filename = s.value("Filename","out.cal").toString().toStdString();
  _write = s.value("WriteCal",true).toBool();
  _aduRange = make_pair(s.value("ADURangeLow",0).toFloat(),
                        s.value("ADURangeUp",0).toFloat());
  _maxConsecutiveCount = s.value("MaximumConsecutiveFrames",5).toUInt();

  const Histogram2DFloat &image(dynamic_cast<const Histogram2DFloat&>(_image->result()));
  pair<size_t,size_t> shape(image.shape());
  createHistList(
        tr1::shared_ptr<Histogram2DFloat>
        (new Histogram2DFloat(shape.first,2*shape.second)));
  loadHotPixelMap();
  Log::add(Log::INFO,"Postprocessor " + name() +
           ": generates the hot pixel map from images contained in '" +
           _image->name() + "'. Condition is '" + _condition->name() + "'");
}

void pp332::loadHotPixelMap()
{
  ifstream in(_filename.c_str(), ios::binary);
  if (!in.is_open())
  {
    Log::add(Log::WARNING,"pp332::loadHotPixelMap: Could not open '" + _filename +
             "'. Skipping reading the hot pixels mask.");
    return;
  }
  in.seekg(0,std::ios::end);
  const int valuesize(sizeof(mask_t));
  const size_t size = in.tellg() / valuesize;
  in.seekg(0,std::ios::beg);
  vector<mask_t> hotpixmask(size);
  in.read(reinterpret_cast<char*>(&hotpixmask.front()), size*valuesize);

  Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(*_result));

  const size_t sizeOfImage(result.shape().first*result.shape().second/2);
  if (sizeOfImage != size)
  {
    Log::add(Log::WARNING,"pp332::loadHotPixelMap: Size of mask to load '" +
             toString(size) + "' does not fit with size of image '" +
             toString(sizeOfImage) + "'. Skipping reading the hot pixels mask in '" +
             _filename +"'.");
    return;
  }
  copy(hotpixmask.begin(),hotpixmask.end(),result.memory().begin());
}

void pp332::writeHotPixelMap()
{
  ofstream out(_filename.c_str(), ios::binary);
  if (!out.is_open())
    throw invalid_argument("pp332::writeCalibration(): Error opening file '" +
                           _filename + "'");

  const Histogram2DFloat &image(dynamic_cast<const Histogram2DFloat&>(*_result));
  const size_t sizeOfImage(image.shape().first*image.shape().second/2);

  const Histogram2DFloat::storage_t &hpmask(image.memory());
  vector<mask_t> mask(sizeOfImage);
  copy(hpmask.begin(),hpmask.begin()+sizeOfImage,mask.begin());
  out.write(reinterpret_cast<char*>(&mask.front()), sizeOfImage*sizeof(mask_t));
}

void pp332::aboutToQuit()
{
  if (_write)
    writeHotPixelMap();
}

void pp332::process(const CASSEvent &evt, HistogramBackend &res)
{
  const Histogram2DFloat &image
      (dynamic_cast<const Histogram2DFloat&>(_image->result(evt.id())));

  Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(res));

  QReadLocker lock(&image.lock);
  Histogram2DFloat::storage_t::const_iterator pixel(image.memory().begin());
  Histogram2DFloat::storage_t::const_iterator ImageEnd(image.memory().end());
  const size_t sizeOfImage(image.shape().first * image.shape().second);

  Histogram2DFloat::storage_t::iterator hotpix(result.memory().begin());
  Histogram2DFloat::storage_t::iterator count(result.memory().begin()+1*sizeOfImage);

  /** go though all pixels of image*/
  for (; pixel != ImageEnd; ++pixel, ++count, ++hotpix)
  {
    /** check if pix is not masked as hot */
    if (qFuzzyCompare(*hotpix,-1.f))
      continue;

    /** check if pixel is within the hot pixel adu range */
    if (_aduRange.first < *pixel && *pixel < _aduRange.second)
    {
      *count += 1;
      if (_maxConsecutiveCount <= *count)
        *hotpix = -1;
    }
    else
      *count = 0;
  }

//  /** if we have reached the requested nbr of frames calculate the gain map */
//  ++_counter;
//  if (_counter % _nFrames == 0)
//    calculateGainMap(result);

}
