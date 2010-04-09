//Copyright (C) 2010 Lutz Foucar

#include <stdexcept>
#include <cmath>
#include <algorithm>

#include <QtCore/QSettings>
#include <QtCore/QString>

#include "acqiris_detectors.h"
#include "histogram.h"
#include "cass_event.h"
#include "acqiris_device.h"
#include "com.h"
#include "cfd.h"
#include "delayline_detector_analyzer_simple.h"
#include "delayline_detector.h"
#include "tof_analyzer_simple.h"
#include "tof_detector.h"

namespace cass
{
  /** function to set the 1d histogram properties from the cass.ini file
    @param[out] hist pointer to the 1D Histogram whos properties should be updated (will be deleted and created with new settings)
    @param[in] id the id of the postprocessor too look up in cass.ini
 */
  void set1DHist(cass::Histogram1DFloat*& hist, size_t id)
  {
    //delete old histogram//
    delete hist;
    //open the settings//
    QSettings param;
    param.beginGroup("postprocessors");
    param.beginGroup(QString("processor_") + QString::number(id));
    //create new histogram using the parameters//
    hist = new cass::Histogram1DFloat(param.value("XNbrBins",1).toUInt(),
                                      param.value("XLow",0).toFloat(),
                                      param.value("XUp",0).toFloat());
  }
  /** function to set the 2d histogram properties from the cass.ini file
  @param[out] hist pointer to the 2D Histogram whos properties should be updated (will be deleted and created with new settings)
  @param[in] id the id of the postprocessor too look up in cass.ini
  */
  void set2DHist(cass::Histogram2DFloat*& hist, size_t id)
  {
    //delete old histogram//
    delete hist;
    //open the settings//
    QSettings param;
    param.beginGroup("postprocessors");
    param.beginGroup(QString("processor_") + QString::number(id));
    //create new histogram using the parameters//
    hist = new cass::Histogram2DFloat(param.value("XNbrBins",1).toUInt(),
                                      param.value("XLow",0).toFloat(),
                                      param.value("XUp",0).toFloat(),
                                      param.value("YNbrBins",1).toUInt(),
                                      param.value("YLow",0).toFloat(),
                                      param.value("YUp",0).toFloat());
  }

  using namespace cass::ACQIRIS;
  /*! predicate class for finding the right key in the list of pairs
    @see HelperAcqirisDetectors::_detectorList
    @author Lutz Foucar*/
  class IsKey
  {
  public:
    /** initialize the key in the constructor*/
    IsKey(const uint64_t key):_key(key){}
    /** compares the first element of the pair to the key*/
    bool operator()(const std::pair<uint64_t,DetectorBackend*>& p)const
    { return (p.first == _key); }
  private:
    /** the key that we will compare to in the operator*/
    const uint64_t _key;
  };

