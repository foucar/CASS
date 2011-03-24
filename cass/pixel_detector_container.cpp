// Copyright (C) 2011 Lutz Foucar

/**
 * @file pixel_detector_container.h file container for pixeldetectors
 *
 * @author Lutz Foucar
 */

#include "pixel_detector_container.h"

#include "coalescing_base.h"

using namespace cass;

const PixelDetector::pixelList_t & PixelDetectorContainer::coalescedPixels()
{
  if (!_coalescedCreated)
  {
    CoalescingBase & coalesce (*_coalesce);
    coalesce(_pixeldetector->pixellist(), _coalescedpixels);
    _coalescedCreated = true;
  }
  return _coalescedpixels;
}

void PixelDetectorContainer::associate(PixelDetector *in)
{
  _pixeldetector = in;
  _coalescedCreated = false;
}
