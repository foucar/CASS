// Copyright (C) 2013 Lutz Foucar

/** @file hitfinder.h contains postprocessors that will extract pixels of
 *                    interrest from 2d histograms.
 * @author Lutz Foucar
 */

#ifndef _HITFINDER_H_
#define _HITFINDER_H_

#include <tr1/functional>

#include "processor.h"
#include "cass_event.h"
#include "histogram.h"
#include "statistics_calculator.hpp"



namespace cass
{


/** get the local background from image.
 *
 * @PPList "203": get the local background from image
 *
 * splits up the image into sections of user choosable size. In each of these
 * sections the local background is determined by taking a box of a user
 * choosable size and determining the median of the pixel values inside this
 * box.
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{HistName} \n
 *           the postprocessor name that contain the 2d histogram. Default
 *           is "blubb".
 * @cassttng Processor/\%name\%/{SectionSizeX|SectionSizeY} \n
 *           Size of the subsection of the image. Default is 1024|512.
 * @cassttng Processor/\%name\%/{BoxSizeX|BoxSizeY} \n
 *           size in x and y of the box that is used for determining the median
 *           background. Default is 10|10.
 *
 * @author Lutz Foucar
 * @author Wolfgang Kabsch
 */
class pp203 : public Processor
{
public:
  /** constructor */
  pp203(const name_t &);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

protected:
  /** pp containing 2d histogram */
  shared_pointer _hist;

  /** the size of the box used for the median filter */
  std::pair<size_t,size_t> _boxSize;

