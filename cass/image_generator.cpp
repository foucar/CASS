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


Registrar<DataGenerator,ImageGenerator> ImageGenerator::reg("Image");

ImageGenerator::ImageGenerator()
{
}

void ImageGenerator::load()
{
  CASSSettings s;
  s.beginGroup("ImageGenerator");
}

void ImageGenerator::fill(CASSEvent& /*evt*/)
{

}
