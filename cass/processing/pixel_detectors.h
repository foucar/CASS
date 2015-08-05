// Copyright (C) 2011 Lutz Foucar

/** @file pixel_detectors.h contains processor dealing with more advanced
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
#include "pixeldetector.hpp"
#include "advanced_pixeldetector.h"


namespace cass
{
// forward declaration
class Histogram0DFloat;
class Histogram1DFloat;
class Histogram2DFloat;



/** Pixeldetector image.
 *
 * @PPList "105": display the image from a pixeldetector defined
 *
 * processor will get the frame of the requested pixeldetector.
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{Detector}\n
 *           Name of the Detector that one is interested in. Default "blubb"
 *           See cass::pixeldetector::DetectorHelper for infos how to set up that
 *           detector.
 *
 * @author Lutz Foucar
 */
class pp105 : public Processor
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
 * @PPList "107": display the correction maps
 *
 * Will display the maps that are used the processing units to process the
 * frame and detect the pixels of interest.
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{Detector}\n
 *           Name of the Detector that one is interested in. Default "blubb"
 *           See cass::pixeldetector::DetectorHelper for infos how to set up that
 *           detector.
 * @cassttng Processor/\%name\%/{MapType}\n
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
class pp107 : public Processor
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
  pixeldetector::Detector::frame_t *_map;

  /** the lock for locking the map */
  QReadWriteLock *_mapLock;
};




/** Pixeldetector raw image.
 *
 * @PPList "109": extract raw pixel detector image
 *
 * processor will get the untreated frame directly from the cassevent
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{CASSID}\n
 *           The CASS ID of the detector device that one wants to extract. One
 *           can set up which CASS ID the detector has in the converter part.
 * @cassttng Processor/\%name\%/{nRows}\n
 *           The number of rows of the device (only needed when value cannot be
 *           determined from the name of the Processor)
 * @cassttng Processor/\%name\%/{nCols}\n
 *           The number of columns of the device (only needed when value cannot
 *           be determined from the name of the Processor)
 *
 * @author Lutz Foucar
 */
class pp109 : public Processor
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
 * @PPList "144": display the coalesced pixels identified from a pixel detector
 *
 * This processor will fill a 2D histogram with the coalesced hits on a
 * pixeldetector. See cass::pixeldetector::AdvancedDetector for the options
 * availalbe to coalesce the detected hits on the pixeldetectors.
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
 *           properties of the 2d histogram
 * @cassttng Processor/\%name\%/{Detector}\n
 *           Name of the Detector that one is interested in. Default "blubb"
 *           See cass::pixeldetector::DetectorHelper for infos how to set up that
 *           detector.
 * @cassttng Processor/\%name\%/{SpectralLowerLimit|SpectralUpperLimit}\n
 *           The range of the z value of the pixel. Only when the pixel is in
 *           this range the pixel will be drawn. Default is 0.0|0.0
 * @cassttng Processor/\%name\%/{SplitLevelUpperLimit|SplitLevelLowerLimit}\n
 *           The range of the Splitlevel of the photon hit. Splitlevel tells
 *           how many pixels contributed to the photonhit. Both limits are
 *           exclusive. IE: to only see a splitlevel of 1 (single pixel
 *           contributed to the photon hit) lower limit needs to be 0 and
 *           upper limit needs to be 2. Default is 0|2
 * @cassttng Processor/\%name\%/{PixelvalueAsWeight}\n
 *           When filling the 2D histogram one can select whether for each hit
 *           the z value should be added on the coordinate or a constant. If
 *           true the z value will be used. If false the coordinate will be
 *           increased by 1. Default is true.
 * @cassttng Processor/\%name\%/{BaseValue}\n
 *           Value of the pixels that are not set. Default is 0
 *
 * @author Lutz Foucar
 */
class pp144 : public Processor
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

  std::tr1::function<pixeldetector::Detector::frame_t::value_type(const pixeldetector::Hit&)> _getZ;

  /** value of the image where no pixel is set */
  float _baseValue;
};




