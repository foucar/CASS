// Copyright (C) 2009 jk, lmf, nc
#include <iostream>
#include <fstream>
#include <cmath>
#include "pnccd_analysis.h"
#include "pnccd_event.h"
#include "cass_event.h"
//#include "remi_event.h"
#include "pnccd_analysis_lib.h"

#include <vector>
#include <sys/time.h>

QMutex cass::pnCCD::Analysis::_mutex;


void cass::pnCCD::Parameter::load()
{
  //the dark-frames calculations are not threadable for the moment
  // do improve it
  std::cout <<"loading Params"<<std::endl;
  //sting for the container index//
  QString s;
  //sync before loading//
  sync();
  //clear the containers before adding new stuff to them//
  _rebinfactors.clear();
  _darkcal_fnames.clear();

  //added the next..
  _save_darkcal_fnames.clear();
  _sigmaMultiplier.clear();
  _adu2eV.clear();
  _dampingCoefficient.clear();

  //clear the containers before adding new stuff to them//
  detROI.clear();
/*
  _pnCCDOriginalRows.clear();
  _pnCCDOriginalCols.clear();
*/
  //the light indicator channel of the acqiris//
  _lightIndicatorChannel = value("LightIndicatorChannel",9).toUInt();
  _This_is_a_dark_run = value("This_is_a_dark_run",0).toBool();
  for (size_t iDet=0; iDet<value("size",1).toUInt(); ++iDet)
  {
    beginGroup(s.setNum(static_cast<int>(iDet)));
    //the rebin factors for the detector//
    _rebinfactors.push_back(value("RebinFactor",1).toUInt());
    //the positions of the darkframe calibration data for the detectors//
    _darkcal_fnames.push_back(
        value("DarkCalibrationFilePath","darkcal.darkcal").toString().toStdString());
    //the positions of the darkframe calibration data for afterwards saving//
    _save_darkcal_fnames.push_back(
        value("DarkCalibrationSaveFilePath","darkcal_save.darkcal").toString().toStdString());
    //the multiplier for the noise//
    _sigmaMultiplier.push_back(value("SigmaMultiplier",4).toDouble());
    //the conversion factor for adu's to eV//
    _adu2eV.push_back(value("Adu2eV",1).toDouble());
    //the daming coefficent//
    _dampingCoefficient.push_back(value("DampingCoefficient",5).toDouble());

//    _pnCCDOriginalRows.push_back(1024);
//    _pnCCDOriginalCols.push_back(1024);

    detROI.push_back(detROI_t());
    detROI[iDet].detID=iDet;
    beginGroup("ROIs");
    for (size_t iROI=0; iROI<value("ROIsize",1).toUInt(); ++iROI)
    {
      beginGroup(s.setNum(static_cast<uint32_t>(iROI)));
      detROI[iDet]._ROI.push_back(ROIsimple());
      //the shape of the ROI//
      detROI[iDet]._ROI[iROI].name = value("ROIShapeName","square").toString().toStdString();
      // the size(s) along the x axis
      detROI[iDet]._ROI[iROI].xsize = value("XSize",1).toUInt();
      // the size(s) along the y axis
      detROI[iDet]._ROI[iROI].ysize = value("YSize",1).toUInt();
      // the centre(s) along the x axis
      detROI[iDet]._ROI[iROI].xcentre = value("XCentre",1).toUInt();
      // the centre(s) along the y axis
      detROI[iDet]._ROI[iROI].ycentre = value("YCentre",1).toUInt();
      // the orientation is used only in the case of a triangular shape
      detROI[iDet]._ROI[iROI].orientation = value("orientation",1).toInt();
      endGroup();
      std::cout <<"ROI loaded "<< iROI << " " << detROI[iDet]._ROI[iROI].xsize 
                <<" " << detROI[iDet]._ROI[iROI].ysize 
                <<" " << detROI[iDet]._ROI[iROI].xcentre
                <<" " << detROI[iDet]._ROI[iROI].ycentre 
                <<" " << detROI[iDet]._ROI[iROI].orientation<<std::endl;
    }
    endGroup();
    std::cout << "done ROI load "<< detROI[iDet]._ROI.size() << " of pnCCD" << iDet << std::endl;
    std::cout << "global size of pnCCD ROIs " << detROI.size() << std::endl;

    endGroup();
  }
  std::cout << "done"<< std::endl;
}

//------------------------------------------------------------------------------
void cass::pnCCD::Parameter::save()
{
  //sting for the container index//
  QString s;
  setValue("LightIndicatorChannel",_lightIndicatorChannel);
  setValue("This_is_a_dark_run",_This_is_a_dark_run);
  setValue("size",static_cast<uint32_t>(_rebinfactors.size()));
  for (size_t iDet=0; iDet<_rebinfactors.size(); ++iDet)
  {
    beginGroup(s.setNum(static_cast<int>(iDet)));
    setValue("RebinFactor",_rebinfactors[iDet]);
    setValue("DarkCalibrationFilePath",_darkcal_fnames[iDet].c_str());
    setValue("SigmaMultiplier",_sigmaMultiplier[iDet]);
    setValue("Adu2eV",_adu2eV[iDet]);
    setValue("DampingCoefficient",_dampingCoefficient[iDet]);

    setValue("ROIsize",static_cast<uint32_t>(detROI[iDet]._ROI.size()));
    beginGroup("ROIs");
    for (size_t iROI=0; iROI< detROI[iDet]._ROI.size(); ++iROI)
    {
        beginGroup(s.setNum(static_cast<int>(iROI)));
        setValue("ROIShapeName",detROI[iDet]._ROI[iROI].name.c_str());
        setValue("XSize",detROI[iDet]._ROI[iROI].xsize);
        setValue("YSize",detROI[iDet]._ROI[iROI].ysize);
        setValue("XCentre",detROI[iDet]._ROI[iROI].xcentre);
        setValue("YCentre",detROI[iDet]._ROI[iROI].ycentre);
        // the orientation is used only in the case of a triangular shape
        setValue("orientation",detROI[iDet]._ROI[iROI].orientation);
        endGroup();
    }
    endGroup();

    endGroup();
  }
}


