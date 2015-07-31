// Copyright (C) 2014 Lutz Foucar

/**
 * @file geom_parser.cpp class to parse and retrieve info from geom files.
 *
 * @author Lutz Foucar
 */

#include <map>
#include <fstream>
#include <stdexcept>
#include <cstdlib>
#include <algorithm>
#include <tr1/functional>

#include "geom_parser.h"
#include "cass.h"

using namespace cass;
using namespace GeometryInfo;
using namespace std;
using tr1::bind;
using tr1::placeholders::_1;
using tr1::placeholders::_2;


struct asicInfo_t
{
  long int min_fs;
  long int min_ss;
  long int max_fs;
  long int max_ss;
  string badrow_direction;
  double res;
  string clen;
  double corner_x;
  double corner_y;
  long int no_index;
  double x_fs;
  double x_ss;
  double y_fs;
  double y_ss;
};

GeometryInfo::pos_t GeometryInfo::minus(const GeometryInfo::pos_t& minuent, const GeometryInfo::pos_t &subtrahend )
{
  GeometryInfo::pos_t pos(minuent);
  pos.x -= subtrahend.x;
  pos.y -= subtrahend.y;
  return pos;
}

size_t GeometryInfo::linearizeComponents(const GeometryInfo::pos_t &pos, const size_t nCols)
{
  const size_t col(static_cast<size_t>(pos.x + 0.5));
  const size_t row(static_cast<size_t>(pos.y + 0.5));
  return (row*nCols + col);
}

