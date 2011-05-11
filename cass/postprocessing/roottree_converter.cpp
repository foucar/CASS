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
#include <QtCore/QString>
#include <QtCore/QStringList>

#include "roottree_converter.h"
#include "histogram.h"
#include "cass_settings.h"
#include "cass_event.h"
#include "convenience_functions.h"
#include "delayline_detector.h"
#include "tree_structure.h"


using namespace cass;
using namespace std;
using namespace ACQIRIS;

namespace cass
{
/** typedef for easier code */
typedef HelperAcqirisDetectors::helperinstancesmap_t::key_type detectorkey_t;

/** typedef for easier code */
typedef list<detectorkey_t> dlddetectors_t;

/** convert a qstring to the key in the list of detectors
 *
 * will convert the requested delayline detector described in the qstring to the
 * delayline detector key. Before returning the key, check whether the requested
 * detector is realy a delayline detector.
 *
 * @return the key to the requested detector
 * @param qstr the QString to convert to the key
 *
 * @author Lutz Foucar
 */
detectorkey_t qstring2detector(const QString & qstr)
{
  detectorkey_t dld(qstr.toStdString());
  HelperAcqirisDetectors *dethelp (HelperAcqirisDetectors::instance(dld));
  if (dethelp->detectortype() != Delayline)
  {
    stringstream ss;
    ss <<"pp2001::loadSettings(): Error detector '"<<dld
       <<"' is not a Delaylinedetector.";
    throw (invalid_argument(ss.str()));
  }
  dethelp->loadSettings();
  return dld;
}
}

pp2001::pp2001(PostProcessors& pp, const cass::PostProcessors::key_t &key, std::string filename)
    : PostprocessorBackend(pp, key),
     _rootfile(TFile::Open(filename.c_str(),"RECREATE")),
     _tree(new TTree("DLDData","Data from the Delayline Detectors")),
     _treestructure_ptr(&_treestructure)
{
  if (!_rootfile)
  {
    stringstream ss;
    ss <<"pp2001 ("<<key<<"): '"<<filename<< "' could not be opened! Maybe deleting the file helps.";
    throw invalid_argument(ss.str());
  }
  _tree->Branch("DLDetectors","map<string,vector<map<string,double> > >",&_treestructure_ptr);
  loadSettings(0);
}

void pp2001::loadSettings(size_t)
{
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  setupGeneral();
  if (!setupCondition(false))
    return;
  QStringList detectors(settings.value("Detectors").toStringList());
  _detectors.resize(detectors.size());
  transform(detectors.begin(),detectors.end(),_detectors.begin(),qstring2detector);
  _write = false;
  _hide = true;
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers,true);
  cout<<endl<<"PostProcessor '"<<_key
      <<"' will write the hits of detectors: ";
  dlddetectors_t::const_iterator detectorsIt(_detectors.begin());
  for (;detectorsIt!=_detectors.end();++detectorsIt)
    cout <<"'"<<(*detectorsIt)<<"', ";
  cout<<" to rootfile '"<<_rootfile->GetName()
      <<"'. Condition is '"<<_condition->key()<<"'"
      <<endl;
}

void pp2001::aboutToQuit()
{
  _tree->Write();
  _rootfile->SaveSelf();
  _rootfile->Close();
  delete _tree;
}

void pp2001::process(const cass::CASSEvent &evt)
{
  _result->lock.lockForWrite();
  dlddetectors_t::const_iterator detector(_detectors.begin());
  for (;detector != _detectors.end(); ++detector)
  {
    DetectorBackend *rawdet
        (HelperAcqirisDetectors::instance(*detector)->detector(evt));
    DelaylineDetector &det (*dynamic_cast<DelaylineDetector*>(rawdet));

    treedetector_t &treedet = _treestructure[*detector];
    treedet.clear();
    detectorHits_t::iterator hit(det.hits().begin());
    for (; hit != det.hits().end(); ++hit)
    {
      treehit_t treehit;
      treehit["x"] = (*hit)["x"];
      treehit["y"] = (*hit)["y"];
      treehit["t"] = (*hit)["t"];
      treedet.push_back(treehit);
    }
  }
  _tree->Fill();
  _result->lock.unlock();
}