//______________________________________________________________________________



int timeval_subtract(struct timeval *result, struct timeval *t2, struct timeval *t1)
{
    long int diff = (t2->tv_usec + 1000000 * t2->tv_sec) - (t1->tv_usec + 1000000 * t1->tv_sec);
    result->tv_sec = diff / 1000000;
    result->tv_usec = diff % 1000000;

    return (diff<0);
}


long long int timeDiff(struct timespec* end, struct timespec* start) {
  long long int diff;
  diff =  (end->tv_sec - start->tv_sec) * 1000000000;
  diff += end->tv_nsec;
  diff -= start->tv_nsec;
  return diff;
}


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
  std::cout <<"load pnCCD settings"<<std::endl;
  QMutexLocker locker(&_mutex);
  //load the settings
  _param.load();
  //resize the vector containers to the right size//
  _param._offsets.resize(_param._darkcal_fnames.size());
  _param._noise.resize(_param._darkcal_fnames.size());
  _param._nbrDarkframes.resize(_param._darkcal_fnames.size());
  _param._ROImask.resize(_param._darkcal_fnames.size());
  _param._ROIiterator.resize(_param._darkcal_fnames.size());
  size_t number_of_pixelsettozero=0;
  //std::cout<<"size "<< _param._nbrDarkframes.size()<<std::endl;
  // Set the dark calibration data in the new analysis instance//
  for(size_t i=0; i<_param._darkcal_fnames.size() ;++i)
  {
//    _pnccd_analyzer[i]->loadDarkCalDataFromFile(_param._darkcal_fnames[i]);
    //open the file that should contain the darkframes//
    ifstream in(_param._darkcal_fnames[i].c_str(), std::ios::binary|std::ios::ate);
    if (in.is_open())
    {
      std::cout <<"reading pnccd "<<i<<" from file \""<<_param._darkcal_fnames[i].c_str()<<"\""<<std::endl;
      //find how big the vectors have to be//
      const size_t size = in.tellg() / 2 / sizeof(double);
      //go to the beginning of the file
      in.seekg(0,std::ios::beg);
      //resize the vectors to the right size//
      _param._offsets[i].resize(size);
      _param._noise[i].resize(size);
      _param._ROImask[i].resize(size);
      _param._ROIiterator[i].resize(size);
      //read the parameters stored in the file//
      //add the next??
      //in.read(reinterpret_cast<char*>(&(_param._nbrDarkframes[i])), sizeof(double));
      in.read(reinterpret_cast<char*>(&(_param._offsets[i][0])), _param._offsets[i].size()*sizeof(double));
      in.read(reinterpret_cast<char*>(&(_param._noise[i][0])), _param._noise[i].size()*sizeof(double));
      std::cout<<"Darkframes "<< _param._nbrDarkframes[i]<<std::endl;

    }
    else
    {
      //safe net in case there is no file yet
      _param._offsets[i].resize(1024 * 1024);
      _param._noise[i].resize(1024 * 1024);
      _param._ROImask[i].resize(1024 * 1024);
      _param._ROIiterator[i].resize(1024 * 1024);

//      _param._offsets[i].resize(_param.pnCCDoriginalrows[i] * _param.pnCCDoriginalcols[i]);
//      _param._noise[i].resize(_param.pnCCDoriginalrows[i] * _param.pnCCDoriginalcols[i]);
    }
  }
  //if this is a darkcalibration run I reset the values in the frames
  if(_param._This_is_a_dark_run==1)
  {
    for(size_t i=0; i<_param._darkcal_fnames.size() ;++i)
    {
      //std::cout<<"safe size "<<_param._offsets[i].size()<<std::endl;
      _param._offsets[i].assign(_param._offsets[i].size(),0);
      _param._noise[i].assign(_param._noise[i].size(),0);
      //_param._nbrDarkframes[i].push_back(0);
    }
    _param._nbrDarkframes.resize(_param._darkcal_fnames.size(),0);
  }

  // I need the ROI only if I am really running with the FEL
  if(_param._This_is_a_dark_run==0)
  {
    std::cout <<"rebin size is "<<_param._rebinfactors.size()<<std::endl;
    for(size_t iDet=0;iDet<_param._rebinfactors.size(); ++iDet)
    {
      // "enable" all  pixel as default
      _param._ROImask[iDet].assign(_param._ROImask[iDet].size(),1);
      for(size_t iROI=0;iROI<_param.detROI[iDet]._ROI.size(); ++iROI)
      {
        size_t index_of_center=_param.detROI[iDet]._ROI[iROI].xcentre
            + 1024 * _param.detROI[iDet]._ROI[iROI].ycentre;
        size_t index_min=_param.detROI[iDet]._ROI[iROI].xcentre - _param.detROI[iDet]._ROI[iROI].xsize
            + 1024 * (_param.detROI[iDet]._ROI[iROI].ycentre - _param.detROI[iDet]._ROI[iROI].ysize);
        size_t index_max=_param.detROI[iDet]._ROI[iROI].xcentre + _param.detROI[iDet]._ROI[iROI].xsize
            + 1024 * (_param.detROI[iDet]._ROI[iROI].ycentre + _param.detROI[iDet]._ROI[iROI].ysize);
        std::cout << "indexes "<< index_of_center<<" "<<index_min<<" "<<index_max<<std::endl;
        size_t indexROI_min=0;
        size_t indexROI_max=(2 * _param.detROI[iDet]._ROI[iROI].xsize + 1)
            * (2 * _param.detROI[iDet]._ROI[iROI].ysize + 1);
        //remember how many pixels I have masked
        //number_of_pixelsettozero+=indexROI_max;
        std::cout << "indexes "<< index_of_center<<" "<<indexROI_min<<" "<<indexROI_max<<std::endl;
        if(_param.detROI[iDet]._ROI[iROI].name=="circ" || _param.detROI[iDet]._ROI[iROI].name=="circle"  )
        {
          int32_t  xlocal,ylocal;
          const uint32_t radius2 =  static_cast<uint32_t>(pow(_param.detROI[iDet]._ROI[iROI].xsize,2)
           /*+pow(_param.detROI[iDet]._ROI[iROI].ysize,2) */);
          std::cout << "circ seen with radius^2= " <<radius2 <<std::endl;
          for(size_t iFrame=indexROI_min;iFrame<indexROI_max; ++iFrame)
          {
            xlocal=iFrame%(2* _param.detROI[iDet]._ROI[iROI].xsize + 1);
            ylocal=iFrame/(2* _param.detROI[iDet]._ROI[iROI].xsize + 1);
#ifdef debug
            std::cout<<"local "<<xlocal<<" "<<ylocal<<" "<<_param.detROI[iDet]._ROI[iROI].xsize<< " " 
                     <<pow(xlocal-static_cast<int32_t>(_param.detROI[iDet]._ROI[iROI].xsize),2)
                     <<std::endl;
#endif
            if( ( pow(xlocal-static_cast<int32_t>(_param.detROI[iDet]._ROI[iROI].xsize),2) +
                  pow(ylocal-static_cast<int32_t>(_param.detROI[iDet]._ROI[iROI].ysize),2) ) <= radius2 )
            {
              _param._ROImask[iDet][index_min+xlocal+ 1024 * (ylocal ) ]=0;
              //remember how many pixels I have masked
              number_of_pixelsettozero++;
#ifdef debug
              std::cout<<"in local "<<xlocal<<" "<<ylocal <<std::endl;
#endif
            }
          }
        }
        if(_param.detROI[iDet]._ROI[iROI].name=="square")
        {
          uint32_t  xlocal,ylocal;
          std::cout << "square seen" <<std::endl;
          for(size_t iFrame=indexROI_min;iFrame<indexROI_max; ++iFrame)
          {
            xlocal=iFrame%(2* _param.detROI[iDet]._ROI[iROI].xsize +1);
            ylocal=iFrame/(2* _param.detROI[iDet]._ROI[iROI].xsize +1);
            _param._ROImask[iDet][index_min+xlocal+ 1024 * (ylocal) ]=0;
            //remember how many pixels I have masked
            number_of_pixelsettozero++;
#ifdef debug
            std::cout<<"local "<<xlocal<<" "<<ylocal<<" "<<_param.detROI[iDet]._ROI[iROI].ycentre
                     << " "<<_param.detROI[iDet]._ROI[iROI].ycentre - _param.detROI[iDet]._ROI[iROI].ysize
                     <<std::endl;
#endif
          }
        }
        if(_param.detROI[iDet]._ROI[iROI].name=="triangle")
        {
          int32_t  xlocal,ylocal;
          float xlocal_f,ylocal_f;
          float xsize,ysize;
          xsize=static_cast<float>(_param.detROI[iDet]._ROI[iROI].xsize);
          ysize=static_cast<float>(_param.detROI[iDet]._ROI[iROI].ysize);

          std::cout << "triangle seen" <<std::endl;
          if(_param.detROI[iDet]._ROI[iROI].orientation==+1)
          {
            std::cout << "triangle seen vertex upwards" <<std::endl;
            //the triangle is at least isosceles
            for(size_t iFrame=indexROI_min;iFrame<indexROI_max; ++iFrame)
            {
              xlocal=iFrame%(2* _param.detROI[iDet]._ROI[iROI].xsize + 1);
              ylocal=iFrame/(2* _param.detROI[iDet]._ROI[iROI].xsize + 1);
              xlocal_f=static_cast<float>(xlocal);
              ylocal_f=static_cast<float>(ylocal);

#ifdef debug
              std::cout<<"local "<<xlocal<<" "<<ylocal
                       << " " <<2 * ysize/xsize*xlocal_f << " " <<4*ysize - 2* ysize/xsize*xlocal_f
                       <<std::endl;
#endif
              if(ylocal-1<(2 * ysize/xsize*xlocal_f)
                 && xlocal<static_cast<int32_t>(_param.detROI[iDet]._ROI[iROI].xsize + 1))
              {
#ifdef debug
                std::cout<<"local1 "<<xlocal<<" "<<ylocal <<std::endl;
#endif
                _param._ROImask[iDet][index_min+xlocal+ 1024 * (ylocal) ]=0;
                //remember how many pixels I have masked
                number_of_pixelsettozero++;
              }
              else if(ylocal-1<(4*ysize - 2* ysize/xsize*xlocal_f)
                      && xlocal>static_cast<int32_t>(_param.detROI[iDet]._ROI[iROI].xsize))
              {
#ifdef debug
                std::cout<<"local2 "<<xlocal<<" "<<ylocal <<std::endl;
#endif
                _param._ROImask[iDet][index_min+xlocal+ 1024 * (ylocal) ]=0;
                //remember how many pixels I have masked
                number_of_pixelsettozero++;
              }
            }
          }
          if(_param.detROI[iDet]._ROI[iROI].orientation==-1)
          {
            std::cout << "triangle seen vertex downwards" <<std::endl;
            for(size_t iFrame=indexROI_min;iFrame<indexROI_max; ++iFrame)
            {
              xlocal=iFrame%(2* _param.detROI[iDet]._ROI[iROI].xsize + 1);
              ylocal=iFrame/(2* _param.detROI[iDet]._ROI[iROI].xsize + 1);
              xlocal_f=static_cast<float>(xlocal);
              ylocal_f=static_cast<float>(ylocal);
#ifdef debug
              std::cout<<"local "<<xlocal<<" "<<ylocal
                       << " " <<(-2 * ysize/xsize*xlocal_f) + 2 * ysize << " "<<-2*ysize + 2 *  ysize/xsize*xlocal
                       <<std::endl;
#endif

              if(ylocal+1>((-2 * ysize/xsize*xlocal_f)
                         + 2 * ysize)
                 && xlocal<static_cast<int32_t>(_param.detROI[iDet]._ROI[iROI].xsize + 1))
              {
#ifdef debug
                std::cout<<"local1 "<<xlocal<<" "<<ylocal <<std::endl;
#endif
                _param._ROImask[iDet][index_min+xlocal+ 1024 * (ylocal) ]=0;
                //remember how many pixels I have masked
                number_of_pixelsettozero++;
              }
              else if(ylocal+1>(-2*ysize +
                              2 * ysize/xsize*xlocal_f)
                      && xlocal>static_cast<int32_t>(_param.detROI[iDet]._ROI[iROI].xsize))
              {
#ifdef debug
                std::cout<<"local2 "<<xlocal<<" "<<ylocal <<std::endl;
#endif
                _param._ROImask[iDet][index_min+xlocal+ 1024 * (ylocal) ]=0;
                //remember how many pixels I have masked
                number_of_pixelsettozero++;
              }
            }
          }
          if(_param.detROI[iDet]._ROI[iROI].orientation==+2)
          {
            std::cout << "triangle seen vertex towards right" <<std::endl;
            for(size_t iFrame=indexROI_min;iFrame<indexROI_max; ++iFrame)
            {
              // not debugged
              xlocal=iFrame%(2* _param.detROI[iDet]._ROI[iROI].xsize + 1);
              ylocal=iFrame/(2* _param.detROI[iDet]._ROI[iROI].xsize + 1);
              xlocal_f=static_cast<float>(xlocal);
              ylocal_f=static_cast<float>(ylocal);
#ifdef debug
              std::cout<<"local "<<xlocal<<" "<<ylocal
                       << " " <<(ysize/xsize*xlocal_f) << " "<<- ysize/xsize*xlocal_f + 4 * ylocal_f
                       <<std::endl;
#endif
              if(ylocal_f>(ysize/(2*xsize) * xlocal_f) && ylocal_f< (-ysize/(2*xsize)*xlocal_f + 2 * ysize) )
              {
                _param._ROImask[iDet][index_min+xlocal+ 1024 * (ylocal) ]=0;
                //remember how many pixels I have masked
                number_of_pixelsettozero++;
              }
            }
          }
          if(_param.detROI[iDet]._ROI[iROI].orientation==-2)
          {
            std::cout << "triangle seen vertex towards left" <<std::endl;
            for(size_t iFrame=indexROI_min;iFrame<indexROI_max; ++iFrame)
            {
              // not debugged
              xlocal=iFrame%(2* _param.detROI[iDet]._ROI[iROI].xsize);
              ylocal=iFrame/(2* _param.detROI[iDet]._ROI[iROI].xsize);
              xlocal_f=static_cast<float>(xlocal);
              ylocal_f=static_cast<float>(ylocal);
#ifdef debug
              std::cout<<"local "<<xlocal<<" "<<ylocal<<" "
                       <<std::endl;
#endif
              if(ylocal>(- ysize/(2*xsize) * xlocal_f + ysize) && ylocal<( ysize/(2*xsize) * xlocal_f + ysize) )
              {
                _param._ROImask[iDet][index_min+xlocal+ 1024 * (ylocal) ]=0;
                //remember how many pixels I have masked
                number_of_pixelsettozero++;
              }
            }
          }
        }
      } // end iROI loop

//#ifdef debug
      std::cout <<"ROI "<< iDet<<" "<<_param._ROImask[iDet].size()<<" ";
      for(size_t i=0;i<_param._ROImask[iDet].size();i++)
          std::cout << _param._ROImask[iDet][i]<< " ";
      std::cout<<std::endl;
//#endif
      // now I know which pixel should be masked!
      /*
      for(size_t iPixel=0;iPixel<_param._ROIiterator[iDet].size(); ++iPixel)
      {
        // the "pointer" is the location/index of the next pixel to be used, and the default should be that a[i]=i+1
        _param._ROIiterator[iDet][iPixel]=iPixel+1;
      }
      */
      size_t nextPixel=1;
      _param._ROIiterator[iDet].resize(_param._ROImask[iDet].size()-number_of_pixelsettozero);
      for(size_t iPixel=0;iPixel<_param._ROImask[iDet].size(); ++iPixel)
      {
        // the "pointer" is the location/index of the next pixel to be used
        if(_param._ROImask[iDet][iPixel]!=0)
        {
          _param._ROIiterator[iDet][nextPixel]=iPixel+1;
          nextPixel++;
        }
      }
      std::cout <<"Roiit sizes "<< iDet<<" "<<_param._ROImask[iDet].size()<<" " 
                <<_param._ROIiterator[iDet].size() <<std::endl;
      for(size_t i=0;i<_param._ROIiterator[iDet].size();i++)
      {
        if(i%16==0) std::cout <<"Roiit"<<iDet<<" ";
        std::cout << _param._ROIiterator[iDet][i]<< " ";
        if(i%16==15) std::cout<<std::endl;
      }
      std::cout<<std::endl;
      
    } // end iDet loop
  } // end This_is_a_dark_run if
  std::cout <<"done loading pnCCD settings"<<std::endl;
}

