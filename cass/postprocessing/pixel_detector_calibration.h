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

}//end namespace cass
#endif
