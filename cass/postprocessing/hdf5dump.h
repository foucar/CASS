// Copyright (C) 2010 Thomas White

#ifndef _HDF5_POSTPROCESSOR_H_
#define _HDF5_POSTPROCESSOR_H_

#include <hdf5.h>

#include "backend.h"
#include "postprocessor.h"
#include "cass_event.h"
#include "cass_acqiris.h"
#include "machine_device.h"

namespace cass
{
  /** HDF5 Dump
   *
   * This postprocessor will dump the pnCCD images and other relevant
   * data to an HDF5 file.  One HDF5 file will be created for each
   * event.
   *
   * @author Thomas White
   */
  class pp1000 : public PostprocessorBackend
  {
  public:
      /** constructor.*/
      pp1000(PostProcessors&, const PostProcessors::key_t&);

      /** destructor */
      virtual ~pp1000(){}

      /** copy image from CASS event to HDF5 */
      virtual void operator()(const CASSEvent&);

  protected:
    void write_HDF5(const cass::CASSEvent &cassevent);
    void add_acqiris_traces(hid_t fh, cass::ACQIRIS::Instruments instrument,
                            const cass::CASSEvent &cassevent);
    double photonEnergyWithoutLossCorrection
                                   (MachineData::MachineDataDevice::bldMap_t d);
    double photonEnergy(MachineData::MachineDataDevice::bldMap_t d);

    void add_bl_data(hid_t fh, hid_t sh, const char *field,
                     MachineData::MachineDataDevice::bldMap_t d);
    void cleanup(hid_t fh);

    QMutex _hdf5_lock;

  };
}

#endif




// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "Stroustrup"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:
