// Copyright (C) 2009 jk, lmf,Nicola Coppola
#include <QtCore/QMutexLocker>
#include <iostream>
#include <fstream>
//#include <algorithm>
#include <cmath>
#include "pnccd_analysis.h"
#include "pnccd_device.h"
#include "cass_event.h"

#include <vector>
#include <time.h>
#include <stdexcept>

bool not_saved_yet;

void cass::pnCCD::Parameter::loadDetectorParameter(size_t idx)
{
  sync();
  //sting for the container index//
  QString s,c;
  time_t rawtime;
  struct tm * timeinfo;
  size_t dateSize=9+4+1;
  char date_and_time[dateSize];

  //retrieve reference to the detector parameter//
  DetectorParameter &dp = _detectorparameters[idx];
  //retrieve the parameter settings//
  beginGroup(s.setNum(static_cast<int>(idx)));
    dp._rebinfactor = value("RebinFactor",1).toUInt();
    std::cout<<"RebinFactor= "<<dp._rebinfactor<<std::endl;
    dp._max_noise = value("MaxNoise",4000).toDouble();
    std::cout<< "The max allowed noise level before mask is set to "<<dp._max_noise <<
      " ADC counts, identified through 3x std dev"<<std::endl;
    dp._sigmaMultiplier = value("SigmaMultiplier",4).toDouble();
    std::cout<<"SigmaMultiplier= "<<dp._sigmaMultiplier<<std::endl;
    dp._adu2eV = value("Adu2eV",1).toDouble();
    std::cout<<"Adu2ev= "<<dp._adu2eV<<std::endl;
    dp._createPixellist = value("CreatePixelList",false).toBool();
    std::cout<<"Create Pixel List is "<<dp._createPixellist<<std::endl;
    dp._doOffsetCorrection = value("DoOffsetCorrection",false).toBool();
    if(dp._doOffsetCorrection) std::cout<<"Offset Correction will be applied for detector "<<idx<<std::endl;
    else std::cout<<"Offset Correction will NOT be applied for detector "<<idx<<std::endl;
    dp._useCommonMode = value("useCommonMode",false).toBool();
    if(dp._useCommonMode) std::cout<<"Common Mode Correction will be applied for detector "<<idx<<std::endl;
    else std::cout<<"Common Mode Correction will NOT be applied for detector "<<idx<<std::endl;
    dp._thres_for_integral = value("IntegralOverThres",0).toUInt();
    if(dp._thres_for_integral>0) 
      std::cout<<"Also the integral of the pixel above thresold will be calculated for detector "<<idx<<std::endl;
    dp._darkcalfilename =
      value("DarkCalibrationFileName",QString("darkcal_%1.cal").arg(idx)).toString().toStdString();
    std::cout<<"Read-in filename is "<<dp._darkcalfilename << std::endl;
    if(_isDarkframe)
    {
      time ( &rawtime );
      timeinfo = localtime ( &rawtime );    
      strftime(date_and_time,dateSize,"%Y%m%d_%H%M",timeinfo);
      std::cout<<"now is "<<date_and_time<<std::endl;
      dp._savedarkcalfilename =
        value("DarkCalibrationSaveFileName", QString("darkcal_save_%1_%2.cal").arg(idx).arg(date_and_time) 
              ).toString().toStdString();
      std::cout<<"Save filename is something like "<<dp._savedarkcalfilename << std::endl;
    }
    //cass::PixelDetector::ROIsimple::load();
    dp._detROI._ROI.clear();

    beginGroup("ROIs");
    for (size_t iROI=0; iROI<value("ROIsize",0).toUInt(); ++iROI)
    {
      beginGroup(c.setNum(static_cast<uint32_t>(iROI)));
        dp._detROI._ROI.push_back(ROIsimple());
        //the shape of the ROI//
        dp._detROI._ROI[iROI].name = value("ROIShapeName","square").toString().toStdString();
        // the size(s) along the x axis
        dp._detROI._ROI[iROI].xsize = value("XSize",1).toUInt();
        // the size(s) along the y axis
        dp._detROI._ROI[iROI].ysize = value("YSize",1).toUInt();
        // the centre(s) along the x axis
        dp._detROI._ROI[iROI].xcentre = value("XCentre",1).toUInt();
        // the centre(s) along the y axis
        dp._detROI._ROI[iROI].ycentre = value("YCentre",1).toUInt();
        // the orientation is used only in the case of a triangular shape
        dp._detROI._ROI[iROI].orientation = value("orientation",1).toInt();
      endGroup();
      std::cout <<"ROI loaded "<< iROI << " xsiz " << dp._detROI._ROI[iROI].xsize 
                <<" ysiz " << dp._detROI._ROI[iROI].ysize 
                <<" xc " << dp._detROI._ROI[iROI].xcentre
                <<" yc " << dp._detROI._ROI[iROI].ycentre 
                <<" ori " << dp._detROI._ROI[iROI].orientation<<std::endl;
    }
    endGroup();
    std::cout << "done ROI load "<< dp._detROI._ROI.size() << " of pnCCD" << std::endl;
  endGroup();

}

