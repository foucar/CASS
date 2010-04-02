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
  /*! Helper for Acqiris related Postprocessors
  This class will retrun the requested detector, which signals are going to
  a Acqiris Instrument. It is implemented as a singleton such that every postprocessor
  can call it without knowing about it.*/
  class HelperAcqirisDetectors
  {
  public:
    /** create an instance of an helper for the requested detector,
    if it doesn't exist already*/
    static HelperAcqirisDetectors * instance(Detectors);
    /** destroy the selected instance*/
    static void destory(Detectors);
    /** return the detector of this instance*/
    DetectorBackend * detector(const CASSEvent& evt) {validate(evt);return _detector;}
    /** tell the detector owned by this instance to reload its settings*/
    void loadParameters(size_t);
  protected:
    /** validate whether we have already seen this event
    if not than add a detector, that is copy constructed from
    the detector this instance owns, to the list */
    void validate(const CASSEvent &evt)
    {
    }
  protected:
    /*! map of detectors, one for each id.
    The contents are copy constructed from the detector that this helper instance owns.
    Needs to be at least the size of workers that can possibly call this helper simultaniously,
    but should be shrinked if it get much bigger than the nbr of workers*/
    std::map<uint64_t, DetectorBackend*> _detectorList;
    /** the detector that is belongs to this instance of the helper*/
    DetectorBackend *_detector;
  private:
    /** prevent people from constructin other than using instance()*/
    HelperAcqirisDetectors() {}
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
    /** Singleton Mutex to lock write operations*/
    static QMutex _mutex;
  };

}//end namespace


//--the helper--
//initialize static members//
std::map<cass::ACQIRIS::Detectors,cass::HelperAcqirisDetectors*> cass::HelperAcqirisDetectors::_instances;
QMutex cass::HelperAcqirisDetectors::_mutex;

cass::HelperAcqirisDetectors* cass::HelperAcqirisDetectors::instance(Detectors)
{
  //check if an instance of the helper class already exists//
  //return it, otherwise create one and return it//
  return 0;
}
void cass::HelperAcqirisDetectors::destory(Detectors)
{
  //delete the requested instance of the helper class//
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

