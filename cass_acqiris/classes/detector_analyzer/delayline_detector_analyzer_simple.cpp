//Copyright (C) 2003-2010 Lutz Foucar

/**
 * @file delayline_detector_analyzer_simple.cpp file contains the definition of
 *                                              classes and functions that
 *                                              analyzses a delayline detector.
 *
 * @author Lutz Foucar
 */

#include <iostream>
#include <limits>
#include <cmath>
#include <stdexcept>
#include <algorithm>

#include "delayline_detector_analyzer_simple.h"
#include "delayline_detector.h"
#include "channel.h"
#include "signal_producer.h"
#include "cass_settings.h"

using namespace cass::ACQIRIS;
namespace cass
{
  namespace ACQIRIS
  {
    //@{
    /** typedefs for shorter code */
    typedef SignalProducer::signal_t signal_t;
    typedef SignalProducer::signals_t signals_t;
    typedef signals_t::iterator sigIt_t;
    typedef std::pair<sigIt_t,sigIt_t> range_t;
    //@}

    /** check whether anode end wire signal is correleated to mcp signal
     *
     * see getSignalRange() for details
     *
     * @author Lutz Foucar
     */
    struct isInRange : std::unary_function<signal_t,bool>
    {
      /** constructor
       *
       * @param mcp the time of the mcp signal
       * @param timesum the timesum of the anode layer
       * @param runtime the time it takes a signal to run across the whole anode
       */
      isInRange(double mcp, double timesum, double maxruntime)
        :_mcp(mcp),_timesum(timesum),_maxruntime(maxruntime)
      {}

      /** check correlation
       *
       * check whether signal can be correlated with the mcp signal time
       *
       * @return true when signal can be correlated
       * @param sig the signal which needs to be checked for correlation
       */
      bool operator()(const signal_t &sig) const
      {
        return fabs(2.*sig["time"] - 2.*_mcp - _timesum) <= _maxruntime;
      }

    private:
      double _mcp;        //!< the time of the mcp signal
      double _timesum;    //!< the timesum of the anode layer
      double _maxruntime; //!< the time it takes the signal to run across the anode
    };

    /** return range of possible anode wire signal candidates
     *
     * For a given Mcp time there are only a few signal on the wire ends that
     * can come with the Mcp Signal. This function will find the indexs of the
     * list of signals which might come together with the mcp signal.
     * This is because we know two things (ie. for the x-layer):
     * \f$|x_1-x_2|<rTime_x\f$
     * and
     * \f$x_1+x_2-2*mcp = ts_x\f$
     * with this knowledge we can calculate the boundries for the anode given
     * the Timesum and the Runtime.
     *
     * @return pair of iterator that define the range
     * @param sigs the vector of signals of the anode wire end
     * @param mcp the Mcp Signal for which to find the right wire end signals
     * @param ts The timesum of the Anode
     * @param rTime The runtime of a Signal over the whole wire of the anode
     *
     * @author Lutz Foucar
     */
    std::pair<sigIt_t,sigIt_t> getSignalRange(signals_t &sigs, const double mcp, const double ts, const double rTime)
    {
      using namespace std;

      sigIt_t begin (find_if(sigs.begin(),sigs.end(),isInRange(mcp,ts,rTime)));
      sigIt_t end (find_if(begin, sigs.end(),not1(isInRange(mcp,ts,rTime))));
      return (make_pair(begin,end));
    }

    /** position calculator base class
     *
     * @author Lutz Foucar
     */
    class PositionCalculator
    {
    public:
      virtual ~PositionCalculator() {}
      virtual std::pair<double,double> operator()(const std::pair<double, double>&)=0;
    };

    /** position calculator for quad anode
     *
     * @author Lutz Foucar
     */
    class XYCalc : public PositionCalculator
    {
    public:
      virtual std::pair<double,double> operator()(const std::pair<double, double>& layer)
      {
        return layer;
      }
    };

    /** position calculator for hex anodes u and v layer
     *
     * @author Lutz Foucar
     */
    class UVCalc : public PositionCalculator
    {
    public:
      virtual std::pair<double,double> operator()(const std::pair<double, double>& layer)
      {
        const double u(layer.first);
        const double v(layer.second);
        return std::make_pair(u, 1./std::sqrt(3) * (u-2.*v));
      }
    };

    /** position calculator for hex anodes u and w layer
     *
     * @author Lutz Foucar
     */
    class UWCalc : public PositionCalculator
    {
    public:
      virtual std::pair<double,double> operator()(const std::pair<double, double>& layer)
      {
        const double u(layer.first);
        const double w(layer.second);
        return std::make_pair(u, 1./std::sqrt(3) * (2.*w-u));
      }
    };

