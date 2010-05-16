/*
 *  Created by Lutz Foucar on 23.02.2010
 *  Modified by Nicola Coppola from 25.02.2010
 */

#include <cmath>
#include "ccd_analysis.h"
#include "cass_event.h"
#include "pixel_detector.h"
#include "ccd_device.h"


void cass::CCD::Parameter::load()
{
  char printoutdef[40];
  sprintf(printoutdef,"Commercial CCD load ");
  //sync before loading//
  sync();
  //sting for the container index//
  QString s;
  //I may have in the future more than 1 "commercial CCD"
  //I should do like it for pnCCD (passing a index to load/save
  size_t idx=0;
  _This_is_a_dark_run = value("This_is_a_dark_run",0).toBool();
  beginGroup(s.setNum(static_cast<uint32_t>(idx)));
    _threshold    = value("Threshold",0).toUInt();
    _rebinfactor  = value("RebinFactor",1).toUInt();
    _detROI._ROI.clear();

    beginGroup("ROIs");
    for (size_t iROI=0; iROI<value("ROIsize",0).toUInt(); ++iROI)
    {
      beginGroup(s.setNum(static_cast<uint32_t>(iROI)));
      _detROI._ROI.push_back(ROIsimple());
      //the shape of the ROI//
      _detROI._ROI[iROI].name = value("ROIShapeName","square").toString().toStdString();
      // the size(s) along the x axis
      _detROI._ROI[iROI].xsize = value("XSize",1).toUInt();
      // the size(s) along the y axis
      _detROI._ROI[iROI].ysize = value("YSize",1).toUInt();
      // the centre(s) along the x axis
      _detROI._ROI[iROI].xcentre = value("XCentre",1).toUInt();
      // the centre(s) along the y axis
      _detROI._ROI[iROI].ycentre = value("YCentre",1).toUInt();
      // the orientation is used only in the case of a triangular shape
      _detROI._ROI[iROI].orientation = value("orientation",1).toInt();
      endGroup();
      std::cout << printoutdef <<"ROI loaded "<< iROI << " xsiz " << _detROI._ROI[iROI].xsize 
                <<" ysiz " << _detROI._ROI[iROI].ysize 
                <<" xc " << _detROI._ROI[iROI].xcentre
                <<" yc " << _detROI._ROI[iROI].ycentre 
                <<" ori " << _detROI._ROI[iROI].orientation<<std::endl;
    }
    endGroup();
    std::cout << printoutdef << "done ROI load "<< _detROI._ROI.size() << " of commercial CCD" << std::endl;

  endGroup();
}

void cass::CCD::Parameter::save()
{
  QString s;
  size_t idx=0;
  setValue("This_is_a_dark_run",_This_is_a_dark_run);
  beginGroup(s.setNum(static_cast<uint32_t>(idx)));
    setValue("Threshold",_threshold);
    setValue("RebinFactor",_rebinfactor);

    setValue("ROIsize",static_cast<uint32_t>(_detROI._ROI.size()));
    beginGroup("ROIs");
    for (size_t iROI=0; iROI< _detROI._ROI.size(); ++iROI)
    {
        beginGroup(s.setNum(static_cast<int>(iROI)));
        setValue("ROIShapeName",_detROI._ROI[iROI].name.c_str());
        setValue("XSize",_detROI._ROI[iROI].xsize);
        setValue("YSize",_detROI._ROI[iROI].ysize);
        setValue("XCentre",_detROI._ROI[iROI].xcentre);
        setValue("YCentre",_detROI._ROI[iROI].ycentre);
        // the orientation is used only in the case of a triangular shape
        setValue("orientation",_detROI._ROI[iROI].orientation);
        endGroup();
    }
    endGroup();

  endGroup();
}


