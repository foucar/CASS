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
#include "coltrims_analysis.h"
#include "pixel_detectors.h"
#include "image_manipulation.h"
#include "partial_covariance.h"
#include "cbf_output.h"

#ifdef HDF5
#include "hdf5_converter.h"
#include "hdf5dump.h"
#endif

#ifdef HDF5
#include "hdf5_converter.h"
#include "hdf5dump.h"
#endif

#ifdef SINGLEPARTICLE_HIT
#include "hitrate.h"
#endif

#ifdef CERNROOT
#include "root_converter.h"
#include "roottree_converter.h"
#endif

//#define DEBUG_FIND_DEPENDANT

using namespace cass;
using namespace std;

// ============define static members (do not touch)==============
PostProcessors::shared_pointer PostProcessors::_instance;
QMutex PostProcessors::_mutex;

PostProcessors::shared_pointer PostProcessors::instance(string outputfilename)
{
#ifdef VERBOSE
  static int /*n(0),*/ create(0);
#endif
//  VERBOSEOUT(cerr<<"PostProcessors::instance -- call "<<++n<<endl);
  QMutexLocker locker(&_mutex);
  if(!_instance)
  {
    VERBOSEOUT(cerr<<"PostProcessors::instance -- create "<<++create
               << endl);
    _instance = shared_pointer(new PostProcessors(outputfilename));
    _instance->loadSettings(0);
  }
  return _instance;
}

PostProcessors::shared_pointer PostProcessors::instance()
{
  QMutexLocker lock(&_mutex);
  if (!_instance)
    throw logic_error("PostProcessors::instance(): The instance has not yet been created");
  return _instance;
}

PostProcessors::shared_pointer::element_type& PostProcessors::reference()
{
  QMutexLocker lock(&_mutex);
  if (!_instance)
    throw logic_error("PostProcessors::reference(): The instance has not yet been created");
  return *_instance;
}
//===============================================================



/** Internal helper function to convert QVariant to id_t */
static inline string QStringToStdString(QString str)
{
  return string(str.toStdString());
}





PostProcessors::PostProcessors(string outputfilename)
  :_keys(new IdList()),
  _outputfilename(outputfilename)

{
  VERBOSEOUT(cout<<"Postprocessors::constructor: output Filename: "
             <<_outputfilename
             <<endl);
}

void PostProcessors::operator()(const CASSEvent& event)
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
  postprocessors_t::iterator iter(_postprocessors.begin());
  postprocessors_t::iterator end(_postprocessors.end());
  for(;iter != end; ++iter)
  {
//     cout <<event.id()<<" running '" << iter->first<<"'"<<endl;
    (*(iter->second))(event);
  }
}

void PostProcessors::aboutToQuit()
{
  postprocessors_t::iterator iter = _postprocessors.begin();
  postprocessors_t::iterator end = _postprocessors.end();
  while( iter != end )
    (*iter++).second->aboutToQuit();
}

void PostProcessors::loadSettings(size_t)
{
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
  _postprocessors["DefaultTrueHist"] =
      PostprocessorBackend::shared_pointer(new pp10(*this, "DefaultTrueHist",true));
  _active.push_back("DefaultFalseHist");
  _postprocessors["DefaultFalseHist"] =
      PostprocessorBackend::shared_pointer(new pp10(*this, "DefaultFalseHist",false));
  setup(_active);
  cout <<"   Active postprocessor(s): "<<endl;
  copy(_active.begin(), _active.end(), ostream_iterator<string>(cout,"\n"));
  cout<<endl;
}

void PostProcessors::saveSettings()
{
  postprocessors_t::iterator iter(_postprocessors.begin());
  postprocessors_t::iterator end(_postprocessors.end());
  while (iter != iter)
    (*iter++).second->saveSettings(0);
}

PostprocessorBackend& PostProcessors::getPostProcessor(const key_t &key)
{
  postprocessors_t::iterator it (_postprocessors.find(key));
  if (_postprocessors.end() == it)
    throw InvalidPostProcessorError(key);
  return *(it->second);
}

tr1::shared_ptr<IdList> PostProcessors::keys()
{
  keyList_t active;
  postprocessors_t::iterator iter(_postprocessors.begin());
  postprocessors_t::iterator end(_postprocessors.end());
  for(; iter != end; ++iter)
#ifndef DEBUG
    if (!iter->second->hide())
#endif
      active.push_back(iter->first);
  _keys->setList(active);
  return _keys;
}

