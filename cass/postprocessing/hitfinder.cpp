// Copyright (C) 2013 Lutz Foucar

/** @file hitfinder.cpp contains postprocessors that will extract pixels of
 *                      interrest from 2d histograms.
 * @author Lutz Foucar
 */

#include <QtCore/QString>
#include <iterator>
#include <algorithm>

#include "cass.h"
#include "hitfinder.h"
#include "postprocessor.h"
#include "histogram.h"
#include "convenience_functions.h"
#include "cass_settings.h"
#include "log.h"

using namespace std;
using namespace cass;


// ********** Postprocessor 203: subtract local background ************

pp203::pp203(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp203::loadSettings(size_t)
{
  CASSSettings s;

  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));

  // size of box for median
  _boxSize = make_pair(s.value("BoxSizeX", 10).toUInt(),
                       s.value("BoxSizeY",10).toUInt());
  _sectionSize = make_pair(s.value("SectionSizeX", 1024).toUInt(),
                           s.value("SectionSizeY",512).toUInt());

  setupGeneral();

  // Get the input
  _hist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(_hist && ret)) return;

  // Create the output
  setup(_hist->getHist(0));

  Log::add(Log::INFO,"PostProcessor '" + _key +
           "' removes median of a box with size x'" + toString(_boxSize.first) +
           "' and y '" + toString(_boxSize.second) + "' from individual pixels" +
           " of hist in pp '"+ _hist->key() + "'. Condition is '" +
           _condition->key() + "'");
}

void pp203::setup(const HistogramBackend &hist)
{
  _result = hist.clone();
  createHistList(2*cass::NbrOfWorkers);
}

void pp203::histogramsChanged(const HistogramBackend* in)
{
  QWriteLocker lock(&_histLock);
  //return when there is no incomming histogram
  if(!in)
    return;
  //return when the incomming histogram is not a direct dependant
  if (find(_dependencies.begin(),_dependencies.end(),in->key()) == _dependencies.end())
    return;
  setup(*in);
  //notify all pp that depend on us that our histograms have changed
  PostProcessors::keyList_t dependands (_pp.find_dependant(_key));
  PostProcessors::keyList_t::iterator it (dependands.begin());
  for (; it != dependands.end(); ++it)
    _pp.getPostProcessor(*it).histogramsChanged(_result);
  Log::add(Log::VERBOSEINFO,"Postprocessor '" + _key +
             "': histograms changed => delete existing histo" +
             " and created new one from input");
}

void pp203::process(const CASSEvent &evt)
{
  // Get the input
  const HistogramFloatBase &hist
      (dynamic_cast<const HistogramFloatBase&>((*_hist)(evt)));
  const HistogramFloatBase::storage_t &image_in(hist.memory());
  HistogramFloatBase::storage_t &image_out
      (dynamic_cast<HistogramFloatBase*>(_result)->memory());

  _result->lock.lockForWrite();
  hist.lock.lockForRead();

  const size_t xsize(hist.axis()[HistogramBackend::xAxis].nbrBins());
  const size_t ysize(hist.axis()[HistogramBackend::yAxis].nbrBins());
  const size_t xsectionsize(_sectionSize.first);
  const size_t ysectionsize(_sectionSize.second);
  const size_t xnbrsections(xsize/xsectionsize);
  const size_t ynbrsections(ysize/ysectionsize);
  const size_t xboxsize(_boxSize.first);
  const size_t yboxsize(_boxSize.second);

  vector<float> box;
  for (size_t sy=0; sy<ynbrsections; ++sy)
  {
    for (size_t sx=0; sx<xnbrsections; ++sx)
    {
      const size_t xsectionbegin(sx*xsectionsize);
      const size_t xsectionend(sx*xsectionsize + xsectionsize);
      const size_t ysectionbegin(sy*ysectionsize);
      const size_t ysectionend(sy*ysectionsize + ysectionsize);

//      cout << xsectionbegin << " "<<xsectionend << " "<<ysectionbegin << " "<<ysectionend<<endl;
      for (size_t y=ysectionbegin; y<ysectionend; ++y)
      {
        for (size_t x=xsectionbegin; x<xsectionend; ++x)
        {
          const size_t pixAddr(y*xsize+x);
          const float pixel(image_in[pixAddr]);

          if ( qFuzzyCompare(pixel,0.f) )
          {
            image_out[pixAddr] = pixel;
            continue;
          }

          const size_t xboxbegin(max(static_cast<int>(xsectionbegin),static_cast<int>(x)-static_cast<int>(xboxsize)));
          const size_t xboxend(min(xsectionend,x+xboxsize));
          const size_t yboxbegin(max(static_cast<int>(ysectionbegin),static_cast<int>(y)-static_cast<int>(yboxsize)));
          const size_t yboxend(min(ysectionend,y+yboxsize));

//          cout <<x << " " << xboxbegin << " "<<xboxend <<" : " <<y<< " "<<yboxbegin << " "<<yboxend<<endl;
          box.clear();
          for (size_t yb=yboxbegin; yb<yboxend;++yb)
          {
            for (size_t xb=xboxbegin; xb<xboxend;++xb)
            {
              const size_t pixAddrBox(yb*xsize+xb);
              const float pixel_box(image_in[pixAddrBox]);
              if ( ! qFuzzyCompare(pixel_box,0.f) )
                box.push_back(pixel_box);
            }
          }

          if (box.empty())
          {
            image_out[pixAddr] = pixel;
          }
          else
          {
            const size_t mid(0.5*box.size());
            nth_element(box.begin(), box.begin() + mid, box.end());
            const float median = box[mid];
            image_out[pixAddr] = median;
//            const float backgroundsubstractedPixel(pixel-median);
//            image_out[pixAddr] = backgroundsubstractedPixel;
          }
        }
      }
    }
  }
  hist.lock.unlock();
  _result->nbrOfFills() = 1;
  _result->lock.unlock();
}



