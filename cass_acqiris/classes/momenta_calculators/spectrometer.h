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
    /** a region of a spectrometer
     *
     * this class defines a region of a REMI type spectrometer (Spectrometer)
     * It has a length and a electric field strength
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
      /** the electric field strength inside this region in \frac{V}{cm} */
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
     * @author Lutz Foucar
     */
    class Spectrometer
    {
    public:
      /** typef for better readable code */
      typedef std::vector<SpectrometerRegion> regions_t;

      /** load the settings from .ini file
       *
       * @param s the CASSSettings object to read the information from
       */
      void loadSettings(CASSSettings &s);

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