    /** position calculator for hex anodes u and v layer
     *
     * @author Lutz Foucar
     */
    class VWCalc : public PositionCalculator
    {
    public:
      virtual std::pair<double,double> operator()(const std::pair<double, double>& layer)
      {
        const double v(layer.first);
        const double w(layer.second);
        return std::make_pair(v+w, 1./std::sqrt(3) * (w-v));
      }
    };

    class RotateX
    {
    public:
      RotateX(const double &angle)
        : _sinangle(sin(angle*3.14/180.)),
          _cosangle(cos(angle*3.14/180.))
      {}
      detectorHit_t & operator()(detectorHit_t & hit)const
      {
        hit["x_rot"]	= ( _cosangle*hit["x"] + _sinangle*hit["y"] );
        return hit;
      }
    private:
      double _sinangle;
      double _cosangle;
    };
    class RotateY
    {
    public:
      RotateY(const double &angle)
        : _sinangle(sin(angle*3.14/180.)),
          _cosangle(cos(angle*3.14/180.))
      {}
      detectorHit_t& operator()(detectorHit_t & hit)const
      {
        hit["y_rot"]	= (-_sinangle*hit["x"] + _cosangle*hit["y"] );
        return hit;
      }
    private:
      double _sinangle;
      double _cosangle;
    };

    class CorrectX
    {
    public:
      CorrectX(const double &cor)
        :_cor(cor)
      {}
      detectorHit_t & operator()(detectorHit_t &hit)const
      {
        hit["x_cor"] = hit["x"] - _cor;
        return hit;
      }
    private:
      double _cor;
    };

    class CorrectY
    {
    public:
      CorrectY(const double &cor)
        :_cor(cor)
      {}
      detectorHit_t& operator()(detectorHit_t &hit)const
      {
        hit["y_cor"] = hit["y"] - _cor;
        return hit;
      }
    private:
      double _cor;
    };
  }
}

detectorHits_t& DelaylineDetectorAnalyzerSimple::operator()(detectorHits_t &hits)
{
  using namespace std;
  using namespace cass::ACQIRIS;
  typedef SignalProducer::signal_t signal_t;
  typedef SignalProducer::signals_t signals_t;
  typedef signals_t::iterator sigIt_t;
  typedef std::pair<sigIt_t,sigIt_t> range_t;

  SignalProducer::signals_t &mcpsignals (_mcp->output());
  SignalProducer::signals_t &f1signals (_layerCombination.first.first->output());
  SignalProducer::signals_t &f2signals (_layerCombination.first.second->output());
  SignalProducer::signals_t &s1signals (_layerCombination.second.first->output());
  SignalProducer::signals_t &s2signals (_layerCombination.second.second->output());

  for (sigIt_t iMcp (mcpsignals.begin());iMcp != mcpsignals.end() ;++iMcp)
  {
    if ((*iMcp)["isUsed"] > sqrt(numeric_limits<signal_t::value_type>::epsilon())) continue;
    const double mcp ((*iMcp)["time"]);
    range_t f1range(getSignalRange(f1signals,mcp,_ts.first,_runtime));
    range_t f2range(getSignalRange(f2signals,mcp,_ts.first,_runtime));
    range_t s1range(getSignalRange(s1signals,mcp,_ts.second,_runtime));
    range_t s2range(getSignalRange(s2signals,mcp,_ts.second,_runtime));
    for (sigIt_t iF1 (f1range.first);iF1!=f1range.second;++iF1)
    {
      if ((*iF1)["isUsed"] > sqrt(numeric_limits<signal_t::value_type>::epsilon())) continue;
      for (sigIt_t iF2 (f2range.first);iF2!=f2range.second;++iF2)
      {
        if ((*iF2)["isUsed"] > sqrt(numeric_limits<signal_t::value_type>::epsilon())) continue;
        for (sigIt_t iS1 (s1range.first);iS1!=s1range.second;++iS1)
        {
          if ((*iS1)["isUsed"] > sqrt(numeric_limits<signal_t::value_type>::epsilon())) continue;
          for (sigIt_t iS2 (s2range.first);iS2!=s2range.second;++iS2)
          {
            if ((*iS2)["isUsed"] > sqrt(numeric_limits<signal_t::value_type>::epsilon())) continue;

            const double mcp ((*iMcp)["time"]);

            const double f1 ((*iF1)["time"]);
            const double f2 ((*iF2)["time"]);
            const double s1 ((*iS1)["time"]);
            const double s2 ((*iS2)["time"]);
            const double sumf (f1+f2 - 2.* mcp);
            const double sums (s1+s2 - 2.* mcp);
            const double f((f1-f2) * _sf.first);
            const double s((s1-s2) * _sf.second);

            const pair<double,double> pos ((*_poscalc)(make_pair(f,s)));

            const double radius (sqrt(pos.first*pos.first + pos.second*pos.second));

            if ( (sumf > _tsrange.first.first) && (sumf < _tsrange.first.second) )
            {
              if ( (sums > _tsrange.second.first) && (sums < _tsrange.second.second) )
              {
                if (radius < _mcpRadius)
                {
                  detectorHit_t hit;
                  hit["x"] = pos.first;
                  hit["y"] = pos.second;
                  hit["t"] = (*iMcp)["time"];
                  hits.push_back(hit);
                  (*iMcp)["isUsed"] = true;
                  (*iF1)["isUsed"] = true;
                  (*iF2)["isUsed"] = true;
                  (*iS1)["isUsed"] = true;
                  (*iS2)["isUsed"] = true;
                }
              }
            }
          }
        }
      }
    }
  }
  return hits;
}

