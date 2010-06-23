// Copyright (C) 2010 Lutz Foucar
// Copyright (C) 2010 Jochen Küpper

#include <QtCore/QMutex>
#include <QtCore/QStringList>

#include <cassert>
#include <algorithm>
#include <iostream>
#include <functional>

#include "acqiris_detectors.h"
#include "histogram.h"
#include "ccd.h"
#include "alignment.h"
#include "postprocessor.h"
#include "waveform.h"
#include "hdf5dump.h"
#include "operations.h"
#include "imaging.h"
#include "machine_data.h"
#include "backend.h"
#include "machine_data.h"
#include "id_list.h"
#include "cass_exceptions.h"
#include "operation_templates.hpp"
#include "cass_settings.h"

#ifdef SINGLEPARTICLE_HIT
#include "hitrate.h"
#endif




// ============define static members (do not touch)==============
cass::PostProcessors *cass::PostProcessors::_instance(0);
QMutex cass::PostProcessors::_mutex;

// create an instance of the singleton
cass::PostProcessors *cass::PostProcessors::instance(std::string outputfilename)
{
#ifdef VERBOSE
  static int n(0), create(0);
#endif
  VERBOSEOUT(std::cerr<<"PostProcessors::instance -- call "<<++n<<std::endl);
  QMutexLocker locker(&_mutex);
  if(0 == _instance)
  {
    VERBOSEOUT(std::cerr<<"PostProcessors::instance -- create "<<++create
               << std::endl);
    _instance = new PostProcessors(outputfilename);
  }
  return _instance;
}

// destroy the instance of the singleton
void cass::PostProcessors::destroy()
{
  QMutexLocker locker(&_mutex);
  delete _instance;
  _instance = 0;
}
//===============================================================



/** Internal helper function to convert QVariant to id_t */
static inline std::string QStringToStdString(QString str)
{
  return std::string(str.toStdString());
}





cass::PostProcessors::PostProcessors(std::string outputfilename)
  :_IdList(new IdList()),
  _outputfilename(outputfilename)

{
  VERBOSEOUT(std::cout<<"Postprocessors::constructor: output Filename: "
             <<_outputfilename
             <<std::endl);
}

void cass::PostProcessors::process(const CASSEvent& event)
{
  /**
   * @todo catch when postprocessor throws an exeption and delete the
   *       postprocessor from the active list.
   *       - create a remove list with all postprocessors that depend on this
   *       - go through that list and fill all pp that depend on the ones in
   *         the list recursivly.
   *       - remove all pp that made it on the removelist
   *       - this needs to be done in a locked way since more than one thread
   *         do this
   */
  for(postprocessors_t::iterator iter(_postprocessors.begin());
      iter != _postprocessors.end();
      ++iter)
    (*(iter->second))(event);
}

void cass::PostProcessors::aboutToQuit()
{
  for(postprocessors_t::iterator iter = _postprocessors.begin();
      iter != _postprocessors.end();
      ++iter)
    iter->second->aboutToQuit();
}

void cass::PostProcessors::loadSettings(size_t)
{
  using namespace std;
  VERBOSEOUT(cout << "Postprocessor::loadSettings" << endl);
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  QStringList list(settings.childGroups());
#ifdef VERBOSE
  cout << settings.fileName().toStdString() << " " ;
  cout << "Entries of "<< settings.group().toStdString() << ": ";
  foreach(QString str, list){
    cout<<str.toStdString() << ", ";
  }
  cout << endl;
#endif
  keyList_t active(list.size());
  transform(list.begin(), list.end(), active.begin(), QStringToStdString);
  cout <<"   Number of unique postprocessor activations: "<<active.size()
      << endl;
  //add a default true and false pp to container//
  active.push_back("DefaultTrueHist");
  if (_postprocessors.end() == _postprocessors.find("DefaultTrueHist"))
    _postprocessors["DefaultTrueHist"] = new pp10(*this, "DefaultTrueHist",true);
  active.push_back("DefaultFalseHist");
  if (_postprocessors.end() == _postprocessors.find("DefaultFalseHist"))
    _postprocessors["DefaultFalseHist"] = new pp10(*this, "DefaultFalseHist",false);
  setup(active);
  cout <<"   Active postprocessor(s): ";
  for(keyList_t::iterator iter = active.begin(); iter != active.end(); ++iter)
    cout << *iter << " ";
  cout<<endl;
}

