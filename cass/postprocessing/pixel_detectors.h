// Copyright (C) 2011 Lutz Foucar

/** @file pixel_detectors.h contains postprocessor dealing with more advanced
 *                          pixel detectors.
 *
 * @author Lutz Foucar
 */

#ifndef _PIXEL_DETECTORS_H_
#define _PIXEL_DETECTORS_H_

#include <tr1/functional>

#include "processor.h"
#include "cass_event.h"
#include "pixel_detector_helper.h"
#include "cass_pixeldetector.h"
#include "pixeldetector.hpp"


namespace cass
{
// forward declaration
class Histogram0DFloat;
class Histogram1DFloat;
class Histogram2DFloat;



/** Pixeldetector image.
 *
 * Postprocessor will get the frame of the requested pixeldetector.
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{Detector}\n
 *           Name of the Detector that one is interested in. Default "blubb"
 *           See cass::pixeldetector::DetectorHelper for infos how to set up that
 *           detector.
 *
 * @author Lutz Foucar
 */
class pp105 : public PostProcessor
{
public:
  /** constructor */
  pp105(const name_t &name);

  /** copy image from CASS event to histogram storage
   *
   * @throws invalid_argument if user provided size is incorrect
   */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** load the settings for this pp */
  virtual void loadSettings(size_t);

protected:
  /** detector to work on */
  pixeldetector::DetectorHelper::instancesmap_t::key_type _detector;
};







/** display the maps
 *
 * Will display the maps that are used the processing units to process the
 * frame and detect the pixels of interest.
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{Detector}\n
 *           Name of the Detector that one is interested in. Default "blubb"
 *           See cass::pixeldetector::DetectorHelper for infos how to set up that
 *           detector.
 * @cassttng PostProcessor/\%name\%/{MapType}\n
 *           the type of the map that should be displayed. See
 *           cass::pixeldetector::CommonData for details. Default is "offset".
 *           Possible values are:
 *           - "offset": The offset map
 *           - "noise": The noise map
 *           - "gain_cte": The map containing cte and gain values
 *           - "correction": The map containing the correction values used for
 *                           processing.
 *
 * @author Lutz Foucar
 */
class pp107 : public PostProcessor
{
public:
  /** constructor */
  pp107(const name_t &name);

  /** copy pixels from Map to histogram storage
   *
   * @throws invalid argument when the size of the maps have changed
   */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** set the histogram size */
  virtual void loadSettings(size_t);

protected:
  /** detector to work on */
  pixeldetector::DetectorHelper::instancesmap_t::key_type _detector;

  /** pointer to the map */
  pixeldetector::frame_t *_map;

  /** the lock for locking the map */
  QReadWriteLock *_mapLock;
};




/** Pixeldetector raw image.
 *
 * @PPList "109": extract raw pixel detector image
 *
 * Postprocessor will get the untreated frame directly from the cassevent
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{Detector}\n
 *           The CASS ID of the detector device that one wants to extract. One
 *           can set up which CASS ID the detector has in the converter part.
 * @cassttng PostProcessor/\%name\%/{nRows}\n
 *           The number of rows of the device (only needed when value cannot be
 *           determined from the name of the PostProcessor)
 * @cassttng PostProcessor/\%name\%/{nCols}\n
 *           The number of columns of the device (only needed when value cannot
 *           be determined from the name of the PostProcessor)
 *
 * @author Lutz Foucar
 */
class pp109 : public PostProcessor
{
public:
  /** constructor */
  pp109(const name_t &name);

