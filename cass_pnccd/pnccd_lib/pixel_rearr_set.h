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

// pixel_rearr_set.h
// Rearrange multiple segments of pixels / subarrays into a new
// array configuration.
// 

#ifndef PIXEL_REARR_SET_H
#define PIXEL_REARR_SET_H

#include <map>

#include "pixel_rearrangement.C"

template<typename DS, typename DT> class PixelRearrSet
{
public:
    PixelRearrSet(void);
    ~PixelRearrSet();
// Set the sizes of the start and the target array,
// returns false if the amount of pixels in the
// arrays is not equal:
    bool setTotalArraySizes(long int starr_x, long int starr_y,
			    long int tgarr_x, long int tgarr_y);
// Add a new pixel rearrangement step to the list of
// rearrangements. Pixel rearrangements are performed
// in the order of their addition to the list:
    int  addPixelRearrgmnt(long int sgm_arr_x,   long int sgm_arr_y,
			   long int sgm_stpvt_x, long int sgm_stpvt_y,
			   long int sgm_tgpvt_x, long int sgm_tgpvt_y,
			   long int rot_qdr_x,   long int rot_qdr_y);
// After all pixel rearrangements are added, they must be
// initialized, returns false if the start and target
// array sizes are not set or if no pixel rearrangements are
// defined:
    bool initPixRearrSet(void);
// Perform all rearrangements of pixel array segments
// from the start into the target array:
    bool rearrangeAllPixels(DS *start_array,
			    DT *target_array);

private:
// The map which associates the subarray rearrangements
// with step indices:
    std::map< int, PixelRearrangement<DS,DT> > pixrearr_list_;
    int      list_size_;
// The total array sizes:
    long int starr_x_;
    long int starr_y_;
    long int tgarr_x_;
    long int tgarr_y_;
// Status flags:
    bool     init_ok_;
};


#endif // PIXEL_REARR_SET_H

// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "Stroustrup"
// fill-column: 100
// End:

// end of file pixel_rearr_set.h