/** Number of coalesced pixels (hits) in a pixeldetector
 *
 * @PPList "145": shows the number of coalesced pixels
 *
 * This processor retrieve how many coalesced photonhits have been
 * detected in a ccd frame.
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{Detector}\n
 *           Name of the Detector that one is interested in. Default "blubb"
 *           See cass::pixeldetector::DetectorHelper for infos how to set up that
 *           detector.
 *
 * @author Lutz Foucar
 */
class pp145 : public Processor
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
 * @PPList "146": histogram of the split level of coalescing
 *
 * This processor creates a 1d histogram displaying what the split level
 * of the photonhit was (how many pixels contributed to the photonhit). See
 * cass::pixeldetector::AdvancedDetector for the options available to coalesce
 * the detected hits on the pixeldetectors.
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{Detector}\n
 *           Name of the Detector that one is interested in. Default "blubb"
 *           See cass::pixeldetector::DetectorHelper for infos how to set up that
 *           detector.
 * @cassttng Processor/\%name\%/{XNbrBins|XLow|Xup}\n
 *           properties of the 1D histogram:
 *
 * @author Lutz Foucar
 */
class pp146 : public Processor
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
 * @PPList "148": display an image generated from the identified pixels
 *
 * This processor will fill a 2D histogram with the detected pixels in a
 * pixeldetector. See cass::pixeldetector::AdvancedDetector for the options
 * available identify pixels of interest.
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
 *           properties of the 2d histogram
 * @cassttng Processor/\%name\%/{Detector}\n
 *           Name of the Detector that one is interested in. Default "blubb"
 *           See cass::pixeldetector::DetectorHelper for infos how to set up that
 *           detector.
 * @cassttng Processor/\%name\%/{SpectralLowerLimit|SpectralUpperLimit}\n
 *           The range of the z value of the pixel. Only when the pixel is in
 *           this range the pixel will be drawn. Default is 0.0|0.0
 * @cassttng Processor/\%name\%/{PixelvalueAsWeight}\n
 *           When filling the 2D histogram one can select whether for each hit
 *           the z value should be added on the coordinate or a constant. If
 *           true the z value will be used. If false the coordinate will be
 *           increased by 1. Default is true.
 * @cassttng Processor/\%name\%/{BaseValue}\n
 *           Value of the pixels that are not set. Default is 0
 *
 * @author Lutz Foucar
 */
class pp148 : public Processor
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
  std::tr1::function<pixeldetector::Detector::frame_t::value_type(const pixeldetector::Pixel&)> _getZ;

  /** value of the image where no pixel is set */
  float _baseValue;
};




/** Number of detected pixels of an pixeldetector
 *
 * @PPList "149": the number of detected pixels in a pixeldetector
 *
 * This processor retrieve how many pixels have been detected in a
 * pixeldetector frame. See cass::pixeldetector::AdvancedDetector for available
 * options on how to detect pixels of interest.
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{Detector}\n
 *           Name of the Detector that one is interested in. Default "blubb"
 *           See cass::pixeldetector::DetectorHelperHelper for infos how to set up that
 *           detector.
 *
 * @author Lutz Foucar
 */
