// Copyright (C) 2010 Lutz Foucar
// Copyright (C) 2010 Jochen Küpper

/**
 * @file processor_manager.cpp contains the manager for the processors
 *
 * @author Lutz Foucar
 */

#include <QtCore/QMutex>
#include <QtCore/QStringList>

#include <cassert>
#include <algorithm>
#include <iostream>
#include <functional>
#include <tr1/functional>

#include "processor_manager.h"

#include "cass_exceptions.hpp"
#include "processor.h"
#include "result.hpp"
#include "id_list.h"
#include "cass_settings.h"
#include "log.h"
#include "acqiris_detectors.h"
#include "achimcalibrator_hex.h"
#include "alignment.h"
#include "waveform.h"
#include "operations.h"
#include "rankfilter.h"
#include "imaging.h"
#include "machine_data.h"
#include "coltrims_analysis.h"
#include "pixel_detectors.h"
#include "image_manipulation.h"
#include "partial_covariance.h"
#include "cbf_output.h"
#include "hitfinder.h"
#include "table_operations.h"
#include "autocorrelation.h"
#include "pixel_detector_calibration.h"

#ifdef HDF5
#include "hdf5_converter.h"
#endif

#ifdef SINGLEPARTICLE_HIT
#include "hitrate.h"
#endif

#ifdef CERNROOT
#include "root_converter.h"
#include "roottree_converter.h"
#endif

#ifdef FFTW
#include "fft.h"
#endif

using namespace cass;
using namespace std;

// ============define static members (do not touch)==============
ProcessorManager::shared_pointer ProcessorManager::_instance;
QMutex ProcessorManager::_mutex;

ProcessorManager::shared_pointer ProcessorManager::instance(string outputfilename)
{
  static int create(0);
  QMutexLocker locker(&_mutex);
  if(!_instance)
  {
    Log::add(Log::VERBOSEINFO,"ProcessorManager::instance -- create "+ toString(++create));
    _instance = shared_pointer(new ProcessorManager(outputfilename));
    locker.unlock();
    _instance->loadSettings(0);
  }
  return _instance;
}

ProcessorManager::shared_pointer ProcessorManager::instance()
{
  QMutexLocker lock(&_mutex);
  if (!_instance)
    throw logic_error("ProcessorManager::instance(): The instance has not yet been created");
  return _instance;
}

ProcessorManager::shared_pointer::element_type& ProcessorManager::reference()
{
  QMutexLocker lock(&_mutex);
  if (!_instance)
    throw logic_error("ProcessorManager::reference(): The instance has not yet been created");
  return *_instance;
}
//===============================================================



ProcessorManager::ProcessorManager(string outputfilename)
  :_keys(new IdList()),
  _outputfilename(outputfilename)

{
  Log::add(Log::DEBUG0,"ProcessorManager::constructor: output Filename: " +
           _outputfilename);
}

void ProcessorManager::operator()(const CASSEvent& event)
{
  /**
   * @todo catch when processor throws an exeption and delete the
   *       processor from the active list.
   *       - create a remove list with all processors that depend on this
   *       - go through that list and fill all pp that depend on the ones in
   *         the list recursivly.
   *       - remove all pp that made it on the removelist
   *       - this needs to be done in a locked way since more than one thread
   *         do this
   *
   * @note one should not use for_each macro here, because the procressors
   *       rely on beeing processed sequantially in the right order. Using
   *       for_each could result in parallel execution via omp.
   */
  processors_t::iterator iter(_processors.begin());
  processors_t::iterator end(_processors.end());
  while(iter != end)
    (*iter++)->processEvent(event);
  /** tell the pp that the event is completly processed */
  iter=_processors.begin();
  while(iter != end)
    (*(iter++))->releaseEvent(event);
  pixeldetector::DetectorHelper::releaseDetector(event.id());
  ACQIRIS::HelperAcqirisDetectors::releaseDetector(event.id());
}

void ProcessorManager::aboutToQuit()
{
 /** @note one should not use for_each macro here, because the procressors
  *        rely on beeing processed sequantially in the right order. Using
  *        for_each could result in parallel execution via omp.
  */
  processors_t::iterator iter = _processors.begin();
  processors_t::iterator end = _processors.end();
  while( iter != end )
    (*iter++)->aboutToQuit();
}