//------------------------------------------------------------------------------
void cass::pnCCD::Analysis::saveSettings()
{
  //save settings//
  _param.save();
  //now save the noise and the offset vectors to the designated files//
  // and this means: only if it was a dark run!!
  if(_param._This_is_a_dark_run==1)
  {
    QMutexLocker locker(&_mutex);
    for (size_t i=0; i<_param._save_darkcal_fnames.size() ; ++i)
    {
      //only if the vectors have some information inside//
      if (!_param._offsets[i].empty() && !_param._noise[i].empty())
      {
#ifdef debug_darkmaps
        std::cout<<"sizes "<<_param._offsets[i].size()<<" "<<_param._noise[i].size()<<std::endl;
#endif
        //normalise the values
        for(size_t j=0;j<_param._offsets[i].size();j++)
        {
#ifdef debug_darkmaps
          std::cout<<"off "<<i<<" "<<_param._offsets[i][j] << " " <<_param._nbrDarkframes[i]<<" "<<
              _param._offsets[i][j]/_param._nbrDarkframes[i]<<" ";
#endif
          _param._offsets[i][j]=_param._offsets[i][j]/_param._nbrDarkframes[i];
#ifdef debug_darkmaps
          std::cout<<_param._offsets[i][j]<<" ";
          std::cout<<"noise "<<_param._noise[i][j] << " "<< _param._noise[i][j]/_param._nbrDarkframes[i]<<" "
                   <<_param._offsets[i][j] * _param._offsets[i][j]<<" " <<
              _param._noise[i][j] / _param._nbrDarkframes[i] - _param._offsets[i][j] * _param._offsets[i][j];
#endif
          _param._noise[i][j]=sqrt(
              _param._noise[i][j] / _param._nbrDarkframes[i] - _param._offsets[i][j] * _param._offsets[i][j]);
#ifdef debug_darkmaps
          std::cout<<" "<<    _param._noise[i][j] <<" "<< j <<std::endl;
#endif
        }
/*
        for(size_t j=0;j<_param._offsets[i].size();j++)
        {
#ifdef debug_darkmaps
          std::cout<<"off "<<_param._offsets[i][j] << " " <<_param._nbrDarkframes[i]<<" ";
#endif
          _param._offsets[i][j]=_param._offsets[i][j]/_param._nbrDarkframes[i];
#ifdef debug_darkmaps
          std::cout<<_param._offsets[i][j] <<std::endl;
#endif
        }
*/
/*
        for(size_t j=0;j<_param._noise[i].size();j++)
        {
#ifdef debug_darkmaps
          std::cout<<"noise "<<_param._noise[i][j]/_param._nbrDarkframes[i] << " " 
                   <<_param._offsets[i][j] * _param._offsets[i][j]<<" "
                   << _param._noise[i][j]/_param._nbrDarkframes[i] - _param._offsets[i][j] 
              * _param._offsets[i][j]<<" ";
#endif
          _param._noise[i][j]=sqrt(
              _param._noise[i][j] / _param._nbrDarkframes[i] - _param._offsets[i][j] * _param._offsets[i][j]);
#ifdef debug_darkmaps
          std::cout<<_param._noise[i][j] <<std::endl;
#endif
        }
*/
        //create a output file//
        ofstream out(_param._save_darkcal_fnames[i].c_str(), std::ios::binary);
        if (out.is_open())
        {
          std::cout <<"writing pnccd "<<i<<" to file \""<<_param._save_darkcal_fnames[i].c_str()<<"\""<<std::endl;
          //write the parameters to the file//
          //add the next??
          //out.write(reinterpret_cast<char*>(&(_param._nbrDarkframes[i])), sizeof(double));
          out.write(reinterpret_cast<char*>(&(_param._offsets[i][0])), _param._offsets[i].size()*sizeof(double));
          out.write(reinterpret_cast<char*>(&(_param._noise[i][0])), _param._noise[i].size()*sizeof(double));
        }
      }
    }
  }
}