GeometryInfo::conversion_t GeometryInfo::generateConversionMap(const string &filename, const size_t sizeOfSrc, const size_t nSrcCols, const bool convertFromCheetahToCASS)
{
  typedef map<string,asicInfo_t> asicInfoMap_t;
  asicInfoMap_t geomInfos;
  conversion_t src2lab(sizeOfSrc);

  /** open file @throw invalid_argument when it could not be opened */
  ifstream geomFile (filename.c_str());
  if (!geomFile.is_open())
    throw invalid_argument("GeometryInfo::generateLookupTable(): could not open file '" +
                           filename +"'");

  /** read the file line by line */
  string line;
  while(!geomFile.eof())
  {
    getline(geomFile, line);

    /** if there is no '/' in line skip line */
    if (line.find('/') == string::npos)
      continue;

    /** get asic string, value name and value (as string) from line */
    const string asic(line.substr(0,line.find('/')));
    const string valueNameAndValue(line.substr(line.find('/')+1));
    string valueName(valueNameAndValue.substr(0,valueNameAndValue.find('=')));
    string valueString(valueNameAndValue.substr(valueNameAndValue.find('=')+1));

    /** eliminate whitespace from value name */
    valueName.erase(remove(valueName.begin(), valueName.end(), ' '), valueName.end());

    /** depending on the value name retrieve the value as right type */
    char * pEnd;
    if (valueName == "min_fs")
      geomInfos[asic].min_fs = std::strtol(valueString.c_str(),&pEnd,10);
    else if (valueName == "min_ss")
      geomInfos[asic].min_ss = std::strtol(valueString.c_str(),&pEnd,10);
    else if (valueName == "max_fs")
      geomInfos[asic].max_fs = std::strtol(valueString.c_str(),&pEnd,10);
    else if (valueName == "max_ss")
      geomInfos[asic].max_ss = std::strtol(valueString.c_str(),&pEnd,10);
    else if (valueName == "badrow_direction")
    {
      valueString.erase(remove(valueString.begin(), valueString.end(), ' '), valueString.end());
      geomInfos[asic].badrow_direction = valueString;
    }
    else if (valueName == "res")
      geomInfos[asic].res = std::strtod(valueString.c_str(),&pEnd);
    else if (valueName == "clen")
    {
      valueString.erase(remove(valueString.begin(), valueString.end(), ' '), valueString.end());
      geomInfos[asic].clen = valueString;
    }
    else if (valueName == "corner_x")
      geomInfos[asic].corner_x = std::strtod(valueString.c_str(),&pEnd);
    else if (valueName == "corner_y")
      geomInfos[asic].corner_y = std::strtod(valueString.c_str(),&pEnd);
    else if (valueName == "no_index")
      geomInfos[asic].no_index = std::strtol(valueString.c_str(),&pEnd,10);
    else if (valueName == "fs")
    {
      /** if value is fs then parse the string containing the 2 numbers */
      pEnd = &valueString[0];
      for (int i(0); i < 2 ; ++i)
      {
        const double number = std::strtod(pEnd,&pEnd);
        if (pEnd[0] == 'x')
          geomInfos[asic].x_fs = number;
        else if (pEnd[0] == 'y')
          geomInfos[asic].y_fs = number;
        else
          throw runtime_error(string("GeometryInfo::generateConversionMap: Cannot assign '") +
                              pEnd[0] + "' to x or y, in line with contents'" +
                              line + "'. Parsed info so far: " +
                              "asic '" + asic + "', "
                              "value Name and Value '" + valueNameAndValue + "', "
                              "value Name '" + valueName + "', "
                              "value as String '" + valueString + "', "
                              "number extracted so far '" + toString(number) + "', "
                              "parsing iteration '" + toString(i) +  "'");
        ++pEnd;
      }
    }
    else if (valueName == "ss")
    {
      pEnd = &valueString[0];
      for (int i(0); i < 2 ; ++i)
      {
        const double number = std::strtod(pEnd,&pEnd);
        if (pEnd[0] == 'x')
          geomInfos[asic].x_ss = number;
        else if (pEnd[0] == 'y')
          geomInfos[asic].y_ss = number;
        else
          throw runtime_error(string("GeometryInfo::generateConversionMap: Cannot assign '") +
                              pEnd[0] + "' to x or y, in line with contents'" +
                              line + "'. Parsed info so far: " +
                              "asic '" + asic + "', "
                              "value Name and Value '" + valueNameAndValue + "', "
                              "value Name '" + valueName + "', "
                              "value as String '" + valueString + "', "
                              "number extracted so far '" + toString(number) + "', "
                              "parsing iteration '" + toString(i) +  "'");
        ++pEnd;
      }
    }
  }

  /** go through all defined asics */
  asicInfoMap_t::iterator it(geomInfos.begin());
  for (; it != geomInfos.end(); ++it)
  {
    asicInfo_t& ai(it->second);

    /** if requested transform the start and end positions from the cheetah
       *  layout to the raw cass layout
       */
    if (convertFromCheetahToCASS)
    {
      const int nx(194);
      const int ny(185);
      const int quad(ai.min_fs/(2*nx));
      const int asicRow(ai.min_ss/(1*ny));
      const int xbegin(ai.min_fs/(1*nx) % 2);
      const int ybegin(quad*2*4+asicRow);

      ai.min_fs = xbegin*nx;
      ai.max_fs = xbegin*nx + nx-1;

      ai.min_ss = ybegin*ny;
      ai.max_ss = ybegin*ny + ny-1;
    }


    /** go through all pixels of this asics module */
    const int rowAsicRange(ai.max_ss - ai.min_ss);
    const int colAsicRange(ai.max_fs - ai.min_fs);
    for (int rowInAsic = 0; rowInAsic <= rowAsicRange; ++rowInAsic)
    {
      for (int colInAsic = 0; colInAsic <= colAsicRange; ++colInAsic)
      {
        /** find the position in the lab frame (in pixel units) of the current
           *  position (colInAsic,rowInAsic) in the asic
           */
        double xInLab = ai.x_fs*colInAsic + ai.x_ss*rowInAsic + ai.corner_x;
        double yInLab = ai.y_fs*colInAsic + ai.y_ss*rowInAsic + ai.corner_y;

        /** determine where the current position in the asic is in the src image */
        int colInSrc = ai.min_fs+colInAsic;
        int rowInSrc = ai.min_ss+rowInAsic;

        /** find position in the linearized array */
        int idxInSrc = rowInSrc * nSrcCols + colInSrc;

        /** check if whats been given in the geomfile goes together with the src */
        if  (idxInSrc >= static_cast<int>(sizeOfSrc))
          throw out_of_range("generateConversionMap(): The generated index '" +
                             toString(idxInSrc) + "' is too big for the src with size '"+
                             toString(sizeOfSrc) + "' in position in asic '" +
                             it->first + "', row '" + toString(rowInAsic) +
                             ", col '" + toString(colInAsic) +
                             ", resulting position in lab would be x '" +
                             toString(xInLab) + ", y '" + toString(yInLab) +
                             "'. With position in source col '" +
                             toString(colInSrc) + "', row '" +
                             toString(rowInSrc) + "'");

        /** remember what x,y position in the lab does this position in the
           *  asic correspond to
           */
        src2lab[idxInSrc].x = xInLab;
        src2lab[idxInSrc].y = yInLab;
      }
    }
  }
  return src2lab;
}

