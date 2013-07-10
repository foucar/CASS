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
using tr1::bind;


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
  const int peakRadius(s.value("BraggPeakRadius",2).toInt());
  _peakRadiusSq = peakRadius*peakRadius;
  const int goodBoxSize(sqrt(3.1415) * peakRadius);
  _box = make_pair(s.value("BoxSizeX", goodBoxSize).toUInt(),
                   s.value("BoxSizeY",goodBoxSize).toUInt());
  _section = make_pair(s.value("SectionSizeX", 1024).toUInt(),
                       s.value("SectionSizeY",512).toUInt());
  _threshold = s.value("Threshold",300).toFloat();
  _minSnr = s.value("MinSignalToNoiseRatio",20).toFloat();
  _minNeighbourSNR = s.value("MinNeighbourSNR",3).toFloat();
  _minBckgndPixels = s.value("MinNbrBackgrndPixels",10).toInt();

  setupGeneral();

  _hist = setupDependency("HistName");

  bool ret (setupCondition());
  if (!(_hist && ret))
    return;

  /** Create the result output */
  _result = new Histogram2DFloat(nbrOf);
  createHistList(2*cass::NbrOfWorkers);

  /** log what the user was requesting */
  string output("PostProcessor '" + _key + "' finds bragg peaks." +
                "'. Boxsize '" + toString(_box.first)+"x"+ toString(_box.second)+
                "'. SectionSize '" + toString(_section.first)+"x"+ toString(_section.second)+
                "'. Threshold '" + toString(_threshold) +
                "'. MinSignalToNoiseRatio '" + toString(_minSnr) +
                "'. MinNbrBackgrndPixels '" + toString(_minBckgndPixels) +
                "'. Square BraggPeakRadius '" + toString(_peakRadiusSq) +
                "'. Using input histogram :" + _hist->key() +
                "'. Condition is '" + _condition->key() + "'");
  Log::add(Log::INFO,output);
}


int pp204::getBoxStatistics(HistogramFloatBase::storage_t::const_iterator pixel,
                            const imagepos_t ncols,
                            pixelval_t &mean, pixelval_t &stdv, int &count)
{
  enum{good,skip};
  CummulativeStatisticsCalculator<pixelval_t> stat;
  for (imagepos_t bRow=-_box.second; bRow <= _box.second; ++bRow)
  {
    for (imagepos_t bCol=-_box.first; bCol <= _box.first; ++bCol)
    {
      /** only consider pixels that are not in the origin of the box */
      if (bRow == 0 && bCol == 0)
        continue;

      /** check if the current box pixel value is bigger than the center pixel
       *  value. If so skip this pixel
       */
      const imagepos_t bLocIdx(bRow*ncols+bCol);
      const pixelval_t bPixel(pixel[bLocIdx]);
      if(*pixel < bPixel )
        return skip;

      /** if box pixel is outside the radius, add it to the statistics, if it
       *  is inside the radius bad then skip, as no pixel that could potentially
       *  be part of the peak should be a bad pixel.
       */
      const bool pixIsBad(qFuzzyIsNull(bPixel));
      const int radiussq(bRow*bRow + bCol*bCol);
      if (_peakRadiusSq < radiussq)
        stat.addDatum(bPixel);
      else if (pixIsBad)
        return skip;
    }
  }
  count = stat.count();
  mean = stat.mean();
  stdv = stat.stdv();
  return good;
}