//------------------------------------------------------------------------------
void cass::CCD::Analysis::loadSettings()
{
  char printoutdef[40];
  sprintf(printoutdef,"Commercial CCD loadSettings ");
  //QMutexLocker locker(&_mutex);
  /*
  //retrieve a pointer to the ccd device we are working on//
  cass::CCD::CCDDevice* dev = dynamic_cast<cass::CCD::CCDDevice*>(cassevent->devices()[cass::CASSEvent::CCD]);
  //retrieve a reference to the pulnix detector//
  const cass::PixelDetector &det = dev->detector();
  const cass::PixelDetector::frame_t& frame      = det.frame();
  */
  //load the settings
  _param.load();
  size_t number_of_pixelsettozero=0;
  _param._ROImask.clear();
  _param._ROIiterator.clear();

  /*if(frame.size()>0)
  {
    _param._ROImask.resize(frame.size());
    _param._ROIiterator.resize(frame.size());
  }
  else
  {*/
    _param._ROImask.resize(CCD::opal_default_size_sq);
    _param._ROIiterator.resize(CCD::opal_default_size_sq);
  /*}*/
  // "enable" all  pixel as default
  _param._ROImask.assign(_param._ROImask.size(),1);
  //I need to reset the number of masked pixel per frame
  number_of_pixelsettozero=0;
  for(size_t iROI=0;iROI<_param._detROI._ROI.size(); ++iROI)
  {
    const size_t half_CCD_def_size=CCD::opal_default_size/2;
    const int32_t sign_CCD_def_size=static_cast<int32_t>(CCD::opal_default_size);
    const int32_t sign_CCD_def_size_sq=static_cast<int32_t>(CCD::opal_default_size_sq);
    const int32_t signed_ROI_xcentre=static_cast<int32_t>(_param._detROI._ROI[iROI].xcentre);
    const int32_t signed_ROI_ycentre=static_cast<int32_t>(_param._detROI._ROI[iROI].ycentre);
    const int32_t signed_ROI_xsize=static_cast<int32_t>(_param._detROI._ROI[iROI].xsize);
    const int32_t signed_ROI_ysize=static_cast<int32_t>(_param._detROI._ROI[iROI].ysize);
    int32_t index_of_centre=signed_ROI_xcentre
      + sign_CCD_def_size * _param._detROI._ROI[iROI].xcentre;
    int32_t index_min;
    int32_t this_index;
    /*index_min=_param._detROI._ROI[iROI].xcentre - _param._detROI._ROI[iROI].xsize
      + CCD::opal_default_size_sq * (_param._detROI._ROI[iROI].ycentre - _param._detROI._ROI[iROI].ysize);*/
    int32_t signed_index_min=signed_ROI_xcentre - signed_ROI_xsize
          + sign_CCD_def_size * (signed_ROI_ycentre - signed_ROI_ysize);
#ifdef debug
    size_t index_max=_param._detROI._ROI[iROI].xcentre + _param._detROI._ROI[iROI].xsize
      + CCD::opal_default_size_sq * (_param._detROI._ROI[iROI].ycentre + _param._detROI._ROI[iROI].ysize);
#endif
    int diff_Xsize_Xcentre_to_boundary=signed_ROI_xcentre - signed_ROI_xsize;
    int diff_Ysize_Ycentre_to_boundary=signed_ROI_ycentre - signed_ROI_ysize;
    if( diff_Xsize_Xcentre_to_boundary<0 || diff_Ysize_Ycentre_to_boundary<0 )
      std::cout << printoutdef <<  "too small distances x;y "<< diff_Xsize_Xcentre_to_boundary 
                <<" "<< diff_Ysize_Ycentre_to_boundary<< " for ROI " << iROI
                <<" "<< signed_index_min<<" " <<index_of_centre <<std::endl;
    index_min=signed_ROI_xcentre - signed_ROI_xsize
      + sign_CCD_def_size * (signed_ROI_ycentre - signed_ROI_ysize);

#ifdef debug
    std::cout << "indexes "<< index_of_centre<<" "<<index_min<<" "<<index_max<<std::endl;
#endif
    int32_t indexROI_min=0;
    int32_t indexROI_max=(2 * _param._detROI._ROI[iROI].xsize + 1)
      * (2 * _param._detROI._ROI[iROI].ysize + 1);
    size_t u_indexROI_min=static_cast<size_t>(indexROI_min);
    size_t u_indexROI_max=static_cast<size_t>(indexROI_max);
#ifdef debug
    std::cout << "indexes "<< index_of_centre<<" "<<indexROI_min<<" "<<indexROI_max<<std::endl;
#endif
    if(_param._detROI._ROI[iROI].name=="circ" || _param._detROI._ROI[iROI].name=="circle"  )
    {
      int32_t  xlocal,ylocal;
      const int32_t radius2 =  static_cast<int32_t>(pow(_param._detROI._ROI[iROI].xsize,2)
           /*+pow(_param._detROI._ROI[iROI].ysize,2) */);
#ifdef debug
      std::cout << printoutdef << "circ seen with radius^2= " <<radius2 <<std::endl;
#endif
      for(size_t iFrame=u_indexROI_min;iFrame<u_indexROI_max; ++iFrame)
      {
        xlocal=iFrame%(2* _param._detROI._ROI[iROI].xsize + 1);
        ylocal=iFrame/(2* _param._detROI._ROI[iROI].xsize + 1);
        if( ( pow(xlocal-signed_ROI_xsize,2) +
              pow(ylocal-signed_ROI_ysize,2) ) <= radius2 )
        {
          this_index=index_min+xlocal+ sign_CCD_def_size * (ylocal);
          //I do not need to set again to zero a pixel that was already masked!
          //I have also to check that I have not landed on the other side of the CHIP
          if (this_index>=0 && (this_index < sign_CCD_def_size_sq) && _param._ROImask[this_index]!=0)
          {
            if( _param._detROI._ROI[iROI].xcentre<=half_CCD_def_size )
            {
              if( (this_index%sign_CCD_def_size)<= signed_ROI_xsize+signed_ROI_xcentre )
              {
                _param._ROImask[this_index]=0;
                //remember how many pixels I have masked
                number_of_pixelsettozero++;
              }
            }
            else
            {
              if( (signed_ROI_xcentre-signed_ROI_xsize) < (this_index%sign_CCD_def_size) )
              {
                _param._ROImask[this_index]=0;
                //remember how many pixels I have masked
                number_of_pixelsettozero++;
              }
            }
          }
        }
      }
    }
    if(_param._detROI._ROI[iROI].name=="ellipse" )
    {
      int32_t xlocal,ylocal;
      const float a2 =  static_cast<float>(pow(_param._detROI._ROI[iROI].xsize,2));
      const float b2 =  static_cast<float>(pow(_param._detROI._ROI[iROI].ysize,2));
#ifdef debug
      std::cout << printoutdef << "ellipse seen with semi-axis^2= " << a2 << " and " << b2 <<std::endl;
#endif
      for(size_t iFrame=u_indexROI_min;iFrame<u_indexROI_max; ++iFrame)
      {
        xlocal=iFrame%(2* _param._detROI._ROI[iROI].xsize + 1);
        ylocal=iFrame/(2* _param._detROI._ROI[iROI].xsize + 1);
        if( ( pow(static_cast<float>(xlocal)-static_cast<float>(_param._detROI._ROI[iROI].xsize),2)/a2 +
              pow(static_cast<float>(ylocal)-static_cast<float>(_param._detROI._ROI[iROI].ysize),2)/b2 ) <= 1 )
        {
          this_index=index_min+xlocal+ sign_CCD_def_size * (ylocal);
          //I do not need to set again to zero a pixel that was already masked!
          //I have also to check that I have not landed on the other side of the CHIP
          if (this_index>=0 && (this_index < sign_CCD_def_size_sq) && _param._ROImask[this_index]!=0)
          {
            if( _param._detROI._ROI[iROI].xcentre<=half_CCD_def_size )
            {
              if( (this_index%sign_CCD_def_size)<= signed_ROI_xsize+signed_ROI_xcentre )
              {
                _param._ROImask[this_index]=0;
                //remember how many pixels I have masked
                number_of_pixelsettozero++;
              }
            }
            else
            {
              if( (signed_ROI_xcentre-signed_ROI_xsize) < (this_index%sign_CCD_def_size) )
              {
                _param._ROImask[this_index]=0;
                //remember how many pixels I have masked
                number_of_pixelsettozero++;
              }
            }
          }
        }
      }
    }
    if(_param._detROI._ROI[iROI].name=="square")
    {
      int32_t  xlocal,ylocal;
#ifdef debug
      std::cout << printoutdef << "square seen" <<std::endl;
#endif
      for(size_t iFrame=u_indexROI_min;iFrame<u_indexROI_max; ++iFrame)
      {
        xlocal=iFrame%(2* _param._detROI._ROI[iROI].xsize +1);
        ylocal=iFrame/(2* _param._detROI._ROI[iROI].xsize +1);
        this_index=index_min+xlocal+ sign_CCD_def_size * (ylocal);
        //I do not need to set again to zero a pixel that was already masked!
        //I have also to check that I have not landed on the other side of the CHIP
        if (this_index>=0 && (this_index < sign_CCD_def_size_sq) && _param._ROImask[this_index]!=0)
        {
          if( _param._detROI._ROI[iROI].xcentre<=half_CCD_def_size )
          {
            if( (this_index%sign_CCD_def_size)<= signed_ROI_xsize+signed_ROI_xcentre )
            {
              _param._ROImask[this_index]=0;
              //remember how many pixels I have masked
              number_of_pixelsettozero++;
            }
          }
          else
          {
            if( (signed_ROI_xcentre-signed_ROI_xsize) < (this_index%sign_CCD_def_size) )
            {
              _param._ROImask[this_index]=0;
              //remember how many pixels I have masked
              number_of_pixelsettozero++;
            }
          }
        }
      }
    }
    if(_param._detROI._ROI[iROI].name=="triangle")
    {
      int32_t  xlocal,ylocal;
      float xlocal_f,ylocal_f;
      float xsize,ysize;
      xsize=static_cast<float>(_param._detROI._ROI[iROI].xsize);
      ysize=static_cast<float>(_param._detROI._ROI[iROI].ysize);

#ifdef debug
     std::cout << printoutdef << "triangle seen" <<std::endl;
#endif
      if(_param._detROI._ROI[iROI].orientation==+1)
      {
#ifdef debug
        std::cout << printoutdef  << "triangle seen vertex upwards" <<std::endl;
#endif
        //the triangle is at least isosceles
        for(size_t iFrame=u_indexROI_min;iFrame<u_indexROI_max; ++iFrame)
        {
          xlocal=iFrame%(2* _param._detROI._ROI[iROI].xsize + 1);
          ylocal=iFrame/(2* _param._detROI._ROI[iROI].xsize + 1);
          xlocal_f=static_cast<float>(xlocal);
          ylocal_f=static_cast<float>(ylocal);

#ifdef debug
          std::cout<<"local "<<xlocal<<" "<<ylocal
                   << " " <<2 * ysize/xsize*xlocal_f << " " <<4*ysize - 2* ysize/xsize*xlocal_f
                   <<std::endl;
#endif
          if(ylocal-1<(2 * ysize/xsize*xlocal_f)
             && xlocal<(signed_ROI_xsize + 1))
          {
            this_index=index_min+xlocal+ sign_CCD_def_size * (ylocal);
            //I do not need to set again to zero a pixel that was already masked!
            //I have also to check that I have not landed on the other side of the CHIP
            if (this_index>=0 && (this_index < sign_CCD_def_size_sq) && _param._ROImask[this_index]!=0)
            {
              if( _param._detROI._ROI[iROI].xcentre<=half_CCD_def_size )
              {
                if( (this_index%sign_CCD_def_size)<= signed_ROI_xsize+signed_ROI_xcentre )
                {
                  _param._ROImask[this_index]=0;
                  //remember how many pixels I have masked
                  number_of_pixelsettozero++;
                }
              }
              else
              {
                if( (signed_ROI_xcentre-signed_ROI_xsize) < (this_index%sign_CCD_def_size) )
                {
                  _param._ROImask[this_index]=0;
                  //remember how many pixels I have masked
                  number_of_pixelsettozero++;
                }
              }
            }
          }
          else if(ylocal-1<(4*ysize - 2* ysize/xsize*xlocal_f)
                  && xlocal>signed_ROI_xsize)
          {
            this_index=index_min+xlocal+ sign_CCD_def_size * (ylocal);
            //I do not need to set again to zero a pixel that was already masked!
            //I have also to check that I have not landed on the other side of the CHIP
            if (this_index>=0 && (this_index < sign_CCD_def_size_sq) && _param._ROImask[this_index]!=0)
            {
              if( _param._detROI._ROI[iROI].xcentre<=half_CCD_def_size )
              {
                if( (this_index%sign_CCD_def_size)<= signed_ROI_xsize+signed_ROI_xcentre )
                {
                  _param._ROImask[this_index]=0;
                  //remember how many pixels I have masked
                  number_of_pixelsettozero++;
                }
              }
              else
              {
                if( (signed_ROI_xcentre-signed_ROI_xsize) < (this_index%sign_CCD_def_size) )
                {
                  _param._ROImask[this_index]=0;
                  //remember how many pixels I have masked
                  number_of_pixelsettozero++;
                }
              }
            }
          }
        }
      }
      if(_param._detROI._ROI[iROI].orientation==-1)
      {
#ifdef debug
       std::cout << printoutdef << "triangle seen vertex downwards" <<std::endl;
#endif
       for(size_t iFrame=u_indexROI_min;iFrame<u_indexROI_max; ++iFrame)
        {
          xlocal=iFrame%(2* _param._detROI._ROI[iROI].xsize + 1);
          ylocal=iFrame/(2* _param._detROI._ROI[iROI].xsize + 1);
          xlocal_f=static_cast<float>(xlocal);
          ylocal_f=static_cast<float>(ylocal);
#ifdef debug
          std::cout<<"local "<<xlocal<<" "<<ylocal
                   << " " <<(-2 * ysize/xsize*xlocal_f) + 2 * ysize
                   << " "<<-2*ysize + 2 *  ysize/xsize*xlocal <<std::endl;
#endif

          if(ylocal+1>((-2 * ysize/xsize*xlocal_f)
                       + 2 * ysize)
             && xlocal<(signed_ROI_xsize + 1))
          {
            this_index=index_min+xlocal+ sign_CCD_def_size * (ylocal);
            //I do not need to set again to zero a pixel that was already masked!
            //I have also to check that I have not landed on the other side of the CHIP
            if (this_index>=0 && (this_index < sign_CCD_def_size_sq) && _param._ROImask[this_index]!=0)
            {
              if( _param._detROI._ROI[iROI].xcentre<=half_CCD_def_size )
              {
                if( (this_index%sign_CCD_def_size)<= signed_ROI_xsize+signed_ROI_xcentre )
                {
                  _param._ROImask[this_index]=0;
                  //remember how many pixels I have masked
                  number_of_pixelsettozero++;
                }
              }
              else
              {
                if( (signed_ROI_xcentre-signed_ROI_xsize) < (this_index%sign_CCD_def_size) )
                {
                  _param._ROImask[this_index]=0;
                  //remember how many pixels I have masked
                  number_of_pixelsettozero++;
                }
              }
            }
          }
          else if(ylocal+1>(-2*ysize +
                            2 * ysize/xsize*xlocal_f)
                  && xlocal>signed_ROI_xsize)
          {
            this_index=index_min+xlocal+ sign_CCD_def_size * (ylocal);
            //I do not need to set again to zero a pixel that was already masked!
            //I have also to check that I have not landed on the other side of the CHIP
            if (this_index>=0 && (this_index < sign_CCD_def_size_sq) && _param._ROImask[this_index]!=0)
            {
              if( _param._detROI._ROI[iROI].xcentre<=half_CCD_def_size )
              {
                if( (this_index%sign_CCD_def_size)<= signed_ROI_xsize+signed_ROI_xcentre )
                {
                  _param._ROImask[this_index]=0;
                  //remember how many pixels I have masked
                  number_of_pixelsettozero++;
                }
              }
              else
              {
                if( (signed_ROI_xcentre-signed_ROI_xsize) < (this_index%sign_CCD_def_size) )
                {
                  _param._ROImask[this_index]=0;
                  //remember how many pixels I have masked
                  number_of_pixelsettozero++;
                }
              }
            }
          }
        }
      }
      if(_param._detROI._ROI[iROI].orientation==+2)
      {
#ifdef debug
       std::cout << printoutdef << "triangle seen vertex towards right" <<std::endl;
#endif
        for(size_t iFrame=u_indexROI_min;iFrame<u_indexROI_max; ++iFrame)
        {
          // not debugged
          xlocal=iFrame%(2* _param._detROI._ROI[iROI].xsize + 1);
          ylocal=iFrame/(2* _param._detROI._ROI[iROI].xsize + 1);
          xlocal_f=static_cast<float>(xlocal);
          ylocal_f=static_cast<float>(ylocal);
#ifdef debug
          std::cout<<"local "<<xlocal<<" "<<ylocal
                   << " " <<(ysize/xsize*xlocal_f) << " "<<- ysize/xsize*xlocal_f + 4 * ylocal_f
                   <<std::endl;
#endif
          if(ylocal_f>(ysize/(2*xsize) * xlocal_f) && ylocal_f< (-ysize/(2*xsize)*xlocal_f + 2 * ysize) )
          {
            this_index=index_min+xlocal+ sign_CCD_def_size * (ylocal);
            //I do not need to set again to zero a pixel that was already masked!
            //I have also to check that I have not landed on the other side of the CHIP
            if (this_index>=0 && (this_index < sign_CCD_def_size_sq) && _param._ROImask[this_index]!=0)
            {
              if( _param._detROI._ROI[iROI].xcentre<=half_CCD_def_size )
              {
                if( (this_index%sign_CCD_def_size)<= signed_ROI_xsize+signed_ROI_xcentre )
                {
                  _param._ROImask[this_index]=0;
                  //remember how many pixels I have masked
                  number_of_pixelsettozero++;
                }
              }
              else
              {
                if( (signed_ROI_xcentre-signed_ROI_xsize) < (this_index%sign_CCD_def_size) )
                {
                  _param._ROImask[this_index]=0;
                  //remember how many pixels I have masked
                  number_of_pixelsettozero++;
                }
              }
            }
          }
        }
      }
      if(_param._detROI._ROI[iROI].orientation==-2)
      {
#ifdef debug
        std::cout << printoutdef << "triangle seen vertex towards left" <<std::endl;
#endif
        for(size_t iFrame=u_indexROI_min;iFrame<u_indexROI_max; ++iFrame)
        {
          xlocal=iFrame%(2* _param._detROI._ROI[iROI].xsize);
          ylocal=iFrame/(2* _param._detROI._ROI[iROI].xsize);
          xlocal_f=static_cast<float>(xlocal);
          ylocal_f=static_cast<float>(ylocal);
          if(ylocal>(- ysize/(2*xsize) * xlocal_f + ysize) && ylocal<( ysize/(2*xsize) * xlocal_f + ysize) )
          {
            this_index=index_min+xlocal+ sign_CCD_def_size * (ylocal);
            //I do not need to set again to zero a pixel that was already masked!
            //I have also to check that I have not landed on the other side of the CHIP
            if (this_index>=0 && (this_index < sign_CCD_def_size_sq) && _param._ROImask[this_index]!=0)
            {
              if( _param._detROI._ROI[iROI].xcentre<=half_CCD_def_size )
              {
                if( (this_index%sign_CCD_def_size)<= signed_ROI_xsize+signed_ROI_xcentre )
                {
                  _param._ROImask[this_index]=0;
                  //remember how many pixels I have masked
                  number_of_pixelsettozero++;
                }
              }
              else
              {
                if( (signed_ROI_xcentre-signed_ROI_xsize) < (this_index%sign_CCD_def_size) )
                {
                  _param._ROImask[this_index]=0;
                  //remember how many pixels I have masked
                  number_of_pixelsettozero++;
                }
              }
            }
          }
        }
      }
    }
  } // end iROI loop

#ifdef debug
  std::cout <<"ROI "<< " "<<_param._ROImask.size()<<" ";
  for(size_t i=0;i<_param._ROImask.size();i++)
    std::cout << _param._ROImask[i]<< " ";
  std::cout<<std::endl;
#endif
  // now I know which pixel should be masked!
  size_t nextPixel=0;
  _param._ROIiterator.resize(_param._ROImask.size()-number_of_pixelsettozero-1);
  for(size_t iPixel=0;iPixel<_param._ROImask.size(); ++iPixel)
  {
    // the "pointer" is the location/index of the next pixel to be used
    if(_param._ROImask[iPixel]!=0)
    {
      _param._ROIiterator[nextPixel]=iPixel+1;
      nextPixel++;
    }
  }
  std::cout << printoutdef << "Commercial CCD Roiit sizes "<<_param._ROImask.size()<<" " 
            <<_param._ROIiterator.size()<< " "
            <<number_of_pixelsettozero <<std::endl;
#ifdef debug
  for(size_t i=0;i<_param._ROIiterator.size();i++)
  {
    if(i%16==0) std::cout <<"Roiit"<<" ";
    std::cout << _param._ROIiterator[i]<< " ";
    if(i%16==15) std::cout<<std::endl;
  }
  std::cout<<std::endl;
#endif

}

