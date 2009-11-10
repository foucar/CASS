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

// dark_frame_caldata.C
// Implementation of the class DarkFrameCaldata and its methods
// as declared in dark_frame_caldata.h . Dark frame calibration
// data generated with FrameData can be saved or reread to/from
// a file. Note that bad pixel maps are handled by BadPixMapEdit.
// The pixel statistics data is stored locally in DarkFrameCaldata
// after it has been set or read from a file:

#include "dark_frame_caldata.h"

DarkFrameCaldata::DarkFrameCaldata
(void)
{
// Set the defaults for the private member variables:
    info_msg_.str("");
    error_msg_.str("");

    pixel_stat_map_ = 0;
    badpix_map_     = 0;
    offset_map_     = 0;
    noise_map_      = 0;
    frame_width_    = 0;
    frame_height_   = 0;
    pix_count_      = 0;
    inpfile_ok_     = false;
    outfile_ok_     = false;

    return;
}

DarkFrameCaldata::~DarkFrameCaldata
()
{
    this->cleanUpLocalStorage_();
}

bool
DarkFrameCaldata::setPixelStatMapAddr
(staDataType *pix_stats, char *badpix_flags,
 uint32_t width, uint32_t height)
{
    uint32_t i;

// Check the argument values:
    if( !pix_stats || (width < 1) || (height < 1) )
    {
	error_msg_.str("");
	error_msg_ << "Error in setPixelStatMapAddr(), in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , invalid function arguments: width = "
		   << width << " , height = " << height
		   << " , address of pix_stats = " << pix_stats;
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	return false;
    }
// Check whether a memory (re-)allocation is necessary:
    if( (width != frame_width_) || (height != frame_height_) )
    {
	frame_width_  = width;
	frame_height_ = height;
	pix_count_    = frame_width_*frame_height_;

	if( !this->allocLocalStorage_() )
	{
	    frame_width_  = 0;
	    frame_height_ = 0;
	    pix_count_    = 0;
	    return false;
	}
    }
// Copy the statistics data and the bad pixel flag map to
// the local storage arrays:
    for( i=0; i<pix_count_; i++ )
    {
	pixel_stat_map_[i] = pix_stats[i];
	badpix_map_[i]     = badpix_flags[i];
    }
// Create the offset and noise maps:
    if( !this->createOffsetMap_() ) return false;
    if( !this->createNoiseMap_()  ) return false;

    return true;
}

staDataType*
DarkFrameCaldata::getPixelStatMapAddr
(uint32_t *width, uint32_t *height)
{
// Assign the private member values to the arguments,
// return zero if the private arrays have not been
// allocated:
    if( (frame_width_  < 1 ) ||
	(frame_height_ < 1 ) ||
	(pix_count_    < 1) )
    {
	return reinterpret_cast<staDataType*>(0);
    }
    *width        = frame_width_;
    *height       = frame_height_;

    return pixel_stat_map_;
}

char*
DarkFrameCaldata::getBadPixelMapAddr
(uint32_t *width, uint32_t *height)
{
// Assign the private member values to the arguments,
// return zero if the private arrays have not been
// allocated:
    if( (frame_width_  < 1 ) ||
	(frame_height_ < 1 ) ||
	(pix_count_    < 1) )
    {
	return reinterpret_cast<char*>(0);
    }
    *width        = frame_width_;
    *height       = frame_height_;

    return badpix_map_;
}

double*
DarkFrameCaldata::getOffsetMapAddr
(uint32_t *width, uint32_t *height)
{
// Assign the private member values to the arguments,
// return zero if the private arrays have not been
// allocated:
    if( (frame_width_  < 1 ) ||
	(frame_height_ < 1 ) ||
	(pix_count_    < 1) )
    {
	return reinterpret_cast<double*>(0);
    }
    *width        = frame_width_;
    *height       = frame_height_;

    return offset_map_;
}

double*
DarkFrameCaldata::getNoiseMapAddr
(uint32_t *width, uint32_t *height)
{
// Assign the private member values to the arguments,
// return zero if the private arrays have not been
// allocated:
    if( (frame_width_  < 1 ) ||
	(frame_height_ < 1 ) ||
	(pix_count_    < 1) )
    {
	return reinterpret_cast<double*>(0);
    }
    *width        = frame_width_;
    *height       = frame_height_;

    return noise_map_;
}

