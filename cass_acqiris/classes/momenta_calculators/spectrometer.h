//Copyright (C) 2009-2010 Lutz Foucar

/**
 * @file spectrometer.cpp file contains the classes that describe a REMI type
 *                        spectrometer.
 *
 * @author Lutz Foucar
 */

#ifndef __Spectrometer_H_
#define __Spectrometer_H_

#include <vector>

#include "cass_acqiris.h"

namespace cass
{
  class CASSSettings;

  namespace ACQIRIS
  {
    class Particle;

    /** a region of a spectrometer
     *
     * this class defines a region of a REMI type spectrometer (Spectrometer)
     * It has a length and a electric field strength.
     *
     * @cassttng AcqirisDetectors/\%detectorname\%/\%particlename%/Spectrometer/Regions/\%regionindex\%/{Length}}\n
     *           The length of the spectrometer region in mm. Default is 10.
     * @cassttng AcqirisDetectors/\%detectorname\%/\%particlename%/Spectrometer/Regions/\%regionindex\%/{EField}}\n
     *           The strength of the electric field in the spectrometer region
     *           in \f$\frac{V}{cm}\f$. Default is 10.
     *
     * @author Lutz Foucar
     */
    class SpectrometerRegion
    {
    public:
      /** load the settings from .ini file
       *
       * @param s the CASSSettings object to read the information from
       */
      void loadSettings(CASSSettings&s);

      //@{
      /** retrieve region properties */
      double EField_Vpcm()const  {return _efield;}
      double length_mm()const    {return _length;}
      //@}

    private:
      /** the length of the spectrometer region in mm */
      double _length;
      /** the electric field strength inside this region in \f$\frac{V}{cm}\f$ */
      double _efield;
    };

    /** a REMI type spectrometer
     *
     * class describes a remi type spectroemter that has one or more separeted
     * regions. In the spectrometer a magenetic field can be present. In this
     * case it is important to know in which direction the magnetic field is
     * pointing and how strong it is. The latter can be measured by the cyclotron
     * period of the electrons and the former by the rotation direction of the
     * electrons.
     *
     * @cassttng AcqirisDetectors/\%detectorname\%/\%particlename%/Spectrometer/{BFieldIsOn}}\n
     *           Flag to tell whether a magnetic field is present in the
     *           spectrometer. Default is false
     * @cassttng AcqirisDetectors/\%detectorname\%/\%particlename%/Spectrometer/{RotationClockwise}}\n
     *           Flag to tell in which direction the magentic field is pointing
     *           This can be told by telling in which way the electrons rotate.
     *           If they rotate clockwise this should be true. Default is true.
     * @cassttng AcqirisDetectors/\%detectorname\%/\%particlename%/Spectrometer/{CyclotronPeriode}}\n
     *           The time it takes for an electron to make a complete rotation
     *           in the magnetic field in ns. Default is 10
     *
     * @author Lutz Foucar
     */
    class Spectrometer
    {
    public:
      /** typef for better readable code */
      typedef std::vector<SpectrometerRegion> regions_t;

      /** load the settings from .ini file
       *
       * load all spectrometer regions from file before loading our own members.
       * We need to convert the cyclotron period that is given by the user in
       * electron periods to the period that this particle has. Therefore we
       * need to multiply the period by the ratio of mass to charge of the
       * particle.
       *
       * @param s the CASSSettings object to read the information from
       * @param particle the particle that this spectrometer belongs to
       */
      void loadSettings(CASSSettings &s, const Particle& particle);

      //@{
      /** retrieve magnetic field parameter */
      double cyclotronPeriod_ns()const  {return _cyclotronPeriod;}
      bool BFieldIsOn()const            {return _BFieldIsOn;}
      bool rotationClockWise()const     {return _rotationClockwise;}
      //@}

      /** retrieve the regions of the spectrometer */
      const regions_t& regions()const {return _regions;}

    private:
      /** the regions of the spectrometer */
      regions_t _regions;

      /** flag to tell whether the magnetic field is turned on */
      bool _BFieldIsOn;

      /** the cyclotron frequency of the particles in ns*/
      double _cyclotronPeriod;

      /** flag showing the direction of the magnetic field
       *
       * this is done by saying wether electrons turn clock or counterclockwise
       */
      bool _rotationClockwise;
    };
  }
}
#endif