void cass::pnCCD::Parameter::load()
{
  //the flag//
  _isDarkframe = value("IsDarkFrames",false).toBool();
  if(_isDarkframe)
  {
    //not_saved_yet=true;
    std::cout<<"This is a DarkFrame Run"<<std::endl;
  }
  //string for the container index//
  QString s;
  //sync before loading//
  sync();
  size_t size_from_ini = value("size",1).toUInt();
  //std::cout<<"test "<<_detectorparameters.size() << " "<<value("size",1).toUInt()<<std::endl;
  //resize the detector parameter container before adding new stuff//
  if(size_from_ini>_detectorparameters.size())
    _detectorparameters.resize(size_from_ini,DetectorParameter());

  std::cout<<"I have to treat "<<_detectorparameters.size()<<" pnCCD detector(s)"<<std::endl;
  //go through all detectors and load the parameters for them//
  for (size_t iDet=0; iDet<_detectorparameters.size(); ++iDet)
  {
    //load the detector parameters for this detector//
    std::cout<<"Going to load parameters for detector #"<< iDet<<std::endl;
    loadDetectorParameter(iDet);
  }

}

//------------------------------------------------------------------------------
void cass::pnCCD::Parameter::save()
{

  //the flag//
  setValue("IsDarkFrames",_isDarkframe);

  //string for the container index//
  QString s,c;
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
    setValue("useCommonMode",dp._useCommonMode);
    setValue("IntegralOverThres",dp._thres_for_integral);
    setValue("DarkCalibrationFileName",dp._darkcalfilename.c_str());
    setValue("DarkCalibrationSaveFileName",dp._savedarkcalfilename.c_str());
    //cass::PixelDetector::ROIsimple::save();
    beginGroup("ROIs");
    for (size_t iROI=0; iROI< dp._detROI._ROI.size(); ++iROI)
    {
      beginGroup(c.setNum(static_cast<int>(iROI)));
      setValue("ROIShapeName",dp._detROI._ROI[iROI].name.c_str());
      setValue("XSize",dp._detROI._ROI[iROI].xsize);
      setValue("YSize",dp._detROI._ROI[iROI].ysize);
      setValue("XCentre",dp._detROI._ROI[iROI].xcentre);
      setValue("YCentre",dp._detROI._ROI[iROI].ycentre);
      // the orientation is used only in the case of a triangular shape
      setValue("orientation",dp._detROI._ROI[iROI].orientation);
      endGroup();
    }
    endGroup();
    endGroup();
  }

}



//______________________________________________________________________________






//------------------------------------------------------------------------------
cass::pnCCD::Analysis::Analysis()
{
  //load the settings//
  loadSettings();
}

//------------------------------------------------------------------------------
/*cass::pnCCD::Analysis::~Analysis()
{
}*/


