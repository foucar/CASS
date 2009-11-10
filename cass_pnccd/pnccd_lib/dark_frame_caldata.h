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

// dark_frame_caldata.h
// A facility for loading from and saving dark frame calibration
// result data to a file.

#ifndef DARK_FRAME_CALDATA_H
#define DARK_FRAME_CALDATA_H

#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <string>
#include <cmath>
#include <algorithm>

#include <inttypes.h>
#include <string.h>

// external c headers, specify the linkage:

extern "C" {
#include "xonline_data_types.h"
#include "fformat.h"
}

class DarkFrameCaldata
{
public:
    DarkFrameCaldata(void);
    ~DarkFrameCaldata();
    bool setPixelStatMapAddr(
	staDataType *pix_stats, char *badpix_flags,
	uint32_t width, uint32_t height);
    staDataType* getPixelStatMapAddr(
	uint32_t *width, uint32_t *height);
    char* getBadPixelMapAddr(
	uint32_t *width, uint32_t *height);
    double* getOffsetMapAddr(
	uint32_t *width, uint32_t *height);
    double* getNoiseMapAddr(
	uint32_t *width, uint32_t *height);
    bool writePixelStatMapToFile(const std::string& out_fname,
				 bool overwrite=false);
    bool readPixelStatMapFromFile(const std::string& in_fname);
private:
//
// Private function members:
//
    bool allocLocalStorage_(void);
    bool cleanUpLocalStorage_(void);
    bool createOffsetMap_(void);
    bool createNoiseMap_(void);
//
// The parent widget for sending error messages:
//
//    QWidget *parent_;
//
// Info and error messages:
//
    std::ostringstream info_msg_;
    std::ostringstream error_msg_;
//
// The pixel statistics map which contains all the
// calibration information:
//
    staDataType *pixel_stat_map_;
    char        *badpix_map_;
    double      *offset_map_;
    double      *noise_map_;
    uint32_t     frame_width_;
    uint32_t     frame_height_;
    uint32_t     pix_count_;
// The pixel statistics map file streams:
// The bad pixel file streams:
    std::fstream   statmap_inpfile_;
    bool           inpfile_ok_;
    std::fstream   statmap_outfile_;
    bool           outfile_ok_;

};





#endif // DARK_FRAME_CALDATA_H

// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "Stroustrup"
// fill-column: 100
// End:

// end of file dark_frame_caldata.h

