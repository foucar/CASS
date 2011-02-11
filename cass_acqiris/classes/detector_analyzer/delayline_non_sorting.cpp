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

using namespace std;
using namespace cass::ACQIRIS;
using namespace std::tr1;

DelaylineNonSorting::DelaylineNonSorting()
  :DetectorAnalyzerBackend()
{}

detectorHits_t& DelaylineNonSorting::operator()(detectorHits_t &hits)
{
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
