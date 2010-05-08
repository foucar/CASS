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

  void set1DHist(cass::Histogram1DFloat*& hist, size_t id)
  {
    //open the settings//
    QSettings param;
    param.beginGroup("PostProcessor");
    param.beginGroup(QString("p") + QString::number(id));
    //create new histogram using the parameters//
    std::cerr << "Creating 1D histogram with"
        <<" XNbrBins:"<<param.value("XNbrBins",1).toUInt()
        <<" XLow:"<<param.value("XLow",0).toFloat()
        <<" XUp:"<<param.value("XUp",0).toFloat()
        <<std::endl;
    hist = new cass::Histogram1DFloat(param.value("XNbrBins",1).toUInt(),
                                      param.value("XLow",0).toFloat(),
                                      param.value("XUp",0).toFloat());
  }

  void set2DHist(cass::Histogram2DFloat*& hist, size_t id)
  {
    //open the settings//
    QSettings param;
    param.beginGroup("PostProcessor");
    param.beginGroup(QString("p") + QString::number(id));
    //create new histogram using the parameters//
    std::cerr << "Creating 2D histogram with"
        <<" XNbrBins:"<<param.value("XNbrBins",1).toUInt()
        <<" XLow:"<<param.value("XLow",0).toFloat()
        <<" XUp:"<<param.value("XUp",0).toFloat()
        <<" YNbrBins:"<<param.value("YNbrBins",1).toUInt()
        <<" YLow:"<<param.value("YLow",0).toFloat()
        <<" YUp:"<<param.value("YUp",0).toFloat()
        <<std::endl;
    hist = new cass::Histogram2DFloat(param.value("XNbrBins",1).toUInt(),
                                      param.value("XLow",0).toFloat(),
                                      param.value("XUp",0).toFloat(),
                                      param.value("YNbrBins",1).toUInt(),
                                      param.value("YLow",0).toFloat(),
                                      param.value("YUp",0).toFloat());
  }


  using namespace cass::ACQIRIS;
  /** predicate class for find_if.
   * this helps finding the right key in the list of pairs
   * @see HelperAcqirisDetectors::_detectorList
   * @author Lutz Foucar
   */
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

  /** Helper for Acqiris related Postprocessors.
   * This class will return the requested detector, which signals are going to
   * a Acqiris Instrument. It is implemented as a singleton such that every postprocessor
   * can call it without knowing about it.
   * @todo make sure that the detectors are protected from beeing written
   *       while they are read from
   */
  class HelperAcqirisDetectors
  {
  public:
    /** static function creating instance of this.
     * create an instance of an helper for the requested detector.
     * if it doesn't exist already. Create the maps with analyzers
     * @note creating the list of analyzers might be more useful inside the constuctor. But
     *       then there would be a map for each detector.. We need to change this to
     *       let the detectors calculate the requested vaules lazly in the near future
     */
    static HelperAcqirisDetectors * instance(Detectors);
    /** destroy the whole helper*/
    static void destroy();
    /** retrieve detector for event.
     * after validating that the event for this detector exists,
     * return the detector from our list
     */
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
    /** validation of event.
     * validate whether we have already seen this event
     * if not than add a detector, that is copy constructed or
     * assigned from the detector this instance owns, to the list.
     * @return the pointer to this detector
     * @param evt the cass event to validate
     */
    DetectorBackend * validate(const CASSEvent &evt)
    {
      //lock this so that only one helper will retrieve the detector at a time//
      QMutexLocker lock(&_helperMutex);
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
        //copy the information of our detector to this detector//
        *det = *_detector;
//        std::cout<<"Acqiris Helper validate: our det mcp chan:"<< dynamic_cast<DelaylineDetector*>(_detector)->mcp().channelNbr()
//            <<" list det mcp chan:"<<dynamic_cast<DelaylineDetector*>(det)->mcp().channelNbr()
//            <<" our det u1 chan:"<<dynamic_cast<DelaylineDetector*>(_detector)->layers()['U'].wireend()['1'].channelNbr()
//            <<" list det u1 chan:"<<dynamic_cast<DelaylineDetector*>(det)->layers()['U'].wireend()['1'].channelNbr()
//            <<" our det ana type:"<<dynamic_cast<DelaylineDetector*>(_detector)->analyzerType()
//            <<" list det ana type:"<<dynamic_cast<DelaylineDetector*>(det)->analyzerType()
//            <<" our mcp ana type:"<<dynamic_cast<DelaylineDetector*>(_detector)->mcp().analyzerType()
//            <<" list mcp ana type:"<<dynamic_cast<DelaylineDetector*>(det)->mcp().analyzerType() <<std::endl;
        //process the detector using the detectors analyzers in a global container
        (*_detectoranalyzer[det->analyzerType()])(*det, *dev);
//        std::cout << "validate: "<<dynamic_cast<TofDetector*>(det)->mcp().peaks().size()
//            <<" analyzer type: "<<det->analyzerType()<<std::endl;
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
    /** list of pairs of id-detectors.
     * The contents are copy constructed from the detector that this helper instance owns.
     * Needs to be at least the size of workers that can possibly call this helper simultaniously,
     * but should be shrinked if it get much bigger than the nbr of workers
     */
    detectorList_t _detectorList;
    /** the detector that is belongs to this instance of the helper*/
    DetectorBackend *_detector;
  private:
    /** prevent people from constructin other than using instance().*/
    HelperAcqirisDetectors() {}
    /** private constructor.
     * create our instance of the detector depending on the detector type
     * and the list of detectors.
     */
    HelperAcqirisDetectors(Detectors);
    /** prevent copy-construction*/
    HelperAcqirisDetectors(const HelperAcqirisDetectors&);
    /** private desctuctor.
     * prevent destruction other than trhough destroy(),
     * delete the detector and the detectorlist for this instance
     */
    ~HelperAcqirisDetectors();
    /** prevent assingment */
    HelperAcqirisDetectors& operator=(const HelperAcqirisDetectors&);
    /** the helperclass instances.
     * the instances of this class put into map
     * one instance for each available detector
     */
    static std::map<Detectors,HelperAcqirisDetectors*> _instances;
    /** global list of analyzers for waveforms */
    static waveformanalyzer_t _waveformanalyzer;
    /** global list of analyzers for detectors */
    static detectoranalyzer_t _detectoranalyzer;
    /** Singleton Mutex to lock write operations*/
    static QMutex _mutex;
    /** Mutex for each helper*/
    QMutex _helperMutex;
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
    std::cout << "the list of waveform analyzers is empty, we need to inflate it"<<std::endl;
    _waveformanalyzer[cfd8]  = new CFD8Bit();
    _waveformanalyzer[cfd16] = new CFD16Bit();
    _waveformanalyzer[com8]  = new CoM8Bit();
    _waveformanalyzer[com16] = new CoM16Bit();
  }
  if (_detectoranalyzer.empty())
  {
    std::cout << "the list of detector analyzers is empty, we need to inflate it"<<std::endl;
    _detectoranalyzer[DelaylineSimple] = new DelaylineDetectorAnalyzerSimple(&_waveformanalyzer);
    _detectoranalyzer[ToFSimple] = new ToFAnalyzerSimple(&_waveformanalyzer);
  }
  //check if an instance of the helper class already exists//
  //return it, otherwise create one and return it//
  if (0 == _instances[dettype])
  {
    std::cout << "creating an instance of the Acqiris Detector Helper for detector type "<<dettype<<std::endl;
    _instances[dettype] = new HelperAcqirisDetectors(dettype);
  }
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
  std::cout << "AcqirisDetectorHelper constructor: we are responsible for det type "<<dettype<<", which name is ";
  using namespace cass::ACQIRIS;
  //create the detector
  //create the detector list with twice the amount of elements than workers
  switch(dettype)
  {
  case HexDetector:
    {
      std::cout <<"HexDetector"<<std::endl;
      _detector = new DelaylineDetector(Hex,"HexDetector");
      for (size_t i=0; i<NbrOfWorkers*2;++i)
        _detectorList.push_front(std::make_pair(0,new DelaylineDetector(Hex,"HexDetector")));
    }
    break;
  case QuadDetector:
    {
      std::cout <<"QuadDetector"<<std::endl;
      _detector = new DelaylineDetector(Quad,"QuadDetector");
      for (size_t i=0; i<NbrOfWorkers*2;++i)
        _detectorList.push_front(std::make_pair(0,new DelaylineDetector(Quad,"QuadDetector")));
    }
    break;
  case VMIMcp:
    {
      std::cout <<"VMIMcp"<<std::endl;
      _detector = new TofDetector("VMIMcp");
      for (size_t i=0; i<NbrOfWorkers*2;++i)
        _detectorList.push_front(std::make_pair(0,new TofDetector("VMIMcp")));
    }
    break;
  case FELBeamMonitor:
    {
      std::cout <<"Beamdump"<<std::endl;
      _detector = new TofDetector("FELBeamMonitor");
      for (size_t i=0; i<NbrOfWorkers*2;++i)
        _detectorList.push_front(std::make_pair(0,new TofDetector("FELBeamMonitor")));
    }
    break;
  case YAGPhotodiode:
    {
      std::cout <<"YAGPhotodiode"<<std::endl;
      _detector = new TofDetector("YAGPhotodiode");
      for (size_t i=0; i<NbrOfWorkers*2;++i)
        _detectorList.push_front(std::make_pair(0,new TofDetector("YAGPhotodiode")));
    }
    break;
  case FsPhotodiode:
    {
      std::cout <<"FsPhotodiode"<<std::endl;
      _detector = new TofDetector("FsPhotodiode");
      for (size_t i=0; i<NbrOfWorkers*2;++i)
        _detectorList.push_front(std::make_pair(0,new TofDetector("FsPhotodiode")));
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
  std::cout << "Acqiris Helper load Parameters: loading parameters of detector "<< _detector->name()<<std::endl;
  QSettings par;
//  par.beginGroup("postprocessors");
  par.beginGroup("AcqirisDetectors");
  _detector->loadParameters(&par);
  std::cout << "Acqiris Helper load Parameters: done loading for "<< _detector->name()<<std::endl;
}






//----------------Nbr of Peaks MCP---------------------------------------------
//-----------pp550, pp600, pp650, pp660, pp670, pp680--------------------------
cass::pp550::pp550(PostProcessors &pp, PostProcessors::id_t id)
  :cass::PostprocessorBackend(pp,id),
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
  case PostProcessors::FELBeamMonitorNbrSignals:
    _detector = FELBeamMonitor;break;
  case PostProcessors::YAGPhotodiodeNbrSignals:
    _detector = YAGPhotodiode;break;
  case PostProcessors::FsPhotodiodeNbrSignals:
    _detector = FsPhotodiode;break;
  default:
    throw std::invalid_argument(QString("postprocessor %1 is not for Nbr MCP Signals").arg(_id).toStdString());
  }
  //create the histogram by loading the settings//
  loadParameters(0);
}

