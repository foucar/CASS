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

// pixel_rearrangement.h
// Rearrange a segment of pixels into a new array configuration.
// 

#ifndef PIXEL_REARRANGEMENT_H
#define PIXEL_REARRANGEMENT_H

template<typename DS, typename DT> class PixelRearrangement
{
public:
    PixelRearrangement(void);
    ~PixelRearrangement();
    bool setStartArrSize(
	long int starr_x, long int starr_y);
    bool setTargetArrSize(
	long int tgarr_x, long int tgarr_y);
    bool setSegmArrStartSize(
	long int sgm_arr_x, long int sgm_arr_y);
    bool setSegmStartPivot(
	long int sgm_stpvt_x, long int sgm_stpvt_y);
    bool setSegmTargetPivot(
	long int sgm_tgpvt_x, long int sgm_tgpvt_y);
//
// Define the rotation of the array segment as a rotation
// in one of the four quadrants of a coordinate system.
// E.g. rot_qdr_x = 1 and rot_qdr_y = -1 means rotation
// around 90 degrees clockwise.
//
    bool setSegmRotationQuadr(
	long int rot_qdr_x, long int rot_qdr_y);
//
// Initialize the transformation with the currently set
// parameters:
//
    bool initWithCurrParams(void);
//
// Rearrange the pixels in the start array into their
// positions in the target array.
//
    bool rearrangePixels(
	DS *start_array, DT *target_array);
//
// Rearrange the pixels in the start array and add their
// values to the positions in the target array.
//
    bool rearrAccumPixels(
	DS *start_array, DT *target_array);
private:
// Member function pointer type for the pixel coordinate
// transformation:
    typedef bool (PixelRearrangement::*XFORM_PXCRDS)(
	long int  x_in,  long int  y_in,
	long int *x_out, long int *y_out);
    XFORM_PXCRDS xformPixCoords;
// Member function for the pixel coordinate
// transformation:
    bool transformPixelCoords_(
	long int  x_in,  long int  y_in,
	long int *x_out, long int *y_out);
// The dimensions of the start array:
    long int starr_x_;
    long int starr_y_;
// The dimensions of the target array:
    long int tgarr_x_;
    long int tgarr_y_;
// The dimensions of the segment to be rearranged
// before its rotation:
    long int sgm_arr_x_;
    long int sgm_arr_y_;
// The lower left corner (pivot) of the segment in
// the start array:
    long int sgm_stpvt_x_;
    long int sgm_stpvt_y_;
// The lower left corner (pivot) of the segment in
// the target array:
    long int sgm_tgpvt_x_;
    long int sgm_tgpvt_y_;
// The rotation definition for the segment as quadrant
// signs in a coordinate system:
    long int rot_qdr_x_;
    long int rot_qdr_y_;
// The rotation matrix:
    long int rot_mtx_[4];
// The translation which must be added after the
// rotation of the coordinates:
    long int zero_trvct_[2];
};


#endif // PIXEL_REARRANGEMENT_H

// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "Stroustrup"
// fill-column: 100
// End:

// end of file pixel_rearrangement.h