// ********** Postprocessor 204: find bragg peaks ************

pp204::pp204(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp204::loadSettings(size_t)
{
  CASSSettings s;

  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));

  // size of box for median
  _box = make_pair(s.value("BoxSizeX", 10).toUInt(),
                   s.value("BoxSizeY",10).toUInt());
  _section = make_pair(s.value("SectionSizeX", 1024).toUInt(),
                       s.value("SectionSizeY",512).toUInt());
  _threshold = s.value("Threshold",300).toFloat();
  _minSnr = s.value("MinSignalToNoiseRatio",20).toFloat();
  _minBckgndPixels = s.value("MinNbrBackgrndPixels",10).toInt();
  const int peakRadius(s.value("BraggPeakRadius",2).toInt());
  _peakRadiusSq = peakRadius*peakRadius;
  snrall_mean = 0;
  snrall_stdv = 0;
  counterall = 0;
  snr_mean = 0;
  snr_stdv = 0;
  counter = 0;
  radius_mean = 0;
  radius_stdv = 0;
  counter_rad = 0;

  setupGeneral();

  // Get the input
  _hist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(_hist && ret)) return;

  // Create the output
  _result = new Histogram2DFloat(nbrOf);
  createHistList(2*cass::NbrOfWorkers);

  Log::add(Log::INFO,"PostProcessor '" + _key +
           "' finds bragg peaks in" + _hist->key() +
           "'. Boxsize '" + toString(_box.first)+"x"+ toString(_box.second)+
           "'. SectionSize '" + toString(_section.first)+"x"+ toString(_section.second)+
           "'. Threshold '" + toString(_threshold) +
           "'. MinSignalToNoiseRatio '" + toString(_minSnr) +
           "'. MinNbrBackgrndPixels '" + toString(_minBckgndPixels) +
           "'. Square BraggPeakRadius '" + toString(_peakRadiusSq) +
           "'. Condition is '" + _condition->key() + "'");
}