cass::pp550::~pp550()
{
  _pp.histograms_delete(_id);
  _nbrSignals=0;
}

void cass::pp550::loadParameters(size_t)
{
  std::cout <<std::endl<< "load the parameters of postprocessor "<<_id
      <<" it histograms the Nbr of Mcp Peaks"
      <<" of detector "<<_detector<<std::endl;
  //create the histogram
  set1DHist(_nbrSignals,_id);
  _pp.histograms_replace(_id,_nbrSignals);
  //load the detectors settings
  HelperAcqirisDetectors::instance(_detector)->loadParameters();
  std::cout << "done loading postprocessor "<<_id<<"'s parameters"<<std::endl;
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
cass::pp551::pp551(PostProcessors &pp, PostProcessors::id_t id)
  :cass::PostprocessorBackend(pp,id),
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
    throw std::invalid_argument(QString("postprocessor %1 is not responsible for Nbr Anode Signals").arg(id).toStdString());
  }
  //create the histogram by loading the settings//
  loadParameters(0);
}

cass::pp551::~pp551()
{
  _pp.histograms_delete(_id);
  _nbrSignals=0;
}

void cass::pp551::loadParameters(size_t)
{
  std::cout <<std::endl<< "load the parameters of postprocessor "<<_id
      <<" it histograms the Nbr of Anode Layer Peaks "
      <<" of detector "<<_detector
      <<" layer "<<_layer
      <<" wireend "<<_signal<<std::endl;
  //create the histogram
  set1DHist(_nbrSignals,_id);
  _pp.histograms_replace(_id,_nbrSignals);
  //load the detectors settings
  HelperAcqirisDetectors::instance(_detector)->loadParameters();
  std::cout << "done loading postprocessor "<<_id<<"'s parameters"<<std::endl;
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
cass::pp557::pp557(PostProcessors &pp, PostProcessors::id_t id)
  :cass::PostprocessorBackend(pp,id),
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
    throw std::invalid_argument(QString("postprocessor %1 is not responsible for Ratio of Layers").arg(id).toStdString());
  }
  //create the histogram by loading the settings//
  loadParameters(0);
}