void cass::PostProcessors::clear(const key_t &key)
{
  postprocessors_t::iterator it (_postprocessors.find(key));
  if (_postprocessors.end() != it)
    it->second->clearHistograms();
}

cass::PostprocessorBackend& cass::PostProcessors::getPostProcessor(const key_t &key)
{
  postprocessors_t::iterator it (_postprocessors.find(key));
  if (_postprocessors.end() == it)
    throw InvalidPostProcessorError(key);
  return *(it->second);
}

cass::IdList* cass::PostProcessors::getIdList()
{
  _IdList->clear();
  keyList_t active;
  for(postprocessors_t::iterator iter = _postprocessors.begin(); iter != _postprocessors.end(); ++iter)
#ifndef DEBUG
    if (!iter->second->hide())
#endif
      active.push_back(iter->first);
  _IdList->setList(active);
  return _IdList;
}

cass::PostProcessors::keyList_t cass::PostProcessors::find_dependant(const PostProcessors::key_t &key)
{
  using namespace std;
  //go through all pp and retrieve their dependencies//
  //make a list of all key that have a dependency on the requested key
  keyList_t dependandList;
  postprocessors_t::iterator iter = _postprocessors.begin();
  for(;iter != _postprocessors.end(); ++iter)
  {
    keyList_t dependencyList(iter->second->dependencies());
    if (find(dependencyList.begin(),dependencyList.end(),key) != dependencyList.end())
      dependandList.push_front(iter->first);
  }
  return dependandList;
}

