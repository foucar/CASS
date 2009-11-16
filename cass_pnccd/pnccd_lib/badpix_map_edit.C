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


// badpix_map_edit.C
// Implementation of the bad pixel map editing class
// declared in badpix_map_edit.h

#include "badpix_map_edit.h"

BadpixMapEdit::BadpixMapEdit
(void)
{
// Set the private variables to default values:
    user_bpx_regions_.resize(0);
    num_user_regions_ = 0;
    max_user_regions_ = 10000;
    deleted_sys_regs_.resize(0);
    num_del_sysregs_  = 0;
    max_del_sysregs_  = 10000;
    memory_ok_        = false;
    bpxmap_addr_      = 0;
    bpxmap_bckp_      = 0;
    map_width_        = 0;
    map_height_       = 0;
    map_pixels_       = 0;
    inpfile_ok_       = false;
    outfile_ok_       = false;
}

BadpixMapEdit::~BadpixMapEdit
()
{
// Delete the local copy of the bad pixel map:
    if( bpxmap_bckp_ ) delete[] bpxmap_bckp_;
}

bool
BadpixMapEdit::setBadPixelMap
(char *bpxmap, const int& width, const int& height)
{
    uint32_t old_width;
    uint32_t old_height;

// Check the argument values:
    if( !bpxmap || (width < 1) || (height < 1) )
    {
	return false;
    }
    old_width  = map_width_;
    old_height = map_height_;
// Assign the bad pixel map address to the local pointer and
// set the map dimensions:
    bpxmap_addr_ = bpxmap;
    map_width_   = static_cast<uint32_t>(width);
    map_height_  = static_cast<uint32_t>(height);
    map_pixels_  = map_width_*map_height_;
// If the map size changed, reserve the memory for the backup
// map where the original bad pixel map is stored:
    if( (old_width  != map_width_) ||
	(old_height != map_height_) )
    {
	if( !this->allocMemResources_() )
	{
	    bpxmap_addr_ = 0;
	    map_width_   = 0;
	    map_height_  = 0;
	    map_pixels_  = 0;
	    return false;
	}
    }
// Copy the new bad pixel map to the backup map:
    if( !this->copyBpxmapToBackup_() )
    {
	return false;
    }

    return true;
}

bool
BadpixMapEdit::getModifiedBpxmap
(char **bpxmap, int *width, int *height)
{
// Check if the map address and the backup map are ok:
    if( !bpxmap_addr_ || (map_pixels_ < 1) || !memory_ok_ )
    {
	*bpxmap = 0;
	*width  = 0;
	*height = 0;
	return false;
    }
// Assign the local variables to the arguments:
    *bpxmap = bpxmap_addr_;
    *width  = map_width_;
    *height = map_height_;

    return true;
}

bool
BadpixMapEdit::setUserBadChannel
(const int& channel_x)
{
    uint32_t     i;
    bad_pix_type user_bpx;
    std::vector<bad_pix_type> user_bpxlist;

// Set the size of the user bad pixel vector to zero:
    user_bpxlist.resize(0);
// Limit the number of user selected bad pixel regions:
    if( num_user_regions_ >= max_user_regions_ ) return false;
// Check the input argument:
    if( channel_x < 1                                 ||
	static_cast<uint32_t>(channel_x) > map_width_ )
    {
	return false;
    }
// Check if the map address and the backup map are ok:
    if( !bpxmap_addr_ || (map_pixels_ < 1) || !memory_ok_ )
    {
	return false;
    }
// Set the pixels which are not yet flagged bad
// to BAD_USER:
    for( i=0; i<map_height_; i++ )
    {
// Check the bad pixel value in the backup map:
	if( bpxmap_bckp_[(channel_x - 1) + i*map_width_] != GOOD )
	{
	    bpxmap_addr_[(channel_x - 1) + i*map_width_] =
		bpxmap_bckp_[(channel_x - 1) + i*map_width_];
	}
	else
	{
	    bpxmap_addr_[(channel_x - 1) + i*map_width_] = BAD_USER;
// Store the bad pixel in the user bad pixel list:
	    user_bpx.pix_x = channel_x - 1;
	    user_bpx.pix_y = i;
	    user_bpx.flag  = BAD_USER;
	    user_bpxlist.push_back(user_bpx);
	}
    }
// Add the selected bad pixel region to the list of user
// defined bad pixel regions:
    num_user_regions_++;
    user_bpx_regions_.push_back(user_bpxlist);

    return true;
}