bool
DarkFrameCaldata::writePixelStatMapToFile
(const std::string& out_fname, bool overwrite)
{
// Write the pixel statistics map to a file. The file
// begins with a header structure with a size of
// 1024 bytes:
//
    char        ftest_line[512];
    char        header_intro[24] = "HE pixel statistics map";
    char        fill_rest[988]; // Fill up to 1024 bytes
    uint32_t    width, height, length;
    uint32_t    statmap_bytesz, bpxmap_bytesz;
    std::string ftest_string;

// Initialize the fill array with zeros:
    memset(fill_rest,0,988);
// Check whether the bad pixel map address is not zero,
// or the bad pixel map size is zero:
//    if( !pixel_stat_map_    ||
    if(
	(frame_width_ < 1)  ||
	(frame_height_ < 1) ||
	(pix_count_ < 1) )
    {
	error_msg_.str("");
	error_msg_ << "Error in writePixelStatMapToFile_(), in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , the pixel statistics map data has not"
		   << " yet been set!";
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	return false;
    }
// Check whether the output file is already open and
// close it if necessary:
    if( statmap_outfile_.is_open() )
    {
	statmap_outfile_.close();
	statmap_outfile_.clear();
    }
// Check whether the file should be overwritten and act
// accordingly:
    if( !overwrite )
    {
// Do not overwrite, try to open the file with the name
// defined in fname for reading
	statmap_outfile_.open(out_fname.c_str(),std::ios::in);
	if( !statmap_outfile_.good() )
	{
	    error_msg_.str("");
	    error_msg_ << "Error in writePixelStatMapToFile_(), in file: "
		       << __FILE__ << " , in line: " << __LINE__
		       << " , the file with the name '"
		       << out_fname
		       << "' could not be opened!";
//	    AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
// Could not open correctly, close and clear the stream
// state:
	    outfile_ok_ = false;
	    statmap_outfile_.close();
	    statmap_outfile_.clear();
	    return false;
	}
// Check whether pixel statistics data are defined in this file
// and stop the write attempt if true:
	statmap_outfile_.getline(ftest_line,24);
	if( statmap_outfile_.gcount() < 23 )
	{
// No statistics data are defined in this file: close, clear,
// and proceed:
	    outfile_ok_ = false;
	    statmap_outfile_.close();
	    statmap_outfile_.clear();
	}
	else
	{
// Try to find the sequence "HE pixel statistics" :
	    ftest_string = std::string(ftest_line,23);
	    if( ftest_string.find("HE pixel statistics")
		!= std::string::npos )
	    {
// The sequence is found: this is a valid pixel statistics data
// file. Overwrite is false, stop and return with false:
	    error_msg_.str("");
	    error_msg_ << "Error in writePixelStatMapToFile_(), in file: "
		       << __FILE__ << " , in line: " << __LINE__
		       << " , the file with the name '"
		       << out_fname
		       << "' contains pixel statistics data, but"
		       << " overwrite is set false!";
//	    AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
// Could not open correctly, close and clear the stream
// state:
	    outfile_ok_ = false;
	    statmap_outfile_.close();
	    statmap_outfile_.clear();
	    return false;
	    }
	}
    }
// Overwrite is true or no pixel statistics data are defined in this
// file, close the file if opened before and open it for writing:
    if( statmap_outfile_.is_open() )
    {
	statmap_outfile_.close();
	statmap_outfile_.clear();
    }
    statmap_outfile_.open(out_fname.c_str(),
			  std::ios::out|std::ios::trunc);
    if( !statmap_outfile_.good() )
    {
	error_msg_.str("");
	error_msg_ << "Error in writePixelStatMapToFile_(), in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , the file with the name '"
		   << out_fname
		   << "' could not be opened for writing!";
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
// Could not open corrcetly, close and clear the stream
// state:
	outfile_ok_ = false;
	statmap_outfile_.close();
	statmap_outfile_.clear();
	return false;
    }
// Write the file header data:
    width  = frame_width_;
    height = frame_height_;
    length = pix_count_;

    statmap_outfile_.write(header_intro,24);
    statmap_outfile_.write(reinterpret_cast<char*>(&width),4);
    statmap_outfile_.write(reinterpret_cast<char*>(&height),4);
    statmap_outfile_.write(reinterpret_cast<char*>(&length),4);
    statmap_outfile_.write(fill_rest,988);
// Write the pixel statistics map:
    statmap_bytesz = sizeof(staDataType)*pix_count_;
    statmap_outfile_.write(
	reinterpret_cast<char*>(pixel_stat_map_),statmap_bytesz);
// Write the bad pixel flag map:
    bpxmap_bytesz  = sizeof(char)*pix_count_;
    statmap_outfile_.write(
	reinterpret_cast<char*>(badpix_map_),bpxmap_bytesz);
// Finished, close the file and clear the stream state:
    if( statmap_outfile_.is_open() )
    {
	outfile_ok_ = false;
	statmap_outfile_.close();
	statmap_outfile_.clear();
    }

    return true;
}

