// pnCCDEvent.cpp : Nils Kimmel 2009
// The pnCCD detector data that result from one machine event
// of e.g. a Free Electron Laser. This data collection is pipe-
// lined through the operator() of the class cass::pnCCD::Analysis
// and generates the basic analysis products for a user of raw
// pnCCD frames.

#include <iostream>
#include "pnccd_event.h"

// Default constructor for an empty event with safe default values:

cass::pnCCD::pnCCDEvent::pnCCDEvent
(void)
{

// Per default, two detector arrays are foreseen in CAMP:
  num_pixel_arrays_ = 2;
// Resize the vectors which hold the event data:
  array_x_size_.resize(num_pixel_arrays_);
  array_y_size_.resize(num_pixel_arrays_);
  max_photons_per_event_.resize(num_pixel_arrays_);
  raw_signal_values_.resize(num_pixel_arrays_);
  corr_signal_values_.resize(num_pixel_arrays_);
  unrec_photon_hits_.resize(num_pixel_arrays_);
  num_unrec_phits_.resize(num_pixel_arrays_);
  recom_photon_hits_.resize(num_pixel_arrays_);
  num_recom_phits_.resize(num_pixel_arrays_);
// Set the default size values and initialize all array addresses
// with zeros:
  for( int i=0; i<num_pixel_arrays_; i++ )
  {
    array_x_size_.at(i)          = 1024;
    array_y_size_.at(i)          = 1024;
    max_photons_per_event_.at(i) = 2048;
    raw_signal_values_.at(i)     = 0;
    corr_signal_values_.at(i)    = 0;
    unrec_photon_hits_.at(i)     = 0;
    recom_photon_hits_.at(i)     = 0;
  }
// Reserve memory for the default event:
  this->initEventStorage();

}

// Create the event with given parameters for the geometry
// of a specific number of detectors and a given maximum
// number of photon hits stored for each event:

cass::pnCCD::pnCCDEvent::pnCCDEvent
(uint16_t num_pixel_arrays,
 std::vector<uint32_t> array_x_size,
 std::vector<uint32_t> array_y_size,
 std::vector<uint32_t> max_photons_per_event)
{
// Check whether the argument values are consistent:
  if( num_pixel_arrays != array_x_size.size() ||
      num_pixel_arrays != array_y_size.size() ||
      num_pixel_arrays != max_photons_per_event.size() )
  {
    return;
  }
// Set the number of detector/pixel arrays:
  num_pixel_arrays_ = num_pixel_arrays;
// Assign the arrays which carry the size information:
  array_x_size_ = array_x_size;
  array_y_size_ = array_y_size;
  max_photons_per_event_ = max_photons_per_event;
// Resize the vectors which hold the event data:
  raw_signal_values_.resize(num_pixel_arrays_);
  corr_signal_values_.resize(num_pixel_arrays_);
  unrec_photon_hits_.resize(num_pixel_arrays_);
  num_unrec_phits_.resize(num_pixel_arrays_);
  recom_photon_hits_.resize(num_pixel_arrays_);
  num_recom_phits_.resize(num_pixel_arrays_);
// Initialize all array addresses with zeros:
  for( int i=0; i<num_pixel_arrays_; i++ )
  {
    raw_signal_values_.at(i)  = 0;
    corr_signal_values_.at(i) = 0;
    unrec_photon_hits_.at(i)  = 0;
    recom_photon_hits_.at(i)  = 0;
  }
}

cass::pnCCD::pnCCDEvent::~pnCCDEvent
()
{
// Free the memory resources which were allocated for this event:
  for( size_t i=0; i<raw_signal_values_.size(); i++ )
  {
    delete[] raw_signal_values_.at(i);
  }
  for( size_t i=0; i<corr_signal_values_.size(); i++ )
  {
    delete[] corr_signal_values_.at(i);
  }
  for( size_t i=0; i<unrec_photon_hits_.size(); i++ )
  {
    delete[] unrec_photon_hits_.at(i);
  }
  for( size_t i=0; i<recom_photon_hits_.size(); i++ )
  {
    delete[] recom_photon_hits_.at(i);
  }
}

// assignment operator that will only copy the relevant data
cass::pnCCD::pnCCDEvent&
cass::pnCCD::pnCCDEvent::operator =
(const cass::pnCCD::pnCCDEvent& rhs)
{
    if (this != &rhs) // protect against invalid self-assignment
    {
        array_x_size_ = rhs.array_x_size_;
        array_y_size_ = rhs.array_y_size_;
    }
    return *this;
}


// Initialize the event with the data stored in an xtc:

// Get information from the file header:

bool
cass::pnCCD::pnCCDEvent::init
(fileHeaderType* pnccd_fhdr, uint32_t ccd_id)
{
  if( !pnccd_fhdr ) return false;

// Set the frame size of the CCD with id ccd_id, this is
// needed for the correct initialization of the raw and
// corrected pixel signal data:
  array_x_size_.at(ccd_id-1) = pnccd_fhdr->the_width;
  array_y_size_.at(ccd_id-1) = pnccd_fhdr->the_maxHeight;

  return true;
}

