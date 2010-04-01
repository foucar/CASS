//Copyright (C) 2010 lmf

#include <stdexcept>
#include <cmath>
#include <algorithm>

#include "delayline.h"
#include "histogram.h"
#include "cass_event.h"
#include "acqiris_device.h"


//----------------Nbr of Peaks-------------------------------------------------
//-----------pp500 - pp556 & pp600 - 604---------------------------------------
cass::pp550::pp550(PostProcessors::histograms_t &hist, PostProcessors::id_t id)
  :cass::PostprocessorBackend(hist,id)
{
}

cass::pp550::~pp550()
{
  delete _nbrSignals;
  _nbrSignals=0;
}

void cass::pp550::operator()(const cass::CASSEvent &evt)
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

void cass::pp566::operator()(const cass::CASSEvent &evt)
{
}

