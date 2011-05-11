// Copyright (C) 2011 Lutz Foucar

/**
 * @file roottree_converter.h file contains declaration of postprocessor 2001
 *
 * @author Lutz Foucar
 */

#ifndef _ROOTTREE_CONVERTER_H_
#define _ROOTTREE_CONVERTER_H_

#include <string>

#include "postprocessing/backend.h"

class TTree;
class TFile;

namespace cass
{
  /** root file converter
   *
   * will write detectorhits of user specified delayline detectors to a root
   * tree.
   *
   * @author Lutz Foucar
   */
  class pp2001 : public PostprocessorBackend
  {
  public:
    /** Construct postprocessor for converting histograms to root histograms */
    pp2001(PostProcessors&, const PostProcessors::key_t&, std::string rootfilename);

    /** only a stub does nothing, but needs to be there because its pure virtual in base class */
    virtual void process(const CASSEvent&);

    /** load the settings of this pp */
    virtual void loadSettings(size_t);

    /** dump all histogram to a root file just before quitting */
    virtual void aboutToQuit();

  protected:
    /** the root file */
    TFile * _rootfile;

    /** the root tree to fill */
    TTree * _tree;
  };
}

#endif
