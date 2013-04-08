//Copyright (C) 2012 Lutz Foucar

/**
 * @file image_generator.h file contains a class for image generation
 *
 * @author Lutz Foucar
 */

#ifndef _IMAGEGENERATOR_H
#define _IMAGEGENERATOR_H

#include "data_generator.h"

namespace cass
{
/** generate a waveform of a chosen type
 *
 * @GenList "Image": Generates a user defined image

 * @cassttng TestInput/ImageGenerator/{InstrumentId} \n
 *
 * @author Lutz Foucar
 */
class ImageGenerator : public DataGenerator
{
public:
  /** constructor
   *
   * does nothing
   */
  ImageGenerator();

  /** loads the parameters from the ini file */
  void load();

  /** fills the cass event with a random waveform
   *
   * @param evt the cassevent that should be filled.
   */
  void fill(CASSEvent& evt);

private:
  /** object to register this PostProcessor type to the factory */
  static DataGeneratorRegister<ImageGenerator> reg;
};
}//end namespace cass

#endif
