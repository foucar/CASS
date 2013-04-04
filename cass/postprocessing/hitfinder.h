// Copyright (C) 2013 Lutz Foucar

/** @file hitfinder.h contains postprocessors that will extract pixels of
 *                    interrest from 2d histograms.
 * @author Lutz Foucar
 */

#ifndef _HITFINDER_H_
#define _HITFINDER_H_

#include <tr1/functional>

#include "backend.h"
#include "cass_event.h"
#include "histogram.h"



namespace cass
{


/** get the local background from image.
 *
 * splits up the image into sections of user choosable size. In each of these
 * sections the local background is determined by taking a box of a user
 * choosable size and determining the median of the pixel values inside this
 * box.
 *
 * @see PostprocessorBackend for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{HistName} \n
 *           the postprocessor name that contain the 2d histogram. Default
 *           is "blubb".
 * @cassttng PostProcessor/\%name\%/{SectionSizeX|SectionSizeY} \n
 *           Size of the subsection of the image. Default is 1024|512.
 * @cassttng PostProcessor/\%name\%/{BoxSizeX|BoxSizeY} \n
 *           size in x and y of the box that is used for determining the median
 *           background. Default is 10|10.
 *
 * @author Lutz Foucar
 * @author Wolfgang Kabsch
 */
class pp203 : public PostprocessorBackend
{
public:
  /** constructor */
  pp203(PostProcessors& hist, const PostProcessors::key_t&);

  /** process event */
  virtual void process(const CASSEvent&);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

protected:
  /** change own histograms when one of the ones we depend on has changed histograms */
  virtual void histogramsChanged(const HistogramBackend*);

  /** pp containing 2d histogram */
  PostprocessorBackend *_hist;

  /** the size of the box used for the median filter */
  std::pair<size_t,size_t> _boxSize;

  /** size of a image section */
  std::pair<size_t,size_t> _sectionSize;

private:
  /** set up the histogram */
  void setup(const HistogramBackend &hist);
};


/** find bragg peaks and store them in a list
 *
 *
 * @see PostprocessorBackend for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{HistName} \n
 *           the postprocessor name that contain the 2d histogram. Default
 *           is "blubb".
 * @cassttng PostProcessor/\%name\%/{SectionSizeX|SectionSizeY} \n
 *           Size of the subsection of the image. Default is 1024|512.
 * @cassttng PostProcessor/\%name\%/{BoxSizeX|BoxSizeY} \n
 *           size in x and y of the box that is used for determining the median
 *           background. Default is 10|10.
 * @cassttng PostProcessor/\%name\%/{Threshold} \n
 * @cassttng PostProcessor/\%name\%/{FunctionType} \n
 * @cassttng PostProcessor/\%name\%/{MinSignalToNoiseRatio} \n
 * @cassttng PostProcessor/\%name\%/{MinNbrBackgrndPixels} \n
 * @cassttng PostProcessor/\%name\%/{BraggPeakRadius} \n
 * @cassttng PostProcessor/\%name\%/Input/{size} \n
 * @cassttng PostProcessor/\%name\%/Input/\%id\%/{Name} \n
 *
 * @author Lutz Foucar
 */
class pp204 : public PostprocessorBackend
{
public:
  /** constructor */
  pp204(PostProcessors& hist, const PostProcessors::key_t&);

  /** process event */
  virtual void process(const CASSEvent&);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

protected:
  /** typedefinintio for container for all pp  */
  typedef std::vector<std::pair<PostprocessorBackend*, const HistogramFloatBase*> > resources_t;

  /** container for all pp that are needed for processing */
  resources_t _resources;

  /** definition of the table */
  typedef HistogramFloatBase::storage_t table_t;

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

  /** the function object that will be called to process the event */
  std::tr1::function<void(void)> _process;

protected:
  /** find bragg peaks in image using signal to noise ratio */
  void SNR();

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
                       const int nColumns,
                       float &mean, float &stdv, int &count);

  /** the size of the box within which the peak should lie */
  std::pair<int,int> _box;

  /** size of a image section */
  std::pair<int,int> _section;

  /** pixel threshold to be exceeded */
  float _threshold;

  /** the square size of bragg peak radius */
  int _peakRadiusSq;

  /** the min signal to noise ratio that needs to be exceeded */
  float _minSnr;

  /** min amount of pixels for the background calc */
  int _minBckgndPixels;

  float snr_mean, snrall_mean,snr_stdv,snrall_stdv,radius_mean,radius_stdv;
  int counter,counterall,counter_rad;

protected:
  /** find pixels that are above a threshold
   *
   * @author Wolfgang Kabsch
   * @author Lutz Foucar
   */
  void Threshold();

};





/** visualize the peaks that were found in the image itself
 *
 * @see PostprocessorBackend for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{HistName} \n
 *           the postprocessor name that contain the 2d histogram. Default
 *           is "blubb".
 * @cassttng PostProcessor/\%name\%/{TableName} \n
 *           name of postprocessor that contains the table like histogram that
 *           the pixels of interest are taken from
 * @cassttng PostProcessor/\%name\%/{BoxSizeX|BoxSizeY} \n
 *           size in x and y of the box that should be drawn around the found
 *           peak
 * @cassttng PostProcessor/\%name\%/{DrawPixelValue} \n
 * @cassttng PostProcessor/\%name\%/{Radius} \n
 * @cassttng PostProcessor/\%name\%/{IndexColumn} \n
 * @cassttng PostProcessor/\%name\%/{DrawCircle} \n
 * @cassttng PostProcessor/\%name\%/{DrawBox} \n
 *
 *
 * @author Lutz Foucar
 * @author Wolfgang Kabsch
 */
class pp205 : public PostprocessorBackend
{
public:
  /** constructor */
  pp205(PostProcessors& hist, const PostProcessors::key_t&);

  /** process event */
  virtual void process(const CASSEvent&);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

protected:
  /** change own histograms when one of the ones we depend on has changed histograms */
  virtual void histogramsChanged(const HistogramBackend*);

  /** pp containing 2d histogram */
  PostprocessorBackend *_hist;

  /** pp containing the results */
  PostprocessorBackend *_table;

  /** draw flags as bitmask */
  HistogramFloatBase::storage_t::value_type _drawVal;
  float _radius;
  std::pair<int,int> _boxsize;
  bool _drawCircle,_drawBox;

  /** the number of the column where the global index of the pixel is */
  size_t _idxCol;
};

}//end namespace cass

#endif