bool
BadpixMapEdit::setUserBadRow
(const int& row_y)
{
    uint32_t     i;
    bad_pix_type user_bpx;
    std::vector<bad_pix_type> user_bpxlist;

// Set the size of the user bad pixel vector to zero:
    user_bpxlist.resize(0);
// Limit the number of user selected bad pixel regions:
    if( num_user_regions_ >= max_user_regions_ ) return false;
// Check the input argument:
    if( row_y < 1                                  ||
	static_cast<uint32_t>(row_y) > map_height_ )
    {
	return false;
    }
// Check if the map address and the backup map are ok:
    if( !bpxmap_addr_ || (map_pixels_ < 1) || !memory_ok_ )
    {
	return false;
    }
// Set the pixels which are not yet flagged bad
// to BAD_USER:
    for( i=0; i<map_width_; i++ )
    {
// Check the bad pixel value in the backup map:
	if( bpxmap_bckp_[i + (row_y - 1)*map_width_] != GOOD )
	{
	    bpxmap_addr_[i + (row_y - 1)*map_width_] =
		bpxmap_bckp_[i + (row_y - 1)*map_width_];
	}
	else
	{
	    bpxmap_addr_[i + (row_y - 1)*map_width_] = BAD_USER;
// Store the bad pixel in the user bad pixel list:
	    user_bpx.pix_x = i;
	    user_bpx.pix_y = row_y - 1;
	    user_bpx.flag  = BAD_USER;
	    user_bpxlist.push_back(user_bpx);
	}
    }
// Add the selected bad pixel region to the list of user
// defined bad pixel regions:
    num_user_regions_++;
    user_bpx_regions_.push_back(user_bpxlist);

    return true;
}

bool
BadpixMapEdit::setUserBadRegion
(const int& btmleft_x,  const int& btmleft_y,
 const int& topright_x, const int& topright_y)
{
    int i;
    int j;
    int region_width;
    int region_height;
    bad_pix_type user_bpx;
    std::vector<bad_pix_type> user_bpxlist;

// Set the size of the user bad pixel vector to zero:
    user_bpxlist.resize(0);
// Limit the number of user selected bad pixel regions:
    if( num_user_regions_ >= max_user_regions_ ) return false;
// Check if the map address and the backup map are ok:
    if( !bpxmap_addr_ || (map_pixels_ < 1) || !memory_ok_ )
    {
	return false;
    }
// Get the size of the region:
    region_width  = topright_x - btmleft_x;
    region_height = topright_y - btmleft_y;
// Check the input arguments:
    if( (btmleft_x  < 1)                                  ||
	(static_cast<uint32_t>(btmleft_x)  > map_width_)  ||
	(btmleft_y  < 1)                                  ||
	(static_cast<uint32_t>(btmleft_y)  > map_height_) ||
	(topright_x < 1)                                  ||
	(static_cast<uint32_t>(topright_x) > map_width_)  ||
	(topright_y < 1)                                  ||
	(static_cast<uint32_t>(topright_y) > map_height_) ||
	(region_width < 0) || (region_height < 0) )
    {
	return false;
    }
// Set the pixels which are not yet flagged bad
// to BAD_USER:
    for( j=btmleft_y - 1; j < topright_y; j++ )
    {
	for( i=btmleft_x - 1; i < topright_x; i++ )
	{
// Check the bad pixel value in the backup map:
	    if( bpxmap_bckp_[i + j*map_width_] != GOOD )
	    {
		bpxmap_addr_[i + j*map_width_] = 
		    bpxmap_bckp_[i + j*map_width_];
	    }
	    else
	    {
		bpxmap_addr_[i + j*map_width_] = BAD_USER;
// Store the bad pixel in the user bad pixel list:
		user_bpx.pix_x = i;
		user_bpx.pix_y = j;
		user_bpx.flag  = BAD_USER;
		user_bpxlist.push_back(user_bpx);
	    }
	}
    }
// Add the selected bad pixel region to the list of user
// defined bad pixel regions:
    num_user_regions_++;
    user_bpx_regions_.push_back(user_bpxlist);

    return true;
}

