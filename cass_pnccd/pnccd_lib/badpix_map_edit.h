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


// badpix_map_edit.h
// A class that provides editing functionality for the
// bad pixel map. User selected bad pixel regions can be
// added and removed. The system generated bad pixels
// keep their original flag if they are located inside
// of a user selected bad pixel region.

#ifndef BADPIX_MAP_EDIT_H
#define BADPIX_MAP_EDIT_H

#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <deque>

#include <inttypes.h>

#include "xonline_constants.h"

class BadpixMapEdit
{
public:
    BadpixMapEdit(void);
    ~BadpixMapEdit();
// Set the system generated bad pixel map:
    bool setBadPixelMap(char *bpxmap,
			const int& width,
			const int& height);
// Get the modified bad pixel map:
    bool getModifiedBpxmap(char **bpxmap, int *width, int *height);
// Add bad pixel regions with the BAD_USER flag:
    bool setUserBadChannel(const int& channel_x);
    bool setUserBadRow(const int& row_y);
    bool setUserBadRegion(
	const int& btmleft_x,  const int& btmleft_y,
	const int& topright_x, const int& topright_y);
// Delete bad pixel regions which were set by the system:
    bool deleteSysPixInChannel(const int& channel_x);
    bool deleteSysPixInRow(const int& row_y);
    bool deleteSysPixInRegion(
	const int& btmleft_x,  const int& btmleft_y,
	const int& topright_x, const int& topright_y);
// Remove the last region which was added to the bad
// pixel map, but keep the system generated bad pixels:
    bool removeLastAddedRegion(void);
// Restore the system generated bad pixels in the region
// which was last deleted:
    bool restoreLastRemovedRegion(void);
// Restore the system generated bad pixel map:
    bool restoreSystemBadPixels(void);
// Read a bad pixel map from a file:
    bool readBadPixMapFromFile(const std::string& fname);
// Write the current bad pixel map to a file:
    bool writeBadPixMapToFile(const std::string& fname,
			      bool overwrite=false);
private:
// The pixel coordinate data structure with the bad pixel
// type flag:
    typedef struct
    {
	int pix_x;
	int pix_y;
	char flag;
    } bad_pix_type;
//
// Private function members:
//
    bool addBadPixelToMap_(uint32_t bpx_x, uint32_t bpx_y);
    bool copyBpxmapToBackup_(void);
    bool allocMemResources_(void);
//
// The storage container for the user added bad pixels:
//
    std::vector< std::vector<bad_pix_type> > user_bpx_regions_;
    int            num_user_regions_;
    int            max_user_regions_;
    std::vector< std::vector<bad_pix_type> > deleted_sys_regs_;
    int            num_del_sysregs_;
    int            max_del_sysregs_;

    bool           memory_ok_;
    char          *bpxmap_addr_;
    char          *bpxmap_bckp_;
    uint32_t       map_width_;
    uint32_t       map_height_;
    int            map_pixels_;
// The bad pixel file streams:
    std::fstream   bpxmap_inpfile_;
    bool           inpfile_ok_;
    std::fstream   bpxmap_outfile_;
    bool           outfile_ok_;
};


#endif // BADPIX_MAP_EDIT_H

// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "Stroustrup"
// fill-column: 100
// End:

// end of file badpix_map_edit.h


