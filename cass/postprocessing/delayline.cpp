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


//--function to set the histogram properties from the cass.ini file--//
void set1DHist(Histogram1DFloat*& hist, size_t id)
{
  //delete old histogram//
  delete hist;
  //open the settings//
  QSettings param;
  param.beginGroup("postprocessors");
  param.beginGroup(QString("processor_") + QString::number(id));
  //create new histogram using the parameters//
  hist = new Histogram1DFloat(param.value("XNbrBins",1),
                              param.value("XLow",0),
                              param.value("XUp",0));
}
void set2DHist(Histogram2DFloat*& hist, size_t id)
{
  //delete old histogram//
  delete hist;
  //open the settings//
  QSettings param;
  param.beginGroup("postprocessors");
  param.beginGroup(QString("processor_") + QString::number(id));
  //create new histogram using the parameters//
  hist = new Histogram2DFloat(param.value("XNbrBins",1),
                              param.value("XLow",0),
                              param.value("XUp",0),
                              param.value("YNbrBins",1),
                              param.value("YLow",0),
                              param.value("YUp",0));
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

void cass::pp550::loadParameters(size_t)
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

void cass::pp550::loadParameters(size_t)
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

void cass::pp550::loadParameters(size_t)
{
}

void cass::pp566::operator()(const cass::CASSEvent &evt)
{
}

