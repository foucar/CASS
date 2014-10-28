// Copyright (C) 2014 Lutz Foucar

/**
 * @file sacla_converter.h contains class to convert sacla data to cassevent
 *
 * @author Lutz Foucar
 */

#ifndef _SACLACONVERTER_
#define _SACLACONVERTER_

#include "cass.h"

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
 * - "%tilename%_DetectorRecordingFrequency": The detector recording frequency
 * - "%tilename%_DetectorFrequency": The detector frequency
 *
 * For each requested Database Parameter the following additional information is
 * available in the machine data:
 * - "%ValueName%_SyncDataFrequency": The recorded frequency
 *
 * @cassttng SACLAConverter/{RetrieveAcceleratorData}\n
 *           Flag that tells whether the accelerator data should be retrieved.
 *           Default is true. When set to true the following data is available
 *           in the Machine Data:
 *           - "Acc_electronEnergy_eV"
 *           - "Acc_KParams"
 *           - "Acc_PhotonEnergy"
 *           - "Acc_SACLAFrequency"
 *           - "Acc_MasterFrequency"
 * @cassttng SACLAConverter/OctalPixelDetectors/{size}\n
 *           The number of octal MPCCD detectors one wants to retrieve. Be sure
 *           that for each detector there is a unique id in the list below.
 * @cassttng SACLAConverter/OctalPixelDetectors/\%index\%/{DetectorName}\n
 *           The base name of the octal MPCCD. Unlike the API this name will be
 *           used to determine the names of the individual tiles of the MPCCD.
 *           Default is "Invalid" which will caus to skip this index.
 * @cassttng SACLAConverter/OctalPixelDetectors/\%index\%/{CASSID}\n
 *           The id of the detector that CASS will use internally.
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
 *
 * @author Lutz Foucar
 */
class SACLAConverter
{
public:
  /** constructor */
  SACLAConverter();

  /** read the frms6 file contents put them into cassevent
   *
   * @return size of the filled data in bytes
   * @param blNbr the beamline number
   * @param highTagNumber first part of the tag
   * @param tagNbr the acutal Tag
   * @param event the CASSEvent where the data will be put into
   */
  uint64_t operator()(const int blNbr, const int highTagNbr,
                      const int tagNbr, CASSEvent& event);

  /** load the settings of the reader */
  void loadSettings();

private:
  /** the list of requested machine values */
  std::vector<std::string> _machineVals;

  /** define the octal detectors container type */
  typedef std::map<int32_t,std::pair<std::string,bool> > octalDets_t;

  /** the list of requested octal detectors */
  octalDets_t _octalDetectors;

  /** flag to tell whether to retrieve the accelerator data */
  bool _retrieveAcceleratorData;
};
}//end namespace cass
#endif