int pp204::getBoxStatistics(HistogramFloatBase::storage_t::const_iterator pixel,
                            int ncols,
                            float &mean, float &stdv, int &count)
{
  enum{good,skip};
  float tmp_mean(0);
  float tmp_stdv(0);
  count = 0;
  for (int bRow=-_box.second; bRow <= _box.second; ++bRow)
  {
    for (int bCol=-_box.first; bCol <= _box.first; ++bCol)
    {
      if (bRow == 0 && bCol == 0)
        continue;

      const int bLocIdx(bRow*ncols+bCol);
      const float bPixel(pixel[bLocIdx]);

      if(*pixel < bPixel )
        return skip;

      const int radiussq(bRow*bRow + bCol*bCol);
      if (_peakRadiusSq < radiussq)
      {
        if (qFuzzyIsNull(bPixel))
          return skip;

        ++count;
        const float old_mean(tmp_mean);
        tmp_mean += ((bPixel - old_mean) / static_cast<float>(count));
        tmp_stdv += ((bPixel - old_mean)*(bPixel - tmp_mean));
      }
    }
  }
  mean = tmp_mean;
  stdv = sqrt(tmp_stdv/ (count-1));
  return good;
}

void pp204::process(const CASSEvent &evt)
{
  const HistogramFloatBase &hist
      (dynamic_cast<const HistogramFloatBase&>((*_hist)(evt)));
  const HistogramFloatBase::storage_t &image(hist.memory());

  Histogram2DFloat &result(*dynamic_cast<Histogram2DFloat*>(_result));

  result.lock.lockForWrite();
  hist.lock.lockForRead();

  result.clearTable();
  result.nbrOfFills() = 1;


  const size_t ncols(hist.axis()[HistogramBackend::xAxis].size());
  const size_t nrows(hist.axis()[HistogramBackend::yAxis].size());

  BraggPeak peak(nbrOf);
  vector<bool> checkedPixels(image.size(),false);

  vector<bool>::iterator checkedPixel(checkedPixels.begin());
  HistogramFloatBase::storage_t::const_iterator pixel(image.begin());
  int idx(0);
  for (;pixel != image.end(); ++pixel,++idx, ++checkedPixel)
  {
    /** check if pixel has been touched before */
    if (*checkedPixel)
      continue;

    /** check above a set threshold */
    if (*pixel < _threshold)
      continue;

    /** get coordinates of pixel and tell that it is touched */
    const uint16_t col(idx % ncols);
    const uint16_t row(idx / ncols);

    /** make sure that pixel is located such that the box will not conflict with
     *  the image and section boundaries
     */
    if (col < _box.first  || ncols - _box.first  < col || //within box in x
        row < _box.second || nrows - _box.second < row || //within box in y
        (col - _box.first)  / _section.first  != (col + _box.first)  / _section.first || //x within same section
        (row - _box.second) / _section.second != (row + _box.second) / _section.second)  //y within same section
      continue;

    /** check wether current pixel value is highest and generate background values */
    float mean,stdv;
    int count;
    if (getBoxStatistics(pixel,ncols,mean,stdv,count))
      continue;

    /** check that we took more than the minimum pixels for the background */
    if (count < _minBckgndPixels)
      continue;

    /** check if signal to noise ration is good for the current pixel */
    const float snr((*pixel - mean) / stdv);

//    ++counterall;
//    const float oldsnrall_mean(snrall_mean);
//    snrall_mean += ((snr - oldsnrall_mean)/static_cast<float>(counterall));
//    snrall_stdv += ((snr - oldsnrall_mean)*(snr - snrall_mean));


//    const bool NoOutlier = counter < 2000 ? (snr < _minSnr) : (snr - snr_mean < 10 * sqrt(snr_stdv/(counter -1)));
//    const bool NoOutlier = (snr - snrall_mean < _minSnr * sqrt(snrall_stdv/(counterall -1)));


    if (snr < _minSnr)
//    if (NoOutlier)
    {
//      char tmp[256];
//      snprintf(tmp, 255, "\r%6.5f %6.5f  |  %6.5f %6.5f   |  %6.5f",
//               snr_mean, sqrt(snr_stdv/(counter -1)),snrall_mean,sqrt(snrall_stdv/(counterall -1)),snr );
//      cout << tmp << flush;

      continue;
    }

    /** find all pixels in the box whose signal to noise ratio is big enough and
     *  centriod them. Mark the pixels as touched, so that they don't have to
     *  be checked again. Find out the maximum and minimum radius
     */
    float integral = 0;
    float weightCol = 0;
    float weightRow = 0;
    int nPix = 0;
    int max_radiussq=0;
    int min_radiussq=max(_box.first,_box.second)*max(_box.first,_box.second);
    for (int bRow=-_box.second; bRow <= _box.second; ++bRow)
    {
      for (int bCol=-_box.first; bCol <= _box.first; ++bCol)
      {
        const int bLocIdx(bRow*ncols+bCol);
        const float bPixel(pixel[bLocIdx]);
        const float bPixelWOBckgnd(bPixel - mean);
        const float bSnr(bPixelWOBckgnd / stdv);
        if (_minSnr < bSnr)
//        if (_minSnr * sqrt(snrall_stdv/(counterall -1)) < bSnr - snrall_mean)
        {
          const int radiussq(bRow*bRow + bCol*bCol);
          if (radiussq > max_radiussq)
            max_radiussq = radiussq;
          if (radiussq < min_radiussq)
            min_radiussq = radiussq;
          integral += bPixelWOBckgnd;
          weightCol += (bPixelWOBckgnd * bCol);
          weightRow += (bPixelWOBckgnd * bRow);
          ++nPix;
        }
        checkedPixel[bLocIdx] = true;
      }
    }
//    ++counter;
//    const float oldsnr_mean(snr_mean);
//    snr_mean += ((nPix - oldsnr_mean)/static_cast<float>(counter));
//    snr_stdv += ((nPix - oldsnr_mean)*(nPix - snr_mean));

//    if (nPix - snr_mean < 2 * sqrt(snr_stdv/(counter -1)))
//    if (nPix - snr_mean < 0)
//      continue;

    peak[centroidColumn] = weightCol / integral;
    peak[centroidRow] = weightRow / integral;
    peak[Intensity] = integral;
    peak[nbrOfPixels] = nPix;
    peak[SignalToNoise] = snr;
    peak[MaxRadius] = sqrt(max_radiussq);
    peak[MinRadius] = sqrt(min_radiussq);
    peak[Index] = idx;
    peak[Column] = col;
    peak[Row] = row;
    peak[MaxADU] = *pixel;
    peak[LocalBackground] = mean;
    peak[LocalBackgroundDeviation] = stdv;
    peak[nbrOfBackgroundPixels] = count;

    result.addRow(peak);

//    ++counter_rad;
//    const float oldradius_mean(radius_mean);
//    radius_mean += ((max_radiussq - oldradius_mean)/static_cast<float>(counter_rad));
//    radius_stdv += ((max_radiussq - oldradius_mean)*(max_radiussq - radius_mean));
//    char tmp[256];
//    snprintf(tmp, 255, "\r%6.5f %6.5f %5i",
//             radius_mean,sqrt(radius_stdv/(counter_rad -1)), max_radiussq );
//    cout << tmp << flush;

  }

  hist.lock.unlock();
  result.lock.unlock();
}




