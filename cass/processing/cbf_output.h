// Copyright (C) 2012 Lutz Foucar

/**
 * @file cbf_output.h output of 2d histograms into the cbf.
 *
 * @author Lutz Foucar
 */

#ifndef _CBF_OUTPUT_H_
#define _CBF_OUTPUT_H_

#include <string>

#include "processor.h"

namespace cass
{
/** converts histograms to (c)rystal (b)inary (f)ormat files.
 *
 * @PPList "pp1500": converts histograms to (c)rystal (b)inary (f)ormat files.
 *
 * @cassttng Processor/\%name\%/{Name} \n
 *           processor name containing the histogram that write to cbf.
 *           If not set then no cbf file will be written when condition is true
 * @cassttng Processor/\%name\%/{SummaryName} \n
 *           processor name containing Histogram to be written as summary to
 *           cbf. If not set then no summary will be written
 * @cassttng Processor/\%name\%/{FileBaseName} \n
 *           Default name given by program parameter
 * @cassttng Processor/\%name\%/{MaximumNbrFilesPerDir} \n
 *           Distribute the files over subdirectories where each subdir contains
 *           this amount of files. If -1 it will not distribute the files.
 *           Default is -1.
 *
 * @author Stephan Kassemeyer
 * @author Lutz Foucar
 */
class pp1500 : public Processor
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
  virtual const result_t& result(const CASSEvent::id_t eventid=0);

  /** overwrite the release */
  virtual void releaseEvent(const CASSEvent &){}

protected:
  /** the filename that the data will be written to */
  std::string _basefilename;

  /** pp containing histogram to dump to cbf */
  shared_pointer _pHist;

  /** pp containing histogram that will be written as summary to dump to cbf */
  shared_pointer _summaryHist;

  /** the number of files in each subdir */
  int _maxFilePerSubDir;

  /** counter to count how many files have been written */
  int _filecounter;

private:
  /** a lock to make the process reentrant */
  QMutex _lock;
};
} //end namespace cass
#endif
