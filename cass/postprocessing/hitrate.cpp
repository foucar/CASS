// Copyright (C) 2010 Lutz Foucar

#include <QtCore/QSettings>
#include <QtCore/QString>


#include "cass.h"
#include "hitrate.h"
#include "postprocessor.h"
#include "histogram.h"


namespace cass
{

// *** postprocessor 589 finds Single particle hits ***
cass::pp589::pp589(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key), _result(0), _integralimg(NULL), _rowsum(NULL)
{
  loadSettings(0);
}

cass::pp589::~pp589()
{
  _pp.histograms_delete(_key);
  delete _integralimg;
  _integralimg = NULL;
  delete _rowsum;
  _rowsum = NULL;
  _result = 0;
}

cass::PostProcessors::active_t cass::pp589::dependencies()
{
  PostProcessors::active_t list;
  list.push_front(_idHist);
  return list;
}

void cass::pp589::loadSettings(size_t)
{
  using namespace std;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  bool OneNotvalid (!retrieve_and_validate(_pp,_key,"HistName",_idHist));
  if (OneNotvalid)
    return;
  _threshold = settings.value("threshold", 1.0).toFloat();
  _xstart = settings.value("xstart", 0).toInt();
  _ystart = settings.value("ystart", 0).toInt();
  _xend = settings.value("xend", -1).toInt();
  _yend = settings.value("yend", -1).toInt();
  _pp.histograms_delete(_key);
  _result = new Histogram0DFloat;
  //_pp.histograms_replace(_key,_result);


  const Histogram2DFloat* img
      //(dynamic_cast<Histogram2DFloat*>(histogram_checkout(_idHist)));
      (dynamic_cast<Histogram2DFloat*>( _pp.histograms_checkout().find(_idHist)->second));
  _pp.histograms_release();
  _integralimg = new Histogram2DFloat(*img);
  _rowsum = new Histogram2DFloat(*img);

  _pp.histograms_replace(_key,_integralimg);  // for debuging, output _integralimage instead of _result

  std::cout<<"Postprocessor "<<_key
      <<": detects Single particle hits in PostProcessor "<< _idHist
      <<" threshold for detection:"<<_threshold
      <<" ROI for detection: ["<<_xstart<<","<<_ystart<<","<<_xend<<","<<_yend<<"]"
      <<std::endl;
}

void cass::pp589::operator()(const CASSEvent&)
{
  using namespace std;
  Histogram2DFloat* one
      (dynamic_cast<Histogram2DFloat*>(histogram_checkout(_idHist)));
  _integralimg->lock.lockForWrite();
  _rowsum->lock.lockForWrite();
  one->lock.lockForRead();


  const size_t nxbins (one->axis()[HistogramBackend::xAxis].nbrBins());
  const size_t nybins (one->axis()[HistogramBackend::yAxis].nbrBins());

  HistogramFloatBase::storage_t& img_mem( one->memory() );
  HistogramFloatBase::storage_t& rowsum_mem( _rowsum->memory() );
  HistogramFloatBase::storage_t& integralimg_mem( _integralimg->memory() );

  // sanity checks and auto-set nxbins. todo: restore -1 so that nxbins/nybins gets updated on size change?
  if (_xstart < 0) _xstart = 0;
  if (_ystart < 0) _ystart = 0;
  if (_xend >= nxbins) _xend = nxbins-1;
  if (_yend >= nybins) _yend = nybins-1;
  if (_xend < 0) _xend = nxbins-1;
  if (_yend < 0) _yend = nybins-1;

  //for (size_t row=0; row<nybins; ++row)
  //  for (size_t col=0; col<nxbins; ++col) {
  //}

  std::cout << "INTEGRAL IMAGE: XXXXXXXXXXXX [" <<_xstart<<","<<_ystart<<","<<_xend<<","<<_yend<<"]" << std::endl;
      // calculate integral-image:
      // row sum initialize first row:
      for (int xx = _xstart; xx<=_xend; ++xx)
        rowsum_mem[xx-_xstart] = img_mem[xx + _ystart*nxbins];
      // row sum rest:
      for (int xx = _xstart; xx<=_xend; ++xx)
        for (int yy=_ystart+1; yy<=_yend; ++yy)
          rowsum_mem[xx-_xstart + (yy-_ystart)*nxbins] = rowsum_mem[xx-_xstart + (yy-_ystart-1)*nxbins] + img_mem[xx + yy*nxbins];
      // intImg first col:
      for (int yy = _ystart; yy<=_yend; ++yy)
        integralimg_mem[(yy-_ystart)*nxbins] = rowsum_mem[(yy-_ystart)*nxbins];
      // intImg rest:
      for (int xx = _xstart+1; xx<=_xend; ++xx)
        for (int yy = _ystart; yy<=_yend; ++yy)
          integralimg_mem[xx-_xstart + (yy-_ystart)*nxbins] = integralimg_mem[xx-_xstart-1 + (yy-_ystart)*nxbins] + rowsum_mem[xx-_xstart + (yy-_ystart)*nxbins];

  one->lock.unlock();
  _integralimg->lock.unlock();
  _rowsum->lock.unlock();
  _result->lock.lockForWrite();
  *_result = _threshold;
  _result->lock.unlock();
}




}