  /*! @brief Helper for Acqiris related Postprocessors
  This class will return the requested detector, which signals are going to
  a Acqiris Instrument. It is implemented as a singleton such that every postprocessor
  can call it without knowing about it.
  */
  class HelperAcqirisDetectors
  {
  public:
    /** create an instance of an helper for the requested detector,
    if it doesn't exist already. Create the maps with analyzers
    @todo creating the list of analyzers might be more useful inside the constuctor. But
          then there would be a map for each detector.. We need to change this to
          let the detectors calculate the requested vaules lazly in the near future
  */
    static HelperAcqirisDetectors * instance(Detectors);
    /** destroy the whole helper*/
    static void destroy();
    /** after validating that the event for this detector exists,
    return the detector from our list*/
    DetectorBackend * detector(const CASSEvent& evt)  {return validate(evt);}
    /** tell the detector owned by this instance to reload its settings*/
    void loadParameters(size_t=0);
  protected:
    /** typdef defining the list of detectors for more readable code*/
    typedef std::list<std::pair<uint64_t, DetectorBackend*> > detectorList_t;
    /** typdef defining the list of detectoranalyzers*/
    typedef std::map<DetectorAnalyzers, DetectorAnalyzerBackend*> detectoranalyzer_t;
    /** typdef defining the list of waveformanalyzers*/
    typedef std::map<WaveformAnalyzers, WaveformAnalyzerBackend*> waveformanalyzer_t;
    /** validate whether we have already seen this event
    if not than add a detector, that is copy constructed or
    assigned from the detector this instance owns, to the list.
    return the pointer to this detector
    */
    DetectorBackend * validate(const CASSEvent &evt)
    {
      //find the pair containing the detector//
      detectorList_t::iterator it =
        std::find_if(_detectorList.begin(), _detectorList.end(), IsKey(evt.id()));
      //check wether id is not already on the list//
      if(_detectorList.end() == it)
      {
        //retrieve a pointer to the acqiris device//
        Device *dev =
            dynamic_cast<Device*>(evt.devices().find(cass::CASSEvent::Acqiris)->second);
        //take the last element and get the the detector from it//
        DetectorBackend* det = _detectorList.back().second;
        //copy the informtion of our detector to this detector//
        *det = *_detector;
        //process the detector using the detectors analyzers in a global contaxiner
        (*_detectoranalyzer[det->analyzerType()])(*det, *dev);
        //create a new key from the id with the reloaded detector
        detectorList_t::value_type newPair = std::make_pair(evt.id(),det);
        //put it to the beginning of the list//
        _detectorList.push_front(newPair);
        //erase the outdated element at the back//
        _detectorList.pop_back();
        //make the iterator pointing to the just added element of the list//
        it = _detectorList.begin();
      }
      return it->second;
    }
  protected:
    /*! @brief list of pairs of id-detectors
    The contents are copy constructed from the detector that this helper instance owns.
    Needs to be at least the size of workers that can possibly call this helper simultaniously,
    but should be shrinked if it get much bigger than the nbr of workers*/
    detectorList_t _detectorList;
    /** the detector that is belongs to this instance of the helper*/
    DetectorBackend *_detector;
  private:
    /** prevent people from constructin other than using instance().*/
    HelperAcqirisDetectors() {}
    /** create our instance of the detector depending on the detector type
        and the list of detectors */
    HelperAcqirisDetectors(Detectors);
    /** prevent copy-construction*/
    HelperAcqirisDetectors(const HelperAcqirisDetectors&);
    /** prevent destruction other than trhough destroy(),
    delete the detector and the detectorlist for this instance*/
    ~HelperAcqirisDetectors();
    /** prevent assingment */
    HelperAcqirisDetectors& operator=(const HelperAcqirisDetectors&);
    /** the instances of this class put into map
    one instance for each available detector*/
    static std::map<Detectors,HelperAcqirisDetectors*> _instances;
    /** global list of analyzers for waveforms */
    static waveformanalyzer_t _waveformanalyzer;
    /** global list of analyzers for detectors */
    static detectoranalyzer_t _detectoranalyzer;
    /** Singleton Mutex to lock write operations*/
    static QMutex _mutex;
  };

}//end namespace


//--the helper--
//initialize static members//
std::map<cass::ACQIRIS::Detectors,cass::HelperAcqirisDetectors*>
    cass::HelperAcqirisDetectors::_instances;
std::map<cass::ACQIRIS::DetectorAnalyzers, cass::ACQIRIS::DetectorAnalyzerBackend*>
    cass::HelperAcqirisDetectors::_detectoranalyzer;
std::map<cass::ACQIRIS::WaveformAnalyzers, cass::ACQIRIS::WaveformAnalyzerBackend*>
    cass::HelperAcqirisDetectors::_waveformanalyzer;
QMutex cass::HelperAcqirisDetectors::_mutex;

cass::HelperAcqirisDetectors* cass::HelperAcqirisDetectors::instance(cass::ACQIRIS::Detectors dettype)
{
  //lock this//
  QMutexLocker lock(&_mutex);
  using namespace cass::ACQIRIS;
  //if the maps with the analyzers are empty, fill them//
  if (_waveformanalyzer.empty())
  {
    _waveformanalyzer[cfd8]  = new CFD8Bit();
    _waveformanalyzer[cfd16] = new CFD16Bit();
    _waveformanalyzer[com8]  = new CoM8Bit();
    _waveformanalyzer[com16] = new CoM16Bit();
  }
  if (_detectoranalyzer.empty())
  {
    _detectoranalyzer[DelaylineSimple] = new DelaylineDetectorAnalyzerSimple(&_waveformanalyzer);
    _detectoranalyzer[ToFSimple] = new ToFAnalyzerSimple(&_waveformanalyzer);
  }
  //check if an instance of the helper class already exists//
  //return it, otherwise create one and return it//
  if (0 == _instances[dettype])
    _instances[dettype] = new HelperAcqirisDetectors(dettype);
  return _instances[dettype];
}