  /** copy image from CASS event to histogram storage
   *
   * @throws invalid_argument if user provided size is incorrect
   */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** load the settings for this pp */
  virtual void loadSettings(size_t);

protected:
  /** detector to work on */
   pixeldetector::Device::detectors_t::key_type _detector;
};







/** coalesced pixels (hits) on a pixeldetector.
 *
 * This postprocessor will fill a 2D histogram with the coalesced hits on a
 * pixeldetector. See cass::pixeldetector::AdvancedDetector for the options
 * availalbe to coalesce the detected hits on the pixeldetectors.
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
 *           properties of the 2d histogram
 * @cassttng PostProcessor/\%name\%/{Detector}\n
 *           Name of the Detector that one is interested in. Default "blubb"
 *           See cass::pixeldetector::DetectorHelper for infos how to set up that
 *           detector.
 * @cassttng PostProcessor/\%name\%/{SpectralLowerLimit|SpectralUpperLimit}\n
 *           The range of the z value of the pixel. Only when the pixel is in
 *           this range the pixel will be drawn. Default is 0.0|0.0
 * @cassttng PostProcessor/\%name\%/{SplitLevelUpperLimit|SplitLevelLowerLimit}\n
 *           The range of the Splitlevel of the photon hit. Splitlevel tells
 *           how many pixels contributed to the photonhit. Both limits are
 *           exclusive. IE: to only see a splitlevel of 1 (single pixel
 *           contributed to the photon hit) lower limit needs to be 0 and
 *           upper limit needs to be 2. Default is 0|2
 * @cassttng PostProcessor/\%name\%/{PixelvalueAsWeight}\n
 *           When filling the 2D histogram one can select whether for each hit
 *           the z value should be added on the coordinate or a constant. If
 *           true the z value will be used. If false the coordinate will be
 *           increased by 1. Default is true.
 *
 * @author Lutz Foucar
 */
class pp144 : public PostProcessor
{
public:
  /** constructor */
  pp144(const name_t &name);

  /** copy pixels from CASS event to histogram storage */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** set the histogram size */
  virtual void loadSettings(size_t);

protected:
  /** detector to work on */
  pixeldetector::DetectorHelper::instancesmap_t::key_type _detector;

  /** gate on the z */
  std::pair<float,float> _range;

  /** gate on split level */
  std::pair<size_t, size_t> _splitLevelRange;

  std::tr1::function<pixeldetector::frame_t::value_type(const pixeldetector::Hit&)> _getZ;
};




/** Number of coalesced pixels (hits) in a pixeldetector
 *
 * This postprocessor retrieve how many coalesced photonhits have been
 * detected in a ccd frame.
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{Detector}\n
 *           Name of the Detector that one is interested in. Default "blubb"
 *           See cass::pixeldetector::DetectorHelper for infos how to set up that
 *           detector.
 *
 * @author Lutz Foucar
 */
class pp145 : public PostProcessor
{
public:
  /** constructor */
  pp145(const name_t &name);

  /** copy pixels from CASS event to histogram storage */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** set the histogram size */
  virtual void loadSettings(size_t);

protected:
  /** detector to work on */
  pixeldetector::DetectorHelper::instancesmap_t::key_type _detector;
};




/** split level of the colesced pixels (hits) of CCD's.
 *
 * This postprocessor creates a 1d histogram displaying what the split level
 * of the photonhit was (how many pixels contributed to the photonhit). See
 * cass::pixeldetector::AdvancedDetector for the options available to coalesce
 * the detected hits on the pixeldetectors.
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{Detector}\n
 *           Name of the Detector that one is interested in. Default "blubb"
 *           See cass::pixeldetector::DetectorHelper for infos how to set up that
 *           detector.
 * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|Xup}\n
 *           properties of the 1D histogram:
 *
 * @author Lutz Foucar
 */
class pp146 : public PostProcessor
{
public:
  /** constructor */
  pp146(const name_t &name);