cass::pp557::~pp557()
{
  _pp.histograms_delete(_id);
  _ratio=0;
}

void cass::pp557::loadParameters(size_t)
{
  std::cout <<std::endl<< "load the parameters of postprocessor "<<_id
      <<" it histograms the Ratio of Anode Layer Peaks"
      <<" of detector "<<_detector
      <<" layer "<<_layer<<std::endl;
  //create the histogram
  set1DHist(_ratio,_id);
  _pp.histograms_replace(_id,_ratio);
  //load the detectors settings
  HelperAcqirisDetectors::instance(_detector)->loadParameters();
  std::cout << "done loading postprocessor "<<_id<<"'s parameters"<<std::endl;
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
cass::pp558::pp558(PostProcessors &pp, PostProcessors::id_t id)
  :cass::PostprocessorBackend(pp,id),
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
    throw std::invalid_argument(QString("postprocessor %1 is not responsible for Ratio of Signals vs. MCP").arg(id).toStdString());
  }
  //create the histogram by loading the settings//
  loadParameters(0);
}

cass::pp558::~pp558()
{
  _pp.histograms_delete(_id);
  _ratio=0;
}

void cass::pp558::loadParameters(size_t)
{
  std::cout <<std::endl<< "load the parameters of postprocessor "<<_id
      <<" it histograms the Ratio of Anode Layer wireend Peaks vs Mcp Peaks "<<_layer
      <<" of detector "<<_detector
      << "layer "<<_layer
      <<" wireend "<<_wireend<<std::endl;
  //create the histogram
  set1DHist(_ratio,_id);
  _pp.histograms_replace(_id,_ratio);
  //load the detectors settings
  HelperAcqirisDetectors::instance(_detector)->loadParameters();
  std::cout << "done loading postprocessor "<<_id<<"'s parameters"<<std::endl;
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
cass::pp566::pp566(PostProcessors &pp, PostProcessors::id_t id)
  :cass::PostprocessorBackend(pp,id),
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
    throw std::invalid_argument(QString("postprocessor %1 is not responsible for Ratio of reconstructed Hits vs MCP Hits").arg(id).toStdString());
  }
  //create the histogram by loading the settings//
  loadParameters(0);
}