void cass::HelperAcqirisDetectors::destroy()
{
  //lock this//
  QMutexLocker lock(&_mutex);
  //delete all instances of the helper class//
  for (std::map<ACQIRIS::Detectors,HelperAcqirisDetectors*>::iterator it=_instances.begin();
       it != _instances.end();
       ++it)
    delete it->second;
  //destroy all analyzers//
  for (waveformanalyzer_t::iterator it=_waveformanalyzer.begin();
       it != _waveformanalyzer.end();
       ++it)
    delete it->second;
  for (detectoranalyzer_t::iterator it=_detectoranalyzer.begin();
       it != _detectoranalyzer.end();
       ++it)
    delete it->second;
}

cass::HelperAcqirisDetectors::HelperAcqirisDetectors(cass::ACQIRIS::Detectors dettype)
{
  using namespace cass::ACQIRIS;
  //create the detector
  //create the detector list with twice the amount of elements than workers
  switch(dettype)
  {
  case HexDetector:
    {
      _detector = new DelaylineDetector(Hex,"HexDetector");
      for (size_t i=0; i<NbrOfWorkers*2;++i)
        _detectorList.push_front(std::make_pair(0,new DelaylineDetector(Hex,"HexDetector")));
    }
  case QuadDetector:
    {
      _detector = new DelaylineDetector(Quad,"QuadDetector");
      for (size_t i=0; i<NbrOfWorkers*2;++i)
        _detectorList.push_front(std::make_pair(0,new DelaylineDetector(Quad,"QuadDetector")));
    }
  case VMIMcp:
    {
      _detector = new TofDetector("VMIMcp");
      for (size_t i=0; i<NbrOfWorkers*2;++i)
        _detectorList.push_front(std::make_pair(0,new TofDetector("VMIMcp")));
    }
  case IntensityMonitor:
    {
      _detector = new TofDetector("IntensityMonitor");
      for (size_t i=0; i<NbrOfWorkers*2;++i)
        _detectorList.push_front(std::make_pair(0,new TofDetector("IntensityMonitor")));
    }
  case Photodiode:
    {
      _detector = new TofDetector("Photodiode");
      for (size_t i=0; i<NbrOfWorkers*2;++i)
        _detectorList.push_front(std::make_pair(0,new TofDetector("Photodiode")));
    }
    break;
  default: throw std::invalid_argument("no such detector is present");
  }
}

cass::HelperAcqirisDetectors::~HelperAcqirisDetectors()
{
  //delete the detectorList
  for (detectorList_t::iterator it=_detectorList.begin();
       it != _detectorList.end();
       ++it)
    delete it->second;
  //delete the detector
  delete _detector;
}

void cass::HelperAcqirisDetectors::loadParameters(size_t)
{
  QSettings par;
  par.beginGroup("postprocessors");
  par.beginGroup("AcqirisDetectors");
  _detector->loadParameters(&par);
}






//----------------Nbr of Peaks MCP---------------------------------------------
//-----------pp550, pp600, pp650, pp660, pp670---------------------------------
cass::pp550::pp550(PostProcessors::histograms_t &hist, PostProcessors::id_t id)
  :cass::PostprocessorBackend(hist,id),
  _nbrSignals(0)
{
  //find out which detector and Signal we should work on
  switch (_id)
  {
  case PostProcessors::HexMCPNbrSignals:
    _detector = HexDetector;break;
  case PostProcessors::QuadMCPNbrSignals:
    _detector = QuadDetector;break;
  case PostProcessors::VMIMcpNbrSignals:
    _detector = VMIMcp;break;
  case PostProcessors::IntensityMonitorNbrSignals:
    _detector = IntensityMonitor;break;
  case PostProcessors::PhotodiodeNbrSignals:
    _detector = Photodiode;break;
  default:
    throw std::invalid_argument("this postprocessor is not responsible for Nbr Signals Postprocessor");
  }
  //create the histogram by loading the settings//
  loadParameters(0);
}

cass::pp550::~pp550()
{
  delete _nbrSignals;
  _nbrSignals=0;
  _histograms[_id] =  _nbrSignals;
}

void cass::pp550::loadParameters(size_t)
{
  //create the histogram
  set1DHist(_nbrSignals,_id);
  _histograms[_id] =  _nbrSignals;
  //load the detectors settings
  HelperAcqirisDetectors::instance(_detector)->loadParameters();
}

void cass::pp550::operator()(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  //get right filled detector from the helper
  TofDetector *det =
      dynamic_cast<TofDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt));
  _nbrSignals->fill(det->mcp().peaks().size());
}