  /** size of a image section */
  std::pair<size_t,size_t> _sectionSize;
};






/** find bragg peaks and store them in a list
 *
 * @PPList "204": find bragg peaks and store them in a list
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{HistName} \n
 *           the postprocessor name that contain the 2d histogram. Default
 *           is "blubb".
 * @cassttng Processor/\%name\%/{SectionSizeX|SectionSizeY} \n
 *           Size of the subsection of the image. Default is 1024|512.
 * @cassttng Processor/\%name\%/{BoxSizeX|BoxSizeY} \n
 *           size in x and y of the box that is used for determining the median
 *           background. Default is 10|10.
 * @cassttng Processor/\%name\%/{Threshold} \n
 * @cassttng Processor/\%name\%/{MinSignalToNoiseRatio} \n
 * @cassttng Processor/\%name\%/{MinNbrBackgrndPixels} \n
 * @cassttng Processor/\%name\%/{BraggPeakRadius} \n
 *
 * @author Lutz Foucar
 */
class pp204 : public Processor
{
public:
  /** constructor */
  pp204(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

protected:
  /** postprocessor containing the image to find the bragg peaks in */
  shared_pointer _hist;

  /** definition of the table */
  typedef HistogramFloatBase::storage_t table_t;

public:
  /** enum describing the contents of the resulting table */
  enum ColumnNames
  {
    Intensity                 =  0,
    centroidColumn            =  1,
    centroidRow               =  2,
    nbrOfPixels               =  3,
    SignalToNoise             =  4,
    Index                     =  5,
    Column                    =  6,
    Row                       =  7,
    LocalBackground           =  8,
    LocalBackgroundDeviation  =  9,
    nbrOfBackgroundPixels     = 10,
    MaxRadius                 = 11,
    MinRadius                 = 12,
    MaxADU                    = 13,
    nbrOf
  };

protected:
  /** define the values of the pixels */
  typedef  HistogramFloatBase::storage_t::value_type pixelval_t;

  /** define the positions in the image */
  typedef int imagepos_t;

  /** check highest pixel and generate the mean and standart deviation
   *
   * function is used in SNR peak finder
   *
   * Check if the center pixel is heigher than all other pixels in the box. If
   * this is the case return 0. If there is at least one pixel whos value is
   * higher than the center pixel return 1.
   *
   * Generate the mean and standart deviation within the box around the center
   * pixel. Only take pixels into account that are outside of the peak radius.
   *
   * @return 0 if all pixels in the box are lower than the center pixel, 1 otherwise
   * @param centerPixel iterator to the center pixel
   * @param nColumns the number of columns in the image
   * @param[out] mean contains the mean value
   * @param[out] stdv contains the standart deviation
   * @param[out] count contains the number of pixels that were used to calculate
   *                   mean and stdv
   */
  int getBoxStatistics(HistogramFloatBase::storage_t::const_iterator centerPixel,
                       const imagepos_t nColumns,
                       pixelval_t &mean, pixelval_t &stdv, int &count);

  /** the size of the box within which the peak should lie */
  std::pair<imagepos_t,imagepos_t> _box;

  /** size of a image section */
  std::pair<imagepos_t,imagepos_t> _section;

  /** pixel threshold to be exceeded */
  float _threshold;

  /** the square size of bragg peak radius */
  int _peakRadiusSq;

  /** the min signal to noise ratio that needs to be exceeded */
  float _minSnr;

  /** the min signal to noise ratio that needs to be exceeded */
  float _minNeighbourSNR;

  /** min amount of pixels for the background calc */
  int _minBckgndPixels;
};





/** visualize the peaks that were found in the image itself
 *
 * @PPList "205":  visualize the peaks that were found in the image itself
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{HistName} \n
 *           the postprocessor name that contain the 2d histogram. Default
 *           is "blubb".
 * @cassttng Processor/\%name\%/{TableName} \n
 *           name of postprocessor that contains the table like histogram that
 *           the pixels of interest are taken from
 * @cassttng Processor/\%name\%/{BoxSizeX|BoxSizeY} \n
 *           size in x and y of the box that should be drawn around the found
 *           peak
 * @cassttng Processor/\%name\%/{DrawPixelValue} \n
 * @cassttng Processor/\%name\%/{Radius} \n
 * @cassttng Processor/\%name\%/{IndexColumn} \n
 * @cassttng Processor/\%name\%/{DrawCircle} \n
 * @cassttng Processor/\%name\%/{DrawBox} \n
 *
 *
 * @author Lutz Foucar
 * @author Wolfgang Kabsch
 */
class pp205 : public Processor
{
public:
  /** constructor */
  pp205(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

protected:
  /** pp containing 2d histogram */
  shared_pointer _hist;

  /** pp containing the results */
  shared_pointer _table;

  /** draw flags as bitmask */
  HistogramFloatBase::storage_t::value_type _drawVal;
  float _radius;
  std::pair<int,int> _boxsize;
  bool _drawCircle,_drawBox;

  /** the number of the column where the global index of the pixel is */
  size_t _idxCol;
};





/** find pixels of bragg peaks and store them in a list
 *
 * @PPList "206": find pixels of bragg peaks and store them in a list
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{HistName} \n
 *           the postprocessor name that contain the 2d histogram. Default
 *           is "blubb".
 * @cassttng Processor/\%name\%/{SectionSizeX|SectionSizeY} \n
 *           Size of the subsection of the image. Default is 1024|512.
 * @cassttng Processor/\%name\%/{BoxSizeX|BoxSizeY} \n
 *           size in x and y of the box that is used for determining the median
 *           background. Default is 10|10.
 * @cassttng Processor/\%name\%/{Threshold} \n
 * @cassttng Processor/\%name\%/{Multiplier} \n
 *
 * @author Lutz Foucar
 */
class pp206 : public Processor
{
public:
  /** constructor */
  pp206(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

protected:
  /** postprocessor containing the image to find the bragg peaks in */
  shared_pointer _imagePP;

  /** postprocessor containing the noise image for thresholding */
  shared_pointer _noisePP;

  /** definition of the table */
  typedef HistogramFloatBase::storage_t table_t;

protected:
  /** the size of the box within which the peak should lie */
  std::pair<int,int> _box;

  /** size of a image section */
  std::pair<int,int> _section;

  /** pixel threshold to be exceeded */
  float _threshold;

  /** multiplier for the noise threshold */
  float _multiplier;
};









/** image of pixels in the list
 *
 * @PPList "207": createimage of pixels in the list
 *
 * This postprocessor will fill a 2D histogram with the detected pixels that are
 * placed on a list
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
 *           properties of the 2d histogram
 * @cassttng Processor/\%name\%/{Table}\n
 * @cassttng Processor/\%name\%/{ColumnIndex}\n
 * @cassttng Processor/\%name\%/{RowIndex}\n
 * @cassttng Processor/\%name\%/{ValIndex}\n
 *
 * @author Lutz Foucar
 */
class pp207 : public Processor
{
public:
  /** constructor */
  pp207(const name_t&);

