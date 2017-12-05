// Copyright (C) 2015 Lutz Foucar

/**
 * @file sacla_online_input.h contains input that uses the sacla online interface
 *
 * @author Lutz Foucar
 */

#ifndef _SACLAONLINEINPUT_
#define _SACLAONLINEINPUT_

#include <string>

#include "cass.h"
#include "input_base.h"


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
 * @cassttng SACLAOnlineInput/{MaxGuessedEvents}\n
 *           The number of consecutive events that should be guessed. Once this
 *           number of events has been reached. The latest tag number will
 *           be retrieved for the next event. Default is 60.
 * @cassttng SACLAOnlineInput/{BeamlineNumber}\n
 *           The Beamline at which the experiment is running. Default is 3
 * @cassttng SACLAOnlineInput/OctalPixelDetectors/{size}\n
 *           The number of octal pixeldetectors that one wants to
 *           retrieve. Ensure that each parameter has a unique id in the list.
 * @cassttng SACLAOnlineInput/OctalPixelDetectors/\%index\%/{CASSID}\n
 *           The id of the detector that CASS will use internally.
 *           Default is "-1" which will cause to skip this index.
 * @cassttng SACLAOnlineInput/OctalPixelDetectors/\%index\%/{NextTagNumberAdvancedBy}\n
 *           It is needed to guess the next tag number therefore one has to
 *           tell how much the tag number advanced from one shot to the next.
 *           The tag number is increased with 60 Hz. Therefore at 30 Hz this
 *           number should be 2. Default is 2.
 * @cassttng SACLAOnlineInput/OctalPixelDetectors/\%index\%/Tiles/{size}\n
 *           The number of tiles contained within the octal pixeldetectors
 *           Ensure that each parameter has a unique id in the list.
 * @cassttng SACLAOnlineInput/OctalPixelDetectors/\%index\%/Tiles/\%index\%/{TileName}\n
 *           Name of the tile in the SACLA DAQ. Default is "Invalid".
 * @cassttng SACLAOnlineInput/OctalPixelDetectors/\%index\%/Tiles/\%index\%{NbrCalibrationRows}\n
 *           Number of additional rows that are not part of the image but used for
 *           calibrating the image. Default is 6.
 * @cassttng SACLAOnlineInput/OctalPixelDetectors/\%index\%/Tiles/\%index\%{NormalizeTo}\n
 *           Using this option one can control whether the pixel values of this
 *           tile will be normalized to the gain value of which other
 *           tile. The pixel values of this tile will be modified using:
 *           \f$ pixval_{tile} = \frac{gain_{tile}}{gain_{NormalizeTo}}*pixval_{tile}\f$
 *           Default is 0 in which case the tile will not be normalized.
 * @cassttng SACLAOnlineInput/DatabaseValues/{size}\n
 *           The number of values one wants to retrieve from the database. Be sure
 *           that for each detector there is a unique id in the list below.
 *           At least one pixel detector is needed in order to be able to
 *           retrieve data using the SACLADataAccessUserAPI, since the runnumber
 *           is needed to retrieve the needed hightag number.
 * @cassttng SACLAOnlineInput/DatabaseValues/\%index\%/{ValueName}\n
 *           The name of the database value to retrieve. Default is "Invalid"
 *           which will cause to skip that index.
 * @cassttng SACLAOnlineInput/DatabaseValues/\%index\%/{CASSName}\n
 *           The name that the value should have within the CASSEvent. Default is
 *           "Invalid" in which case the name of the value will be chosen.
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
   * this is the main while loop collecting the requested data and making it
   * available using the ringbuffer.
   */
  void runthis();

  /** do not load anything after it is started */
  void load() {}

  /** retrieve the number of processed events
   *
   * @return number of processed events
   */
  uint64_t eventcounter() {return _evtCounter;}

  /** retrieve the number of skipped processed events
   *
   * @return number of processed events
   */
  uint64_t skippedeventcounter() {return _skippedEvtCounter;}

private:
  /** constructor
   *
   * creates the thread.
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

   /** a counter for the events so far */
   uint64_t _evtCounter;

   /** a counter for the skipped events so far */
   uint64_t _skippedEvtCounter;
};

}//end namespace cass

#endif