bool
BadpixMapEdit::deleteSysPixInChannel
(const int& channel_x)
{
    uint32_t     i;
    bad_pix_type sys_bpx;
    std::vector<bad_pix_type> sys_bpxlist;

// Set the size of the system bad pixel vector to zero:
    sys_bpxlist.resize(0);
// Limit the number of deleted system bad pixel regions:
    if( num_del_sysregs_ >= max_del_sysregs_ ) return false;
// Check the input argument:
    if( channel_x < 1 || static_cast<uint32_t>(channel_x) > map_width_ )
    {
	return false;
    }
// Check if the map address and the backup map are ok:
    if( !bpxmap_addr_ || (map_pixels_ < 1) || !memory_ok_ )
    {
	return false;
    }
// Set the pixels which are flagged bad to GOOD and save the
// coordinate and the original flag:
    for( i=0; i<map_height_; i++ )
    {
	if( bpxmap_addr_[(channel_x - 1) + i*map_width_] == GOOD )
	{
	    continue;
	}
// Store the bad pixel in the removed system bad pixel list:
	switch( bpxmap_addr_[(channel_x - 1) + i*map_width_] )
	{
	    case BAD_USER:
		break;
	    default:
		sys_bpx.pix_x = channel_x - 1;
		sys_bpx.pix_y = i;
		sys_bpx.flag  = bpxmap_addr_[(channel_x - 1) + i*map_width_];
		sys_bpxlist.push_back(sys_bpx);
		break;
	}
// Set the pixel to GOOD:
	bpxmap_addr_[(channel_x - 1) + i*map_width_] = GOOD;
    }
// Add the deleted bad pixels to the list of user deleted
// system bad pixel regions:
    if( sys_bpxlist.size() > 0 )
    {
	num_del_sysregs_++;
	deleted_sys_regs_.push_back(sys_bpxlist);
    }

    return true;
}

bool
BadpixMapEdit::deleteSysPixInRow
(const int& row_y)
{
    uint32_t     i;
    bad_pix_type sys_bpx;
    std::vector<bad_pix_type> sys_bpxlist;

// Set the size of the system bad pixel vector to zero:
    sys_bpxlist.resize(0);
// Limit the number of deleted system bad pixel regions:
    if( num_del_sysregs_ >= max_del_sysregs_ ) return false;
// Check the input argument:
    if( row_y < 1 || static_cast<uint32_t>(row_y) > map_height_ )
    {
	return false;
    }
// Check if the map address and the backup map are ok:
    if( !bpxmap_addr_ || (map_pixels_ < 1) || !memory_ok_ )
    {
	return false;
    }
// Save the pixels which are flagged bad in the system pixel list
// and set them to GOOD:
    for( i=0; i<map_width_; i++ )
    {
	if( bpxmap_addr_[i + (row_y - 1)*map_width_] == GOOD )
	{
	    continue;
	}
// Store the bad pixel in the system generated bad pixel list:
	switch( bpxmap_addr_[i + (row_y - 1)*map_width_] )
	{
	    case BAD_USER:
		break;
	    default:
		sys_bpx.pix_x = i;
		sys_bpx.pix_y = row_y - 1;
		sys_bpx.flag  = bpxmap_addr_[i + (row_y - 1)*map_width_];
		sys_bpxlist.push_back(sys_bpx);
		break;
	}
// Set the pixel to GOOD:
	bpxmap_addr_[i + (row_y - 1)*map_width_] = GOOD;
    }
// Add the deleted bad pixel region to the list of deleted
// system bad pixel regions:
    if( sys_bpxlist.size() > 0 )
    {
	num_del_sysregs_++;
	deleted_sys_regs_.push_back(sys_bpxlist);
    }

    return true;
}

