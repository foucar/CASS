//Copyright (C) 2010 lmf

#include <stdexcept>
#include <cmath>
#include <algorithm>

#include <QtCore/QSettings>
#include <QtCore/QString>

#include "delayline.h"
#include "histogram.h"
#include "cass_event.h"
#include "acqiris_device.h"
#include "com.h"
#include "cfd.h"
#include "delayline_detector_analyzer_simple.h"
#include "delayline_detector.h"

namespace cass
{
  /** function to set the 1d histogram properties from the cass.ini file*/
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
  /** function to set the 2d histogram properties from the cass.ini file*/
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

  /*! Helper for Acqiris related Postprocessors
  This class will retrun the requested detector, which signals are going to
  a Acqiris Instrument. It is implemented as a singleton such that every postprocessor
  can call it without knowing about it.
  @todo how do I have only one container which contains all waveform analyzer and other analyzers,
        mabye we can make it such that we don't need them, since they are only functions. But we have
        to create pointers to classes since we need the choice... Therefore we need to make sure, that
        the analyzers don't contain to much info, such that we can easily create once function class
        for each detector / signal of the detector??
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
    void loadParameters(size_t);
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
    /*//find the pair containing the detector//
      detectorList::iterator it =
        std::find_if(_detectorList.begin(), _detectorList.end(), IsKey(evt.id()));
      //check wether id is not already on the list//
      if(_detectorList.end() == it)
      {
        //retrieve a pointer to the acqiris device//
        Device *dev =
            dynamic_cast<Device*>(evt->devices().find(cass::CASSEvent::Acqiris)->second);
        //take the last element and get the the detector from it//
        DetectorBackend* det = _detectorList.back().second;
        //copy the informtion of our detector to this detector//
        *det = *_detector;
        //process the detector using the detectors analyzers in a global contaxiner
        _detectoranalyzer[det->analyzerType()]->analyze(*det, dev->channels());
        //create a new key from the id with the reloaded detector
        detectorList_t::value_type newPair = std::make_pair(evt.id(),det);
        //put it to the beginning of the list//
        _detectorList.push_front(newPair);
        //erase the outdated element at the back//
        _detectorList.pop_back();
        //make the iterator pointing to the just added element of the list//
        it = _detectorList.begin();
      }*/
      return 0; //it->second
    }
  protected:
    /*! list of pairs of id-detectors
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
    delete the detector for this instance*/
    ~HelperAcqirisDetectors() {delete _detector;}
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
  /*
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
  }
  //check if an instance of the helper class already exists//
  //return it, otherwise create one and return it//
  if (0 == _instances[dettype])
    _instances[dettype] = new HelperAcqirisDetectors(dettype);
  */
  return 0; //_instances[dettype];
}

void cass::HelperAcqirisDetectors::destroy()
{
/*  //delete all instances of the helper class//
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
*/
}

cass::HelperAcqirisDetectors::HelperAcqirisDetectors(cass::ACQIRIS::Detectors dettype)
{
/*  using namespace cass::ACQIRIS;
  //create the detector
  //create the detector list with twice the amount of elements than workers
  switch(dettype)
  {
  case HexDetector:
    {
      _detector = new DelaylineDetector();
      for (size_t i=0; i<NbrOfWorkers*2;++i)
        _detectorList.push_front(std::make_pair(0,new DelaylineDetector(Hex)));
    }
  case QuadDetector:
    {
      _detector = new DelaylineDetector();
      for (size_t i=0; i<NbrOfWorkers*2;++i)
        _detectorList.push_front(std::make_pair(0,new DelaylineDetector(Quad)));
    }
    break;
  default: throw std::invalid_argument("no such detector is present");
  }
*/
}

cass::HelperAcqirisDetectors::~HelperAcqirisDetectors()
{
 /* //delete the detectorList
  for (detectorList_t::iterator it=_detectorList.begin();
       it != _detectorList.end();
       ++it)
    delete it->second;
  //delete the detector
  delete _detector;
*/
}

void cass::HelperAcqirisDetectors::loadParameters(size_t)
{
  //QSettings par;
  //par.beginGroup("postprocessors");
  //par.beginGroup("AcqirisDetectors");
  //_detector->loadParameters(&par);
}






