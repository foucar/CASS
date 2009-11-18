// Copyright (C) 2009 jk, lmf
#include <iostream>
#include "pnccd_analysis.h"
#include "pnccd_event.h"
#include "cass_event.h"

#include <vector>

cass::pnCCD::Analysis::Analysis
(void)
{
  this->loadSettings();
  pnccd_analysis_ = new pnCCDFrameAnalysis();

  return;
}

cass::pnCCD::Analysis::~Analysis
()
{
  if( pnccd_analysis_ ) delete pnccd_analysis_;
}

void cass::pnCCD::Analysis::loadSettings()
{
  //sync before loading//
  sync();

  //initialize your analyzer here using param//
}

void cass::pnCCD::Analysis::saveSettings()
{
  //save your settings here//
}

void cass::pnCCD::Analysis::operator ()(cass::CASSEvent* cassevent)
{
  //extract a reference to the pnccdevent in cassevent//
  cass::pnCCD::pnCCDEvent &pnccdevent = cassevent->pnCCDEvent();
  //clear the event//
  for (size_t i=0; i<pnccdevent.detectors().size();++i)
  {
    pnccdevent.detectors()[i].recombined().clear();
    pnccdevent.detectors()[i].nonrecombined().clear();
  }
  //go through all detectors//
  for (size_t i=0; i<pnccdevent.detectors().size();++i)
  {
    //retrieve a reference to the detector we are working on right now//
    cass::pnCCD::pnCCDDetector &det = pnccdevent.detectors()[i];
    //get the dimesions of the detector//
    //const uint16_t NbrOfRows = det.rows();
    //const uint16_t NbrOfCols = det.columns();

    //resize the corrected frame container to the size of the raw frame container//
//    pnccdevent.detectors()[i].correctedFrame().resize(pnccdevent.detectors()[i].rawFrame().size());
    det.correctedFrame().assign(det.rawFrame().begin(), det.rawFrame().end()); //for testing copy the contents of raw to cor


    //do the "massaging" of the detector here//
//    pnccd_analysis_->processPnCCDDetectorData(&det);
    //calc the integral (the sum of all bins)//
    det.integral() = 0;
    for (size_t j=0; j<det.correctedFrame().size();++j)
      det.integral() += det.correctedFrame()[j];

    //find the photon hits here//

    //rebin image frame

    
    for(size_t jcol=0;jcol<det.columns();jcol++)
    { 
        for(size_t jrow=0;jrow<det.rows();jrow++)
        {

//            if(jcol==0)   std::cout<< jcol << " a " << jrow << " b " << ((jcol*det.columns()+jrow)&0x7FFF) << std::endl;
            det.correctedFrame()[jcol*det.columns()+jrow]=(jcol*det.columns()+jrow)&0x7FFF;
        }
    }

    std::vector<int16_t> rebinned_frame;
    size_t rebinning=2;

    rebinned_frame.resize(det.rows()/rebinning*det.columns()/rebinning);
//    std::cout<<det.columns() << " a " << det.rows() << " b " << rebinning << std::endl;

    for(size_t jcol=0;jcol<det.columns()/rebinning;jcol++)
    { 
        for(size_t jrow=0;jrow<det.rows()/rebinning;jrow++)
        {
           rebinned_frame[jcol*det.columns()/rebinning+jrow]=
              det.correctedFrame()[jcol*det.columns()+jrow]+
               det.correctedFrame()[jcol*det.columns()+(jrow+1)]+
               det.correctedFrame()[(jcol+1)*det.columns()+jrow]+
               det.correctedFrame()[(jcol+1)*det.columns()+(jrow+1)];
        }
    }

    det.correctedFrame().assign(rebinned_frame.begin(), rebinned_frame.end());
//    det.columns()=det.columns()/rebinning;
//    det.rows()=det.rows()/rebinning;
    // the prev does not works....
    det.columns()=1024/2;
    det.rows()=1024/2;
//    std::cout<<det.columns() << " a " << det.rows() << " b " << rebinning << std::endl;

  }
}


// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