bool
BadpixMapEdit::deleteSysPixInRegion
(const int& btmleft_x,  const int& btmleft_y,
 const int& topright_x, const int& topright_y)
{
    int i;
    int j;
    int region_width;
    int region_height;
    bad_pix_type sys_bpx;
    std::vector<bad_pix_type> sys_bpxlist;

// Set the size of the system bad pixel vector to zero:
    sys_bpxlist.resize(0);
// Limit the number of user deleted system bad pixel regions:
    if( num_user_regions_ >= max_user_regions_ ) return false;
// Check if the map address and the backup map are ok:
    if( !bpxmap_addr_ || (map_pixels_ < 1) || !memory_ok_ )
    {
	return false;
    }
// Get the size of the region:
    region_width  = topright_x - btmleft_x;
    region_height = topright_y - btmleft_y;
// Check the input arguments:
    if( (btmleft_x  < 1)                                  ||
	(static_cast<uint32_t>(btmleft_x)  > map_width_)  ||
	(btmleft_y  < 1)                                  ||
	(static_cast<uint32_t>(btmleft_y)  > map_height_) ||
	(topright_x < 1)                                  ||
	(static_cast<uint32_t>(topright_x) > map_width_)  ||
	(topright_y < 1)                                  ||
	(static_cast<uint32_t>(topright_y) > map_height_) ||
	(region_width < 0) || (region_height < 0) )
    {
	return false;
    }
// Store the pixels which are flagged bad in the system bad
// pixel vector:
    for( j=btmleft_y - 1; j < topright_y; j++ )
    {
	for( i=btmleft_x - 1; i < topright_x; i++ )
	{
	    if( bpxmap_addr_[i + j*map_width_] == GOOD )
	    {
		continue;
	    }
// Store the bad pixel in the system bad pixel list:
	    switch( bpxmap_addr_[i + j*map_width_] )
	    {
		case BAD_USER:
		    break;
		default:
		    sys_bpx.pix_x = i;
		    sys_bpx.pix_y = j;
		    sys_bpx.flag  = bpxmap_addr_[i + j*map_width_];
		    sys_bpxlist.push_back(sys_bpx);
		    break;
	    }
// Set the bad pixel to GOOD:
	    bpxmap_addr_[i + j*map_width_] = GOOD;
	}
    }
// Add the deleted bad pixels to the list of removed
// system bad pixel regions:
    if( sys_bpxlist.size() > 0 )
    {
	num_del_sysregs_++;
	deleted_sys_regs_.push_back(sys_bpxlist);
    }

    return true;
}

bool
BadpixMapEdit::removeLastAddedRegion
(void)
{
    std::vector< std::vector<bad_pix_type> >::iterator
	reglist_itr;
    std::vector<bad_pix_type>::iterator
	pixlist_itr;

// Check if the map address and the backup map are ok:
    if( !bpxmap_addr_ || (map_pixels_ < 1) || !memory_ok_ )
    {
	return false;
    }
// Check whether any user bad pixel regions are defined:
    if( user_bpx_regions_.size() < 1 ) return false;
// Get the region which was added last:
    reglist_itr = --(user_bpx_regions_.end());
// Reset all user bad pixels which are defined in the
// region:
    pixlist_itr = reglist_itr->begin();
    while( pixlist_itr != reglist_itr->end() )
    {
// Set the pixel to 'GOOD':
	bpxmap_addr_[
	    pixlist_itr->pix_x
	    + pixlist_itr->pix_y*map_width_] = GOOD;
// Increment the iterator:
	pixlist_itr++;
    }
// Remove the cleared region from the end of the user
// region list:
    user_bpx_regions_.pop_back();

    return true;
}

bool
BadpixMapEdit::restoreLastRemovedRegion
(void)
{
    std::vector< std::vector<bad_pix_type> >::iterator
	reglist_itr;
    std::vector<bad_pix_type>::iterator
	pixlist_itr;

// Check if the map address and the backup map are ok:
    if( !bpxmap_addr_ || (map_pixels_ < 1) || !memory_ok_ )
    {
	return false;
    }
// Check whether any system bad pixel regions were removed:
    if( deleted_sys_regs_.size() < 1 ) return false;
// Get the region which was removed last:
    reglist_itr = --(deleted_sys_regs_.end());
// Reset all deleted bad pixels which are defined in the
// region to the values defined by the system:
    pixlist_itr = reglist_itr->begin();
    while( pixlist_itr != reglist_itr->end() )
    {
// Set the pixel to the original system value:
	bpxmap_addr_[
	    pixlist_itr->pix_x
	    + pixlist_itr->pix_y*map_width_] = pixlist_itr->flag;
// Increment the iterator:
	pixlist_itr++;
    }
// Remove the restored region from the end of the deleted
// system bad pixel region list:
    deleted_sys_regs_.pop_back();

    return true;
}

