// Copyright (C) 2011 Lutz Foucar

/**
 * @file lma_file_header.h contains the layout of the lma file headers.
 *
 * @author Lutz Foucar
 */

#include <stdint.h>

namespace cass
{
namespace lmaFile
{
#pragma pack(2)
/** the general file header
 *
 * @author Lutz Foucar
 */
struct GeneralHeader
{
  /** size of the Header in bytes */
  int32_t headersize;

  /** nbr of channels in the instrument */
  int16_t nbrChannels;

  /** the size of the entries in the waveform in bits */
  int16_t nbrBits;

  /** the sampling interval (the time between two datapoints in s */
  double samplingInterval;

  /** number of samples in the waveform */
  int32_t nbrSamples;

  /** time between trigger and first sample */
  double delayTime;

  /** the triggering channel */
  int16_t triggerChannel;

  /** triggering level of the trigger in V */
  double triggerLevel;

  /** on which slope the instruments triggers on */
  int16_t triggerSlope;

  /** bitmask describing which channels are recorded */
  int32_t usedChannelBitmask;

  /** bitmask describing which channels are combined */
  int32_t channelCombinationBitmask;

  /** how many converters are being used per channel */
  int16_t nbrConvertersPerChan;
};

/** the header of an recorded channel
 *
 * @author Lutz Foucar
 */
struct ChannelHeader
{
  /** the full scale of the channel in mV */
  int16_t fullscale_mV;

  /** the offset of the channel in mV */
  int16_t offset_mV;

  /** the vertical Gain (conversion factor to convert the bits to mV) */
  double gain_mVperLSB;

  /** baseline that was used for zero substraction in digitizer units */
  int16_t baseline;

  /** noiselevel for zero substraction in digitizer units
   *
   * the zero substaction will check whether a value of the recorded waveform
   * is outside the noiselevel. Mathematically:
   * \f \left| value_Du - baseline \right| > noiselevel \f
   */
  int16_t noiseLevel;

  /** stepsize in sample interval units
   *
   * after finding a waveform value thats outside the noiselevel this is the
   * amount of values skipped before checking whether the wavefrom is in the
   * noiselevel again.
   */
  int32_t stepsize;

  /** backsize of the zerosubstraction in sample interval units
   *
   * how many steps we should go back after a value is outside the noiselevel
   * to start recording the wavefrom values
   *
   * puls of waveform outside noise := puls;
   * waveform index first value outside noise := puls[i]
   * puls.begin = puls[i-backsize]
   */
  int32_t backsize;
};
} //end namespace lmafile
#pragma pack()
} //end namespace cass