//----------------Nbr of Peaks Anode-------------------------------------------
//-----------pp551 - pp556 & pp601 - 604---------------------------------------
cass::pp551::pp551(PostProcessors::histograms_t &hist, PostProcessors::id_t id)
  :cass::PostprocessorBackend(hist,id),
  _nbrSignals(0)
{
  //find out which detector and Signal we should work on
  switch (_id)
  {
  case PostProcessors::HexU1NbrSignals:
    _detector = HexDetector; _layer = 'U'; _signal = '1';break;
  case PostProcessors::HexU2NbrSignals:
    _detector = HexDetector; _layer = 'U'; _signal = '2';break;
  case PostProcessors::HexV1NbrSignals:
    _detector = HexDetector; _layer = 'V'; _signal = '1';break;
  case PostProcessors::HexV2NbrSignals:
    _detector = HexDetector; _layer = 'V'; _signal = '2';break;
  case PostProcessors::HexW1NbrSignals:
    _detector = HexDetector; _layer = 'W'; _signal = '1';break;
  case PostProcessors::HexW2NbrSignals:
    _detector = HexDetector; _layer = 'W'; _signal = '2';break;

  case PostProcessors::QuadX1NbrSignals:
    _detector = QuadDetector; _layer = 'X'; _signal = '1';break;
  case PostProcessors::QuadX2NbrSignals:
    _detector = QuadDetector; _layer = 'X'; _signal = '2';break;
  case PostProcessors::QuadY1NbrSignals:
    _detector = QuadDetector; _layer = 'Y'; _signal = '1';break;
  case PostProcessors::QuadY2NbrSignals:
    _detector = QuadDetector; _layer = 'Y'; _signal = '2';break;

  default:
    throw std::invalid_argument("id is not responsible for Nbr Signals Postprocessor");
  }
  //create the histogram by loading the settings//
  loadParameters(0);
}

cass::pp551::~pp551()
{
  delete _nbrSignals;
  _nbrSignals=0;
  _histograms[_id] =  _nbrSignals;
}

void cass::pp551::loadParameters(size_t)
{
  //create the histogram
  set1DHist(_nbrSignals,_id);
  _histograms[_id] =  _nbrSignals;
  //load the detectors settings
  HelperAcqirisDetectors::instance(_detector)->loadParameters();
}

void cass::pp551::operator()(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  //get right filled detector from the helper
  DelaylineDetector *det =
      dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt));
  _nbrSignals->fill(det->layers()[_layer].wireend()[_signal].peaks().size());
}






//----------------Ratio of Layers----------------------------------------------
//-----------pp557, pp560, pp563, pp605, pp608---------------------------------
cass::pp557::pp557(PostProcessors::histograms_t &hist, PostProcessors::id_t id)
  :cass::PostprocessorBackend(hist,id),
  _ratio(0)
{
  //find out which detector and Signal we should work on
  switch (_id)
  {
  case PostProcessors::HexU1U2Ratio:
    _detector = HexDetector; _layer = 'U';break;
  case PostProcessors::HexV1V2Ratio:
    _detector = HexDetector; _layer = 'V';break;
  case PostProcessors::HexW1W2Ratio:
    _detector = HexDetector; _layer = 'W';break;

  case PostProcessors::QuadX1X2Ratio:
    _detector = QuadDetector; _layer = 'X';break;
  case PostProcessors::QuadY1Y2Ratio:
    _detector = QuadDetector; _layer = 'X';break;

  default:
    throw std::invalid_argument("id is not responsible for Nbr Signals Postprocessor");
  }
  //create the histogram by loading the settings//
  loadParameters(0);
}

cass::pp557::~pp557()
{
  delete _ratio;
  _ratio=0;
  _histograms[_id] =  _ratio;
}

void cass::pp557::loadParameters(size_t)
{
  //create the histogram
  set1DHist(_ratio,_id);
  _histograms[_id] =  _ratio;
  //load the detectors settings
  HelperAcqirisDetectors::instance(_detector)->loadParameters();
}

void cass::pp557::operator()(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  //get right filled detector from the helper
  DelaylineDetector *det =
      dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt));
  const float one = det->layers()[_layer].wireend()['1'].peaks().size();
  const float two = det->layers()[_layer].wireend()['2'].peaks().size();
  _ratio->fill(one/two);
}