void ProcessorManager::loadSettings(size_t)
{
  /** remove all processors */
  _processors.clear();

  /** load all processors declared in the ini file and convert them to list
   *  of std strings
   */
  Log::add(Log::DEBUG0,"ProcessorManager::loadSettings");
  CASSSettings settings;
  settings.beginGroup("Processor");
  QStringList list(settings.childGroups());
  string output("Processor::loadSettings(): ini file '" + settings.fileName().toStdString() +
                "' contains in Group '" + settings.group().toStdString() + "': ");
  foreach(QString str, list)
  {
    output += (str.toStdString() + ", ");
  }
  Log::add(Log::DEBUG1,output);
  Processor::names_t declaredProcessors(list.size());
  transform(list.begin(), list.end(), declaredProcessors.begin(),
            tr1::bind(&QString::toStdString,tr1::placeholders::_1));
  Log::add(Log::VERBOSEINFO, "ProcessorManager::loadSettings(): Number of unique processor activations: " +
           toString(declaredProcessors.size()));

  /** add a default true and false processors to beginning of list*/
  declaredProcessors.push_front("DefaultTrueHist");
  declaredProcessors.push_front("DefaultFalseHist");

  /** create all processors in the list*/
  Processor::names_t::const_iterator iter(declaredProcessors.begin());
  Processor::names_t::const_iterator end = declaredProcessors.end();
  while( iter != end )
  {
    _processors.push_back(create(*iter++));
  }

  /** sort the processors such that the ones with no dependencies are ealier
   *  in the list
   */
  processors_t::iterator pp(_processors.begin());
  processors_t::iterator pEnd(_processors.end());
  while (pp != pEnd)
  {
    /** retrive dependencies of processor */
    typedef Processor::names_t deps_t;
    const deps_t &deps((*pp)->dependencies());

    /** move all dependencies of this processor that appear later in the list
     *  to one before this processor in the list and set the pointer to
     *  the moved processor
     */
    bool reordered(false);
    deps_t::const_iterator dep(deps.begin());
    deps_t::const_iterator depEnd(deps.end());
    while (dep != depEnd)
    {
      processors_t::iterator it(pp);
      while (it != pEnd)
      {
        if (*dep == (*it)->name())
        {
          pp = _processors.insert(pp,*it);  //pp points to the inserted element
          _processors.erase(it);            //it points to the element after the erased element
          reordered = true;
          break;
        }
        ++it;
      }
      ++dep;
    }

    /** when the list has not been reordered we continue with the next element
     *  in the next iteration, otherwise we should treat the current element in
     *  the next iteration
     */
    if(!reordered)
      ++pp;
  }

  /** log which pp are generated and their order*/
  output = "ProcessorManager::loadSettings(): Processors in the order they are called:";
  pp = _processors.begin();
  while (pp != pEnd)
    output += ((*pp++)->name() + ", ");
  Log::add(Log::INFO,output);

  /** load the settings of the processors */
  pp = _processors.begin();
  while (pp != pEnd)
    (*pp++)->loadSettings(0);
}

Processor::shared_pointer
ProcessorManager::getProcessorSPointer(const Processor::name_t &name)
{
  processors_t::iterator iter(_processors.begin());
  processors_t::iterator end(_processors.end());
  while (iter != end)
  {
    if ((*iter)->name() ==  name)
      return *iter;
    ++iter;
  }
  throw InvalidProcessorError(name);
}

Processor& ProcessorManager::getProcessor(const Processor::name_t &name)
{
  return *getProcessorSPointer(name);
}

tr1::shared_ptr<IdList> ProcessorManager::keys()
{
  keyList_t active;
  processors_t::iterator iter(_processors.begin());
  processors_t::iterator end(_processors.end());
  for(; iter != end; ++iter)
    if (!(*iter)->hide())
      active.push_back((*iter)->name());
  _keys->setList(active);
  return _keys;
}