//------------------------------------------------------------------------------
void cass::pnCCD::Analysis::operator()(cass::CASSEvent* cassevent)
{
  struct timeval tvBegin, tvEnd, tvDiff;
  struct timespec start, now;
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
     (pnccdevent.detectors().size() > _param._dampingCoefficient.size()) ||
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
    _param._dampingCoefficient.resize(pnccdevent.detectors().size(),5);
    saveSettings();
  }

  //   //check if we have enough analyzers for the amount of detectors//
  //   //increase it if necessary
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
#ifdef bit32
#ifdef debug
    std::cout << "signed 32 bits version" <<std::endl;
#endif
    cass::pnCCD::pnCCDDetector::frame_i32_t &cf = det.correctedFrame();
#elif defined(fbit32)
#ifdef debug
    std::cout << "float 32 bits version" <<std::endl;
#endif
    cass::pnCCD::pnCCDDetector::frame_f32_t &cf = det.correctedFrame();
#elif defined(every)
#ifdef debug
    std::cout << "all versions" <<std::endl;
#endif
    cass::pnCCD::pnCCDDetector::frame_i32_t &cf32i = det.correctedFrame32i();
    cass::pnCCD::pnCCDDetector::frame_f32_t &cf32f = det.correctedFrame32f();
    cass::pnCCD::pnCCDDetector::frame_t &cf = det.correctedFrame16u();