  /** copy pixels from CASS event to histogram storage */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** set the histogram size */
  virtual void loadSettings(size_t);

protected:
  /** detector to work on */
  pixeldetector::DetectorHelper::instancesmap_t::key_type _detector;
};








/** image of detected pixels in a pixeldetector.
 *
 * This postprocessor will fill a 2D histogram with the detected pixels in a
 * pixeldetector. See cass::pixeldetector::AdvancedDetector for the options
 * available identify pixels of interest.
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
 *           properties of the 2d histogram
 * @cassttng PostProcessor/\%name\%/{Detector}\n
 *           Name of the Detector that one is interested in. Default "blubb"
 *           See cass::pixeldetector::DetectorHelper for infos how to set up that
 *           detector.
 * @cassttng PostProcessor/\%name\%/{SpectralLowerLimit|SpectralUpperLimit}\n
 *           The range of the z value of the pixel. Only when the pixel is in
 *           this range the pixel will be drawn. Default is 0.0|0.0
 * @cassttng PostProcessor/\%name\%/{PixelvalueAsWeight}\n
 *           When filling the 2D histogram one can select whether for each hit
 *           the z value should be added on the coordinate or a constant. If
 *           true the z value will be used. If false the coordinate will be
 *           increased by 1. Default is true.
 *
 * @author Lutz Foucar
 */
class pp148 : public PostProcessor
{
public:
  /** constructor */
  pp148(const name_t &name);

  /** copy pixels from CASS event to histogram storage */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** set the histogram size */
  virtual void loadSettings(size_t);

protected:
  /** detector to work on */
  pixeldetector::DetectorHelper::instancesmap_t::key_type _detector;

  /** gate on the z */
  std::pair<float,float> _range;

  /** function to retrieve the z value of the pixels */
  std::tr1::function<pixeldetector::frame_t::value_type(const pixeldetector::Pixel&)> _getZ;
};




/** Number of detected pixels of an pixeldetector
 *
 * This postprocessor retrieve how many pixels have been detected in a
 * pixeldetector frame. See cass::pixeldetector::AdvancedDetector for available
 * options on how to detect pixels of interest.
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{Detector}\n
 *           Name of the Detector that one is interested in. Default "blubb"
 *           See cass::pixeldetector::DetectorHelperHelper for infos how to set up that
 *           detector.
 *
 * @author Lutz Foucar
 */
class pp149 : public PostProcessor
{
public:
  /** constructor */
  pp149(const name_t &name);

  /** copy pixels from CASS event to histogram storage */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** set the histogram size */
  virtual void loadSettings(size_t);

protected:
  /** detector to work on */
  pixeldetector::DetectorHelper::instancesmap_t::key_type _detector;
};







/** decreased offset correction
 *
 * modifies the incomming 2d histogram such that a wrong offset du to much charge
 * on the pnCCD detector will be corrected.
 *
 * The correction is different for each quadrant. Basically one corrects a line
 * with a linear slope. Therefore one determines how much the nominal offset at
 * the edge of the detector has been disturbed. It should not be disturbed at
 * all since these parts are shielded from the light. A distortion will therefore
 * indicate how much these lines have been distorted.
 *
 * This postprocessor will therefore first determine at the edge of the detector
 * how much the offset has been distorted. Once this is know one can use a
 * correction factor and substract this from the measured value. A pixels
 * correction is therefore done like
 *
 * determine the averageOffsetAtEdge value and from that determine the slope for
 * the quadrant. Then pixel values are calculated like this:
 *
 * cor_function_value = slope * column_number - averageOffsetAtEdge;
 * final_corection = pix_raw_val - offset_from_darkcal - cor_function_value;
 *
 * Because the average offset at the edge of the detector might contain statisics
 * outliers. One can includes the line above and below with less (user selectable)
 * weight. If one chooses 0 weight that line will not be included in the
 * calculation of the determination of the average offset value at the edge.
 *
 * The average offset at the edge of the first two and the last two rows will
 * only be determined by the current row.
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{HistName} \n
 *           Name of PostProcessor containing the distorted pnCCD image. Default
 *           is "".
 * @cassttng PostProcessor/\%name\%/{ThresholdQuadrantA} \n
 *           Threshold for the rows of quadrant A. If the averaged value is
 *           below this value the whole row of this quadrant is changed. Default
 *           is 0.
 * @cassttng PostProcessor/\%name\%/{ThresholdQuadrantB} \n
 *           Threshold for the rows of quadrant B. If the averaged value is
 *           below this value the whole row of this quadrant is changed. Default
 *           is 0.
 * @cassttng PostProcessor/\%name\%/{ThresholdQuadrantC} \n
 *           Threshold for the rows of quadrant A. If the averaged value is
 *           below this value the whole row of this quadrant is changed. Default
 *           is 0.
 * @cassttng PostProcessor/\%name\%/{ThresholdQuadrantD} \n
 *           Threshold for the rows of quadrant D. If the averaged value is
 *           below this value the whole row of this quadrant is changed. Default
 *           is 0.
 * @cassttng PostProcessor/\%name\%/{WeightAdjecentRow} \n
 *           How much will the row that is adjecent to the current row be weightd
 *           when calculating the average offset at the edge of the detector.
 *           Default is 0.75.
 * @cassttng PostProcessor/\%name\%/{WeightSecondNextRow} \n
 *           How much will the row that is the next over to the current row be
 *           weightd when calculating the average offset at the edge of the
 *           detector. Default is 0.5.
 * @cassttng PostProcessor/\%name\%/{MinimumRow} \n
 *           Define from which row the correction should be applied. Default is 0.
 * @cassttng PostProcessor/\%name\%/{MaximumRow} \n
 *           Define to which row the correction should be applied. Default is 1024.
 *
 * @author Lutz Foucar
 */
class pp241 : public PostProcessor
{
public:
  /** constructor */
  pp241(const name_t &);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** load the settings */
  virtual void loadSettings(size_t);

protected:
  /** pp containing 2d histogram to work on */
  shared_pointer _hist;

