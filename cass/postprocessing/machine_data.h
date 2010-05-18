//Copyright (C) 2010 Lutz Foucar

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
   * @cassttng PostProcessor/p\%id\$/{variableName}
   *           The name of the beamline data variable you are interested in
   *
   * Implementes postprocessors 850
   *
   * @author Lutz Foucar
   */
  class pp850 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp850(PostProcessors& hist, PostProcessors::id_t id);

    /** Free _image space */
    virtual ~pp850();

    /** copy data from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

    /** load the settings from cass.ini */
    virtual void loadSettings(size_t);

  protected:
    /** name of the variable in the beamline data */
    std::string _varname;

    /** resulting histgram */
    Histogram0DFloat *_data;
  };











  /** retrieval of Epics data.
   *
   * This postprocessor will retrieve the requested epics data from the cass-event.
   *
   * @cassttng PostProcessor/p\%id\$/{variableName}
   *           The name of the epics data variable you are interested in.
   *
   * Implementes postprocessors 851
   *
   * @author Lutz Foucar
   */
  class pp851 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp851(PostProcessors& hist, PostProcessors::id_t id);

    /** Free _image space */
    virtual ~pp851();

    /** copy data from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

    /** load the settings from cass.ini */
    virtual void loadSettings(size_t);

  protected:
    /** name of the variable in the beamline data */
    std::string _varname;

    /** resulting histgram */
    Histogram0DFloat *_data;
  };
}

#endif