#else
#ifdef debug
    std::cout << "unsigned 16 bits version" <<std::endl;
#endif
    cass::pnCCD::pnCCDDetector::frame_t &cf = det.correctedFrame();
#endif
    //retrieve a reference to the raw frame of the detector//
    const cass::pnCCD::pnCCDDetector::frame_t &rf = det.rawFrame();
    static size_t ii=0;

    //retrieve a reference to the ROI mask and/or iterator
    const std::vector<uint16_t> &mask = _param._ROImask[iDet];
    const std::vector<uint32_t> &iter = _param._ROIiterator[iDet];

    //retrieve a reference to the nonrecombined photon hits of the detector//
    //cass::pnCCD::pnCCDDetector::photonHits_t &phs = det.nonrecombined();
    //retrieve a reference to the noise vector of this detector//
    std::vector<double> &noise = _param._noise[iDet];
    //retrieve a reference to the offset of the detector//
    std::vector<double> &offset = _param._offsets[iDet];
    //retrieve a reference to the number of Darkframes that were taken for this the detector//
    size_t &nDarkframes = _param._nbrDarkframes[iDet];
    //retrieve a reference to the multiplier of the sigma of the noise//
    //const double &sigmaMultiplier = _param._sigmaMultiplier[iDet];
    //retrieve a reference to the conversionfactor from "adu" to eV of the noise//
    //const double &adu2eV = _param._adu2eV[iDet];
    //retrieve a reference to the rebinfactor of this detector//
    uint32_t &rebinfactor = _param._rebinfactors[iDet];
    //retrieve a reference to the damping coefficent of this detector//
    // I am not using it anymore??
    //const double damping = _param._dampingCoefficient[iDet];