//----------------Ratio of Signals vs. MCP-------------------------------------
//-----------pp558-559, pp561-562, pp564-565, pp606-607, pp609-610-------------
cass::pp558::pp558(PostProcessors::histograms_t &hist, PostProcessors::id_t id)
  :cass::PostprocessorBackend(hist,id),
  _ratio(0)
{
  //find out which detector and Signal we should work on
  switch (_id)
  {
  case PostProcessors::HexU1McpRatio:
    _detector = HexDetector; _layer = 'U'; _wireend = '1';break;
  case PostProcessors::HexU2McpRatio:
    _detector = HexDetector; _layer = 'U'; _wireend = '2';break;
  case PostProcessors::HexV1McpRatio:
    _detector = HexDetector; _layer = 'V'; _wireend = '1';break;
  case PostProcessors::HexV2McpRatio:
    _detector = HexDetector; _layer = 'V'; _wireend = '2';break;
  case PostProcessors::HexW1McpRatio:
    _detector = HexDetector; _layer = 'W'; _wireend = '1';break;
  case PostProcessors::HexW2McpRatio:
    _detector = HexDetector; _layer = 'W'; _wireend = '2';break;

  case PostProcessors::QuadX1McpRatio:
    _detector = QuadDetector; _layer = 'X'; _wireend = '1';break;
  case PostProcessors::QuadX2McpRatio:
    _detector = QuadDetector; _layer = 'X'; _wireend = '2';break;
  case PostProcessors::QuadY1McpRatio:
    _detector = QuadDetector; _layer = 'Y'; _wireend = '1';break;
  case PostProcessors::QuadY2McpRatio:
    _detector = QuadDetector; _layer = 'Y'; _wireend = '2';break;

  default:
    throw std::invalid_argument("id is not responsible for Nbr Signals Postprocessor");
  }
  //create the histogram by loading the settings//
  loadParameters(0);
}

cass::pp558::~pp558()
{
  delete _ratio;
  _ratio=0;
  _histograms[_id] =  _ratio;
}

void cass::pp558::loadParameters(size_t)
{
  //create the histogram
  set1DHist(_ratio,_id);
  _histograms[_id] =  _ratio;
  //load the detectors settings
  HelperAcqirisDetectors::instance(_detector)->loadParameters();
}

void cass::pp558::operator()(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  //get right filled detector from the helper
  DelaylineDetector *det =
      dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt));
  const float wireend = det->layers()[_layer].wireend()[_wireend].peaks().size();
  const float mcp = det->mcp().peaks().size();
  _ratio->fill(wireend/mcp);
}








//----------------Ratio of rec. Hits vs. MCP Hits------------------------------
//------------------------------pp566, pp611-----------------------------------
cass::pp566::pp566(PostProcessors::histograms_t &hist, PostProcessors::id_t id)
  :cass::PostprocessorBackend(hist,id),
  _ratio(0)
{
  //find out which detector and Signal we should work on
  switch (_id)
  {
  case PostProcessors::HexRekMcpRatio:
    _detector = HexDetector;break;
  case PostProcessors::QuadRekMcpRatio:
    _detector = QuadDetector;break;

  default:
    throw std::invalid_argument("id is not responsible for Nbr Signals Postprocessor");
  }
  //create the histogram by loading the settings//
  loadParameters(0);
}

cass::pp566::~pp566()
{
  delete _ratio;
  _ratio=0;
  _histograms[_id] =  _ratio;
}

void cass::pp566::loadParameters(size_t)
{
  //create the histogram
  set1DHist(_ratio,_id);
  _histograms[_id] =  _ratio;
  //load the detectors settings
  HelperAcqirisDetectors::instance(_detector)->loadParameters();
}

void cass::pp566::operator()(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  //get right filled detector from the helper
  DelaylineDetector *det =
      dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt));
  const float rek = det->hits().size();
  const float mcp = det->mcp().peaks().size();
  _ratio->fill(rek/mcp);
}









//----------------MCP Hits (Tof)-----------------------------------------------
//-------------pp567, pp612, pp651, pp661, pp671-------------------------------
cass::pp567::pp567(PostProcessors::histograms_t &hist, PostProcessors::id_t id)
  :cass::PostprocessorBackend(hist,id),
  _tof(0)
{
  //find out which detector and Signal we should work on
  switch (_id)
  {
  case PostProcessors::HexAllMcp:
    _detector = HexDetector;break;
  case PostProcessors::QuadAllMcp:
    _detector = QuadDetector;break;
  case PostProcessors::VMIMcpAllMcp:
    _detector = VMIMcp;break;
  case PostProcessors::IntensityMonitorAllMcp:
    _detector = IntensityMonitor;break;
  case PostProcessors::PhotodiodeAllMcp:
    _detector = Photodiode;break;

  default:
    throw std::invalid_argument("id is not responsible for Nbr Signals Postprocessor");
  }
  //create the histogram by loading the settings//
  loadParameters(0);
}