void pp204::process(const CASSEvent & evt)
{
  const HistogramFloatBase &hist
      (dynamic_cast<const HistogramFloatBase&>((*_hist)(evt)));
  const HistogramFloatBase::storage_t &image(hist.memory());

  Histogram2DFloat &result(*dynamic_cast<Histogram2DFloat*>(_result));

  hist.lock.lockForRead();
  result.lock.lockForWrite();
  result.clearTable();

  const imagepos_t ncols(hist.axis()[HistogramBackend::xAxis].size());
  const imagepos_t nrows(hist.axis()[HistogramBackend::yAxis].size());

  imagepos_t neigbourOffsetArray[] =
  {
    ncols-1,
    ncols,
    ncols+1,
    -1,
    1,
    -ncols-1,
    -ncols,
    -ncols+1,
  };
  vector<imagepos_t> neighboursOffsets(neigbourOffsetArray,
                                neigbourOffsetArray + sizeof(neigbourOffsetArray)/sizeof(int));
  table_t peak(nbrOf,0);
  vector<bool> checkedPixels(image.size(),false);

  vector<bool>::iterator checkedPixel(checkedPixels.begin());
  HistogramFloatBase::storage_t::const_iterator pixel(image.begin());
  imagepos_t idx(0);
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
    pixelval_t mean,stdv;
    int count;
    if (getBoxStatistics(pixel,ncols,mean,stdv,count))
      continue;

    /** check that we took more than the minimum pixels for the background */
    if (count < _minBckgndPixels)
      continue;

    /** check if signal to noise ration is good for the current pixel */
    const pixelval_t snr((*pixel - mean) / stdv);
    if (snr < _minSnr)
      continue;

    /** from the central pixel look around and see which pixels are direct
     *  part of the peak. If a neighbour is found, mask it such that one does
     *  not use it twice.
     *
     *  Create a list that should contain the indizes of the pixels that are
     *  part of the peak. Go through that list and check for each found
     *  neighbour whether it also has a neighbour. If so add it to the list, but
     *  only if it is part of the box.
     */
    vector<imagepos_t> peakIdxs;
    peakIdxs.push_back(idx);
    *checkedPixel = true;
    for (size_t pix=0; pix < peakIdxs.size(); ++pix)
    {
      const imagepos_t pixpos(peakIdxs[pix]);
      vector<imagepos_t>::const_iterator nOffset(neighboursOffsets.begin());
      vector<imagepos_t>::const_iterator neighboursEnd(neighboursOffsets.end());
      while(nOffset != neighboursEnd)
      {
        const size_t nIdx(pixpos + *nOffset++);
        const imagepos_t nCol(nIdx % ncols);
        const imagepos_t nRow(nIdx / ncols);
        if (checkedPixels[nIdx] ||
            _box.first < abs(col - nCol) ||
            _box.second < abs(row - nRow))
          continue;
        const pixelval_t nPixel(image[nIdx]);
        const pixelval_t nPixelWOBckgnd(nPixel - mean);
        const pixelval_t nSNR(nPixelWOBckgnd / stdv);
        if (_minNeighbourSNR < nSNR)
        {
          peakIdxs.push_back(nIdx);
          checkedPixels[nIdx] = true;
        }
      }
    }

    /** go through all pixels in the box, find out which pixels are part of the
     *  peak and centroid them. Mask all pixels in the box as checked so one
     *  does not check them again
     */
    pixelval_t integral = 0;
    pixelval_t weightCol = 0;
    pixelval_t weightRow = 0;
    imagepos_t nPix = 0;
    imagepos_t max_radiussq=0;
    imagepos_t min_radiussq=max(_box.first,_box.second)*max(_box.first,_box.second);
    for (int bRow=-_box.second; bRow <= _box.second; ++bRow)
    {
      for (int bCol=-_box.first; bCol <= _box.first; ++bCol)
      {
        const imagepos_t bLocIdx(bRow*ncols+bCol);
        const pixelval_t bPixel(pixel[bLocIdx]);
        const pixelval_t bPixelWOBckgnd(bPixel - mean);
        if (checkedPixel[bLocIdx])
        {
          const imagepos_t radiussq(bRow*bRow + bCol*bCol);
          if (radiussq > max_radiussq)
            max_radiussq = radiussq;
          if (radiussq < min_radiussq)
            min_radiussq = radiussq;
          integral += bPixelWOBckgnd;
          weightCol += (bPixelWOBckgnd * (bCol+col));
          weightRow += (bPixelWOBckgnd * (bRow+row));
          ++nPix;
        }
        checkedPixel[bLocIdx] = true;
      }
    }
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

    result.appendRows(peak);

  }

  result.nbrOfFills() = 1;
  result.lock.unlock();
  hist.lock.unlock();
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






