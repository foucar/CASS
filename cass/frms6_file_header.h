// Copyright (C) 2011 Lutz Foucar

/**
 * @file frms6_file_header.h contains the layout of the lma file headers.
 *
 * @author Lutz Foucar
 */

#include <stdint.h>
#include <vector>
#include <fstream>
#include <stdexcept>

#include <QtCore/QDataStream>

namespace cass
{
namespace frms6File
{
/** a raw sss frame */
typedef  int16_t pixel;
typedef std::vector<pixel> frame_t;

#pragma pack(4)
/** the file header
 *
 * took from fformat.h from pnCCD lib. Size should be 1024 bytes
 *
 * @author Peter Holl
 * @author Nils Kimmel
 */
struct FileHeader
  {
  /** number of bytes in file header */
  uint16_t myLength;

  /** number of bytes in frame header */
  uint16_t fhLength;

  /** number of CCD to be read out */
  uint8_t nCCDs;

  /** number of channels */
  uint8_t width;

  /** (maximum) number of lines */
  uint8_t maxHeight;

  /** format version */
  uint8_t version;

  /** identification of data */
  char dataSetID[80];

// following lines added for Version 6

  /** the true number of channels */
  uint16_t the_width;

  /** the true (maximum) number of lines */
  uint16_t the_maxHeight;

  /** fill up space to get the complete size of 1024 bytes*/
  char fill[932];
  } ;

/** the file header
 *
 * took from fformat.h from pnCCD lib. Size should be 64 bytes
 *
 * @author Peter Holl
 * @author Nils Kimmel
 */
struct FrameHeader
{
  /** starting line, indicates window mode if not */
  uint8_t start;

  /** info byte */
  uint8_t info;

  /** CCD id */
  uint8_t id;

  /** number of lines in following frame */
  uint8_t height;

  /** start data taking time in seconds */		// this must not be time_t !!
  uint32_t tv_sec;

  /** start data taking time in microseconds */	// PxH from long to int, 17-Aug-07
  uint32_t tv_usec;
  \
  /** index number */				// PxH from long to int, 17-Aug-07
  uint32_t index;

  /** temperature voltage */
  double temp;

  // following lines added for Version 6

  /** the true starting line, indicates window mode if > 0 */
  uint16_t the_start;

  /** the true number of lines in following frame */
  uint16_t the_height;

  /** Frame ID from an external trigger, e.g. the bunch ID at DESY/FLASH */
  uint32_t external_id;

  /** LCLS bunch ID */
  uint64_t bunch_id;

  /** fill up space to get 64 bytes */
  char fill[24];
};

/** reading the Header parts from the QDataStream
 *
 * @tparam T the type that should be read from the stream
 * @return reference to the stream
 * @param stream the stream to read from
 * @param evt the header to read to
 *
 * @author Lutz Foucar
 */
template<typename T>
QDataStream &operator>>(QDataStream& stream, T& evt)
{
  if(stream.readRawData(reinterpret_cast<char*>(&evt),sizeof(T)) != sizeof(T))
    throw std::runtime_error("operator>>(QDdataStream&,T&): could not retrieve the right size");
  return stream;
}

/** reading the Header parts from the filestream
 *
 * @tparam T the type that should be read from the stream
 * @return reference to the stream
 * @param stream the stream to read from
 * @param evt the header to read to
 *
 * @author Lutz Foucar
 */
template<typename T>
std::ifstream &operator>>(std::ifstream& stream, T& evt)
{
  stream.read(reinterpret_cast<char*>(&evt),sizeof(T));
  if(stream.rdstate() != std::ios_base::goodbit)
    throw std::runtime_error("operator>>(ifstream&,T&): could not retrieve the right size");
  return stream;
}

} //end namespace frms6File
#pragma pack()
} //end namespace cass
