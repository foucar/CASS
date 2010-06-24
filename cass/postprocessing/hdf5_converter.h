// Copyright (C) 2010 Lutz Foucar

/** @file hdf5_converter.h declaration of pp1001 (hdf5_converter)
 * @author Lutz Foucar
 */

#ifndef _HDF5_CONVERTER_H_
#define _HDF5_CONVERTER_H_

#include <string>

#include "postprocessing/backend.h"

namespace cass
{
  /** converts histograms to csv like data and puts it into hdf5 container
   *
   * Will go through all postprocessors and checks their write flag. If it is
   * true the histogram will be converted into a csv style data format.
   * - 2D histogram: a 3 column (x,y,z) by x*x rows table.
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
   * @author Lutz Foucar
   */
  class pp1001 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp1001(PostProcessors &, const PostProcessors::key_t &, const std::string& outfilename);

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
  };
}
#endif
