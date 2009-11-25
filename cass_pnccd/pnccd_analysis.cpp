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
  //sting for the container index//
  QString s;
  //sync before loading//
  sync();
	//clear the containers before adding new stuff to them//
  _rebinfactors.clear();
	darkcal_fnames_.clear();
  for (size_t iDet=0; iDet<value("size",1).toUInt(); ++iDet)
  {
    beginGroup(s.setNum(static_cast<int>(iDet)));
		  //the rebin factors for the detector//
      _rebinfactors.push_back(value("RebinFactor",1).toUInt());
			//the positions of the darkframe calibration data for the detectors//
      darkcal_fnames_.push_back(value("DarkCalibratioFilePath","darkcal.darkcal").toString().toStdString());
			
    endGroup();
  }

}

//------------------------------------------------------------------------------
void cass::pnCCD::Parameter::save()
{
  //sting for the container index//
  QString s;
  setValue("size",static_cast<uint32_t>(_rebinfactors.size()));
  for (size_t iDet=0; iDet<_rebinfactors.size(); ++iDet)
  {
    beginGroup(s.setNum(static_cast<int>(iDet)));
      setValue("RebinFactor",_rebinfactors[iDet]);
      setValue("DarkCalibratioFilePath",darkcal_fnames_[iDet].c_str());
    endGroup();
  }
}


//______________________________________________________________________________


//------------------------------------------------------------------------------
cass::pnCCD::Analysis::Analysis(void)
{
  //load the settings//
  loadSettings();
}

//------------------------------------------------------------------------------
cass::pnCCD::Analysis::~Analysis()
{
  for(size_t i=0; i<_pnccd_analyzer.size(); i++ )
    delete _pnccd_analyzer[i];
	_pnccd_analyzer.clear();
}

