// Copyright (C) 2009 jk, lmf
#include <iostream>
#include <cmath>
#include "pnccd_analysis.h"
#include "pnccd_event.h"
#include "cass_event.h"
#include "pnccd_analysis_lib.h"

#include <vector>


void cass::pnCCD::Parameter::load()
{
  //sync before loading//
  sync();
  _rebinfactor = value("RebinFactor",1).toUInt();
}

//------------------------------------------------------------------------------
void cass::pnCCD::Parameter::save()
{
  setValue("RebinFactor",static_cast<uint32_t>(_rebinfactor));
}


//______________________________________________________________________________


//------------------------------------------------------------------------------
cass::pnCCD::Analysis::Analysis(void)
    :pnccd_analysis_(0)
{
  //load the settings//
  loadSettings();
  //create an instance of the frame analysis from Munich//
  pnccd_analysis_ = new pnCCDFrameAnalysis();
}

//------------------------------------------------------------------------------
cass::pnCCD::Analysis::~Analysis()
{
  delete pnccd_analysis_;
}

//------------------------------------------------------------------------------
void cass::pnCCD::Analysis::loadSettings()
{
  //save the settings
  _param.load();
}

//------------------------------------------------------------------------------
void cass::pnCCD::Analysis::saveSettings()
{
  //save settings//
  _param.save();
}

//------------------------------------------------------------------------------
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
  for (size_t iDet=0; iDet<pnccdevent.detectors().size();++iDet)
  {
    //retrieve a reference to the detector we are working on right now//
    cass::pnCCD::pnCCDDetector &det = pnccdevent.detectors()[iDet];

   //retrieve a reference to the corrected frame of the detector//
    cass::pnCCD::pnCCDDetector::frame_t &cf = det.correctedFrame();

    //get the dimesions of the detector//
    const uint16_t nRows = det.rows();
    const uint16_t nCols = det.columns();

    //resize the corrected frame container to the size of the raw frame container//
//   cf.resize(pnccdevent.detectors()[i].rawFrame().size());
    cf.assign(det.rawFrame().begin(), det.rawFrame().end()); //for testing copy the contents of raw to cor


    //do the "massaging" of the detector//
    //and find the photon hits//
//    pnccd_analysis_->processPnCCDDetectorData(&det);


    //calc the integral (the sum of all bins)//
    det.integral() = 0;
    for (size_t iInt=0; iInt<cf.size() ;++iInt)
      det.integral() += cf[iInt];

//test do not delete
#ifdef test
      for(size_t iCol=0; iCol<nCols ;++iCol)
      {
        for(size_t iRow=0; iRow<nRows ;iRow++)
        {
	    cf[(iCol) *nRows+(iRow) ]= ((iCol)*nRows+(iRow))&0x3FFF;
	}
      }
#endif

    //rebin image frame//
    if (_param._rebinfactor != 1)
    {
      if(nRows%_param._rebinfactor!=0)
      {
          //pow(2,int(log2(_param._rebinfactor)));
        Double_t res_tes= static_cast<Double_t>(_param._rebinfactor);
        res_tes=log(res_tes)/0.693;
        res_tes=floor(res_tes);
        _param._rebinfactor=static_cast<UInt_t>(pow(2. , res_tes ));
      }
      //get the new dimensions//
      const size_t newRows = nRows / _param._rebinfactor;
      const size_t newCols = nCols / _param._rebinfactor;
      //set the new dimensions in the detector//
      det.rows()    = newRows;
      det.columns() = newCols;
      //resize the temporary container to fit the rebinned image
      //initialize it with 0
      _tmp.assign(newRows * newCols,0);
      //go through the whole frame//
      //and do the magic work//
      for(size_t iCol=0; iCol<newCols ;++iCol)
      {
        for(size_t iRow=0; iRow<newRows ;iRow++)
        {
          /* the following works only for rebin=2
          _tmp[iCol*newCols+iRow]=  // actually is newRows
              cf[iCol    *nCols+ iRow   ]+
              cf[iCol    *nCols+(iRow+1)]+
              cf[(iCol+1)*nCols+ iRow   ]+
              cf[(iCol+1)*nCols+(iRow+1)];*/

          for(size_t iReby=0;iReby<_param._rebinfactor;iReby++)
          {
            for(size_t iRebx=0;iRebx<_param._rebinfactor;iRebx++)
            {
              _tmp[iCol*newRows+iRow]+=
                  cf[(iCol +iReby ) *nRows+(iRow +iRebx)  ];
            }
          }
        }
      }
      //copy the temporary frame to the right place
      cf.assign(_tmp.begin(), _tmp.end());
    }

  }
}


// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