  /** copy pixels from CASS event to histogram storage */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** set the histogram size */
  virtual void loadSettings(size_t);

protected:
  /** pp containing the results */
  shared_pointer _table;

  /** the index in the table that indicates the pixels row */
  size_t _pixColIdx;

  /** the index in the table that indicates the pixels row */
  size_t _pixRowIdx;

  /** the index in the table that indicates the pixels row */
  size_t _pixValIdx;
};





/** find bragg peaks and store them in list
 *
 * @PPList "208": find bragg peaks and store them in list
 *
 * Finds bragg peaks by checking how many pixels that are connected are above
 * the given singal to noise ratio. The mean and standart deviation for
 * calculating the SNR is determined by a local box size. Here the statistics
 * is cleaned from the outliers so that pixels that are potentially part of the
 * peak are not included in the background and stdv calculation. If the ratio
 * between outliers of the distribution and pixel that are part of the
 * distribution does not satisfy the user setting, the box size will be
 * increased and the process to determine the local background and std is
 * repeated.
 *
 * This algorithm was inspired by an algorithm described in
 * Zhang et al. J. Appl. Cryst. (2006). 39, 112-119
 * [ doi:10.1107/S0021889805040677 ]
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{HistName} \n
 *           the postprocessor name that contain the 2d histogram. Default
 *           is "blubb".
 * @cassttng Processor/\%name\%/{SectionSizeX|SectionSizeY} \n
 *           Size of the subsection of the image. Default is 1024|512.
 * @cassttng Processor/\%name\%/{BraggPeakDiameter} \n
 *           Minimum Diameter of a Bragg Peak. Used for determining the optimal
 *           box size and nbr of pixels in the Bragg Peak. Default is 2.
 * @cassttng Processor/\%name\%/{MinRatio} \n
 *           The minimum ratio of pixel that are outliers of the distribution
 *           in the box to the pixels that are part of the distribution in the
 *           box. 3 mean that there have to be at least 3 times as meany pixels
 *           that are part of the distribution than outliers of the distribution.
 *           Default is 3.
 * @cassttng Processor/\%name\%/{Threshold} \n
 *           Threshold of pixel in adu to be exeeded. Default is 0.
 * @cassttng Processor/\%name\%/{MinSignalToNoiseRatio} \n
 *           Signal to noise ratio of a pixel. Value needs to be exceeded in
 *           order for the pixel to be part of a bragg peak. Default is 4.
 * @cassttng Processor/\%name\%/{MinNbrPixels} \n
 *           Minimum Nbr of Pixels to be part of a Bragg Peak. Default is
 *           determined by BraggPeakRadius.
 *           \f$ nbr = (2 \times BraggPeakRadius)^2 \f$
 * @cassttng Processor/\%name\%/{BoxSizeX|BoxSizeY} \n
 *           col (x) and rows (y) of the box that is used for determining the
 *           background. The box is going from -BoxSizeX ... BoxSizeX in x and
 *           same in y. Default is determined by the BraggPeakRadius.
 *           \f$ size = \sqrt{\pi} \times BraggPeakRadius \f$
 * @cassttng Processor/\%name\%/{GeometryFilename} \n
 *           The geom file to use. Default is "wrong_file" in which case the
 *           resolution will be calculated completely wrong
 * @cassttng Processor/\%name\%/{ConvertCheetahToCASSLayout} \n
 *           Set this true if the geom file is for a cheetah layout of the data,
 *           but the image in HistName is the image in CASS layout.
 * @cassttng Processor/\%name\%/{Wavelength_A} \n
 *           The wavelength in Angstroem. Can also be the name of a PP that
 *           contains the Wavelength. Default is 1.
 * @cassttng Processor/\%name\%/{DetectorDistance_m} \n
 *           The detector distance in m. Can also be the name of a PP that
 *           contains the detector distance. Default is 60e-2.
 * @cassttng Processor/\%name\%/{PixelSize_m} \n
 *           The pixel size in m. Default is 109.92-6
 *
 * @author Lutz Foucar
 */
class pp208 : public Processor
{
public:
  /** constructor */
  pp208(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &result);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

protected:
  /** postprocessor containing the image to find the bragg peaks in */
  shared_pointer _imagePP;

  /** definition of the table */
  typedef HistogramFloatBase::storage_t table_t;

