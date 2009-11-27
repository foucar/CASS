/*************************************************************************

 Copyright (C) 2007-2009 by Peter Holl and Nils Kimmel.
 All rights reserved.
 
 This code implementation is the intellectual property of

 Peter Holl    <pxh@hll.mpg.de> , MPI semiconductor laboratory,
                                  PNSensor GmbH

 Nils Kimmel   <nik@hll.mpg.de> , MPI semiconductor laboratory,
                                  Max-Planck-Institut fuer extra-
                                  terrestrische Physik

 By copying, distributing or modifying the Program (or any work
 based on the Program) you indicate your acceptance of this statement,
 and all its terms.

*************************************************************************/

// pixel_rearrangement.C
// Implementation of the the pixel rearrangement
// functionality declared in pixel_rearrangement.h

#ifndef PIXEL_REARRANGEMENT_C
#define PIXEL_REARRANGEMENT_C

#include "pixel_rearrangement.h"

template<typename DS, typename DT>
PixelRearrangement<DS,DT>::PixelRearrangement
(void)
{
//
// Set default values for the private parameters:
//
    starr_x_       = 0;
    starr_y_       = 0;
    tgarr_x_       = 0;
    tgarr_y_       = 0;
    sgm_arr_x_     = 0;
    sgm_arr_y_     = 0;
    sgm_stpvt_x_   = 0;
    sgm_stpvt_y_   = 0;
    sgm_tgpvt_x_   = 0;
    sgm_tgpvt_y_   = 0;
    rot_qdr_x_     = 1;
    rot_qdr_y_     = 1;
//
// Initialize with the identity matrix:
//
    rot_mtx_[0]    = 1; // first column, first row  
    rot_mtx_[1]    = 0; // first column, second row 
    rot_mtx_[2]    = 0; // second column, first row 
    rot_mtx_[3]    = 1; // second column, second row
// Identity matrix -> no zero translation needed:
    zero_trvct_[0] = 0;
    zero_trvct_[1] = 0;

}

template<typename DS, typename DT>
PixelRearrangement<DS,DT>::~PixelRearrangement
()
{
    ;
}

template<typename DS, typename DT>
bool
PixelRearrangement<DS,DT>::setStartArrSize
(long int starr_x, long int starr_y)
{
// Do a basic check of the argument values:
    if( (starr_x < 1) || (starr_y < 1) )
    {
	return false;

    }
// Assign to the private members:
    starr_x_ = starr_x;
    starr_y_ = starr_y;

    return true;
}

template<typename DS, typename DT>
bool
PixelRearrangement<DS,DT>::setTargetArrSize
(long int tgarr_x, long int tgarr_y)
{
// Do a basic check of the argument values:
    if( (tgarr_x < 1) || (tgarr_y < 1) )
    {
	return false;
    }
// Assign to the private members:
    tgarr_x_ = tgarr_x;
    tgarr_y_ = tgarr_y;

    return true;
}

template<typename DS, typename DT>
bool
PixelRearrangement<DS,DT>::setSegmArrStartSize
(long int sgm_arr_x, long int sgm_arr_y)
{
// Do a basic check of the argument values:
    if( (sgm_arr_x < 1) || (sgm_arr_y < 1) )
    {
	return false;
    }
// Assign to the private members:
    sgm_arr_x_ = sgm_arr_x;
    sgm_arr_y_ = sgm_arr_y;

    return true;
}

template<typename DS, typename DT>
bool
PixelRearrangement<DS,DT>::setSegmStartPivot
(long int sgm_stpvt_x, long int sgm_stpvt_y)
{
// Do a basic check of the argument values:
    if( (sgm_stpvt_x < 0) || (sgm_stpvt_y < 0) )
    {
	return false;
    }
// Assign to the private members:
    sgm_stpvt_x_ = sgm_stpvt_x;
    sgm_stpvt_y_ = sgm_stpvt_y;

    return true;
}

template<typename DS, typename DT>
bool
PixelRearrangement<DS,DT>::setSegmTargetPivot
(long int sgm_tgpvt_x, long int sgm_tgpvt_y)
{
// Do a basic check of the argument values:
    if( (sgm_tgpvt_x < 0) || (sgm_tgpvt_y < 0) )
    {
	return false;
    }
// Assign to the private members:
    sgm_tgpvt_x_ = sgm_tgpvt_x;
    sgm_tgpvt_y_ = sgm_tgpvt_y;

    return true;
}

template<typename DS, typename DT>
bool
PixelRearrangement<DS,DT>::setSegmRotationQuadr
(long int rot_qdr_x, long int rot_qdr_y)
{
// Check the argunment values:
    if( ((rot_qdr_x != -1) && (rot_qdr_x != 1)) ||
	((rot_qdr_y != -1) && (rot_qdr_y != 1)) )
    {
	return false;
    }
// Assign to the private members:
    rot_qdr_x_ = rot_qdr_x;
    rot_qdr_y_ = rot_qdr_y;

    return true;
}

