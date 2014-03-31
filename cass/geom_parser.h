// Copyright (C) 2014 Lutz Foucar

/**
 * @file geom_parser.h class to parse and retrieve info from geom files.
 *
 * @author Lutz Foucar
 */

#ifndef _GEOMPARSER_
#define _GEOMPARSER_

#include <string>
#include <vector>

namespace cass
{
namespace GeometryInfo
{
/** combine the position in the lab into a struct */
struct pos_t
{
  typedef double x_t;
  typedef double y_t;
  x_t x;
  y_t y;
};

/** define the conversion table type */
typedef std::vector<pos_t> conversion_t;

/** combine info needed for the lookuptable */
struct lookupTable_t
{
  std::vector<size_t> lut;
  pos_t min;
  pos_t max;
  size_t nCols;
  size_t nRows;
};

/** functor to substract one position from the other
 *
 * @return the result of the subtraction
 * @param minuent the minuent of the subtraction
 * @param subtrahend the subtrahend of the subtraction
 *
 * @author Lutz Foucar
 */
pos_t minus(const pos_t& minuent, const pos_t &subtrahend );

/** convert index with 2 components into a linearized index
 *
 * @return linearized index
 * @param pos the index in the frame with 2 components
 * @param nCols the number of columns (fast changing index) in the frame
 *
 * @author Lutz Foucar
 */
size_t linearizeComponents(const pos_t &pos, const size_t nCols);

/** parse the geom file and generate a lookup table
 *
 * in the lookup table each entry corresponsed to the coordinates in the lab
 *
 * @return list of index to pos in lab
 * @param filename The filename of the geomfile
 * @param sizeOfSrc the size of the source image
 * @param nSrcCols the number of columns (fast changing index) in the image
 * @param convertFromCheetahToCASS flag to tell whether the geom file uses
 *                                 layout in Cheetah but CASS uses CASS raw
 *                                 image coordinates
 *
 * @author Lutz Foucar
 */
conversion_t generateConversionMap(const std::string &filename,
                                   const size_t sizeOfSrc,
                                   const size_t nSrcCols,
                                   const bool convertFromCheetahToCASS);

/** generate a lookup table for a new image
 *
 * generate a lookup table that has the corresponding index in an image that
 * looks like an lab image
 *
 * @return the index lookup table
 * @param filename The filename of the geomfile
 * @param sizeOfSrc the size of the source image
 * @param nSrcCols the number of columns (fast changing index) in the image
 * @param convertFromCheetahToCASS flag to tell whether the geom file uses
 *                                 layout in Cheetah but CASS uses CASS raw
 *                                 image coordinates
 *
 * @author Lutz Foucar
 */
lookupTable_t generateLookupTable(const std::string &filename,
                                  const size_t sizeOfSrc,
                                  const size_t nSrcCols,
                                  const bool convertFromCheetahToCASS);
}//end namespace geometryInfo

}//end namespace cass

#endif
