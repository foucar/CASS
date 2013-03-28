//Copyright (C) 2011 Lutz Foucar

/**
 * @file pnccd_analysis_new.h the pnccd correction in a new way
 *
 * @author Lutz Foucar
 */

#ifndef _PNCCDANALSYSIS_NEW_H_
#define _PNCCDANALSYSIS_NEW_H_

#include "analysis_backend.h"

namespace cass
{
namespace pnCCD
{

/** analyse the pnCCD data
 *
 * Do the necessary corrections to create senseful data from the raw data.
 * Optionally do event finding
 *
 * @author Lutz Foucar
 */
class NewAnalysis : public AnalysisBackend
{
public:
  /** constructor */
  NewAnalysis();

  /** load the settings of the analyser */
  void loadSettings();

  /** save the settings of the analyzer
   *
   * @deprecated
   */
  void saveSettings();

  /** the operator that will do the corrections and anlysis
   *
   * @param evt pointer to the CASSEvent that stored the data of the pnCCD to
   *            work on.
   */
  void operator() (CASSEvent* evt);
};

}
}

#endif

