// Copyright (C) 2010 Lutz Foucar
// Copyright (C) 2010 Jochen Küpper

/**
 * @file processor_manager.cpp contains the manager for the postprocessors
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

#include "acqiris_detectors.h"
#include "achimcalibrator_hex.h"
#include "histogram.h"
#include "alignment.h"
#include "processor_manager.h"
#include "waveform.h"
#include "operations.h"
#include "rankfilter.h"
#include "imaging.h"
#include "machine_data.h"
#include "processor.h"
#include "machine_data.h"
#include "id_list.h"
#include "cass_exceptions.h"
#include "cass_settings.h"
#include "coltrims_analysis.h"
#include "pixel_detectors.h"
#include "image_manipulation.h"
#include "partial_covariance.h"
#include "cbf_output.h"
#include "log.h"
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
PostProcessors::shared_pointer PostProcessors::_instance;
QMutex PostProcessors::_mutex;

PostProcessors::shared_pointer PostProcessors::instance(string outputfilename)
{
  static int create(0);
  QMutexLocker locker(&_mutex);
  if(!_instance)
  {
    Log::add(Log::VERBOSEINFO,"PostProcessors::instance -- create "+ toString(++create));
    _instance = shared_pointer(new PostProcessors(outputfilename));
    locker.unlock();
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
  Log::add(Log::DEBUG0,"Postprocessors::constructor: output Filename: " +
           _outputfilename);
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
   *
   * @note one should not use for_each macro here, because the postprocressors
   *       rely on beeing processed sequantially in the right order. Using
   *       for_each could result in parallel execution via omp.
   */
  postprocessors_t::iterator iter(_postprocessors.begin());
  postprocessors_t::iterator end(_postprocessors.end());
  while(iter != end)
    (*iter++)->processEvent(event);
  /** tell the pp that the event is completly processed */
  iter=_postprocessors.begin();
  while(iter != end)
    (*(iter++))->releaseEvent(event);
}

void PostProcessors::aboutToQuit()
{
 /** @note one should not use for_each macro here, because the postprocressors
  *        rely on beeing processed sequantially in the right order. Using
  *        for_each could result in parallel execution via omp.
  */
  postprocessors_t::iterator iter = _postprocessors.begin();
  postprocessors_t::iterator end = _postprocessors.end();
  while( iter != end )
    (*iter++)->aboutToQuit();
}