//------------------------------------------------------------------------------
void cass::pnCCD::Analysis::loadSettings()
{
  //save the settings
  _param.load();
	// Set the dark calibration data in the new analysis instance//
  for(size_t i=0; i<_pnccd_analyzer.size() ;++i)
   _pnccd_analyzer[i]->loadDarkCalDataFromFile(_param.darkcal_fnames_[i]);
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

  //check if we have enough rebin parameters and analzyer for the amount of detectors//
  //increase it if necessary
  if(pnccdevent.detectors().size() > _param._rebinfactors.size())
  {
    //resize to fit the new size and initialize the new settings//
    _param._rebinfactors.resize(pnccdevent.detectors().size(),1);
		//save the new settings//
    _param.save();
  }
  //check if we have enough darkframenames parameters for the amount of detectors//
  //increase it if necessary
  if(pnccdevent.detectors().size() > _param.darkcal_fnames_.size())
  {
    //resize to fit the new size//
    _param.darkcal_fnames_.resize(pnccdevent.detectors().size(),"darkcal.darkcal");
		//save the new settings//
    _param.save();
  }
  //check if we have enough analyzers for the amount of detectors//
  //increase it if necessary
  if(pnccdevent.detectors().size() > _pnccd_analyzer.size())
  {
	  //remember the size the analyzer container had before//
		const size_t before = _pnccd_analyzer.size();
    //resize to fit the new size//
    _pnccd_analyzer.resize(pnccdevent.detectors().size(),new pnCCDFrameAnalysis());
    //initialize the new right darkframe//
    for (size_t i=before; i<_pnccd_analyzer.size() ;++i)
      //load the dark frame for pnCCDFrameAnalysis//
      _pnccd_analyzer[i]->loadDarkCalDataFromFile(_param.darkcal_fnames_[i]);
		//save the new settings//
    _param.save();
  }

  //go through all detectors//
  for (size_t iDet=0; iDet<pnccdevent.detectors().size();++iDet)
  {
    //retrieve a reference to the detector we are working on right now//
    cass::pnCCD::pnCCDDetector &det = pnccdevent.detectors()[iDet];
    //retrieve a reference to the corrected frame of the detector//
    cass::pnCCD::pnCCDDetector::frame_t &cf = det.correctedFrame();
    //retrieve a reference to the raw frame of the detector//
    cass::pnCCD::pnCCDDetector::frame_t &rf = det.rawFrame();

    //resize the corrected frame container to the size of the raw frame container//
    cf.resize(det.rawFrame().size());
    //for now just rearrange the frame so that it looks like a real frame//
    //get the dimesions of the detector before the rebinning//
    const uint16_t nRows = det.originalrows();
    const uint16_t nCols = det.originalcolumns();



    //do the "massaging" of the detector//
    //and find the photon hits//
    if(!_pnccd_analyzer[iDet]->processPnCCDDetectorData(&det,iDet))
    {
      //if nothing was done then rearrange the frame to the right geometry//
      //go through the complete frame and copy the first row to the first row//
      //and the next row to the last row and so on//
      size_t sourceindex=0;
      for (size_t i=0; i<nRows/2 ;++i)
      {
        memcpy(&cf[i*nCols],
               &rf[sourceindex*nCols],
               1024*sizeof(int16_t));
        ++sourceindex;
        memcpy(&cf[(nCols-1-i)*nCols],
               &rf[sourceindex*nCols],
               1024*sizeof(int16_t));
        ++sourceindex;
      }
    }


    //calc the integral (the sum of all bins)//
    det.integral() = 0;
    for (size_t iInt=0; iInt<cf.size() ;++iInt)
      det.integral() += cf[iInt];

    //test do not delete
    /*#ifdef test
    for(size_t iCol=0; iCol<nCols ;++iCol)
    {
      for(size_t iRow=0; iRow<nRows ;iRow++)
      {
        cf[(iCol) *nRows+(iRow) ]= ((iCol)*nRows+(iRow))&0x3FFF;
      }
    }
    #endif*/

    //rebin image frame if requested//
    if (_param._rebinfactors[iDet] != 1)
    {
      //if the rebinfactor doesn't fit the original dimensions//
      //checks wether rebinfactor is of power of 2//
      //look for the next smaller number that is a power of 2//
      if(nRows%_param._rebinfactors[iDet]!=0)
      {
        //pow(2,int(log2(_param._rebinfactor)));
        double res_tes= static_cast<double>(_param._rebinfactors[iDet]);
        res_tes = log(res_tes)/0.693;
        res_tes = floor(res_tes);
        _param._rebinfactors[iDet] = static_cast<UInt_t>(pow(2. , res_tes ));
        saveSettings();
      }
      //get the new dimensions//
      const size_t newRows = nRows / _param._rebinfactors[iDet];
      const size_t newCols = nCols / _param._rebinfactors[iDet];
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

          for(size_t iReby=0;iReby<_param._rebinfactors[iDet];iReby++)
          {
            for(size_t iRebx=0;iRebx<_param._rebinfactors[iDet];iRebx++)
            {
              _tmp[iCol+newCols*iRow]+=
                  cf[(iCol*_param._rebinfactors[iDet] +iReby ) +nCols*(iRow*_param._rebinfactors[iDet] +iRebx)  ];
            }
          }
        }
      }

      /*//another way//
        for (size_t iIdx=0; iIdx<cf.size() ;++iIdx)
        {
          //calculate the row and column of the current Index//
          const size_t row = iIdx / nCols;
          const size_t col = iIdx % nCols;
          //calculate the index of the rebinned frame//
          const size_t newRow = row / _param._rebinfactors[iDet];
          const size_t newCol = col / _param._rebinfactors[iDet];
          //calculate the index in the rebinned frame//
          //that newRow and newCol belongs to//
          const size_t newIndex = newRow*newCols + newCol;
          //add this index value to the newIndex value//
          _tmp[newIndex] += cf[iIdx];

        }*/

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
