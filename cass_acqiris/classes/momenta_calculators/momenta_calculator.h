//Copyright (C) 2001-2010 Lutz Foucar

/**
 * @file momenta_calculator.h file contains the classes that calculate the
 *                            momenta of particles from their detector hits.
 *
 * @author Lutz Foucar
 */

#ifndef __MomentaCalculator_H_
#define __MomentaCalculator_H_

#include "cass_acqiris.h"

namespace cass
{
  class CASSSettings;

  namespace ACQIRIS
  {
    class Particle;

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
      virtual void operator()(const detectorHit_t &dethit, const Particle &particle, particleHit_t& particlehit)const=0;

      /** load the settings
       *
       * load the correction factors from the .ini file
       *
       * @param s the CASSSettings object to read the information from
       */
      void loadSettings(CASSSettings &s);

      /** create instance of requested type
       *
       * creates an instance of the requested type
       *
       * @return pointer to the instance of the requested type
       * @param type the type of momentum calculator requested
       */
      static MomentumCalculator* instance(const MomCalcType &type);

    protected:
      /** correct the position in the detector plane
       *
       * First copy the position from the dethit, then correct this position,
       * then scale the correted postion and finaly rotate the corrected scaled
       * positition around the given angle.
       *
       * @param[in] dethit the detector hit to correct
       * @param[out] particlehit this is where the correct position goes
       */
      void correctDetectorPlane(const detectorHit_t &dethit, particleHit_t& particlehit)const;

      /** correct the time of flight
       *
       * after copying the time of the detector hit substract _t0 from it.
       *
       * @param[in] dethit the detector hit to correct
       * @param[out] particlehit this is where the correct position goes
       */
      void correctTof(const detectorHit_t &dethit, particleHit_t& particlehit)const;

    protected:
      /** the correction factor of the time of flight */
      double _t0;

      /** the correction of the position */
      std::pair<double,double> _pos0;

      /** the correction of the scale */
      std::pair<double,double> _scalefactors;

      /** the angle to rotate the position */
      double _angle;
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
      void operator()(const detectorHit_t &dethit, const Particle &particle, particleHit_t& particlehit)const;
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
      void operator()(const detectorHit_t &dethit, const Particle &particle, particleHit_t& particlehit)const;
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
      void operator()(const detectorHit_t &dethit, const Particle &particle, particleHit_t& particlehit)const;
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
      void operator()(const detectorHit_t &dethit, const Particle &particle, particleHit_t& particlehit)const;
    };
  }
}
#endif
