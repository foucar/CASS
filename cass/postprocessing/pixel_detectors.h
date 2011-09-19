// Copyright (C) 2011 Lutz Foucar

/** @file pixel_detectors.h contains postprocessor dealing with more advanced
 *                          pixel detectors.
 *
 * @author Lutz Foucar
 */

#ifndef _PIXEL_DETECTORS_H_
#define _PIXEL_DETECTORS_H_

#include "backend.h"
#include "cass_event.h"
#include "postprocessor.h"
#include "pixel_detector_helper.h"


namespace cass
{
// forward declaration
class Histogram0DFloat;
class Histogram1DFloat;
class Histogram2DFloat;



/** Spectrum of coalesced PhotonHits of CCD's.
 *
 * This postprocessor will fill a 1D histogram with the z values in detected
 * Photonhits. Photonhits will be detected in the according pre analyzer.
 *
 * @see PostprocessorBackend for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|Xup}\n
 *           properties of the 1D histogram:
 * @cassttng PostProcessor/\%name\%/{Detector}\n
 *           Name of the Detector that one is interested in. Default "blubb"
 *           See cass::HelperPixelDetectors for infos how to set up that
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




/** coalesced PhotonHits of CCD's.
 *
 * This postprocessor will fill a 2D histogram with the detected Photonhits.
 * Photonhits will be detected in the pre analyzers. Set the Parameters
 * for detecting photonhits there. (PNCCD::Analyzer or CCD::Analyzer)
 * One can select pixel only to be shown when it has a certain adu.
 *
 * @see PostprocessorBackend for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
 *           properties of the 2d histogram
 * @cassttng PostProcessor/\%name\%/{Detector}\n
 *           Name of the Detector that one is interested in. Default "blubb"
 *           See cass::HelperPixelDetectors for infos how to set up that
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
};




/** Number of coalesced PhotonHits of CCD's.
 *
 * This postprocessor retrieve how many coalesced photonhits have been
 * detected in a ccd frame.
 *
 * @see PostprocessorBackend for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{Detector}\n
 *           Name of the Detector that one is interested in. Default "blubb"
 *           See cass::HelperPixelDetectors for infos how to set up that
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




/** split level of the  PhotonHits of CCD's.
 *
 * This postprocessor creates a 1d histogram displaying what the split level
 * of the photonhit was (how many pixels contributed to the photonhit)
 *
 * @see PostprocessorBackend for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{Detector}\n
 *           Name of the Detector that one is interested in. Default "blubb"
 *           See cass::HelperPixelDetectors for infos how to set up that
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


}//end namespace cass
#endif