  /** define the type of the pixel in image */
  typedef Histogram2DFloat::storage_t::value_type pixelval_t;

  /** define the type of statistics used */
  typedef CummulativeStatisticsNoOutlier<pixelval_t> stat_t;

  /** define the index in the image */
  typedef int64_t index_t;

  /** define the shape of the image */
  typedef std::pair<index_t,index_t> shape_t;

  /** define the list of neighbours */
  typedef std::vector<index_t> neighbourList_t;

  /** enum describing the contents of the resulting table */
  enum ColumnNames
  {
    Intensity                 =  0,
    centroidColumn            =  1,
    centroidRow               =  2,
    nbrOfPixels               =  3,
    SignalToNoiseHighestPixel =  4,
    Index                     =  5,
    Column                    =  6,
    Row                       =  7,
    LocalBackground           =  8,
    LocalBackgroundDeviation  =  9,
    MaxRadius                 = 10,
    MinRadius                 = 11,
    MaxADU                    = 12,
    nUpOutliers               = 13,
    Resolution                = 14,
    SignalToNoiseSpot         = 15,
    nbrOf
  };

protected:
  /** retrieve the box statistics
   *
   * Details:
   *
   * @return 0 when all non bad pixels have been added to the statistics.
   *         1 if one of the pixels in the box was higher than the center pixel
   * @param pixel const iterator to the center pixel
   * @param linIdx the linearized index of the pixel
   * @param box the shape of the box to check
   * @param stat the statistics calculator used to determine mean and stdv of
   *             pixels in box
   */
  int getBoxStatistics(HistogramFloatBase::storage_t::const_iterator pixel,
                       const index_t linIdx, const shape_t &box, stat_t &stat);

  /** check if pixel is not highest within box
   *
   * Details:
   *
   * @return 0 if pixel is heighest, 1 otherwise
   * @param pixel const iterator to the pixel to be checked
   * @param linIdx the linearized index of the pixel
   * @param box the shape of the box to be checked (cols x rows)
   * @param stat the statistics calculator used to determine mean and stdv of
   *             pixels in box
   */
  int isNotHighest(HistogramFloatBase::storage_t::const_iterator pixel,
                   const index_t linIdx, shape_t box, stat_t &stat);

  /** retrieve the constant wavelength
   *
   * @param id unused
   */
  double lambdaFromConstant(const CASSEvent::id_t& /*id*/) {return _wavelength;}

  /** retrieve the wavelength from the processor
   *
   * @param id the id of the event to get the wavelength from
   */
  double lambdaFromProcessor(const CASSEvent::id_t& id);

  /** retrieve the constant detector distance
   *
   * @param id unused
   */
  double distanceFromConstant(const CASSEvent::id_t& /*id*/) {return _detdist;}

  /** retrieve the detector distance from the processor
   *
   * @param id the id of the event to get the detector distance from
   */
  double distanceFromProcessor(const CASSEvent::id_t& id);

protected:
  /** the size of the box within which the peak should lie */
  shape_t _box;

  /** size of a image section */
  shape_t _section;

  /** size of the incomming image */
  shape_t _imageShape;

  /** pixel threshold to be exceeded */
  pixelval_t _threshold;

  /** minimum Signal to Noise Ratio thats is needed for a pixel to be an outlier */
  stat_t::value_type _minSnr;

  /** minimum ratio of nbr of points used for statistics to nbr of outliers */
  float _minRatio;

  /** the minimum nbr of pixels in the bragg peak*/
  stat_t::count_type _minNbrPixels;

  /** the list of offsets to next neighbours */
  neighbourList_t _neighbourOffsets;

  /** the conversion table from raw to lab */
  std::vector<double> _src2labradius;

  /** the wavelength in case its fixed */
  double _wavelength;

  /** pp containing wavelength in case its not fixed */
  shared_pointer _wavelengthPP;

  /** function that gets the wavelength */
  std::tr1::function<double(const CASSEvent::id_t&)> _getLambda;

  /** the detector distance in case its fixed */
  double _detdist;

  /** pp containing detector distance in case its not fixed */
  shared_pointer _detdistPP;

  /** function that gets the detectordistance */
  std::tr1::function<double(const CASSEvent::id_t&)> _getDistance;

};


}//end namespace cass

#endif
