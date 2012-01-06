// Copyright (C) 2011 Lutz Foucar

/** @file pixel_detectors.h contains postprocessor dealing with more advanced
 *                          pixel detectors.
 *
 * @author Lutz Foucar
 */

#ifndef _PIXEL_DETECTORS_H_
#define _PIXEL_DETECTORS_H_

#include <tr1/functional>

#include "backend.h"
#include "cass_event.h"
#include "postprocessor.h"
#include "pixel_detector_helper.h"
#include "cass_pixeldetector.h"


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
 * @see PostprocessorBackend for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{Detector}\n
 *           Name of the Detector that one is interested in. Default "blubb"
 *           See cass::pixeldetector::DetectorHelper for infos how to set up that
 *           detector.
 *
 * @author Lutz Foucar
 */
class pp105 : public PostprocessorBackend
{
public:
  /** constructor */
  pp105(PostProcessors&, const PostProcessors::key_t&);

  /** copy image from CASS event to histogram storage
   *
   * if the size has changed resize the histograms and notify all dependands of
   * this.
   */
  virtual void process(const CASSEvent&);

  /** load the settings for this pp */
  virtual void loadSettings(size_t);

protected:
  /** detector to work on */
  pixeldetector::DetectorHelper::instancesmap_t::key_type _detector;
};






/** Pixeldetector frame's histogram
 *
 * Fill all the pixels z values into a 1D histogram. The pixels that are masked
 * will be disregarded and not be filled into the 1D histogram
 *
 * @see PostprocessorBackend for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|Xup}\n
 *           properties of the 1D histogram:
 * @cassttng PostProcessor/\%name\%/{Detector}\n
 *           Name of the Detector that one is interested in. Default "blubb"
 *           See cass::pixeldetector::DetectorHelper for infos how to set up that
 *           detector.
 *
 * @author Lutz Foucar
 */
class pp106 : public PostprocessorBackend
{
public:
  /** constructor */
  pp106(PostProcessors&, const PostProcessors::key_t&);

  /** copy image from CASS event to histogram storage */
  virtual void process(const CASSEvent&);

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
 * @see PostprocessorBackend for a list of all commonly available cass.ini
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
class pp107 : public PostprocessorBackend
{
public:
  /** constructor */
  pp107(PostProcessors&, const PostProcessors::key_t&);

  /** copy pixels from CASS event to histogram storage */
  virtual void process(const CASSEvent&);

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






/** Spectrum of coalesced pixels (hits) in a pixeldetector.
 *
 * This postprocessor will fill a 1D histogram with the z values of the coalesced
 * hits. See cass::pixeldetector::AdvancedDetector for the options available
 * to coalesce the detected hits on the pixeldetectors. If one wants to fill
 * only pixels that are within a certain area of the pixeldetector, one can
 * use the mask to mask out uninteresting areas. See
 * cass::pixeldetector::CommonData for details how to set the mask.
 *
 * @see PostprocessorBackend for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|Xup}\n
 *           properties of the 1D histogram:
 * @cassttng PostProcessor/\%name\%/{Detector}\n
 *           Name of the Detector that one is interested in. Default "blubb"
 *           See cass::pixeldetector::DetectorHelper for infos how to set up that
 *           detector.
 * @cassttng PostProcessor/\%name\%/{SplitLevelUpperLimit|SplitLevelLowerLimit}\n
 *           The range of the Splitlevel of the photon hit. Splitlevel tells
 *           how many pixels contributed to the photonhit. Both limits are
 *           exclusive. IE: to only see a splitlevel of 1 (single pixel
 *           contributed to the photon hit) lower limit needs to be 0 and
 *           upper limit needs to be 2. Default is 0|2
 *
 * @author Lutz Foucar
 */
class pp143 : public PostprocessorBackend
{
public:
  /** constructor */
  pp143(PostProcessors&, const PostProcessors::key_t&);

  /** copy pixels from CASS event to histogram storage */
  virtual void process(const CASSEvent&);

  /** set the histogram size */
  virtual void loadSettings(size_t);

protected:
  /** detector to work on */
  pixeldetector::DetectorHelper::instancesmap_t::key_type _detector;

