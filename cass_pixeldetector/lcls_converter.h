// Copyright (C) 2009, 2010, 2011 Lutz Foucar

/**
 * @file lcls_converter.h contains the converters to convert ccd and pnccd data
 *                          to CASSEvent
 *
 * @author Lutz Foucar
 */

#ifndef _LCLSCONVERTER_H_
#define _LCLSCONVERTER_H_

#include <map>
#include <vector>
#include <tr1/memory>
#include <utility>

#include <QtCore/QMutex>

#include "conversion_backend.h"


namespace cass
{
class CASSEvent;

namespace pixeldetector
{
namespace lclsid
{
class Key;
}

/** Converter for pnCCD, CsPad and commercial CCD Data.
 *
 * @note that for pnCCD and CsPad data one must assing also the corresponding
 *       config, as the config is needed to be able to parse the data part.
 *
 * @cassttng Converter/LCLSPixelDetectors/Detector/{size} \n
 *           Number of user defined detectors to be pulled out of the xtc
 * @cassttng Converter/LCLSPixelDetectors/Detector/\%id\%/{TypeName} \n
 *           The type of the detector. Only the following types are supported:
 *          - Id_pnCCDconfig : config for pnCCD data
 *          - Id_pnCCDframe : the pnCCD data
 *          - Id_Frame : Opal, TM6740 images
 *          - Id_CspadConfig: config for CsPad data
 *          - Id_CspadElement: the CsPad data
 *          - Id_Cspad2x2Element: the CsPad2x2 data
 * @cassttng Converter/LCLSPixelDetectors/Detector/\%id\%/{DetectorName} \n
 *           Name of the detector. Default is invalid
 * @cassttng Converter/LCLSPixelDetectors/Detector/\%id\%/{DetectorID} \n
 *           the id of the detector. Default is 0.
 * @cassttng Converter/LCLSPixelDetectors/Detector/\%id\%/{DeviceName} \n
 *           Name of the detector device
 * @cassttng Converter/LCLSPixelDetectors/Detector/\%id\%/{DeviceID} \n
 *           Id of the detector device
 * @cassttng Converter/LCLSPixelDetectors/Detector/\%id\%/{CASSID} \n
 *           the Id the detector should get in the CASSEvent. One needs this
 *           number for further processing. Note that the config and the data
 *           part must have the same CASSID.
 *
 * see Converter::operator() for details about the functionality
 *
 * @author Lutz Foucar
 * @author Stephan Kassemeyer
 */
class Converter : public cass::ConversionBackend
{
public:
  /** create singleton if doesnt exist already */
  static ConversionBackend::shared_pointer instance();

