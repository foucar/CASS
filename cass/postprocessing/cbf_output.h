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
 * details
 *
 * @cassttng PostProcessor/\%name\%/{DarkName} \n
 *           Postprocessor name offsetmap to write to cbf when program quits.
 * @cassttng PostProcessor/\%name\%/{HistName} \n
 *           Postprocessor name containing the histogram that write to cbf.
 *
 * @author Stephan Kassemeyer
 * @author Lutz Foucar
 */
class pp1500 : public PostprocessorBackend
{
public:
  /** constructor
   *
   * @param pp reference to the postprocessor manager
   * @param key the name of this PostProce
   * @param filename initial string of the output filename
   */
  pp1500(PostProcessors &pp, const PostProcessors::key_t &key, const std::string& filename);

  /** process the event */
  virtual void process(const CASSEvent&);

  /** dump dark to cbf just before quitting */
  virtual void aboutToQuit();

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

protected:
  /** the filename that the data will be written to */
  std::string _basefilename;

  /** pp containing histogram to dump to cbf */
  PostprocessorBackend *_pHist;

  /** pp containing offset histogram to dump to cbf */
  PostprocessorBackend *_darkHist;

private:
  /** a lock to make the process reentrant */
  QMutex _lock;
};
} //end namespace cass
#endif
