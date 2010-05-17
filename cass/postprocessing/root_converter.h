// Copyright (C) 2010 Lutz Foucar

#ifndef _ROOT_CONVERTER_H_
#define _ROOT_CONVERTER_H_

#include <string>

#include "postprocessor.h"

namespace cass
{
  class pp2000 : public PostprocessorBackend
  {
  public:
    /*! Construct postprocessor for converting histograms to root histograms */
    pp2000(PostProcessors&, PostProcessors::id_t, std::string rootfilename);

    /** when we are destroyed write all pp histograms to a root file*/
    virtual ~pp2000();

    /** only a stub does nothing, but needs to be there because its pure virtual in base class */
    virtual void operator()(const CASSEvent&){}

  protected:
    /** the root filename where we store the data in */
    std::string _rootfilename;
  };
}

#endif