//------------------------------------------------------------------------------
void cass::pnCCD::Analysis::loadSettings()
{

  QMutexLocker locker(&_mutex);
  //load the settings
  _param.load();
  size_t number_of_pixelsettozero=0;
  //now load the offset and noise map from the dark cal file//
  for(size_t iDet=0;iDet<_param._detectorparameters.size();++iDet)
  {
    //retrieve a reference to the detector parameter
    DetectorParameter &dp = _param._detectorparameters[iDet];
    //open the file that should contain the darkframes//
    std::ifstream in(dp._darkcalfilename.c_str(), std::ios::binary|std::ios::ate);
    //This Code is completely unsafe vs human mistakes, if Xonline files
    // are used the sizes are completey wrong as the values that are read in!!!

    //read only if this is NOT a dark-frame run
     std::cout<<"Trying to open file: "<<dp._darkcalfilename.c_str()<<std::endl;
    if (in.is_open() && !_param._isDarkframe)
    {
      //std::cout <<"reading pnccd "<<i<<" from file \""<<_param._darkcal_fnames[i].c_str()<<"\""<<std::endl;
      //find how big the vectors have to be//
      const size_t size = in.tellg() / 2 / sizeof(double);
      if(size%(1024*1024)==0) //or 512*1024??
      {
        //go to the beginning of the file
        in.seekg(0,std::ios::beg);
        //resize the vectors to the right size//
        dp._offset.resize(size);
        dp._noise.resize(size);
        dp._ROImask.resize(size);
        dp._ROIiterator.resize(size);
        dp._ROImask_converter.resize(size);
        dp._ROIiterator_converter.resize(size);
        //read the parameters stored in the file//
        in.read(reinterpret_cast<char*>(&(dp._offset[0])), dp._offset.size()*sizeof(double));
        in.read(reinterpret_cast<char*>(&(dp._noise[0])), dp._noise.size()*sizeof(double));
        std::cout<<"offset and noise maps loaded for det# "<<iDet <<std::endl;
      }
      else
      {
        //safe net in case there is no file yet
        dp._offset.resize(pnCCD_default_size_sq);
        dp._noise.resize(pnCCD_default_size_sq);
        dp._ROImask.resize(pnCCD_default_size_sq);
        dp._ROIiterator.resize(pnCCD_default_size_sq);
        dp._ROImask_converter.resize(pnCCD_default_size_sq);
        dp._ROIiterator_converter.resize(pnCCD_default_size_sq);
      }
    }
    else
    {
      std::cout<<"Not able to open file "<<dp._darkcalfilename.c_str()<<std::endl;
      if(_param._isDarkframe) std::cout<<"and this is a Darkframe run"<<std::endl;
      else
      {
        std::cout<<"and this is NOT a Darkframe run"<<std::endl;

        //Is this safe or I should just not do it for certain cases?
        // in principle I do not need it if _param._isDarkframe==1
        // and I should never come here if _param._isDarkframe==0 as I should always have a darkcalib file
        // to load
        dp._ROImask.resize(pnCCD_default_size_sq);
        dp._ROImask_converter.resize(pnCCD_default_size_sq);
      }
      if(!dp._doOffsetCorrection)
      {
        std::cout<<"I am not going to apply offset corrections anyway"<<std::endl;
        //safe net in case there is no file yet and I do not want to make offset-corrections
        dp._offset.resize(pnCCD_default_size_sq);
        dp._noise.resize(pnCCD_default_size_sq);
        //resetting the offset/noise maps
        dp._offset.assign(dp._offset.size(),0);
        dp._noise.assign(dp._noise.size(),0);
      }
      else
      {
        //I should not get here
        throw std::runtime_error(
          QString("I have been asked to Offset correct the frames, but there is no darkframe file named:\n\t "
                  ).arg(dp._darkcalfilename.c_str()).toStdString());
      }
    }
    //in case this is a Dark-Run
    if(_param._isDarkframe)
    {
      dp._offset.resize(pnCCD_default_size_sq);
      dp._noise.resize(pnCCD_default_size_sq);
      //reset the number of dark frames taken so far
      dp._nbrDarkframes=0;
      //resetting the offset/noise maps
      dp._offset.assign(dp._offset.size(),0);
      dp._noise.assign(dp._noise.size(),0);
    }
    // I need the ROI only if I am really running with the FEL
    if(_param._isDarkframe==0)
    {
      // "enable" all  pixel as default
      dp._ROImask.assign(dp._ROImask.size(),1);
      dp._ROImask.assign(dp._ROImask_converter.size(),1);
      //I need to reset the number of masked pixel per frame
      number_of_pixelsettozero=0;
      for(size_t iROI=0;iROI<dp._detROI._ROI.size(); ++iROI)
      {
        size_t index_of_center=dp._detROI._ROI[iROI].xcentre
            + 1024 * dp._detROI._ROI[iROI].ycentre;
        size_t index_min=dp._detROI._ROI[iROI].xcentre - dp._detROI._ROI[iROI].xsize
            + 1024 * (dp._detROI._ROI[iROI].ycentre - dp._detROI._ROI[iROI].ysize);
        size_t index_max=dp._detROI._ROI[iROI].xcentre + dp._detROI._ROI[iROI].xsize
            + 1024 * (dp._detROI._ROI[iROI].ycentre + dp._detROI._ROI[iROI].ysize);
        std::cout << "indexes "<< index_of_center<<" "<<index_min<<" "<<index_max<<std::endl;
        size_t indexROI_min=0;
        size_t indexROI_max=(2 * dp._detROI._ROI[iROI].xsize + 1)
            * (2 * dp._detROI._ROI[iROI].ysize + 1);
        //remember how many pixels I have masked
        //number_of_pixelsettozero+=indexROI_max;
        std::cout << "indexes "<< index_of_center<<" "<<indexROI_min<<" "<<indexROI_max<<std::endl;
        if(dp._detROI._ROI[iROI].name=="circ" || dp._detROI._ROI[iROI].name=="circle"  )
        {
          int32_t  xlocal,ylocal;
          const uint32_t radius2 =  static_cast<uint32_t>(pow(dp._detROI._ROI[iROI].xsize,2)
           );
          std::cout << "circ seen with radius^2= " <<radius2 <<std::endl;
          for(size_t iFrame=indexROI_min;iFrame<indexROI_max; ++iFrame)
          {
            xlocal=iFrame%(2* dp._detROI._ROI[iROI].xsize + 1);
            ylocal=iFrame/(2* dp._detROI._ROI[iROI].xsize + 1);
#ifdef debug
            std::cout<<"local "<<xlocal<<" "<<ylocal<<" "<<dp._detROI._ROI[iROI].xsize<< " " 
                     <<pow(xlocal-static_cast<int32_t>(dp._detROI._ROI[iROI].xsize),2)
                     <<std::endl;
#endif
            if( ( pow(xlocal-static_cast<int32_t>(dp._detROI._ROI[iROI].xsize),2) +
                  pow(ylocal-static_cast<int32_t>(dp._detROI._ROI[iROI].ysize),2) ) <= radius2 )
            {
              //I do not need to set again to zero a pixel that was already masked!
              if (dp._ROImask[index_min+xlocal+ 1024 * (ylocal ) ]!=0)
              {
                dp._ROImask[index_min+xlocal+ 1024 * (ylocal ) ]=0;
                //remember how many pixels I have masked
                number_of_pixelsettozero++;
              }
#ifdef debug
              std::cout<<"in local "<<xlocal<<" "<<ylocal <<std::endl;
#endif
            }
          }
        }
        if(dp._detROI._ROI[iROI].name=="square")
        {
          uint32_t  xlocal,ylocal;
          std::cout << "square seen" <<std::endl;
          for(size_t iFrame=indexROI_min;iFrame<indexROI_max; ++iFrame)
          {
            xlocal=iFrame%(2* dp._detROI._ROI[iROI].xsize +1);
            ylocal=iFrame/(2* dp._detROI._ROI[iROI].xsize +1);
            //I do not need to set again to zero a pixel that was already masked!
            if (dp._ROImask[index_min+xlocal+ 1024 * (ylocal ) ]!=0)
            {
              dp._ROImask[index_min+xlocal+ 1024 * (ylocal ) ]=0;
              //remember how many pixels I have masked
              number_of_pixelsettozero++;
            }
#ifdef debug
            std::cout<<"local "<<xlocal<<" "<<ylocal<<" "<<dp._detROI._ROI[iROI].ycentre
                     << " "<<dp._detROI._ROI[iROI].ycentre - dp._detROI._ROI[iROI].ysize
                     <<std::endl;
#endif
          }
        }
        if(dp._detROI._ROI[iROI].name=="triangle")
        {
          int32_t  xlocal,ylocal;
          float xlocal_f,ylocal_f;
          float xsize,ysize;
          xsize=static_cast<float>(dp._detROI._ROI[iROI].xsize);
          ysize=static_cast<float>(dp._detROI._ROI[iROI].ysize);

          std::cout << "triangle seen" <<std::endl;
          if(dp._detROI._ROI[iROI].orientation==+1)
          {
            std::cout << "triangle seen vertex upwards" <<std::endl;
            //the triangle is at least isosceles
            for(size_t iFrame=indexROI_min;iFrame<indexROI_max; ++iFrame)
            {
              xlocal=iFrame%(2* dp._detROI._ROI[iROI].xsize + 1);
              ylocal=iFrame/(2* dp._detROI._ROI[iROI].xsize + 1);
              xlocal_f=static_cast<float>(xlocal);
              ylocal_f=static_cast<float>(ylocal);

#ifdef debug
              std::cout<<"local "<<xlocal<<" "<<ylocal
                       << " " <<2 * ysize/xsize*xlocal_f << " " <<4*ysize - 2* ysize/xsize*xlocal_f
                       <<std::endl;
#endif
              if(ylocal-1<(2 * ysize/xsize*xlocal_f)
                 && xlocal<static_cast<int32_t>(dp._detROI._ROI[iROI].xsize + 1))
              {
#ifdef debug
                std::cout<<"local1 "<<xlocal<<" "<<ylocal <<std::endl;
#endif
                if (dp._ROImask[index_min+xlocal+ 1024 * (ylocal ) ]!=0)
                {
                  dp._ROImask[index_min+xlocal+ 1024 * (ylocal ) ]=0;
                  //remember how many pixels I have masked
                  number_of_pixelsettozero++;
                }
              }
              else if(ylocal-1<(4*ysize - 2* ysize/xsize*xlocal_f)
                      && xlocal>static_cast<int32_t>(dp._detROI._ROI[iROI].xsize))
              {
#ifdef debug
                std::cout<<"local2 "<<xlocal<<" "<<ylocal <<std::endl;
#endif
                if (dp._ROImask[index_min+xlocal+ 1024 * (ylocal ) ]!=0)
                {
                  dp._ROImask[index_min+xlocal+ 1024 * (ylocal ) ]=0;
                  //remember how many pixels I have masked
                  number_of_pixelsettozero++;
                }
              }
            }
          }
          if(dp._detROI._ROI[iROI].orientation==-1)
          {
            std::cout << "triangle seen vertex downwards" <<std::endl;
            for(size_t iFrame=indexROI_min;iFrame<indexROI_max; ++iFrame)
            {
              xlocal=iFrame%(2* dp._detROI._ROI[iROI].xsize + 1);
              ylocal=iFrame/(2* dp._detROI._ROI[iROI].xsize + 1);
              xlocal_f=static_cast<float>(xlocal);
              ylocal_f=static_cast<float>(ylocal);
#ifdef debug
              std::cout<<"local "<<xlocal<<" "<<ylocal
                       << " " <<(-2 * ysize/xsize*xlocal_f) + 2 * ysize
                       << " "<<-2*ysize + 2 *  ysize/xsize*xlocal <<std::endl;
#endif

              if(ylocal+1>((-2 * ysize/xsize*xlocal_f)
                         + 2 * ysize)
                 && xlocal<static_cast<int32_t>(dp._detROI._ROI[iROI].xsize + 1))
              {
#ifdef debug
                std::cout<<"local1 "<<xlocal<<" "<<ylocal <<std::endl;
#endif
                if (dp._ROImask[index_min+xlocal+ 1024 * (ylocal ) ]!=0)
                {
                  dp._ROImask[index_min+xlocal+ 1024 * (ylocal ) ]=0;
                  //remember how many pixels I have masked
                  number_of_pixelsettozero++;
                }
              }
              else if(ylocal+1>(-2*ysize +
                              2 * ysize/xsize*xlocal_f)
                      && xlocal>static_cast<int32_t>(dp._detROI._ROI[iROI].xsize))
              {
#ifdef debug
                std::cout<<"local2 "<<xlocal<<" "<<ylocal <<std::endl;
#endif
                if (dp._ROImask[index_min+xlocal+ 1024 * (ylocal ) ]!=0)
                {
                  dp._ROImask[index_min+xlocal+ 1024 * (ylocal ) ]=0;
                  //remember how many pixels I have masked
                  number_of_pixelsettozero++;
                }
              }
            }
          }
          if(dp._detROI._ROI[iROI].orientation==+2)
          {
            std::cout << "triangle seen vertex towards right" <<std::endl;
            for(size_t iFrame=indexROI_min;iFrame<indexROI_max; ++iFrame)
            {
              // not debugged
              xlocal=iFrame%(2* dp._detROI._ROI[iROI].xsize + 1);
              ylocal=iFrame/(2* dp._detROI._ROI[iROI].xsize + 1);
              xlocal_f=static_cast<float>(xlocal);
              ylocal_f=static_cast<float>(ylocal);
#ifdef debug
              std::cout<<"local "<<xlocal<<" "<<ylocal
                       << " " <<(ysize/xsize*xlocal_f) << " "<<- ysize/xsize*xlocal_f + 4 * ylocal_f
                       <<std::endl;
#endif
              if(ylocal_f>(ysize/(2*xsize) * xlocal_f) && ylocal_f< (-ysize/(2*xsize)*xlocal_f + 2 * ysize) )
              {
                if (dp._ROImask[index_min+xlocal+ 1024 * (ylocal ) ]!=0)
                {
                  dp._ROImask[index_min+xlocal+ 1024 * (ylocal ) ]=0;
                  //remember how many pixels I have masked
                  number_of_pixelsettozero++;
                }
              }
            }
          }
          if(dp._detROI._ROI[iROI].orientation==-2)
          {
            std::cout << "triangle seen vertex towards left" <<std::endl;
            for(size_t iFrame=indexROI_min;iFrame<indexROI_max; ++iFrame)
            {
              xlocal=iFrame%(2* dp._detROI._ROI[iROI].xsize);
              ylocal=iFrame/(2* dp._detROI._ROI[iROI].xsize);
              xlocal_f=static_cast<float>(xlocal);
              ylocal_f=static_cast<float>(ylocal);
#ifdef debug
              std::cout<<"local "<<xlocal<<" "<<ylocal<<" " <<std::endl;
#endif
              if(ylocal>(- ysize/(2*xsize) * xlocal_f + ysize) && ylocal<( ysize/(2*xsize) * xlocal_f + ysize) )
              {
                if (dp._ROImask[index_min+xlocal+ 1024 * (ylocal ) ]!=0)
                {
                  dp._ROImask[index_min+xlocal+ 1024 * (ylocal ) ]=0;
                  //remember how many pixels I have masked
                  number_of_pixelsettozero++;
                }
              }
            }
          }
        }
      } // end iROI loop

#ifdef debug
      std::cout <<"ROI "<< iDet<<" "<<dp._ROImask.size()<<" ";
      for(size_t i=0;i<dp._ROImask.size();i++)
          std::cout << dp._ROImask[i]<< " ";
      std::cout<<std::endl;
#endif
      // now I know which pixel should be masked!
      size_t nextPixel=0;
      dp._ROIiterator.resize(dp._ROImask.size()-number_of_pixelsettozero-1);
      size_t extra_masked_pixel=0;
      for(size_t iPixel=0;iPixel<dp._ROImask.size(); ++iPixel)
      {
        // the "pointer" is the location/index of the next pixel to be used
        if(dp._ROImask[iPixel]!=0)
        {
          if(dp._noise[iPixel]<3*dp._max_noise)
          {
            dp._ROIiterator[nextPixel]=iPixel+1;
            nextPixel++;
          }
          else
          {
            extra_masked_pixel++;
            dp._ROImask[iPixel]=2; //I'll put it to 2 in this case instead
                                   // so that I know that it was maked BAD...
          }
        }
      }
      dp._ROIiterator.resize(dp._ROImask.size()-number_of_pixelsettozero-1-extra_masked_pixel);
      std::cout << "Extra masked pixel(s) for detector "<< iDet<<" are "<<extra_masked_pixel<<std::endl;
      std::cout <<"Roiit sizes "<< iDet<<" "<<dp._ROImask.size()<<" " 
                <<dp._ROIiterator.size()<< " "
                <<number_of_pixelsettozero <<std::endl;
#ifdef debug
      for(size_t i=0;i<dp._ROIiterator.size();i++)
      {
        if(i%16==0) std::cout <<"Roiit"<<iDet<<" ";
        std::cout << dp._ROIiterator[i]<< " ";
        if(i%16==15) std::cout<<std::endl;
      }
      std::cout<<std::endl;
#endif

    }
  }

}

