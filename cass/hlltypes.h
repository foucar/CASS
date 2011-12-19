// Copyright (C) 2011 Lutz Foucar

/**
 * @file hlltypes.h contains the layout of the hll data types and conversion to
 *                  cass data types
 *
 * @author Lutz Foucar
 */

#include <stdint.h>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <iterator>

#include <QtCore/QDataStream>

namespace cass
{
namespace hllDataTypes
{
/** define a pixel */
typedef  int16_t pixel;

/** define a frame */
typedef std::vector<pixel> frame_t;

#pragma pack(4)
/** the file header
 *
 * took from fformat.h from pnCCD lib. Size should be 1024 bytes
 *
 * @author Peter Holl
 * @author Nils Kimmel
 */
struct Frms6FileHeader
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

/** the header that describe the frames
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

/** the file header structure of the hll darkcal file format
 *
 * derived from the code within the pnCCD lib
 *
 * @author Lutz Foucar
 */
struct DarkcalFileHeader
{
  /** string to identify that it is a hll darkcal file
   *
   * should contain "HE pixel statistics map"
   */
  char identifystring[24];

  /** the width of the frames */
  uint32_t columns;

  /** the height of the frames */
  uint32_t rows;

  /** the overal length of the frame, if the matrix is linearized */
  uint32_t length;

  /** empty to fill the header up to 1024 bytes */
  char fillspace[988];
};

/** struct describing the statistics saved in a HLL Darkcal file
 *
 * copied from the pnCCD lib
 *
 * @author Peter Holl
 * @author Nils Kimmel
 */
struct statistics
{
  /** internal use */
  double sum;

  /** offset mean value of pixel (raw) */
  double offset;

  /** noise sigma value of pixel */
  double sigma;

  /** internal use */
  double sumSq;

  /** internal use */
  int count;

