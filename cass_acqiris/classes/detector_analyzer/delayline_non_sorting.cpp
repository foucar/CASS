// Copyright (C) 2010 Lutz Foucar

/**
 * @file delayline_non_sorting.cpp ile contains the class that finds detectorhits
 *                               without sorting
 *
 * @author Lutz Foucar
 */


#include "delayline_non_sorting.h"

#include "poscalculator.hpp"
#include "cass_settings.h"
#include "signal_producer.h"

using namespace std;
using namespace cass::ACQIRIS;
using namespace std::tr1;

DelaylineNonSorting::DelaylineNonSorting()
  :DetectorAnalyzerBackend()
{}

detectorHits_t& DelaylineNonSorting::operator()(detectorHits_t &hits)
{
  typedef SignalProducer::signals_t signals_t;
  typedef signals_t::iterator sigIt_t;

  /** extract the signal arrays from the signal producers */
  signals_t &f1signals (_layerCombination.first.first->output());
  signals_t &f2signals (_layerCombination.first.second->output());
  signals_t &s1signals (_layerCombination.second.first->output());
  signals_t &s2signals (_layerCombination.second.second->output());

  /** extract the iterators from the arrays */
  sigIt_t iF1(f1signals.begin());
  sigIt_t iF2 (f2signals.begin());
  sigIt_t iS1 (s1signals.begin());
  sigIt_t iS2 (s2signals.begin());

  /** find out which array is the shortest and assign the iterators of that
   *  array to the loop variables */
  size_t minsize(f1signals.size());
  sigIt_t sigIt(f1signals.begin());
  sigIt_t end(f1signals.end());
  if (minsize > f2signals.size())
  {
    minsize = f2signals.size();
    sigIt = f2signals.begin();
    end = f2signals.end();
  }
  if (minsize > s1signals.size())
  {
    minsize = s1signals.size();
    sigIt = s1signals.begin();
    end = s1signals.end();
  }
  if (minsize > s2signals.size())
  {
    minsize = s2signals.size();
    sigIt = s2signals.begin();
    end = s2signals.end();
  }

  /** go linear through all hits and create detectorhits for each entry in the
   *  array
   */
  for (; sigIt != end; ++sigIt, ++iF1, ++iF2, ++iS1, ++iS2)
  {
    detectorHit_t hit;
    const double f1 ((*iF1)["time"]);
    const double f2 ((*iF2)["time"]);
    const double s1 ((*iS1)["time"]);
    const double s2 ((*iS2)["time"]);
    const double f ((f1-f2) * _sf.first);
    const double s ((s1-s2) * _sf.second);
    const pair<double,double> pos ((*_poscalc)(make_pair(f,s)));
    hit["x"] = pos.first;
    hit["y"] = pos.second;
    hit["t"] = 0;
    hits.push_back(hit);
  }
  return hits;
}

void DelaylineNonSorting::loadSettings(CASSSettings& s, DelaylineDetector &d)
{
  enum LayerComb{xy,uv,uw,vw};

  DelaylineType delaylinetype
      (static_cast<DelaylineType>(s.value("DelaylineType",Hex).toInt()));

  s.beginGroup("NonSorting");
  LayerComb lc (static_cast<LayerComb>(s.value("LayersToUse",xy).toInt()));
  if (lc == xy)
  {
    if (delaylinetype == Hex)
    {
      stringstream ss;
      ss << "DelaylineNonSorting::loadSettings: Error using layers xy for Hex-Detector";
      throw invalid_argument(ss.str());
    }
  }
  if (delaylinetype == Quad)
  {
    if (lc == uv || lc == uw || lc == vw)
    {
      stringstream ss;
      ss << "DelaylineNonSorting::loadSettings: Error using layers uv, uw or vw for Quad-Detector";
      throw invalid_argument(ss.str());
    }
  }

  switch (lc)
  {
  case (xy):
    _layerCombination = make_pair(make_pair(&d.layers()['X'].wireends()['1'],
                                            &d.layers()['X'].wireends()['2']),
                                  make_pair(&d.layers()['Y'].wireends()['1'],
                                            &d.layers()['Y'].wireends()['2']));
    _poscalc = shared_ptr<PositionCalculator>(new XYCalc);
    break;
  case (uv):
    _layerCombination = make_pair(make_pair(&d.layers()['U'].wireends()['1'],
                                            &d.layers()['U'].wireends()['2']),
                                  make_pair(&d.layers()['V'].wireends()['1'],
                                            &d.layers()['V'].wireends()['2']));
    _poscalc = shared_ptr<PositionCalculator>(new UVCalc);
    break;
  case (uw):
    _layerCombination = make_pair(make_pair(&d.layers()['U'].wireends()['1'],
                                            &d.layers()['U'].wireends()['2']),
                                  make_pair(&d.layers()['W'].wireends()['1'],
                                            &d.layers()['W'].wireends()['2']));
    _poscalc = shared_ptr<PositionCalculator>(new UWCalc);
    break;
  case (vw):
    _layerCombination = make_pair(make_pair(&d.layers()['V'].wireends()['1'],
                                            &d.layers()['V'].wireends()['2']),
                                  make_pair(&d.layers()['W'].wireends()['1'],
                                            &d.layers()['W'].wireends()['2']));
    _poscalc = shared_ptr<PositionCalculator>(new VWCalc);
    break;
  default:
    {
      stringstream ss;
      ss <<"DelaylineDetectorAnalyzerSimple::loadSettings: Layercombination '"<<lc<<"' not available";
      throw invalid_argument(ss.str());
    }
      break;
  }
  _sf = make_pair(s.value("ScalefactorFirstLayer",0.4).toDouble(),
                  s.value("ScalefactorSecondLayer",0.4).toDouble());
  s.endGroup();
}