//------------------------------------------------------------------------------
void cass::pnCCD::Analysis::saveSettings()
{
  time_t rawtime;
  struct tm * timeinfo;
  size_t dateSize=9+4+1;
  char date_and_time[dateSize];

  //QMutexLocker locker(&_mutex);
  //save settings//
  //I actually don't want to write in the CASS.ini file!!
  //_param.save();
  //now save the noise and the offset maps to the designated files//
  if(_param._isDarkframe)
  {
    std::cout<<"I have been asked to save the files"<<std::endl;
    for (size_t iDet=0;iDet<_param._detectorparameters.size();++iDet)
    {
      //retrieve a reference to the detector parameter
      //I need to "normalize the values
      DetectorParameter &dp = _param._detectorparameters[iDet];
      //save only if the vectors have some information inside//
      //save only if this is a dark-frame run
      if (!dp._offset.empty() && !dp._noise.empty() && _param._isDarkframe )
      {
        //normalise the values
        if(dp._nbrDarkframes)
          for(size_t j=0;j<dp._offset.size();j++)
          {
            dp._offset[j]=dp._offset[j]/dp._nbrDarkframes;
            dp._noise[j]=sqrt(dp._noise[j] / dp._nbrDarkframes - dp._offset[j] * dp._offset[j]);
          }
        //adjust the filenames
        time ( &rawtime );
        timeinfo = localtime ( &rawtime );    
        strftime(date_and_time,dateSize,"%Y%m%d_%H%M",timeinfo);
        std::cout<<"now is "<<date_and_time<<std::endl;
        dp._savedarkcalfilename =
          QString("darkcal_save_%1_%2.cal").arg(iDet).arg(date_and_time).toStdString();
        std::cout<<"saved filename is "<<dp._savedarkcalfilename << std::endl;

        //create a output file//
        std::ofstream out(dp._savedarkcalfilename.c_str(), std::ios::binary);
        if (out.is_open())
        {
          //std::cout <<"writing pnccd "<<iDet<<" to file \""<<dp._save_darkcalfilename.c_str()<<"\""<<std::endl;
          //write the parameters to the file//
          out.write(reinterpret_cast<const char*>(&(dp._offset[0])), dp._offset.size()*sizeof(double));
          out.write(reinterpret_cast<const char*>(&(dp._noise[0])), dp._noise.size()*sizeof(double));
        }
        else std::cout<<"Not able to save into file "<<dp._savedarkcalfilename.c_str()<<std::endl;
      }
    }
  }
  else std::cout<<"I have been asked to save the files, but this is not a darkcal-run, I will do nothing of that sort!"<<std::endl;
}