cass::pp566::~pp566()
{
  _pp.histograms_delete(_id);
  _ratio=0;
}

void cass::pp566::loadParameters(size_t)
{
  std::cout <<std::endl<< "load the parameters of postprocessor "<<_id
      <<" it histograms the ratio of reconstructed hits vs. Mcp peaks"
      <<" of detector "<<_detector<<std::endl;
  //create the histogram
  set1DHist(_ratio,_id);
  _pp.histograms_replace(_id,_ratio);
  //load the detectors settings
  HelperAcqirisDetectors::instance(_detector)->loadParameters();
  std::cout << "done loading postprocessor "<<_id<<"'s parameters"<<std::endl;
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
//-------------pp567, pp612, pp651, pp661, pp671, pp681------------------------
cass::pp567::pp567(PostProcessors &pp, PostProcessors::id_t id)
  :cass::PostprocessorBackend(pp,id),
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
  case PostProcessors::FELBeamMonitorAllMcp:
    _detector = FELBeamMonitor;break;
  case PostProcessors::YAGPhotodiodeAllMcp:
    _detector = YAGPhotodiode;break;
  case PostProcessors::FsPhotodiodeAllMcp:
    _detector = FsPhotodiode;break;

  default:
    throw std::invalid_argument(QString("postprocessor %1 is not responsible for All MCP Hits").arg(id).toStdString());
  }
  //create the histogram by loading the settings//
  loadParameters(0);
}

