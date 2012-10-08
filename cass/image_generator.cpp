//Copyright (C) 2012 Lutz Foucar

/**
 * @file image_generator.cpp file contains a class for image generation
 *
 * @author Lutz Foucar
 */

#include <algorithm>

#include "image_generator.h"

#include "cass_event.h"
#include "cass_settings.h"

using namespace std;
using namespace cass;


DataGeneratorRegister<ImageGenerator> ImageGenerator::reg("Image");

ImageGenerator::ImageGenerator()
{
}

void ImageGenerator::load()
{
  CASSSettings s;
  s.beginGroup("WaveformGenerator");
}

void ImageGenerator::fill(CASSEvent& evt)
{
}