//------------------------------------------------------------------------------
void cass::pnCCD::Analysis::operator()(cass::CASSEvent* cassevent)
{

  //extract a reference to the pnccddevice//
  cass::pnCCD::pnCCDDevice &dev =
      *dynamic_cast<pnCCDDevice*>(cassevent->devices()[cass::CASSEvent::pnCCD]);

  //check if we have enough detector parameters for the amount of detectors//
  //increase it if necessary
  if(dev.detectors()->size() > _param._detectorparameters.size())
  {
    //resize detectorparameters and initialize with the new settings//
    _param._detectorparameters.resize(dev.detectors()->size());
    /*for (size_t iDet=0; iDet<_param._detectorparameters.size();++iDet)
      _param.loadDetectorParameter(iDet);*/
    // the problem is that the prev line is not the whole truth...
    loadSettings();
  }

  //if we are collecting darkframes right now then add frames to the off&noisemap//
  //and do no further analysis//
  if(_param._isDarkframe)
  {
    //QWriteLocker locker(&_RWlock);
    // the following was uncommented. who did insert it??
    //QMutexLocker locker(&_mutex);
    createOffsetAndNoiseMap(dev);
    return;
  }

  //clear the pixellist of all detectors in the device//
  for (size_t i=0; i<dev.detectors()->size();++i)
    (*dev.detectors())[i].pixellist().clear();

  //go through all detectors//
  for (size_t iDet=0; iDet<dev.detectors()->size();++iDet)
  {
    //retrieve a reference to the detector parameter for det we are working on//
    DetectorParameter &dp = _param._detectorparameters[iDet];
    //retrieve a reference to the detector we are working on right now//
    cass::PixelDetector &det = (*dev.detectors())[iDet];
    //retrieve a reference to the frame of the detector//
    cass::PixelDetector::frame_t &f = det.frame();

    //if the size of the rawframe is 0, this detector with this id is not in the datastream//
    //so we are not going to anlyse this detector further//
    if (f.empty())
      continue;

    //substract offsetmap//
    if(dp._doOffsetCorrection)
    {
      //do something
      //retrieve a reference to the frame of the detector//
      cass::PixelDetector::frame_t &f = det.frame();
      cass::PixelDetector::frame_t::iterator itFrame = f.begin();
      cass::pnCCD::DetectorParameter::correctionmap_t::const_iterator itOffset = dp._offset.begin();
      cass::pnCCD::DetectorParameter::correctionmap_t::const_iterator itNoise  = dp._noise.begin();
      const cass::ROI::ROIiterator_t &iter = dp._ROIiterator;
      cass::ROI::ROIiterator_t::const_iterator itROI = iter.begin();
      //const bool ShouldIuseCommonMode= dp._useCommonMode;
      size_t pixelidx=0;
      //let's initialize a bit
      det.integral()=0;
      det.integral_overthres()=0;
      det.maxPixelValue()=0;
      if(!dp._useCommonMode/*I don't use CommonMode*/)
      {
        for ( ; itROI != iter.end(); ++itROI,++pixelidx)
        {
          advance(itFrame,iter[pixelidx+1]-iter[pixelidx]);
          advance(itOffset,iter[pixelidx+1]-iter[pixelidx]);
          advance(itNoise,iter[pixelidx+1]-iter[pixelidx]);
          //I could execute the follwing line only if dp._useCommonMode==false
          // and get rid of the if-statement if(!dp._useCommonMode...)
          //actually my method was introduced to make such lines as the following one not needed.....
          for(size_t jj=iter[pixelidx]; jj<iter[pixelidx+1]-1; jj++) f[jj] = 0;

          // the following work only if I am copying, not if I modify!!
          // If I am modifying the pixel values.. This method leave the masked-pixels unchanged!!
          *itFrame =  *itFrame - *itOffset;
          det.integral() += static_cast<uint64_t>( std::max(*itFrame,float(0.)) );
          if(dp._thres_for_integral && *itFrame > dp._thres_for_integral)
            det.integral_overthres() += static_cast<uint64_t>( std::max(*itFrame,float(0.)) );

          //Should I do it also if _doOffsetCorrection==false?
          //if user wants to extract the pixels that are above threshold, do it//
          if (dp._createPixellist)
          {      
            Pixel this_pixel;
            //itFrame is already offset-subtructed
            if( *itFrame> dp._sigmaMultiplier * *itNoise )
            {
              this_pixel.x()=iter[pixelidx]%det.columns();
              this_pixel.y()=iter[pixelidx]/det.columns();
              this_pixel.z()=*itFrame;
              det.pixellist().push_back(this_pixel);
              //I could "tag" the pixel
              // something like "mask[iFrame]=3"
            }
          }//end loop to create pixels'list
        }//end loop over frame
      }
      else //start useCommonMode loop
      {
        //const cass::ROI::ROImask_t &mask = dp._ROImask;
        //merd
        //std::cout<<"I need to do something"<<std::endl;
        const size_t Num_pixel_per_line = 128;
        const size_t num_lines = det.originalrows()* det.originalcolumns() /Num_pixel_per_line;
        size_t i_pixel;
        for (size_t i_line=0;i_line<num_lines;i_line++)
        {
          double common_level=0;
          double Pixel_wo_offset;
          size_t used_pixel=0;
          for(i_pixel=0;i_pixel<Num_pixel_per_line;++i_pixel,++itFrame,++itNoise,++itOffset)
          {
            //I consider only the "good" pixels that are not to be masked
            if(dp._ROImask[i_line*Num_pixel_per_line+i_pixel]==1) //I could only remove the BAD-pixel
            {
              Pixel_wo_offset= static_cast<double>(*itFrame) - *itOffset;
              //I add only the pixel w/o a signal-photon
              if( Pixel_wo_offset>0 && Pixel_wo_offset< dp._sigmaMultiplier * *itNoise )
              {
                common_level+= Pixel_wo_offset;
                used_pixel++;
              }
            }
          }
          if(used_pixel>8)
          {
            common_level/=(used_pixel-1);
#ifdef debug_a_lot
            std::cout<<"Common mode subtraction value is "<< static_cast<pixel_t>(common_level)
                     <<" based on "<<used_pixel <<" pixels" <<std::endl;
#endif
          }
          else common_level=0;
          //come back to the beginning of the line
          advance(itFrame,-Num_pixel_per_line);
          advance(itOffset,-Num_pixel_per_line);
          advance(itNoise,-Num_pixel_per_line);
          for(i_pixel=0;i_pixel<Num_pixel_per_line;++i_pixel,++itFrame,++itNoise,++itOffset)
          {
            *itFrame =  *itFrame - *itOffset - common_level ;
          }
        }//end loop over frame
        //I am actually not finished I need now to mask the pixels belonging to ROIs...
        itFrame = f.begin();
        itOffset = dp._offset.begin();
        itNoise  = dp._noise.begin();
        itROI = iter.begin();

        for ( ; itROI != iter.end(); ++itROI,++pixelidx)
        {
          advance(itFrame,iter[pixelidx+1]-iter[pixelidx]);
          advance(itOffset,iter[pixelidx+1]-iter[pixelidx]);
          advance(itNoise,iter[pixelidx+1]-iter[pixelidx]);
          //I could execute the follwing line only if dp._useCommonMode==false
          // and get rid of the if-statement if(!dp._useCommonMode...)
          //actually my method was introduced to make such lines as the following one not needed.....
          for(size_t jj=iter[pixelidx]; jj<iter[pixelidx+1]-1; jj++) f[jj] = 0;

          // the following work only if I am copying, not if I modify!!
          // If I am modifying the pixel values.. This method leave the masked-pixels unchanged!!
          *itFrame =  *itFrame - *itOffset;
          det.integral() += static_cast<uint64_t>(std::max(*itFrame,float(0.)));
          if(dp._thres_for_integral && *itFrame > dp._thres_for_integral)
            det.integral_overthres() += static_cast<uint64_t>(std::max(*itFrame,float(0.)));

          //Should I do it also if _doOffsetCorrection==false?
          //if user wants to extract the pixels that are above threshold, do it//
          if (dp._createPixellist)
          {      
            Pixel this_pixel;
            //itFrame is already offset-subtructed
            if( *itFrame> dp._sigmaMultiplier * *itNoise )
            {
              this_pixel.x()=iter[pixelidx]%det.columns();
              this_pixel.y()=iter[pixelidx]/det.columns();
              this_pixel.z()=*itFrame;
              det.pixellist().push_back(this_pixel);
              //I could "tag" the pixel
              // something like "mask[iFrame]=3"
            }
          }//end loop to create pixels'list
        }
      }// endif CommonMode Subtraction
#ifdef debug
      if(dp._createPixellist) std::cout<<"number of found photons on pnCCD " << iDet
                                       << " is "<< det.pixellist().size()<<std::endl;
#endif
    }//end if OffsetCorrection
    //if the user requested rebinning then rebin//
    if(dp._rebinfactor > 1)
      rebin(dev,iDet);
  }//end loop iDet

}