// ********** Postprocessor 206: find pixels of bragg peaks ************

pp206::pp206(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp206::loadSettings(size_t)
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
  _multiplier = s.value("Multiplier",4).toFloat();

  setupGeneral();

  _imagePP = setupDependency("ImageName");
  _noisePP = setupDependency("NoiseName");

  bool ret (setupCondition());
  if (!(_imagePP && ret && _noisePP))
    return;

  /** Create the result output */
  _result = new Histogram2DFloat(pp204::nbrOf);
  createHistList(2*cass::NbrOfWorkers);

  /** log what the user was requesting */
  string output("PostProcessor '" + _key + "' finds bragg peaks." +
                "'. Boxsize '" + toString(_box.first)+"x"+ toString(_box.second)+
                "'. SectionSize '" + toString(_section.first)+"x"+ toString(_section.second)+
                "'. Threshold '" + toString(_threshold) +
                "'. Image Histogram :" + _imagePP->key() +
                "'. Noise Histogram :" + _noisePP->key() +
                "'. Condition is '" + _condition->key() + "'");
  Log::add(Log::INFO,output);
}

void pp206::process(const CASSEvent & evt)
{
  const HistogramFloatBase &imageHist
      (dynamic_cast<const HistogramFloatBase&>((*_imagePP)(evt)));
  const HistogramFloatBase::storage_t &image(imageHist.memory());
  const HistogramFloatBase &noiseHist
      (dynamic_cast<const HistogramFloatBase&>((*_noisePP)(evt)));
  const HistogramFloatBase::storage_t &noisemap(noiseHist.memory());

  Histogram2DFloat &result(*dynamic_cast<Histogram2DFloat*>(_result));

  imageHist.lock.lockForRead();
  noiseHist.lock.lockForRead();
  result.lock.lockForWrite();
  result.clearTable();


  table_t peak(pp204::nbrOf,0);

  HistogramFloatBase::storage_t::const_iterator pixel(image.begin());
  HistogramFloatBase::storage_t::const_iterator imageEnd(image.end());
  HistogramFloatBase::storage_t::const_iterator noise(noisemap.begin());
  size_t idx(0);
  vector<float> box;
  const uint16_t ncols(imageHist.axis()[HistogramBackend::xAxis].size());
  const uint16_t nrows(imageHist.axis()[HistogramBackend::yAxis].size());
  for (; pixel != imageEnd; ++pixel, ++noise, ++idx)
  {
//    if(*noise * _multiplier < *pixel)
    if (_threshold < *pixel)
    {
      const uint16_t x(idx % ncols);
      const uint16_t y(idx / ncols);

      const uint16_t xboxbegin(max(static_cast<int>(0),static_cast<int>(x)-static_cast<int>(_box.first)));
      const uint16_t xboxend(min(ncols,static_cast<uint16_t>(x+_box.first)));
      const uint16_t yboxbegin(max(static_cast<int>(0),static_cast<int>(y)-static_cast<int>(_box.second)));
      const uint16_t yboxend(min(nrows,static_cast<uint16_t>(y+_box.second)));

      box.clear();
      for (size_t yb=yboxbegin; yb<yboxend;++yb)
      {
        for (size_t xb=xboxbegin; xb<xboxend;++xb)
        {
          const size_t pixAddrBox(yb*ncols+xb);
          const float pixel_box(image[pixAddrBox]);
          /** check if current sourrounding pixel is a bad pixel (0.),
           *  if so we should disregard the pixel as a candiate and check the
           *  next pixel.
           */
          if (qFuzzyCompare(pixel_box,0.f) )
            goto NEXTPIXEL;
          else
            box.push_back(pixel_box);
        }
      }

      if (box.size() > 1)
      {
        const size_t mid(0.5*box.size());
        nth_element(box.begin(), box.begin() + mid, box.end());
        const float bckgnd = box[mid];
        const float clrdpixel(*pixel - bckgnd);
//        if(*noise * _multiplier < clrdpixel)
        if (_threshold < clrdpixel)
        {
          peak[pp204::Column] = x;
          peak[pp204::Row] = y;
          peak[pp204::MaxADU] = *pixel;
          peak[pp204::Intensity] = clrdpixel;

          result.appendRows(peak);
        }
      }
NEXTPIXEL:;
    }
  }
  result.nbrOfFills() = 1;
  result.lock.unlock();
  imageHist.lock.unlock();
  noiseHist.lock.unlock();

}