cass::pp567::~pp567()
{
  delete _tof;
  _tof=0;
  _histograms[_id] =  _tof;
}

void cass::pp567::loadParameters(size_t)
{
  //create the histogram
  set1DHist(_tof,_id);
  _histograms[_id] =  _tof;
  //load the detectors settings
  HelperAcqirisDetectors::instance(_detector)->loadParameters();
}

void cass::pp567::operator()(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  //get right filled detector from the helper
  TofDetector *det =
      dynamic_cast<TofDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt));
  //reference to all found peaks of the mcp channel//
  const Signal::peaks_t &mcpSignals = det->mcp().peaks();
  //fill all found peaks into the histogram//
  for (Signal::peaks_t::const_iterator it = mcpSignals.begin();
       it != mcpSignals.end();
       ++it)
    _tof->fill(it->time());
}








//----------------Timesum for the layers---------------------------------------
//-----------pp568-570, pp613-614----------------------------------------------
cass::pp568::pp568(PostProcessors::histograms_t &hist, PostProcessors::id_t id)
  :cass::PostprocessorBackend(hist,id),
  _timesum(0)
{
  //find out which detector and Signal we should work on
  switch (_id)
  {
  case PostProcessors::HexTimesumU:
    _detector = HexDetector; _layer = 'U'; break;
  case PostProcessors::HexTimesumV:
    _detector = HexDetector; _layer = 'V'; break;
  case PostProcessors::HexTimesumW:
    _detector = HexDetector; _layer = 'W'; break;

  case PostProcessors::QuadTimesumX:
    _detector = QuadDetector; _layer = 'X'; break;
  case PostProcessors::QuadTimesumY:
    _detector = QuadDetector; _layer = 'Y'; break;

  default:
    throw std::invalid_argument("id is not responsible for Nbr Signals Postprocessor");
  }
  //create the histogram by loading the settings//
  loadParameters(0);
}

cass::pp568::~pp568()
{
  delete _timesum;
  _timesum=0;
  _histograms[_id] =  _timesum;
}

void cass::pp568::loadParameters(size_t)
{
  //create the histogram
  set1DHist(_timesum,_id);
  _histograms[_id] =  _timesum;
  //load the detectors settings
  HelperAcqirisDetectors::instance(_detector)->loadParameters();
}

void cass::pp568::operator()(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  //get right filled detector from the helper
  DelaylineDetector *det =
      dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt));
  _timesum->fill(det->timesum(_layer));
}









//----------------Timesum vs Postition for the layers--------------------------
//-----------pp571-573, pp615-616----------------------------------------------
cass::pp571::pp571(PostProcessors::histograms_t &hist, PostProcessors::id_t id)
  :cass::PostprocessorBackend(hist,id),
  _timesumvsPos(0)
{
  //find out which detector and Signal we should work on
  switch (_id)
  {
  case PostProcessors::HexTimesumUvsU:
    _detector = HexDetector; _layer = 'U'; break;
  case PostProcessors::HexTimesumVvsV:
    _detector = HexDetector; _layer = 'V'; break;
  case PostProcessors::HexTimesumWvsW:
    _detector = HexDetector; _layer = 'W'; break;

  case PostProcessors::QuadTimesumXvsX:
    _detector = QuadDetector; _layer = 'X'; break;
  case PostProcessors::QuadTimesumYvsY:
    _detector = QuadDetector; _layer = 'Y'; break;

  default:
    throw std::invalid_argument("id is not responsible for Nbr Signals Postprocessor");
  }
  //create the histogram by loading the settings//
  loadParameters(0);
}

cass::pp571::~pp571()
{
  delete _timesumvsPos;
  _timesumvsPos=0;
  _histograms[_id] =  _timesumvsPos;
}

void cass::pp571::loadParameters(size_t)
{
  //create the histogram
  set2DHist(_timesumvsPos,_id);
  _histograms[_id] =  _timesumvsPos;
  //load the detectors settings
  HelperAcqirisDetectors::instance(_detector)->loadParameters();
}

void cass::pp571::operator()(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  //get right filled detector from the helper
  DelaylineDetector *det =
      dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt));
  _timesumvsPos->fill(det->position(_layer),det->timesum(_layer));
}