  /** threshold for quadrant A */
  float _thresholdA;

  /** threshold for quadrant A */
  float _thresholdB;

  /** threshold for quadrant A */
  float _thresholdC;

  /** threshold for quadrant A */
  float _thresholdD;

  /** the weight of the row next to the current one */
  float _weightAdjectentRow;

  /** the weight of the row next over to the current one */
  float _weightSecondRow;

  /** the value by which one has to divide to get the right average value */
  float _weightSum;

  /** the minimum row */
  size_t _minRow;

  /** the maximum row */
  size_t _maxRow;
};








/** process untreated frame with mask
 *
 * Copys the frame data and then sets all maked  pixels in the 2d histogram to a
 * predefined value
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{Detector}\n
 *           Name of the Detector who's values should be masked
 * @cassttng PostProcessor/\%name\%/{Value}\n
 *           Value that should be assigned to pixels that should be masked
 *           Default is 0.
 *
 * @author Lutz Foucar
 */
class pp242 : public PostProcessor
{
public:
  /** constructor */
  pp242(const name_t &name);

  /** copy pixels from CASS event to histogram storage */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** set the histogram size */
  virtual void loadSettings(size_t);

protected:
  /** detector to work on */
  pixeldetector::DetectorHelper::instancesmap_t::key_type _detector;

  /** pointer to the mask */
  pixeldetector::frame_t *_mask;

  /** the lock for locking the map */
  QReadWriteLock *_maskLock;

  /** the value that the masked things should take */
  float _value;
};









/** apply mask to an image, set the masked pixel to a certain value
 *
 * Copys the frame data and then sets all pixels that are 0 in the mask to a
 * predefined value
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{HistName}\n
 *           Name of the Histogram that should be masked
 * @cassttng PostProcessor/\%name\%/{MaskName}\n
 *           The name of the Mask.
 * @cassttng PostProcessor/\%name\%/{Value}\n
 *           Value that should be assigned to pixels that should be masked
 *           Default is 0.
 *
 * @author Lutz Foucar
 */
class pp243 : public PostProcessor
{
public:
  /** constructor */
  pp243(const name_t &name);

  /** copy pixels from CASS event to histogram storage */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** set the histogram size */
  virtual void loadSettings(size_t);

protected:
  /** pp containing image that will be masked*/
  shared_pointer _image;

  /** pp containing the mask to apply */
  shared_pointer _mask;

  /** the value that the masked things should take */
  float _value;
};






}//end namespace cass
#endif