  /** operator to convert the LCLS Data to CASSEvent
   *
   * retrieve the corresponding CASS key from the lcls xtc that are contained in
   * the TypeId and Src (DetInfo) parts of the Xtc. Use the_LCLSToCASSId to do
   * this.
   *
   * in case that xtc is a Id_pnCCDconfig:\n
   * extract the version and create a config object toghether with this. Then
   * copy the information from the xtc config to the config object. Then store
   * the config object in the _pnccdConfigStore map.
   *
   * in case that xtc is a Id_pnCCDframe:\n
   * the conversion will only be performed if there is an config present for
   * the data. The config of version 1 does not contain information about the
   * frame size, therefore one needs to set it manually. In version 2 these
   * values are present, but sometimes there is a problem and one gets wrong
   * data. To prevent this, a check for consistency is implemented that will
   * output the wrong values and set them to the default values.\n
   * The frame data of the pnccd is subdived into 4 segments in the lcls data
   * format and one is provided with pointers to the beginning of the segments.
   * The data of the segments is put into a linearised array.
   * The frame looks as follows in the lab (when viwing into the beam)
 @verbatim

   -----------
|  | 2  | 3  | ^
|  | D  | B  | |
|  -----O----- |
|  | 1  | 0  | |
v  | C  | A  | |
   -----------

 @endverbatim
   * Here the numbers indicate the segment number in the lcls data.
   * The data alignmet of the array is indicated with the arrows. This means
   * that the slow increasing axis for segments 0 and 3 is going to the left and
   * for segments 1 and 2 to the right. In order to read the segments fast, it
   * is decided that one should read the segments data of segment 0 and 3 first
   * and then 1 and 2, with the fast increasing axis being upward. This effectively
   * rotates the frame in the cass representation 90 degress clockwise with
   * respect to the lab. The resulting segments in CASS are therefore given in
   * letters.\n
   * To copy one now has to copy the first row of segment 0 (A in CASS coordinates)
   * then copy the the first row of segment 3 (B in CASS coordinates). Then one
   * copies 2nd row of segment 0 and 2nd row of segment 3. This keeps going
   * until one has copied all rows of segment 0 and 3. Then one needs to copy
   * the data from segment 1 reversely, meaning that one copies the last row in
   * reverse direction. After that one does the same with segment 2. Then one
   * reversly copies the 2nd to last row of segment 1 and the 2nd to last row of
   * segment 2. This is done until all the data has been copied.\n
   * This gives to following assignment from lcls segments to CASS coordinates:
   * LCLS segment -> CASS Tiles \n
   * 0 -> A \n
   * 3 -> B \n
   * 1 (reverse) -> C \n
   * 2 (reverse) -> D \n
   * If one wants to see the fram as it is oriented in the lab one has to rotate
   * the data by 90 degrees counterclockwise. There is a processor that can
   * do this.\n
   * While copying the data one has to ignore the upper two bits of the data.
   *
   * in case that xtc is a Id_Frame:\n
   * the frame data of the lcls is just copied to the detectors frame. The
   * alignment is already like it is within CASS as a linearised array of pixels
   * with x being the fast increasing axis. One has to substract the offset from
   * all pixels, which is done during the copying. The first 8 pixels of the
   * frame contain status information and are therefore set to the same value
   * that the ninth pixel has.
   *
   * in case that xtc is a CsPad:\n
   * the data is layed out in the xtc in quadrants. Each quadrant contains 8
   * segments, which results in a total sum of 32 segments. The 8 segments of
   * each quadrant are stored in a linearized array where the fast changing axis
   * is along the x-axis. Each segment consits of 2*194 pixels along the x axis and
   * 185 pixels along the y axis. The data is copied into a linearized array where
   * the segments of each quadrant are aligned on top of each other like follows:
@verbatim
                 \
  +-------------+ |
  |     31      | |
  +-------------+ |
  |     30      | } quadrant 3
  +-------------+ |
  |     29      | |
  +-------------+ |
        .         .
        .         .
        .         .
  +-------------+ |
  |     02      | |
  +-------------+ |
^ |     01      | } quadrant 0
| +-------------+ |
y |     00      | |
| +-------------+ |
+---x--->        /
@endverbatim
   *
   * in case that xtc is a CsPad2x2:\n
   * the data from a CsPad2x2 is almost the same as from a regular CsPad, but
   * much easier. The data consists of one block with a Header (same as CsPad)
   * followed by a sequence of pairs. The first of the pair belongs to the first
   * segement and the second to the second segment. In CASS they will be aligned
   * like to segements of the cspad on top of each other:
   *
@verbatim
  +-------------+
^ |   second    |
| +-------------+
y |    first    |
| +-------------+
+---x--->
@endverbatim
   *
   * @param xtc the part of the datagram that this converter is responsible for
   * @param evt The CASSEvent that should store the information from the xtc.
   */
  void operator()(const Pds::Xtc* xtc, cass::CASSEvent* evt);

private:
  /** map that will map the LCLS key to the CASS key */
  typedef std::map<lclsid::Key, int32_t>  idmap_t;

  /** pair the version of the config with a shared pointer of the config */
  typedef std::pair<uint32_t, std::vector<uint8_t> > config_t;

  /** map containing the detector id together with its configuration */
  typedef std::map<int32_t, config_t> configStore_t;

  /** constructor
   *
   * set up the pds type ids that it is responsible for and creates the map
   * that maps the lcls id to the cass key.
   */
  Converter();

  /** prevent copy construction */
  Converter(const Converter&);

  /** prevent assignment */
  Converter& operator=(const Converter&);

  /** the singleton container */
  static ConversionBackend::shared_pointer _instance;

  /** singleton locker for mutithreaded requests */
  static QMutex _mutex;

  /** map lcls id to cass id */
  idmap_t _LCLSToCASSId;

  /** store for the configurations.
   *
   * Will store the version and the configuration itself in a pair
   * and has the casskey as key.
   */
  configStore_t _configStore;
};
}//end namespace vmi
}//end namespace cass

#endif
