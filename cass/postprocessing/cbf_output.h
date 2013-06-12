// Copyright (C) 2012 Lutz Foucar

/**
 * @file cbf_output.h output of 2d histograms into the cbf.
 *
 * @author Lutz Foucar
 */

#ifndef _CBF_OUTPUT_H_
#define _CBF_OUTPUT_H_

#include <string>

#include "backend.h"

namespace cass
{
/** converts histograms to (c)rystal (b)inary (f)ormat files.
 *
 * @PPList "pp1500": converts histograms to (c)rystal (b)inary (f)ormat files.
 *
 * @cassttng PostProcessor/\%name\%/{DarkName} \n
 *           Postprocessor name offsetmap to write to cbf when program quits.
 * @cassttng PostProcessor/\%name\%/{HistName} \n
 *           Postprocessor name containing the histogram that write to cbf.
 * @cassttng PostProcessor/\%name\%/{FileBaseName} \n
 *           Default name given by program parameter
 *
 *
 * @author Stephan Kassemeyer
 * @author Lutz Foucar
 */
class pp1500 : public PostProcessor
{
public:
  /** constructor */
  pp1500(const name_t &);

  /** overwrite process event */
  virtual void processEvent(const CASSEvent&);

  /** dump dark to cbf just before quitting */
  virtual void aboutToQuit();

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

  /** overwrite the retrieval of an histogram */
  virtual const HistogramBackend& result(const CASSEvent::id_t eventid=0);

  /** overwrite the release */
  virtual void releaseEvent(const CASSEvent &){}

protected:
  /** the filename that the data will be written to */
  std::string _basefilename;

  /** pp containing histogram to dump to cbf */
  shared_pointer _pHist;

  /** pp containing offset histogram to dump to cbf */
  shared_pointer _darkHist;

private:
  /** a lock to make the process reentrant */
  QMutex _lock;
};
} //end namespace cass
#endif
