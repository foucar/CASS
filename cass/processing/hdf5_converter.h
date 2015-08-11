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

namespace hdf5
{
class WriteEntry;
}//end namespace hdf5

/** saves a selected 2d histogram to hdf5
 *
 * @PPList "1002": saves a selected 2d histogram to hdf5
 *
 * it will just save one histogram in a file and then write the next into the
 * next hdf5 file. Inside the hdf5 it uses the same layout that the Chapman
 * crew is using to be able to read and process the hdf5 with crystfel
 *
 * @cassttng Processor/\%name\%/{CompressLevel} \n
 *           The compression level. Default is 2
 * @cassttng Processor/\%name\%/{FileBaseName} \n
 *           Base Name of the Files to be written. In case of writing mutiple
 *           Events to the same file an alpha counter will be added to the file
 *           name. In case of writing an event to a single file the event id
 *           Will be appended to the file name.
 * @cassttng Processor/\%name\%/{WriteMultipleEventsInOneFile} \n
 *           Flag to tell whether to write multiple events to the same file
 *           (true) or each event into a single file (false). Default is false.
 * @cassttng Processor/\%name\%/{MaximumNbrFilesPerDir} \n
 *           In case of single files per event, distribute the files over
 *           subdirectories where each subdir contains this amount of files.
 *           If -1 it will not distribute the files. Default is -1.
 * @cassttng Processor/\%name\%/{MaximumFileSize_GB} \n
 *           In case of multiple events per file, this is the maximum file size
 *           before the alpha counter of the filename will be increased and a
 *           the events will be written to the new file. Default is 200
 * @cassttng Processor/\%name\%/Processor/{size} \n
 *           How many Processors should be written to the h5 file.
 * @cassttng Processor/\%name\%/Processor/\%id\%/{Name} \n
 *           Name of the Processor that should be written into the h5 file.
 *           Default is "unknown"
 * @cassttng Processor/\%name\%/Processor/\%id\%/{GroupName} \n
 *           Name of the group in the h5 file into which the Processor
 *           should be written into. Default is "/"
 * @cassttng Processor/\%name\%/Processor/\%id\%/{ValName} \n
 *           Name that the data should have in the h5 file. Default is the
 *           name of the Processor.
 * @cassttng Processor/\%name\%/ProcessorSummary/{size} \n
 *           How many Processors should be written to the h5 file.
 * @cassttng Processor/\%name\%/ProcessorSummary/\%id\%/{Name} \n
 *           Name of the Processor that should be written into the h5 file.
 *           Default is "unknown"
 * @cassttng Processor/\%name\%/ProcessorSummary/\%id\%/{GroupName} \n
 *           Name of the group in the h5 file into which the Processor
 *           should be written into. Default is "/"
 * @cassttng Processor/\%name\%/ProcessorSummary/\%id\%/{ValName} \n
 *           Name that the data should have in the h5 file. Default is the
 *           name of the Processor.
 *
 * @todo enable that one can write into just one h5 file multiple events
 *
 * @author Lutz Foucar
 */
class pp1002 : public Processor
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
     * @param _pp the processor holding the data to be written
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

    /** processor holding the data to be written */
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
  virtual const result_t& result(const CASSEvent::id_t eventid=0);

  /** overwrite the release */
  virtual void releaseEvent(const CASSEvent &){}

protected:
  /** write the summary to a file that contains multiple events */
  void writeSummaryToMultipleEventsFile();

  /** function to write the events to a file that contains multiple events
   *
   * @param evt The event containg the data to write
   */
  void writeEventToMultipleEventsFile(const CASSEvent &evt);

  /** function to write the summary to a single file */
  void writeSummaryToSingleFile();

  /** function to write the events to a single file
   *
   * @param evt The event containg the data to write
   */
  void writeEventToSingleFile(const CASSEvent &evt);

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

  /** the entry writer */
  std::tr1::shared_ptr<hdf5::WriteEntry>_entryWriter;

  /** the maximum file size of the single file */
  int _maxFileSize;

  /** write summary to file */
  std::tr1::function<void(void)> _writeSummary;

  /** write event to file */
  std::tr1::function<void(const CASSEvent&)> _writeEvent;

private:
  /** a lock to make the process reentrant */
  QMutex _lock;
};
}//end namespace cass
#endif