bool
BadpixMapEdit::restoreSystemBadPixels
(void)
{
    int i;

// Check the address of the original map and the backup
// map:
    if( !bpxmap_addr_     ||
	(map_width_ < 1)  ||
	(map_height_ < 1) ||
	!memory_ok_ )
    {
	return false;
    }
// Copy the system generated bad pixel entries from the backup
// map to the active bad pixel map:
    for( i=0; i<map_pixels_; i++ )
    {
	if( bpxmap_bckp_[i] == BAD_USER )
	{
	    continue;
	}
	bpxmap_addr_[i] = bpxmap_bckp_[i];
    }

    return true;
}

bool
BadpixMapEdit::readBadPixMapFromFile
(const std::string& fname)
{
    char                   ftest_line[512];
    uint32_t               i, j;
    uint32_t               badpix_x;
    uint32_t               badpix_y;
    uint32_t               badcol_x;
    uint32_t               badrow_y;
    uint32_t               badbox_btml_x;
    uint32_t               badbox_btml_y;
    uint32_t               badbox_topr_x;
    uint32_t               badbox_topr_y;
    std::string            ftest_string;
    std::istringstream     value_line;
    std::string::size_type token_pos;
    std::string::size_type vline_endpos;

// Check whether the bad pixel map address is not zero,
// or the bad pixel map size is zero:
    if( !bpxmap_addr_     ||
	(map_width_ < 1)  ||
	(map_height_ < 1) )
    {
	return false;
    }
// Check whether the input file is already open and
// close it if necessary:
    if( bpxmap_inpfile_.is_open() )
    {
	bpxmap_inpfile_.close();
	bpxmap_inpfile_.clear();
    }
// Try to open the file with the name defined in fname
// for reading:
    bpxmap_inpfile_.open(fname.c_str(),std::ios::in);
    if( !bpxmap_inpfile_.good() )
    {
// Could not open correctly, close and clear the stream
// state:
	inpfile_ok_ = false;
	bpxmap_inpfile_.close();
	bpxmap_inpfile_.clear();
	return false;
    }
    else
    {
	inpfile_ok_ = true;
    }
//
// Check whether bad pixels are defined in this file. If
// yes, add them to the user bad pixel list:
//

// Read at most 501 characters for each line of the input
// file and check whether a bad pixels are defined in the
// lines:
    while( inpfile_ok_ )
    {
	bpxmap_inpfile_.getline(ftest_line,502);
	if(!bpxmap_inpfile_.good() )
	{
// Could not read correctly, close and clear the stream
// state:
	    inpfile_ok_ = false;
	    bpxmap_inpfile_.close();
	    bpxmap_inpfile_.clear();
	    continue;
	}
	if( bpxmap_inpfile_.gcount() < 1 )
	{
// The file does not contain bad pixel data, close it
// and proceed:
	    inpfile_ok_ = false;
	    bpxmap_inpfile_.close();
	    bpxmap_inpfile_.clear();
	    continue;
	}
	else
	{
// Try to find the sequences "PIX", "COL", "ROW" or "BOX":
	    ftest_string = std::string(ftest_line);

	    value_line.str("");
	    if( (token_pos =
		 ftest_string.find("PIX")) != std::string::npos )
	    {
		token_pos += 3;
		vline_endpos  = ftest_string.find("\n");
		value_line.str(ftest_string.substr(
				   token_pos,vline_endpos)+" ");
		value_line >> badpix_x >> badpix_y;
// The value extraction failed, stop the extraction of further
// parameters and close the file:
		if( !value_line.good() )
		{
		    inpfile_ok_ = false;
		    bpxmap_inpfile_.close();
		    bpxmap_inpfile_.clear();
		    continue;
		}
// Add the bad pixel to the map:
		this->addBadPixelToMap_(badpix_x,badpix_y);
	    }
	    else if( (token_pos =
		     ftest_string.find("COL")) != std::string::npos )
	    {
		token_pos += 3;
		vline_endpos  = ftest_string.find("\n");
		value_line.str(ftest_string.substr(
				   token_pos,vline_endpos)+" ");
		value_line >> badcol_x;
// The value extraction failed, stop the extraction of further
// parameters and close the file:
		if( !value_line.good() )
		{
		    inpfile_ok_ = false;
		    bpxmap_inpfile_.close();
		    bpxmap_inpfile_.clear();
		    continue;
		}
// Add the bad column to the map:
		for( j=0; j<map_height_; j++ )
		{
		    this->addBadPixelToMap_(badcol_x,j);
		}
	    }
	    else if ( (token_pos =
		       ftest_string.find("ROW")) != std::string::npos )
	    {
		token_pos += 3;
		vline_endpos  = ftest_string.find("\n");
		value_line.str(ftest_string.substr(
				   token_pos,vline_endpos)+" ");
		value_line >> badrow_y;
// The value extraction failed, stop the extraction of further
// parameters and close the file:
		if( !value_line.good() )
		{
		    inpfile_ok_ = false;
		    bpxmap_inpfile_.close();
		    bpxmap_inpfile_.clear();
		    continue;
		}
// Add the bad row to the map:
		for( i=0; i<map_width_; i++ )
		{
		    this->addBadPixelToMap_(i,badrow_y);
		}
	    }
	    else if( (token_pos =
		      ftest_string.find("BOX")) != std::string::npos )
	    {
		token_pos += 3;
		vline_endpos  = ftest_string.find("\n");
		value_line.str(ftest_string.substr(
				   token_pos,vline_endpos)+" ");
		value_line >> badbox_btml_x >> badbox_btml_y
			   >> badbox_topr_x >> badbox_topr_y;
// The value extraction failed, stop the extraction of further
// parameters and close the file:
		if( !value_line.good() )
		{
		    inpfile_ok_ = false;
		    bpxmap_inpfile_.close();
		    bpxmap_inpfile_.clear();
		    continue;
		}
// Add the bad box to the map:
		for( j=badbox_btml_y; j<=badbox_topr_y; j++ )
		    for( i=badbox_btml_x; i<=badbox_topr_x; i++ )
		    {
			this->addBadPixelToMap_(i,j);
		    }
	    }
// Default, nothing found:
	    else
	    {
// None of the above tokens were found, continue the search.
// If the end of the file is reached, the file stream changes its
// state to 'bad' and the loop is stopped after trying to read a
// line, see above.
		continue;
	    }
	}

    }


    return true;
}