template<typename DS, typename DT>
bool
PixelRearrangement<DS,DT>::initWithCurrParams
(void)
{
//
// Set the rotation matrix and the translation which is
// applied after the rotation:
//

// No rotation:
    if( (rot_qdr_x_ == 1) && (rot_qdr_y_ == 1) )
    {
	rot_mtx_[0]    = 1; // first column, first row  
	rot_mtx_[1]    = 0; // first column, second row 
	rot_mtx_[2]    = 0; // second column, first row 
	rot_mtx_[3]    = 1; // second column, second row

	zero_trvct_[0] = 0;
	zero_trvct_[1] = 0;
    }
// Rotation by 90 deg. clockwise:
    else if( (rot_qdr_x_ == 1) && (rot_qdr_y_ == -1) )
    {
	rot_mtx_[0]    =  0; // first column, first row  
	rot_mtx_[1]    = -1; // first column, second row 
	rot_mtx_[2]    =  1; // second column, first row 
	rot_mtx_[3]    =  0; // second column, second row

	zero_trvct_[0] = 0;
	zero_trvct_[1] = sgm_arr_x_ - 1;
    }
// Rotation by 180 deg. clockwise:
    else if(  (rot_qdr_x_ == -1) && (rot_qdr_y_ == -1) )
    {
	rot_mtx_[0]    = -1; // first column, first row  
	rot_mtx_[1]    =  0; // first column, second row 
	rot_mtx_[2]    =  0; // second column, first row 
	rot_mtx_[3]    = -1; // second column, second row

	zero_trvct_[0] = sgm_arr_x_ - 1;
	zero_trvct_[1] = sgm_arr_y_ - 1;
    }
// Rotation by 270 deg. clockwise:
    else if(  (rot_qdr_x_ == -1) && (rot_qdr_y_ == 1) )
    {
	rot_mtx_[0]    =  0; // first column, first row  
	rot_mtx_[1]    =  1; // first column, second row 
	rot_mtx_[2]    = -1; // second column, first row 
	rot_mtx_[3]    =  0; // second column, second row

	zero_trvct_[0] = sgm_arr_y_ - 1;
	zero_trvct_[1] = 0;
    }
// Wrongly defined rotation quadrants, do nothing
// and return false:
    else
    {
	return false;
    }

    return true;
}

template<typename DS, typename DT>
bool
PixelRearrangement<DS,DT>::rearrangePixels
(DS *start_array, DT *target_array)
{
    long int i, j;
    long int tg_x, tg_y;
    long int x_starr_end, y_starr_end;

// Check whether the addresses in the arguments are not
// equal zero, but be warned that there is nothing more
// we can do!
    if( (!start_array) || (!target_array) )
    {
	return false;
    }
// Loop over the pixel coordinates in the start array,
// beginning at the start pivot and over the dimensions
// of the segment which should be transformed and copied
// to the target array:
    x_starr_end = sgm_stpvt_x_ + sgm_arr_x_;
    y_starr_end = sgm_stpvt_y_ + sgm_arr_y_;

/*
    std::cout << "\n sgm_stpvt_y_ = " << sgm_stpvt_y_ << " ,"
	      << " y_starr_end = "    << y_starr_end  << " ,"
	      << " sgm_stpvt_x_ = "   << sgm_stpvt_x_ << " ,"
	      << " x_starr_end = "    << x_starr_end
	      << " starr_x_ = "       << starr_x_ << " , "
	      << " tgarr_x_ = "       << tgarr_x_
	      << std::flush;
*/

    for( j=sgm_stpvt_y_; j<y_starr_end; j++ )
    {
	for( i=sgm_stpvt_x_; i<x_starr_end; i++ )
	{
// Perform the coordinate transformation to the target
// array:
	    this->transformPixelCoords_(i,j,&tg_x,&tg_y);
// Copy the pixel values:
	    target_array[tg_x+tg_y*tgarr_x_]
		= start_array[i+j*starr_x_];
	}
    }

    return true;
}

template<typename DS, typename DT>
bool
PixelRearrangement<DS,DT>::rearrAccumPixels
(DS *start_array, DT *target_array)
{
// Check whether the addresses in the arguments are not
// equal zero, but be warned that there is nothing more
// we can do!
    if( (!start_array) || (!target_array) )
    {
	return false;
    }

    return true;
}

//
// Private function members:
//

template<typename DS, typename DT>
bool
PixelRearrangement<DS,DT>::transformPixelCoords_
(long int  x_in,  long int  y_in,
 long int *x_out, long int *y_out)
{
//
// Do not check the input arguments. Perform the
// transformation regardless of the input values in
// order to improve performance.
// Perform the matrix multiplication, the zero shift
// of the resulting array and the shift of the zeroed
// array to the target pivot point:
//
    x_in -= sgm_stpvt_x_;
    y_in -= sgm_stpvt_y_;

    *x_out = rot_mtx_[0]*x_in
           + rot_mtx_[2]*y_in
           + zero_trvct_[0]
           + sgm_tgpvt_x_;

    *y_out = rot_mtx_[1]*x_in
           + rot_mtx_[3]*y_in
           + zero_trvct_[1]
           + sgm_tgpvt_y_;

    return true;
} 


#endif // PIXEL_REARRANGEMENT_C

// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "Stroustrup"
// fill-column: 100
// End:

// end of file pixel_rearrangement.C



