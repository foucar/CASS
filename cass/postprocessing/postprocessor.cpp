// Copyright (C) 2010 Lutz Foucar
// Copyright (C) 2010 Jochen Küpper

#include <QtCore/QMutex>
#include <QtCore/QSettings>
#include <QtCore/QStringList>

#include <cassert>
#include <algorithm>
#include <iostream>

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
#include "tais_helper.h"




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
  _invalidMime("invalidMimetype"),
  _outputfilename(outputfilename)

{
  VERBOSEOUT(std::cout<<"Postprocessors::constructor: output Filename: "
             <<_outputfilename
             <<std::endl);
}

void cass::PostProcessors::process(CASSEvent& event)
{
  /**
   * @todo catch when postprocessor throws an exeption and delete the
   *       postprocessor from the active list.
   *       - create a remove list with all postprocessors that depend on this
   *       - go through that list and fill all pp that depend on the ones in
   *         the list recursivly.
   *       - remove all pp that made it on the removelist
   */
  for(keyList_t::iterator iter(_leave.begin()); iter != _leave.end(); ++iter)
    (*(_postprocessors[*iter]))(event);
}

void cass::PostProcessors::loadSettings(size_t)
{
  VERBOSEOUT(std::cout << "Postprocessor::loadSettings" << std::endl);
  QSettings settings;
  settings.beginGroup("PostProcessor");
  QStringList list(settings.childGroups());
#ifdef VERBOSE
  std::cout << settings.fileName().toStdString() << " " ;
  std::cout << "Entries of "<< settings.group().toStdString() << ": ";
  foreach(QString str, list){
    std::cout<<str.toStdString() << ", ";
  }
  std::cout << std::endl;
#endif
  keyList_t active(list.size());
  std::transform(list.begin(), list.end(), active.begin(), QStringToStdString);
  std::cout <<"   Number of unique postprocessor activations: "<<active.size()
      << std::endl;
  setup(active);
  std::cout <<"   Active postprocessor(s): ";
  for(keyList_t::iterator iter = active.begin(); iter != active.end(); ++iter)
    std::cout << *iter << " ";
}

void cass::PostProcessors::clear(key_t key)
{
  postprocessors_t::iterator it (_postprocessors.find(key));
  if (_postprocessors.end() != it)
    it->second->clear();
}

cass::PostprocessorBackend* PostProcessors::getPostProcessor(const key_t key)
{
  postprocessors_t::iterator it (_postprocessors.find(key));
  if (_postprocessors.end() != it)
    return it->second;
  else
    return 0;
}

cass::IdList* cass::PostProcessors::getIdList()
{
  _IdList->clear();
  keyList_t active;
  for(postprocessors_t::iterator iter = _postprocessors.begin(); iter != _postprocessors.end(); ++iter)
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
  for(postprocessors_t::iterator iter = _postprocessors.begin();
      iter != _postprocessors.end();
      ++iter)
  {
    keyList_t dependencyList(iter->second->dependencies());
    if (find(dependencyList.begin(),dependencyList.end(),key) != dependencyList.end())
      dependandList.push_front(iter->first);
  }
  return dependandList;
}

