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

namespace cass
{
namespace pixeldetector
{
//forward declaration
class CoalescingBase;
class CASSSettings;
class CASSEvent;
class FrameAnalyzerBase;
class FrameExtractorBase;


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
  const Detector::frame_t& frame();

  /** retrieve the pixellist */
  const pixels_t& pixellist();

  /** retrieve the hits */
  const hits_t& hits();



private:
  /** the frame of the detector */
  Detector::frame_t _frame;

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

  /** device that contains the right pixel detector */
  int32_t _device;
};

}
}
#endif
