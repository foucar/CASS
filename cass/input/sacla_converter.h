// Copyright (C) 2014, 2015 Lutz Foucar

/**
 * @file sacla_converter.h contains class to convert sacla data to cassevent
 *
 * @author Lutz Foucar
 */

#ifndef _SACLACONVERTER_
#define _SACLACONVERTER_

#include <DataAccessUserAPI.h>

#include "cass.h"
#include "pixeldetector.hpp"

namespace cass
{
class CASSEvent;

/** Class for reading SACLA data.
 *
 * This class will interface the API provided by SACLA to retrieve the available
 * data. One can use it to retrieve data from the Database, Octal Detector and
 * other detectors.
 *
 * The additional data of the MPCCD, such as the position, angle, gain,
 * width and height will be written in to the BeamlineData and can be
 * retrieved for each tile using the tilename:
 * - "%tilename%_Width": The number of pixels in X
 * - "%tilename%_Height": The number of pixels in Y
 * - "%tilename%_PosX_um": The x position of the tile in um
 * - "%tilename%_PosY_um": The y position of the tile in um
 * - "%tilename%_PosZ_um": The z position of the tile in um
 * - "%tilename%_Angle_deg": The angle of the tile in deg
 * - "%tilename%_PixSize_um": The pixel size in um
 * - "%tilename%_AbsGain": The absolute gain of the tile
 *
 * @cassttng SACLAConverter/{RetrieveAcceleratorData}\n
 *           Flag that tells whether the accelerator data should be retrieved.
 *           Default is true. When set to true the following data is available
 *           in the Machine Data:
 *           - "Acc_electronEnergy_eV"
 *           - "Acc_KParams"
 *           - "Acc_PhotonEnergy"
 * @cassttng SACLAConverter/OctalPixelDetectors/{size}\n
 *           The number of octal MPCCD detectors one wants to retrieve. Be sure
 *           that for each detector there is a unique id in the list below.
 * @cassttng SACLAConverter/OctalPixelDetectors/\%index\%/{DetectorName}\n
 *           The base name of the octal MPCCD. Unlike the API this name will be
 *           used to determine the names of the individual tiles of the MPCCD.
 *           Default is "Invalid" which will caus to skip this index.
 * @cassttng SACLAConverter/OctalPixelDetectors/\%index\%/{CASSID}\n
 *           The id of the detector that CASS will use internally. Default is "0".
 * @cassttng SACLAConverter/OctalPixelDetectors/\%index\%/{NormalizeToAbsGain}\n
 *           Using this option one can control whether the pixel values of the
 *           individual tiles will be normalized to the gain value of the first
 *           tile. When true, the pixel values of tiles 2 to 8 will modified
 *           using:
 *           \f$ pixval_{tile} = \frac{gain_{tile}}{gain_{tile1}}*pixval_{tile}\f$
 *           Default is true.
 * @cassttng SACLAConverter/DatabaseValues/{size}\n
 *           The number of values one wants to retrieve from the database. Be sure
 *           that for each detector there is a unique id in the list below.
 * @cassttng SACLAConverter/DatabaseValues/\%index\%/{ValueName}\n
 *           The name of the database value to retrieve. Default is "Invalid"
 *           which will cause to skip that index.
 * @cassttng SACLAConverter/DatabaseValues/\%index\%/{CASSName}\n
 *           The name of the value that the database value should have within
 *           the cassevent. Default is the same name as ValueName
 *
 * @author Lutz Foucar
 */
class SACLAConverter
{
public:
  /** constructor */
  SACLAConverter();

  /** read data from SaclaDataAccessUserAPI
   *
   * @return size of the filled data in bytes
   * @param runNbr the run number
   * @param blNbr the beamline number
   * @param highTagNbr first part of the tag
   * @param tagNbr the acutal Tag
   * @param event the CASSEvent where the data will be put into
   */
  uint64_t operator()(const int runNbr, const int blNbr, const int highTagNbr,
                      const int tagNbr, CASSEvent& event);

  /** load the settings of the reader */
  void loadSettings();

