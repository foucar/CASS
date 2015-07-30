// Copyright (C) 2010 Lutz Foucar

/**
 * @file root_converter.h file contains declaration of postprocessor 2000
 *
 * @author Lutz Foucar
 */

#ifndef _ROOT_CONVERTER_H_
#define _ROOT_CONVERTER_H_

#include <string>

#include "processor.h"

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
 * @cassttng Processor/\%name\%/{FileName} \n
 *           The name of the output root file. Default is "output.root"
 * @cassttng Processor/\%name\%/Processor/{size} \n
 *           How many Processors should be written to the root file.
 * @cassttng Processor/\%name\%/Processor/\%id\%/{Name} \n
 *           Name of the Processor that should be written into the root file.
 *           Default is "unknown"
 * @cassttng Processor/\%name\%/Processor/\%id\%/{GroupName} \n
 *           Name of the group in the h5 file into which the Processor
 *           should be written into. Default is "/". Note that the eventid will
 *           be prepended to the Name given here.
 * @cassttng Processor/\%name\%/Processor/\%id\%/{ValName} \n
 *           Name that the data should have in the root file. Default is the
 *           name of the Processor.
 * @cassttng Processor/\%name\%/ProcessorSummary/size \n
 *           How many Processors should be written to the root file.
 * @cassttng Processor/\%name\%/ProcessorSummary/\%id\%/{Name} \n
 *           Name of the Processor that should be written into the root file.
 *           Default is "unknown"
 * @cassttng Processor/\%name\%/ProcessorSummary/\%id\%/{GroupName} \n
 *           Name of the group in the root file into which the Processor
 *           should be written into. Default is "/"
 * @cassttng Processor/\%name\%/ProcessorSummary/\%id\%/{ValName} \n
 *           Name that the data should have in the root file. Default is the
 *           name of the Processor.
 *
 * @author Lutz Foucar
 */
class pp2000 : public Processor
{
public:
  /** struct bundleing info for writing an entry to file
   *
   * @author Lutz Foucar
   */
  struct entry_t
  {
    /** constructor
     *
     * @param _name the name of the value in the file
     * @param _groupname the group where the data will be written to
     * @param _pp the postprocessor holding the data to be written
     */
    entry_t(const std::string &_name,
            const std::string &_groupname,
            shared_pointer _pp)
      : name(_name), groupname(_groupname),pp(_pp)
    {}

    /** name of the value in the file */
    std::string name;

    /** group where the data will be written to */
    std::string groupname;

    /** postprocessor holding the data to be written */
    shared_pointer pp;
  };

  /** Construct postprocessor for converting histograms to root histograms */
  pp2000(const name_t&name);

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
  std::string _rootfilename;

  /** container with all pps that contain the histograms to dump to hdf5 */
  std::list<entry_t> _ppList;

  /** container for all pps that should be written when program quits */
  std::list<entry_t> _ppSummaryList;

  /** the root file */
  TFile * _rootfile;

private:
  /** a lock to make the process reentrant */
  QMutex _lock;
};
}//end namespace cass

#endif