// Get information from the frame header:

bool
cass::pnCCD::pnCCDEvent::init
(frameHeaderType* pnccd_frame, uint32_t ccd_id)
{
  uint32_t  width;
//  uint32_t  height;
  uint64_t  datasize;
  pxType   *frm_data;

  if( !pnccd_frame ) return false;
// Get the start address of the frame data block. In memory
// The frame data block begins directly after the frame
// header:
  frm_data = reinterpret_cast<pxType*>(pnccd_frame + 1);
// Check whether the geometry information in the frame header
// corresponds to the initialization parameters of the pnCCDEvent:
  width = pnccd_frame->the_height;
// Test version: do nothing by now!
//  if( width != array_x_size_.at(ccd_id) ) ;
  datasize = array_x_size_.at(ccd_id-1)
            *array_y_size_.at(ccd_id-1)
            *sizeof(pxType);
// Copy the contents of the memory region of the frame in
// the xtc datagram to the corresponding memory region in
// the cass::pnCCD::pnCCDEvent :
  memcpy(this->raw_signal_values_.at(ccd_id-1),
         frm_data,
         datasize);

  return true;
}


// Initialize the storage of pnCCD events with the previously
// defined size parameters:

bool
cass::pnCCD::pnCCDEvent::initEventStorage
(void)
{
  if( num_pixel_arrays_ < 1 ) return false;
// Allocate memory for the event data storage:
  for( int i=0; i<num_pixel_arrays_; i++ )
  {
    raw_signal_values_.at(i) =
        new uint16_t[array_x_size_.at(i)*array_y_size_.at(i)];
    corr_signal_values_.at(i) =
        new uint16_t[array_x_size_.at(i)*array_y_size_.at(i)];
    unrec_photon_hits_.at(i) =
        new pnccd_photon_hit[max_photons_per_event_.at(i)];
    recom_photon_hits_.at(i) =
        new pnccd_photon_hit[max_photons_per_event_.at(i)];
  }

  return true;
}

uint16_t
cass::pnCCD::pnCCDEvent::getNumPixArrays
(void)
{
  return num_pixel_arrays_;
}

std::vector<uint32_t>
cass::pnCCD::pnCCDEvent::getArrXSize
(void)
{
  return array_x_size_;
}

std::vector<uint32_t>
cass::pnCCD::pnCCDEvent::getArrYSize
(void)
{
  return array_y_size_;
}

std::vector<uint32_t>
cass::pnCCD::pnCCDEvent::getMaxPhotPerEvt
(void)
{
  return max_photons_per_event_;
}

uint16_t*
cass::pnCCD::pnCCDEvent::rawSignalArrayAddr
(uint16_t index)
{
  if( (index < 1) || (index > num_pixel_arrays_) ) return 0;
  if( !raw_signal_values_.at(index - 1) )          return 0;

  return raw_signal_values_.at(index - 1);
}

uint32_t
cass::pnCCD::pnCCDEvent::rawSignalArrayByteSize
(uint16_t index)
{
  return sizeof(uint16_t)*array_x_size_.at(index - 1)
                         *array_y_size_.at(index - 1);
}

uint16_t*
cass::pnCCD::pnCCDEvent::corrSignalArrayAddr
(uint16_t index )
{
  if( (index < 1) || (index > num_pixel_arrays_) ) return 0;
  if( !corr_signal_values_.at(index - 1) )         return 0;

  return corr_signal_values_.at(index - 1);
}

uint32_t
cass::pnCCD::pnCCDEvent::corrSignalArrayByteSize
(uint16_t index)
{
  return sizeof(uint16_t)*array_x_size_.at(index - 1)
                         *array_y_size_.at(index - 1);
}

cass::pnCCD::pnccd_photon_hit*
cass::pnCCD::pnCCDEvent::unrecPhotonHitAddr
(uint16_t index)
{
  if( (index < 1) || (index > num_pixel_arrays_) ) return 0;
  if( !unrec_photon_hits_.at(index - 1) )          return 0;

  return unrec_photon_hits_.at(index - 1);
}

uint32_t
cass::pnCCD::pnCCDEvent::numUnrecPhotonHits
(uint16_t index)
{
  return num_unrec_phits_.at(index - 1);
}

cass::pnCCD::pnccd_photon_hit*
cass::pnCCD::pnCCDEvent::recomPhotonHitAddr
(uint16_t index)
{
  if( (index < 1) || (index > num_pixel_arrays_) ) return 0;
  if( !recom_photon_hits_.at(index - 1) )          return 0;

  return recom_photon_hits_.at(index - 1);
}

uint32_t
cass::pnCCD::pnCCDEvent::numRecomPhotonHits
(uint16_t index)
{
  return num_recom_phits_.at(index - 1);
}

// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:

// end of file pnCCDEvent.cpp


