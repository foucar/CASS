// Copyright (C) 2015 Lutz Foucar

/**
 * @file hdf5_file_input.h contains a hdf5 file reader class
 *
 * @author Lutz Foucar
 */

#ifndef _HDF5FILEINPUT_H_
#define _HDF5FILEINPUT_H_

#include <string>

#include "input_base.h"
#include "cass.h"
#include "ringbuffer.h"
#include "cass_event.h"
#include "file_reader.h"
#include "cass_acqiris.h"

namespace cass
{
/** HDF5 File Input for cass
 *
 * The HDF5 file is expected to contain the data separated by the events. For
 * each event there should be a group that contains the data of all the
 * detectors and deveices that were recorded for this event. The reader will
 * gather a list of all the "root" groups and then iterate through them. Thus
 * all dataset keys should be given with respect to the event group.
 *
 * @cassttng HDF5FileInput/{EventIDKey}\n
 *           The name of the dataset within the hdf5 file that allows to
 *           retrieve the current event ID.
 *           The Groupname of the Event can be omitted. Default is "EventID"
 * @cassttng HDF5FileInput/MachineValues/{size}\n
 *           The number of machine parameters that one wants to retrieve. Ensure
 *           that each parameter has a unique id in the list. All reuested
 *           machine parameters will be stored in the beamlineparameter part of
 *           the CASSEvent. Use PostProcessor 120 to retrieve those values for
 *           processing later on.
 * @cassttng HDF5FileInput/MachineValues/\%index\%/{HDF5Key}\n
 *           Name of the dataset that contains the requested machine parameter.
 *           The Groupname of the Event can be omitted. Default is "Invalid"
 * @cassttng HDF5FileInput/MachineValues/\%index\%/{CASSName}\n
 *           Name of the parameter within the Beamlineparameter part of the
 *           CASSEvent. Default is "Invalid" in which case the name of the
 *           dataset will be used.
 * @cassttng HDF5FileInput/AcqirisValues/{size}\n
 *           The number of acqirs instrument channels that one wants to
 *           retrieve. Each channel of an instrument has to be defined separately.
 *           Ensure that each parameter has a unique id in the list.
 * @cassttng HDF5FileInput/AcqirisValues/\%index\%/{HDF5DataKey}\n
 *           Name of the dataset that contains the requested acqiris channel data.
 *           The Groupname of the Event can be omitted. Default is "Invalid"
 * @cassttng HDF5FileInput/AcqirisValues/\%index\%/{HDF5HorposKey}\n
 *           Name of the dataset that contains the Horpos value  of the
 *           requested acqiris channel.
 *           The Groupname of the Event can be omitted. Default is "Invalid"
 * @cassttng HDF5FileInput/AcqirisValues/\%index\%/{HDF5VerticalOffsetKey}\n
 *           Name of the dataset that contains vertical offset value of the
 *           requested acqiris channel.
 *           The Groupname of the Event can be omitted. Default is "Invalid"
 * @cassttng HDF5FileInput/AcqirisValues/\%index\%/{HDF5GainKey}\n
 *           Name of the dataset that contains the vertical gain value of the
 *           requested acqiris channel.
 *           The Groupname of the Event can be omitted. Default is "Invalid"
 * @cassttng HDF5FileInput/AcqirisValues/\%index\%/{HDF5SampleIntervalKey}\n
 *           Name of the dataset that contains the sample interval value of the
 *           requested acqiris channel.
 *           The Groupname of the Event can be omitted. Default is "Invalid"
 * @cassttng HDF5FileInput/AcqirisValues/\%index\%/{CASSInstrumentID}\n
 *           The id of the insturment within the CASSEVent that the acqiris data
 *           should be copied to. Default is "1". This should not be changed since
 *           this is the generic instrument id. Note that not all ids are available.
 * @cassttng HDF5FileInput/AcqirisValues/\%index\%/{CASSChannelNumber}\n
 *           The channel number within the instrument of the CASSEvent that the
 *           channel data should be copied to. Default is 0.
 * @cassttng HDF5FileInput/PixelDetectorValues/{size}\n
 *           The number of pixeldetectors that one wants to
 *           retrieve. Ensure that each parameter has a unique id in the list.
 * @cassttng HDF5FileInput/PixelDetectorValues/\%index\%/{HDF5DataKey}\n
 *           Name of the dataset that contains the requested pixel detector data.
 *           The Groupname of the Event can be omitted. Default is "Invalid"
 * @cassttng HDF5FileInput/PixelDetectorValues/\%index\%/{CASSID}\n
 *           The id that the pixel detector should have whithin the CASSEvent
 *           Default is 0.
 *
 * @author Lutz Foucar
 */
class HDF5FileInput :  public InputBase
{
public:
  /** create instance of this
   *
   * @param filelistname name of the file containing all files that should be
   *                     processed
   * @param ringbuffer reference to the ringbuffer containing the CASSEvents
   * @param ratemeter reference to the ratemeter to measure the rate of the input
   * @param loadmeter reference to the ratemeter to measure the load of the input
   * @param quitwhendone flag that tells this class that it should quit the
   *                     Program when its done reading all events
   * @param parent The parent QT Object of this class
   */
  static void instance(std::string filelistname,
                       RingBuffer<CASSEvent>&,
                       Ratemeter &ratemeter,
                       Ratemeter &loadmeter,
                       bool quitwhendone,
                       QObject *parent=0);

  /** function with the main loop */
  void run();

  /** load the parameters used for the multifile input */
  void load();

public:
  /** parameters needed to retrieve the Acqiris data */
  struct AcqirisParams
  {
    /** name of the waveform datafield */
    std::string DataKey;
    /** name of the field that contains the horpos value */
    std::string HorposKey;
    /** name of the field that contains the vertical offset value */
    std::string VertOffsetKey;
    /** name of the field that contains the gain value */
    std::string GainKey;
    /** name of the field that contains the sample interval */
    std::string SampleIntervalKey;
    /** the instrument within the CASSEvent that should be used */
    ACQIRIS::Instruments Instrument;
    /** the channel with the instrument within the CASSEvent that the data should
     *  loaded to
     */
    size_t ChannelNumber;
  };

  /** parameters needed to retrieve the pixeldetector data */
  struct PixeldetectorParams
  {
    /** name of the waveform datafield */
    std::string DataKey;

    /** the id of the pixeldetector that it should have in the CASSEvent */
    int CASSID;
  };

private:
  /** constructor
   *
   * @param filelistname name of the file containing all files that should be
   *                     processed
   * @param ringbuffer reference to the ringbuffer containing the CASSEvents
   * @param ratemeter reference to the ratemeter to measure the rate of the input
   * @param loadmeter reference to the ratemeter to measure the load of the input
   * @param quitwhendone flag that tells this class that it should quit the
   *                     Program when its done reading all events
   * @param parent The parent QT Object of this class
   */
  HDF5FileInput(std::string filelistname,
                RingBuffer<CASSEvent>&,
                Ratemeter &ratemeter,
                Ratemeter &loadmeter,
                bool quitwhendone,
                QObject *parent=0);

  /** flag to tell the thread to quit when its done with all files */
  bool _quitWhenDone;

  /** name of the file containing all files that we need to process */
  std::string _filelistname;

};

}//end namespace cass

#endif
