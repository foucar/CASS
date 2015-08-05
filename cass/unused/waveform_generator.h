//Copyright (C) 2012 Lutz Foucar

/**
 * @file waveform_generator.h file contains a class for waveform generation
 *
 * @author Lutz Foucar
 */

#ifndef _WAVEFORMGENERATOR_H
#define _WAVEFORMGENERATOR_H

#include "data_generator.h"
#include "acqiris_device.h"
#include "generic_factory.hpp"

namespace cass
{
/** generate a waveform of a chosen type
 *
 * @GenList "Waveform": Generates a user defined waveform.
 *
 * @cassttng WaveformGenerator/{InstrumentId} \n
 * @cassttng WaveformGenerator/{NbrOfSamples} \n
 * @cassttng WaveformGenerator/{SampleInterval} \n
 * @cassttng WaveformGenerator/Channel/{size} \n
 * @cassttng WaveformGenerator/Channel/\%id\%/{Offset} \n
 * @cassttng WaveformGenerator/Channel/\%id\%/{FullScale} \n
 *
 * @author Lutz Foucar
 */
class WaveformGenerator : public DataGenerator
{
public:
  /** constructor
   *
   * does nothing
   */
  WaveformGenerator();

  /** loads the parameters from the ini file */
  void load();

  /** fills the cass event with a random waveform
   *
   * @param evt the cassevent that should be filled.
   */
  void fill(CASSEvent& evt);

private:
  /** an acqiris instrument */
  ACQIRIS::Instrument _instrument;

  /** the id of the instrument */
  int _instrID;

  /** object to register this data generator to the factory */
  static Registrar<DataGenerator,WaveformGenerator> reg;
};
}//end namespace cass

#endif