//----------------Detector First Hit-------------------------------------------
//-----------pp574-577, pp617--------------------------------------------------
cass::pp574::pp574(PostProcessors::histograms_t &hist, PostProcessors::id_t id)
  :cass::PostprocessorBackend(hist,id),
  _pos(0)
{
  //find out which detector and Signal we should work on
  switch (_id)
  {
  case PostProcessors::HexFirstUV:
    _detector = HexDetector; _first = 'U'; _second = 'V'; break;
  case PostProcessors::HexFirstUW:
    _detector = HexDetector; _first = 'U'; _second = 'W'; break;
  case PostProcessors::HexFirstVW:
    _detector = HexDetector; _first = 'V'; _second = 'W'; break;

  case PostProcessors::QuadFirstXY:
    _detector = QuadDetector; _first = 'X'; _second = 'Y'; break;

  default:
    throw std::invalid_argument("id is not responsible for Nbr Signals Postprocessor");
  }
  //create the histogram by loading the settings//
  loadParameters(0);
}

cass::pp574::~pp574()
{
  delete _pos;
  _pos=0;
  _histograms[_id] =  _pos;
}

void cass::pp574::loadParameters(size_t)
{
  //create the histogram
  set2DHist(_pos,_id);
  _histograms[_id] =  _pos;
  //load the detectors settings
  HelperAcqirisDetectors::instance(_detector)->loadParameters();
}

void cass::pp574::operator()(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  //get right filled detector from the helper
  DelaylineDetector *det =
      dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt));
  //get the requested layers//
  AnodeLayer &f = det->layers()[_first];
  AnodeLayer &s = det->layers()[_second];
  //get the timesums for the layers//
  const double tsf = det->timesum(_first);
  const double tss = det->timesum(_second);
  //check timesum//
  const bool csf = (f.tsLow() < tsf && tsf < f.tsHigh());
  const bool css = (s.tsLow() < tss && tss < s.tsHigh());
  //only fill when timesum is fullfilled
  if (csf && css)
    _pos->fill(f.position(),s.position());
}











//----------------Detector Values----------------------------------------------
//-----------pp578-580, pp61-620-----------------------------------------------
cass::pp578::pp578(PostProcessors::histograms_t &hist, PostProcessors::id_t id)
  :cass::PostprocessorBackend(hist,id),
  _hist(0)

{
  //find out which detector and Signal we should work on
  switch (_id)
  {
  case PostProcessors::HexXY:
    _detector = HexDetector; _first = 'x'; _second = 'y'; break;
  case PostProcessors::HexXT:
    _detector = HexDetector; _first = 't'; _second = 'x'; break;
  case PostProcessors::HexYT:
    _detector = HexDetector; _first = 'x'; _second = 'y'; break;

  case PostProcessors::QuadXY:
    _detector = QuadDetector; _first = 'x'; _second = 'y'; break;
  case PostProcessors::QuadXT:
    _detector = QuadDetector; _first = 't'; _second = 'x'; break;
  case PostProcessors::QuadYT:
    _detector = QuadDetector; _first = 't'; _second = 'y'; break;

  default:
    throw std::invalid_argument("id is not responsible for Nbr Signals Postprocessor");
  }
  //create the histogram by loading the settings//
  loadParameters(0);
}

cass::pp578::~pp578()
{
  delete _hist;
  _hist=0;
  _histograms[_id] =  _hist;
}

void cass::pp578::loadParameters(size_t)
{
  //create the histogram
  set2DHist(_hist,_id);
  _histograms[_id] =  _hist;
  //load the detectors settings
  HelperAcqirisDetectors::instance(_detector)->loadParameters();
}

void cass::pp578::operator()(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  //get right filled detector from the helper
  DelaylineDetector *det =
      dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt));
  //get the hits//
  DelaylineDetector::dethits_t &hits = det->hits();
  //go through all hits of the detector//
  for (DelaylineDetector::dethits_t::iterator it = hits.begin();
       it != hits.end();
       ++it)
  {
    _hist->fill(it->values()[_first],it->values()[_second]);
  }
}








