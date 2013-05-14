// Copyright (C) 2010 Lutz Foucar
// Copyright (C) 2010 Jochen Küpper

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
#include "log.h"
#include "hitfinder.h"
#include "table_operations.h"
#include "autocorrelation.h"

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
   */
  postprocessors_t::iterator iter(_postprocessors.begin());
  postprocessors_t::iterator end(_postprocessors.end());
  while(iter != end)
    (*(*iter++))(event);
}

void PostProcessors::aboutToQuit()
{
  postprocessors_t::iterator iter = _postprocessors.begin();
  postprocessors_t::iterator end = _postprocessors.end();
  while( iter != end )
    (*iter++)->aboutToQuit();
}

bool ppsort(PostprocessorBackend::shared_pointer first, PostprocessorBackend::shared_pointer second)
{
  return (*first) < (*second);
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
  PostprocessorBackend::names_t declaredPostProcessors(list.size());
  transform(list.begin(), list.end(), declaredPostProcessors.begin(), tr1::bind(&QString::toStdString,tr1::placeholders::_1));
  Log::add(Log::VERBOSEINFO, "PostProcessors::loadSettings(): Number of unique postprocessor activations: " +
           toString(declaredPostProcessors.size()));

  /** add a default true and false postprocessors to beginning of list*/
  declaredPostProcessors.push_front("DefaultTrueHist");
  declaredPostProcessors.push_front("DefaultFalseHist");

  /** create all postprocessors in the list*/
  PostprocessorBackend::names_t::const_iterator iter(declaredPostProcessors.begin());
  PostprocessorBackend::names_t::const_iterator end = declaredPostProcessors.end();
  while( iter != end )
  {
    _postprocessors.push_back(create(*iter++));
  }

  /** sort the postprocessors such that the ones with no dependencies are ealier
   *  in the list
   */
//  sort(_postprocessors.begin(),_postprocessors.end(),ppsort);
  for (size_t i=0; i < _postprocessors.size(); ++i)
  {
    for (size_t j=i+1; j < _postprocessors.size(); ++j)
    {
      *_postprocessors[i]  < *_postprocessors[j];
    }
  }

  /** log which pp are generated and their order*/
  output = "PostProcessors::loadSettings(): PostProcessors in the order they are called:";
  postprocessors_t::iterator pp(_postprocessors.begin());
  postprocessors_t::iterator pEnd(_postprocessors.end());
  while (pp != pEnd)
    output += ((*pp++)->name() + ", ");
  Log::add(Log::INFO,output);

  /** load the settings of the postprocessors */
  pp = _postprocessors.begin();
  while (pp != pEnd)
    (pp++)->get()->loadSettings(0);
}

void PostProcessors::saveSettings()
{
  postprocessors_t::iterator iter(_postprocessors.begin());
  postprocessors_t::iterator end(_postprocessors.end());
  while (iter != end)
    (*iter++)->saveSettings(0);
}

PostprocessorBackend& PostProcessors::getPostProcessor(const PostprocessorBackend::name_t &name)
{
  postprocessors_t::iterator iter(_postprocessors.begin());
  postprocessors_t::iterator end(_postprocessors.end());
  while (iter != end)
  {
    if ((*(*iter)).name() ==  name)
      return *(*iter);
    ++iter;
  }
  throw InvalidPostProcessorError(name);
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


PostprocessorBackend::shared_pointer PostProcessors::create(const key_t &key)
{
  if (key == "DefaultTrueHist")
    return PostprocessorBackend::shared_pointer(new pp10(*this, "DefaultTrueHist",true));
  if (key == "DefaultFalseHist")
    return PostprocessorBackend::shared_pointer(new pp10(*this, "DefaultFalseHist",false));

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
  case ThresholdImage:
    processor = PostprocessorBackend::shared_pointer(new pp41(*this, key));
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
  case RetrieveColFromTable:
    processor = PostprocessorBackend::shared_pointer(new pp72(*this, key));
    break;
  case SubsetTable:
    processor = PostprocessorBackend::shared_pointer(new pp73(*this, key));
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
  case axisparameter:
      processor = PostprocessorBackend::shared_pointer(new pp88(*this, key));
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
  case TofDetDeadtime:
    processor = PostprocessorBackend::shared_pointer
          (new pp153(*this, key));
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
  case DelaylineAnodeSigDeadtime:
    processor = PostprocessorBackend::shared_pointer
          (new pp167(*this, key));
    break;
  case HEXCalibrator:
    processor = PostprocessorBackend::shared_pointer
          (new HexCalibrator(*this, key));
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
  case MedianBoxBackground:
    processor = PostprocessorBackend::shared_pointer(new pp203(*this,key));
    break;
  case BraggPeakSNR:
    processor = PostprocessorBackend::shared_pointer(new pp204(*this,key));
    break;
  case DrawPeaks:
    processor = PostprocessorBackend::shared_pointer(new pp205(*this,key));
    break;
  case BraggPeakThreshold:
    processor = PostprocessorBackend::shared_pointer(new pp206(*this,key));
    break;
  case ImageFromTable:
    processor = PostprocessorBackend::shared_pointer(new pp207(*this,key));
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
  case Autocorrelation:
      processor = PostprocessorBackend::shared_pointer (new pp310(*this, key));
      break;
  case Autocorrelation2:
      processor = PostprocessorBackend::shared_pointer (new pp311(*this, key));
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
  case ChetahConv:
    processor = PostprocessorBackend::shared_pointer
        (new pp1600(*this,key));
    break;
  case CoarseCsPadAligment:
    processor = PostprocessorBackend::shared_pointer
        (new pp1601(*this,key));
    break;
  case GeomFileCsPadAligment:
    processor = PostprocessorBackend::shared_pointer(new pp1602(*this,key));
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
