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

namespace cass
{
namespace ACQIRIS
{
/** Achims resort routine wrapper
 *
 * this class will use achims resort routine to calculate the detectorhits
 * from the signals on the wireends and the mcp. It needs to be used closely
 * together with PostProcessor 170 (cass::HexCalibrator) that will calculate
 * parameters for full functionality of achims resort routine.
 *
 * @cassttng AcqirisDetectors/\%detectorname\%/HexSorting/{TimeSumU}\n
 *           Center of the timesum of layer U. Default is 100.
 * @cassttng AcqirisDetectors/\%detectorname\%/HexSorting/{TimeSumUWidth}\n
 *           With at the base of timesum of layer U. Default is 0.
 * @cassttng AcqirisDetectors/\%detectorname\%/HexSorting/{TimeSumV}\n
 *           Center of the timesum of layer W. Default is 100.
 * @cassttng AcqirisDetectors/\%detectorname\%/HexSorting/{TimeSumVWidth}\n
 *           With at the base of timesum of layer U. Default is 0.
 * @cassttng AcqirisDetectors/\%detectorname\%/HexSorting/{TimeSumW}\n
 *           Center of the timesum of layer W. Default is 100.
 * @cassttng AcqirisDetectors/\%detectorname\%/HexSorting/{TimeSumWWidth}\n
 *           With at the base of timesum of layer U. Default is 0.
 * @cassttng AcqirisDetectors/\%detectorname\%/HexSorting/{MaxRuntime}\n
 *           the maximum time it takes a signal to get across the delayline wire.
 *           Default is 150.
 * @cassttng AcqirisDetectors/\%detectorname\%/HexSorting/{DeadTimeAnode}\n
 *           the deadtime betwenn 2 anode singals in ns. Default is 20.
 * @cassttng AcqirisDetectors/\%detectorname\%/HexSorting/{DeadTimeMCP}\n
 *           the deadtime between 2 mcp signals in ns. Default is 20.
 * @cassttng AcqirisDetectors/\%detectorname\%/HexSorting/{MCPRadius}\n
 *           The radius of the MCP in mm. Should be ~10 percent bigger that the
 *           actual value. Default is 88.
 * @cassttng AcqirisDetectors/\%detectorname\%/HexSorting/{UseMCP}\n
 *           use the mcp signal when reconstructing the detector hits. Default
 *           is 'true'
 * @cassttng AcqirisDetectors/\%detectorname\%/HexSorting/{ScalefactorU}\n
 *           Conversion factor to convert postion from ns to mm. The scalefactors
 *           of the other two layers will be determined by the calibrator and
 *           written into the settingsfile (see parameter SettingsFilename)
 * @cassttng AcqirisDetectors/\%detectorname\%/HexSorting/{CenterX|CenterY}\n
 *           Center position of the detector image, if not centered around 0.
 *           Default is 0|0
 * @cassttng AcqirisDetectors/\%detectorname\%/HexSorting/{SettingsFilename}\n
 *           Name of the file that contains the advanced settings. This will be
 *           created by the PostProcessor that does the Hex calibration (pp170).
 *           See cass::HexCalibrator for descriptions of this PostProcessor.
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

private:
  /** container for tdc like arrays mapped to the corrosponding signalproducer
   *
   * the order in the vector is as follows:
   * - 0: mcp
   * - 1: u1
   * - 2: u2
   * - 3: v1
   * - 4: v2
   * - 5: w1
   * - 6: w2
   */
  std::vector<std::pair<SignalProducer*,std::vector<double> > > _signals;

  /** the instance of Achims routine */
  std::tr1::shared_ptr<sort_class> _sorter;

  /** counter array for achims routine
   *
   * this is used so that the routine knows how many signals are in each
   * array
   */
  std::vector<int32_t> _count;

  /** the timesums
   *
   * the timesums in this containers are layed out as follows:
   * - 0: u layer
   * - 1: v layer
   * - 2: w layer
   */
  std::vector<double> _timesums;

  /** the center of the detector */
  std::pair<double,double> _center;

  /** the w-layer offset */
  double _wLayerOffset;
};

}//end namespace acqiris
}//end namespace cass
#endif
