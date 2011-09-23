//Copyright (C) 2011 Lutz Foucar

/**
 * @file pixeldetector_mask.cpp contains definition of the mask of a pixeldetector
 *
 * @author Lutz Foucar
 */

#include "pixeldetector_mask.h"

#include "cass_settings.h"

void createCASSMask(CommonData &data, CASSSettings &s);

using namespace cass;
using namespace pixeldetector;
using namespace std;

void createCASSMask(CommonData &data, CASSSettings &s)
{
  int size = s.beginReadArray("Mask");
  vector<MaskElement> mask;
  for (int i = 0; i < size; ++i)
  {
    s.setArrayIndex(i);
    MaskElement element;
    element.type = s.value("ROIShapeType","square").toString().toStdString();
    element.xsize = value("XSize",1).toUInt();
    element.ysize = value("YSize",1).toUInt();
    element.xcentre = value("XCentre",1).toUInt();
    element.ycentre = value("YCentre",1).toUInt();
    element.orientation = value("orientation",1).toInt();
    mask.push_back(element);
  }
  s.endArray();

  // "enable" all  pixel as default
  dp._ROImask.assign(dp._ROImask.size(),1);
  dp._ROImask.assign(dp._ROImask_converter.size(),1);
  //I need to reset the number of masked pixel per frame
  number_of_pixelsettozero=0;
  for(size_t iROI=0;iROI<dp._detROI._ROI.size(); ++iROI)
  {
    const size_t half_pnCCD_def_size=pnCCD::default_size/2;
    const int32_t sign_pnCCD_def_size=static_cast<int32_t>(pnCCD::default_size);
    const int32_t sign_pnCCD_def_size_sq=static_cast<int32_t>(pnCCD::default_size_sq);
    const int32_t signed_ROI_xcentre=static_cast<int32_t>(dp._detROI._ROI[iROI].xcentre);
    const int32_t signed_ROI_ycentre=static_cast<int32_t>(dp._detROI._ROI[iROI].ycentre);
    const int32_t signed_ROI_xsize=static_cast<int32_t>(dp._detROI._ROI[iROI].xsize);
    const int32_t signed_ROI_ysize=static_cast<int32_t>(dp._detROI._ROI[iROI].ysize);

    int32_t index_of_centre=signed_ROI_xcentre + sign_pnCCD_def_size * signed_ROI_ycentre;
    int32_t index_min;
    int32_t this_index;

    int32_t signed_index_min=signed_ROI_xcentre - signed_ROI_xsize
      + sign_pnCCD_def_size * (signed_ROI_ycentre - signed_ROI_ysize);
    int diff_Xsize_Xcentre_to_boundary=signed_ROI_xcentre - signed_ROI_xsize;
    int diff_Ysize_Ycentre_to_boundary=signed_ROI_ycentre - signed_ROI_ysize;
    if( diff_Xsize_Xcentre_to_boundary<0 || diff_Ysize_Ycentre_to_boundary<0 )
      std::cout << printoutdef <<  "too small distances x;y "<< diff_Xsize_Xcentre_to_boundary
                <<" "<< diff_Ysize_Ycentre_to_boundary<< " for ROI " << iROI
                <<" "<< signed_index_min<<" " <<index_of_centre <<std::endl;
    index_min=signed_ROI_xcentre - signed_ROI_xsize
      + sign_pnCCD_def_size * (signed_ROI_ycentre - signed_ROI_ysize);
    if(index_min>sign_pnCCD_def_size_sq) std::cout<<"What? "<<dp._detROI._ROI[iROI].xcentre << " "<<
                              dp._detROI._ROI[iROI].xsize << " "<<
                              dp._detROI._ROI[iROI].ycentre << " "<< dp._detROI._ROI[iROI].ysize<<std::endl;
#ifdef debug
    size_t index_max=dp._detROI._ROI[iROI].xcentre + dp._detROI._ROI[iROI].xsize
      + pnCCD::default_size * (dp._detROI._ROI[iROI].ycentre + dp._detROI._ROI[iROI].ysize);
    std::cout << printoutdef << "indexes "<< index_of_centre<<" "<<index_min<<" "<<index_max<<std::endl;
#endif
    int32_t indexROI_min=0;
    int32_t indexROI_max=(2 * signed_ROI_xsize + 1)
        * (2 * signed_ROI_ysize + 1);
    size_t u_indexROI_min=static_cast<size_t>(indexROI_min);
    size_t u_indexROI_max=static_cast<size_t>(indexROI_max);
#ifdef debug
    std::cout << printoutdef <<  "indexes "<< index_of_centre<<" "<<indexROI_min<<" "<<indexROI_max<<std::endl;
#endif
    if(dp._detROI._ROI[iROI].name=="circ" || dp._detROI._ROI[iROI].name=="circle"  )
    {
      int32_t  xlocal,ylocal;
      const int32_t radius2 =  static_cast<int32_t>(pow(dp._detROI._ROI[iROI].xsize,2) );
#ifdef debug
      std::cout << printoutdef << "circ seen with radius^2= " <<radius2 <<std::endl;
#endif
      for(size_t iFrame=u_indexROI_min;iFrame<u_indexROI_max; ++iFrame)
      {
        xlocal=iFrame%(2* dp._detROI._ROI[iROI].xsize + 1);
        ylocal=iFrame/(2* dp._detROI._ROI[iROI].xsize + 1);
        if( ( pow(xlocal-signed_ROI_xsize,2) +
              pow(ylocal-signed_ROI_ysize,2) ) <= radius2 )
        {
          this_index=index_min+xlocal+ sign_pnCCD_def_size * (ylocal);
          //I do not need to set again to zero a pixel that was already masked!
          //I have also to check that I have not landed on the other side of the CHIP
          if (this_index>=0 && (this_index < sign_pnCCD_def_size_sq) && dp._ROImask[this_index]!=0)
          {
            if( dp._detROI._ROI[iROI].xcentre<=half_pnCCD_def_size )
            {
              if( (this_index%sign_pnCCD_def_size)<= signed_ROI_xsize+signed_ROI_xcentre )
              {
                dp._ROImask[this_index]=0;
                //remember how many pixels I have masked
                number_of_pixelsettozero++;
              }
            }
            else
            {
              if( (signed_ROI_xcentre-signed_ROI_xsize) < (this_index%sign_pnCCD_def_size) )
              {
                dp._ROImask[this_index]=0;
                //remember how many pixels I have masked
                number_of_pixelsettozero++;
              }
            }
          }
        }
      }
    }
    if(dp._detROI._ROI[iROI].name=="ellipse" )
    {
      int32_t xlocal,ylocal;
      const float a2 =  static_cast<float>(pow(dp._detROI._ROI[iROI].xsize,2));
      const float b2 =  static_cast<float>(pow(dp._detROI._ROI[iROI].ysize,2));
#ifdef debug
      std::cout << printoutdef << "circ ellipse with semi-axis^2= " << a2 << " and " << b2 <<std::endl;
#endif
      for(size_t iFrame=u_indexROI_min;iFrame<u_indexROI_max; ++iFrame)
      {
        xlocal=iFrame%(2* dp._detROI._ROI[iROI].xsize + 1);
        ylocal=iFrame/(2* dp._detROI._ROI[iROI].xsize + 1);
#ifdef debug
        std::cout<<"local "<<xlocal<<" "<<ylocal<<" "<<dp._detROI._ROI[iROI].xsize<< " "
                 <<pow(xlocal-signed_ROI_xsize,2) <<std::endl;
#endif
        if( ( pow(static_cast<float>(xlocal)-static_cast<float>(dp._detROI._ROI[iROI].xsize),2)/a2 +
              pow(static_cast<float>(ylocal)-static_cast<float>(dp._detROI._ROI[iROI].ysize),2)/b2 ) <= 1 )
        {
          this_index=index_min+xlocal+ sign_pnCCD_def_size * (ylocal);
          //I do not need to set again to zero a pixel that was already masked!
          //I have also to check that I have not landed on the other side of the CHIP
          if (this_index>=0 && (this_index < sign_pnCCD_def_size_sq) && dp._ROImask[this_index]!=0)
          {
            if( dp._detROI._ROI[iROI].xcentre<=half_pnCCD_def_size )
            {
              if( (this_index%sign_pnCCD_def_size)<= signed_ROI_xsize+signed_ROI_xcentre )
              {
                dp._ROImask[this_index]=0;
                //remember how many pixels I have masked
                number_of_pixelsettozero++;
              }
            }
            else
            {
              if( (signed_ROI_xcentre-signed_ROI_xsize) < (this_index%sign_pnCCD_def_size) )
              {
                dp._ROImask[this_index]=0;
                //remember how many pixels I have masked
                number_of_pixelsettozero++;
              }
            }
          }
        }
      }
    }
    if(dp._detROI._ROI[iROI].name=="specialline")
    {
      float  xlocal1_f,ylocal1_f,xlocal2_f,ylocal2_f;
      int32_t  xlocal,ylocal;
      const int32_t radius_min2 =  static_cast<int32_t>(pow(dp._detROI._ROI[iROI].xsize,2));
      const int32_t radius_max2 =  static_cast<int32_t>(pow(dp._detROI._ROI[iROI].ysize,2));
      indexROI_max=sign_pnCCD_def_size*140/100/2;
      const float slope=static_cast<float>(dp._detROI._ROI[iROI].orientation);
      if(std::abs(slope)<200.) std::cout<<"slope "<<slope<<std::endl;
      else std::cout<<"vertical slope"<<std::endl;
      for(int iFrame=0;iFrame<indexROI_max; ++iFrame)
      {
        if(std::abs(slope)<200.)
        {
          xlocal1_f=static_cast<float>(signed_ROI_xcentre + iFrame);
          xlocal2_f=static_cast<float>(signed_ROI_xcentre - iFrame);
          ylocal1_f=static_cast<float>(signed_ROI_ycentre) + static_cast<float>(iFrame) * slope ;
          ylocal2_f=static_cast<float>(signed_ROI_ycentre) - static_cast<float>(iFrame) * slope ;
        }
        else
        {
          xlocal1_f=static_cast<float>(signed_ROI_xcentre);
          xlocal2_f=static_cast<float>(signed_ROI_xcentre);
          ylocal1_f=static_cast<float>(signed_ROI_ycentre + iFrame);
          ylocal2_f=static_cast<float>(signed_ROI_ycentre - iFrame);
        }
#ifdef thisdebug
        std::cout<<"local "<<xlocal1_f<<" "<<ylocal1_f
                 << " " << " "<< xlocal2_f<<" "<<ylocal2_f <<std::endl;
#endif
        //Inside the first radius
        if( ( pow(xlocal1_f-signed_ROI_xcentre,2) +
              pow(ylocal1_f-signed_ROI_ycentre,2) ) < radius_min2 )
        {
#ifdef thisdebug
        std::cout<<"locali "<<xlocal1_f<<" "<<ylocal1_f
                 << " " << " "<< xlocal2_f<<" "<<ylocal2_f <<std::endl;
#endif
          xlocal=static_cast<int32_t>(xlocal1_f);
          ylocal=static_cast<int32_t>(ylocal1_f);
          this_index=xlocal + sign_pnCCD_def_size * (ylocal);
          //              std::cout<<"local1a "<<this_index;
          //I do not need to set again to zero a pixel that was already masked!
          //I have also to check that I have not landed on the other side of the CHIP
          if (this_index>=0 && (this_index < sign_pnCCD_def_size_sq) && dp._ROImask[this_index]!=0)
          {
            dp._ROImask[this_index]=0;
            //remember how many pixels I have masked
            number_of_pixelsettozero++;
          }
          if (this_index+1>=0 && (this_index+1 < sign_pnCCD_def_size_sq) && dp._ROImask[this_index+1]!=0)
          {
            //std::cout<<"local1f "<<this_index+1;
            dp._ROImask[this_index+1]=0;
            //remember how many pixels I have masked
            number_of_pixelsettozero++;
          }
          if (this_index-1>=0 && (this_index-1 < sign_pnCCD_def_size_sq) && dp._ROImask[this_index-1]!=0)
          {
            //std::cout<<"local1f "<<this_index+1;
            dp._ROImask[this_index-1]=0;
            //remember how many pixels I have masked
            number_of_pixelsettozero++;
          }

          xlocal=static_cast<int32_t>(xlocal2_f);
          ylocal=static_cast<int32_t>(ylocal2_f);
          this_index=xlocal + sign_pnCCD_def_size * (ylocal);
          //I do not need to set again to zero a pixel that was already masked!
          //I have also to check that I have not landed on the other side of the CHIP
          if (this_index>=0 && (this_index < sign_pnCCD_def_size_sq) && dp._ROImask[this_index]!=0)
          {
            //std::cout<<"local1f "<<this_index;
            dp._ROImask[this_index]=0;
            //remember how many pixels I have masked
            number_of_pixelsettozero++;
          }

          if (this_index+1>=0 && (this_index+1 < sign_pnCCD_def_size_sq) && dp._ROImask[this_index+1]!=0)
          {
            //std::cout<<"local1f "<<this_index+1;
            dp._ROImask[this_index+1]=0;
            //remember how many pixels I have masked
            number_of_pixelsettozero++;
          }
          if (this_index-1>=0 && (this_index-1 < sign_pnCCD_def_size_sq) && dp._ROImask[this_index-1]!=0)
          {
            //std::cout<<"local1f "<<this_index+1;
            dp._ROImask[this_index-1]=0;
            //remember how many pixels I have masked
            number_of_pixelsettozero++;
          }
        }
        else
        {
          //Outside the second radius
          if( ( pow(xlocal1_f-signed_ROI_xcentre,2) +
                pow(ylocal1_f-signed_ROI_ycentre,2) ) > radius_max2 )
          {
#ifdef thisdebug
            std::cout<<"local2 "<<xlocal1_f<<" "<<ylocal1_f
                     << " " << " "<< xlocal2_f<<" "<<ylocal2_f <<std::endl;
#endif
            xlocal=static_cast<int32_t>(xlocal1_f);
            ylocal=static_cast<int32_t>(ylocal1_f);
            this_index=xlocal + sign_pnCCD_def_size * (ylocal);
#ifdef thisdebug
            std::cout<<"local2a "<<this_index;
#endif
            //I do not need to set again to zero a pixel that was already masked!
            //I have also to check that I have not landed on the other side of the CHIP
            if (this_index>=0 && (this_index < sign_pnCCD_def_size_sq) && dp._ROImask[this_index]!=0)
            {
              //std::cout<<"local2f "<<this_index;
              dp._ROImask[this_index]=0;
              //remember how many pixels I have masked
              number_of_pixelsettozero++;
            }
            if (this_index+1>=0 && (this_index+1 < sign_pnCCD_def_size_sq) && dp._ROImask[this_index+1]!=0)
            {
              //std::cout<<"local1f "<<this_index+1;
              dp._ROImask[this_index+1]=0;
              //remember how many pixels I have masked
              number_of_pixelsettozero++;
            }
            if (this_index-1>=0 && (this_index-1 < sign_pnCCD_def_size_sq) && dp._ROImask[this_index-1]!=0)
            {
              //std::cout<<"local1f "<<this_index+1;
              dp._ROImask[this_index-1]=0;
              //remember how many pixels I have masked
              number_of_pixelsettozero++;
            }

            xlocal=static_cast<int32_t>(xlocal2_f);
            ylocal=static_cast<int32_t>(ylocal2_f);
            this_index=xlocal + sign_pnCCD_def_size * (ylocal);
            //I do not need to set again to zero a pixel that was already masked!
            //I have also to check that I have not landed on the other side of the CHIP
            if (this_index>=0 && (this_index < sign_pnCCD_def_size_sq) && dp._ROImask[this_index]!=0)
            {
              dp._ROImask[this_index]=0;
              //remember how many pixels I have masked
              number_of_pixelsettozero++;
            }
            if (this_index+1>=0 && (this_index+1 < sign_pnCCD_def_size_sq) && dp._ROImask[this_index+1]!=0)
            {
              //std::cout<<"local1f "<<this_index+1;
              dp._ROImask[this_index+1]=0;
              //remember how many pixels I have masked
              number_of_pixelsettozero++;
            }
            if (this_index-1>=0 && (this_index-1 < sign_pnCCD_def_size_sq) && dp._ROImask[this_index-1]!=0)
            {
              //std::cout<<"local1f "<<this_index+1;
              dp._ROImask[this_index-1]=0;
              //remember how many pixels I have masked
              number_of_pixelsettozero++;
            }
          }
        }
      }
    }
    if(dp._detROI._ROI[iROI].name=="square")
    {
      int32_t  xlocal,ylocal;
#ifdef debug
      std::cout << printoutdef <<  "square seen" <<std::endl;
#endif
      for(size_t iFrame=u_indexROI_min;iFrame<u_indexROI_max; ++iFrame)
      {
        xlocal=iFrame%(2* dp._detROI._ROI[iROI].xsize +1);
        ylocal=iFrame/(2* dp._detROI._ROI[iROI].xsize +1);
        this_index=index_min+xlocal+ sign_pnCCD_def_size * (ylocal);
        //I do not need to set again to zero a pixel that was already masked!
        //I have also to check that I have not landed on the other side of the CHIP
        if (this_index>=0 && (this_index < sign_pnCCD_def_size_sq) && dp._ROImask[this_index]!=0)
        {
          if( dp._detROI._ROI[iROI].xcentre<=half_pnCCD_def_size )
          {
            if( (this_index%sign_pnCCD_def_size)<= signed_ROI_xsize+signed_ROI_xcentre )
            {
              dp._ROImask[this_index]=0;
              //remember how many pixels I have masked
              number_of_pixelsettozero++;
            }
          }
          else
          {
            if( (signed_ROI_xcentre-signed_ROI_xsize) < (this_index%sign_pnCCD_def_size) )
            {
              dp._ROImask[this_index]=0;
              //remember how many pixels I have masked
              number_of_pixelsettozero++;
            }
          }
        }
#ifdef debug
        std::cout<<"local "<<xlocal<<" "<<ylocal<<" "<<dp._detROI._ROI[iROI].ycentre
                 << " "<<dp._detROI._ROI[iROI].ycentre - dp._detROI._ROI[iROI].ysize <<std::endl;
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

#ifdef debug
      std::cout << printoutdef <<  "triangle seen" <<std::endl;
#endif
      if(dp._detROI._ROI[iROI].orientation==+1)
      {
#ifdef debug
        std::cout << printoutdef <<  "triangle seen vertex upwards" <<std::endl;
#endif
        //the triangle is at least isosceles
        for(size_t iFrame=u_indexROI_min;iFrame<u_indexROI_max; ++iFrame)
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
             && xlocal< (signed_ROI_xsize+1) )
          {
            this_index=index_min+xlocal+ sign_pnCCD_def_size * (ylocal);
            //I do not need to set again to zero a pixel that was already masked!
            //I have also to check that I have not landed on the other side of the CHIP
            if (this_index>=0 && (this_index < sign_pnCCD_def_size_sq) && dp._ROImask[this_index]!=0)
            {
              if( dp._detROI._ROI[iROI].xcentre<=half_pnCCD_def_size )
              {
                if( (this_index%sign_pnCCD_def_size)<= signed_ROI_xsize+signed_ROI_xcentre )
                {
                  dp._ROImask[this_index]=0;
                  //remember how many pixels I have masked
                  number_of_pixelsettozero++;
                }
              }
              else
              {
                if( (signed_ROI_xcentre-signed_ROI_xsize) < (this_index%sign_pnCCD_def_size) )
                {
                  dp._ROImask[this_index]=0;
                  //remember how many pixels I have masked
                  number_of_pixelsettozero++;
                }
              }
            }
          }
          else if(ylocal-1<(4*ysize - 2* ysize/xsize*xlocal_f)
                  && xlocal>signed_ROI_xsize)
          {
            this_index=index_min+xlocal+ sign_pnCCD_def_size * (ylocal);
            //I do not need to set again to zero a pixel that was already masked!
            //I have also to check that I have not landed on the other side of the CHIP
            if (this_index>=0 && (this_index < sign_pnCCD_def_size_sq) && dp._ROImask[this_index]!=0)
            {
              if( dp._detROI._ROI[iROI].xcentre<=half_pnCCD_def_size )
              {
                if( (this_index%sign_pnCCD_def_size)<= signed_ROI_xsize+signed_ROI_xcentre )
                {
                  dp._ROImask[this_index]=0;
                  //remember how many pixels I have masked
                  number_of_pixelsettozero++;
                }
              }
              else
              {
                if( (signed_ROI_xcentre-signed_ROI_xsize) < (this_index%sign_pnCCD_def_size) )
                {
                  dp._ROImask[this_index]=0;
                  //remember how many pixels I have masked
                  number_of_pixelsettozero++;
                }
              }
            }
          }
        }
      }
      if(dp._detROI._ROI[iROI].orientation==-1)
      {
#ifdef debug
        std::cout << printoutdef <<  "triangle seen vertex downwards" <<std::endl;
#endif
        for(size_t iFrame=u_indexROI_min;iFrame<u_indexROI_max; ++iFrame)
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
             && xlocal< (signed_ROI_xsize+1))
          {
            this_index=index_min+xlocal+ sign_pnCCD_def_size * (ylocal);
            //I do not need to set again to zero a pixel that was already masked!
            //I have also to check that I have not landed on the other side of the CHIP
            if (this_index>=0 && (this_index < sign_pnCCD_def_size_sq) && dp._ROImask[this_index]!=0)
            {
              if( dp._detROI._ROI[iROI].xcentre<=half_pnCCD_def_size )
              {
                if( (this_index%sign_pnCCD_def_size)<= signed_ROI_xsize+signed_ROI_xcentre )
                {
                  dp._ROImask[this_index]=0;
                  //remember how many pixels I have masked
                  number_of_pixelsettozero++;
                }
              }
              else
              {
                if( (signed_ROI_xcentre-signed_ROI_xsize) < (this_index%sign_pnCCD_def_size) )
                {
                  dp._ROImask[this_index]=0;
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
            this_index=index_min+xlocal+ sign_pnCCD_def_size * (ylocal);
            //I do not need to set again to zero a pixel that was already masked!
            //I have also to check that I have not landed on the other side of the CHIP
            if (this_index>=0 && (this_index < sign_pnCCD_def_size_sq) && dp._ROImask[this_index]!=0)
            {
              if( dp._detROI._ROI[iROI].xcentre<=half_pnCCD_def_size )
              {
                if( (this_index%sign_pnCCD_def_size)<= signed_ROI_xsize+signed_ROI_xcentre )
                {
                  dp._ROImask[this_index]=0;
                  //remember how many pixels I have masked
                  number_of_pixelsettozero++;
                }
              }
              else
              {
                if( (signed_ROI_xcentre-signed_ROI_xsize) < (this_index%sign_pnCCD_def_size) )
                {
                  dp._ROImask[this_index]=0;
                  //remember how many pixels I have masked
                  number_of_pixelsettozero++;
                }
              }
            }
          }
        }
      }
      if(dp._detROI._ROI[iROI].orientation==+2)
      {
#ifdef debug
        std::cout << printoutdef <<  "triangle seen vertex towards right" <<std::endl;
#endif
        for(size_t iFrame=u_indexROI_min;iFrame<u_indexROI_max; ++iFrame)
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
            this_index=index_min+xlocal+ sign_pnCCD_def_size * (ylocal);
            //I do not need to set again to zero a pixel that was already masked!
            //I have also to check that I have not landed on the other side of the CHIP
            if (this_index>=0 && (this_index < sign_pnCCD_def_size_sq) && dp._ROImask[this_index]!=0)
            {
              if( dp._detROI._ROI[iROI].xcentre<=half_pnCCD_def_size )
              {
                if( (this_index%sign_pnCCD_def_size)<= signed_ROI_xsize+signed_ROI_xcentre )
                {
                  dp._ROImask[this_index]=0;
                  //remember how many pixels I have masked
                  number_of_pixelsettozero++;
                }
              }
              else
              {
                if( (signed_ROI_xcentre-signed_ROI_xsize) < (this_index%sign_pnCCD_def_size) )
                {
                  dp._ROImask[this_index]=0;
                  //remember how many pixels I have masked
                  number_of_pixelsettozero++;
                }
              }
            }
          }
        }
      }
      if(dp._detROI._ROI[iROI].orientation==-2)
      {
#ifdef debug
        std::cout << printoutdef <<  "triangle seen vertex towards left" <<std::endl;
#endif
        for(size_t iFrame=u_indexROI_min;iFrame<u_indexROI_max; ++iFrame)
        {
          xlocal=iFrame%(2* dp._detROI._ROI[iROI].xsize);
          ylocal=iFrame/(2* dp._detROI._ROI[iROI].xsize);
          xlocal_f=static_cast<float>(xlocal);
          ylocal_f=static_cast<float>(ylocal);
          if(ylocal>(- ysize/(2*xsize) * xlocal_f + ysize) && ylocal<( ysize/(2*xsize) * xlocal_f + ysize) )
          {
            this_index=index_min+xlocal+ sign_pnCCD_def_size * (ylocal);
            //I do not need to set again to zero a pixel that was already masked!
            //I have also to check that I have not landed on the other side of the CHIP
            if (this_index>=0 && (this_index < sign_pnCCD_def_size_sq) && dp._ROImask[this_index]!=0)
            {
              if( dp._detROI._ROI[iROI].xcentre<=half_pnCCD_def_size )
              {
                if( (this_index%sign_pnCCD_def_size)<= signed_ROI_xsize+signed_ROI_xcentre )
                {
                  dp._ROImask[this_index]=0;
                  //remember how many pixels I have masked
                  number_of_pixelsettozero++;
                }
              }
              else
              {
                if( (signed_ROI_xcentre-signed_ROI_xsize) < (this_index%sign_pnCCD_def_size) )
                {
                  dp._ROImask[this_index]=0;
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

}