// ********** Postprocessor 205: display peaks ************

pp205::pp205(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp205::loadSettings(size_t)
{
  CASSSettings s;

  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));

  setupGeneral();

  // Get the input
  _hist = setupDependency("HistName");
  _table = setupDependency("TableName");
  bool ret (setupCondition());
  if (!(_hist && ret && _table))
    return;

  _boxsize = make_pair(s.value("BoxSizeX", 10).toUInt(),
                       s.value("BoxSizeY",10).toUInt());
  _drawVal = s.value("DrawPixelValue",16000.f).toFloat();
  _radius = s.value("Radius",2.f).toFloat();
  _idxCol = s.value("IndexColumn").toUInt();
  _drawCircle = s.value("DrawCircle",true).toBool();
  _drawBox = s.value("DrawBox",true).toBool();

  size_t maxIdx(_table->getHist(0).axis()[HistogramBackend::xAxis].size());
  if (_idxCol >= maxIdx)
    throw runtime_error("pp205::loadSettings(): '" + _key + "' The requested " +
                        "column '" + toString(_idxCol) + " 'exeeds the " +
                        "maximum possible value of '" + toString(maxIdx) + "'");

  // Create the output
  _result = _hist->getHist(0).clone();
  createHistList(2*cass::NbrOfWorkers);

  Log::add(Log::INFO,"PostProcessor '" + _key +
           "' displays the peaks that are listed in '" + _table->key() +
           "' and that were found in '" + _hist->key() + "'. Condition is '" +
           _condition->key() + "'");
}