//----------------Nbr of Peaks MCP-------------------------------------------------
//--------------------------pp550, pp600-------------------------------------------
cass::pp550::pp550(PostProcessors::histograms_t &hist, PostProcessors::id_t id)
  :cass::PostprocessorBackend(hist,id),
  _nbrSignals(0)
{
  //find out which detector and Signal we should work on
  switch (_id)
  {
//  case PostProcessors::HexMCPNbrSignals:
//    _detector = HexDelayline;break;
//  case PostProcessors::QuadMCPNbrSignals:
//    _detector = QuadDelayline;break;
  default:
    throw std::invalid_argument("id is not responsible for Nbr Signals Postprocessor");
  }
  //create the histogram by loading the settings//
  loadSettings(0);
}

cass::pp550::~pp550()
{
  delete _nbrSignals;
  _nbrSignals=0;
}

void cass::pp550::loadParameters(size_t)
{
  //create the histogram
  set1DHist(_nbrSignals,_id);
  _histograms[_id] =  _nbrSignals;
}

void cass::pp550::operator()(const cass::CASSEvent &evt)
{
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
//  case PostProcessors::HexU1NbrSignals:
//    _detector = ; _layer = 'U'; _signal = '1';break;
//  case PostProcessors::HexU2NbrSignals:
//    _detector = ; _layer = 'U'; _signal = '2';break;
//  case PostProcessors::HexV1NbrSignals:
//    _detector = ; _layer = 'V'; _signal = '1';break;
//  case PostProcessors::HexV2NbrSignals:
//    _detector = ; _layer = 'V'; _signal = '2';break;
//  case PostProcessors::HexW1NbrSignals:
//    _detector = ; _layer = 'W'; _signal = '1';break;
//  case PostProcessors::HexW2NbrSignals:
//    _detector = ; _layer = 'W'; _signal = '2';break;
//
//  case PostProcessors::QuadX1NbrSignals:
//    _detector = ; _layer = 'X'; _signal = '1';break;
//  case PostProcessors::QuadX2NbrSignals:
//    _detector = ; _layer = 'X'; _signal = '2';break;
//  case PostProcessors::QuadY1NbrSignals:
//    _detector = ; _layer = 'Y'; _signal = '1';break;
//  case PostProcessors::QuadY2NbrSignals:
//    _detector = ; _layer = 'Y'; _signal = '2';break;

  default:
    throw std::invalid_argument("id is not responsible for Nbr Signals Postprocessor");
  }
}

cass::pp551::~pp551()
{
  delete _nbrSignals;
  _nbrSignals=0;
}

void cass::pp551::loadParameters(size_t)
{
  //create the histogram
  set1DHist(_nbrSignals,_id);
  _histograms[_id] =  _nbrSignals;
}

void cass::pp551::operator()(const cass::CASSEvent &evt)
{
}






//----------------Ratio of Layers----------------------------------------------
//-----------pp557, pp560, pp563, pp605, pp608---------------------------------
cass::pp557::pp557(PostProcessors::histograms_t &hist, PostProcessors::id_t id)
  :cass::PostprocessorBackend(hist,id)
{
}

cass::pp557::~pp557()
{
}

void cass::pp557::loadParameters(size_t)
{
}

void cass::pp557::operator()(const cass::CASSEvent &evt)
{
}







//----------------Ratio of Signals vs. MCP-------------------------------------
//-----------pp558-559, pp561-562, pp564-565, pp606-607, pp609-610-------------
cass::pp558::pp558(PostProcessors::histograms_t &hist, PostProcessors::id_t id)
  :cass::PostprocessorBackend(hist,id)
{
}

cass::pp558::~pp558()
{
}

void cass::pp558::loadParameters(size_t)
{
}

void cass::pp558::operator()(const cass::CASSEvent &evt)
{
}








//----------------Ratio of rec. Hits vs. MCP Hits------------------------------
//------------------------------pp566, pp611-----------------------------------
cass::pp566::pp566(PostProcessors::histograms_t &hist, PostProcessors::id_t id)
  :cass::PostprocessorBackend(hist,id)
{
}

cass::pp566::~pp566()
{
}

void cass::pp566::loadParameters(size_t)
{
}

void cass::pp566::operator()(const cass::CASSEvent &evt)
{
}