void cass::CCD::Analysis::operator()(cass::CASSEvent *cassevent)
{
  //retrieve a pointer to the ccd device we are working on//
  cass::CCD::CCDDevice* dev = dynamic_cast<cass::CCD::CCDDevice*>(cassevent->devices()[cass::CASSEvent::CCD]);
  //go through all detectors and do your stuff//
  detectors_t::iterator detIt (dev->detectors()->begin());
  for (; detIt != dev->detectors()->end(); ++detIt)
  {
    //retrieve a reference to the ccd detector from the iterator//
    cass::PixelDetector& det(*detIt);

    //clear the pixel list//
    det.pixellist().clear();

    //initialize the start values for integral and max pixel value//
    pixel_t maxpixelvalue                       = 0;
    float integral                              = 0;
    uint16_t framewidth                         = det.columns();
    uint16_t frameheight                        = det.rows();
    cass::PixelDetector::frame_t& frame      = det.frame();
    //if(frame.size()!=CCD_default_size_sq) 

    //go through all pixels of the frame//
    for (size_t i=0; i<frame.size(); ++i)
    {
      //extract the value and coordinate from the frame//
      pixel_t pixel         = frame[i];
      uint16_t xcoordinate  = i % framewidth;
      uint16_t ycoordinate  = i / framewidth;
      //do something only if there was some ROI defined
      if(_param._ROImask.size()-_param._ROIiterator.size() > 1)
      {
        if(_param._ROImask[i]==0)
        { 
          frame[i]=0;
          pixel=0;
        }
      }
      //calc integral//
      integral += pixel;

      //check whether pixel is above threshold//
      //then check whether pixel is a local maximum//
      //if so add it to the pixel list//
      if (pixel > _param._threshold)
        //check wether point is not at an edge
        if (ycoordinate > 0 &&
            ycoordinate < frameheight-1 &&
            xcoordinate > 0 &&
            xcoordinate < framewidth+1)
          // Check all surrounding pixels
          if (frame[i-framewidth-1] < pixel && //upper left
              frame[i-framewidth]   < pixel && //upper middle
              frame[i-framewidth+1] < pixel && //upper right
              frame[i-1]            < pixel && //left
              frame[i+1]            < pixel && //right
              frame[i+framewidth-1] < pixel && //lower left
              frame[i+framewidth]   < pixel && //lower middle
              frame[i+framewidth+1] < pixel)   //lower right
          {
            det.pixellist().push_back(Pixel(xcoordinate,ycoordinate,pixel));
          }

      //get the maximum pixel value//
      if (maxpixelvalue < pixel)
        maxpixelvalue = pixel;
    }
    //write the found integral and maximum Pixel value to the event//
    det.integral()     = static_cast<uint32_t>(integral);
    det.maxPixelValue()= maxpixelvalue;

    //rebinning the frame//
    //rebin image frame//
    if (_param._rebinfactor != 1)
    {
      //lock this section due to the access to the tmp frame//
      QMutexLocker locker(&_mutex);

      //get the new dimensions//
      const size_t newRows = framewidth  / _param._rebinfactor;
      const size_t newCols = frameheight / _param._rebinfactor;
      //set the new dimensions in the detector//
      det.rows()    = newRows;
      det.columns() = newCols;
      //resize the temporary container to fit the rebinned image
      //initialize it with 0
      _tmp.assign(newRows * newCols,0);
      //rebin the frame//
      for (size_t iIdx=0; iIdx<frame.size() ;++iIdx)
      {
        //calculate the row and column of the current Index//
        const size_t row = iIdx / framewidth;
        const size_t col = iIdx % framewidth;
        //calculate the index of the rebinned frame//
        const size_t newRow = row / _param._rebinfactor;
        const size_t newCol = col / _param._rebinfactor;
        //calculate the index in the rebinned frame//
        //that newRow and newCol belongs to//
        const size_t newIndex = newRow*newCols + newCol;
        //add this index value to the newIndex value//
        _tmp[newIndex] += frame[iIdx];
      }
      //copy the temporary frame to the right place
      det.frame().assign(_tmp.begin(), _tmp.end());
    }
  }
}