bool
BadpixMapEdit::writeBadPixMapToFile
(const std::string& fname, bool overwrite)
{
    char               ftest_line[512];
    uint32_t           i, j;
    uint32_t           badpix_x;
    uint32_t           badpix_y;
    std::string        ftest_string;
    std::ostringstream bpxfile_line;

// Check whether the bad pixel map address is not zero,
// or the bad pixel map size is zero:
    if( !bpxmap_addr_     ||
	(map_width_ < 1)  ||
	(map_height_ < 1) )
    {
	return false;
    }
// Check whether the output file is already open and
// close it if necessary:
    if( bpxmap_outfile_.is_open() )
    {
	bpxmap_outfile_.close();
	bpxmap_outfile_.clear();
    }
// Check whether the file should be overwritten and act
// accordingly:
    if( !overwrite )
    {
// Do not overwrite, try to open the file with the name
// defined in fname for reading
	bpxmap_outfile_.open(fname.c_str(),std::ios::in);
	if( !bpxmap_outfile_.good() )
	{
// Could not open correctly, close and clear the stream
// state:
	    outfile_ok_ = false;
	    bpxmap_outfile_.close();
	    bpxmap_outfile_.clear();
	    return false;
	}
//
// Check whether bad pixels are defined in this file:
//

// Read at most three characters from the beginning of the file
// and check whether a bad pixel token is found:
	bpxmap_outfile_.getline(ftest_line,4);
	if( bpxmap_outfile_.gcount() < 3 )
	{
// The file does not contain bad pixel data, close it
// and proceed:
	    outfile_ok_ = false;
	    bpxmap_outfile_.close();
	    bpxmap_outfile_.clear();
	}
	else
	{
// Try to find the sequences "PIX", "COL", "ROW" or "BOX":
	    ftest_string = std::string(ftest_line,3);
	    if( (ftest_string.find("PIX") != std::string::npos) ||
		(ftest_string.find("COL") != std::string::npos) ||
		(ftest_string.find("ROW") != std::string::npos) ||
		(ftest_string.find("BOX") != std::string::npos) )
	    {
// The sequence is found. This is a valid bad pixel file and
// overwrite is set false, stop and return with false.
		outfile_ok_ = false;
		bpxmap_outfile_.close();
		bpxmap_outfile_.clear();
		return false;
	    }
	}
    }

// No bad pixels are defined, close the file and open it for
// writing:
    if( bpxmap_outfile_.is_open() )
    {
	bpxmap_outfile_.close();
	bpxmap_outfile_.clear();
    }
    bpxmap_outfile_.open(fname.c_str(),
			 std::ios::out|std::ios::trunc);
    if( !bpxmap_outfile_.good() )
    {
// Could not open corrcetly, close and clear the stream
// state:
	outfile_ok_ = false;
	bpxmap_outfile_.close();
	bpxmap_outfile_.clear();
	return false;
    }
//
// Write the bad pixel file:
//

// Go through the bad pixel map:
    for( j=0; j<map_height_; j++ )
    {
	for( i=0; i<map_width_; i++ )
	{
	    if( bpxmap_addr_[i+j*map_width_] != GOOD )
	    {
		badpix_x = i;
		badpix_y = j;
// Create a line of the bad pixel file:
		bpxfile_line.str("");
		bpxfile_line << "PIX ";
		bpxfile_line << std::resetiosflags(
		    std::ios::left|std::ios::right);
		bpxfile_line.setf(std::ios::right);
		bpxfile_line.fill(' ');
		bpxfile_line.width(6);
		bpxfile_line << badpix_x;
		bpxfile_line << " ";
		bpxfile_line << std::resetiosflags(
		    std::ios::left|std::ios::right);
		bpxfile_line.setf(std::ios::right);
		bpxfile_line.fill(' ');
		bpxfile_line.width(6);
		bpxfile_line << badpix_y;
		bpxfile_line << "\n";
// Write the line to the file:
		bpxmap_outfile_ << bpxfile_line.str()
				<< std::flush;
	    }
	}
    }

// Finished, close the file and clear the stream state:
    if( bpxmap_outfile_.is_open() )
    {
	outfile_ok_ = false;
	bpxmap_outfile_.close();
	bpxmap_outfile_.clear();
    }

    return true;
}

