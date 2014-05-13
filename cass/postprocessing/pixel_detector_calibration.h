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
 * The output of this PostProcessor consits of 3 images stiched together along
 * the y (slow) coordinate. The ouput has the same x width as the input, but 3
 * times the y width as the input.
 *
 * Y from 0 * y_input .. 1 * y_input-1: The calculated Gain Map
 * Y from 1 * y_input .. 2 * y_input-1: The photons counts map
 * Y from 2 * y_input .. 3 * y_input-1: The Average value of the photons of
 *                                      interest in that pixel
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
 * @cassttng PostProcessor/\%name\%/{NbrOfFrames} \n
 *           The number of frames after which the gain map will be calculated.
 *           Default is -1, which sais that it will never be calulated during
 *           running and only when the program ends.
 * @cassttng PostProcessor/\%name\%/{PnCCDNoCTE}\n
 *           If the detector is a pnCCD and one doesn't want to correct for the
 *           CTE this option will calcultate the gain for a column of a quadrant
 *           of the detector. Default is false
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

  /** receive commands from the gui */
  virtual void processCommand(std::string command);

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

  /** the number of frames after which the gain map is calculted */
  int _nFrames;

  /** counter to count how many times this has been called */
  int _counter;

  /** offset to the gain part */
  size_t _gainOffset;

  /** offset to the counts part */
  size_t _countOffset;

  /** offset to the average part */
  size_t _aveOffset;

  /** the size of the input image */
  size_t _sizeOfImage;

  /** flag to tell whether its a pnCCD we are not interested in correction the cte */
  bool _isPnCCDNoCTE;
};










/** pixel detector hot pixel detection
 *
 * @PPList "332": pixel detector hot pixel detection
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
 * @cassttng PostProcessor/\%name\%/{MaximumConsecutiveFrames} \n
 *           The maximum number of frames that a pixel should have an adu value
 *           in the range before the pixel is mased as hot. Default is 5
 * @cassttng PostProcessor/\%name\%/{MaxADUValue} \n
 *           If a pixel ever exceeds this value it will be masked as bad.
 *           Default is 1e6
 * @cassttng PostProcessor/\%name\%/{NbrOfFrames} \n
 *           The number of frames after which the gain map will be calculated.
 *           Default is -1, which sais that it will never be calulated during
 *           running and only when the program ends.
 *
 * @author Lutz Foucar
 */
class pp332 : public AccumulatingPostProcessor
{
public:
  /** constructor. */
  pp332(const name_t&);

  /** overwrite default behaviour don't do anything */
  virtual void process(const CASSEvent&, HistogramBackend&);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

  /** write the calibrations before quitting */
  virtual void aboutToQuit();

protected:
  /** write the calibration data to file */
  void loadHotPixelMap();

  /** write the calibration data to file */
  void writeHotPixelMap();

private:
  /** define the output mask type */
  typedef char mask_t;

  /** the image to create the hotpixel map from */
  shared_pointer _image;

  /** the number of times a pixel is high before masking it as hot pixel */
  size_t _maxConsecutiveCount;

  /** define the range type */
  typedef std::pair<float,float> range_t;

  /** the range of adu that indicates whether a pixel is hot */
  range_t _aduRange;

  /** the maximum allowed adu value */
  float _maxADUVal;

  /** flag to tell whether the calibration should be written */
  bool _write;

  /** the filename that is used to save the calibration */
  std::string _filename;

  /** the number of frames after which the gain map is calculted */
  int _nFrames;

  /** counter to count how many times this has been called */
  int _counter;
};







/** pixel detector common mode background calculation
 *
 * @PPList "333": pixel detector hot pixel detection
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{Image} \n
 *           the image of the pixel detector
 * @cassttng PostProcessor/\%name\%/{Width} \n
 *           The width of the subpart of the detector for which the common
 *           mode level needs to be calculated.
 * @cassttng PostProcessor/\%name\%/{CalculationType} \n
 *           The type of calculation used to calculate the common mode level.
 *           Default is "mean". Possible values are:
 *           - "mean": Iterative retrieval of the mean value of distribution of
 *                     pixels. In each iteration the outliers of the distribution
 *                     are remove from the distribution.
 *           - "median": Calculates the mean of the distribution of pixels.
 * @cassttng PostProcessor/\%name\%/{SNR} \n
 *           In case of using the mean calculator the signal to noise ratio is
 *           indicating which values to remove from the distribution before
 *           recalculating the mean. Default is 4
 *
 * @author Lutz Foucar
 */
class pp333 : public PostProcessor
{
public:
  /** constructor. */
  pp333(const name_t&);

  /** overwrite default behaviour don't do anything */
  virtual void process(const CASSEvent&, HistogramBackend&);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

protected:
  /** function to calulate the common mode level as mean value
   *
   * @return the commonmode level
   * @param begin iterator to the beginning of the distribution
   * @param end iterator to the end of the distribution
   */
  float meanCalc(HistogramFloatBase::storage_t::const_iterator begin,
                 HistogramFloatBase::storage_t::const_iterator end);

  /** function to calulate the common mode level via the median value
   *
   * @return the commonmode level
   * @param begin iterator to the beginning of the distribution
   * @param end iterator to the end of the distribution
   */
  float medianCalc(HistogramFloatBase::storage_t::const_iterator begin,
                   HistogramFloatBase::storage_t::const_iterator end);
private:
  /** the image to create the hotpixel map from */
  shared_pointer _image;

  /** the number of times a pixel is high before masking it as hot pixel */
  size_t _width;

  /** the signal to noise ratio in case one uses the mean calculator */
  float _snr;

  /** the function that calculates the commond mode level */
  std::tr1::function<float(HistogramFloatBase::storage_t::const_iterator,
                           HistogramFloatBase::storage_t::const_iterator)> _calcCommonmode;
};

}//end namespace cass
#endif