void cass::PostProcessors::setup(const keyList_t &active)
{
  using namespace std;
  /** @todo when load settings throws exception then remove this pp and all pp
   *        that depend on it (like in process)
   */
  // Add all PostProcessors on active list -- for histograms we simply make sure the pointer is 0 and let
  // the postprocessor correctly initialize it whenever it wants to.
  // When the PostProcessor has a dependency resolve it
  //  There can be the following cases:
  //  1) pp is not in container but key is on active list
  //  2) pp is not in conatiner and key is not in active list
  //  3) pp is in container
  //  5) pp is in container and id is not on list
  VERBOSEOUT(cout << "Postprocessor::setup(): add postprocessors to list"<<endl);
  keyList_t::iterator iter(active.begin());
  while(iter != active.end())
  {
    VERBOSEOUT(cout << "Postprocessor::setup(): check if "<<*iter
               <<" is not yet in pp container"
               <<endl);
    if(_postprocessors.end() == _postprocessors.find(*iter))
    {
      VERBOSEOUT(cout<<"Postprocessor::setup(): did not find "<<*iter
                 <<" in pp container => creating it"
                 <<endl);
      _postprocessors[*iter] = create(*iter);
    }
    else
    {
      VERBOSEOUT(cout<<"Postprocessor::setup(): "<<*iter
                 <<" is on list. Now loading Settings for it"
                 <<endl);
      _postprocessors[*iter]->loadSettings(0);
    }
    VERBOSEOUT(cout<<"Postprocessor::setup(): done creating / loading "<<*iter
               <<" Now checking pp's dependecies."
               <<endl);
    bool update(false);
    keyList_t deps(_postprocessors[*iter]->dependencies());
#ifdef VERBOSE
    if (deps.empty())
      cout<<"Postprocessor::setup(): "<<*iter
          <<" has no dependecies"
          <<endl;
#endif
    for(keyList_t::iterator d=deps.begin(); d!=deps.end(); ++d)
    {
      VERBOSEOUT(cout<<"Postprocessor::setup(): "<<*iter
                 <<" depends on "<<*d
                 <<" checking whether dependency pp is already there"
                 <<endl);
      if(_postprocessors.end() == _postprocessors.find(*d))
      {
        //solves cases 2
        VERBOSEOUT(cout<<"Postprocessor::setup(): "<<*d
                   <<" is not in pp container."
                   <<" Inserting it into the active list"
                   <<endl);
        active.push_front(*d);
        update = true;
      }
      else
      {
        VERBOSEOUT(cout<<"Postprocessor::setup(): dependency pp "<<*d
                   <<" of "<<*iter
                   <<" is in the container, check if its key is in the active list"
                   <<endl);
        keyList_t::iterator isthere(find(active.begin(), active.end(), *d));
        if(active.end() == isthere)
        {
          //solves case 5
          VERBOSEOUT(cout<<"Postprocessor::setup(): dependency "<<*d
                     <<" appeard not at all in active list "
                     <<" => add it "<<*iter
                     <<endl);
          active.push_front(*d);
          update = true;
        }
      }
    }
    // if we have updated active, start over again
    if(update)
    {
      // start over
      VERBOSEOUT(cout<<"Postprocessor::setup(): start over again."<<endl);
      iter = active.begin();
      continue;
    }
    ++iter;
  }

  // some of the postprocessors are have been created and are in the container
  // but might not be on the active list anymore. Check which they are. Put
  // them on a delete list and then delete them.
  keyList_t eraseList;
  for(postprocessors_t::iterator iter = _postprocessors.begin();
      iter != _postprocessors.end();
      ++iter)
  {
    VERBOSEOUT(cout<<"PostProcessor::setup(): Check whether "<< iter->first
               <<" is still on active list"
               <<endl);
    if(active.end() == find(active.begin(),active.end(),iter->first))
    {
      VERBOSEOUT(cout<<"PostProcessor::setup(): "<< iter->first
                 <<" is not on active list. Put it to erase list"
                 <<endl);
      eraseList.push_back(iter->first);
    }
  }
  // go through erase list and erase all postprocessors on that list
  for(keyList_t::const_iterator it = eraseList.begin();
      it != eraseList.end();
      ++it)
  {
    VERBOSEOUT(cout<<"PostProcessor::setup(): erasing "<< *it
               <<" from postprocessor container"
               <<endl);
    PostprocessorBackend* p = _postprocessors[*it];
    delete p;
    _postprocessors.erase(*it);
  }

  // now we need to find out which of the postprocessors noone depends on.
  // go through active list and retrieve the list of dependands. If thats
  // empty than we put it to the leave list
  _leave.clear();
  for(keyList_t::const_iterator it = eraseList.begin();
      it != eraseList.end();
      ++it)
  {
    VERBOSEOUT(cout<<"PostProcessor::setup():"
               <<" check whether someone depends on "<<*it
               <<endl);
    if (find_dependant(*it).empty())
      _leave.push_front(*it);
  }
}

cass::PostprocessorBackend * cass::PostProcessors::create(const key_t &key)
{
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString::fromStdString(key));
  id_t ppid (static_cast<PostProcessors::id_t>(settings.value("ID",0).toUInt()));
  VERBOSEOUT(std::cout<<"PostProcessor::create(): Create PP " << key << " with ID="<<ppid<<std::endl);
  PostprocessorBackend * processor(0);
  switch(ppid)
  {
  case ConstantLess:
    processor = new pp1(*this, key);
    break;
  case ConstantGreater:
    processor = new pp2(*this, key);
    break;
  case ConstantEqual:
    processor = new pp3(*this, key);
    break;
  case BooleanNOT:
    processor = new pp4(*this, key);
    break;
  case BooleanAND:
    processor = new pp5(*this, key);
    break;
  case BooleanOR:
    processor = new pp6(*this, key);
    break;
  case CompareForLess:
    processor = new pp7(*this, key);
    break;
  case CompareForEqual:
    processor = new pp8(*this, key);
    break;
  case CheckRange:
    processor = new pp9(*this, key);
    break;
  case ConstantTrue:
    processor = new pp10(*this, key);
    break;
  case ConstantFalse:
    processor = new pp11(*this, key);
    break;
  case SubstractHistograms:
    processor = new pp20(*this, key);
    break;
  case DivideHistograms:
    processor = new pp21(*this, key);
    break;
  case MultiplyHistograms:
    processor = new pp22(*this, key);
    break;
  case MultiplyConstant:
    processor = new pp23(*this, key);
    break;
  case SubstractConstant:
    processor = new pp24(*this, key);
    break;
  case Threshold:
    processor = new pp25(*this, key);
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
  case AdvancedPhotonFinder:
    processor = new pp210(*this,key);
    break;
  case AdvancedPhotonFinderSpectrum:
    processor = new pp211(*this,key);
    break;
  case AdvancedPhotonFinderDump:
    processor = new pp212(*this,key);
    break;
  case PIPICO:
    processor = new pp220(*this,key);
    break;
  case TaisHelperAnswer:
    processor = new pp4000(*this,key);
    break;
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
