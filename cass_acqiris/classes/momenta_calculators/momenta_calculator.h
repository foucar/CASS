//Copyright (C) 2001-2010 Lutz Foucar

/**
 * @file momenta_calculator.h file contains the classes that calculate the
 *                            momenta of particles from their detector hits.
 *
 * @author Lutz Foucar
 */

#ifndef __MomentaCalculator_H_
#define __MomentaCalculator_H_

#include <memory>

#include "cass_acqiris.h"

namespace cass
{
  class CASSSettings;

  namespace ACQIRIS
  {
    class Particle;

    /** copy and correct detectorhit properties
     *
     * copy the position and time value from the detectorhit and then correct
     * those values.
     *
     * @cassttng AcqirisDetectors/\%detectorname\%/Particles/\%particlename%/Corrections/{T0}\n
     *           Time in ns that should be substracted from the recorded time
     *           of the detectorhit. Default is 0.
     * @cassttng AcqirisDetectors/\%detectorname\%/Particles/\%particlename%/Corrections/{CorrectX|CorrectY}\n
     *           Position in mm that should be substracted from the x and y
     *           position of the detectorhit. Default is 0|0.
     * @cassttng AcqirisDetectors/\%detectorname\%/\%particlename%/Corrections/{ScaleX|ScaleY}\n
     *           Factor by which the corrected position should be multiplied, to
     *           get a right sized image. Default is 1|1.
     * @cassttng AcqirisDetectors/\%detectorname\%/Particles/\%particlename%/Corrections/{Angle}\n
     *           Angle in degree around which the corrected and scaled position
     *           should be roated. Default is 0.
     *
     * @author Lutz Foucar
     */
    class HitCorrector
    {
    public:
      /** load the settings
       *
       * load the correction factors from the .ini file
       *
       * @param s the CASSSettings object to read the information from
       */
      void loadSettings(CASSSettings &s);

      /** correct the position in the detector plane
       *
       * Create a particlehit.
       * Then copy the position and the time from the dethit to the particlehit.
       * Then correct this position, then scale the correted postion and finaly
       * rotate the corrected scaled positition around the given angle.
       * Then substract _t0 form the time of the detectorhit
       *
       * @param[in] dethit the detector hit to correct
       * @param[out] particlehit this is where the correct position goes
       */
      particleHit_t operator()(const detectorHit_t &dethit)const;

    private:
      /** the correction factor of the time of flight */
      double _t0;

      /** the correction of the position */
      std::pair<double,double> _pos0;

      /** the correction of the scale */
      std::pair<double,double> _scalefactors;

      /** the angle to rotate the position */
      double _angle;
    };

    /** base class for calculating momenta from a detector hit
     *
     * @author Lutz Foucar
     */
    class MomentumCalculator
    {
    public:
      /** enum for the types of momcalculators */
      enum MomCalcType{PxPyWBField, PxPyWOBField, PzOneRegion, PzMultipleRegions};

      /** calculate the momenta
       *
       * calculates the momenta of the particle form a given detectorhit. First
       * correct the position of the detectorhit and the time of flight.
       *
       * @param[in] dethit the detectorhit to calculate the momenta from
       * @param[in] particle the particle object that contains the properties of
       *                     the particle that the momentum needs to be
       *                     calculated from
       * @param[out] particlehit the particle hit that contains all momenta
       */
      virtual particleHit_t& operator()(const Particle &particle, particleHit_t& particlehit)const=0;

      /** create instance of requested type
       *
       * creates an instance of the requested type
       *
       * @return pointer to the instance of the requested type
       * @param type the type of momentum calculator requested
       */
      static std::auto_ptr<MomentumCalculator> instance(const MomCalcType &type);

    };

    /** calculate px,py momenta
     *
     * This calcultates the momenta components of the particle in the
     * detectorplane for a spectrometer without a magnetic field.
     *
     * @author Lutz Foucar
     */
    class PxPyCalculatorWithoutBField : public MomentumCalculator
    {
    public:
      particleHit_t& operator()(const Particle &particle, particleHit_t& particlehit)const;
    };

    /** calculate px,py momenta
     *
     * This calcultates the momenta components of the particle in the
     * detectorplane for a spectrometer with a magnetic field.
     *
     * @author Lutz Foucar
     */
    class PxPyCalculatorWithBField : public MomentumCalculator
    {
    public:
      particleHit_t& operator()(const Particle &particle, particleHit_t& particlehit)const;
    };

    /** calculate pz momenta
     *
     * This calcultates the momenta componetnt of the particle along the time of
     * flight for a spectrometer with only one region. This can be done
     * analytical.
     *
     * @author Lutz Foucar
     */
    class PzCalculatorDirectOneRegion : public MomentumCalculator
    {
    public:
      particleHit_t& operator()(const Particle &particle, particleHit_t& particlehit)const;
    };

    /** calculate pz momenta
     *
     * This calcultates the momenta componetnt of the particle along the time of
     * flight for a spectrometer with more than one region. We do this iterativly.
     *
     * @author Lutz Foucar
     */
    class PzCalculatorMulitpleRegions : public MomentumCalculator
    {
    public:
      particleHit_t& operator()(const Particle &particle, particleHit_t& particlehit)const;
    };
  }
}
#endif
