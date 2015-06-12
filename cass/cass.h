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
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <tr1/memory>
#include <QtCore/qglobal.h>
#include <QtCore/QDataStream>
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
/** convert any type to a string
 *
 * should be used for converting numbers to strings. This function was inspired
 * by a function found at Sep, 24th 2011 here:
 * http://notfaq.wordpress.com/2006/08/30/c-convert-int-to-string/
 *
 * @tparam Type the type of the number
 * @param t the number to convert to string
 *
 * @author Lutz Foucar
 */
template <typename Type>
std::string toString (const Type& t)
{
  std::stringstream ss;
  ss << t;
  return ss.str();
}

/** multiply number by itself
 *
 * @tparam T type of value to be squared
 * @param val value to be squared
 *
 * @author Jochen Kuepper
 */
template<typename T>
inline T square(const T& val) { return val * val; }

/** A resource that will point at a specific location within a file
 *
 * @author Lutz Foucar
 */
struct FilePointer
{
  /** defining a shared pointer to the stream */
  typedef std::tr1::shared_ptr<std::ifstream> filestream_t;

  /** the position with the file */
  std::streampos _pos;

  /** the stream to the file */
  filestream_t _filestream;

  /** @return a stream to the right position within the file */
  std::ifstream& getStream()
  {
    _filestream->seekg(_pos);
    return *_filestream.get();
  }
};

/** tokenize to return all lines of an ascii file in a vector
 *
 * will return a list containing all non empty lines of the file. Before
 * returning the list strip the 'new line' and 'line feed' from the line.
 * Also skip all lines that contain either a '#' or a ';'.
 *
 * @author Lutz Foucar
 */
struct Tokenizer
{
  /** the operator
   *
   * @return vector of string containing all non empty lines of the file
   * @param file the filestream to tokenize
   */
  std::vector<std::string> operator()(std::ifstream &file)
  {
    using namespace std;
    vector<string> lines;
    while (!file.eof())
    {
      string line;
      getline(file,line);
      /* remove newline */
      if(line[line.length()-1] == '\n')
      {
        line.resize(line.length()-1);
      }
      /* remove line feed */
      if(line[line.length()-1] == '\r')
      {
        line.resize(line.length()-1);
      }
      /* dont read newlines */
      if(line.empty() || line[0] == '\n')
      {
        continue;
      }
      /* don't read lines containing ';' or '#' */
      if(line.find(';') != string::npos || line.find('#') != string::npos)
      {
        continue;
      }
      lines.push_back(line);
    }
    return lines;
  }
};

/** split the line into the values in that line
 *
 * @author Lutz Foucar
 */
struct Splitter
{
  /** the operator for splitting a line of values
   *
   * @param line string containing the line that should be split
   * @param elems vector containing the elements of the line
   * @param delim the delimiter that the line should be splitted by.
   */
  void operator()(const std::string &line, std::vector<double> &elems, char delim)
  {
    using namespace std;
    stringstream ss(line);
    string str;
    while(getline(ss, str, delim))
    {
      if ((str.size() == 1 && !(isalpha(str[0]))) || str.empty())
        continue;
      stringstream ssvalue(str);
      double value;
      ssvalue >> value;
      elems.push_back(value);
    }
  }

  /** the operator for splitting into substrings
   *
   * @param line string containing the line that should be split
   * @param elems vector containing the elements of the line
   * @param delim the delimiter that the line should be splitted by.
   */
  void operator()(const std::string &line, std::vector<std::string> &elems, char delim)
  {
    using namespace std;
    stringstream ss(line);
    string str;
    while(getline(ss, str, delim))
    {
      if ((str.size() == 1 && !(isalpha(str[0]))) || str.empty())
        continue;
      elems.push_back(str);
    }
  }
};

namespace Streaming
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

/** reading a type from the QDataStream
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
    throw std::runtime_error("operator>>(QDdataStream&,T&): could not retrieve all requested bytes");
  return stream;
}

/** reading a type from the filestream
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
    throw std::runtime_error("operator>>(ifstream&,T&): could not retrieve all requested bytes");
  return stream;
}

}//end namespace FileStreaming

/** global variable to set the ring buffer size */
const size_t RingBufferSize=32;
/** global variable to set the number of worker threads */
const size_t NbrOfWorkers=16;
/** the maximum size of one datagram should be 16 MB */
const size_t DatagramBufferSize=2*0x1000000;
/** the type of a pixel of a ccd image*/
typedef float pixel_t;
//forward decalration//
class PixelDetector;
class FileReader;
/** type of the container for ccd detectors */
typedef std::vector<PixelDetector> detectors_t;
/** known/supported Qt image formats */
enum ImageFormat {PNG=1, TIFF=2, JPEG=3, GIF=4, BMP=5};
/** pair of a file pointer with the associated file reader */
typedef std::pair<std::tr1::shared_ptr<FileReader>, FilePointer> filereaderpointerpair_t;
/** map file name to the filepointer */
typedef std::vector<filereaderpointerpair_t> positionreaders_t;
/** the list of events contained  in a file with the associated position and reader*/
typedef std::map<uint64_t, positionreaders_t> event2positionreaders_t;
}

#endif
