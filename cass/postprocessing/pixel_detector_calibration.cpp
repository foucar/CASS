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

  const size_t sizeOfImage(0);
  const Histogram2DFloat::storage_t &result
      (dynamic_cast<const Histogram2DFloat*>(_result.get())->memory());

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

  _filename = s.value("Filename","out.cal").toString().toStdString();
  _write = s.value("WriteCal",true).toBool();
  _aduRange = make_pair(s.value("ADURangeLow",0).toFloat(),
                        s.value("ADURangeUp",0).toFloat());
  _minPhotonCount = s.value("MinimumNbrPhotons",200).toUInt();
  _constGain = s.value("DefaultGainValue",1).toFloat();

  const Histogram2DFloat &image(dynamic_cast<const Histogram2DFloat&>(_image->result()));
  _statistics.clear();
  _statistics.resize(image.memory().size(),make_pair(0,0));
  pair<size_t,size_t> shape(image.shape());
  createHistList(
        tr1::shared_ptr<Histogram2DFloat>
        (new Histogram2DFloat(shape.first,shape.second)));
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

  const Histogram2DFloat::storage_t &gains
      (dynamic_cast<const Histogram2DFloat*>(_result.get())->memory());
  out.write(reinterpret_cast<const char*>(&gains.front()), gains.size()*sizeof(float));
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
  int count(0);
  double ave(0);
  statistics_t::const_iterator stat(_statistics.begin());
  statistics_t::const_iterator statEnd(_statistics.end());
  while (stat != statEnd)
  {
    const statistic_t &s(*stat++);
    if (s.first < _minPhotonCount)
      continue;
    ++count;
    ave = ave + (s.second - ave)/count;
  }

  /** assining the gain value for each pixel that has seen enough statistics.
   *  gain is calculated by formula
   *  \f$ gain = frac{average_average_pixelvalue}{average_pixelvalue} \f$
   *  If not enough photons are in the pixel, set the predefined user value
   */
  Histogram2DFloat::storage_t::iterator gain(gainmap.memory().begin());
  stat = _statistics.begin();
  while (stat != statEnd)
  {
    const statistic_t &s(*stat++);
    *gain++ = (s.first < _minPhotonCount) ? _constGain : ave/s.second;
  }
}

void pp331::process(const CASSEvent &evt, HistogramBackend &/*res*/)
{
  const Histogram2DFloat &image
      (dynamic_cast<const Histogram2DFloat&>(_image->result(evt.id())));

  //Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(res));

  QReadLocker lock(&image.lock);
  Histogram2DFloat::storage_t::const_iterator pixel(image.memory().begin());
  Histogram2DFloat::storage_t::const_iterator ImageEnd(image.memory().end());

  statistics_t::iterator stat(_statistics.begin());

  /** go though all pixels of image*/
  for (; pixel != ImageEnd; ++pixel, ++stat)
  {
    /** check if pixel is within the 1 photon adu range */
    if (_aduRange.first < *pixel && *pixel < _aduRange.second)
    {
      /** calculate the mean pixel value */
      double &ave(stat->second);
      size_t &N(stat->first);

      ++N;
      ave += ((*pixel - ave) / N);
    }
  }
}