// *** will display the detected pixel in the table as 2d image ***

pp207::pp207(PostProcessors& pp, const cass::PostProcessors::key_t &key)
    : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp207::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));

  _table = setupDependency("Table");

  setupGeneral();
  bool ret (setupCondition());
  if (!(ret && _table))
    return;

  set2DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);

  _pixColIdx = s.value("ColumnIndex",0).toUInt();
  _pixRowIdx = s.value("RowIndex",0).toUInt();
  _pixValIdx = s.value("ValIndex",0).toUInt();

  Log::add(Log::INFO,"Postprocessor '" + _key +
           "' displays the pixels in table '" + _table->key() +
           "'' Index of Pixles column '" + toString(_pixColIdx) +
           "'' Index of Pixles row '" + toString(_pixRowIdx) +
           "'' Index of Pixles val '" + toString(_pixValIdx) +
           "'. Condition is '" + _condition->key() + "'");
}

void pp207::process(const CASSEvent& evt)
{
  const HistogramFloatBase &table
      (dynamic_cast<const Histogram2DFloat&>((*_table)(evt)));
  const HistogramFloatBase::storage_t &tableContents(table.memory());
  HistogramFloatBase::storage_t::const_iterator tableIt(tableContents.begin());

  Histogram2DFloat &result(*dynamic_cast<Histogram2DFloat*>(_result));

  result.lock.lockForWrite();
  result.clear();

  table.lock.lockForRead();

  const size_t nRows(table.axis()[HistogramBackend::yAxis].size());
  const size_t nCols(table.axis()[HistogramBackend::xAxis].size());

  /** go through all rows in table, extract the pixel column, row and value.
   *  then advance the table iterator by one row
   */
  for (size_t row=0; row < nRows; ++row)
  {
    const int pixCol(tableIt[_pixColIdx]);
    const int pixRow(tableIt[_pixRowIdx]);
    const float pixVal(tableIt[_pixValIdx]);
    result.fill(pixCol,pixRow,pixVal);
    tableIt += nCols;
  }

  table.lock.unlock();
  result.lock.unlock();
}







// ********** Postprocessor 204: find bragg peaks ************

pp208::pp208(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);

}

