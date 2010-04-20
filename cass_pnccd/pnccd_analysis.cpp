// Copyright (C) 2009 jk, lmf
#include <QtCore/QMutexLocker>
#include <iostream>
#include <fstream>
#include <cmath>
#include "pnccd_analysis.h"
#include "pnccd_device.h"
#include "cass_event.h"

#include <vector>


void cass::pnCCD::Parameter::loadDetectorParameter(size_t idx)
{
  QString s;
  //retrieve reference to the detector parameter//
  DetectorParameter &dp = _detectorparameters[idx];
  //retrieve the parameter settings//
  beginGroup(s.setNum(static_cast<int>(idx)));
  dp._rebinfactor = value("RebinFactor",1).toUInt();
  dp._sigmaMultiplier = value("SigmaMultiplier",4).toDouble();
  dp._adu2eV = value("Adu2eV",1).toDouble();
  dp._createPixellist = value("CreatePixelList",false).toBool();
  dp._doOffsetCorrection = value("DoOffsetCorrection",true).toBool();
  dp._darkcalfilename =
      value("DarkCalibrationFileName",QString("darkcal_%1.cal").arg(idx)).toString().toStdString();
  endGroup();
}

void cass::pnCCD::Parameter::load()
{
  //the flag//
  _isDarkframe = value("IsDarkFrames",false).toBool();
  //string for the container index//
  QString s;
  //sync before loading//
  sync();
  //resize the detector parameter container before adding new stuff//
  _detectorparameters.resize(value("size",1).toUInt(),DetectorParameter());
  //go through all detectors and load the parameters for them//
  for (size_t iDet=0; iDet<value("size",1).toUInt(); ++iDet)
  {
    //load the detector parameters for this detector//
    loadDetectorParameter(iDet);
  }
}

//------------------------------------------------------------------------------
void cass::pnCCD::Parameter::save()
{
  //the flag//
  setValue("IsDarkFrames",_isDarkframe);

  //string for the container index//
  QString s;
  setValue("size",static_cast<uint32_t>(_detectorparameters.size()));
  for (size_t iDet=0; iDet<_detectorparameters.size(); ++iDet)
  {
    //retrieve reference to the detector parameter//
    DetectorParameter &dp = _detectorparameters[iDet];
    //set the values of the detector parameter//
    beginGroup(s.setNum(static_cast<int>(iDet)));
    setValue("RebinFactor",dp._rebinfactor);
    setValue("SigmaMultiplier",dp._sigmaMultiplier);
    setValue("Adu2eV",dp._adu2eV);
    setValue("CreatePixelList",dp._createPixellist);
    setValue("DoOffsetCorrection",dp._doOffsetCorrection);
    setValue("DarkCalibrationFileName",dp._darkcalfilename.c_str());
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
}

//------------------------------------------------------------------------------
void cass::pnCCD::Analysis::loadSettings()
{
  QMutexLocker locker(&_mutex);
  //load the settings
  _param.load();
  //now load the offset and noise map from the dark cal file//
  for(size_t iDet=0;iDet<_param._detectorparameters.size();++iDet)
  {
    //retrieve a reference to the detector parameter
    DetectorParameter &dp = _param._detectorparameters[iDet];
    //open the file that should contain the darkframes//
    std::ifstream in(dp._darkcalfilename.c_str(), std::ios::binary|std::ios::ate);
    if (in.is_open())
    {
//      std::cout <<"reading pnccd "<<i<<" from file \""<<_param._darkcal_fnames[i].c_str()<<"\""<<std::endl;
      //find how big the vectors have to be//
      const size_t size = in.tellg() / 2 / sizeof(double);
      //go to the beginning of the file
      in.seekg(0,std::ios::beg);
      //resize the vectors to the right size//
      dp._offset.resize(size);
      dp._noise.resize(size);
      //read the parameters stored in the file//
      //!!! needs to be tested, didnt work last time//
      in.read(reinterpret_cast<char*>(&(dp._offset[0])), dp._offset.size()*sizeof(double));
      in.read(reinterpret_cast<char*>(&(dp._noise[0])), dp._noise.size()*sizeof(double));
    }
  }
}

//------------------------------------------------------------------------------
void cass::pnCCD::Analysis::saveSettings()
{
  QMutexLocker locker(&_mutex);
  //save settings//
  _param.save();
  //now save the noise and the offset maps to the designated files//
  for (size_t iDet=0;iDet<_param._detectorparameters.size();++iDet)
  {
    //retrieve a reference to the detector parameter
    const DetectorParameter &dp = _param._detectorparameters[iDet];
    //save only if the vectors have some information inside//
    if (!dp._offset.empty() && !dp._noise.empty())
    {
      //create a output file//
      std::ofstream out(dp._darkcalfilename.c_str(), std::ios::binary);
      if (out.is_open())
      {
//        std::cout <<"writing pnccd "<<i<<" to file \""<<_param._save_darkcal_fnames[i].c_str()<<"\""<<std::endl;
        //write the parameters to the file//
        out.write(reinterpret_cast<const char*>(&(dp._offset[0])), dp._offset.size()*sizeof(double));
        out.write(reinterpret_cast<const char*>(&(dp._noise[0])), dp._noise.size()*sizeof(double));
      }
    }
  }
}

//------------------------------------------------------------------------------
void cass::pnCCD::Analysis::operator()(cass::CASSEvent* cassevent)
{
  //extract a reference to the pnccddevice//
  cass::pnCCD::pnCCDDevice &dev =
      *dynamic_cast<pnCCDDevice*>(cassevent->devices()[cass::CASSEvent::pnCCD]);

  //clear the pixellist of all detectors in the device//
  for (size_t i=0; i<dev.detectors().size();++i)
    dev.detectors()[i].pixellist().clear();

  //check if we have enough detector parameters for the amount of detectors//
  //increase it if necessary
  if(dev.detectors().size() > _param._detectorparameters.size())
  {
    //resize detectorparameters and initialize with the new settings//
    _param._detectorparameters.resize(dev.detectors().size());
    for (size_t iDet=0; iDet<_param._detectorparameters.size();++iDet)
      _param.loadDetectorParameter(iDet);
  }

  //if we are collecting darkframes right now then add frames to the off&noisemap//
  //and do no further analysis//
  if(_param._isDarkframe)
  {
    createOffsetAndNoiseMap(dev);
    return;
  }

  //go through all detectors//
  for (size_t iDet=0; iDet<dev.detectors().size();++iDet)
  {
    //retrieve a reference to the detector parameter for det we are working on//
    DetectorParameter &dp = _param._detectorparameters[iDet];
    //retrieve a reference to the detector we are working on right now//
    cass::CCDDetector &det = dev.detectors()[iDet];
    //retrieve a reference to the frame of the detector//
    cass::CCDDetector::frame_t &f = det.frame();

    //if the size of the rawframe is 0, this detector with this id is not in the datastream//
    //so we are not going to anlyse this detector further//
    if (f.empty())
      continue;

    //substract offsetmap//
    if(dp._doOffsetCorrection)

    //if user wants to extract the pixels that are above threshold, do it//
    if (dp._createPixellist)
    {
    }

    //if the user requested rebinning then rebin//
    if(dp._rebinfactor > 1)
      rebin();
  }
}


// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
