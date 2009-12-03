// Copyright (C) 2009 jk, lmf
#include <iostream>
#include <fstream>
#include <cmath>
#include "pnccd_analysis.h"
#include "pnccd_event.h"
#include "cass_event.h"
#include "remi_event.h"
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
  _darkcal_fnames.clear();
  //the light indicator channel of the acqiris//
  _lightIndicatorChannel = value("LightIndicatorChannel",9).toUInt();
  for (size_t iDet=0; iDet<value("size",1).toUInt(); ++iDet)
  {
    beginGroup(s.setNum(static_cast<int>(iDet)));
      //the rebin factors for the detector//
      _rebinfactors.push_back(value("RebinFactor",1).toUInt());
      //the positions of the darkframe calibration data for the detectors//
      _darkcal_fnames.push_back(
          value("DarkCalibrationFilePath","darkcal.darkcal").toString().toStdString());
      //the multiplier for the noise//
      _sigmaMultiplier.push_back(value("SigmaMultiplier",4).toDouble());
      //the conversion factor for adu's to eV//
      _adu2eV.push_back(value("Adu2eV",1).toDouble());
    endGroup();
  }

}

//------------------------------------------------------------------------------
void cass::pnCCD::Parameter::save()
{
  //sting for the container index//
  QString s;
  setValue("LightIndicatorChannel",_lightIndicatorChannel);
  setValue("size",static_cast<uint32_t>(_rebinfactors.size()));
  for (size_t iDet=0; iDet<_rebinfactors.size(); ++iDet)
  {
    beginGroup(s.setNum(static_cast<int>(iDet)));
      setValue("RebinFactor",_rebinfactors[iDet]);
      setValue("DarkCalibrationFilePath",_darkcal_fnames[iDet].c_str());
      setValue("SigmaMultiplier",_sigmaMultiplier[iDet]);
      setValue("Adu2eV",_adu2eV[iDet]);
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
/*  for(size_t i=0; i<_pnccd_analyzer.size(); i++ )
    delete _pnccd_analyzer[i];
      _pnccd_analyzer.clear();*/
}

//------------------------------------------------------------------------------
void cass::pnCCD::Analysis::loadSettings()
{
  //load the settings
  _param.load();
  // Set the dark calibration data in the new analysis instance//
  for(size_t i=0; i<_pnccd_analyzer.size() ;++i)
  {
//    _pnccd_analyzer[i]->loadDarkCalDataFromFile(_param._darkcal_fnames[i]);
    ifstream in(_param._darkcal_fnames[i].c_str(), std::ios::binary|std::ios::ate);
    if (in.is_open())
    {
      //find big the vectors have to be//
      const size_t size = in.tellg() / 2 / sizeof(double);
      //go to the beginning of the file
      in.seekg(0,std::ios::beg);
      //resize the vectors to the right size//
      _param._offsets[i].resize(size);
      _param._noise[i].resize(size);
      //read the parameters stored in the file//
      in.read(reinterpret_cast<char*>(&(_param._offsets[i][0])), _param._offsets.size()*sizeof(double));
      in.read(reinterpret_cast<char*>(&(_param._noise[i][0])), _param._noise.size()*sizeof(double));
    }
  }
}

//------------------------------------------------------------------------------
void cass::pnCCD::Analysis::saveSettings()
{
  //save settings//
  _param.save();
  //now save the noise and the offset vectors to the designated files//
  for (size_t i=0; i<_pnccd_analyzer.size() ; ++i)
  {
    ofstream out(_param._darkcal_fnames[i].c_str(), std::ios::binary);
    if (out.is_open())
    {
      out.write(reinterpret_cast<char*>(&(_param._offsets[i][0])), _param._offsets.size()*sizeof(double));
      out.write(reinterpret_cast<char*>(&(_param._noise[i][0])), _param._noise.size()*sizeof(double));
    }
  }
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
    pnccdevent.detectors()[i].calibrated()=false;
  }

  //check if we have enough rebin parameters and darkframe names for the amount of detectors//
  //increase the noise and offset vectors//
  //increase it if necessary
  if((pnccdevent.detectors().size() > _param._rebinfactors.size()) ||
     (pnccdevent.detectors().size() > _param._darkcal_fnames.size()) ||
     (pnccdevent.detectors().size() > _param._noise.size()) ||
     (pnccdevent.detectors().size() > _param._sigmaMultiplier.size()) ||
     (pnccdevent.detectors().size() > _param._adu2eV.size()) ||
     (pnccdevent.detectors().size() > _param._offsets.size()) ||
     (pnccdevent.detectors().size() > _param._nbrDarkframes.size()))
  {
    //resize to fit the new size and initialize the new settings//
    _param._rebinfactors.resize(pnccdevent.detectors().size(),1);
    _param._darkcal_fnames.resize(pnccdevent.detectors().size(),"darkcal.darkcal");
    _param._noise.resize(pnccdevent.detectors().size());
    _param._offsets.resize(pnccdevent.detectors().size());
    _param._nbrDarkframes.resize(pnccdevent.detectors().size(),0);
    _param._sigmaMultiplier.resize(pnccdevent.detectors().size(),4);
    _param._adu2eV.resize(pnccdevent.detectors().size(),1);
    saveSettings();
  }

  //check if we have enough analyzers for the amount of detectors//
  //increase it if necessary
//   if(pnccdevent.detectors().size() > _pnccd_analyzer.size())
//   {
//     //remember the size the analyzer container had before//
//     const size_t before = _pnccd_analyzer.size();
//     //resize to fit the new size//
//     _pnccd_analyzer.resize(pnccdevent.detectors().size(),0);
//     //initialize the new right darkframe//
//     for (size_t i=before; i<_pnccd_analyzer.size() ;++i)
//     {
//       _pnccd_analyzer[i] = new pnCCDFrameAnalysis();
//       _pnccd_analyzer[i]->loadDarkCalDataFromFile(_param._darkcal_fnames[i]);
//     }
//   }

  //go through all detectors//
  for (size_t iDet=0; iDet<pnccdevent.detectors().size();++iDet)
  {
    //retrieve a reference to the detector we are working on right now//
    cass::pnCCD::pnCCDDetector &det = pnccdevent.detectors()[iDet];
    //retrieve a reference to the corrected frame of the detector//
    cass::pnCCD::pnCCDDetector::frame_t &cf = det.correctedFrame();
    //retrieve a reference to the raw frame of the detector//
    const cass::pnCCD::pnCCDDetector::frame_t &rf = det.rawFrame();
    //retrieve a reference to the nonrecombined photon hits of the detector//
    cass::pnCCD::pnCCDDetector::photonHits_t &phs = det.nonrecombined();
    //retrieve a reference to the noise vector of this detector//
    std::vector<double> &noise = _param._noise[iDet];
    //retrieve a reference to the offset of the detector//
    std::vector<double> &offset = _param._offsets[iDet];
    //retrieve a reference to the number of Darkframes that were taken for this the detector//
    size_t &nDarkframes = _param._nbrDarkframes[iDet];
    //retrieve a reference to the multiplier of the sigma of the noise//
    const double &sigmaMultiplier = _param._sigmaMultiplier[iDet];
    //retrieve a reference to the conversionfactor from "adu" to eV of the noise//
    const double &adu2eV = _param._adu2eV[iDet];

//     std::cout<<iDet<< " "<<pnccdevent.detectors().size()<<" "<< det.rows() << " " <<  det.columns() << " " << det.originalrows() << " " <<det.originalcolumns()<<" "<<rf.size()<< " "<<_pnccd_analyzer[iDet]<<std::endl;

    //if the size of the rawframe is 0, this detector with this id is not in the datastream//
    //so we are not going to anlyse this detector further//
    if (rf.empty()) 
      continue;

    //resize the corrected frame container the noise and the offset to the size of the raw frame container//
    //this code relies on the fact, that the frame size won't change at runtime//
    cf.resize(det.rawFrame().size());
    noise.resize(det.rawFrame().size());
    offset.resize(det.rawFrame().size());
    //for now just rearrange the frame so that it looks like a real frame//
    //get the dimesions of the detector before the rebinning//
    const uint16_t nRows = det.originalrows();
    const uint16_t nCols = det.originalcolumns();


    //to find out whether there was light in the chamber we test to see a signal//
    //on one of the acqiris channels idealy on the intensity monitor signal//
    //create the offset and noise vector//
    if (cassevent->REMIEvent().channels().size()-1 >= _param._lightIndicatorChannel)
    {
      if (cassevent->REMIEvent().channels()[_param._lightIndicatorChannel].peaks().size())
      {
        std::vector<double>::iterator itOffset = offset.begin();
        std::vector<double>::iterator itNoise = noise.begin();
        cass::pnCCD::pnCCDDetector::frame_t::const_iterator itFrame = rf.begin();
        for (; itFrame != rf.end(); ++itFrame,++itNoise,++itOffset)
        {
          *itOffset +=  *itFrame;
          *itNoise  += (*itFrame)*(*itFrame);
        }
        ++nDarkframes;
      }
    }

    //do the selfmade "massaging" of the detector//
    //only if we have already enough darkframes//
    if (nDarkframes > 1)
    {
      std::vector<double>::iterator itOffset = offset.begin();
      std::vector<double>::iterator itNoise  = noise.begin();
      cass::pnCCD::pnCCDDetector::frame_t::const_iterator itRawFrame = rf.begin();
      cass::pnCCD::pnCCDDetector::frame_t::iterator itCorFrame = cf.begin();
      for ( ; itRawFrame != rf.end(); ++itRawFrame,++itCorFrame,++itOffset)
      {
	//statistics//
        const double mean =  *itOffset / nDarkframes;
        const double meansquared =  mean * mean;
        const double sumofsquare = *itNoise;
        const double sigma = sqrt( 1/nDarkframes * sumofsquare - meansquared ); 
        //remove the offset of the frame and copy it into the corrected frame//
        *itCorFrame = static_cast<int16_t>(*itRawFrame - mean);
        //find out whether this pixel is a photon hit//
        if (*itCorFrame > (sigmaMultiplier * sigma) )
        {
          //create a photon hit//
          cass::pnCCD::PhotonHit ph;
          //set the values of the photon hit//
          //todo : find the positions that are clear only after the rearrangement//
          ph.amplitude() = *itCorFrame;
          ph.energy()    = ph.amplitude() * adu2eV;
          //add it to the vector of photon hits//
          phs.push_back(ph);
        }
      }
    }
    //do the "massaging" of the detector//
    //and find the photon hits//
//    if(!_pnccd_analyzer[iDet]->processPnCCDDetectorData(&det))
    if (true)
    {
      //NC the following is increadibly slow...
      //maybe we could shift it downwards, to have a "downsized copy"

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
    else 
      det.calibrated()=true;

    
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
      if(nRows%_param._rebinfactors[iDet] != 0)
      {
        _param._rebinfactors[iDet] = static_cast<uint32_t>(pow(2,int(log2(_param._rebinfactors[iDet]))));
/*        double res_tes= static_cast<double>(_param._rebinfactors[iDet]);
        res_tes = log(res_tes)/0.693;
        res_tes = floor(res_tes);
        _param._rebinfactors[iDet] = static_cast<UInt_t>(pow(2. , res_tes ));*/
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
//       //go through the whole frame//
//       //and do the magic work//
//       for(size_t iCol=0; iCol<newCols ;++iCol)
//       {
//         for(size_t iRow=0; iRow<newRows ;iRow++)
//         {
//           /* the following works only for rebin=2
//           _tmp[iCol*newCols+iRow]=  // actually is newRows
//               cf[iCol    *nCols+ iRow   ]+
//               cf[iCol    *nCols+(iRow+1)]+
//               cf[(iCol+1)*nCols+ iRow   ]+
//               cf[(iCol+1)*nCols+(iRow+1)];*/
// 
//           for(size_t iReby=0;iReby<_param._rebinfactors[iDet];iReby++)
//           {
//             for(size_t iRebx=0;iRebx<_param._rebinfactors[iDet];iRebx++)
//             {
//               _tmp[iCol+newCols*iRow]+=
//                   cf[(iCol*_param._rebinfactors[iDet] +iReby ) +nCols*(iRow*_param._rebinfactors[iDet] +iRebx)  ];
//             }
//           }
//         }
//       }

        //another way//
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