void cass::PostProcessors::setup(keyList_t &active)
{
  using namespace std;
  /**
   * @todo when load settings throws exception then remove this pp and all pp
   *        that depend on it (like in process)
   */
  //  There can be the following cases:
  //  1) pp is not in container but key is on active list
  //  2) pp is not in conatiner and key is not in active list, but it's a
  //     dependency of another pp
  //  3) pp is in container and key is on active list
  //  5) pp is in container and key is not on active list

  VERBOSEOUT(cout << "Postprocessor::setup(): Clearing all postprocessor "
                  << "dependencies, since they will now be setup again."
                  << endl);
  postprocessors_t::iterator it (_postprocessors.begin());
  for(;it != _postprocessors.end();++it)
  {
    VERBOSEOUT(cout << "Postprocessor::setup(): Clearing dependencies of "
                    << it->second->key() << endl);
    it->second->clearDependencies();
  }

  VERBOSEOUT(cout << "Postprocessor::setup(): Going through the active list, "
                  << "and checking whether each PP is already in the container."
                  << endl);
  keyList_t::iterator iter(active.begin());
  while(iter != active.end())
  {
    VERBOSEOUT(cout << "Postprocessor::setup(): Checking if '" << *iter
                    << "' is in the PP container." << endl);
    if(_postprocessors.end() == _postprocessors.find(*iter))
    {
      VERBOSEOUT(cout << "Postprocessor::setup(): Did not find '" << *iter
                      << "in the PP container, so I am creating it." << endl);
      _postprocessors[*iter] = create(*iter);
    }
    else
    {
      VERBOSEOUT(cout << "Postprocessor::setup(): '" << *iter
                      << "' is in the container. Now loading Settings for it."
                      << endl);
      _postprocessors[*iter]->loadSettings(0);
    }
    VERBOSEOUT(cout << "Postprocessor::setup(): Done creating/loading '"
                    << *iter <<"'.  Now checking its dependencies..."
                    << endl);
    bool update(false);
    keyList_t deps(_postprocessors[*iter]->dependencies());
#ifdef VERBOSE
    if (deps.empty())
      cout << "Postprocessor::setup(): '" << *iter <<"' has no dependencies."
           << endl;
#endif
    for(keyList_t::iterator d=deps.begin(); d!=deps.end(); ++d)
    {
      VERBOSEOUT(cout << "Postprocessor::setup(): '" << *iter
                      << "' depends on '" << *d
                      <<"'. Checking whether it is in the container..."
                 <<endl);
      if(_postprocessors.end() == _postprocessors.find(*d))
      {
        //solves cases 2
        VERBOSEOUT(cout << "Postprocessor::setup(): '" << *d
                        << "' is not in the PP container. "
                        << "If it is already on active list, we will move it "
                        << "to the front of the list. "
                        << "If its not there, we will add it to front of list. "
                        << "This ensures that during the next cycle it will be "
                        << "created before '" << *iter << "'." << endl);
        if(active.end() != find(active.begin(), active.end(), *d))
          active.remove(*d);
        active.push_front(*d);
        update = true;
      }
      else
      {
        VERBOSEOUT(cout << "Postprocessor::setup(): Dependency PP '" << *d
                        << "' of '" << *iter
                        << "' is in the container. Check if its key is in the "
                        << "active list..." << endl);
        if(active.end() == find(active.begin(), active.end(), *d))
        {
          //solves case 5
          VERBOSEOUT(cout << "Postprocessor::setup(): Dependency PP '" << *d
                          << "' did not appear in the active list, so I will "
                          << "add it for '" << *iter << "'." << endl);
          active.push_front(*d);
          update = true;
        }
#ifdef VERBOSE
        else
        {
          VERBOSEOUT(cout << "Postprocessor::setup(): Dependency PP '" << *d
                          << "' is in active list." << endl);
        }
#endif
      }
    }
    // if we have updated active, start over again
    if(update)
    {
      // start over
      VERBOSEOUT(cout<< "Postprocessor::setup(): Starting over again." << endl);
      iter = active.begin();
      continue;
    }
    ++iter;
  }

  // some of the postprocessors are have been created and are in the container
  // but might not be on the active list anymore. Check which they are. Put
  // them on a delete list and then delete them.
  keyList_t eraseList;
  it = _postprocessors.begin();
  for(;it != _postprocessors.end(); ++it)
  {
    VERBOSEOUT(cout << "PostProcessor::setup(): Checking whether '" << it->first
                    << "' is still on active list." << endl);
    if(active.end() == find(active.begin(),active.end(),it->first))
    {
      VERBOSEOUT(cout << "PostProcessor::setup(): '"<< it->first
                      << "' is not on active list.  Adding it to the erase "
                      << "list." << endl);
      eraseList.push_back(it->first);
    }
#ifdef VERBOSE
    else
    {
      VERBOSEOUT(cout << "PostProcessor::setup(): '"<< it->first
                      << "' is still on active list."
                      << endl);
    }
#endif
  }
  // go through erase list and erase all postprocessors on that list
  iter = (eraseList.begin());
  for(;iter != eraseList.end(); ++iter)
  {
    VERBOSEOUT(cout << "PostProcessor::setup(): Erasing '"<< *iter
                    << "' from the postprocessor container." << endl);
    PostprocessorBackend* p = _postprocessors[*iter];
    delete p;
    _postprocessors.erase(*iter);
  }
}

