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


// ********** Postprocessor 203: substract local background ************

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

      for (size_t y=ysectionbegin; y<ysectionend; ++y)
      {
        for (size_t x=xsectionbegin; x<xsectionend; ++x)
        {
          const size_t xboxbegin(max(static_cast<int>(xsectionbegin),static_cast<int>(x)-static_cast<int>(xboxsize)));
          const size_t xboxend(min(xsectionend,x+xboxsize));
          const size_t yboxbegin(max(static_cast<int>(ysectionbegin),static_cast<int>(y)-static_cast<int>(yboxsize)));
          const size_t yboxend(min(ysectionend,y+yboxsize));

          box.clear();
          for (size_t yb=yboxbegin; yb<yboxend;++yb)
          {
            for (size_t xb=xboxbegin; xb<xboxend;++xb)
            {
              const size_t pixAddrBox(yb*xsize+xb);
              const float pixel_box(image_in[pixAddrBox]);
              box.push_back(pixel_box);
            }
          }

          nth_element(box.begin(), box.begin() + 0.5*box.size(), box.end());
          const float median = box[0.5*box.size()];
          const size_t pixAddr(y*xsize+x);
//          const float pixel(image_in[pixAddr]);
//          const float backgroundsubstractedPixel(pixel-median);
          image_out[pixAddr] = median;
        }
      }
    }
  }
  hist.lock.unlock();
  _result->nbrOfFills() = 1;
  _result->lock.unlock();
}

