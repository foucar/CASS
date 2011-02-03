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
#include <stdint.h>

#include "delayline_detector_analyzer_backend.h"
#include "delayline_detector.h"


class sort_class;
class sum_walk_calibration_class;
class scalefactors_calibration_class;

namespace cass
{
  namespace ACQIRIS
  {
    /** Achims resort routine wrapper
     *
     * this class will use achims resort routine to calculate the detectorhits
     * from the signals on the wireends and the mcp.
     *
     * @todo add a function that will write all the settings to a file.
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT HexSorter
      : public DetectorAnalyzerBackend
    {
    public:
      /** constructor
       *
       * creates and intitializes the achims routine
       */
      HexSorter();

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

      /** typedefs */
      typedef std::tr1::shared_ptr<sum_walk_calibration_class> tsumcalibratorPtr_t;
      typedef std::tr1::shared_ptr<scalefactors_calibration_class> scalefactorcalibratorPtr_t;
    private:
      /** container for tdc like arrays mapped to the corrosponding signalproducer */
      std::vector<std::pair<SignalProducer*,std::vector<double> > > _signals;

      /** the instance of Achims routine */
      std::tr1::shared_ptr<sort_class> _sorter;

      /** counter array for achims routine
       *
       * this is used so that the routine knows how many singals are in each
       * array
       */
      std::vector<int32_t> _count;

      /** the timesums */
      std::vector<double> _timesums;

      /** the center of the detector */
      std::pair<double,double> _center;

      /** the w-layer offset */
      double _wLayerOffset;

      /** the time sum calibrator
       *
       * this will take the timesum and after a while it knows how to correct
       * the timesum to be a straight line
       */
      tsumcalibratorPtr_t _tsum_calibrator;

      /** pointer to scalfactor calibrator
       *
       * this is a class that will help finding the scalefactor and the
       * w-Layer offset of the Hex-Anode.
       */
      scalefactorcalibratorPtr_t _scalefactor_calibrator;

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
