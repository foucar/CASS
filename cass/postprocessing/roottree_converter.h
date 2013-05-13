// Copyright (C) 2011 Lutz Foucar

/**
 * @file roottree_converter.h file contains declaration of postprocessor 2001
 *
 * @author Lutz Foucar
 */

#ifndef _ROOTTREE_CONVERTER_H_
#define _ROOTTREE_CONVERTER_H_

#include <string>
#include <list>
#include <map>

#include "postprocessing/backend.h"
#include "acqiris_detectors_helper.h"
#include "tree_structure.h"
#include "delayline_detector.h"


class TTree;
class TFile;

namespace cass
{
/** root file converter
 *
 * will write detectorhits of user specified delayline detectors to a root
 * tree.
 *
 * @cassttng PostProcessor/\%name\%/{Detectors}\n
 *           comma separated list of Delaylinedetectors who's hits should be
 *           added to the tree.
 * @cassttng PostProcessor/\%name\%/{Particles}\n
 *           comma separated list of Particles who's hits should be
 *           added to the tree. The corrosponding detector does not need to be
 *           added above since this postprocessor will find out what detector
 *           a particle belongs to automatically.
 * @cassttng PostProcessor/\%name\%/{PostProcessors}\n
 *           comma separated list of 0d PostProcessors who's values should be
 *           added to the tree.
 * @cassttng PostProcessor/\%name\%/{MachineData}\n
 *           Flag whether to add the Beamline and Epics data to the tree.
 *           Default is false.
 * @cassttng PostProcessor/\%name\%/{EventStatus}\n
 *           Flag whether to add the EventStatus array data to the tree.
 *           Default is false.
 *
 * @author Lutz Foucar
 */
class pp2001 : public PostprocessorBackend
{
public:
  /** Construct postprocessor for converting histograms to root histograms */
  pp2001(PostProcessors&, const name_t&, std::string rootfilename);

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

  /** list of detectors who's hits should be filled into the tree */
  std::list<ACQIRIS::HelperAcqirisDetectors::helperinstancesmap_t::key_type> _detectors;

  /** list of particles whos hits should be added to the tree */
  std::list<std::pair<ACQIRIS::DelaylineDetector::particles_t::key_type,
  ACQIRIS::HelperAcqirisDetectors::helperinstancesmap_t::key_type> > _particles;

  /** structure that should be written to tree */
  treestructure_t _treestructure;

  /** pointer to the above structure (needed by the tree) */
  treestructure_t *_treestructure_ptr;

  /** copy of the event id */
  uint64_t _eventid;

  /** machine data structure */
  machinestructure_t _machinestructure;

  /** pointer to the machine structure defined above */
  machinestructure_t *_machinestructure_ptr;

  /** event status structure */
  eventStatus_t _eventstatusstructure;

  /** pointer to the event status structure */
  eventStatus_t *_eventstatusstructure_ptr;

  /** container for all 0d PostProcessors that should be added to the tree */
  std::list<PostprocessorBackend*> _pps;

  /** 0d postprocessor structure */
  ppstructure_t _ppstructure;

  /** pointer to the 0d postprocessor structure */
  ppstructure_t *_ppstructure_ptr;
};
} // end namespace cass

#endif
