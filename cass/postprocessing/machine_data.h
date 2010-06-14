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
   * @cassttng PostProcessor/\%name\%/{VariableName}
   *           The name of the beamline data variable you are interested in
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





  /** retrieval of Epics data.
   *
   * This postprocessor will retrieve the requested epics data from the cass-event.
   *
   * @cassttng PostProcessor/\%name\%/{VariableName}
   *           The name of the epics data variable you are interested in.
   *
   * @author Lutz Foucar
   */
  class pp130 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp130(PostProcessors& hist, const PostProcessors::key_t&);

    /** Free _image space */
    virtual ~pp130();

    /** copy data from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

    /** load the settings from cass.ini */
    virtual void loadSettings(size_t);

  protected:
    /** name of the variable in the beamline data */
    std::string _varname;

    /** requested value */
    Histogram0DFloat *_value;
  };
















  /** retrieve photonenergy.
   *
   * This postprocessor will calculate the photonenergy from the BLD
   *
   * @author Lutz Foucar
   */
  class pp230 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp230(PostProcessors& hist, const PostProcessors::key_t&);

    /** Free _image space */
    virtual ~pp230();

    /** copy data from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

  protected:
    /** resulting histgram */
    Histogram0DFloat *_data;
  };
}

#endif
