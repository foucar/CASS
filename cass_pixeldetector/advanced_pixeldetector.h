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
struct CommonData;
class CoalescingBase;
class FrameAnalyzerBase;
class FrameExtractorBase;

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
   * @cassttng PixelDetectors/\%name\%/{Device}\n
   *           The device that contains the ccd image.Default is 0. Options are:
   *           - 0: pnCCD
   *           - 2: Commercial CCD
   * @cassttng PixelDetectors/\%name\%/{Detector}\n
   *           The detector that contains the ccd image. Default is 0. Options are:
   *           - 0: Front pnCCD / Commercial CCD
   *           - 1: Rear pnCCD
   * @cassttng PixelDetectors/\%name\%/{CoalescingFunctionType}\n
   *           Functor to coalesce the pixels into hits. Default is "simple". Options are:
   *           - "simple": simple coalescing with basic checks.
   *                       See cass::SimpleCoalesce.
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
  AdvancedDetector(const std::string &name)
    : _frameExtracted(false),
      _pixellistCreated(false),
      _hitListCreated(false),
      _name(name)
  {}

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
  std::tr1::shared_ptr<FrameExtractorBase> _extract;

  /** the list of pixels */
  pixels_t _pixels;

  /** flag to tell whether the pixel list has been created */
  bool _pixellistCreated;

  /** functor to extract the pixel list */
  std::tr1::shared_ptr<FrameAnalyzerBase> _analyze;

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
