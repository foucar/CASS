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


namespace cass
{


// ============define static members (do not touch)==============
PostProcessors *PostProcessors::_instance(0);
QMutex PostProcessors::_mutex;


// create an instance of the singleton
PostProcessors *PostProcessors::instance(std::string outputfilename)
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
void PostProcessors::destroy()
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





PostProcessors::PostProcessors(std::string outputfilename)
  :_IdList(new IdList()),
  _invalidMime("invalidMimetype"),
  _outputfilename(outputfilename)

{
  VERBOSEOUT(std::cout<<"Postprocessors::constructor: output Filename: "
             <<_outputfilename
             <<std::endl);
    // set up list of all active postprocessors/histograms
    // and fill maps of histograms and postprocessors
    /**
     * @todo maybe delay the call to load settings, so that in load settings
     *       one can call instance again.
     */
    loadSettings(0);
}


void PostProcessors::process(CASSEvent& event)
{
  /**
   * @todo catch when postprocessor throws an exeption and delete the
   *       postprocessor from the active list.
   *       - create a remove list with all postprocessors that depend on this
   *       - go through that list and fill all pp that depend on the ones in
   *         the list recursivly.
   *       - remove all pp that made it on the removelist
   */
  for(active_t::iterator iter(_active.begin()); iter != _active.end(); ++iter)
    (*(_postprocessors[*iter]))(event);
}


void PostProcessors::loadSettings(size_t)
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
  _active.resize(list.size());
  std::transform(list.begin(), list.end(), _active.begin(), QStringToStdString);
  std::cout <<"   Number of unique postprocessor activations: "<<_active.size()
      << std::endl;
  setup();
  std::cout <<"   Active postprocessor(s): ";
  for(active_t::iterator iter = _active.begin(); iter != _active.end(); ++iter)
    std::cout << *iter << " ";
}

void PostProcessors::clear(key_t key)
{
  try
  {
    validate(key);
  }
  catch (InvalidHistogramError&)
  {
    return;
  }
  histograms_checkout().find(key)->second->clear();
  histograms_release();
}

IdList* PostProcessors::getIdList()
{
  _IdList->clear();
  _IdList->setList(_active);
  return _IdList;
}

const std::string& PostProcessors::getMimeType(key_t key)
{
  /** @todo make sure that we do not need to block access to the histograms */
  histograms_t::iterator it = _histograms.find(key);
  if (it!=_histograms.end())
    return it->second->mimeType();
  VERBOSEOUT(std::cout << "PostProcessors::getMimeType id not found "<<key
             <<std::endl);
  return _invalidMime;
}

void PostProcessors::_delete(key_t key)
{
  histograms_t::iterator iter(_histograms.find(key));
  if (iter == _histograms.end())
    return;
  HistogramBackend *hist(iter->second);
  _histograms.erase(iter);
  delete hist;
}

void PostProcessors::_replace(key_t key, HistogramBackend *hist)
{
  _delete(key);
  hist->key() = key;
  _histograms.insert(std::make_pair(key, hist));
}

void PostProcessors::setup()
{
  /**
   * @todo don't delete all pp at the beginning:
   *       - go through all active and create / load settings, find depend, sort them
   *         - when load settings throws exception InvalidHistogram, catch it
   *           put it to a reinitialize list.
   *       - go through reinitialize list and call load setting for them once more
   *         - when load settings throws an exception then remove this pp and
   *           all pp that depend on it (like in process)
   *       - go through active list, compare to existing pp
   *       - when there are existing pp that are not active put them on remove list
   *       - delete all pp on the remove list
   */
  // for the time beeing delete all existing postprocessors
  for(postprocessors_t::iterator iter = _postprocessors.begin();
      iter != _postprocessors.end();
      ++iter)
    delete iter->second;
  _postprocessors.clear();

  // Add all PostProcessors on active list -- for histograms we simply make sure
  // the pointer is 0 and let the postprocessor correctly initialize it whenever
  // it wants to. When the PostProcessor has a dependency resolve it
  VERBOSEOUT(std::cout << "Postprocessor::setup(): add postprocessors to list"<<std::endl);
  active_t::iterator iter(_active.begin());
  while(iter != _active.end())
  {
    VERBOSEOUT(std::cout << "Postprocessor::setup(): check that "<<*iter
               <<" is not implemented"
               <<std::endl);
    // check that the postprocessor is not already implemented
    if(_postprocessors.end() == _postprocessors.find(*iter))
    {
      VERBOSEOUT(std::cout << "Postprocessor::setup(): did not find "<<*iter
                 <<" in histogram container => creating it"
                 <<std::endl);
      // create postprocessor
      _histograms[*iter] = 0;
      _postprocessors[*iter] = create(*iter);
      VERBOSEOUT(std::cout << "Postprocessor::setup(): done creating "<<*iter
                 <<" Now checking its dependecies."
                 <<std::endl);
      // check for dependencies; if there are any open dependencies put all of
      // them in front of us
      bool update(false);
      active_t deps(_postprocessors[*iter]->dependencies());
      for(active_t::iterator d=deps.begin(); d!=deps.end(); ++d)
      {
        VERBOSEOUT(std::cout << "Postprocessor::setup(): "<<*iter<<" depends on "<<*d
                   <<" checking whether dependecy is already there"
                   <<std::endl);
        if(_postprocessors.end() == _postprocessors.find(*d))
        {
          VERBOSEOUT(std::cout << "Postprocessor::setup(): "<<*d
                     <<" is not in postprocessor container"
                     <<" inserting it into the active list before "<<*iter
                     <<std::endl);
          _active.insert(iter, *d);
          active_t::iterator remove(find(iter, _active.end(), *d));
          VERBOSEOUT(std::cout << "Postprocessor::setup(): check whether dependency "<<*d
                     <<" was on the active list, but not at the right position"
                     <<std::endl);
          if(_active.end() != remove)
          {
            VERBOSEOUT(std::cout << "Postprocessor::setup(): dependency "<<*d <<" was on list"
                       <<" removing the later double entry."
                       <<std::endl);
            _active.erase(remove);
          }
          update = true;
        }
      }
      // if we have updated _active, start over again
      if(update)
      {
        // start over
        VERBOSEOUT(std::cout << "Postprocessor::setup(): start over again."<<std::endl);
        iter = _active.begin();
        continue;
      }
    }
    ++iter;
  }

  for(active_t::iterator iter = _active.begin(); iter != _active.end(); ++iter)
  {
    VERBOSEOUT(std::cout << "PostProcessor::setup(): 2nd loading: load settings of "
               << *iter
               <<std::endl);
    _postprocessors[*iter]->loadSettings(0);
  }

#ifdef VERBOSE
  VERBOSEOUT(std::cout << "active postprocessors processing order: ");
  for(active_t::iterator iter(_active.begin()); iter != _active.end(); ++iter)
    VERBOSEOUT(std::cout << *iter << ", ");
  VERBOSEOUT( std::cout << std::endl);
#endif
}


PostprocessorBackend * PostProcessors::create(const key_t &key)
{
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString::fromStdString(key));
  id_t ppid (static_cast<PostProcessors::id_t>(settings.value("ID",0).toUInt()));
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
  case SingleCcdImage:
    processor = new pp100(*this, key);
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
  case PIPICO:
    processor = new pp220(*this,key);
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

} // end namespace cass



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