//
// Private functions:
//

bool
BadpixMapEdit::addBadPixelToMap_
(uint32_t bpx_x, uint32_t bpx_y)
{
// Check whether the coordinates fit in the defined size
// of the bad pixel map:
    if( (bpx_x >= map_width_) || (bpx_y >= map_height_) )
    {
	return false;
    }
// Add the bad pixel to the map as a user bad pixel:
    if( bpxmap_addr_[bpx_x+map_width_*bpx_y] == GOOD )
    {
	bpxmap_addr_[bpx_x+map_width_*bpx_y] = BAD_USER;
    }

    return true;
}

bool
BadpixMapEdit::copyBpxmapToBackup_
(void)
{
    uint32_t i, j;
// Check the address of the original map and the backup
// map:
    if( !bpxmap_addr_     ||
	(map_width_ < 1)  ||
	(map_height_ < 1) ||
	!memory_ok_ )
    {
	return false;
    }
// Copy the pixel codes, row by row:
    for( j=0; j<map_height_; j++ )
    {
	for( i=0; i<map_width_; i++ )
	{
	    if( bpxmap_addr_[i+j*map_width_] == BAD_USER )
	    {
		continue;
	    }
	    bpxmap_bckp_[i+j*map_width_]
		= bpxmap_addr_[i+j*map_width_];
	}
    }

    return true;
}

bool
BadpixMapEdit::allocMemResources_
(void)
{
    int i;
// Delete the old memory if necessary:
    if( bpxmap_bckp_ ) delete[] bpxmap_bckp_;
// Reserve a new memory block:
    if( map_pixels_ < 1 )
    {
	memory_ok_ = false;
	return false;
    }
    bpxmap_bckp_ = new char[map_pixels_];
    if( !bpxmap_bckp_ )
    {
	bpxmap_bckp_ = 0;
	memory_ok_   = false;
	return false;
    }
// Initialize the bad pixel map backup with 'GOOD'
// flags:
    for( i=0; i<map_pixels_; i++ )
    {
	bpxmap_bckp_[i] = GOOD;
    }
    memory_ok_ = true;

    return true;
}

// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "Stroustrup"
// fill-column: 100
// End:

// end of file badpix_map_edit.C

