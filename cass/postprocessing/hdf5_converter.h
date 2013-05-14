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
#include <tr1/tuple>

#include "postprocessing/backend.h"

namespace cass
{
/** converts histograms to csv like data and puts it into hdf5 container
 *
 * @PPList "1001": converts histograms to csv like data and puts it into hdf5
 *                 container
 *
 * Will go through all postprocessors and checks their write flag. If it is
 * true the histogram will be converted into a csv style data format.
 * - 2D histogram: a 3 column (x,y,z) by x*y rows table.
 * - 1D histogram: a 2 column (x,y) by x rows table.
 * - 0D histogram: a single value
 *
 * Additional to the data there are the usercomments and the overflow and
 * Underflow values of the histograms available.
 *
 * This postprocessor is optional conditional to only output the events the
 * user is interested in. When no condition is selected it won't write anything
 * during the run. Only the summary at the end of the analysis is beeing written.
 * It will create a group for each event it outputs.
 *
 * The groupname will be created from the event id, which is converted into
 * something human readable before. At the end of the analysis, when all files
 * are processed it will output all chosen histograms in a summary.
 *
 * When ther user has chosen the compress option to compress the 1d and 2d
 * data, the program will first check whether this option is enabled in the
 * hdf5 library. If it is not enabled then it will not compress the data.
 *
 * @cassttng PostProcessor/\%name\%/{Compress}\n
 *           Tells the converter to compress the 1d and 2d data using a zip
 *           algorithm. Default is false.
 * @cassttng PostProcessor/\%name\%/{Write2dAsMatrix}\n
 *           Instead of storing the 2d data in a table where the first two
 *           columns contain the coordinate and the third column contains the
 *           element value, the 2d data is stored in a matrix, that can only
 *           be addressed by their indizes. Default is false.
 * @cassttng PostProcessor/\%name\%/{WriteBeamlineData}\n
 *           When enabled this will write all beamline data available. This is
 *           not written in the summary only when the condition is true.
 *           Default is false.
 * @cassttng PostProcessor/\%name\%/{WriteEpicsData}\n
 *           When enabled this will write all epcis data available. This is
 *           not written in the summary only when the condition is true.
 *           Default is false.
 * @cassttng PostProcessor/\%name\%/{WriteEvrCodes}\n
 *           When enabled this will write all evr codes that are contained in
 *           the event. Those are only available at LCLS. This is not written
 *           in the summary only when the condition is true. Default is false.
 * @cassttng PostProcessor/\%name\%/{WriteSummary}\n
 *           With this flag one can enable or disable that a summary is
 *           written before cass quits. In the summary all PostProcessors are
 *           written, that the user did not disable writing for. Default is
 *           true.
 * @cassttng PostProcessor/\%name\%/{MaximumFilesize}\n
 *           The maximum size that the output file should have in MB.
 *           Default is 2048
 *
 * @author Lutz Foucar
 */
class pp1001 : public PostprocessorBackend
{
public:
  /** constructor */
  pp1001(PostProcessors &, const name_t &, const std::string& outfilename);

  /** process the event */
  virtual void process(const CASSEvent&);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

  /** dump all pp histograms to summary group just before quitting */
  virtual void aboutToQuit();

protected:
  /** the filename that the data will be written to */
  std::string _outfilename;

  /** the filehandle of the hdf5 file */
  int _filehandle;

  /** flag to tell whether to compress the data */
  bool _compress;

  /** default behaviour */
  bool _events_root_is_filehandle;

  /** flag to tell whether 2d hist should be stored as matrix */
  bool _store2dAsMatrix;

  /** flag to tell whether to dump all machine data */
  bool _dumpMachineData;

  /** flag to tell whether to write bld data */
  bool _dumpBLD;

  /** flag to tell wehther to write epics data */
  bool _dumpEpics;

  /** flag to tell whether to write EVR Codes */
  bool _dumpEVR;

  /** flag to tell whether to write a summary */
  bool _writeSummary;

  /** maximum filesize of the output hdf5 file in bytes */
  size_t _maxsize;

  /** lock for the scan group */
  QReadWriteLock _calibGroupLock;

  /** create a folder with the scan value */
  hid_t getGroupNameForCalibCycle(const cass::CASSEvent &);
};


/** saves a selected 2d histogram to hdf5
 *
 * @PPList "1002": saves a selected 2d histogram to hdf5
 *
 * it will just save one histogram in a file and then write the next into the
 * next hdf5 file. Inside the hdf5 it uses the same layout that the Chapman
 * crew is using to be able to read and process the hdf5 with crystfel
 *
 * @cassttng PostProcessor/\%name\%/PostProcessor/size \n
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
 * @author Lutz Foucar
 */
class pp1002 : public PostprocessorBackend
{
public:
  /** define a name, groupname, postprocessor tuple */
  typedef std::tr1::tuple<std::string,std::string,shared_pointer,uint32_t> entry_t;

  /** enum to make tuple entries more readable */
  enum entry_names {name=0,group,pp,options};

  /** constructor
   *
   * @param pp reference to the postprocessor manager
   * @param key the name of this PostProce
   * @param filename initial string of the output filename
   */
  pp1002(PostProcessors &pp, const name_t &key, const std::string& filename);

  /** process the event */
  virtual void process(const CASSEvent&);

  /** dump all pp histograms to summary group just before quitting */
  virtual void aboutToQuit();

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

protected:
  /** the filename that the data will be written to */
  std::string _basefilename;

  /** container with all pps that contain the histograms to dump to hdf5 */
  std::list<entry_t> _ppList;

  /** container for all pps that should be written when program quits */
  std::list<entry_t> _ppSummaryList;


private:
  /** a lock to make the process reentrant */
  QMutex _lock;

  /** compress flag */
  bool _compress;
};
}//end namespace cass
#endif
