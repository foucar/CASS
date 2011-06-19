//Copyright (C) 2010 Lutz Foucar

/**
 * @file machine_data.h file contains declaration of postprocessors that
 *                      extract information from the beamline and epics data.
 *
 * @author Lutz Foucar
 */

#ifndef _MACHINE_DATA_H_
#define _MACHINE_DATA_H_

#include <string>

#include "postprocessor.h"
#include "backend.h"
#include "cass_event.h"

namespace cass
{
  //forward declaration
  class Histogram0DFloat;



  /** retrieval of beamline data.
   *
   * This postprocessor will retrieve the requested Beamline Data from
   * the cass event.
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{VariableName}
   *           The name of the beamline data variable you are interested in.
   *           Default is "". Available values are:
   *           - FEE Gas Detector values
   *             - f_11_ENRC
   *             - f_12_ENRC
   *             - f_21_ENRC
   *             - f_22_ENRC
   *           - E-Beam values
   *             - EbeamCharge
   *             - EbeamL3Energy
   *             - EbeamLTUAngX
   *             - EbeamLTUAngY
   *             - EbeamLTUPosX
   *             - EbeamLTUPosY
   *             - EbeamPkCurrBC2
   *           - Phase Cavity values
   *             - Charge1
   *             - Charge2
   *             - FitTime1
   *             - FitTime2
   *           - Ipimb values
   *             - Channel0
   *             - Channel1
   *             - Channel2
   *             - Channel3
   *
   * @author Lutz Foucar
   */
  class pp120 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp120(PostProcessors& hist, const PostProcessors::key_t&);

    /** copy data from CASS event to histogram storage */
    virtual void process(const CASSEvent&);

    /** load the settings from cass.ini */
    virtual void loadSettings(size_t);

  protected:
    /** name of the variable in the beamline data */
    std::string _varname;
  };







  /** check whether event contains eventcode
   *
   * This postprocessor will check whether an eventcode is present in the event.
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{EventCode}
   *           The EventCode to check for. Default is 0
   *
   * @author Lutz Foucar
   */
  class pp121 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp121(PostProcessors& hist, const PostProcessors::key_t&);

    /** copy data from CASS event to histogram storage */
    virtual void process(const CASSEvent&);

    /** load the settings from cass.ini */
    virtual void loadSettings(size_t);

  protected:
    /** name of the variable in the beamline data */
   size_t _eventcode;
  };











  /** retrieval of Epics data.
   *
   * This postprocessor will retrieve the requested epics data from the cass-event.
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{VariableName}
   *           The name of the epics data variable you are interested in.
   *           Default is "". Please aks your PI which EPICS variables have been
   *           put into the datastream for archiving.
   *
   * @author Lutz Foucar
   */
  class pp130 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp130(PostProcessors& hist, const PostProcessors::key_t&);

    /** copy data from CASS event to histogram storage */
    virtual void process(const CASSEvent&);

    /** load the settings from cass.ini */
    virtual void loadSettings(size_t);

  protected:
    /** name of the variable in the beamline data */
    std::string _varname;
  };










  /** retrieve photonenergy.
   *
   * This postprocessor will calculate the photonenergy from the BLD
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @author Lutz Foucar
   */
  class pp230 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp230(PostProcessors& hist, const PostProcessors::key_t&);

    /** calc the photonenergy from the bld */
    virtual void process(const CASSEvent&);

    /** load the settings from cass.ini */
    virtual void loadSettings(size_t);
  };
}

#endif