GeometryInfo::lookupTable_t GeometryInfo::generateLookupTable(const std::string &filename,
                                                              const size_t sizeOfSrc,
                                                              const size_t nSrcCols,
                                                              const bool convertFromCheetahToCASS)
{
  lookupTable_t lookupTable;
  lookupTable.lut.resize(sizeOfSrc);
  GeometryInfo::conversion_t src2lab =
      generateConversionMap(filename, sizeOfSrc, nSrcCols, convertFromCheetahToCASS);

  /** get the minimum and maximum position in lab x and y */
  lookupTable.min.x = min_element(src2lab.begin(),src2lab.end(),
                                  bind(less<pos_t::x_t>(),
                                       bind<pos_t::x_t>(&pos_t::x,_1),
                                       bind<pos_t::x_t>(&pos_t::x,_2)))->x;
  lookupTable.min.y = min_element(src2lab.begin(),src2lab.end(),
                                  bind(less<pos_t::y_t>(),
                                       bind<pos_t::y_t>(&pos_t::y,_1),
                                       bind<pos_t::y_t>(&pos_t::y,_2)))->y;
  lookupTable.max.x = max_element(src2lab.begin(),src2lab.end(),
                                  bind(less<pos_t::x_t>(),
                                       bind<pos_t::x_t>(&pos_t::x,_1),
                                       bind<pos_t::x_t>(&pos_t::x,_2)))->x;
  lookupTable.max.y = max_element(src2lab.begin(),src2lab.end(),
                                  bind(less<pos_t::y_t>(),
                                       bind<pos_t::y_t>(&pos_t::y,_1),
                                       bind<pos_t::y_t>(&pos_t::y,_2)))->y;

  /** move all values, such that they start at 0
   *  \f$ pos.x -= min_x \f$
   *  \f$ pos.y -= min_y \f$
   */
  transform(src2lab.begin(),src2lab.end(),src2lab.begin(),
            bind(minus,_1,lookupTable.min));

  /** get the new maximum value of the shifted lab, which corresponds to the
   *  number of pixels that are required in the dest image, since all lab
   *  values are in pixel coordinates.
   */
  const double max_x = max_element(src2lab.begin(),src2lab.end(),
                                   bind(less<pos_t::x_t>(),
                                        bind<pos_t::x_t>(&pos_t::x,_1),
                                        bind<pos_t::x_t>(&pos_t::x,_2)))->x;
  const double max_y = max_element(src2lab.begin(),src2lab.end(),
                                   bind(less<pos_t::y_t>(),
                                        bind<pos_t::y_t>(&pos_t::y,_1),
                                        bind<pos_t::y_t>(&pos_t::y,_2)))->y;

  /** determine the dimensions of the destination image */
  lookupTable.nCols = static_cast<int>(max_x + 0.5)+1;
  lookupTable.nRows = static_cast<int>(max_y + 0.5)+1;

  /** convert the positions in the lab space (pixel units) to linearized indizes
   *  in the destination image
   *  \f$ _lookuptable = round(src2lab.x) + round(src2lab.y)*nDestCols \f$
   */
  transform(src2lab.begin(),src2lab.end(),lookupTable.lut.begin(),
            bind(&linearizeComponents,_1,lookupTable.nCols));

  /** check if the boundaries are ok, @throw out of range if not. */
  if(lookupTable.nCols*lookupTable.nRows <= *max_element(lookupTable.lut.begin(),lookupTable.lut.end()))
    throw out_of_range("generateLookupTable(): the maximum index in the lookup table '" +
                       toString(*max_element(lookupTable.lut.begin(),lookupTable.lut.end())) +
                       "' does not fit with the destination size of '" +
                       toString(lookupTable.nCols*lookupTable.nRows) + "'");

  return lookupTable;
}