cass::pp567::~pp567()
{
  _pp.histograms_delete(_id);
  _tof=0;
}

void cass::pp567::loadParameters(size_t)
{
  std::cout <<std::endl<< "load the parameters of postprocessor "<<_id
      <<" it histograms times of the found MCP Hits"
      <<" of detector "<<_detector<<std::endl;
  //create the histogram
  set1DHist(_tof,_id);
  _pp.histograms_replace(_id,_tof);
  //load the detectors settings
  HelperAcqirisDetectors::instance(_detector)->loadParameters();
  std::cout << "done loading postprocessor "<<_id<<"'s parameters"<<std::endl;
}

void cass::pp567::operator()(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  //get right filled detector from the helper
  TofDetector *det =
      dynamic_cast<TofDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt));
  //reference to all found peaks of the mcp channel//
  Signal::peaks_t::const_iterator it = det->mcp().peaks().begin();
  //fill all found peaks into the histogram//
  for (; it != det->mcp().peaks().end(); ++it)
    _tof->fill(it->time());
}








//----------------Timesum for the layers---------------------------------------
//-----------pp568-570, pp613-614----------------------------------------------
cass::pp568::pp568(PostProcessors &pp, PostProcessors::id_t id)
  :cass::PostprocessorBackend(pp,id),
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
    throw std::invalid_argument(QString("postprocessor %1 is not responsible for Timesum").arg(id).toStdString());
  }
  //create the histogram by loading the settings//
  loadParameters(0);
}

cass::pp568::~pp568()
{
  _pp.histograms_delete(_id);
  _timesum=0;
}

void cass::pp568::loadParameters(size_t)
{
  std::cout <<std::endl<< "load the parameters of postprocessor "<<_id
      <<" it histograms the timesum of layer "<<_layer
      <<" of detector "<<_detector<<std::endl;
  //create the histogram
  set1DHist(_timesum,_id);
  _pp.histograms_replace(_id,_timesum);
  //load the detectors settings
  HelperAcqirisDetectors::instance(_detector)->loadParameters();
  std::cout << "done loading postprocessor "<<_id<<"'s parameters"<<std::endl;
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
cass::pp571::pp571(PostProcessors &pp, PostProcessors::id_t id)
  :cass::PostprocessorBackend(pp,id),
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
    throw std::invalid_argument(QString("postprocessor %1 is not responsible for Timesum vs. Pos").arg(id).toStdString());
  }
  //create the histogram by loading the settings//
  loadParameters(0);
}

cass::pp571::~pp571()
{
  _pp.histograms_delete(_id);
  _timesumvsPos=0;
}

void cass::pp571::loadParameters(size_t)
{
  std::cout <<std::endl<< "load the parameters of postprocessor "<<_id
      <<" it histograms the timesum vs Postion on layer "<<_layer
      <<" of detector "<<_detector<<std::endl;
  //create the histogram
  set2DHist(_timesumvsPos,_id);
  _pp.histograms_replace(_id,_timesumvsPos);
  //load the detectors settings
  HelperAcqirisDetectors::instance(_detector)->loadParameters();
  std::cout << "done loading postprocessor "<<_id<<"'s parameters"<<std::endl;
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
cass::pp574::pp574(PostProcessors &pp, PostProcessors::id_t id)
  :cass::PostprocessorBackend(pp,id),
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
    throw std::invalid_argument(QString("postprocessor %1 is not responsible for Detector Picture of First Hit").arg(id).toStdString());
  }
  //create the histogram by loading the settings//
  loadParameters(0);
}