PostProcessors::keyList_t PostProcessors::find_dependant(const PostProcessors::key_t &key)
{
  //go through all pp and retrieve their dependencies//
  //make a list of all key that have a dependency on the requested key//
  //iteratively go through and find all depends depandats
  keyList_t dependandList;
  dependandList.push_front(key);
  postprocessors_t::const_iterator iter = _postprocessors.begin();
  postprocessors_t::const_iterator end = _postprocessors.end();
  while(iter != end)
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

void PostProcessors::setup(keyList_t &active)
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
    _postprocessors.erase(*iter);
  }
}

PostprocessorBackend::shared_pointer PostProcessors::create(const key_t &key)
{
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString::fromStdString(key));
  id_t ppid (static_cast<PostProcessors::id_t>(settings.value("ID",0).toUInt()));
  VERBOSEOUT(cout << "PostProcessor::create(): Creating PP '" << key
                  << "' with ID=" << ppid
                  << endl);
  PostprocessorBackend::shared_pointer processor;
  switch(ppid)
  {
  case ConstantLess:
    processor = PostprocessorBackend::shared_pointer
          (new pp1<less<float> >(*this, key, less<float>()));
    break;
  case ConstantGreater:
    processor = PostprocessorBackend::shared_pointer
          (new pp1<greater<float> >(*this, key, greater<float>()));
    break;
  case ConstantEqual:
    processor = PostprocessorBackend::shared_pointer
          (new pp1<equal_to<float> >(*this, key, equal_to<float>()));
    break;
  case BooleanNOT:
    processor = PostprocessorBackend::shared_pointer
          (new pp4(*this, key));
    break;
  case BooleanAND:
    processor = PostprocessorBackend::shared_pointer
          (new pp5<logical_and<bool> >(*this, key, logical_and<bool>()));
    break;
  case BooleanOR:
    processor = PostprocessorBackend::shared_pointer
          (new pp5<logical_or<bool> >(*this, key, logical_or<bool>()));
    break;
  case CompareForLess:
    processor = PostprocessorBackend::shared_pointer
          (new pp7<less<float> >(*this, key, less<float>()));
    break;
  case CompareForEqual:
    processor = PostprocessorBackend::shared_pointer
          (new pp7<greater<float> >(*this, key, greater<float>()));
    break;
  case CheckRange:
    processor = PostprocessorBackend::shared_pointer
          (new pp9(*this, key));
    break;
  case ConstantTrue:
    processor = PostprocessorBackend::shared_pointer
          (new pp10(*this, key, true));
    break;
  case ConstantFalse:
    processor = PostprocessorBackend::shared_pointer
          (new pp10(*this, key, false));
    break;
  case ConstantValue:
    processor = PostprocessorBackend::shared_pointer(new pp12(*this, key));
    break;
  case CheckChange:
    processor = PostprocessorBackend::shared_pointer(new pp15(*this, key));
    break;
  case SubtractHistograms:
    processor = PostprocessorBackend::shared_pointer
          (new pp20<minus<float> >(*this, key, minus<float>()));
    break;
  case AddHistograms:
    processor = PostprocessorBackend::shared_pointer
          (new pp20<plus<float> >(*this, key, plus<float>()));
    break;
  case DivideHistograms:
    processor = PostprocessorBackend::shared_pointer
          (new pp20<divides<float> >(*this, key, divides<float>()));
    break;
  case MultiplyHistograms:
    processor = PostprocessorBackend::shared_pointer
          (new pp20<multiplies<float> >(*this, key, multiplies<float>()));
    break;
  case SubtractConstant:
    processor = PostprocessorBackend::shared_pointer
          (new pp24<minus<float> >(*this, key, minus<float>()));
    break;
  case AddConstant:
    processor = PostprocessorBackend::shared_pointer
          (new pp24<plus<float> >(*this, key, plus<float>()));
    break;
  case MultiplyConstant:
    processor = PostprocessorBackend::shared_pointer
          (new pp24<multiplies<float> >(*this, key, multiplies<float>()));
    break;
  case DivideConstant:
    processor = PostprocessorBackend::shared_pointer
          (new pp24<divides<float> >(*this, key, divides<float>()));
    break;
  case Subtract0DConstant:
    processor = PostprocessorBackend::shared_pointer
          (new pp30<minus<float> >(*this, key, minus<float>()));
    break;
  case Add0DConstant:
    processor = PostprocessorBackend::shared_pointer
          (new pp30<plus<float> >(*this, key, plus<float>()));
    break;
  case Multiply0DConstant:
    processor = PostprocessorBackend::shared_pointer
          (new pp30<multiplies<float> >(*this, key, multiplies<float>()));
    break;
  case Divide0DConstant:
    processor = PostprocessorBackend::shared_pointer
        (new pp30<divides<float> >(*this, key, divides<float>()));
    break;
  case Threshold:
    processor = PostprocessorBackend::shared_pointer
          (new pp40(*this, key));
    break;
  case TwoDProjection:
    processor = PostprocessorBackend::shared_pointer
          (new pp50(*this, key));
    break;
  case OneDIntergral:
    processor = PostprocessorBackend::shared_pointer
          (new pp51(*this, key));
    break;
  case RadalProjection:
    processor = PostprocessorBackend::shared_pointer
          (new pp52(*this, key));
    break;
  case AngularDistribution:
    processor = PostprocessorBackend::shared_pointer
          (new pp53(*this, key));
    break;
  case R_Phi_Representation:
    processor = PostprocessorBackend::shared_pointer
          (new pp54(*this, key));
    break;
  case imageManip:
      processor = PostprocessorBackend::shared_pointer(new pp55(*this, key));
    break;
  case previousHist:
      processor = PostprocessorBackend::shared_pointer(new pp56(*this, key));
    break;
  case ZeroDHistogramming:
    processor = PostprocessorBackend::shared_pointer
          (new pp60(*this, key));
    break;
  case HistogramAveraging:
    processor = PostprocessorBackend::shared_pointer
          (new pp61(*this, key));
    break;
  case HistogramSumming:
    processor = PostprocessorBackend::shared_pointer
          (new pp62(*this, key));
    break;
  case TimeAverage:
    processor = PostprocessorBackend::shared_pointer
          (new pp63(*this, key));
    break;
  case running1Dfrom0D:
    processor = PostprocessorBackend::shared_pointer
          (new pp64(*this, key));
    break;
  case ZeroDto2DHistogramming:
    processor = PostprocessorBackend::shared_pointer
          (new pp65(*this, key));
    break;
  case OneDto2DHistogramming:
    processor = PostprocessorBackend::shared_pointer
          (new pp66(*this, key));
    break;
  case ZeroDto1DHistogramming:
    processor = PostprocessorBackend::shared_pointer
          (new pp67(*this, key));
    break;
  case ZeroDand1Dto2DHistogramming:
    processor = PostprocessorBackend::shared_pointer
          (new pp68(*this, key));
    break;
  case OneDtoScatterPlot:
    processor = PostprocessorBackend::shared_pointer(new pp69(*this, key));
    break;
  case SubsetHistogram:
    processor = PostprocessorBackend::shared_pointer
          (new pp70(*this, key));
    break;
  case RetrieveValue:
    processor = PostprocessorBackend::shared_pointer(new pp71(*this, key));
    break;
  case ClearHistogram:
    processor = PostprocessorBackend::shared_pointer(new pp75(*this, key));
    break;
  case QuitCASS:
    processor = PostprocessorBackend::shared_pointer(new pp76(*this, key));
    break;
  case IdIsOnList:
    processor = PostprocessorBackend::shared_pointer(new pp77(*this, key));
    break;
  case nbrOfFills:
    processor = PostprocessorBackend::shared_pointer
          (new pp80(*this, key));
    break;
  case maximumBin:
    processor = PostprocessorBackend::shared_pointer
          (new pp81(*this, key));
    break;
  case meanvalue:
    processor = PostprocessorBackend::shared_pointer(new pp82(*this, key));
    break;
  case standartDev:
    processor = PostprocessorBackend::shared_pointer(new pp83(*this, key));
    break;
  case sumbins:
    processor = PostprocessorBackend::shared_pointer(new pp84(*this, key));
    break;
  case fwhmPeak:
      processor = PostprocessorBackend::shared_pointer
            (new pp85(*this, key));
      break;
  case step:
      processor = PostprocessorBackend::shared_pointer(new pp86(*this, key));
      break;
  case centerofmass:
      processor = PostprocessorBackend::shared_pointer(new pp87(*this, key));
      break;
  case SingleCcdImage:
    processor = PostprocessorBackend::shared_pointer
          (new pp100(*this, key));
    break;
  case SingleCcdImageIntegral:
    processor = PostprocessorBackend::shared_pointer
          (new pp101(*this, key));
    break;
  case SingleCcdImageIntegralOverThres:
    processor = PostprocessorBackend::shared_pointer
          (new pp102(*this, key));
    break;
  case PixelDetectorImage:
    processor = PostprocessorBackend::shared_pointer
          (new pp105(*this, key));
    break;
  case PixelDetectorImageHistogram:
    processor = PostprocessorBackend::shared_pointer
          (new pp106(*this, key));
    break;
  case CorrectionMaps:
    processor = PostprocessorBackend::shared_pointer
          (new pp107(*this, key));
    break;
  case SumPixels:
    processor = PostprocessorBackend::shared_pointer
          (new pp108(*this, key));
    break;
  case AcqirisWaveform:
    processor = PostprocessorBackend::shared_pointer
          (new pp110(*this,key));
    break;
  case BlData:
    processor = PostprocessorBackend::shared_pointer
          (new pp120(*this,key));
    break;
  case EvrCode:
    processor = PostprocessorBackend::shared_pointer
          (new pp121(*this,key));
    break;
  case EventID:
    processor = PostprocessorBackend::shared_pointer
          (new pp122(*this,key));
    break;
  case EpicsData:
    processor = PostprocessorBackend::shared_pointer
          (new pp130(*this,key));
    break;
  case CCDPhotonHitsSpectrum:
    processor = PostprocessorBackend::shared_pointer
          (new pp140(*this,key));
    break;
  case CCDPhotonHitsImage:
    processor = PostprocessorBackend::shared_pointer
          (new pp141(*this,key));
    break;
  case NbrOfCCDPhotonHits:
    processor = PostprocessorBackend::shared_pointer
          (new pp142(*this,key));
    break;
  case CCDCoalescedPhotonHitsSpectrum:
    processor = PostprocessorBackend::shared_pointer
          (new pp143(*this,key));
    break;
  case CCDCoalescedPhotonHitsImage:
    processor = PostprocessorBackend::shared_pointer
          (new pp144(*this,key));
    break;
  case NbrOfCCDCoalescedPhotonHits:
    processor = PostprocessorBackend::shared_pointer
          (new pp145(*this,key));
    break;
  case SplitLevelCoalescedPhotonHits:
    processor = PostprocessorBackend::shared_pointer
          (new pp146(*this,key));
    break;
  case NewCCDPhotonHitsSpectrum:
    processor = PostprocessorBackend::shared_pointer
          (new pp147(*this,key));
    break;
  case NewCCDPhotonHitsImage:
    processor = PostprocessorBackend::shared_pointer
          (new pp148(*this,key));
    break;
  case NewNbrOfCCDPhotonHits:
    processor = PostprocessorBackend::shared_pointer
          (new pp149(*this,key));
    break;
  case TofDetNbrSignals:
    processor = PostprocessorBackend::shared_pointer
          (new pp150(*this, key));
    break;
  case TofDetAllSignals:
    processor = PostprocessorBackend::shared_pointer
          (new pp151(*this, key));
    break;
  case TofDetMcpHeightVsFwhm:
    processor = PostprocessorBackend::shared_pointer
          (new pp152(*this, key));
    break;
  case SumFoundPixels:
    processor = PostprocessorBackend::shared_pointer
          (new pp155(*this, key));
    break;
  case SumPhotonHits:
    processor = PostprocessorBackend::shared_pointer
          (new pp156(*this, key));
    break;
  case WireendNbrSignals:
    processor = PostprocessorBackend::shared_pointer
          (new pp160(*this, key));
    break;
  case WireendHeightvsFwhm:
    processor = PostprocessorBackend::shared_pointer
          (new pp161(*this, key));
    break;
  case AnodeTimesum:
    processor = PostprocessorBackend::shared_pointer
          (new pp162(*this, key));
    break;
  case AnodeTimesumVsPos:
    processor = PostprocessorBackend::shared_pointer
          (new pp163(*this, key));
    break;
  case DelaylineFirstGoodHit:
    processor = PostprocessorBackend::shared_pointer
          (new pp164(*this, key));
    break;
  case DelaylineNbrReconstructedHits:
    processor = PostprocessorBackend::shared_pointer
          (new pp165(*this, key));
    break;
  case DelaylineAllReconstuctedHits:
    processor = PostprocessorBackend::shared_pointer
          (new pp166(*this, key));
    break;
  case Cos2Theta:
    processor = PostprocessorBackend::shared_pointer
          (new pp200(*this,key));
    break;
  case RealAngularDistribution:
    processor = PostprocessorBackend::shared_pointer
          (new pp201(*this,key));
    break;
  case RealPolarTransformation:
    processor = PostprocessorBackend::shared_pointer
          (new pp202(*this,key));
    break;
  case AdvancedPhotonFinderDump:
    processor = PostprocessorBackend::shared_pointer
          (new pp212(*this,key));
    break;
  case PIPICO:
    processor = PostprocessorBackend::shared_pointer
          (new pp220(*this,key));
    break;
  case PhotonEnergy:
    processor = PostprocessorBackend::shared_pointer
          (new pp230(*this,key));
    break;
    case TestImage:
      processor = PostprocessorBackend::shared_pointer
            (new pp240(*this,key));
      break;
  case fixOffset:
    processor = PostprocessorBackend::shared_pointer(new pp241(*this,key));
    break;
  case MaskValue:
    processor = PostprocessorBackend::shared_pointer(new pp242(*this,key));
    break;
  case ParticleValue:
    processor = PostprocessorBackend::shared_pointer
          (new pp250(*this,key));
    break;
  case ParticleValues:
    processor = PostprocessorBackend::shared_pointer
          (new pp251(*this,key));
    break;
  case NbrParticles:
    processor = PostprocessorBackend::shared_pointer
          (new pp252(*this,key));
    break;
#ifdef SINGLEPARTICLE_HIT
  case SingleParticleDetection:
    processor = PostprocessorBackend::shared_pointer
          (new pp300(*this,key));
    break;
#endif
  case medianLastValues:
    processor = PostprocessorBackend::shared_pointer
          (new pp301(*this,key));
  break;
  case binaryFile2D:
      processor = PostprocessorBackend::shared_pointer (new pp302(*this, key));
      break;
  case tof2energy:
      processor = PostprocessorBackend::shared_pointer(new pp400(*this,key));
      break;
  case calcVariance:
      processor = PostprocessorBackend::shared_pointer(new pp401(*this,key));
      break;
  case HistogramSqAveraging:
      processor = PostprocessorBackend::shared_pointer(new pp402(*this, key));
      break;
  case Bin1DHist:
      processor = PostprocessorBackend::shared_pointer(new pp403(*this, key));
      break;
  case TofToMTC:
      processor = PostprocessorBackend::shared_pointer(new pp404(*this, key));
      break;
  case PulseDuration:
      processor = PostprocessorBackend::shared_pointer(new pp405(*this,key));
      break;
  case tof2energy0D:
      processor = PostprocessorBackend::shared_pointer(new pp406(*this,key));
      break;
  case tof2energylinear:
      processor = PostprocessorBackend::shared_pointer(new pp407(*this,key));
      break;
  case tof2energylinear0D:
      processor = PostprocessorBackend::shared_pointer(new pp408(*this,key));
      break;
  case calcCovarianceMap:
      processor = PostprocessorBackend::shared_pointer(new pp410(*this, key));
      break;
  case calcCorrection:
      processor = PostprocessorBackend::shared_pointer(new pp412(*this, key));
      break;
  case EventNumber:
      processor = PostprocessorBackend::shared_pointer(new pp420(*this, key));
      break;
#ifdef HDF5
  case PnccdHDF5:
    processor = PostprocessorBackend::shared_pointer
          (new pp1000(*this,key));
    break;
  case HDF5Converter:
    processor = PostprocessorBackend::shared_pointer
          (new pp1001(*this,key,_outputfilename));
    break;
  case HDF52dConverter:
    processor = PostprocessorBackend::shared_pointer
          (new pp1002(*this,key,_outputfilename));
    break;
#endif
  case CBFOutput:
    processor = PostprocessorBackend::shared_pointer
        (new pp1500(*this,key,_outputfilename));
    break;
#ifdef CERNROOT
  case ROOTDump:
    processor = PostprocessorBackend::shared_pointer
          (new pp2000(*this,key,_outputfilename));
    break;
  case ROOTTreeDump:
    processor = PostprocessorBackend::shared_pointer
          (new pp2001(*this,key,_outputfilename));
    break;
#endif
  case ElectronEnergy:
     processor = PostprocessorBackend::shared_pointer
           (new pp5000(*this,key));
     break;
  case TrippleCoincidence:
     processor = PostprocessorBackend::shared_pointer
           (new pp5001(*this,key));
     break;
  default:
    throw invalid_argument("PostProcessors::create(): Postprocessor '" +  key +
                           "' has unknown ID '" + toString(ppid) + "'");
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