  /** retrieve requested beamline parameters in one go
   *
   * this is a hack to improve speed, as requesting parameters for each tag
   * individually seems to be a bottleneck
   *
   * @param first iterator to the first tag in the list
   * @param last iterator to one beyond the last tag in the list
   * @param blNbr the beamline number for the experiment
   * @param runNbr the run number for the experiment
   * @param highTagNbr the high tagnumber for the tag list
   */
  void cacheParameters(std::vector<int>::const_iterator first,
                       std::vector<int>::const_iterator last,
                       int blNbr, int runNbr, int highTagNbr);

private:
  /** container for the cached machine value */
  struct MachineValue
  {
    /** the name of the machine value within the database */
    std::string databaseName;

    /** the name of the machine value within the cassevent */
    std::string cassName;

    /** define map with tags as key and value and mapped type */
    typedef std::map<int, double> values_t;

    /** map that contains the tag as key and value for the value
     *  associated with the tag
     */
    values_t values;
  };

  /** define the machine values */
  typedef std::vector<MachineValue> machineVals_t;

  /** the list of requested machine values */
  machineVals_t _machineVals;

public:
  /** non changeing parameters of a pixel detector tile */
  struct detTileParams
  {
    /** constructor
     *
     * set the default parameters of the members
     */
    detTileParams()
      : name(""),
        xsize(0),
        ysize(0),
        datasize_bytes(0),
        pixsizex_um(0),
        pixsizey_um(0),
        posx_um(0),
        posy_um(0),
        posz_um(0),
        angle_deg(0),
        gain(0),
        bytes_retrieved(0),
        normalize(false),
        relativeGain(1),
        sreader(NULL),
        readBuf(NULL)
    {}

    /** destruct the tile parameter */
    ~detTileParams();

    /** init the tile reader
     *
     * @return status of the initialization
     * @param runNbr the runnumber to intialize the stream reader with
     * @param blNbr the beamline number to initialize the stream reader with
     */
    bool init(int runNbr, int blNbr);

    /** cache the the cachable data */

    /** the name of the tile */
    std::string name;

    /** the number of columns of the tile */
    uint32_t xsize;

    /** the number of rows of the tile */
    uint32_t ysize;

    /** the size of the frame in bytes */
    int datasize_bytes;

    /** the x size of a pixel in um */
    float pixsizex_um;

    /** the y size of a pixel in um */
    float pixsizey_um;

    /** the position in x in lab space of the tile in um */
    float posx_um;

    /** the position in y in lab space of the tile in um */
    float posy_um;

    /** the position in z in lab space of the tile in um */
    float posz_um;

    /** the angle in degree in lab space of the tile */
    float angle_deg;

    /** the absolute gain of the tile */
    float gain;

    /** the number of bytes retrieved for this tile */
    size_t bytes_retrieved;

    /** iterator to the start within the frame of this tile */
    pixeldetector::Detector::frame_t::iterator start;

    /** flag to tell whether the data of this tile should be normalized to
     *  another tile
     */
    bool normalize;

    /** the realtive gain with respect to the tile that this tile should be
     *  normalized to
     */
    float relativeGain;

    /** a astream reader object for the tile */
    char * sreader;

    /** a read buffer object for the tile */
    char * readBuf;
  };

  /** detector consists of tiles */
  struct detParams
  {
    /** vector containing the tiles of this detector */
    std::vector<detTileParams> tiles;

    /** flag to tell whether the individual tiles of this detector should be
     *  normalized to the first tile
     */
    bool normalize;

    /** the id of the detector within the CASSEvent */
    int CASSID;

    /** flag to tell whether the non-changeing data of this tile has been loaded */
    bool notLoaded;
  };

  /** define the pixel detectors container type */
  typedef std::vector<detParams> pixDets_t;

private:
  /** the list of requested machine values */
  pixDets_t _pixelDetectors;

  /** the list of requested octal detectors */
  pixDets_t _octalDetectors;

  /** flag to tell whether to retrieve the accelerator data */
  bool _retrieveAcceleratorData;
};
}//end namespace cass
#endif