cass::pp574::~pp574()
{
  _pp.histograms_delete(_id);
  _pos=0;
}

void cass::pp574::loadParameters(size_t)
{
  std::cout <<std::endl<< "load the parameters of postprocessor "<<_id
      <<" it histograms a detector picture of the first Hit on the detector created"
      <<" from  Layers "<<_first
      << " and "<<_second
      <<" of detector "<<_detector<<std::endl;
  //create the histogram
  set2DHist(_pos,_id);
  _pp.histograms_replace(_id,_pos);
  //load the detectors settings
  HelperAcqirisDetectors::instance(_detector)->loadParameters();
  std::cout << "done loading postprocessor "<<_id<<"'s parameters"<<std::endl;
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
cass::pp578::pp578(PostProcessors &pp, PostProcessors::id_t id)
  :cass::PostprocessorBackend(pp,id),
  _hist(0)

{
  //find out which detector and Signal we should work on
  switch (_id)
  {
  case PostProcessors::HexXY:
    _detector = HexDetector; _first = 'x'; _second = 'y'; _third = 't'; break;
  case PostProcessors::HexXT:
    _detector = HexDetector; _first = 't'; _second = 'x'; _third = 'y'; break;
  case PostProcessors::HexYT:
    _detector = HexDetector; _first = 't'; _second = 'y'; _third = 'x'; break;

  case PostProcessors::QuadXY:
    _detector = QuadDetector; _first = 'x'; _second = 'y'; _third = 't'; break;
  case PostProcessors::QuadXT:
    _detector = QuadDetector; _first = 't'; _second = 'x'; _third = 'y'; break;
  case PostProcessors::QuadYT:
    _detector = QuadDetector; _first = 't'; _second = 'y'; _third = 'x'; break;

  default:
    throw std::invalid_argument(QString("postprocessor %1 is not responsible for Detector Values").arg(id).toStdString());
  }
  //create the histogram by loading the settings//
  loadParameters(0);
}

cass::pp578::~pp578()
{
  _pp.histograms_delete(_id);
  _hist=0;
}

void cass::pp578::loadParameters(size_t)
{
  QSettings param;
  param.beginGroup("PostProcessor");
  param.beginGroup(QString("p") + QString::number(_id));
  //load the condition on the third component//
  float f = param.value("ConditionLow",-50000.).toFloat();
  float s = param.value("ConditionHigh",50000.).toFloat();
  param.endGroup();
  //make sure that the first value of the condition is the lower and second the higher value//
  _condition.first = (f<=s)?f:s;
  _condition.second = (s>f)?s:f;
  //tell the user what is loaded//

  std::cout <<std::endl<< "load the parameters of postprocessor "<<_id
      <<" it histograms the Property "<<_second
      <<" vs. "<<_first
      <<" of the reconstructed Detectorhits of detector "<<_detector
      <<" condition Low "<<_condition.first
      <<" High "<<_condition.second
      <<" on Property "<< _third
      <<std::endl;
  //create the histogram
  set2DHist(_hist,_id);
  _pp.histograms_replace(_id,_hist);
  //load the detectors settings
  HelperAcqirisDetectors::instance(_detector)->loadParameters();
  std::cout << "done loading postprocessor "<<_id<<"'s parameters"<<std::endl;
}

void cass::pp578::operator()(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  //get right filled detector from the helper
  DelaylineDetector *det =
      dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt));
  //get iterator to the hits//
  DelaylineDetector::dethits_t::iterator it = det->hits().begin();
//  std::cout << det->hits().size()<<std::endl;
  //go through all hits of the detector//
  for (; it != det->hits().end(); ++it)
  {
//    std::cout
//        <<" "<<_first<<":"<<it->values()[_first]
//        <<" "<<_second<<":"<<it->values()[_second]
//        <<" "<<_third<<":"<<it->values()[_third]
//        <<std::endl;
    if (_condition.first < it->values()[_third] && it->values()[_third] < _condition.second)
      _hist->fill(it->values()[_first],it->values()[_second]);
  }
}