//     std::cout<<iDet<< " "<<pnccdevent.detectors().size()<<" "<< det.rows() << " " <<  det.columns() << " " << det.originalrows() << " " <<det.originalcolumns()<<" "<<rf.size()<< " "<<_pnccd_analyzer[iDet]<<std::endl;

    //if the size of the rawframe is 0, this detector with this id is not in the datastream//
    //so we are not going to anlyse this detector further//
    if (rf.empty()) 
      continue;

    //resize the corrected frame container the noise and the offset to the size of the raw frame container//
    //this code relies on the fact, that the frame size won't change at runtime//
#ifdef bit32
    cf.resize(det.rawFrame().size());
#elif defined(fbit32)
    cf.resize(det.rawFrame().size());
#elif defined(every)
    cf32i.resize(det.rawFrame().size());
    cf32f.resize(det.rawFrame().size());
    cf.resize(det.rawFrame().size());
#else
    cf.resize(det.rawFrame().size());
#endif
    noise.resize(det.rawFrame().size());
    offset.resize(det.rawFrame().size());
    //for now just rearrange the frame so that it looks like a real frame//
    //get the dimesions of the detector before the rebinning//
    const uint16_t nRows = det.originalrows();
    const uint16_t nCols = det.originalcolumns();

    if(_param._This_is_a_dark_run==0)
    {
      std::cout<<"this is not a dark run"<<std::endl;

#ifdef no_dark_possible
      //take the average over the first and last 3 rows//
      cass::pnCCD::pnCCDDetector::frame_t::const_iterator itFrame = rf.begin();
      cass::pnCCD::pnCCDDetector::frame_t::const_iterator ritFrame = rf.end()-1024*3;
    
      for (size_t iOff=0; iOff<1024*3;++itFrame,++ritFrame,++iOff)
      {
        offset[iOff%1024] = (offset[iOff%1024] * damping + *itFrame) / (damping+1.);
        noise[iOff%1024]  = ( noise[iOff%1024] * damping + *ritFrame)/ (damping+1.);
        //                                                 ^this seems an error!!
      }
      ++nDarkframes;

      //do the selfmade "massaging" of the detector//
      //only if we have already enough darkframes//
      if (nDarkframes > 1)
      {
#endif
        //std::cout <<"we fill the corrected frame"<<std::endl;
        std::vector<double>::iterator itOffset = offset.begin();
        std::vector<double>::iterator itNoise  = noise.begin();
        std::vector<uint32_t>::const_iterator itROI  = iter.begin();
        cass::pnCCD::pnCCDDetector::frame_t::const_iterator itRawFrame = rf.begin();
#ifdef bit32
        cass::pnCCD::pnCCDDetector::frame_i32_t::iterator itCorFrame = cf.begin();
#elif defined(fbit32)
        cass::pnCCD::pnCCDDetector::frame_f32_t::iterator itCorFrame = cf.begin();
#elif defined(every)
        cass::pnCCD::pnCCDDetector::frame_i32_t::iterator itCorFrame32i = cf32i.begin();
        cass::pnCCD::pnCCDDetector::frame_f32_t::iterator itCorFrame32f = cf32f.begin();
        cass::pnCCD::pnCCDDetector::frame_t::iterator itCorFrame16u = cf.begin();
#else
        cass::pnCCD::pnCCDDetector::frame_t::iterator itCorFrame = cf.begin();
#endif
        size_t pixelidx=0;
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
        gettimeofday(&tvBegin, NULL);
#ifdef every
        for ( ; itRawFrame != rf.end(); ++itRawFrame,++itCorFrame32i,
               ++itCorFrame32f,++itCorFrame16u,++itOffset,++pixelidx)
#else
#define simple
#ifdef simple
        for ( ; itRawFrame != rf.end() ; ++itRawFrame,++itCorFrame,++itOffset,++pixelidx)
#else
        for ( ; itROI != iter.end() /*&& skip the last few pixels*/ ; ++itROI)
#endif
#endif
        {
#ifndef simple
          itRawFrame=itROI;
          itCorFrame=itROI;
          itOffset=itROI;
#endif
          //statistics//
          //  const double mean =  *itOffset / nDarkframes;
          const double mean = *itOffset; 
          //const double mean = (pixelidx < 1024*512) ? offset[pixelidx%1024] : noise[pixelidx%1024]; 
          //  const double meansquared =  mean * mean;
          //  const double sumofsquare = *itNoise;
          //  const double sigma = sqrt( 1/nDarkframes * sumofsquare - meansquared ); 
          //remove the offset of the frame and copy it into the corrected frame//
          //*itCorFrame = static_cast<int16_t>(*itRawFrame - mean);
          // I could reset to zero if negative.. This would allow to use uint16 instead of int16...
          // and give wider range, I have anyway to do some "If" statements
#ifdef bit32
          if(*itRawFrame> mean) *itCorFrame = static_cast<int32_t>(*itRawFrame - mean);
          else *itCorFrame=0;
#elif defined(fbit32)
          if(*itRawFrame> mean) *itCorFrame = static_cast<float>(*itRawFrame - mean);
          else *itCorFrame=0;
#elif defined(every)
          if(*itRawFrame> mean)
          {
            *itCorFrame32i = static_cast<int32_t>(*itRawFrame - mean);
            *itCorFrame32f = static_cast<float>(*itRawFrame - mean);
            *itCorFrame16u = static_cast<uint16_t>(*itRawFrame - mean);
          }
          else
          {
            *itCorFrame32i=0;
            *itCorFrame32f=0;
            *itCorFrame16u=0;
          }
#else
          if(*itRawFrame> mean) *itCorFrame = static_cast<uint16_t>(*itRawFrame - mean);
          else *itCorFrame=0;
#endif
          //find out whether this pixel is a photon hit//
          /*if (*itCorFrame > (sigmaMultiplier * sigma) )
          {
          //create a photon hit//
          cass::pnCCD::PhotonHit ph;
          //set the values of the photon hit//
          //todo : find the positions that are clear only after the rearrangement//
          ph.amplitude() = *itCorFrame;
          ph.energy()    = ph.amplitude() * adu2eV;
          //add it to the vector of photon hits//
          phs.push_back(ph);
          }*/
        }
        gettimeofday(&tvEnd, NULL);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &now);
        
#ifdef no_dark_possible
      }
      else
      {
        cass::pnCCD::pnCCDDetector::frame_t::const_iterator itRawFrame = rf.begin();
        gettimeofday(&tvBegin, NULL);
#ifdef bit32
        cass::pnCCD::pnCCDDetector::frame_i32_t::iterator itCorFrame = cf.begin();
        for ( ; itRawFrame != rf.end(); ++itRawFrame,++itCorFrame)
            *itCorFrame = static_cast<int32_t>(*itRawFrame);
#elif defined(fbit32)
        cass::pnCCD::pnCCDDetector::frame_f32_t::iterator itCorFrame = cf.begin();
        for ( ; itRawFrame != rf.end(); ++itRawFrame,++itCorFrame)
            *itCorFrame = static_cast<float>(*itRawFrame);
#elif defined(every)
        cass::pnCCD::pnCCDDetector::frame_i32_t::iterator itCorFrame32i = cf32i.begin();
        cass::pnCCD::pnCCDDetector::frame_f32_t::iterator itCorFrame32f = cf32f.begin();
        cass::pnCCD::pnCCDDetector::frame_t::iterator itCorFrame16u = cf.begin();
        for ( ; itRawFrame != rf.end(); ++itRawFrame,++itCorFrame32i,
                  ++itCorFrame32f,++itCorFrame16u)
        {
          *itCorFrame32i = static_cast<int32_t>(*itRawFrame);
          *itCorFrame32f = static_cast<float>(*itRawFrame);
          *itCorFrame16u = static_cast<uint16_t>(*itRawFrame);
        }
#else
        cass::pnCCD::pnCCDDetector::frame_t::iterator itCorFrame = cf.begin();
        for ( ; itRawFrame != rf.end(); ++itRawFrame,++itCorFrame)
            *itCorFrame = static_cast<uint16_t>(*itRawFrame);
#endif
        gettimeofday(&tvEnd, NULL);

      }