  /** offset mean value of pixel (common mode corrected) */
  int16_t mean;
};
#pragma pack()

/** convert a linearised matrix in the CASS format to the hll format
 *
 * the difference between the CASS and HLL is, that the CASS format has all 4
 * quadrants in a quadrat wheras in HLL they are aligned in a rectangle.
 *
 @verbatim

   -----------
   | 1  | 2  |
   | C  | D  |
   -----O-----
   | 0  | 3  |
   | A  | B  |
   -----------
   --------->

       |
       v

   ----()--------()-----
   | 0  | 1  | 2  | 3  |
   | A  | C  | D  | B  |
   ---------------------
        --------->

 @endverbatim
 * The numbers indicate the tile of the frame wihtin the HLL frame format, the
 * letters indicate the tiles within the CASS format. The arrows indicate the
 * fast increasing coordinate within the linearised array. The parenthesis and
 * the 0 indicate where the hole btw the quater holes are in the new tiles.
 * This implies that the top two tiles in the CASS format have to be rotated by
 * 180 degrees before adding them to HLL format array. This basically mean that
 * one has to read these tiles reverseley. This is done in this function by
 * using reverse iterators that will point at the last element of the tile.
 *
 * This function will then read the first row of tile A then the last of tile C
 * then the last row of tile C, then the first row Tile B to the first row of
 * the HLL format.
 * The second row of the HLL format is then build by the 2nd row of Tile A, the
 * 2nd to last row of Tile C then the 2nd to last row of tile D and the 2nd row
 * of tile B. This continues until one has read all the rows of the tiles.
 *
 * @tparam inputContainerType the type of the container containing the input
 * @tparam outputContainerType the type of the container containing the output
 * @param CASSMatrix the container containing the linearised input matrix
 * @param HLLMatrix the container containing the linearised out matrix
 * @param quadrantColumns the number of columns in one quadrant
 * @param quadrantRows the number of rows in one quadrant
 * @param CASSColumns the number of columns in the CASS input container
 *
 * @author Lutz Foucar
 */
template <typename inputContainerType, typename outputContainerType>
void CASS2HLL(const inputContainerType& CASSMatrix,
              outputContainerType& HLLMatrix,
              size_t quadrantColumns,
              size_t quadrantRows,
              size_t CASSColumns)
{
  typename inputContainerType::const_iterator cassquadrant0(CASSMatrix.begin());
  typename inputContainerType::const_iterator cassquadrant1(CASSMatrix.begin()+quadrantColumns);
  typename inputContainerType::const_reverse_iterator cassquadrant2(CASSMatrix.rbegin()+quadrantColumns);
  typename inputContainerType::const_reverse_iterator cassquadrant3(CASSMatrix.rbegin());

  typename outputContainerType::iterator HLL(HLLMatrix.begin());

  for (size_t quadrantRow(0); quadrantRow < quadrantRows; ++quadrantRow)
  {
    copy(cassquadrant0,cassquadrant0+quadrantColumns,HLL);
    advance(HLL,quadrantColumns);
    copy(cassquadrant2,cassquadrant2+quadrantColumns,HLL);
    advance(HLL,quadrantColumns);
    copy(cassquadrant3,cassquadrant3+quadrantColumns,HLL);
    advance(HLL,quadrantColumns);
    copy(cassquadrant1,cassquadrant1+quadrantColumns,HLL);
    advance(HLL,quadrantColumns);

    advance(cassquadrant0,CASSColumns);
    advance(cassquadrant1,CASSColumns);
    advance(cassquadrant2,CASSColumns);
    advance(cassquadrant3,CASSColumns);
  }
}

/** convert a linearised matrix in the hll format to the CASS format
 *
 * the difference between the CASS and HLL is, that the CASS format has all 4
 * quadrants in a quadrat wheras in HLL they are aligned in a rectangle.
 @verbatim


   ----()--------()-----
   | 0  | 1  | 2  | 3  |
   | A  | C  | D  | B  |
   ---------------------
        --------->

       |
       v

   -----------
   | 1  | 2  |
   | C  | D  |
   -----O-----
   | 0  | 3  |
   | A  | B  |
   -----------
   --------->
 @endverbatim
 * The numbers indicate the tile of the frame wihtin the HLL frame format, the
 * letters indicate the tiles within the CASS format. The arrows indicate the
 * fast increasing coordinate within the linearised array. The parenthesis and
 * the 0 indicate where the hole btw the quater holes are in the new tiles.
 * This implies that the tiles 1 and 2  in the HLL format have to be rotated by
 * 180 degrees before adding them to CASS format array. This basically mean that
 * one has to read these tiles reverseley. This is done in this function by
 * using reverse iterators that will point at the last element of the tile.
 *
 * This function will then read the first row of tile 0 then the first rowv of
 * tile 3. Then the 2nd row of tile 0 and then the 2nd row of tile 3. This
 * continues until all rows of the tiles have been read.
 * It then continues with the last of tile 1 and the last row of tile 2. Then
 * the 2nd to last row of tile 1 and the 2nd to last row of tile 2. This
 * continues until all rows of the tiles have been read.
 *
 * @note The resulting image in the CASS format is rotated by 90 degrees clockwise
 *       to the image as it would look like inside the lab frame if one looks
 *       into the beam.
 *
 * @tparam inputContainerType the type of the container containing the input
 * @tparam outputContainerType the type of the container containing the output
 * @param HLLMatrix the container containing the linearised input matrix
 * @param CASSMatrix the container containing the linearised out matrix
 * @param quadrantColumns the number of columns in one quadrant
 * @param quadrantRows the number of rows in one quadrant
 * @param HLLColumns the number of columns in the HLL input container
 *
 * @author Lutz Foucar
 */
template <typename inputContainerType, typename outputContainerType>
void HLL2CASS(const inputContainerType& HLLMatrix,
              outputContainerType& CASSMatrix,
              size_t quadrantColumns,
              size_t quadrantRows,
              size_t HLLColumns)
{
  typename inputContainerType::const_iterator hllquadrant0(HLLMatrix.begin());
  typename inputContainerType::const_reverse_iterator hllquadrant1(HLLMatrix.rbegin()+2*quadrantColumns);
  typename inputContainerType::const_reverse_iterator hllquadrant2(HLLMatrix.rbegin()+1*quadrantColumns);
  typename inputContainerType::const_iterator hllquadrant3(HLLMatrix.begin()+3*quadrantColumns);

  typename outputContainerType::iterator cass(CASSMatrix.begin());

  //copy quadrant read to right side (lower in CASS)
  for (size_t quadrantRow(0); quadrantRow < quadrantRows; ++quadrantRow)
  {
    copy(hllquadrant0,hllquadrant0+quadrantColumns,cass);
    advance(cass,quadrantColumns);
    copy(hllquadrant3,hllquadrant3+quadrantColumns,cass);
    advance(cass,quadrantColumns);

    advance(hllquadrant0,HLLColumns);
    advance(hllquadrant3,HLLColumns);
  }
  //copy quadrants read to left side (upper in CASS)
  for (size_t quadrantRow(0); quadrantRow < quadrantRows; ++quadrantRow)
  {
    copy(hllquadrant1,hllquadrant1+quadrantColumns,cass);
    advance(cass,quadrantColumns);
    copy(hllquadrant2,hllquadrant2+quadrantColumns,cass);
    advance(cass,quadrantColumns);

    advance(hllquadrant1,HLLColumns);
    advance(hllquadrant2,HLLColumns);
  }
}
} //end namespace hlltypes
} //end namespace cass