//----------------MCP Fwhm vs. height------------------------------------------
//-------------pp581, pp621, pp652, pp662, pp672, pp682------------------------
cass::pp581::pp581(PostProcessors &pp, PostProcessors::id_t id)
  :cass::PostprocessorBackend(pp,id),
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
  case PostProcessors::FELBeamMonitorHeightvsFwhmMcp:
    _detector = FELBeamMonitor;break;
  case PostProcessors::YAGPhotodiodeHeightvsFwhmMcp:
    _detector = YAGPhotodiode;break;
  case PostProcessors::FsPhotodiodeHeightvsFwhmMcp:
    _detector = FsPhotodiode;break;

  default:
    throw std::invalid_argument(QString("postprocessor %1 is not responsible for FWHM vs. Height of MCP").arg(id).toStdString());
  }
  //create the histogram by loading the settings//
  loadParameters(0);
}

cass::pp581::~pp581()
{
  _pp.histograms_delete(_id);
  _sigprop=0;
}

void cass::pp581::loadParameters(size_t)
{
  std::cout <<std::endl<< "load the parameters of postprocessor "<<_id
      <<" it histograms the FWHM vs the height of the found MCP Peaks"
      <<" of  detector "<<_detector<<std::endl;
  //create the histogram
  set2DHist(_sigprop,_id);
  _pp.histograms_replace(_id,_sigprop);
  //load the detectors settings
  HelperAcqirisDetectors::instance(_detector)->loadParameters();
  std::cout << "done loading postprocessor "<<_id<<"'s parameters"<<std::endl;
}

void cass::pp581::operator()(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  //get right filled detector from the helper
  TofDetector *det =
      dynamic_cast<TofDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt));
  //reference to all found peaks of the mcp channel//
  Signal::peaks_t::const_iterator it = det->mcp().peaks().begin();
  //fill all found peaks into the histogram//
//  std::cout<<det->mcp().peaks().size()<<std::endl;

  for (;it != det->mcp().peaks().end(); ++it)
    _sigprop->fill(it->fwhm(),it->height());
}










//----------------FWHM vs. Height of Wireend Signals---------------------------
//-----------pp582-587, pp622-625----------------------------------------------
cass::pp582::pp582(PostProcessors &pp, PostProcessors::id_t id)
  :cass::PostprocessorBackend(pp,id),
  _sigprop(0)
{
  //find out which detector and Signal we should work on
  switch (_id)
  {
  case PostProcessors::HexHeightvsFwhmU1:
    _detector = HexDetector; _layer = 'U'; _signal = '1';break;
  case PostProcessors::HexHeightvsFwhmU2:
    _detector = HexDetector; _layer = 'U'; _signal = '2';break;
  case PostProcessors::HexHeightvsFwhmV1:
    _detector = HexDetector; _layer = 'V'; _signal = '1';break;
  case PostProcessors::HexHeightvsFwhmV2:
    _detector = HexDetector; _layer = 'V'; _signal = '2';break;
  case PostProcessors::HexHeightvsFwhmW1:
    _detector = HexDetector; _layer = 'W'; _signal = '1';break;
  case PostProcessors::HexHeightvsFwhmW2:
    _detector = HexDetector; _layer = 'W'; _signal = '2';break;

  case PostProcessors::QuadHeightvsFwhmX1:
    _detector = QuadDetector; _layer = 'X'; _signal = '1';break;
  case PostProcessors::QuadHeightvsFwhmX2:
    _detector = QuadDetector; _layer = 'X'; _signal = '2';break;
  case PostProcessors::QuadHeightvsFwhmY1:
    _detector = QuadDetector; _layer = 'Y'; _signal = '1';break;
  case PostProcessors::QuadHeightvsFwhmY2:
    _detector = QuadDetector; _layer = 'Y'; _signal = '2';break;

  default:
    throw std::invalid_argument(QString("postprocessor %1 is not responsible for FWHM vs. Height of Layer Signals").arg(id).toStdString());
  }
  //create the histogram by loading the settings//
  loadParameters(0);
}

