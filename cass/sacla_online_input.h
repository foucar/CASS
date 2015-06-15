// Copyright (C) 2014 Lutz Foucar

/**
 * @file sacla_online_input.h contains input that uses tcp as interface
 *
 * @author Lutz Foucar
 */

#ifndef _SACLAONLINEINPUT_
#define _SACLAONLINEINPUT_

#include <string>

#include "cass.h"
#include "input_base.h"
#include "cass_event.h"
#include "ringbuffer.h"


namespace cass
{
/** Online Input to be used at SACLA
 *
 * Makes use of the "OnlineUserAPI" to retrieve the detector data online. All
 * data that is stored in the database is retrieved using the
 * SACLADataAccessUserAPI.
 *
 * @note At least one pixel detector is needed in order to be able to retrieve
 *       data using the SACLADataAccessUserAPI, since the runnumber is needed to
 *       retrieve the needed hightag number.
 *
 * @cassttng SACLAOnlineInput/{BeamlineNumber}\n
 *           The Beamline at which the experiment is running. Default is 3
 * @cassttng SACLAOnlineInput/OctalPixelDetectors/{size}\n
 *           The number of octal pixeldetectors that one wants to
 *           retrieve. Ensure that each parameter has a unique id in the list.
 * @cassttng SACLAOnlineInput/OctalPixelDetectors/\%index\%/{DetectorIDName}\n
 *           The base name of the octal MPCCD. This name will be used to
 *           determine the names of the individual tiles of the MPCCD.
 *           Default is "Invalid" which will caus to skip this index.
 * @cassttng SACLAOnlineInput/OctalPixelDetectors/\%index\%/{NbrOfTiles}\n
 *           This option gives the user control over how many tiles the detector
 *           has. Default is 8.
 * @cassttng SACLAOnlineInput/OctalPixelDetectors/\%index\%/{NormalizeToAbsGain}\n
 *           Using this option one can control whether the pixel values of the
 *           individual tiles will be normalized to the gain value of the first
 *           tile. When true, the pixel values of tiles 2 to 8 will modified
 *           using:
 *           \f$ pixval_{tile} = \frac{gain_{tile}}{gain_{tile1}}*pixval_{tile}\f$
 *           Default is true.
 * @cassttng SACLAOnlineInput/OctalPixelDetectors/\%index\%/{CASSID}\n
 *           The id of the detector that CASS will use internally. Default is "0".
 * @cassttng SACLAOnlineInput/DatabaseValues/{size}\n
 *           The number of values one wants to retrieve from the database. Be sure
 *           that for each detector there is a unique id in the list below.
 *           At least one pixel detector is needed in order to be able to
 *           retrieve data using the SACLADataAccessUserAPI, since the runnumber
 *           is needed to retrieve the needed hightag number.
 * @cassttng SACLAOnlineInput/DatabaseValues/\%index\%/{ValueName}\n
 *           The name of the database value to retrieve. Default is "Invalid"
 *           which will cause to skip that index.
 *
 * @author Lutz Foucar
 */
class SACLAOnlineInput : public InputBase
{
public:
  /** create an instance of this
   *
   * this initializes the _instance member of the base class. Check here if
   * it is already initialized, if so throw logic error.
   *
   * @param buffer the ringbuffer, that we take events out and fill it
   *        with the incomming information
   * @param ratemeter reference to the ratemeter to measure the rate of the input
   * @param loadmeter reference to the ratemeter to measure the load of the input
   * @param parent the parent of this object
   */
  static void instance(RingBuffer<CASSEvent>& buffer,
                       Ratemeter &ratemeter, Ratemeter &loadmeter,
                       QObject *parent=0);

  /** starts the thread
   *
   * Starts the thread and the loop that waits for data. When an timout occured
   * it will just restart the loop until the quit flag is set.
   */
  void run();

  /** do not load anything after it is started */
  void load() {}

private:
  /** constructor
   *
   * creates the thread. Connects to the tcp server and then retrieves the
   * data streams. The data within the stream will be deserialized with the
   * help of deserialization functions, where the user has to choose which
   * one is appropriate via the .ini file parameters. The thread runs as long
   * as noone calls the end() member of the base class.
   * In case a timeout occurs when waiting for a new event, it will just continue
   * and wait for the next timeout. In case that a timeout occurred when waiting
   * for the data of an event it throws an runtime error.
   *
   * @param buffer the ringbuffer, that we take events out and fill it
   *        with the incomming information
   * @param ratemeter reference to the ratemeter to measure the rate of the input
   * @param loadmeter reference to the ratemeter to measure the load of the input
   * @param parent the parent of this object
   */
  SACLAOnlineInput(RingBuffer<CASSEvent>& buffer,
                    Ratemeter &ratemeter, Ratemeter &loadmeter,
                    QObject *parent=0);

};

}//end namespace cass

#endif
