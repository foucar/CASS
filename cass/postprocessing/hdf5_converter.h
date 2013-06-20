// Copyright (C) 2010 Lutz Foucar

/**
 * @file hdf5_converter.h declaration of pp1001 (hdf5_converter)
 *
 * @author Lutz Foucar
 */

#ifndef _HDF5_CONVERTER_H_
#define _HDF5_CONVERTER_H_

#include <string>
#include <list>
#include <hdf5.h>

#include "processor.h"

namespace cass
{
/** saves a selected 2d histogram to hdf5
 *
 * @PPList "1002": saves a selected 2d histogram to hdf5
 *
 * it will just save one histogram in a file and then write the next into the
 * next hdf5 file. Inside the hdf5 it uses the same layout that the Chapman
 * crew is using to be able to read and process the hdf5 with crystfel
 *
 * @cassttng PostProcessor/\%name\%/{compress} \n
 *           Default true
 * @cassttng PostProcessor/\%name\%/{FileBaseName} \n
 *           Default name given by program parameter
 * @cassttng PostProcessor/\%name\%/{MaximumNbrFilesPerDir} \n
 *           Distribute the files over subdirectories where each subdir contains
 *           this amount of files. If -1 it will not distribute the files.
 *           Default is -1.
 * @cassttng PostProcessor/\%name\%/PostProcessor/{size} \n
 *           How many PostProcessors should be written to the h5 file.
 * @cassttng PostProcessor/\%name\%/PostProcessor/\%id\%/{Name} \n
 *           Name of the PostProcessor that should be written into the h5 file.
 *           Default is "unknown"
 * @cassttng PostProcessor/\%name\%/PostProcessor/\%id\%/{GroupName} \n
 *           Name of the group in the h5 file into which the PostProcessor
 *           should be written into. Default is "/"
 * @cassttng PostProcessor/\%name\%/PostProcessorSummary/size \n
 *           How many PostProcessors should be written to the h5 file.
 * @cassttng PostProcessor/\%name\%/PostProcessorSummary/\%id\%/{Name} \n
 *           Name of the PostProcessor that should be written into the h5 file.
 *           Default is "unknown"
 * @cassttng PostProcessor/\%name\%/PostProcessorSummary/\%id\%/{GroupName} \n
 *           Name of the group in the h5 file into which the PostProcessor
 *           should be written into. Default is "/"
 * @cassttng PostProcessor/\%name\%/PostProcessorSummary/\%id\%/{ValName} \n
 *           Name that the data should have in the h5 file. Default is the
 *           name of the PostProcessor.
 *
 * @todo enable that one can write into just one h5 file multiple events
 * @todo enable that one can write the additional info of a histogram to h5
 *
 * @author Lutz Foucar
 */
class pp1002 : public PostProcessor
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
     * @param _options the options for writing
     * @param _pp the postprocessor holding the data to be written
     */
    entry_t(const std::string &_name,
            const std::string &_groupname,
            const uint32_t _options,
            shared_pointer _pp)
      : name(_name), groupname(_groupname), options(_options),pp(_pp)
    {}

    /** name of the value in the file */
    std::string name;

    /** group where the data will be written to */
    std::string groupname;

    /** options for writing */
    uint32_t options;

    /** postprocessor holding the data to be written */
    shared_pointer pp;
  };

  /** constructor */
  pp1002(const name_t &);

  /** process the event */
  virtual void processEvent(const CASSEvent&);

  /** dump all pp histograms to summary group just before quitting */
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

  /** container with all pps that contain the histograms to dump to hdf5 */
  std::list<entry_t> _ppList;

  /** container for all pps that should be written when program quits */
  std::list<entry_t> _ppSummaryList;

  /** the number of files in each subdir */
  int _maxFilePerSubDir;

  /** counter to count how many files have been written */
  int _filecounter;

private:
  /** a lock to make the process reentrant */
  QMutex _lock;
};
}//end namespace cass
#endif
