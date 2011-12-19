// Copyright (C) 2011 Lutz Foucar

/**
 * @file raw_sss_file_header.h contains the layout of the lma file headers.
 *
 * @author Lutz Foucar
 */

#include <stdint.h>
#include <vector>

namespace cass
{
namespace sssFile
{
/** a raw sss frame */
typedef std::vector<uint8_t> image_t;

#pragma pack(4)
/** the raw sss header
 *
 * @author Lutz Foucar
 */
struct Header
{
  /** width of the frame */
  uint32_t width;

  /** the height of the frame */
  uint32_t height;

  /** number of frames contained in file */
  uint32_t nFrames;
};
#pragma pack()
} //end namespace lmafile
} //end namespace cass
