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
  _image = setupDependency("HistName");
  setupGeneral();
  bool ret (setupCondition());
  if (!(_image && ret))
    return;
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
