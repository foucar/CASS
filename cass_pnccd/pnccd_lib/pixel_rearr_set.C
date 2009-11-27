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

// pixel_rearr_set.C
// Implementation of the the pixel rearrangement functionality
// for multiple sub arrays as declared in pixel_rearr_set.h

#ifndef PIXEL_REARR_SET_C
#define PIXEL_REARR_SET_C

#include "pixel_rearr_set.h"

template<typename DS, typename DT>
PixelRearrSet<DS,DT>::PixelRearrSet
(void)
{
// Initialize the map which links a list of pixel rearrangements
// for sub arrays to a list of indices with zero length:
    pixrearr_list_ = std::map<int, PixelRearrangement<DS,DT> >();
    list_size_     = 0;
// Initialize the start and target array size with zero:
    starr_x_       = 0;
    starr_y_       = 0;
    tgarr_x_       = 0;
    tgarr_y_       = 0;
    init_ok_       = false;

    return;
}

template<typename DS, typename DT>
PixelRearrSet<DS,DT>::~PixelRearrSet
()
{
    ;
}

template<typename DS, typename DT>
bool
PixelRearrSet<DS,DT>::setTotalArraySizes
(long int starr_x, long int starr_y,
 long int tgarr_x, long int tgarr_y)
{
// Check whether the array size is not zero and whether
// the arrays contain the identical number of pixels:
    if( (starr_x < 1 ) || (starr_y < 1) )
    {
	starr_x = starr_y = tgarr_x = tgarr_y = 0;
	return false;
    }
    if( starr_x*starr_y > tgarr_x*tgarr_y )
    {
	starr_x = starr_y = tgarr_x = tgarr_y = 0;
	return false;
    }
// Ok, set the array sizes:
    starr_x_ = starr_x;
    starr_y_ = starr_y;
    tgarr_x_ = tgarr_x;
    tgarr_y_ = tgarr_y;

    return true;
}

template<typename DS, typename DT>
int
PixelRearrSet<DS,DT>::addPixelRearrgmnt
(long int sgm_arr_x,   long int sgm_arr_y,
 long int sgm_stpvt_x, long int sgm_stpvt_y,
 long int sgm_tgpvt_x, long int sgm_tgpvt_y,
 long int rot_qdr_x,   long int rot_qdr_y)
{
// Increment the list size and add a new pixel
// rearrangement object to the map:
    list_size_++;
    if( pixrearr_list_.count(list_size_) > 0 )
    {
	return -1;
    }
// Ok, add the pixel rearrangement object:
    pixrearr_list_[list_size_] = PixelRearrangement<DS,DT>();
    pixrearr_list_[list_size_].setSegmArrStartSize(sgm_arr_x,
						   sgm_arr_y);
    pixrearr_list_[list_size_].setSegmStartPivot(sgm_stpvt_x,
						 sgm_stpvt_y);
    pixrearr_list_[list_size_].setSegmTargetPivot(sgm_tgpvt_x,
						  sgm_tgpvt_y);
    pixrearr_list_[list_size_].setSegmRotationQuadr(rot_qdr_x,
						    rot_qdr_y);

    return list_size_;
}

template<typename DS, typename DT>
bool
PixelRearrSet<DS,DT>::initPixRearrSet
(void)
{
    typename std::map<int, PixelRearrangement<DS,DT> >::iterator
	pxr_list_itr;

    init_ok_ = false;
    if( list_size_ < 1 ) return false;
// Iterate through the list of pixel sub array rearrangements
// and initialize them. Return with false of the initialization
// of a list element fails:
    for( pxr_list_itr  = pixrearr_list_.begin();
	 pxr_list_itr != pixrearr_list_.end();
	 pxr_list_itr++ )
    {
	if( pxr_list_itr->first )
	{
	    if( !pxr_list_itr->second.setStartArrSize(starr_x_,starr_y_) )
	    {
		return false;
	    }
	    if( !pxr_list_itr->second.setTargetArrSize(tgarr_x_,tgarr_y_) )
	    {
		return false;
	    }
	    if( !pxr_list_itr->second.initWithCurrParams() )
	    {
		return false;
	    }
	}
	else
	{
	    return false;
	}
    }

    return init_ok_ = true;
}

template<typename DS, typename DT>
bool
PixelRearrSet<DS,DT>::rearrangeAllPixels
(DS *start_array, DT *target_array)
{
    typename std::map<int, PixelRearrangement<DS,DT> >::iterator
	pxr_list_itr;

    if( !init_ok_ ) return init_ok_;
// Iterate through the list of of pixel sub array rearrangements
// and assign every sub array to its position and orientation
// in the target array:
    for( pxr_list_itr  = pixrearr_list_.begin();
	 pxr_list_itr != pixrearr_list_.end();
	 pxr_list_itr++ )
    {
	if( pxr_list_itr->first )
	{
	    if( !pxr_list_itr->second.rearrangePixels(
		    start_array,target_array) )
	    {
		return false;
	    }
	}
	else
	{
	    return false;
	}
    }

    return true;
}

#endif // PIXEL_REARR_SET_C

// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "Stroustrup"
// fill-column: 100
// End:

// end of file pixel_rearr_set.C

