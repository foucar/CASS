// Copyright (C) 2011 Lutz Foucar

/**
 * @file advanced_pixeldetector.h advanced pixeldetectors
 *
 * @author Lutz Foucar
 */

#ifndef _PIXELDETECTORNEW_H_
#define _PIXELDETECTORNEW_H_

#include <tr1/memory>

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
 * the frame conststs of the data and columns and rows
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
   *           - 2: 1st commercial CCD
   *           - 3: 2nd commercial CCD
   *           - 4: 3rd commercial CCD
   * @cassttng PixelDetectors/\%name\%/{FrameProcessorType}\n
   *           Functor for processing the frame. Default is "none". Options are:
   *           - "none": No processing is done to the frame, just the raw frame
   *                     will be returned
   *           - "hll": The type of processing that the semiconductor lab applies
   *                    to their frame data. see cass::pixeldetector::HLLProcess
   * @cassttng PixelDetectors/\%name\%/{PixelFinderType}\n
   *           Functor for finding pixels of interest in the frame. The pixels
   *           will be found after the frame processor has processed the frame.
   *           Default is "aboveNoise". Options are:
   *           - "aboveNoise": uses the noise map
   *                           (see cass::pixeldetector::CommonData) to check
   *                           whether a pixel is of interest. See
   *                           cass::pixeldetector::AboveNoise
   *           - "pixfind": checks whether a pixel value is higher than the
   *                        pixelvalues of the neighbours.
   *                        See cass::pixeldetector::PixelFind
   * @cassttng PixelDetectors/\%name\%/{CoalescingFunctionType}\n
   *           Functor to coalesce the pixels into hits. Default is "simple".
   *           Options are:
   *           - "simple": simple coalescing with basic checks.
   *                       See cass::pixeldetector::SimpleCoalesce.
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
   * @param evt Pointer to the CASSEvent that contains the PixelDetector that
   *            this container is responsible for.
   */
  void associate(const CASSEvent &evt);

  /** load the settings of this
   *
   * @param s the CASSSettings object to read the information from
   */
  void loadSettings(CASSSettings &s);

  /** retrieve the frame */
  const Frame& frame();

  /** retrieve the pixellist */
  const pixels_t& pixels();

  /** retrieve the hits */
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
