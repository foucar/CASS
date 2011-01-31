// Copyright (C) 2008-2011 Lutz Foucar

/**
 * @file achimsorter_hex.h file contains class that uses achims resort routine
 *
 * @author Lutz Foucar
 */

#ifndef _ACHIMSORTER_HEX_H_
#define _ACHIMSORTER_HEX_H_

#include <tr1/memory>
#include <vector>
#include <utility>

#include "delayline_detector_analyzer_backend.h"
#include "delayline_detector.h"

namespace cass
{
  namespace ACQIRIS
  {
    /** Achims resort routine wrapper
     *
     * this class will use achims resort routine to calculate the detectorhits
     * from the signals on the wireends and the mcp.
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT HexSorter
      : public DetectorAnalyzerBackend
    {
    public:
      /** constructor */
      HexSorter()
        :DetectorAnalyzerBackend()
      {}

      /** the function creating the detectorhit list
       *
       * @return reference to the hit container
       * @param hits the hitcontainer
       */
      detectorHits_t& operator()(detectorHits_t &hits);

      /** load the detector analyzers settings from .ini file
       *
       * retrieve all necessary information to be able to sort the signals of
       * the detector to detector hits. Also retrieve the signal producers of
       * the layers whos signals we should sort.
       *
       * @param s the CASSSetting object
       * @param d the detector object that we are belonging to
       */
      void loadSettings(CASSSettings &s, DelaylineDetector &d);

    private:
      /** container for tdc like arrays mapped to the corrosponding signalproducer */
      std::vector<std::pair<SignalProducer*,std::vector<double> > > _signals;
    };

//    class MyDetektorHitSorterAchimHex : public MyDetektorHitSorterHex, public MyDetektorHitSorterAchim
//    {
//    public:
//      MyDetektorHitSorterAchimHex(const MyDetektorInfo&, MyHistos&, int HiOff);
//
//    public:
//      virtual void Sort(MySignalAnalyzedEvent &sae, MyDetektor &d, MyHistos &rm)	{SortImpl(sae,d,rm);}
//      void WriteCalibData(const MyDetektorInfo&);
//
//    protected:
//      void SortImpl(MySignalAnalyzedEvent&, MyDetektor&, MyHistos&);
//      void Shift(const MyDetektor&);
//      void CreateDetHits(MyDetektor&, MyHistos&);
//      void FillHistosAfterShift(const MyDetektor&, MyHistos&);
//      void CreateTDCArrays();
//      void Calibrate(const MyDetektor&, MyHistos&);
//    };
//
//    class MyDetektorHitSorterAchimHexCalib : public MyDetektorHitSorterAchimHex
//    {
//    public:
//      MyDetektorHitSorterAchimHexCalib(const MyDetektorInfo &di, MyHistos &rm, int HiOff):
//          MyDetektorHitSorterAchimHex(di,rm,HiOff) {}
//
//    public:
//      void Sort(MySignalAnalyzedEvent &sae, MyDetektor &d, MyHistos &rm)	{SortImpl(sae,d,rm); Calibrate(d,rm);}
//    };
  }
}
#endif
