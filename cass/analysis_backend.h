// Copyright (C) 2009 Jochen Kuepper
// Copyright (C) 2009, 2010 Lutz Foucar

/**
 * @file analysis_backend.h file contains base class for all pre analyzers
 *
 * @author Lutz Foucar
 */

#ifndef CASS_ANALYSISBACKEND_H
#define CASS_ANALYSISBACKEND_H

#include "cass.h"

namespace cass
{
  //forward declarations
  class CASSEvent;

  /** Base class for preanalyzers
   *
   * The Base class for all Preanalyzers.
   *
   * @author Lutz Foucar
   * @author Jochen Kuepper
   */
  class CASSSHARED_EXPORT AnalysisBackend
  {
  public:
    /** virtual desctructor */
    virtual ~AnalysisBackend()  {}

    /** this function is called when the analyzer should load its settings*/
    virtual void loadSettings() = 0;

    /** this function is called when the analyzer should save its settings*/
    virtual void saveSettings() = 0;

    /** this function is called for all cassevents*/
    virtual void operator()(CASSEvent*) = 0;
  };
}

#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
