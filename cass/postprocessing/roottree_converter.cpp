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
#include "machine_device.h"
#include "rootfile_helper.h"
#include "log.h"


using namespace cass;
using namespace std;
using namespace ACQIRIS;
using namespace MachineData;

namespace cass
{
/** typedef for easier code */
typedef HelperAcqirisDetectors::helperinstancesmap_t::key_type detectorkey_t;

/** typedef for easier code */
typedef list<detectorkey_t> dlddetectors_t;

/** typedef for easier code */
typedef pair<DelaylineDetector::particles_t::key_type,
             HelperAcqirisDetectors::helperinstancesmap_t::key_type> particleskey_t;

/** typedef for easier code */
typedef list<particleskey_t> particleslist_t;

/** load the settings of all acqiris detectors defined in .ini file
 *
 * @author Lutz Foucar
 */
void loadAllDets()
{
  CASSSettings s;
  s.beginGroup("AcqirisDetectors");
  QStringList detectorNamesList(s.childGroups());
  QStringList::const_iterator detName(detectorNamesList.begin());
  for (; detName != detectorNamesList.end(); ++detName)
    HelperAcqirisDetectors::instance(detName->toStdString())->loadSettings();
}

/** check whether the key points to a delayline detector
 *
 * @return true when key points to a delayline detector
 * @param detkey key for the possible delayline detector
 *
 * @author Lutz Foucar
 */
bool isDLD(const detectorkey_t &detkey)
{
  HelperAcqirisDetectors::shared_pointer dethelp (HelperAcqirisDetectors::instance(detkey));
  return (dethelp->detectortype() == Delayline);
}

/** convert a qstring to the key in the list of detectors
 *
 * will convert the requested delayline detector described in the qstring to the
 * delayline detector key. Before returning the key, check whether the requested
 * detector is realy a delayline detector. Then load the settings of the detector
 *
 * @return the key to the requested detector
 * @param qstr the QString to convert to the key
 *
 * @author Lutz Foucar
 */
detectorkey_t qstring2detector(const QString & qstr)
{
  detectorkey_t dld(qstr.toStdString());
  if (!isDLD(dld))
  {
    stringstream ss;
    ss <<"pp2001::loadSettings(): Error detector '"<<dld
       <<"' is not a Delaylinedetector.";
    throw (invalid_argument(ss.str()));
  }
  return dld;
}

/** convert qstring to the particle key pair
 *
 * first try to find the detector that the particle belongs to. If found which
 * detector the particle belongs to then add this as second part of the returned
 * key.
 *
 * @return the key pair containing the particle name and the key to the detector
 *         that the particle belongs to
 * @param qstr the QString of the particle that is requested
 *
 * @author Lutz Foucar
 */
particleskey_t qstring2particle(const QString & qstr)
{
  particleskey_t particlekey(make_pair(qstr.toStdString(),""));
  const HelperAcqirisDetectors::helperinstancesmap_t & knownDetectors(HelperAcqirisDetectors::instances());
  HelperAcqirisDetectors::helperinstancesmap_t::const_iterator det(knownDetectors.begin());
  for (;det != knownDetectors.end(); ++det)
  {
    if (isDLD(det->first))
    {
      HelperAcqirisDetectors &dethelp(*HelperAcqirisDetectors::instance(det->first));
      const DetectorBackend &detback((dethelp.detector()));
      const DelaylineDetector &dld(dynamic_cast<const DelaylineDetector&>(detback));
      const DelaylineDetector::particles_t &particles(dld.particles());
      DelaylineDetector::particles_t::const_iterator particle(particles.find(particlekey.first));
      if (particle != particles.end())
      {
        particlekey.second = det->first;
        return particlekey;
      }
    }
  }
  stringstream ss;
  ss <<"pp2001::loadSettings(): Error particle '"<<particlekey.first
     <<"' is not part of any Delaylinedetector.";
  throw (invalid_argument(ss.str()));
  return particlekey;
}

/** copy map values to map
 *
 * will copy each key pair to the map of the tree structure.
 *
 * @param first iterator to the first element to copy
 * @param last const iterator to one past the last element to copy
 *
 * @author Lutz Foucar
 */
void copyMapValues(map<string,double>::const_iterator first,
                   map<string,double>::const_iterator last,
                   treehit_t& dest)
{
  while(first != last)
  {
    dest[(*first).first] = (*first).second;
    ++first;
  }
}
}//end namespace cass

pp2001::pp2001(PostProcessors& pp, const cass::PostProcessors::key_t &key, std::string filename)
    : PostprocessorBackend(pp, key),
     _rootfile(ROOTFileHelper::create(filename)),
     _tree(new TTree("CASSData","Selected preprocessed data from the CASSEvent")),
     _treestructure_ptr(&_treestructure),
     _machinestructure_ptr(&_machinestructure),
     _eventstatusstructure_ptr(&_eventstatusstructure),
     _ppstructure_ptr(&_ppstructure)
{
  if (!_rootfile)
    throw invalid_argument("pp2001 (" + key + "): '" + filename +
                           "' could not be opened! Maybe deleting the file helps.");
  loadSettings(0);
}