#endif
      timeval_subtract(&tvDiff, &tvEnd, &tvBegin);
#ifdef debug
      printf("time_diff is %ld.%06ld\n", tvDiff.tv_sec, tvDiff.tv_usec);
#endif
      printf("doing_math_took %lld ns\n", timeDiff(&now, &start));

      //calc the integral (the sum of all bins)//
      det.integral() = 0;
      for (size_t iInt=0; iInt<cf.size() ;++iInt)
#ifdef fbit32
          det.integral() += static_cast<uint64_t>(cf[iInt]); // in this case it could just be a copy 
                                                           //if det.int would be float32
#else
      det.integral() += static_cast<uint64_t>(cf[iInt]);
#endif

      gettimeofday(&tvBegin, NULL);
      //rebin image frame if requested//
      std::cout << "integrals: " << det.integral() << std::endl;
      if(iDet==1)ii++;
      if(ii<10)
      {
          std::cout << "frame ";
          for (size_t iInt=0; iInt<cf.size() ;++iInt) std::cout <<cf[iInt] << " " ;
          std::cout << std::endl;
      }

      if (rebinfactor != 1)
      {
        //if the rebinfactor doesn't fit the original dimensions//
        //checks whether rebinfactor is of power of 2//
        //look for the next smaller number that is a power of 2//
        if(nRows % rebinfactor != 0)
        {
          rebinfactor = static_cast<uint32_t>(pow(2,int(log2(rebinfactor))));
          saveSettings();
        }
        //get the new dimensions//
        const size_t newRows = nRows / rebinfactor;
        const size_t newCols = nCols / rebinfactor;
        //set the new dimensions in the detector//
        det.rows()    = newRows;
        det.columns() = newCols;
        //resize the temporary container to fit the rebinned image
        //initialize it with 0
#ifdef fbit32
        _tmpf.assign(newRows * newCols,0.);
#else
        _tmp.assign(newRows * newCols,0);
#endif

        //go through the whole corrected frame//
        for (size_t iIdx=0; iIdx<cf.size() ;++iIdx)
        {
          //calculate the row and column of the current Index//
          const size_t row = iIdx / nCols;
          const size_t col = iIdx % nCols;
          //calculate the index of the rebinned frame//
          const size_t newRow = row / rebinfactor;
          const size_t newCol = col / rebinfactor;
          //calculate the index in the rebinned frame//
          //that newRow and newCol belongs to//
          const size_t newIndex = newRow*newCols + newCol;
          //add this index value to the newIndex value//
#ifdef fbit32
          _tmpf[newIndex] += cf[iIdx];
#else
          _tmp[newIndex] += cf[iIdx];
#endif

          /*// instead of doing the prev line, I could:
          #ifdef signedframeonly
          // "stop" at 32k
          // do not add if already over max allowed
          int16_t temp_sum=_tmp[newIndex] + cf[iIdx];
          if( temp_sum > 0)
          {
              _tmp[newIndex] = temp_sum;
          }
          else _tmp[newIndex]=0x7FFF;
          #endif
          uint32_t temp_sum=static_cast<uint32_t>(_tmp[newIndex] + cf[iIdx]);
          if( temp_sum < 0xFFFF)
          {
              _tmp[newIndex] = static_cast<uint16_t>(temp_sum);
          }
          else _tmp[newIndex]=0xFFFF;

          //if(_tmp[newIndex]<0) std::cout << _tmp[newIndex] << " ";
          // "invent" some math
          //_tmp[newIndex] += cf[iIdx];
          //if( _tmp[newIndex] & 0x8000 ) _tmp[newIndex]=0x7FFF;*/
        }
        //make the correctedframe the right size//
        cf.resize(newRows*newCols);
        //copy the tempframe to the corrected frame//
        //make sure that each pixel is scaled to avoid//
        //decreasing the dynamic range//
#ifdef bit32
        std::vector<uint64_t>::const_iterator itTemp = _tmp.begin();
        cass::pnCCD::pnCCDDetector::frame_i32_t::iterator itCorFrame = cf.begin();
        for (; itCorFrame!=cf.end() ;++itCorFrame,++itTemp)
            *itCorFrame = static_cast<int32_t>(*itTemp / (rebinfactor*rebinfactor));
#elif defined(fbit32)
        std::vector<float>::const_iterator itTemp = _tmpf.begin();
        cass::pnCCD::pnCCDDetector::frame_f32_t::iterator itCorFrame = cf.begin();
        for (; itCorFrame!=cf.end() ;++itCorFrame,++itTemp)
            *itCorFrame = static_cast<float>(*itTemp / (rebinfactor*rebinfactor));
#elif defined(every)
        std::vector<uint64_t>::const_iterator itTemp = _tmp.begin();
        cf32i.resize(newRows*newCols);
        cf32f.resize(newRows*newCols);
        cass::pnCCD::pnCCDDetector::frame_t::iterator itCorFrame16u = cf.begin();
        for (; itCorFrame16u!=cf.end() ;++itCorFrame16u,++itTemp)
            *itCorFrame16u = static_cast<uint16_t>(*itTemp / (rebinfactor*rebinfactor));
        itTemp = _tmp.begin();
        cass::pnCCD::pnCCDDetector::frame_i32_t::iterator itCorFrame32i = cf32i.begin();
        for (; itCorFrame32i!=cf32i.end() ;++itCorFrame32i,++itTemp)
            *itCorFrame32i = static_cast<int32_t>(*itTemp / (rebinfactor*rebinfactor));
        itTemp = _tmp.begin();
        cass::pnCCD::pnCCDDetector::frame_f32_t::iterator itCorFrame32f = cf32f.begin();
        for (; itCorFrame32f!=cf32f.end() ;++itCorFrame32f,++itTemp)
            *itCorFrame32f = static_cast<float>(*itTemp / (rebinfactor*rebinfactor));
#else
        std::vector<uint64_t>::const_iterator itTemp = _tmp.begin();
        cass::pnCCD::pnCCDDetector::frame_t::iterator itCorFrame = cf.begin();
        for (; itCorFrame!=cf.end() ;++itCorFrame,++itTemp)
            *itCorFrame = static_cast<uint16_t>(*itTemp / (rebinfactor*rebinfactor));
#endif
      }
      gettimeofday(&tvEnd, NULL);
      timeval_subtract(&tvDiff, &tvEnd, &tvBegin);
      if (rebinfactor != 1) printf("rebin_diff is %ld.%06ld\n", tvDiff.tv_sec, tvDiff.tv_usec);

    }
    else
    {
      QMutexLocker locker(&_mutex);
      cass::pnCCD::pnCCDDetector::frame_t::const_iterator itFrame = rf.begin();
      std::vector<double>::iterator itOffset = offset.begin();
      std::vector<double>::iterator itNoise  = noise.begin();
      //size_t tts=0;
      for (; itOffset!=offset.end() ;++itOffset,++itFrame,++itNoise)
      {
        *itOffset += static_cast<double>(*itFrame);
        *itNoise  += static_cast<double>(*itFrame) * static_cast<double>(*itFrame);
        /*std::cout << "offs/noi/this/this*this "<< tts<<" "
                  << *itOffset <<" "<< *itNoise <<" "<< static_cast<double>(*itFrame)
        <<" "<<static_cast<double>(*itFrame) * static_cast<double>(*itFrame) << std::endl;
        tts++;*/
        /*offset[iOff%1024] = (offset[iOff%1024] * damping + *itFrame) / (damping+1.);
        noise[iOff%1024]  = ( noise[iOff%1024] * damping + *ritFrame)/ (damping+1.);*/
      }
      ++nDarkframes;
      /*std::cout<<"this is supposed to be a dark run ";
      std::cout<<nDarkframes<<" "<<_param._nbrDarkframes[iDet]<<" "<< iDet<<std::endl;*/
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