bool
DarkFrameCaldata::readPixelStatMapFromFile
(const std::string& in_fname)
{
// Read the pixel statistics map from a file. The file
// begins with a header structure with a size of
// 1024 bytes:
//
    char               read_buffer[2048];
    uint32_t           width, height, length;
    uint32_t           statmap_bytesz, bpxmap_bytesz;
    std::string        ftest_string;
    std::istringstream header_istream;

// Check whether the input file is already open and
// close it if necessary:
    if( statmap_inpfile_.is_open() )
    {
	statmap_inpfile_.close();
	statmap_inpfile_.clear();
    }

// Try to open the file with the name
// defined in fname for reading
    statmap_inpfile_.open(in_fname.c_str(),std::ios::in);
    if( !statmap_inpfile_.good() )
    {
	error_msg_.str("");
	error_msg_ << "Error in readPixelStatMapToFile_(), in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , the file with the name '"
		   << in_fname
		   << "' could not be opened!";
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
// Could not open correctly, close and clear the stream
// state:
	inpfile_ok_ = false;
	statmap_inpfile_.close();
	statmap_inpfile_.clear();
	return false;
    }
// Check whether pixel statistics data are defined in this file
// and stop the read attempt if true:

// Read the header data first:
    statmap_inpfile_.read(read_buffer,1024);
    if( statmap_inpfile_.gcount() < 1024 )
    {
// No statistics data are defined in this file: close, clear,
// and stop reading:
	error_msg_.str("");
	error_msg_ << "Error in readPixelStatMapToFile_(), in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , the file with the name '"
		   << in_fname
		   << "' does not contain pixel statistics data!";
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	
	inpfile_ok_ = false;
	statmap_inpfile_.close();
	statmap_inpfile_.clear();
    }
    else
    {
// Try to find the sequence "HE pixel statistics" :
	ftest_string = std::string(read_buffer,24);
	if( ftest_string.find("HE pixel statistics map")
	    == std::string::npos )
	{
// The sequence is not found: this is not a valid pixel statistics
// data file. Stop further attempts to read the data.
	    error_msg_.str("");
	    error_msg_ << "Error in readPixelStatMapToFile_(), in file: "
		       << __FILE__ << " , in line: " << __LINE__
		       << " , the file with the name '"
		       << in_fname
		       << "' does not contain pixel statistics data!";
//	    AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());

	    inpfile_ok_ = false;
	    statmap_inpfile_.close();
	    statmap_inpfile_.clear();
	    return false;
	}
    }
// Put the header data into an istringstream:
    header_istream.str(std::string(read_buffer,1024));
// Put the read pointer to the position just before the
// 'width' entry of the header:
    header_istream.seekg(24,std::ios::beg);
// Read the width, height, and length information into the
// corresponding variables:
    header_istream.read(reinterpret_cast<char*>(&width),4);
    header_istream.read(reinterpret_cast<char*>(&height),4);
    header_istream.read(reinterpret_cast<char*>(&length),4);

// Test: print the values in the shell window:
    std::cout << "\n Reading calib data: "
	      << " width  = " << width
	      << " , height = " << height
	      << " , length = " << length << "\n" << std::flush;
// Check the width, height, and length information:
    if( width*height != length )
    {
	error_msg_.str("");
	error_msg_ << "Error in readPixelStatMapToFile_(), in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , the array size information in the file '"
		   << in_fname
		   << "' is not consistent!";
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	
	inpfile_ok_ = false;
	statmap_inpfile_.close();
	statmap_inpfile_.clear();
	return false;
    }
// Reallocate the private storage arrays if the frame size
// changed:
    if( frame_width_  != width  ||
	frame_height_ != height )
    {
// Accept the size information in the file:
	frame_width_  = width;
	frame_height_ = height;
	pix_count_    = length;
	if( !this->allocLocalStorage_() ) return false;
    }
// Calculate the sizes of the data blocks:
    statmap_bytesz = sizeof(staDataType)*pix_count_;
    bpxmap_bytesz  = sizeof(char)*pix_count_;
// Read the pixel statistics data:
    statmap_inpfile_.read(
	reinterpret_cast<char*>(pixel_stat_map_),
	statmap_bytesz);
// Read the bad pixel flag data:
    statmap_inpfile_.read(
	reinterpret_cast<char*>(badpix_map_),
	bpxmap_bytesz);

// Finished, close the file and clear the stream state:
    if( statmap_inpfile_.is_open() )
    {
	inpfile_ok_ = false;
	statmap_inpfile_.close();
	statmap_inpfile_.clear();
    }
// Finally create the offset and noise maps:
    if( !this->createOffsetMap_() ) return false;
    if( !this->createNoiseMap_()  ) return false;
    
    return true;
}