void cass::pnCCD::Analysis::createOffsetAndNoiseMap(cass::pnCCD::pnCCDDevice &dev)
{
  QMutexLocker  locker(&_mutex);
  //Insert read/write lock? But I get a Segfault...
  //QWriteLocker  locker(&_RWlock);
  for (size_t iDet=0; iDet<dev.detectors()->size();++iDet)
  {
    //retrieve a reference to the detector we are working on right now//
    cass::PixelDetector &det = (*dev.detectors())[iDet];
    //retrieve a reference to the frame of the detector//
    cass::PixelDetector::frame_t &f = det.frame();
    //retrieve a reference to the detector parameters we are working on right now//
    DetectorParameter &dp = _param._detectorparameters[iDet];

    cass::PixelDetector::frame_t::const_iterator itFrame = f.begin();
    cass::pnCCD::DetectorParameter::correctionmap_t::iterator itOffset = dp._offset.begin();
    cass::pnCCD::DetectorParameter::correctionmap_t::iterator  itNoise  = dp._noise.begin();
    for (; itOffset!=dp._offset.end() ;++itOffset,++itFrame,++itNoise)
    {
      *itOffset += static_cast<double>(*itFrame);
      *itNoise  += static_cast<double>(*itFrame) * static_cast<double>(*itFrame);
    }
    ++dp._nbrDarkframes;
    if(dp._nbrDarkframes>=200 && (dp._nbrDarkframes%20)==0) 
      std::cout<<"reached "<< dp._nbrDarkframes<< " darkframes for pnCCD "<<iDet<<std::endl;
    /*
    if(dp._nbrDarkframes>201 && not_saved_yet)
    {
      //Only one of the Threads should save.... So I don't see why I should do it....
      //This would be only a dirty trick!!!!
      //cass::pnCCD::Analysis::saveSettings();
      saveSettings();
      //std::cout<<"here "<<std::endl;
      not_saved_yet=false;
    }
    */
  }
}