Processor::shared_pointer ProcessorManager::create(const key_t &key)
{
  if (key == "DefaultTrueHist")
    return Processor::shared_pointer(new pp12("DefaultTrueHist"));
  if (key == "DefaultFalseHist")
    return Processor::shared_pointer(new pp12("DefaultFalseHist"));

  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(key));
  id_t ppid (static_cast<ProcessorManager::id_t>(s.value("ID",0).toUInt()));
  Log::add(Log::DEBUG0,"ProcessorManager::create(): Creating PP '" + key +
           "' with ID=" + toString(ppid));
  Processor::shared_pointer processor;
  switch(ppid)
  {
  case OperationsOn2Histos:
    processor = Processor::shared_pointer(new pp1(key));
    break;
  case OperationWithValue:
    processor = Processor::shared_pointer(new pp2(key));
    break;
  case BooleanNOT:
    processor = Processor::shared_pointer(new pp4(key));
    break;
  case CheckRange:
    processor = Processor::shared_pointer(new pp9(key));
    break;
  case ConstantValue:
    processor = Processor::shared_pointer(new pp12(key));
    break;
  case Identity:
    processor = Processor::shared_pointer(new pp13(key));
    break;
  case CheckChange:
    processor = Processor::shared_pointer(new pp15(key));
    break;
  case Threshold:
    processor = Processor::shared_pointer(new pp40(key));
    break;
  case ThresholdImage:
    processor = Processor::shared_pointer(new pp41(key));
    break;
  case TwoDProjection:
    processor = Processor::shared_pointer(new pp50(key));
    break;
  case OneDIntergral:
    processor = Processor::shared_pointer(new pp51(key));
    break;
  case imageManip:
    processor = Processor::shared_pointer(new pp55(key));
    break;
  case previousHist:
      processor = Processor::shared_pointer(new pp56(key));
    break;
  case weightedProject:
      processor = Processor::shared_pointer(new pp57(key));
    break;
  case ZeroDHistogramming:
    processor = Processor::shared_pointer(new pp60(key));
    break;
  case HistogramAveraging:
    processor = Processor::shared_pointer(new pp61(key));
    break;
  case HistogramSumming:
    processor = Processor::shared_pointer(new pp62(key));
    break;
  case TimeAverage:
    processor = Processor::shared_pointer(new pp63(key));
    break;
  case running1Dfrom0D:
    processor = Processor::shared_pointer(new pp64(key));
    break;
  case ZeroDto2DHistogramming:
    processor = Processor::shared_pointer(new pp65(key));
    break;
  case OneDto2DHistogramming:
    processor = Processor::shared_pointer(new pp66(key));
    break;
  case ZeroDto1DHistogramming:
    processor = Processor::shared_pointer(new pp67(key));
    break;
  case ZeroDand1Dto2DHistogramming:
    processor = Processor::shared_pointer(new pp68(key));
    break;
  case OneDtoScatterPlot:
    processor = Processor::shared_pointer(new pp69(key));
    break;
  case SubsetHistogram:
    processor = Processor::shared_pointer(new pp70(key));
    break;
  case RetrieveValue:
    processor = Processor::shared_pointer(new pp71(key));
    break;
  case RetrieveColFromTable:
    processor = Processor::shared_pointer(new pp72(key));
    break;
  case SubsetTable:
    processor = Processor::shared_pointer(new pp73(key));
    break;
  case RetrieveValOfRow:
    processor = Processor::shared_pointer(new pp74(key));
    break;
  case ClearHistogram:
    processor = Processor::shared_pointer(new pp75(key));
    break;
  case QuitCASS:
    processor = Processor::shared_pointer(new pp76(key));
    break;
  case IdIsOnList:
    processor = Processor::shared_pointer(new pp77(key));
    break;
  case Counter:
    processor = Processor::shared_pointer(new pp78(key));
    break;
  case Table2TwoDHist:
    processor = Processor::shared_pointer(new pp79(key));
    break;
  case maximumBin:
    processor = Processor::shared_pointer(new pp81(key));
    break;
  case meanvalue:
    processor = Processor::shared_pointer(new pp82(key));
    break;
  case fwhmPeak:
    processor = Processor::shared_pointer(new pp85(key));
    break;
  case step:
    processor = Processor::shared_pointer(new pp86(key));
    break;
  case centerofmass:
    processor = Processor::shared_pointer(new pp87(key));
    break;
  case axisparameter:
    processor = Processor::shared_pointer(new pp88(key));
    break;
  case highlowpassfilter:
    processor = Processor::shared_pointer(new pp89(key));
    break;
  case qaverage:
    processor = Processor::shared_pointer(new pp90(key));
    break;
  case nodes:
    processor = Processor::shared_pointer(new pp91(key));
    break;
  case PixelDetectorImage:
    processor = Processor::shared_pointer(new pp105(key));
    break;
  case CorrectionMaps:
    processor = Processor::shared_pointer(new pp107(key));
    break;
  case RAWPixeldetectorFrame:
    processor = Processor::shared_pointer(new pp109(key));
    break;
  case AcqirisWaveform:
    processor = Processor::shared_pointer(new pp110(key));
    break;
  case CFDTraceFromWaveform:
    processor = Processor::shared_pointer(new pp111(key));
    break;
  case BlData:
    processor = Processor::shared_pointer(new pp120(key));
    break;
  case EvrCode:
    processor = Processor::shared_pointer(new pp121(key));
    break;
  case EventID:
    processor = Processor::shared_pointer(new pp122(key));
    break;
  case BldSpecData:
    processor = Processor::shared_pointer(new pp123(key));
    break;
  case EpicsData:
    processor = Processor::shared_pointer(new pp130(key));
    break;
  case CCDCoalescedPhotonHitsImage:
    processor = Processor::shared_pointer(new pp144(key));
    break;
  case NbrOfCCDCoalescedPhotonHits:
    processor = Processor::shared_pointer(new pp145(key));
    break;
  case SplitLevelCoalescedPhotonHits:
    processor = Processor::shared_pointer(new pp146(key));
    break;
  case NewCCDPhotonHitsImage:
    processor = Processor::shared_pointer(new pp148(key));
    break;
  case NewNbrOfCCDPhotonHits:
    processor = Processor::shared_pointer(new pp149(key));
    break;
  case TofDetNbrSignals:
    processor = Processor::shared_pointer(new pp150(key));
    break;
  case TofDetAllSignals:
    processor = Processor::shared_pointer(new pp151(key));
    break;
  case TofDetMcpHeightVsFwhm:
    processor = Processor::shared_pointer(new pp152(key));
    break;
  case TofDetDeadtime:
    processor = Processor::shared_pointer(new pp153(key));
    break;
  case WireendNbrSignals:
    processor = Processor::shared_pointer(new pp160(key));
    break;
  case WireendHeightvsFwhm:
    processor = Processor::shared_pointer(new pp161(key));
    break;
  case AnodeTimesum:
    processor = Processor::shared_pointer(new pp162(key));
    break;
  case AnodeTimesumVsPos:
    processor = Processor::shared_pointer(new pp163(key));
    break;
  case DelaylineFirstGoodHit:
    processor = Processor::shared_pointer(new pp164(key));
    break;
  case DelaylineNbrReconstructedHits:
    processor = Processor::shared_pointer(new pp165(key));
    break;
  case DelaylineAllReconstuctedHits:
    processor = Processor::shared_pointer(new pp166(key));
    break;
  case DelaylineAnodeSigDeadtime:
    processor = Processor::shared_pointer(new pp167(key));
    break;
  case HEXCalibrator:
    processor = Processor::shared_pointer(new HexCalibrator(key));
    break;
  case Cos2Theta:
    processor = Processor::shared_pointer(new pp200(key));
    break;
  case RealAngularDistribution:
    processor = Processor::shared_pointer(new pp201(key));
    break;
  case RealPolarTransformation:
    processor = Processor::shared_pointer(new pp202(key));
    break;
  case MedianBoxBackground:
    processor = Processor::shared_pointer(new pp203(key));
    break;
  case BraggPeakSNR:
    processor = Processor::shared_pointer(new pp204(key));
    break;
  case DrawPeaks:
    processor = Processor::shared_pointer(new pp205(key));
    break;
  case BraggPeakThreshold:
    processor = Processor::shared_pointer(new pp206(key));
    break;
  case BraggPeakSNRWOOutliers:
    processor = Processor::shared_pointer(new pp208(key));
    break;
  case PIPICO:
    processor = Processor::shared_pointer(new pp220(key));
    break;
  case PhotonEnergy:
    processor = Processor::shared_pointer(new pp230(key));
    break;
  case TestImage:
    processor = Processor::shared_pointer(new pp240(key));
    break;
  case fixOffset:
    processor = Processor::shared_pointer(new pp241(key));
    break;
  case MaskValue:
    processor = Processor::shared_pointer(new pp242(key));
    break;
  case MaskImageValue:
    processor = Processor::shared_pointer(new pp243(key));
    break;
  case PixelHistogram:
    processor = Processor::shared_pointer(new pp244(key));
    break;
  case ParticleValue:
    processor = Processor::shared_pointer(new pp250(key));
    break;
  case ParticleValues:
    processor = Processor::shared_pointer(new pp251(key));
    break;
  case NbrParticles:
    processor = Processor::shared_pointer(new pp252(key));
    break;
#ifdef SINGLEPARTICLE_HIT
  case SingleParticleDetection:
    processor = Processor::shared_pointer(new pp300(key));
    break;
#endif
  case medianLastValues:
    processor = Processor::shared_pointer(new pp301(key));
  break;
  case binaryFile2D:
    processor = Processor::shared_pointer(new pp302(key));
    break;
  case Autocorrelation:
    processor = Processor::shared_pointer(new pp310(key));
    break;
  case Autocorrelation2:
    processor = Processor::shared_pointer(new pp311(key));
    break;
#ifdef FFTW
  case fft:
    processor = Processor::shared_pointer(new pp312(key));
    break;
#endif
#ifdef SINGLEPARTICLE_HIT
  case convoluteKernel:
    processor = Processor::shared_pointer(new pp313(key));
    break;
#endif
  case calibration:
    processor = Processor::shared_pointer(new pp330(key));
    break;
  case gaincalibration:
    processor = Processor::shared_pointer(new pp331(key));
    break;
  case hotpixmap:
    processor = Processor::shared_pointer(new pp332(key));
    break;
  case commonmodecalc:
    processor = Processor::shared_pointer(new pp333(key));
    break;
  case tof2energy:
    processor = Processor::shared_pointer(new pp400(key));
    break;
  case TofToMTC:
    processor = Processor::shared_pointer(new pp404(key));
    break;
  case PulseDuration:
    processor = Processor::shared_pointer(new pp405(key));
    break;
  case tof2energy0D:
    processor = Processor::shared_pointer(new pp406(key));
    break;
  case tof2energylinear:
    processor = Processor::shared_pointer(new pp407(key));
    break;
  case tof2energylinear0D:
    processor = Processor::shared_pointer(new pp408(key));
    break;
  case calcCovarianceMap:
    processor = Processor::shared_pointer(new pp410(key));
    break;
  case calcCorrection:
    processor = Processor::shared_pointer(new pp412(key));
    break;
#ifdef HDF5
  case HDF52dConverter:
    processor = Processor::shared_pointer(new pp1002(key));
    break;
#endif
  case CBFOutput:
    processor = Processor::shared_pointer(new pp1500(key));
    break;
  case ChetahConv:
    processor = Processor::shared_pointer(new pp1600(key));
    break;
  case CoarseCsPadAligment:
    processor = Processor::shared_pointer(new pp1601(key));
    break;
  case GeomFileCsPadAligment:
    processor = Processor::shared_pointer(new pp1602(key));
    break;
#ifdef CERNROOT
  case ROOTDump:
    processor = Processor::shared_pointer(new pp2000(key));
    break;
  case ROOTTreeDump:
    processor = Processor::shared_pointer(new pp2001(key,_outputfilename));
    break;
#endif
  case ElectronEnergy:
    processor = Processor::shared_pointer(new pp5000(key));
    break;
  case TrippleCoincidence:
    processor = Processor::shared_pointer(new pp5001(key));
    break;
  default:
    throw invalid_argument("ProcessorManager::create(): Processor '" +  key +
                           "' has unknown ID '" + toString(ppid) + "'");
  }
  return processor;
}