cass::PostprocessorBackend * cass::PostProcessors::create(const key_t &key)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString::fromStdString(key));
  id_t ppid (static_cast<PostProcessors::id_t>(settings.value("ID",0).toUInt()));
  VERBOSEOUT(std::cout << "PostProcessor::create(): Creating PP '" << key
                       << "' with ID=" << ppid << std::endl);
  PostprocessorBackend * processor(0);
  switch(ppid)
  {
  case ConstantLess:
    processor = new pp1<less<float> >(*this, key, less<float>());
    break;
  case ConstantGreater:
    processor = new pp1<greater<float> >(*this, key, greater<float>());
    break;
  case ConstantEqual:
    processor = new pp1<equal_to<float> >(*this, key, equal_to<float>());
    break;
  case BooleanNOT:
    processor = new pp4(*this, key);
    break;
  case BooleanAND:
    processor = new pp5<logical_and<bool> >(*this, key, logical_and<bool>());
    break;
  case BooleanOR:
    processor = new pp5<logical_or<bool> >(*this, key, logical_or<bool>());
    break;
  case CompareForLess:
    processor = new pp7<less<float> >(*this, key, less<float>());
    break;
  case CompareForEqual:
    processor = new pp7<greater<float> >(*this, key, greater<float>());
    break;
  case CheckRange:
    processor = new pp9(*this, key);
    break;
  case ConstantTrue:
    processor = new pp10(*this, key, true);
    break;
  case ConstantFalse:
    processor = new pp10(*this, key, false);
    break;
  case SubtractHistograms:
    processor = new pp20<minus<float> >(*this, key, minus<float>());
    break;
  case AddHistograms:
    processor = new pp20<plus<float> >(*this, key, plus<float>());
    break;
  case DivideHistograms:
    processor = new pp20<divides<float> >(*this, key, divides<float>());
    break;
  case MultiplyHistograms:
    processor = new pp20<multiplies<float> >(*this, key, multiplies<float>());
    break;
  case SubtractConstant:
    processor = new pp23<minus<float> >(*this, key, minus<float>());
    break;
  case AddConstant:
    processor = new pp23<plus<float> >(*this, key, plus<float>());
    break;
  case MultiplyConstant:
    processor = new pp23<multiplies<float> >(*this, key, multiplies<float>());
    break;
  case DivideConstant:
    processor = new pp23<divides<float> >(*this, key, divides<float>());
    break;
  case Threshold:
    processor = new pp40(*this, key);
    break;
  case TwoDProjection:
    processor = new pp50(*this, key);
    break;
  case OneDIntergral:
    processor = new pp51(*this, key);
    break;
  case RadalProjection:
    processor = new pp52(*this, key);
    break;
  case AngularDistribution:
    processor = new pp53(*this, key);
    break;
  case R_Phi_Representation:
    processor = new pp54(*this, key);
    break;
  case ZeroDHistogramming:
    processor = new pp60(*this, key);
    break;
  case HistogramAveraging:
    processor = new pp61(*this, key);
    break;
  case HistogramSumming:
    processor = new pp62(*this, key);
    break;
  case TimeAverage:
    processor = new pp63(*this, key);
    break;
  case running1Dfrom0D:
    processor = new pp64(*this, key);
    break;
  case nbrOfFills:
    processor = new pp80(*this, key);
    break;
  case SingleCcdImage:
    processor = new pp100(*this, key);
    break;
  case SingleCcdImageIntegral:
    processor = new pp101(*this, key);
    break;
  case SingleCcdImageIntegralOverThres:
    processor = new pp102(*this, key);
    break;
  case AcqirisWaveform:
    processor = new pp110(*this,key);
    break;
  case BlData:
    processor = new pp120(*this,key);
    break;
  case EpicsData:
    processor = new pp130(*this,key);
    break;
  case CCDPhotonHitsSpectrum:
    processor = new pp140(*this,key);
    break;
  case CCDPhotonHitsImage:
    processor = new pp141(*this,key);
    break;
  case TofDetNbrSignals:
    processor = new pp150(*this, key);
    break;
  case TofDetAllSignals:
    processor = new pp151(*this, key);
    break;
  case TofDetMcpHeightVsFwhm:
    processor = new pp152(*this, key);
    break;
  case WireendNbrSignals:
    processor = new pp160(*this, key);
    break;
  case WireendHeightvsFwhm:
    processor = new pp161(*this, key);
    break;
  case AnodeTimesum:
    processor = new pp162(*this, key);
    break;
  case AnodeTimesumVsPos:
    processor = new pp163(*this, key);
    break;
  case DelaylineFirstGoodHit:
    processor = new pp164(*this, key);
    break;
  case DelaylineNbrReconstructedHits:
    processor = new pp165(*this, key);
    break;
  case DelaylineAllReconstuctedHits:
    processor = new pp166(*this, key);
    break;
  case Cos2Theta:
    processor = new pp200(*this,key);
    break;
  case AdvancedPhotonFinderDump:
    processor = new pp212(*this,key);
    break;
  case PIPICO:
    processor = new pp220(*this,key);
    break;
  case TestImage:
    processor = new pp240(*this,key);
    break;
#ifdef SINGLEPARTICLE_HIT
  case SingleParticleDetection:
    processor = new pp300(*this,key);
    break;
#endif
#ifdef HDF5
  case PnccdHDF5:
    processor = new pp1000(*this,key);
    break;
#endif
#ifdef CERNROOT
  case ROOTDump:
    processor = new pp2000(*this,key,_outputfilename);
    break;
#endif
  default:
    throw std::invalid_argument(QString("Postprocessortype %1 not available")
                                .arg(ppid).toStdString());
  }
  return processor;
}




// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
