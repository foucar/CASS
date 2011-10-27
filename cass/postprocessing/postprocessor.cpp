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
#include "rankfilter.h"
#include "imaging.h"
#include "machine_data.h"
#include "backend.h"
#include "machine_data.h"
#include "id_list.h"
#include "cass_exceptions.h"
#include "operation_templates.hpp"
#include "cass_settings.h"
#include "hdf5_converter.h"
#include "coltrims_analysis.h"

#ifdef SINGLEPARTICLE_HIT
#include "hitrate.h"
#endif

#ifdef CERNROOT
#include "root_converter.h"
#include "roottree_converter.h"
#endif

//#define DEBUG_FIND_DEPENDANT

// ============define static members (do not touch)==============
cass::PostProcessors *cass::PostProcessors::_instance(0);
QMutex cass::PostProcessors::_mutex;

// create an instance of the singleton
cass::PostProcessors *cass::PostProcessors::instance(std::string outputfilename)
{
#ifdef VERBOSE
  static int /*n(0),*/ create(0);
#endif
//  VERBOSEOUT(std::cerr<<"PostProcessors::instance -- call "<<++n<<std::endl);
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
  using namespace std;
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
  postprocessors_t::iterator iter(_postprocessors.begin());
  for(;iter != _postprocessors.end(); ++iter)
  {
//    cout <<event.id()<<" running '" << iter->first<<"'"<<endl;
    (*(iter->second))(event);
  }
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
  QWriteLocker locker(&lock);
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
  _active.clear();
  _active.resize(list.size());
  transform(list.begin(), list.end(), _active.begin(), QStringToStdString);
  cout <<"   Number of unique postprocessor activations: "<<_active.size()
      << endl;
  //add a default true and false pp to container//
  _active.push_back("DefaultTrueHist");
  if (_postprocessors.end() == _postprocessors.find("DefaultTrueHist"))
    _postprocessors["DefaultTrueHist"] = new pp10(*this, "DefaultTrueHist",true);
  _active.push_back("DefaultFalseHist");
  if (_postprocessors.end() == _postprocessors.find("DefaultFalseHist"))
    _postprocessors["DefaultFalseHist"] = new pp10(*this, "DefaultFalseHist",false);
  setup(_active);
  cout <<"   Active postprocessor(s): "<<endl;
  for(keyList_t::iterator iter (_active.begin()); iter != _active.end(); ++iter)
    cout << *iter << " "<<endl;
  cout<<endl;
}

void cass::PostProcessors::clear(const key_t &key)
{
  postprocessors_t::iterator it (_postprocessors.find(key));
  if (_postprocessors.end() != it)
    it->second->clearHistograms();
}

void cass::PostProcessors::receiveCommand(const key_t &key, std::string command)
{
  postprocessors_t::iterator it (_postprocessors.find(key));
  if (_postprocessors.end() != it)
    it->second->processCommand(command);
}

