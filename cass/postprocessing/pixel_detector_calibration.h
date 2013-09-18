// Copyright (C) 2013 Lutz Foucar

#ifndef _PIXELDETECTORCALIBRATION_H_
#define _PIXELDETECTORCALIBRATION_H_

#include <valarray>

#include "processor.h"

#include "statistics_calculator.hpp"

namespace cass
{
//forward declaration
class CASSEvent;

/** pixel detector calibrations
 *
 * @PPList "330": pixel detector calibrations
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{RawImage} \n
 *           the raw image of the pixel detector
 * @cassttng PostProcessor/\%name\%/{Filename} \n
 *           the name of the file where the calibration will be written to.
 *           Default is out.cal
 * @cassttng PostProcessor/\%name\%/{WriteCal} \n
 *           Flag to tell whether the calibration should be written. Default is
 *           true.
 * @cassttng PostProcessor/\%name\%/{Train} \n
 *           Flag to tell whether training should be done. Default is true
 * @cassttng PostProcessor/\%name\%/{NbrTrainingImages} \n
 *           The Number of images used for the training. Default is 200
 * @cassttng PostProcessor/\%name\%/{SNR} \n
 *           The signal to noise value that indicates whether a pixel is an
 *           outlier of the distribution and should not be considered. Default
 *           is 4.
 *
 * @author Lutz Foucar
 */
class pp330 : public AccumulatingPostProcessor
{
public:
  /** constructor. */
  pp330(const name_t&);

  /** overwrite default behaviour don't do anything */
  virtual void process(const CASSEvent&, HistogramBackend&);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

  /** write the calibrations before quitting */
  virtual void aboutToQuit();

protected:
  /** write the calibration data to file */
  void loadCalibration();

  /** write the calibration data to file */
  void writeCalibration();

private:
  /** define the statistics type */
  typedef MovingStatisticsCalculator<std::valarray<float> > stat_t;

private:
  /** the raw image */
  shared_pointer _image;

  /** flag telling whether training is needed */
  bool _train;

  /** the storage for the training images */
  std::vector< std::vector<float> > _trainstorage;

  /** the number of training images acquired */
  size_t _nTrainImages;

  /** the minimum nbr of images necessary for training */
  size_t _minTrainImages;

  /** flag to tell whether the calibration should be written */
  bool _write;

  /** the value above which outliers are removed from the distribution */
  float _snr;

  /** the filename that is used to save the calibration */
  std::string _filename;
};





/** pixel detector gain calibrations
 *
 * @PPList "331": pixel detector gain calibrations
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{Image} \n
 *           the image of the pixel detector
 * @cassttng PostProcessor/\%name\%/{Filename} \n
 *           the name of the file where the calibration will be written to.
 *           Default is out.cal
 * @cassttng PostProcessor/\%name\%/{WriteCal} \n
 *           Flag to tell whether the calibration should be written. Default is
 *           true.
 * @cassttng PostProcessor/\%name\%/{ADURangeLow|ADURangeUp} \n
 *           The adu range that indicates that one photon has hit the pixel.
 *           Default is 0|0
 * @cassttng PostProcessor/\%name\%/{MinimumNbrPhotons} \n
 *           The minimum number of photons that a pixel should have seen before
 *           the gain is calulated for this pixel. Default is 200
 * @cassttng PostProcessor/\%name\%/{DefaultGainValue} \n
 *           The gain value that will be assinged to the pixels that haven't
 *           seen enough photons. Default is 1.
 *
 * @author Lutz Foucar
 */
class pp331 : public AccumulatingPostProcessor
{
public:
  /** constructor. */
  pp331(const name_t&);

  /** overwrite default behaviour don't do anything */
  virtual void process(const CASSEvent&, HistogramBackend&);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

  /** write the calibrations before quitting */
  virtual void aboutToQuit();

protected:
  /** write the calibration data to file */
  void loadCalibration();

  /** write the calibration data to file */
  void writeCalibration();

  /** write the calibration data to file */
  void calculateGainMap(Histogram2DFloat& gainmap);

private:
  /** the raw image */
  shared_pointer _image;

  /** definition of the statistic */
  typedef std::pair<size_t,double> statistic_t;

  /** defintion of the statistics */
  typedef std::vector<statistic_t> statistics_t;

  /** the statistics for each pixel */
  statistics_t _statistics;

  /** the number of photons that a pixel should have seen before calculating the gain */
  size_t _minPhotonCount;

  /** define the range type */
  typedef std::pair<float,float> range_t;

  /** the range of adu that indicates whether a pixel contains a photon */
  range_t _aduRange;

  /** the gain value that will be assinged to the pixel that one could not
   *  calculate the gain for
   */
  float _constGain;

  /** flag to tell whether the calibration should be written */
  bool _write;

  /** the filename that is used to save the calibration */
  std::string _filename;
};

}//end namespace cass
#endif
