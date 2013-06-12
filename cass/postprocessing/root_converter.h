// Copyright (C) 2010 Lutz Foucar

/**
 * @file root_converter.h file contains declaration of postprocessor 2000
 *
 * @author Lutz Foucar
 */

#ifndef _ROOT_CONVERTER_H_
#define _ROOT_CONVERTER_H_

#include <string>

#include "postprocessing/backend.h"

class TFile;

namespace cass
{
/** root file converter
 *
 * @PPList "2000": Writes histograms to root file
 *
 * will convert all histograms defined in cass to root histograms and write
 * them to a root file. Filename can be chosen with the -o parameter at
 * program start.
 *
 * @todo make sure that only the requested list will be written.
 *
 * @author Lutz Foucar
 */
class pp2000 : public PostProcessor
{
public:
  /** Construct postprocessor for converting histograms to root histograms */
  pp2000(const name_t&name, std::string rootfilename);

  /** only a stub does nothing, but needs to be there because its pure virtual in base class */
  virtual void processEvent(const CASSEvent&);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

  /** dump all histogram to a root file just before quitting */
  virtual void aboutToQuit();

  /** overwrite the retrieval of an histogram */
  virtual const HistogramBackend& result(const CASSEvent::id_t eventid=0);

  /** overwrite the release */
  virtual void releaseEvent(const CASSEvent &){}

protected:
  /** the root filename where we store the data in */
  //    std::string _rootfilename;

  /** the root file */
  TFile * _rootfile;

private:
  /** a lock to make the process reentrant */
  QMutex _lock;
};
}//end namespace cass

#endif
