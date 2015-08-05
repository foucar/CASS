//Copyright (C) 2009-2010 Lutz Foucar

/**
 * @file particle.h file contains the classes that describe a particle that hit
 *                       a delayline detector.
 *
 * @author Lutz Foucar
 */

#ifndef __MyParticle_H__
#define __MyParticle_H__

#include <vector>
#include <memory>
#include <tr1/memory>
#include <string>

#include "spectrometer.h"
#include "momenta_calculator.h"
#include "acqiris_analysis_definitions.hpp"

namespace cass
{
class CASSSettings;

namespace ACQIRIS
{
class IsParticleHit;
class MomentumCalculator;
class DelaylineDetector;

/** A Particle
 *
 * class describing a particle. It contains the properties of a particle,
 * classes that can calculate the momentum of the particle. Therefore it
 * also contains the spectrometer through which the particle flys and a
 * class that can correct the values from the detectorhit. To find out which
 * of the provided detectorhits are a particle hit of this particle it owns
 * a comparator class.
 *
 * @cassttng AcqirisDetectors/\%detectorname\%/Particles/\%particlename%/{Charge}\n
 *           The charge of the particle in atomic units. Default is 1.
 * @cassttng AcqirisDetectors/\%detectorname\%/\%particlename%/{Mass}\n
 *           The Mass of the particle in atomic mass units. When one wants
 *           to define a electron the charge has to be -1 and the Mass 1.
 *           Default is 1.
 * @cassttng AcqirisDetectors/\%detectorname\%/Particles/\%particlename%/{Spectrometer}\n
 *           The Spectrometer that the Particles fly through.
 *           See cass::ACQIRIS::Spectrometer
 * @cassttng AcqirisDetectors/\%detectorname\%/Particles/\%particlename%/{Corrections}\n
 *           The corrections that one has to do to the raw values of a hit
 *           in order to convert the hit values to momentum.
 *           See cass::ACQIRIS::HitCorrector
 * @cassttng AcqirisDetectors/\%detectorname\%/Particles/\%particlename%/{ConditionType}\n
 *           The type of condition that we use to identify a particle hit
 *           from a detectorhit. Default is 0. Possible choises are:
 *           - 0: condition on time of flight:
 *                See cass::ACQIRIS::TofCond
 *           - 1: condition on a radius with choseable center of the position:
 *                See cass::ACQIRIS::RadCond
 *           - 2: condition on a rectangle of the position:
 *                See cass::ACQIRIS::RectCond
 *           - 3: combination of condition 0 and 1:
 *                See cass::ACQIRIS::TofCond and cass::ACQIRIS::RadCond
 *           - 4: combination of condition 0 and 2:
 *                See cass::ACQIRIS::TofCond and cass::ACQIRIS::RadCond
 *
 * @author Lutz Foucar
 */
class Particle
{
public:
  /** constructor
   *
   * set the _listIsCreated flag to false
   */
  Particle()
    :_listIsCreated(false)
  {}

  /** load the settings from .ini file
   *
   * First load the spectrometers settings, then the hitcorrector settings.
   * Then load our properties. Then create the comaprison object that the
   * user has chosen and load its settings. After that depending on the
   * spectrometer type load the momentum calculation objects.
   * The mass is always converted from atomic mass units to atomic units.
   * Only if the mass is 1 and the charge is -1 (this is true for an electron)
   * the conversion will be omited.
   *
   * @param s the CASSSettings object to read the info from
   */
  void loadSettings(CASSSettings &s);

  /** retrieve the particle hits
   *
   * goes through the detectorhits and checks whether the hit is a particle
   * of our type. Then calculates the momenta in kartesian and
   * polarcoordinates and puts the new hit into the list. Then returns the
   * list.
   * When the former is already done once, just return the list.
   *
   * @return the list of particle hits
   */
  particleHits_t& hits();

  /** tell which are the detector hits to search through
   *
   * after telling us which detectohit list to go through to search for
   * particle hits, this resets the _listIsCreated flag and clears the
   * _particlehits container.
   *
   * @param detector pointer to the detector containing this particle
   */
  void associate(DelaylineDetector * detector);

  /** retrive the spectormeter */
  const Spectrometer& spectrometer()const {return _spectrometer;}

  //@{
  /** retrieve the particle properties */
  double mass_au()const   {return _mass_au;}
  double charge_au()const {return _charge_au;}
  //@}

private:
  /** the list of particle hits */
  particleHits_t _particlehits;

  /** pointer to the delayline detector that conatins this particle */
  DelaylineDetector *_detector;

  /** flag to tell whether we already created the list of particle hits */
  bool _listIsCreated;

  /** identifer for checking detectorhit is a particle hit */
  std::tr1::shared_ptr<IsParticleHit> _isParticleHit;

  /** calculate momenta along the detector plane of a particle hit */
  std::tr1::shared_ptr<MomentumCalculator> _calc_detplane;

  /** calculate momenta along the ToF direction of a particle hit */
  std::tr1::shared_ptr<MomentumCalculator> _calc_tof;

  /** copy and correction of the detectorhit */
  HitCorrector _copyandcorrect;

  /** the spectrometer of this particle */
  Spectrometer _spectrometer;

  /** the Mass of this Particle in atomic units */
  double _mass_au;

  /** the charge of this Particle in atomic units */
  double _charge_au;
};
}//end namespace acqiris
}//end namespace cass
#endif
