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

namespace cass
{
/** generate a waveform of a chosen type
 *
 * @cassttng TestInput/WaveformGenerator/{InstrumentId} \n
 * @cassttng TestInput/WaveformGenerator/{NbrOfSamples} \n
 * @cassttng TestInput/WaveformGenerator/{SampleInterval} \n
 * @cassttng TestInput/WaveformGenerator/Channel/{size} \n
 * @cassttng TestInput/WaveformGenerator/Channel/\%id\%/{Offset} \n
 * @cassttng TestInput/WaveformGenerator/Channel/\%id\%/{FullScale} \n
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

  /** object to register this PostProcessor type to the factory */
  static DataGeneratorRegister<WaveformGenerator> reg;
};
}//end namespace cass

#endif