void cass::PostProcessors::saveSettings()
{
  // call saveSettings for each postprocessor.
  for (postprocessors_t::iterator it=_postprocessors.begin(); it!=_postprocessors.end(); ++it)
    it->second->saveSettings(0); 
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
  //make a list of all key that have a dependency on the requested key//
  //iteratively go through and find all depends depandats
  keyList_t dependandList;
  dependandList.push_front(key);
  postprocessors_t::const_iterator iter = _postprocessors.begin();
  while(iter != _postprocessors.end())
  {
    bool update(false);
    keyList_t dependencyList(iter->second->dependencies());
    for (keyList_t::const_iterator it(dependandList.begin()); it!=dependandList.end();++it)
    {
#ifdef DEBUG_FIND_DEPENDANT
      cout<<"PostProcessors::find_dependant: check if '"<<*it
          <<"' is on dependency list of '"<<iter->first<<"'"
          <<endl;
#endif
      if (find(dependencyList.begin(),dependencyList.end(),*it) != dependencyList.end())
      {
#ifdef DEBUG_FIND_DEPENDANT
      cout<<"PostProcessors::find_dependant: '"<<*it
          <<"' is on dependency list of '"<<iter->first
          <<"' Now check whether '"<<iter->first
          <<"' is already on the dependand list."
          <<endl;
#endif
        if(find(dependandList.begin(),dependandList.end(),iter->first) == dependandList.end())
        {
#ifdef DEBUG_FIND_DEPENDANT
          cout<<"PostProcessors::find_dependant: '"<<iter->first
              <<"' is not dependant list. Put it there and start over."
              <<endl;
#endif
          dependandList.push_front(iter->first);
          update=true;
        }
#ifdef DEBUG_FIND_DEPENDANT
        else
        {
          cout<<"PostProcessors::find_dependant: '"<<iter->first
              <<"' is on dependant list. Nothing to do."
              <<endl;
        }
#endif
      }
#ifdef DEBUG_FIND_DEPENDANT
      else
      {
        cout<<"PostProcessors::find_dependant: '"<<*it
            <<"' is not on dependency list of '"<<iter->first<<"'"
            <<endl;
      }
#endif
    }
    if (update)
    {
#ifdef DEBUG_FIND_DEPENDANT
      cout<<"PostProcessors::find_dependant: The dependant list was modified, starting over"
          <<endl;
#endif
      iter = _postprocessors.begin();
      continue;
    }
    ++iter;
  }
#ifdef DEBUG_FIND_DEPENDANT
    cout<<"PostProcessors::find_dependant: now the following keys are on the dependand list: ";
    for (keyList_t::const_iterator it(dependandList.begin()); it!=dependandList.end();++it)
      cout<<*it<<", ";
    cout<<" of '"<<key
        <<"'. Now remove the initial key '"<<key<<"'"<<endl;
#endif
  dependandList.remove(key);
#ifdef DEBUG_FIND_DEPENDANT
    cout<<"PostProcessors::find_dependant: Here is the final dependant list of '"<<key
        <<"': ";
    for (keyList_t::const_iterator it(dependandList.begin()); it!=dependandList.end();++it)
      cout<<*it<<", ";
    cout<<endl;
#endif
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
  //  6) pp is in container and key is on active list and dependency is also on
  //     active list but dependency is after pp on active list
  //

  VERBOSEOUT(cout << "Postprocessor::setup(): Clearing all postprocessor "
                  << "dependencies, since they will now be setup again."
                  << endl);
  postprocessors_t::iterator it (_postprocessors.begin());
  for(;it != _postprocessors.end();++it)
  {
    VERBOSEOUT(cout << "Postprocessor::setup(): "
                    << "Clearing dependencies of '"<< it->second->key()<<"'"
                    << endl);
    it->second->clearDependencies();
  }

  VERBOSEOUT(cout << "Postprocessor::setup(): Going through the active list, "
                  << "and checking whether each PP is already in the container."
                  << endl);
  keyList_t::iterator iter(active.begin());
  while(iter != active.end())
  {
#ifdef VERBOSE
    cout <<"Postprocessor::setup(): Entries in the active list in the order we call them:"<<endl;
    for (keyList_t::const_iterator actIt(active.begin()) ; actIt != active.end(); ++actIt)
      cout<<*actIt<<endl;
#endif
    VERBOSEOUT(cout << "Postprocessor::setup(): Checking if '" << *iter
                    << "' is in the PP container." << endl);
    if(_postprocessors.end() == _postprocessors.find(*iter))
    {
      VERBOSEOUT(cout << "Postprocessor::setup(): Did not find '" << *iter
                      << "' in the PP container, so I am creating it." << endl);
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
                      <<"'. Checking whether dependency is in the container..."
                 <<endl);
      if(_postprocessors.end() == _postprocessors.find(*d))
      {
        //solves cases 2
        VERBOSEOUT(cout << "Postprocessor::setup(): Dependency '" << *d
                        << "' of '"<<*iter
                        << "' is not in the PP container. "
                        << "This can have 2 reasons. First it is not at all on "
                        << "the active list. Second it is on the active list, "
                        << "but appears after '"<<*iter
                        << "'. Check if it is on active list."
                        << endl);
        if(active.end() != find(active.begin(), active.end(), *d))
        {
          VERBOSEOUT(cout << "Postprocessor::setup(): Dependency '" << *d
                          << "' is on active list that means it appears after '"<<*iter
                          << "'. Remove it from active list"
                          << endl);
          active.remove(*d);
        }
#ifdef VERBOSE
        else
        {
          VERBOSEOUT(cout << "Postprocessor::setup(): Dependency '" << *d
                          << "' of '"<<*iter
                          <<"' is not on active list."
                          << endl);
        }
#endif
        VERBOSEOUT(cout << "Postprocessor::setup(): Insert dependency '" << *d
                        << "' of '"<<*iter
                        << "' just before '"<<*iter
                        << "' in the active list."
                        << endl);
        active.insert(iter,*d);
        update = true;
      }
      else
      {
        VERBOSEOUT(cout << "Postprocessor::setup(): Dependency PP '" << *d
                        << "' of '" << *iter
                        << "' is in the container. Check if it is also on the "
                        << "active list..."
                        << endl);
        if(active.end() == find(active.begin(), active.end(), *d))
        {
          //solves case 5
          VERBOSEOUT(cout << "Postprocessor::setup(): Dependency PP '" << *d
                          << "' of '" << *iter
                          << "' did not appear in the active list,"
                          << " so insert it just before '"<<*iter
                          << "' in the active list"
                          << endl);
          active.insert(iter,*d);
          update = true;
        }
        else
        {
          VERBOSEOUT(cout << "Postprocessor::setup(): Dependency PP '" << *d
                          << "' is in active list. Now check whether "
                          << "it appears before '"<<*iter<<"'"
                          << endl);
          if (active.end() == find(iter,active.end(),*d))
          {
            VERBOSEOUT(cout << "Postprocessor::setup(): Dependency PP '" << *d
                            << "' appears before '"<<*iter
                            << "' in active list so everything is fine."
                            << endl);
          }
          else
          {
            VERBOSEOUT(cout << "Postprocessor::setup(): Dependency PP '" << *d
                            << "' appears after '"<<*iter
                            << "' in active list, so move it just before '"<<*iter
                            << "' in the active list."
                            << endl);
            active.remove(*d);
            active.insert(iter,*d);
            update = true;
          }
        }
      }
    }
    if(update)
    {
      VERBOSEOUT(cout<< "Postprocessor::setup(): The active list has been "
                     << "modified. Move the iterator back the amount of "
                     << "dependencies that were added in the previous step and "
                     << "start again."
                     << endl);
      advance(iter,negate<int>()(deps.size()));
      continue;
    }
    ++iter;
  }

  // some of the postprocessors exist in the container but might not be on the
  // active list anymore. This means that the user does not want them anymore.
  // Check which they are, put them on a delete list and then delete them.
  keyList_t eraseList;
  it = _postprocessors.begin();
  for(;it != _postprocessors.end(); ++it)
  {
    VERBOSEOUT(cout << "PostProcessor::setup(): Checking whether '" << it->first
                    << "' is still on active list."
                    << endl);
    if(active.end() == find(active.begin(),active.end(),it->first))
    {
      VERBOSEOUT(cout << "PostProcessor::setup(): '"<< it->first
                      << "' is not on active list. Adding it to the erase list."
                      << endl);
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
  iter = (eraseList.begin());
  for(;iter != eraseList.end(); ++iter)
  {
    VERBOSEOUT(cout << "PostProcessor::setup(): Erasing '"<< *iter
                    << "' from the postprocessor container."
                    << endl);
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
  VERBOSEOUT(cout << "PostProcessor::create(): Creating PP '" << key
                  << "' with ID=" << ppid
                  << endl);
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
  case Subtract0DConstant:
    processor = new pp30<minus<float> >(*this, key, minus<float>());
    break;
  case Add0DConstant:
    processor = new pp30<plus<float> >(*this, key, plus<float>());
    break;
  case Multiply0DConstant:
    processor = new pp30<multiplies<float> >(*this, key, multiplies<float>());
    break;
  case Divide0DConstant:
    processor = new pp30<divides<float> >(*this, key, divides<float>());
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
  case ZeroDto2DHistogramming:
    processor = new pp65(*this, key);
    break;
  case OneDto2DHistogramming:
    processor = new pp66(*this, key);
    break;
  case ZeroDto1DHistogramming:
    processor = new pp67(*this, key);
    break;
  case ZeroDand1Dto2DHistogramming:
    processor = new pp68(*this, key);
    break;
  case SubsetHistogram:
    processor = new pp70(*this, key);
    break;
  case nbrOfFills:
    processor = new pp80(*this, key);
    break;
  case maximumBin:
    processor = new pp81(*this, key);
    break;
  case fwhmPeak:
    processor = new pp85(*this, key);
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
  case EvrCode:
    processor = new pp121(*this,key);
    break;
  case EventID:
    processor = new pp122(*this,key);
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
  case NbrOfCCDPhotonHits:
    processor = new pp142(*this,key);
    break;
  case CCDCoalescedPhotonHitsSpectrum:
    processor = new pp143(*this,key);
    break;
  case CCDCoalescedPhotonHitsImage:
    processor = new pp144(*this,key);
    break;
  case NbrOfCCDCoalescedPhotonHits:
    processor = new pp145(*this,key);
    break;
  case SplitLevelCoalescedPhotonHits:
    processor = new pp146(*this,key);
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
  case RealAngularDistribution:
    processor = new pp201(*this,key);
    break;
  case RealPolarTransformation:
    processor = new pp202(*this,key);
    break;
  case AdvancedPhotonFinderDump:
    processor = new pp212(*this,key);
    break;
  case PIPICO:
    processor = new pp220(*this,key);
    break;
  case PhotonEnergy:
    processor = new pp230(*this,key);
    break;
  case TestImage:
    processor = new pp240(*this,key);
    break;
  case ParticleValue:
    processor = new pp250(*this,key);
    break;
  case ParticleValues:
    processor = new pp251(*this,key);
    break;
  case NbrParticles:
    processor = new pp252(*this,key);
    break;
#ifdef SINGLEPARTICLE_HIT
  case SingleParticleDetection:
    processor = new pp300(*this,key);
    break;
#endif
  case medianLastValues:
    processor = new pp301(*this,key);
  break;
#ifdef HDF5
  case PnccdHDF5:
    processor = new pp1000(*this,key);
    break;
  case HDF5Converter:
    processor = new pp1001(*this,key,_outputfilename);
    break;
#endif
#ifdef CERNROOT
  case ROOTDump:
    processor = new pp2000(*this,key,_outputfilename);
    break;
  case ROOTTreeDump:
    processor = new pp2001(*this,key,_outputfilename);
    break;
#endif
  case ElectronEnergy:
     processor = new pp5000(*this,key);
     break;
  case TrippleCoincidence:
     processor = new pp5001(*this,key);
     break;
  default:
    throw invalid_argument(QString("PostProcessors::create(): Postprocessor '%1' has unknown ID=%2")
                           .arg(key.c_str())
                           .arg(ppid)
                           .toStdString());
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