  /** gate on split level */
  std::pair<size_t, size_t> _splitLevelRange;
};




/** coalesced pixels (hits) on a pixeldetector.
 *
 * This postprocessor will fill a 2D histogram with the coalesced hits on a
 * pixeldetector. See cass::pixeldetector::AdvancedDetector for the options
 * availalbe to coalesce the detected hits on the pixeldetectors.
 *
 * @see PostprocessorBackend for a list of all commonly available cass.ini
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
class pp144 : public PostprocessorBackend
{
public:
  /** constructor */
  pp144(PostProcessors&, const PostProcessors::key_t&);

  /** copy pixels from CASS event to histogram storage */
  virtual void process(const CASSEvent&);

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
 * @see PostprocessorBackend for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{Detector}\n
 *           Name of the Detector that one is interested in. Default "blubb"
 *           See cass::pixeldetector::DetectorHelper for infos how to set up that
 *           detector.
 *
 * @author Lutz Foucar
 */
class pp145 : public PostprocessorBackend
{
public:
  /** constructor */
  pp145(PostProcessors&, const PostProcessors::key_t&);

  /** copy pixels from CASS event to histogram storage */
  virtual void process(const CASSEvent&);

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
 * @see PostprocessorBackend for a list of all commonly available cass.ini
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
class pp146 : public PostprocessorBackend
{
public:
  /** constructor */
  pp146(PostProcessors&, const PostProcessors::key_t&);

  /** copy pixels from CASS event to histogram storage */
  virtual void process(const CASSEvent&);

  /** set the histogram size */
  virtual void loadSettings(size_t);

protected:
  /** detector to work on */
  pixeldetector::DetectorHelper::instancesmap_t::key_type _detector;
};





/**  histogram of the z value of detected pixel
 *
 * This postprocessor will fill a 1D histogram with the z values of the detected
 * pixels. See See cass::pixeldetector::AdvancedDetector for the options
 * available detect pixels on the pixeldetectors. If one wants to fill
 * only pixels that are within a certain area of the pixeldetector, one can
 * use the mask to mask out uninteresting areas. See
 * cass::pixeldetector::CommonData for details how to set the mask.
 *
 * @see PostprocessorBackend for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|Xup}\n
 *           properties of the 1D histogram:
 * @cassttng PostProcessor/\%name\%/{Detector}\n
 *           Name of the Detector that one is interested in. Default "blubb"
 *           See cass::pixeldetector::DetectorHelper for infos how to set up that
 *           detector.
 *
 * @author Lutz Foucar
 */
class pp147 : public PostprocessorBackend
{
public:
  /** constructor */
  pp147(PostProcessors&, const PostProcessors::key_t&);

  /** copy pixels from CASS event to histogram storage */
  virtual void process(const CASSEvent&);

  /** set the histogram size */
  virtual void loadSettings(size_t);

protected:
  /** gate on the z */
  std::pair<float,float> _range;

  /** detector to work on */
  pixeldetector::DetectorHelper::instancesmap_t::key_type _detector;
};







/** image of detected pixels in a pixeldetector.
 *
 * This postprocessor will fill a 2D histogram with the detected pixels in a
 * pixeldetector. See cass::pixeldetector::AdvancedDetector for the options
 * available identify pixels of interest.
 *
 * @see PostprocessorBackend for a list of all commonly available cass.ini
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
class pp148 : public PostprocessorBackend
{
public:
  /** constructor */
  pp148(PostProcessors&, const PostProcessors::key_t&);

  /** copy pixels from CASS event to histogram storage */
  virtual void process(const CASSEvent&);

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
 * @see PostprocessorBackend for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{Detector}\n
 *           Name of the Detector that one is interested in. Default "blubb"
 *           See cass::pixeldetector::DetectorHelperHelper for infos how to set up that
 *           detector.
 *
 * @author Lutz Foucar
 */
class pp149 : public PostprocessorBackend
{
public:
  /** constructor */
  pp149(PostProcessors&, const PostProcessors::key_t&);

  /** copy pixels from CASS event to histogram storage */
  virtual void process(const CASSEvent&);

  /** set the histogram size */
  virtual void loadSettings(size_t);

protected:
  /** detector to work on */
  pixeldetector::DetectorHelper::instancesmap_t::key_type _detector;
};



}//end namespace cass
#endif
