// Copyright (C) 2011 Lutz Foucar

/**
 * @file advanced_pixeldetector.h advanced pixeldetectors
 *
 * @author Lutz Foucar
 */

#ifndef _PIXELDETECTORNEW_H_
#define _PIXELDETECTORNEW_H_

#include <tr1/memory>
#include <stdint.h>

#include "pixeldetector.hpp"
#include "cass_pixeldetector.h"

namespace cass
{
//forward declaration
class CASSSettings;
class CASSEvent;

namespace pixeldetector
{
//forward declaration
class CommonData;
class FrameProcessorBase;
class PixelFinderBase;
class CoalescingBase;

/** A Frame of an advance Pixel Detector
 *
 * the frame consists of the data and columns and rows
 *
 * @author Lutz Foucar
 */
struct Frame
{
  /** how many columns */
  uint16_t columns;

  /** how many rows */
  uint16_t rows;

  /** the frame data */
  frame_t data;
};

/** An Advanced Pixel Detector
 *
 * This class describes a pixel detector which has all the opertors to analyse
 * and extract the additional information internally.
 *
 * @cassttng PixelDetectors/\%name\%/{Detector}\n
 *           The detector that contains the ccd image. Default is 0. Options are:
 *           - 0: Front pnCCD
 *           - 1: Rear pnCCD
 *           - 2: 1st commercial CCD in AMO
 *           - 3: 2nd commercial CCD in AMO
 *           - 4: 3rd commercial CCD in AMO
 *           - 5: 1st commercial CCD in XPP
 * @cassttng PixelDetectors/\%name\%/{FrameProcessorType}\n
 *           Functor for processing the frame. Default is "none". Options are:
 *           - "none": No processing is done to the frame, just the raw frame
 *                     will be returned
 *           - "hll": The type of processing that the semiconductor lab applies
 *                    to their frame data. see cass::pixeldetector::HLLProcessor
 * @cassttng PixelDetectors/\%name\%/{PixelFinderType}\n
 *           Functor for finding pixels of interest in the frame. The pixels
 *           will be found after the frame processor has processed the frame.
 *           Default is "aboveNoise". Options are:
 *           - "aboveNoise": uses the noise map
 *                           (see cass::pixeldetector::CommonData) to check
 *                           whether a pixel is of interest. See
 *                           cass::pixeldetector::AboveNoiseFinder
 *           - "simple": checks whether a pixel value is higher than the
 *                       pixelvalues of the neighbours.
 *                       See cass::pixeldetector::PixelFinderSimple
 *           - "simpleMoreOptions": checks whether a pixel value is higher than the
 *                       pixelvalues of the neighbours defining a box
 *                       See cass::pixeldetector::PixelFinderSimpleMoreOptions
 *           - "range": checks whether the pixel value is a user set range. See
 *                      cass::pixeldetector::WithinRange
 * @cassttng PixelDetectors/\%name\%/{CoalescingFunctionType}\n
 *           Functor to coalesce the pixels into hits. Default is "simple".
 *           Options are:
 *           - "simple": simple coalescing with basic checks.
 *                       See cass::pixeldetector::SimpleCoalesce.
 * @cassttng there are more settings for the common data. Please see
 *           cass::pixeldetector::CommonData for details on what to set.
 *
 * @author Lutz Foucar
 */
class AdvancedDetector
{
public:
  /** define the list of coalesced pixels */
  typedef std::vector<Hit> hits_t;

  typedef std::vector<Pixel> pixels_t;

  /** constructor
   *
   * @param name the name of this detector
   */
  AdvancedDetector(const std::string &name);

  /** associate the detector with a simple Pixel Detector within a CASSEvent
   *
   * resets the flags indicating whether the frame, the pixel list and the hit
   * list have been created. Copies the Frame data the info about the columns
   * and rows to the _frame object of this class. The _frame object is then
   * passed to the _common object. This should then build up the necessary
   * Maps for correcting. See CommonData for details
   *
   * @param evt The CASSEvent that contains the PixelDetector that this
   *            container is responsible for.
   */
  void associate(const CASSEvent &evt);

  /** load the settings of this
   *
   * loads which FrameProcessorBase functor should be used, get an instance of
   * the right type and load its settings.
   * loads which PixelFinderBase functor should be used, get an instance of
   * the right type and load its settings.
   * loads which CoalescingBase functor should be used, get an instance of
   * the right type and load its settings.
   * the loads the seetings for the common data. See CommonData for details.
   *
   * @param s the CASSSettings object to read the information from
   */
  void loadSettings(CASSSettings &s);

  /** retrieve the frame
   *
   * the frame from the cass event is treated with the frame processor _process
   * before it is returned, but only when it has not yet been treated. For
   * all available options see this classes cass settings information.
   */
  const Frame& frame();

  /** retrieve the pixellist
   *
   * the pixels list contains all pixels that fullfill a certain criteria. They
   * are found in the processed frame using different finding functions. For a
   * list of all available finding procedures please refer to the description
   * of this class.
   */
  const pixels_t& pixels();

  /** retrieve the hits/
   *
   * Hits are defined as particles or photons that hit the detector. Those hits
   * can potentially not only be detected by just one pixel. Therefore one has
   * to find the pixels that belong to one hit on the detector. This is done
   * by the coalsecing functions available to this class (for a complete
   * list of all available finding procedures please refer to the description
   * of this class). The coalsecing functions work on the pixellist that is
   * created from the processed frame.
   */
  const hits_t& hits();

private:
  /** container for data common for all detectors with this name */
  std::tr1::shared_ptr<CommonData> _common;

  /** the frame of the detector */
  Frame _frame;

  /** flag to tell whether the frame has been extracted already */
  bool _frameExtracted;

  /** functor to extract the frame from the CASSEvent */
  std::tr1::shared_ptr<FrameProcessorBase> _process;

  /** the list of pixels */
  pixels_t _pixels;

  /** flag to tell whether the pixel list has been created */
  bool _pixellistCreated;

  /** functor to extract the pixel list */
  std::tr1::shared_ptr<PixelFinderBase> _find;

  /** hits on the detector */
  hits_t _hits;

  /** flag whether hit list has been created already */
  bool _hitListCreated;

  /** functor that will do the coalescing */
  std::tr1::shared_ptr<CoalescingBase> _coalesce;

  /** the name of this detector */
  std::string _name;

  /** the detector within the device */
  int32_t _detector;
};

}
}
#endif