//
// Private function members:
//

bool
DarkFrameCaldata::allocLocalStorage_
(void)
{
    if( pixel_stat_map_ ) delete[] pixel_stat_map_;
    pixel_stat_map_ = new staDataType[pix_count_];
    if( !pixel_stat_map_ )
    {
	error_msg_.str("");
	error_msg_ << "Error in allocLocalStorage_(), in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , failed to allocate memory for the"
		   << " pixel statistics data storage!";
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	this->cleanUpLocalStorage_();
	return false;
    }

    if( badpix_map_ ) delete[] badpix_map_;
    badpix_map_ = new char[pix_count_];
    if( !badpix_map_ )
    {
	error_msg_.str("");
	error_msg_ << "Error in allocLocalStorage_(), in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , failed to allocate memory for the"
		   << " bad pixel map storage!";
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	this->cleanUpLocalStorage_();
	return false;
    }

    if( offset_map_ ) delete[] offset_map_;
    offset_map_ = new double[pix_count_];
    if( !offset_map_ )
    {
	error_msg_.str("");
	error_msg_ << "Error in allocLocalStorage_(), in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , failed to allocate memory for the"
		   << " offset map storage!";
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	this->cleanUpLocalStorage_();
	return false;
    }

    if( noise_map_ ) delete[] noise_map_;
    noise_map_ = new double[pix_count_];
    if( !noise_map_ )
    {
	error_msg_.str("");
	error_msg_ << "Error in allocLocalStorage_(), in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , failed to allocate memory for the"
		   << " noise map storage!";
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	this->cleanUpLocalStorage_();
	return false;
    }


    return true;
}

bool
DarkFrameCaldata::cleanUpLocalStorage_
(void)
{
    if( pixel_stat_map_ )
    {
	delete[] pixel_stat_map_;
	pixel_stat_map_ = 0;
    }
    if( badpix_map_ )
    {
	delete[] badpix_map_;
	badpix_map_ = 0;
    }
    if( offset_map_ )
    {
	delete[] offset_map_;
	offset_map_ = 0;
    }
    if( noise_map_ ) 
    {
	delete[] noise_map_;
	noise_map_ = 0;
    }

    frame_width_ = frame_height_ = pix_count_ = 0;

    return true;
}

bool
DarkFrameCaldata::createOffsetMap_
(void)
{
    uint32_t     i;
    double      *offsetmap_ptr;
    staDataType *pix_stats;

// Check whether the number of pixels has been set:
    if( pix_count_ < 1 ) return false;
// Assign the local pointers to the global arrays:
    offsetmap_ptr = offset_map_;
    pix_stats     = pixel_stat_map_;
// Loop over all array entries:
    for( i=0; i<pix_count_; i++, offsetmap_ptr++, pix_stats++ ) {
	*offsetmap_ptr = pix_stats->offset;
    }

    return true;
}

bool
DarkFrameCaldata::createNoiseMap_
(void)
{
    uint32_t     i;
    double      *noisemap_ptr;
    staDataType *pix_stats;

// Check whether the number of pixels has been set:
    if( pix_count_ < 1 ) return false;
// Assign the local pointers to the global arrays:
    noisemap_ptr  = noise_map_;
    pix_stats     = pixel_stat_map_;
// Loop over all array entries:
    for( i=0; i<pix_count_; i++, noisemap_ptr++, pix_stats++ ) {
	*noisemap_ptr = pix_stats->sigma;
    }

    return true;
}

// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "Stroustrup"
// fill-column: 100
// End:

// end of file dark_frame_caldata.C

