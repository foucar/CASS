// Copyright (C) 2009, 2010 Lutz Foucar
// Copyright (C) 2010 Jochen Küpper

/**
 * @file cass.h file contains global definitions for project cass
 *
 * @author Lutz Foucar
 */

#ifndef CASS_GLOBAL_H
#define CASS_GLOBAL_H

#include <cassert>
#include <iterator>
#include <list>
#include <vector>
#include <map>
#include <fstream>
#include <tr1/memory>
#include <QtCore/qglobal.h>
#include <stdint.h>


#if defined(CASS_LIBRARY)
#  define CASSSHARED_EXPORT Q_DECL_EXPORT
#else
#  define CASSSHARED_EXPORT Q_DECL_IMPORT
#endif

// OUT macro definitions
#ifdef VERBOSE
#include <iostream>
#define VERBOSEOUT(a) (a)
#else
#define VERBOSEOUT(a) {}
#endif
#ifdef DEBUG
#include <iostream>
#define DEBUGOUT(a) (a)
#else
#define DEBUGOUT(a) {}
#endif



namespace cass
{
/** multiply number by itself
 *
 * @tparam T type of value to be squared
 * @param val value to be squared
 *
 * @author Jochen Kuepper
 */
template<typename T>
inline T square(const T& val) { return val * val; }

/** A resource that will point at a specific location within a file */
struct FilePointer
{
  /** defining a shared poitner to the stream */
  typedef std::tr1::shared_ptr<std::ifstream> filestream_t;

  /** the position with the file */
  int _pos;

  /** the stream to the file */
  filestream_t _filestream;

  /** return a stream to the right position within the file */
  std::ifstream& getStream()
  {
    _filestream->seekg(_pos);
    return *_filestream.get();
  }
};

namespace FileStreaming
{
/** retrieve a variable from a file stream
 *
 * @return the variable
 * @tparam the type of the variable to retrieve
 * @param file The file stream to retrieve the vairable from
 */
template <typename T>
T retrieve(std::ifstream &file)
{
  T var;
  file.read(reinterpret_cast<char*>(&var),sizeof(T));
  return var;
}

/** retrieve a variable from a file stream without extracting it
 *
 * leaves the file stream at the same position it was before
 *
 * @return the variable
 * @tparam the type of the variable to retrieve
 * @param file The file stream to retrieve the vairable from
 */
template <typename T>
T peek(std::ifstream &file)
{
  T var;
  std::streampos currentpos(file.tellg());
  file.read(reinterpret_cast<char*>(&var),sizeof(T));
  file.seekg(currentpos);
  return var;
}
}//end namespace FileStreaming

/** global variable to set the ring buffer size */
const size_t RingBufferSize=32;
/** global variable to set the number of worker threads */
const size_t NbrOfWorkers=16;
/** the maximum size of one datagram should be 10 MB */
const size_t DatagramBufferSize=0x1000000;
/** the type of a pixel of a ccd image*/
typedef float pixel_t;
//forward decalration//
class PixelDetector;
/** type of the container for ccd detectors */
typedef std::vector<PixelDetector> detectors_t;
/** known/supported Qt image formats */
enum ImageFormat {PNG=1, TIFF=2, JPEG=3, GIF=4, BMP=5};
/** map file extrension to the filepointer */
typedef std::map<std::string,FilePointer> filetypes_t;
/** the list of events contained  in a file */
typedef std::map<uint64_t, filetypes_t> eventmap_t;
}

#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
