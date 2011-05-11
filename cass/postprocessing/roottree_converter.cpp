// Copyright (C) 2011 Lutz Foucar

/**
 * @file roottree_converter.cpp file contains definition of postprocessor 2001
 *
 * @author Lutz Foucar
 */

#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>

#include <TObject.h>
#include <TFile.h>
#include <TTree.h>
#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>

#include <QtCore/QDateTime>

#include "roottree_converter.h"
#include "histogram.h"
#include "cass_settings.h"
#include "cass_event.h"


using namespace cass;
using namespace std;


pp2001::pp2001(PostProcessors& pp, const cass::PostProcessors::key_t &key, std::string filename)
    : PostprocessorBackend(pp, key),
     _rootfile(TFile::Open(filename.c_str(),"RECREATE"))
{
  if (!_rootfile)
  {
    stringstream ss;
    ss <<"pp2001 ("<<key<<"): '"<<filename<< "' could not be opened! Maybe deleting the file helps.";
    throw invalid_argument(ss.str());
  }
  loadSettings(0);
}

void pp2001::loadSettings(size_t)
{
//  CASSSettings settings;
//  settings.beginGroup("PostProcessor");
//  settings.beginGroup(_key.c_str());
//  setupGeneral();
//  if (!setupCondition(false))
//    return;
//  _write = false;
//  _hide = true;
//  _result = new Histogram0DFloat();
//  createHistList(2*cass::NbrOfWorkers,true);
//  cout<<endl<<"PostProcessor '"<<_key
//      <<"' will write all cass histograms with the write flag set "
//      <<"to rootfile '"<<_rootfile->GetName()
//      <<"'. Condition is '"<<_condition->key()<<"'"
//      <<endl;
}

void pp2001::aboutToQuit()
{
//  VERBOSEOUT(cout << "pp2000::aboutToQuit() ("<<_key
//             <<"): Histograms will be written to: '"
//             <<_rootfile->GetName()<<"'"
//             <<endl);
//  /** create the summary directory and cd into it */
//  _rootfile->cd("/");
//  _rootfile->mkdir("Summary")->cd();
//  /** retrieve postprocessor container */
//  const PostProcessors::postprocessors_t& container(_pp.postprocessors());
//  /** go through all contents of the container */
//  PostProcessors::postprocessors_t::const_iterator it(container.begin());
//  for (;it != container.end(); ++it)
//  {
//    /** check if histograms of postprocessor should be written */
//    PostprocessorBackend &pp(*(it->second));
//    if (pp.write())
//    {
//      /** if so write it to the root file */
//      const HistogramBackend &cassbackend(pp.getHist(0));
//      const HistogramFloatBase &casshist(dynamic_cast<const HistogramFloatBase&>(cassbackend));
//      ROOT::copyHistToRootFile(casshist);
//    }
//  }
//  /** go back to original directory and save file */
//  _rootfile->cd("/");
//  _rootfile->SaveSelf();
//  _rootfile->Close();
}

void pp2001::process(const cass::CASSEvent &evt)
{
//  /** make sure that only one process is writing to root file */
//  _result->lock.lockForWrite();
//  /** create directory from eventId and cd into it */
//  _rootfile->cd("/");
//  string dirname(ROOT::eventIdToDirectoryName(evt.id()));
//  _rootfile->mkdir(dirname.c_str())->cd();
//  /** retrieve postprocessor container */
//  PostProcessors::postprocessors_t &ppc(_pp.postprocessors());
//  /** go through all contents of the container */
//  PostProcessors::postprocessors_t::iterator it (ppc.begin());
//  for (;it != ppc.end(); ++it)
//  {
//    PostprocessorBackend &pp (*(it->second));
//    if (pp.write())
//    {
//      /** if so write it to the root file */
//      const HistogramBackend &cassbackend(pp(evt));
//      const HistogramFloatBase &casshist(dynamic_cast<const HistogramFloatBase&>(cassbackend));
//      casshist.lock.lockForRead();
//      ROOT::copyHistToRootFile(casshist);
//      casshist.lock.unlock();
//    }
//  }
//  /** go back to original directory and save file */
//  _rootfile->cd("/");
//  _rootfile->SaveSelf();
//  /** unlock this */
//  _result->lock.unlock();
}