void pp2001::loadSettings(size_t)
{
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString::fromStdString(_key));
  setupGeneral();
  QStringList pps(settings.value("PostProcessors").toStringList());
  QStringList::const_iterator ppname(pps.begin());
  _pps.clear();
  for (; ppname != pps.constEnd(); ++ppname)
  {
    PostprocessorBackend *pp(&(_pp.getPostProcessor((*ppname).toStdString())));
    if (!pp)
      return;
    if (pp->getHist(0).dimension() != 0)
      throw invalid_argument("pp2001 (" + _key + "): PostProcessor '" + pp->key() +
                             "' does not handle a 0d histogram.");
    _pps.push_back(pp);
  }
  if (!setupCondition())
    return;
  loadAllDets();
  QStringList detectors(settings.value("Detectors").toStringList());
  _detectors.resize(detectors.size());
  transform(detectors.begin(),detectors.end(),_detectors.begin(),qstring2detector);
  QStringList particles(settings.value("Particles").toStringList());
  _particles.resize(particles.size());
  transform(particles.begin(),particles.end(),_particles.begin(),qstring2particle);
  if (_tree->FindBranch("DLDetectorData") == 0)
    if (!_detectors.empty() || !_particles.empty())
      _tree->Branch("DLDetectorData","map<string,vector<map<string,double> > >",&_treestructure_ptr);
  if (_tree->FindBranch("EvendId") == 0)
    _tree->Branch("EvendId",&_eventid,"id/l");
  if (settings.value("MachineData",false).toBool())
    if (_tree->FindBranch("MachineData") == 0)
    _tree->Branch("MachineData","map<string,double>",&_machinestructure_ptr);
  if (settings.value("EventStatus",false).toBool())
    if (_tree->FindBranch("EventStatus") == 0)
      _tree->Branch("EventStatus","vector<bool>",&_eventstatusstructure_ptr);
  if (!_pps.empty())
    if (_tree->FindBranch("PostProcessors") == 0)
      _tree->Branch("PostProcessors","map<string,double>",&_ppstructure_ptr);

  _write = false;
  _hide = true;
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers,true);
  cout<<endl<<"PostProcessor '"<<_key
      <<"' will write the hits of detectors: ";
  dlddetectors_t::const_iterator detectorsIt(_detectors.begin());
  for (;detectorsIt!=_detectors.end();++detectorsIt)
    cout <<"'"<<(*detectorsIt)<<"', ";
  cout<<" and particles: ";
  particleslist_t::const_iterator particle(_particles.begin());
  for (;particle != _particles.end();++particle)
    cout <<"'"<<particle->first<<"("<<particle->second<<")', ";
  cout<<" to rootfile '"<<_rootfile->GetName()
      <<"'. Condition is '"<<_condition->key()<<"'"
      <<endl;
}

void pp2001::aboutToQuit()
{
  _tree->Write();
  ROOTFileHelper::close(_rootfile);
}

void pp2001::process(const cass::CASSEvent &evt)
{
  _result->lock.lockForWrite();
  _eventid = evt.id();
  dlddetectors_t::const_iterator detector(_detectors.begin());
  dlddetectors_t::const_iterator detectorEnd(_detectors.end());
  for (;detector != detectorEnd; ++detector)
  {
    DetectorBackend &rawdet(
          HelperAcqirisDetectors::instance(*detector)->detector(evt));
    DelaylineDetector &det(dynamic_cast<DelaylineDetector&>(rawdet));

    treedetector_t &treedet(_treestructure[*detector]);
    treedet.clear();
    detectorHits_t::iterator hit(det.hits().begin());
    for (; hit != det.hits().end(); ++hit)
    {
      treehit_t treehit;
      detectorHit_t &hitvalues(*hit);
      copyMapValues(hitvalues.begin(),hitvalues.end(),treehit);
      treedet.push_back(treehit);
    }
  }
  particleslist_t::const_iterator particle(_particles.begin());
  particleslist_t::const_iterator particleEnd(_particles.end());
  for (;particle !=particleEnd;++particle)
  {
    DetectorBackend &rawdet(
          HelperAcqirisDetectors::instance(particle->second)->detector(evt));
    DelaylineDetector &det(dynamic_cast<DelaylineDetector&>(rawdet));

    treedetector_t &treeparticle(_treestructure[particle->first]);
    treeparticle.clear();
    particleHits_t & hits(det.particles()[particle->first].hits());
    particleHits_t::iterator hit(hits.begin());
    particleHits_t::iterator hitEnd(hits.end());
    for (; hit != hitEnd; ++hit)
    {
      treehit_t treehit;
      particleHit_t &hitvalues(*hit);
      copyMapValues(hitvalues.begin(),hitvalues.end(),treehit);
      treeparticle.push_back(treehit);
    }
  }
  const MachineDataDevice &machinedata
      (*dynamic_cast<const MachineDataDevice*>(evt.devices().find(cass::CASSEvent::MachineData)->second));
  copyMapValues(machinedata.BeamlineData().begin(), machinedata.BeamlineData().end(), _machinestructure);
  copyMapValues(machinedata.EpicsData().begin(), machinedata.EpicsData().end(), _machinestructure);
  _eventstatusstructure.resize(machinedata.EvrData().size());
  copy(machinedata.EvrData().begin(),machinedata.EvrData().end(),_eventstatusstructure.begin());

  /** copy the values of each 0d PostProcessor into the postprocessor structure */
  std::list<PostprocessorBackend*>::const_iterator pp(_pps.begin());
  std::list<PostprocessorBackend*>::const_iterator ppEnd(_pps.end());
  for (;pp != ppEnd;++pp)
  {
    PostprocessorBackend *postprocessor(*pp);
    const Histogram0DFloat &val
        (dynamic_cast<const Histogram0DFloat&>((*postprocessor)(evt)));
    val.lock.lockForRead();
    float value(val.getValue());
    val.lock.unlock();
    _ppstructure[postprocessor->key()] = value;
  }
  _tree->Fill();
  _result->lock.unlock();
}

