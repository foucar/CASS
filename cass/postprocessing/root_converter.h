// Copyright (C) 2010 Lutz Foucar

#ifndef _ROOT_CONVERTER_H_
#define _ROOT_CONVERTER_H_

#include <string>

#include "postprocessor.h"

namespace cass
{
  /** root file converter
   *
   * will convert all histograms defined in cass to root histograms and write
   * them to a root file. Filename can be chosen with the -o parameter at
   * program start.
   *
   * @author Lutz Foucar
   */
  class pp2000 : public PostprocessorBackend
  {
  public:
    /** Construct postprocessor for converting histograms to root histograms */
    pp2000(PostProcessors&, const PostProcessors::key_t&, std::string rootfilename);

    /** only a stub does nothing, but needs to be there because its pure virtual in base class */
    virtual void process()(const CASSEvent&){}

    /** dump all histogram to a root file just before quitting */
    virtual void aboutToQuit();

  protected:
    /** the root filename where we store the data in */
    std::string _rootfilename;
  };
}

#endif