class pp149 : public Processor
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
 * @PPList "241" correct the distorted offset in a pnCCD image
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
 * This processor will therefore first determine at the edge of the detector
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
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{HistName} \n
 *           Name of Processor containing the distorted pnCCD image. Default
 *           is "".
 * @cassttng Processor/\%name\%/{ThresholdQuadrantA} \n
 *           Threshold for the rows of quadrant A. If the averaged value is
 *           below this value the whole row of this quadrant is changed. Default
 *           is 0.
 * @cassttng Processor/\%name\%/{ThresholdQuadrantB} \n
 *           Threshold for the rows of quadrant B. If the averaged value is
 *           below this value the whole row of this quadrant is changed. Default
 *           is 0.
 * @cassttng Processor/\%name\%/{ThresholdQuadrantC} \n
 *           Threshold for the rows of quadrant A. If the averaged value is
 *           below this value the whole row of this quadrant is changed. Default
 *           is 0.
 * @cassttng Processor/\%name\%/{ThresholdQuadrantD} \n
 *           Threshold for the rows of quadrant D. If the averaged value is
 *           below this value the whole row of this quadrant is changed. Default
 *           is 0.
 * @cassttng Processor/\%name\%/{WeightAdjecentRow} \n
 *           How much will the row that is adjecent to the current row be weightd
 *           when calculating the average offset at the edge of the detector.
 *           Default is 0.75.
 * @cassttng Processor/\%name\%/{WeightSecondNextRow} \n
 *           How much will the row that is the next over to the current row be
 *           weightd when calculating the average offset at the edge of the
 *           detector. Default is 0.5.
 * @cassttng Processor/\%name\%/{MinimumRow} \n
 *           Define from which row the correction should be applied. Default is 0.
 * @cassttng Processor/\%name\%/{MaximumRow} \n
 *           Define to which row the correction should be applied. Default is 1024.
 *
 * @author Lutz Foucar
 */
class pp241 : public Processor
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
 * @PPList "242" same as pp105 but sets the masked pixels to a user defined value
 *
 * Copys the frame data and then sets all maked  pixels in the 2d histogram to a
 * predefined value
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{Detector}\n
 *           Name of the Detector who's values should be masked
 * @cassttng Processor/\%name\%/{Value}\n
 *           Value that should be assigned to pixels that should be masked
 *           Default is 0.
 *
 * @author Lutz Foucar
 */
class pp242 : public Processor
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
  pixeldetector::Detector::frame_t *_mask;

  /** the lock for locking the map */
  QReadWriteLock *_maskLock;

  /** the value that the masked things should take */
  float _value;
};









/** apply mask to an image, set the masked pixel to a certain value
 *
 * @PPList "243" generate a copy of the incomming image and set masked pixels to
 *               a user defined value
 *
 * Copys the frame data and then sets all pixels that are 0 in the mask to a
 * predefined value
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{HistName}\n
 *           Name of the Histogram that should be masked
 * @cassttng Processor/\%name\%/{MaskName}\n
 *           The name of the Mask.
 * @cassttng Processor/\%name\%/{Value}\n
 *           Value that should be assigned to pixels that should be masked
 *           Default is 0.
 *
 * @author Lutz Foucar
 */
class pp243 : public Processor
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






/** generate a histogram for each pixel of the input 2d image
 *
 * @PPList "244" generate a histogram for each pixel of the input 2d image
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{ImageName}\n
 *           Name of the Image whos pixels should be histogrammed
 * @cassttng Processor/\%name\%/{XNbrBins|XLow|XUp|XTitle}\n
 *           properties of the 1D histogram generated for each Pixel.
 * @cassttng Processor/\%name\%/{Weight}\n
 *           The value that will be added to the histogram. Default is 1
 * @cassttng Processor/\%name\%/{MaskVal}\n
 *           The value that the masked pixels have in the image. Masked pixels
 *           will not be added to the histogram. Default is 0
 * @cassttng Processor/\%name\%/{IsPnCCD}\n
 *           Combine the columns of the pnCCD to lower the memory used.
 *           Default is false. In case of pnCCD the channels of the quadrants
 *           will be displayed in rows 0 to 2047 and the rows, which indicate the
 *           cte will be dispyed in rows 2048 to 2559
 *
 * @author Lutz Foucar
 */
class pp244 : public Processor
{
public:
  /** constructor */
  pp244(const name_t &name);

  /** copy pixels from CASS event to histogram storage */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** set the histogram size */
  virtual void loadSettings(size_t);

protected:
  /** pp containing image that will be masked*/
  shared_pointer _image;

  /** the weight to fill the histogram with */
  float _weight;

  /** the value of masked pixels */
  float _maskval;

  /** flag to tell whether its an pnCCD */
  bool _isPnCCD;
};






}//end namespace cass
#endif