void pp205::histogramsChanged(const HistogramBackend* in)
{
  QWriteLocker lock(&_histLock);
  //return when there is no incomming histogram
  if(!in)
    return;
  //return when the incomming histogram is not a direct dependant
  if (find(_dependencies.begin(),_dependencies.end(),in->key()) == _dependencies.end())
    return;
  _result = in->clone();
  createHistList(2*cass::NbrOfWorkers);
  //notify all pp that depend on us that our histograms have changed
  PostProcessors::keyList_t dependands (_pp.find_dependant(_key));
  PostProcessors::keyList_t::iterator it (dependands.begin());
  for (; it != dependands.end(); ++it)
    _pp.getPostProcessor(*it).histogramsChanged(_result);
  Log::add(Log::VERBOSEINFO,"Postprocessor '" + _key +
             "': histograms changed => delete existing histo" +
             " and created new one from input");
}

void pp205::process(const CASSEvent &evt)
{
  // Get the input
  const HistogramFloatBase &hist
      (dynamic_cast<const HistogramFloatBase&>((*_hist)(evt)));
  const HistogramFloatBase::storage_t &image_in(hist.memory());
  const HistogramFloatBase &table
      (dynamic_cast<const Histogram2DFloat&>((*_table)(evt)));
  const HistogramFloatBase::storage_t &tableContents(table.memory());
  HistogramFloatBase::storage_t &image_out
      (dynamic_cast<HistogramFloatBase*>(_result)->memory());

  _result->lock.lockForWrite();
  hist.lock.lockForRead();
  table.lock.lockForRead();

  /** copy the input image to the resulting image */
  copy(image_in.begin(),image_in.end(),image_out.begin());

  /** extract the nbr of rows and columns of this table and the image */
  const size_t nTableCols(table.axis()[HistogramBackend::xAxis].size());
  const size_t nTableRows(table.axis()[HistogramBackend::yAxis].size());
  const size_t nImageCols(hist.axis()[HistogramBackend::xAxis].size());

  /** go through all rows in table */
  for (size_t row=0; row < nTableRows; ++row)
  {
    /** extract the column with the global index of the peak center */
    size_t idx(tableContents[row*nTableCols + _idxCol]);
    HistogramFloatBase::storage_t::iterator centerpixel(image_out.begin()+idx);

    /** draw user requested info (extract other stuff from table) */
    //box
    if (_drawBox)
    {
      //lower row
      for (int bCol=-_boxsize.first; bCol <= _boxsize.first; ++bCol)
      {
        const int bRow = -_boxsize.second;
        const int bLocIdx(bRow*nImageCols+bCol);
        centerpixel[bLocIdx] = _drawVal;
      }
      //upper row
      for (int bCol=-_boxsize.first; bCol <= _boxsize.first; ++bCol)
      {
        const int bRow = _boxsize.second;
        const int bLocIdx(bRow*nImageCols+bCol);
        centerpixel[bLocIdx] = _drawVal;
      }
      //left col
      for (int bRow=-_boxsize.second; bRow <= _boxsize.second; ++bRow)
      {
        const int bCol = -_boxsize.first;
        const int bLocIdx(bRow*nImageCols+bCol);
        centerpixel[bLocIdx] = _drawVal;
      }
      //right col
      for (int bRow=-_boxsize.second; bRow <= _boxsize.second; ++bRow)
      {
        int bCol = _boxsize.first;
        const int bLocIdx(bRow*nImageCols+bCol);
        centerpixel[bLocIdx] = _drawVal;
      }
    }

    //circle
    if (_drawCircle)
    {
      for(size_t angle_deg = 0; angle_deg <360; ++angle_deg)
      {
        const float angle_rad(3.14 * static_cast<float>(angle_deg)/180.);
        const int cCol (static_cast<size_t>(round(_radius*sin(angle_rad))));
        const int cRow (static_cast<size_t>(round(_radius*cos(angle_rad))));
        const int cLocIdx(cRow*nImageCols+cCol);
        centerpixel[cLocIdx] = _drawVal;
      }
    }

  }
  _result->nbrOfFills() = 1;
  table.lock.unlock();
  hist.lock.unlock();
  _result->lock.unlock();
}

