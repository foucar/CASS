// Copyright (C) 2011 Lutz Foucar

/**
 * @file coalesce_simple.cpp contains class that does the pixel coalescing in a
 *                           simple way.
 *
 * @author Lutz Foucar
 */

#include "coalesce_simple.h"

#include "cass_settings.h"

using namespace cass;
using namespace std;

SimpleCoalesce::SimpleCoalesce()
{}

void SimpleCoalesce::loadSettings()
{
}

PixelDetector::pixelList_t& SimpleCoalesce::operator() (const PixelDetector::pixelList_t &pixellist,
                                                        PixelDetector::pixelList_t &coalescedpixels)
{
  return (coalescedpixels);
}
