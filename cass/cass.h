// Copyright (C) 2009, 2010 Lutz Foucar
// Copyright (C) 2010 Jochen Küpper

#ifndef CASS_GLOBAL_H
#define CASS_GLOBAL_H

#include <cassert>
#include <iterator>
#include <list>
#include <vector>
#include <QtCore/qglobal.h>


#if defined(CASS_LIBRARY)
#  define CASSSHARED_EXPORT Q_DECL_EXPORT
#else
#  define CASSSHARED_EXPORT Q_DECL_IMPORT
#endif

// VERBOSEOUT macro definition
#ifdef VERBOSE
#include <iostream>
#define VERBOSEOUT(a) (a)
#else
#define VERBOSEOUT(a) {}
#endif


namespace cass
{
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

  /** Qt names of known/supported Qt image formats
   *
   * @param fmt the Image Format
   * @return Qt name of format
   */
  inline const std::string imageformatName(ImageFormat fmt)
  {
      std::string fmtname;
      switch(fmt) {
      case PNG:  fmtname = std::string("PNG"); break;
      case TIFF: fmtname = std::string("TIFF"); break;
      case JPEG: fmtname = std::string("JPEG"); break;
      case GIF:  fmtname = std::string("GIF"); break;
      case BMP:  fmtname = std::string("BMP"); break;
      }
      return fmtname;
  };

  /** MIMI names of known/supported Qt image formats
   *
   * @param ImageFormat
   * @return MIME/type of format
   */
  inline const std::string imageformatMIMEtype(ImageFormat fmt)
  {
      std::string fmtname;
      switch(fmt) {
      case PNG:  fmtname = std::string("image/png"); break;
      case TIFF: fmtname = std::string("image/tiff"); break;
      case JPEG: fmtname = std::string("image/jpeg"); break;
      case GIF:  fmtname = std::string("image/gif"); break;
      case BMP:  fmtname = std::string("image/bmp"); break;
      }
      return fmtname;
  };


  /** Names of known/supported Qt image formats
   *
   * @param image format name
   * @return ImageFormat
   */
inline ImageFormat imageformat(const std::string& name)
  {
      ImageFormat fmt(PNG);
      if(std::string("PNG") == name) fmt = PNG;
      else if(std::string("TIFF") == name) fmt = TIFF;
      else if(std::string("JPEG") == name) fmt = TIFF;
      else if(std::string("GIF") == name) fmt = TIFF;
      else if(std::string("BMP") == name) fmt = TIFF;
      return fmt;
  };


  /*! Helper function to delete duplicates from a std::list

    This keeps the earliest entry in the list and removes all later ones
    @param l List to remove duplicates from.
    */
  template<typename T>
  inline void unique(std::list<T>& l)
  {
    // shorten list by removing consecutive duplicates
    l.unique();
    // now remove remaining (harder) duplicates
    for(typename std::list<T>::iterator i1 = l.begin();
    i1 != l.end();
    ++i1) {
      typename std::list<T>::iterator i2(i1);
      ++i2;
      while(l.end() != (i2 = find(i2, l.end(), *i1)))
        l.erase(i2);
    }
  }
}




#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