cass::pp582::~pp582()
{
  _pp.histograms_delete(_id);
  _sigprop=0;
}

void cass::pp582::loadParameters(size_t)
{
  std::cout <<std::endl<< "load the parameters of postprocessor "<<_id
      <<" it histograms the FWHM vs the height of layer "<<_layer
      << " wireend "<<_signal
      <<" of detector "<<_detector<<std::endl;
  //create the histogram
  set2DHist(_sigprop,_id);
  _pp.histograms_replace(_id,_sigprop);
    //load the detectors settings
  HelperAcqirisDetectors::instance(_detector)->loadParameters();
  std::cout << "done loading postprocessor "<<_id<<"'s parameters"<<std::endl;
}

void cass::pp582::operator()(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  //get right filled detector from the helper
  DelaylineDetector *det =
      dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt));
  //go through all peaks of the wireend//
  //reference to all found peaks of the wireend channel//
  Signal::peaks_t::const_iterator it = det->layers()[_layer].wireend()[_signal].peaks().begin();
  //fill all found peaks into the histogram//
  for (; it != det->layers()[_layer].wireend()[_signal].peaks().end(); ++it)
    _sigprop->fill(it->fwhm(),it->height());
}










//----------------PIPICO-------------------------------------------------------
//-----------pp700-701---------------------------------------------------------
cass::pp700::pp700(PostProcessors &pp, PostProcessors::id_t id)
  :cass::PostprocessorBackend(pp,id),
  _pipico(0)
{
  //find out which detector and Signal we should work on
  switch (_id)
  {
  case PostProcessors::HexPIPICO:
    _detector01 = HexDetector; _detector02 = HexDetector; break;
  case PostProcessors::HexQuadPIPICO:
    _detector01 = HexDetector; _detector02 = QuadDetector; break;

  default:
    throw std::invalid_argument(QString("postprocessor %1 is not responsible for FWHM vs. Height of Layer Signals").arg(id).toStdString());
  }
  //create the histogram by loading the settings//
  loadParameters(0);
}

cass::pp700::~pp700()
{
  _pp.histograms_delete(_id);
  _pipico=0;
}

void cass::pp700::loadParameters(size_t)
{
  std::cout <<std::endl<< "load the parameters of postprocessor "<<_id
      <<" it create a PIPICO Histogram"
      <<" of detectors "<<_detector01
      <<" and "<<_detector02<<std::endl;
  //create the histogram
  set2DHist(_pipico,_id);
  _pp.histograms_replace(_id,_pipico);
    //load the detectors settings
  HelperAcqirisDetectors::instance(_detector01)->loadParameters();
  HelperAcqirisDetectors::instance(_detector02)->loadParameters();
  std::cout << "done loading postprocessor "<<_id<<"'s parameters"<<std::endl;
}

void cass::pp700::operator()(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  //get first detector from the helper
  TofDetector *det01 =
      dynamic_cast<TofDetector*>(HelperAcqirisDetectors::instance(_detector01)->detector(evt));
  //get second detector from the helper
  TofDetector *det02 =
      dynamic_cast<TofDetector*>(HelperAcqirisDetectors::instance(_detector02)->detector(evt));
  //get iterator of the peaks in the first detector//
  Signal::peaks_t::const_iterator it01(det01->mcp().peaks().begin());
  //draw all found hits vs another//
  for (; it01 != det01->mcp().peaks().end();++it01)
  {
    //if both detectors are the same, then the second iterator should start
    //i+1, otherwise we will just draw all hits vs. all hits
    Signal::peaks_t::const_iterator it02((_detector01==_detector02) ?
                                         it01+1 :
                                         det02->mcp().peaks().begin());

    for (; it02 != det02->mcp().peaks().end(); ++it02)
    {
      _pipico->fill(it01->time(),it02->time());
    }
  }
}