void cass::pnCCD::Analysis::rebin(cass::pnCCD::pnCCDDevice &dev,size_t iDet)
{
  QMutexLocker  locker(&_mutex);
  //retrieve a reference to the detector and "chip" we are working on right now//
  cass::PixelDetector &det = (*dev.detectors())[iDet];
  //retrieve a reference to the detector parameters we are working on right now//
  DetectorParameter &dp = _param._detectorparameters[iDet];
  //I should not be here
  if(dp._rebinfactor <=1) return;
  //retrieve a reference to the frame of the detector//
  cass::PixelDetector::frame_t &f = det.frame();

  //else to the rebinnning;
  //get the dimesions of the detector before the rebinning//
  const uint16_t nRows = det.originalrows();
  const uint16_t nCols = det.originalcolumns();
  if(nRows % dp._rebinfactor != 0)
  {
    //the rebin factor is not a safe one
    dp._rebinfactor = static_cast<uint32_t>(pow(2,int(log2(dp._rebinfactor))));
  }
  //get the new dimensions//
  const size_t newRows = nRows / dp._rebinfactor;
  const size_t newCols = nCols / dp._rebinfactor;
  //set the new dimensions in the detector//
  det.rows()    = newRows;
  det.columns() = newCols;
  _tmp.clear();
  //resize the temporary container to fit the rebinned image
  _tmp.resize(f.size()/dp._rebinfactor/dp._rebinfactor);
  //initialize it with 0
  _tmp.assign(newRows * newCols,0);
  //go through the whole corrected frame//
  for (size_t iIdx=0; iIdx<f.size() ;++iIdx)
  {
    //calculate the row and column of the current Index//
    const size_t row = iIdx / nCols;
    const size_t col = iIdx % nCols;
    //calculate the index of the rebinned frame//
    const size_t newRow = row / dp._rebinfactor;
    const size_t newCol = col / dp._rebinfactor;
    //calculate the index in the rebinned frame//
    //that newRow and newCol belongs to//
    const size_t newIndex = newRow*newCols + newCol;
    //add this index value to the newIndex value//
    _tmp[newIndex] += f[iIdx]/dp._rebinfactor/dp._rebinfactor;
  }
  //now I should copy the frame back...
  f.resize(newRows*newCols);
  std::copy(_tmp.begin(), _tmp.end(), f.begin());

}