void PostProcessors::loadSettings(size_t)
{
  /** remove all postprocessors */
  _postprocessors.clear();

  /** load all postprocessors declared in the ini file and convert them to list
   *  of std strings
   */
  Log::add(Log::DEBUG0,"Postprocessor::loadSettings");
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  QStringList list(settings.childGroups());
  string output("PostProcessor::loadSettings(): ini file '" + settings.fileName().toStdString() +
                "' contains in Group '" + settings.group().toStdString() + "': ");
  foreach(QString str, list)
  {
    output += (str.toStdString() + ", ");
  }
  Log::add(Log::DEBUG1,output);
  PostProcessor::names_t declaredPostProcessors(list.size());
  transform(list.begin(), list.end(), declaredPostProcessors.begin(), tr1::bind(&QString::toStdString,tr1::placeholders::_1));
  Log::add(Log::VERBOSEINFO, "PostProcessors::loadSettings(): Number of unique postprocessor activations: " +
           toString(declaredPostProcessors.size()));

  /** add a default true and false postprocessors to beginning of list*/
  declaredPostProcessors.push_front("DefaultTrueHist");
  declaredPostProcessors.push_front("DefaultFalseHist");

  /** create all postprocessors in the list*/
  PostProcessor::names_t::const_iterator iter(declaredPostProcessors.begin());
  PostProcessor::names_t::const_iterator end = declaredPostProcessors.end();
  while( iter != end )
  {
    _postprocessors.push_back(create(*iter++));
  }

  /** sort the postprocessors such that the ones with no dependencies are ealier
   *  in the list
   */
  postprocessors_t::iterator pp(_postprocessors.begin());
  postprocessors_t::iterator pEnd(_postprocessors.end());
  while (pp != pEnd)
  {
    /** retrive dependencies of postprocessor */
    typedef PostProcessor::names_t deps_t;
    const deps_t &deps((*pp)->dependencies());

    /** move all dependencies of this postprocessor that appear later in the list
     *  to one before this postprocessor in the list and set the pointer to
     *  the moved postprocessor
     */
    bool reordered(false);
    deps_t::const_iterator dep(deps.begin());
    deps_t::const_iterator depEnd(deps.end());
    while (dep != depEnd)
    {
      postprocessors_t::iterator it(pp);
      while (it != pEnd)
      {
        if (*dep == (*it)->name())
        {
          pp = _postprocessors.insert(pp,*it);  //pp points to the inserted element
          _postprocessors.erase(it);            //it points to the element after the erased element
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
  output = "PostProcessors::loadSettings(): PostProcessors in the order they are called:";
  pp = _postprocessors.begin();
  while (pp != pEnd)
    output += ((*pp++)->name() + ", ");
  Log::add(Log::INFO,output);

  /** load the settings of the postprocessors */
  pp = _postprocessors.begin();
  while (pp != pEnd)
    (*pp++)->loadSettings(0);
}

PostProcessor::shared_pointer
PostProcessors::getPostProcessorSPointer(const PostProcessor::name_t &name)
{
  postprocessors_t::iterator iter(_postprocessors.begin());
  postprocessors_t::iterator end(_postprocessors.end());
  while (iter != end)
  {
    if ((*iter)->name() ==  name)
      return *iter;
    ++iter;
  }
  throw InvalidPostProcessorError(name);
}

PostProcessor& PostProcessors::getPostProcessor(const PostProcessor::name_t &name)
{
  return *getPostProcessorSPointer(name);
}

tr1::shared_ptr<IdList> PostProcessors::keys()
{
  keyList_t active;
  postprocessors_t::iterator iter(_postprocessors.begin());
  postprocessors_t::iterator end(_postprocessors.end());
  for(; iter != end; ++iter)
#ifndef DEBUG
    if (!(*iter)->hide())
#endif
      active.push_back((*iter)->name());
  _keys->setList(active);
  return _keys;
}

PostProcessors::keyList_t PostProcessors::find_dependant(const PostProcessors::key_t &/*key*/)
{
  throw runtime_error("PostProcessors::find_dependant: Should not be used anymore");
  keyList_t dependandList;
  return dependandList;
}


PostProcessor::shared_pointer PostProcessors::create(const key_t &key)
{
  if (key == "DefaultTrueHist")
    return PostProcessor::shared_pointer(new pp12("DefaultTrueHist"));
  if (key == "DefaultFalseHist")
    return PostProcessor::shared_pointer(new pp12("DefaultFalseHist"));

  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(key));
  id_t ppid (static_cast<PostProcessors::id_t>(s.value("ID",0).toUInt()));
  Log::add(Log::DEBUG0,"PostProcessor::create(): Creating PP '" + key +
           "' with ID=" + toString(ppid));
  PostProcessor::shared_pointer processor;
  switch(ppid)
  {
  case OperationsOn2Histos:
    processor = PostProcessor::shared_pointer(new pp1(key));
    break;
  case OperationWithValue:
    processor = PostProcessor::shared_pointer(new pp2(key));
    break;
  case BooleanNOT:
    processor = PostProcessor::shared_pointer(new pp4(key));
    break;
  case CheckRange:
    processor = PostProcessor::shared_pointer(new pp9(key));
    break;
  case ConstantValue:
    processor = PostProcessor::shared_pointer(new pp12(key));
    break;
  case CheckChange:
    processor = PostProcessor::shared_pointer(new pp15(key));
    break;
  case Threshold:
    processor = PostProcessor::shared_pointer(new pp40(key));
    break;
  case ThresholdImage:
    processor = PostProcessor::shared_pointer(new pp41(key));
    break;
  case TwoDProjection:
    processor = PostProcessor::shared_pointer(new pp50(key));
    break;
  case OneDIntergral:
    processor = PostProcessor::shared_pointer(new pp51(key));
    break;
  case imageManip:
    processor = PostProcessor::shared_pointer(new pp55(key));
    break;
  case previousHist:
      processor = PostProcessor::shared_pointer(new pp56(key));
    break;
  case weightedProject:
      processor = PostProcessor::shared_pointer(new pp57(key));
    break;
  case ZeroDHistogramming:
    processor = PostProcessor::shared_pointer(new pp60(key));
    break;
  case HistogramAveraging:
    processor = PostProcessor::shared_pointer(new pp61(key));
    break;
  case HistogramSumming:
    processor = PostProcessor::shared_pointer(new pp62(key));
    break;
  case TimeAverage:
    processor = PostProcessor::shared_pointer(new pp63(key));
    break;
  case running1Dfrom0D:
    processor = PostProcessor::shared_pointer(new pp64(key));
    break;
  case ZeroDto2DHistogramming:
    processor = PostProcessor::shared_pointer(new pp65(key));
    break;
  case OneDto2DHistogramming:
    processor = PostProcessor::shared_pointer(new pp66(key));
    break;
  case ZeroDto1DHistogramming:
    processor = PostProcessor::shared_pointer(new pp67(key));
    break;
  case ZeroDand1Dto2DHistogramming:
    processor = PostProcessor::shared_pointer(new pp68(key));
    break;
  case OneDtoScatterPlot:
    processor = PostProcessor::shared_pointer(new pp69(key));
    break;
  case SubsetHistogram:
    processor = PostProcessor::shared_pointer(new pp70(key));
    break;
  case RetrieveValue:
    processor = PostProcessor::shared_pointer(new pp71(key));
    break;
  case RetrieveColFromTable:
    processor = PostProcessor::shared_pointer(new pp72(key));
    break;
  case SubsetTable:
    processor = PostProcessor::shared_pointer(new pp73(key));
    break;
  case RetrieveValOfRow:
    processor = PostProcessor::shared_pointer(new pp74(key));
    break;
  case ClearHistogram:
    processor = PostProcessor::shared_pointer(new pp75(key));
    break;
  case QuitCASS:
    processor = PostProcessor::shared_pointer(new pp76(key));
    break;
  case IdIsOnList:
    processor = PostProcessor::shared_pointer(new pp77(key));
    break;
  case Counter:
    processor = PostProcessor::shared_pointer(new pp78(key));
    break;
  case maximumBin:
    processor = PostProcessor::shared_pointer(new pp81(key));
    break;
  case meanvalue:
    processor = PostProcessor::shared_pointer(new pp82(key));
    break;
  case fwhmPeak:
    processor = PostProcessor::shared_pointer(new pp85(key));
    break;
  case step:
    processor = PostProcessor::shared_pointer(new pp86(key));
    break;
  case centerofmass:
    processor = PostProcessor::shared_pointer(new pp87(key));
    break;
  case axisparameter:
    processor = PostProcessor::shared_pointer(new pp88(key));
    break;
  case highlowpassfilter:
    processor = PostProcessor::shared_pointer(new pp89(key));
    break;
  case qaverage:
    processor = PostProcessor::shared_pointer(new pp90(key));
    break;
  case nodes:
    processor = PostProcessor::shared_pointer(new pp91(key));
    break;
  case PixelDetectorImage:
    processor = PostProcessor::shared_pointer(new pp105(key));
    break;
  case CorrectionMaps:
    processor = PostProcessor::shared_pointer(new pp107(key));
    break;
  case RAWPixeldetectorFrame:
    processor = PostProcessor::shared_pointer(new pp109(key));
    break;
  case AcqirisWaveform:
    processor = PostProcessor::shared_pointer(new pp110(key));
    break;
  case BlData:
    processor = PostProcessor::shared_pointer(new pp120(key));
    break;
  case EvrCode:
    processor = PostProcessor::shared_pointer(new pp121(key));
    break;
  case EventID:
    processor = PostProcessor::shared_pointer(new pp122(key));
    break;
  case BldSpecData:
    processor = PostProcessor::shared_pointer(new pp123(key));
    break;
  case EpicsData:
    processor = PostProcessor::shared_pointer(new pp130(key));
    break;
  case CCDCoalescedPhotonHitsImage:
    processor = PostProcessor::shared_pointer(new pp144(key));
    break;
  case NbrOfCCDCoalescedPhotonHits:
    processor = PostProcessor::shared_pointer(new pp145(key));
    break;
  case SplitLevelCoalescedPhotonHits:
    processor = PostProcessor::shared_pointer(new pp146(key));
    break;
  case NewCCDPhotonHitsImage:
    processor = PostProcessor::shared_pointer(new pp148(key));
    break;
  case NewNbrOfCCDPhotonHits:
    processor = PostProcessor::shared_pointer(new pp149(key));
    break;
  case TofDetNbrSignals:
    processor = PostProcessor::shared_pointer(new pp150(key));
    break;
  case TofDetAllSignals:
    processor = PostProcessor::shared_pointer(new pp151(key));
    break;
  case TofDetMcpHeightVsFwhm:
    processor = PostProcessor::shared_pointer(new pp152(key));
    break;
  case TofDetDeadtime:
    processor = PostProcessor::shared_pointer(new pp153(key));
    break;
  case WireendNbrSignals:
    processor = PostProcessor::shared_pointer(new pp160(key));
    break;
  case WireendHeightvsFwhm:
    processor = PostProcessor::shared_pointer(new pp161(key));
    break;
  case AnodeTimesum:
    processor = PostProcessor::shared_pointer(new pp162(key));
    break;
  case AnodeTimesumVsPos:
    processor = PostProcessor::shared_pointer(new pp163(key));
    break;
  case DelaylineFirstGoodHit:
    processor = PostProcessor::shared_pointer(new pp164(key));
    break;
  case DelaylineNbrReconstructedHits:
    processor = PostProcessor::shared_pointer(new pp165(key));
    break;
  case DelaylineAllReconstuctedHits:
    processor = PostProcessor::shared_pointer(new pp166(key));
    break;
  case DelaylineAnodeSigDeadtime:
    processor = PostProcessor::shared_pointer(new pp167(key));
    break;
  case HEXCalibrator:
    processor = PostProcessor::shared_pointer(new HexCalibrator(key));
    break;
  case Cos2Theta:
    processor = PostProcessor::shared_pointer(new pp200(key));
    break;
  case RealAngularDistribution:
    processor = PostProcessor::shared_pointer(new pp201(key));
    break;
  case RealPolarTransformation:
    processor = PostProcessor::shared_pointer(new pp202(key));
    break;
  case MedianBoxBackground:
    processor = PostProcessor::shared_pointer(new pp203(key));
    break;
  case BraggPeakSNR:
    processor = PostProcessor::shared_pointer(new pp204(key));
    break;
  case DrawPeaks:
    processor = PostProcessor::shared_pointer(new pp205(key));
    break;
  case BraggPeakThreshold:
    processor = PostProcessor::shared_pointer(new pp206(key));
    break;
  case ImageFromTable:
    processor = PostProcessor::shared_pointer(new pp207(key));
    break;
  case BraggPeakSNRWOOutliers:
    processor = PostProcessor::shared_pointer(new pp208(key));
    break;
  case PIPICO:
    processor = PostProcessor::shared_pointer(new pp220(key));
    break;
  case PhotonEnergy:
    processor = PostProcessor::shared_pointer(new pp230(key));
    break;
  case TestImage:
    processor = PostProcessor::shared_pointer(new pp240(key));
    break;
  case fixOffset:
    processor = PostProcessor::shared_pointer(new pp241(key));
    break;
  case MaskValue:
    processor = PostProcessor::shared_pointer(new pp242(key));
    break;
  case MaskImageValue:
    processor = PostProcessor::shared_pointer(new pp243(key));
    break;
  case PixelHistogram:
    processor = PostProcessor::shared_pointer(new pp244(key));
    break;
  case ParticleValue:
    processor = PostProcessor::shared_pointer(new pp250(key));
    break;
  case ParticleValues:
    processor = PostProcessor::shared_pointer(new pp251(key));
    break;
  case NbrParticles:
    processor = PostProcessor::shared_pointer(new pp252(key));
    break;
#ifdef SINGLEPARTICLE_HIT
  case SingleParticleDetection:
    processor = PostProcessor::shared_pointer(new pp300(key));
    break;
#endif
  case medianLastValues:
    processor = PostProcessor::shared_pointer(new pp301(key));
  break;
  case binaryFile2D:
    processor = PostProcessor::shared_pointer(new pp302(key));
    break;
  case Autocorrelation:
    processor = PostProcessor::shared_pointer(new pp310(key));
    break;
  case Autocorrelation2:
    processor = PostProcessor::shared_pointer(new pp311(key));
    break;
#ifdef FFTW
  case fft:
    processor = PostProcessor::shared_pointer(new pp312(key));
    break;
#endif
  case calibration:
    processor = PostProcessor::shared_pointer(new pp330(key));
    break;
  case gaincalibration:
    processor = PostProcessor::shared_pointer(new pp331(key));
    break;
  case hotpixmap:
    processor = PostProcessor::shared_pointer(new pp332(key));
    break;
  case commonmodecalc:
    processor = PostProcessor::shared_pointer(new pp333(key));
    break;
  case tof2energy:
    processor = PostProcessor::shared_pointer(new pp400(key));
    break;
  case HistogramSqAveraging:
    processor = PostProcessor::shared_pointer(new pp402(key));
    break;
  case TofToMTC:
    processor = PostProcessor::shared_pointer(new pp404(key));
    break;
  case PulseDuration:
    processor = PostProcessor::shared_pointer(new pp405(key));
    break;
  case tof2energy0D:
    processor = PostProcessor::shared_pointer(new pp406(key));
    break;
  case tof2energylinear:
    processor = PostProcessor::shared_pointer(new pp407(key));
    break;
  case tof2energylinear0D:
    processor = PostProcessor::shared_pointer(new pp408(key));
    break;
  case calcCovarianceMap:
    processor = PostProcessor::shared_pointer(new pp410(key));
    break;
  case calcCorrection:
    processor = PostProcessor::shared_pointer(new pp412(key));
    break;
#ifdef HDF5
  case HDF52dConverter:
    processor = PostProcessor::shared_pointer(new pp1002(key));
    break;
#endif
  case CBFOutput:
    processor = PostProcessor::shared_pointer(new pp1500(key));
    break;
  case ChetahConv:
    processor = PostProcessor::shared_pointer(new pp1600(key));
    break;
  case CoarseCsPadAligment:
    processor = PostProcessor::shared_pointer(new pp1601(key));
    break;
  case GeomFileCsPadAligment:
    processor = PostProcessor::shared_pointer(new pp1602(key));
    break;
#ifdef CERNROOT
  case ROOTDump:
    processor = PostProcessor::shared_pointer(new pp2000(key));
    break;
  case ROOTTreeDump:
    processor = PostProcessor::shared_pointer(new pp2001(key,_outputfilename));
    break;
#endif
  case ElectronEnergy:
    processor = PostProcessor::shared_pointer(new pp5000(key));
    break;
  case TrippleCoincidence:
    processor = PostProcessor::shared_pointer(new pp5001(key));
    break;
  default:
    throw invalid_argument("PostProcessors::create(): Postprocessor '" +  key +
                           "' has unknown ID '" + toString(ppid) + "'");
  }
  return processor;
}