void pp208::loadSettings(size_t)
{
  CASSSettings s;

  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));

  _section = make_pair(s.value("SectionSizeX", 1024).toUInt(),
                       s.value("SectionSizeY",512).toUInt());
  _threshold = s.value("Threshold",0).toFloat();
  _minSnr = s.value("MinSignalToNoiseRatio",4).toFloat();
  _minRatio = s.value("MinRatio",3).toFloat();

  const int peakDiameter(s.value("BraggPeakDiameter",2).toInt());
  /** area of box should at least be ratio times area of peak.
   * \f{eqnarray*}{
   * A_{box}&=& ratio \times A_{circle} \\
   * size^2 &=& ratio \times \pi r^2 \\
   * size   &=& \sqrt{\pi r^2 \times ratio} \\
   *        &=& r \sqrt{\pi \times ratio} \\
   *        &=& 0.5 d \sqrt{\pi \times ratio}
   * \f}
   *  If \f$ size \f$ goes from  \f$-s ... s\f$ then \f$ size = 2s+1 \f$
   * \f{eqnarray*}{
   * 2s + 1 &=& 0.5 d \sqrt{\pi \times ratio} \\
   *      s &=& 0.5 \times 0.5 d \sqrt{\pi \times ratio} - 0.5 \\
   *        &=& 0.25 d \sqrt{\pi \times ratio} - 0.5
   * \f}
   * convert to integer
   * \f{eqnarray*}{
   * (int)s &=& 0.25d\sqrt{\pi \times ratio} - 0.5 + 0.5 \\
   *        &=& 0.25d\sqrt{\pi \times ratio}
   * \f}
   */
  const int bsize(0.25 * peakDiameter * sqrt(3.14159 * _minRatio));
  _box = make_pair(s.value("BoxSizeX", bsize).toInt(),
                   s.value("BoxSizeY",bsize).toInt());

  /** min nbr of pixels should reflect the area under the bragg peak
   *  \f$ nbrPeaks = 0.25 \pi d \f$
   */
  _minNbrPixels = s.value("MinNbrPixels",0.25*3.14159*square(peakDiameter)).toInt();

  setupGeneral();

  _imagePP = setupDependency("HistName");

  bool ret (setupCondition());
  if (!(_imagePP && ret))
    return;

  /** set up the neighbouroffset list */
  _imageShape = dynamic_cast<const Histogram2DFloat&>(_imagePP->getHist(0)).shape();
  _neighbourOffsets.clear();
  _neighbourOffsets.push_back(+_imageShape.first-1);     //up left
  _neighbourOffsets.push_back(+_imageShape.first+0);     //up
  _neighbourOffsets.push_back(+_imageShape.first+1);     //up right
  _neighbourOffsets.push_back(-1);                       //left
  _neighbourOffsets.push_back(+1);                       //right
  _neighbourOffsets.push_back(-_imageShape.first-1);     //low left
  _neighbourOffsets.push_back(-_imageShape.first+0);     //low
  _neighbourOffsets.push_back(-_imageShape.first+1);     //low right

  /** Create the result output */
  _result = new Histogram2DFloat(nbrOf);
  createHistList(2*cass::NbrOfWorkers);

  /** log what the user was requesting */
  string output("PostProcessor '" + _key + "' finds bragg peaks." +
                "'. Boxsize '" + toString(_box.first)+"x"+ toString(_box.second)+
                "'. SectionSize '" + toString(_section.first)+"x"+ toString(_section.second)+
                "'. Threshold '" + toString(_threshold) +
                "'. MinSignalToNoiseRatio '" + toString(_minSnr) +
                "'. MinPixels '" + toString(_minNbrPixels) +
                "'. MinFraction '" + toString(_minRatio) +
                "'. Using input histogram :" + _imagePP->name() +
                "'. Condition is '" + _condition->name() + "'");
  Log::add(Log::INFO,output);
}


int pp208::getBoxStatistics(HistogramFloatBase::storage_t::const_iterator pixel,
                            const index_t linIdx, const shape_t &box, stat_t &stat)
{
  enum{use,skip};

  /** get coordinates of pixel from the linearized index */
  const index_t col(linIdx % _imageShape.first);
  const index_t row(linIdx / _imageShape.first);

  /** make sure that pixel is located such that the box will not conflict with
   *  the image and section boundaries. If it does continue with next pixel
   */
  if (col < box.first  || _imageShape.first - box.first  < col || //within box in x
      row < box.second || _imageShape.second - box.second < row || //within box in y
      (col - box.first)  / _section.first  != (col + box.first)  / _section.first || //x within same section
      (row - box.second) / _section.second != (row + box.second) / _section.second)  //y within same section
    return skip;

  /** go through all pixels defined by the box from -rows ... rows, -cols ... cols */
  for (shape_t::second_type bRow = -box.second; bRow <= box.second; ++bRow)
  {
    for (shape_t::first_type bCol = -box.first; bCol <= box.first; ++bCol)
    {
      /** check if the current box pixel value is bigger than the center pixel
       *  value. If so skip this pixel
       */
      const index_t bLocIdx(bRow*_imageShape.first+bCol);
      const pixelval_t bPixel(pixel[bLocIdx]);
      if(*pixel < bPixel )
        return skip;

      /** if its not a bad pixel add pixel to statistics */
      if (!qFuzzyIsNull(bPixel))
        stat.addDatum(bPixel);
    }
  }
  return use;
}