//----------------MCP Fwhm vs. height------------------------------------------
//-------------pp581, pp621, pp652, pp662, pp672-------------------------------
cass::pp581::pp581(PostProcessors::histograms_t &hist, PostProcessors::id_t id)
  :cass::PostprocessorBackend(hist,id),
  _sigprop(0)
{
  //find out which detector and Signal we should work on
  switch (_id)
  {
  case PostProcessors::HexHeightvsFwhmMcp:
    _detector = HexDetector;break;
  case PostProcessors::QuadHeightvsFwhmMcp:
    _detector = QuadDetector;break;
  case PostProcessors::VMIMcpHeightvsFwhmMcp:
    _detector = VMIMcp;break;
  case PostProcessors::IntensityMonitorHeightvsFwhmMcp:
    _detector = IntensityMonitor;break;
  case PostProcessors::PhotodiodeHeightvsFwhmMcp:
    _detector = Photodiode;break;

  default:
    throw std::invalid_argument("id is not responsible for Nbr Signals Postprocessor");
  }
  //create the histogram by loading the settings//
  loadParameters(0);
}

cass::pp581::~pp581()
{
  delete _sigprop;
  _sigprop=0;
  _histograms[_id] =  _sigprop;
}

void cass::pp581::loadParameters(size_t)
{
  //create the histogram
  set2DHist(_sigprop,_id);
  _histograms[_id] =  _sigprop;
  //load the detectors settings
  HelperAcqirisDetectors::instance(_detector)->loadParameters();
}

void cass::pp581::operator()(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  //get right filled detector from the helper
  TofDetector *det =
      dynamic_cast<TofDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt));
  //reference to all found peaks of the mcp channel//
  const Signal::peaks_t &mcpSignals = det->mcp().peaks();
  //fill all found peaks into the histogram//
  for (Signal::peaks_t::const_iterator it = mcpSignals.begin();
       it != mcpSignals.end();
       ++it)
    _sigprop->fill(it->fwhm(),it->height());
}










//----------------FWHM vs. Height of Wireend Signals---------------------------
//-----------pp582-587, pp622-625----------------------------------------------
cass::pp582::pp582(PostProcessors::histograms_t &hist, PostProcessors::id_t id)
  :cass::PostprocessorBackend(hist,id),
  _sigprop(0)
{
  //find out which detector and Signal we should work on
  switch (_id)
  {
  case PostProcessors::HexU1NbrSignals:
    _detector = HexDetector; _layer = 'U'; _signal = '1';break;
  case PostProcessors::HexU2NbrSignals:
    _detector = HexDetector; _layer = 'U'; _signal = '2';break;
  case PostProcessors::HexV1NbrSignals:
    _detector = HexDetector; _layer = 'V'; _signal = '1';break;
  case PostProcessors::HexV2NbrSignals:
    _detector = HexDetector; _layer = 'V'; _signal = '2';break;
  case PostProcessors::HexW1NbrSignals:
    _detector = HexDetector; _layer = 'W'; _signal = '1';break;
  case PostProcessors::HexW2NbrSignals:
    _detector = HexDetector; _layer = 'W'; _signal = '2';break;

  case PostProcessors::QuadX1NbrSignals:
    _detector = QuadDetector; _layer = 'X'; _signal = '1';break;
  case PostProcessors::QuadX2NbrSignals:
    _detector = QuadDetector; _layer = 'X'; _signal = '2';break;
  case PostProcessors::QuadY1NbrSignals:
    _detector = QuadDetector; _layer = 'Y'; _signal = '1';break;
  case PostProcessors::QuadY2NbrSignals:
    _detector = QuadDetector; _layer = 'Y'; _signal = '2';break;

  default:
    throw std::invalid_argument("id is not responsible for Nbr Signals Postprocessor");
  }
  //create the histogram by loading the settings//
  loadParameters(0);
}

cass::pp582::~pp582()
{
  delete _sigprop;
  _sigprop=0;
  _histograms[_id] =  _sigprop;
}

void cass::pp582::loadParameters(size_t)
{
  //create the histogram
  set2DHist(_sigprop,_id);
  _histograms[_id] =  _sigprop;
  //load the detectors settings
  HelperAcqirisDetectors::instance(_detector)->loadParameters();
}

void cass::pp582::operator()(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  //get right filled detector from the helper
  DelaylineDetector *det =
      dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt));
  //go through all peaks of the wireend//
  //reference to all found peaks of the wireend channel//
  const Signal::peaks_t &mcpSignals = det->layers()[_layer].wireend()[_signal].peaks();
  //fill all found peaks into the histogram//
  for (Signal::peaks_t::const_iterator it = mcpSignals.begin();
       it != mcpSignals.end();
       ++it)
    _sigprop->fill(it->fwhm(),it->height());
}










