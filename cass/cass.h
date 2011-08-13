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

/** multiply number by itself
 *
 * @tparam T type of value to be squared
 * @param val value to be squared
 *
 * @author Jochen Kuepper
 */
template<typename T>
inline T square(const T& val) { return val * val; };


namespace cass
{
  /** forwared declaration of a pointer that points to a specific location in a file */
  class filepointer{};
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
  typedef std::map<std::string,filepointer> filetypes_t;
  /** the list of events contained  in a file */
  typedef std::map<uint64_t, filetypes_t> eventlist_t;
}




#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