int pp208::isNotHighest(HistogramFloatBase::storage_t::const_iterator pixel,
                        const index_t linIdx, shape_t box, stat_t &stat)
{
  enum{use,skip};

  bool boxsizeincreased(false);
  do
  {
    /** check whether current pixel value is highest and generate background values */
    stat.reset();
    if (getBoxStatistics(pixel,linIdx,box,stat))
      return skip;

    /** skip this pixel if there are not enough pixels that could potentially be
     *  part of the bragg peak.
     */
    if (stat.nbrUpperOutliers() < _minNbrPixels)
      return skip;

    /** increase the box size and start over if the fraction of outliers to
     *  points used in the statistics is smaller than requested.
     */
    if (stat.nbrPointsUsed() < _minRatio * stat.nbrUpperOutliers())
    {
      ++(box.first);
      ++(box.second);
      boxsizeincreased = true;
    }
    else
    {
      boxsizeincreased = false;
    }
  }
  while(boxsizeincreased);

  return use;
}


void pp208::process(const CASSEvent & evt, HistogramBackend &r)
{
  /** retrive references to work with from incomming image and output table */
  const Histogram2DFloat &hist
      (dynamic_cast<const Histogram2DFloat&>(_imagePP->getHist(evt.id())));
  const HistogramFloatBase::storage_t &image(hist.memory());

  Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(r));

  /** lock the resources */
  QWriteLocker rLock(&result.lock);
  QReadLocker hLock(&hist.lock);

  /** clear the resulting table to fill it with the values of this image */
  result.clearTable();

  /** assert that incomming image has not changed the shape */
  assert(static_cast<index_t>(hist.shape().first) == _imageShape.first &&
         static_cast<index_t>(hist.shape().second) ==  _imageShape.second);

  /** get a table row that we can later add to the table */
  table_t peak(nbrOf,0);

  /** set up a mask that we can see which pixels have been treated */
  vector<bool> checkedPixels(image.size(),false);

  /** get iterators for the mask and the image with which we can iterate through
   *  the image, also rember which linearized index we are working on right
   *  now, to be able to retrieve the column and row that the current pixel
   *  corresponsed to.
   */
  vector<bool>::iterator checkedPixel(checkedPixels.begin());
  HistogramFloatBase::storage_t::const_iterator pixel(image.begin());
  HistogramFloatBase::storage_t::const_iterator ImageEnd(image.end());
  index_t idx(0);
  for (;pixel != ImageEnd; ++pixel,++idx, ++checkedPixel)
  {
    /** check if pixel should be treated, when it has been treated before
     *  continue with the next pixel
     */
    if (*checkedPixel)
      continue;

    /** check if pixel is above the threshold, otherwise continue with the next
     *  pixel
     */
    if (*pixel < _threshold)
      continue;

    /** check if pixel is highest within box. If so, check if there are enough
     *  pixels that are not outliers. If there are not enough pixels in the box
     *  increase the box size and do the checks all over again. If the pixel is
     *  not highest within the bigger box continue with the next pixel.
     */
    shape_t box(_box);
    stat_t stat(_minSnr);
    if (isNotHighest(pixel, idx, box, stat))
      continue;

    /** retrive statistical values from the calculator */
    const stat_t::value_type mean(stat.mean());
    const stat_t::value_type stdv(stat.stdv());

    /** get coordinates of pixel in image from the linearized index */
    const index_t col(idx % _imageShape.first);
    const index_t row(idx / _imageShape.first);

    /** from the central pixel look around and see which pixels are direct
     *  part of the peak. If a neighbour is found, mask it such that one does
     *  not use it twice.
     *
     *  Create a list that should contain the indizes of the pixels that are
     *  part of the peak. Go through that list and check for each found
     *  neighbour whether it also has a neighbour. If so add it to the list, but
     *  only if it is part of the box.
     */
    vector<index_t> peakIdxs;
    peakIdxs.push_back(idx);
    *checkedPixel = true;
    for (size_t pix=0; pix < peakIdxs.size(); ++pix)
    {
      const index_t pixpos(peakIdxs[pix]);
      neighbourList_t::const_iterator ngbrOffset(_neighbourOffsets.begin());
      neighbourList_t::const_iterator neighboursEnd(_neighbourOffsets.end());
      while(ngbrOffset != neighboursEnd)
      {
        const index_t ngbrIdx(pixpos + *ngbrOffset++);
        const index_t ngbrCol(ngbrIdx % _imageShape.first);
        const index_t ngbrRow(ngbrIdx / _imageShape.first);
        if (checkedPixels[ngbrIdx] ||
            box.first < abs(col - ngbrCol) ||   //pixel not inside box col
            box.second < abs(row - ngbrRow))    //pixel not inside box row
          continue;
        const pixelval_t ngbrPixel(image[ngbrIdx]);
        const pixelval_t ngbrPixelWOBckgnd(ngbrPixel - mean);
        if ((_minSnr * stdv) < ngbrPixelWOBckgnd)
        {
          peakIdxs.push_back(ngbrIdx);
          checkedPixels[ngbrIdx] = true;
        }
      }
    }

    /** if the peak doesn't have enough pixels continue with next pixel */
    if (static_cast<int>(peakIdxs.size()) < _minNbrPixels)
      continue;

    /** go through all pixels in the box, find out which pixels are part of the
     *  peak and centroid them. Mask all pixels in the box as checked so one
     *  does not check them again
     */
    pixelval_t integral = 0;
    pixelval_t weightCol = 0;
    pixelval_t weightRow = 0;
    index_t max_radiussq=0;
    index_t min_radiussq=max(box.first,box.second)*max(box.first,box.second);
    for (int bRow = -box.second; bRow <= box.second; ++bRow)
    {
      for (int bCol = -box.first; bCol <= box.first; ++bCol)
      {
        const index_t bLocIdx(bRow*_imageShape.first+bCol);
        if (checkedPixel[bLocIdx])
        {
          const pixelval_t bPixel(pixel[bLocIdx]);
          const pixelval_t bPixelWOBckgnd(bPixel - mean);
          const index_t radiussq(bRow*bRow + bCol*bCol);
          if (radiussq > max_radiussq)
            max_radiussq = radiussq;
          if (radiussq < min_radiussq)
            min_radiussq = radiussq;
          integral += bPixelWOBckgnd;
          weightCol += (bPixelWOBckgnd * (bCol+col));
          weightRow += (bPixelWOBckgnd * (bRow+row));
        }
        checkedPixel[bLocIdx] = true;
      }
    }

    /** set the peak's properties and add peak to the list of found peaks */
    peak[centroidColumn] = weightCol / integral;
    peak[centroidRow] = weightRow / integral;
    peak[Intensity] = integral;
    peak[nbrOfPixels] = peakIdxs.size();
    peak[SignalToNoise] = (*pixel-mean)/stdv;
    peak[MaxRadius] = sqrt(max_radiussq);
    peak[MinRadius] = sqrt(min_radiussq);
    peak[Index] = idx;
    peak[Column] = col;
    peak[Row] = row;
    peak[MaxADU] = *pixel;
    peak[LocalBackground] = mean;
    peak[LocalBackgroundDeviation] = stdv;

    result.appendRows(peak);
  }

  /** tell that only the result of one event (image) is present in the table */
  result.nbrOfFills() = 1;
}