void DelaylineDetectorAnalyzerSimple::loadSettings(CASSSettings& s, DelaylineDetector &d)
{
  using namespace std;
  enum LayerComb{xy,uv,uw,vw};

  DelaylineType delaylinetype
      (static_cast<DelaylineType>(s.value("DelaylineType",Hex).toInt()));

  s.beginGroup("Simple");
  LayerComb lc (static_cast<LayerComb>(s.value("LayersToUse",xy).toInt()));
  if (lc == xy)
  {
    if (delaylinetype == Hex)
    {
      stringstream ss;
      ss << "DelaylineDetectorAnalyzerSimple::loadSettings: Error using layers xy for Hex-Detector";
      throw invalid_argument(ss.str());
    }
  }
  if (delaylinetype == Quad)
  {
    if (lc == uv || lc == uw || lc == vw)
    {
      stringstream ss;
      ss << "DelaylineDetectorAnalyzerSimple::loadSettings: Error using layers uv, uw or vw for Quad-Detector";
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
    _poscalc = auto_ptr<PositionCalculator>(new XYCalc);
    break;
  case (uv):
    _layerCombination = make_pair(make_pair(&d.layers()['U'].wireends()['1'],
                                            &d.layers()['U'].wireends()['2']),
                                  make_pair(&d.layers()['V'].wireends()['1'],
                                            &d.layers()['V'].wireends()['2']));
    _poscalc = auto_ptr<PositionCalculator>(new UVCalc);
    break;
  case (uw):
    _layerCombination = make_pair(make_pair(&d.layers()['U'].wireends()['1'],
                                            &d.layers()['U'].wireends()['2']),
                                  make_pair(&d.layers()['W'].wireends()['1'],
                                            &d.layers()['W'].wireends()['2']));
    _poscalc = auto_ptr<PositionCalculator>(new UWCalc);
    break;
  case (vw):
    _layerCombination = make_pair(make_pair(&d.layers()['V'].wireends()['1'],
                                            &d.layers()['V'].wireends()['2']),
                                  make_pair(&d.layers()['W'].wireends()['1'],
                                            &d.layers()['W'].wireends()['2']));
    _poscalc = auto_ptr<PositionCalculator>(new VWCalc);
    break;
  default:
    {
      stringstream ss;
      ss <<"DelaylineDetectorAnalyzerSimple::loadSettings: Layercombination '"<<lc<<"' not available";
      throw invalid_argument(ss.str());
    }
      break;
  }
  _mcp = &d.mcp();
  _tsrange = make_pair(make_pair(s.value("TimesumFirstLayerLow",0).toDouble(),
                                 s.value("TimesumFirstLayerHigh",200).toDouble()),
                       make_pair(s.value("TimesumSecondLayerLow",0).toDouble(),
                                 s.value("TimesumSecondLayerHigh",200).toDouble()));
  _ts = make_pair(0.5*(_tsrange.first.first + _tsrange.first.second),
                  0.5*(_tsrange.second.first + _tsrange.second.second));
  _sf = make_pair(s.value("ScalefactorFirstLayer",0.4).toDouble(),
                  s.value("ScalefactorSecondLayer",0.4).toDouble());
  _runtime = s.value("Runtime",150).toDouble();
  _mcpRadius = s.value("McpRadius",88).toDouble();
  s.endGroup();
}
