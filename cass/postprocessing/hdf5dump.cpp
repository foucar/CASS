// Copyright (C) 2010 Thomas White
// Based on previous postprocessor.cpp
//  by Anton Barty, Filipe Maia and Rick Kirian

#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <hdf5.h>

#include <QtCore/QString>
#include <qfileinfo.h>

#include "ccd_device.h"
#include "pnccd_device.h"
#include "pnccd_detector.h"
#include "machine_device.h"
#include "histogram.h"
#include "cass_event.h"
#include "postprocessing/postprocessor.h"
#include "postprocessing/hdf5dump.h"
#include "acqiris_detectors_helper.h"
#include "tof_detector.h"
#include "pdsdata/xtc/Dgram.hh"
#include "cass_acqiris.h"


namespace cass
{

#if H5_VERS_MAJOR < 2
#if H5_VERS_MINOR < 8
#define H5Dcreate1(A,B,C,D,E) H5Dcreate(A,B,C,D,E)
#define H5Gcreate1(A,B,C) H5Gcreate(A,B,C)
#define H5Lcreate_soft(A,B,C,D,E) 0
#endif
#endif


void pp1000::cleanup(hid_t fh)
{
	int n_ids, i;
	hid_t ids[256];

	n_ids = H5Fget_obj_ids(fh, H5F_OBJ_ALL, 256, ids);
	for ( i=0; i<n_ids; i++ ) {

		hid_t id;
		H5I_type_t type;

		id = ids[i];
		type = H5Iget_type(id);

		if ( type == H5I_GROUP ) H5Gclose(id);

	}
}


void pp1000::add_bl_data(hid_t fh, hid_t sh, const char *field,
                        MachineData::MachineDataDevice::bldMap_t d)
{
  if ( d.find(field) == d.end() ) {
    std::cout << "Field '" << field << "' not found." << std::endl;
    return;
  }

  double val = d.find(field)->second;
  char tmp[128];
  hid_t dataset_id;

  snprintf(tmp, 127, "/LCLS/%s", field);
  dataset_id = H5Dcreate1(fh, field, H5T_NATIVE_DOUBLE,
                          sh, H5P_DEFAULT);
  H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
           H5P_DEFAULT, &val);
  H5Dclose(dataset_id);
}


/*
   Returns the photon energy in eV.
   It uses Rick K. code at psexport:/reg/neh/home/rkirian/ana2
*/
double  pp1000::photonEnergy(MachineData::MachineDataDevice::bldMap_t d)
{
  if ( d.find("EbeamL3Energy") == d.end() ) {
    std::cout << "Field 'EbeamL3Energy' not found." << std::endl;
    return NAN;
  }

  if ( d.find("EbeamPkCurrBC2") == d.end() ) {
    std::cout << "Field 'EbeamPkCurrBC2' not found." << std::endl;
    return NAN;
  }

  // Get electron beam parameters from beamline data
  double fEbeamL3Energy = d.find("EbeamL3Energy")->second; // in MeV
  double fEbeamPkCurrBC2 = d.find("EbeamPkCurrBC2")->second; // in Amps

  // Get the present peak current in Amps
  double peakCurrent = fEbeamPkCurrBC2;

  // Get present beam energy [GeV]
  double DL2energyGeV = 0.001*fEbeamL3Energy;

  // wakeloss prior to undulators
  double LTUwakeLoss = 0.0016293*peakCurrent;

  // Spontaneous radiation loss per segment
  double SRlossPerSegment = 0.63*DL2energyGeV;

  // wakeloss in an undulator segment
  double wakeLossPerSegment = 0.0003*peakCurrent;

  // energy loss per segment
  double energyLossPerSegment = SRlossPerSegment + wakeLossPerSegment;

  // energy in first active undulator segment [GeV]
  double energyProfile = DL2energyGeV - 0.001*LTUwakeLoss
                                      - 0.0005*energyLossPerSegment;

  // Calculate the resonant photon energy of the first active segment
  double photonEnergyeV = 44.42*energyProfile*energyProfile;

  return photonEnergyeV;
}

/** Calculate the resonant photon energy (without energy loss corrections)
 *  Use the simple expression in e.g. Ayvazyan, V. et al. (2005).
 *  This expression requires:
 *    1) electron energy (in the xtc files)
 *    2) undulator period (~3cm for LCLS)
 *    3) undulator K (~3.5 at the LCLS)
 */
double pp1000::photonEnergyWithoutLossCorrection
                                    (MachineData::MachineDataDevice::bldMap_t d)
{
  if ( d.find("EbeamL3Energy") == d.end() ) {
    std::cout << "Field 'EbeamL3Energy' not found." << std::endl;
    return NAN;
  }
  const double ebEnergy = d.find("EbeamL3Energy")->second;
  const double K = 3.5;  // K of the undulator (provided by Marc Messerschmidt)
  const double lambda = 3.0e7; // LCLS undulator period in nm
  const double hc = 1239.84172; // in eV*nm
  // electron energy in rest mass units (E/mc^2)
  double gamma = ebEnergy/(0.510998903);
  // resonant photon wavelength in same units as undulator period)
  double resonantPhotonEnergy = hc*2*gamma*gamma/(lambda*(1+K*K/2));
  return resonantPhotonEnergy;
}


void pp1000::add_acqiris_traces(hid_t fh, cass::ACQIRIS::Instruments instrument,
                                const cass::CASSEvent &cassevent)
{
  // Get Acqiris device
  const cass::ACQIRIS::Device *acq = dynamic_cast<const cass::ACQIRIS::Device *>
                         (cassevent.devices().find(CASSEvent::Acqiris)->second);
  // Find instrument
  cass::ACQIRIS::Device::instruments_t::const_iterator acqI =
                                  acq->instruments().find(instrument);
  if (acqI == acq->instruments().end())
  {
    std::cerr << "Failed to find Acqiris instrument "
              << instrument << std::endl;
    return;
  }
  const cass::ACQIRIS::Instrument &instr = acqI->second;

  int n_acqiris_channels = instr.channels().size();
  hsize_t dims[2];
  int16_t n_acqiris_channels16 = (int16_t)n_acqiris_channels;
  dims[0] = 1;
  hid_t sh = H5Screate_simple(1, dims, NULL);
  if ( sh == 0 ) {
    printf("Failed to create Acqiris num_channels dataspace\n");
    return;
  }
  hid_t dh = H5Dcreate1(fh, "num_channels", H5T_NATIVE_INT16, sh, H5P_DEFAULT);
  if ( dh == 0 ) {
    printf("Failed to create Acqiris num_channels dataset\n");
    return;
  }
  if ( H5Dwrite(dh, H5T_NATIVE_INT16, H5S_ALL, H5S_ALL,
           H5P_DEFAULT, &n_acqiris_channels16) < 0 ) {
    printf("Failed to write Acqiris num_channels\n");
    return;
  }
  H5Dclose(dh);
  H5Sclose(sh);

  if (n_acqiris_channels >= 2)
  {
    for(int i=0; i<n_acqiris_channels; i++)
    {

      hid_t gh;
      char fieldname[64];

      // Create a group for this channel
      snprintf(fieldname, 63, "ch%i", i);
      gh = H5Gcreate1(fh, fieldname, 0);
      if ( gh == 0 ) {
        printf("Couldn't create group for channel %i\n", i);
        continue;
      }

      // Get the channel
      const cass::ACQIRIS::Channel &channel = instr.channels()[i];

      dims[0] = 1;
      hid_t sh = H5Screate_simple(1, dims, NULL);
      if ( sh == 0 ) {
        printf("Couldn't create dataspace (interval) for channel %i\n", i);
        continue;
      }
      double sampleinterval = channel.sampleInterval();
      hid_t dh = H5Dcreate1(gh, "sample_interval", H5T_NATIVE_DOUBLE, sh,
                            H5P_DEFAULT);
      if ( dh == 0 ) {
        printf("Couldn't create dataset (interval) for channel %i\n", i);
        continue;
      }
      if ( H5Dwrite(dh, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
               H5P_DEFAULT, &sampleinterval) < 0 ) {
        printf("Couldn't write interval for channel %i\n", i);
        continue;
      }
      H5Dclose(dh);
      H5Sclose(sh);

      // Get the waveform
      const cass::ACQIRIS::waveform_t &waveform = channel.waveform();
      if ( waveform.size() == 0 )
      {
        printf("Acqiris with no contents\n");
        continue;
      }
      dims[0] = waveform.size();
      dims[1] = 1;

      sh = H5Screate_simple(2, dims, NULL);
      if ( sh == 0 ) {
        printf("Couldn't create dataspace (ADC) for channel %i\n", i);
        continue;
      }
      dh = H5Dcreate1(gh, "ADC", H5T_NATIVE_SHORT, sh, H5P_DEFAULT);
      if ( dh == 0 ) {
        printf("Couldn't create dataset (ADC) for channel %i\n", i);
        continue;
      }
      if ( H5Dwrite(dh, H5T_NATIVE_SHORT, H5S_ALL, H5S_ALL,
                   H5P_DEFAULT, &waveform[0]) < 0)
      {
        printf("Error when writing data (ADC) %in", i);
        return;
      }
      H5Sclose(sh);
      H5Dclose(dh);

      // Convert to volts
      float *volts(new float[waveform.size()]);
      std::transform(waveform.begin(), waveform.end(), volts,
                     cass::ACQIRIS::Adc2Volts(channel.gain(),channel.offset()));

      snprintf(fieldname, 63, "ch%i_V", i);

      sh = H5Screate_simple(2, dims, NULL);
      if ( sh == 0 ) {
        printf("Couldn't create dataspace (volts) for channel %i\n", i);
        continue;
      }
      dh = H5Dcreate1(gh, "Volts", H5T_NATIVE_FLOAT, sh, H5P_DEFAULT);
      if ( dh == 0 ) {
        printf("Couldn't create dataset (volts) for channel %i\n", i);
        continue;
      }
      if ( H5Dwrite(dh, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                   H5P_DEFAULT, volts) < 0)
      {
        printf("Error when writing data %in", i);
        return;
      }
      delete[](volts);
      H5Sclose(sh);
      H5Dclose(dh);

      H5Gclose(gh);

    }
  }
}


// export current pnCCD frames to HDF5 file
void pp1000::write_HDF5(const cass::CASSEvent &cassevent)
{
  _hdf5_lock.lock();

  // Dig out the pnCCD device from the CASSEvent
  pnCCD::pnCCDDevice *dev = dynamic_cast<pnCCD::pnCCDDevice *>
                           (cassevent.devices().find(CASSEvent::pnCCD)->second);

  static QString xtcfile = QString("exx-rxxxx");

  // Simply return if there are no CCD frames!
  unsigned int nframes = dev->detectors()->size();
  if ( nframes < 1 ) {
    printf("No pnCCD frames found in this event.\n");
    return;
  }

  // Create filename based on date, time and LCLS fiducial for this image
  char outfile[1024];
  char buffer1[80];
  char buffer2[80];
  char buffer3[80];
  /* Check if we get a valid filename. Otherwise just use previous filename */
  if ( cassevent.filename() && cassevent.filename()[0] != 0 ) {
    xtcfile = QFileInfo(cassevent.filename()).baseName();
  }
  const Pds::Dgram *datagram = reinterpret_cast<const Pds::Dgram*>
                                                   (cassevent.datagrambuffer());

  // Get time and fiducial ...
  time_t eventTime = datagram->seq.clock().seconds();
  int32_t eventFiducial = datagram->seq.stamp().fiducials();

  // ... and turn them into something readable
  struct tm timeinfo;
  localtime_r(&eventTime, &timeinfo);
  strftime(buffer1, 80, "%Y_%b%d", &timeinfo);
  strftime(buffer2, 80, "%H%M%S", &timeinfo);
  strncpy(buffer3, xtcfile.toAscii().constData()+4, 5);
  buffer3[5] = '\0';
  sprintf(outfile, "LCLS_%s_%s_%s_%i_pnCCD.h5",
          buffer1, buffer3, buffer2, eventFiducial);

  // Create the HDF5 file
  hid_t   fh;
  hid_t   dataspace_id;
  hid_t   dataset_id;
  hid_t   datatype;
  hsize_t dims[2];
  herr_t  hdf_error;
  hid_t   gh;
  hid_t   gid;

  fh = H5Fcreate(outfile, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  if ( fh == 0 ) {
    std::cout << "Couldn't create file." << std::endl;
    return;
  }

  // Write interesting data to '/data' part of HDF5 file
  gid = H5Gcreate1(fh, "data", 0);
  if ( gid == 0 ) {
    std::cout << "Couldn't create /data." << std::endl;
    return;
  }

  hid_t itof_gid = H5Gcreate1(gid, "Acqiris_IToF", 0);
  if ( itof_gid == 0 ) {
    std::cout << "Couldn't create /data/Acqiris_IToF" << std::endl;
  } else {
    add_acqiris_traces(itof_gid, cass::ACQIRIS::Camp2, cassevent);
    H5Gclose(itof_gid);
  }
  hid_t camp_gid = H5Gcreate1(gid, "Acqiris_CAMP", 0);
  if ( camp_gid == 0 ) {
    std::cout << "Couldn't create /data/Acqiris_CAMP" << std::endl;
  } else {
    add_acqiris_traces(camp_gid, cass::ACQIRIS::Camp1, cassevent);
    H5Gclose(camp_gid);
  }

  // Save VMI frame
  CCD::CCDDevice *vmi = dynamic_cast<CCD::CCDDevice *>
                           (cassevent.devices().find(CASSEvent::CCD)->second);
  if ( vmi->detectors()->size() < 1 ) {

    printf("VMI Detector not found\n");

  } else {

    // Rummage around a bit to get the detector
    PixelDetector &det = (*vmi->detectors())[0];

    // Get rows/cols
    int rows = det.rows();
    int columns = det.columns();

    // Get frame
    const PixelDetector::frame_t &frame(det.frame());

    int r;
    char fieldname[128];
    float *data(new float [rows*columns]);
    std::copy(frame.begin(), frame.end(), data);
    snprintf(fieldname, 127, "/data/VMI");
    dims[0] = rows;
    dims[1] = columns;
    dataspace_id = H5Screate_simple(2, dims, NULL);
    if ( dataspace_id == 0 ) {
      std::cout << "Couldn't create VMI dataspace" << std::endl;
    } else {
      dataset_id = H5Dcreate1(fh, fieldname, H5T_NATIVE_SHORT,
                              dataspace_id, H5P_DEFAULT);
      r = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                   H5P_DEFAULT, data);
      if ( r < 0 ) {
        printf("Error when VMI writing data\n");
      }
      delete[](data);
      H5Dclose(dataset_id);
      H5Sclose(dataspace_id);
    }

  }

  // Save each pnCCD frame in the XTC data set
  int skipped = 0;
  for ( unsigned int i=0; i<nframes; i++) {

    if ( dev->detectors()->size() <= i ) {
      printf("Detector not found\n");
      continue;
    }

    // Rummage around a bit to get the detector
    PixelDetector &det = (*dev->detectors())[i];

    // Get rows/cols
    int rows = det.rows();
    int columns = det.columns();
    if ( !rows || !columns ) {
      skipped++;
      printf("pnCCD frame with illogical size %dx%d!\n",columns,rows);
      continue;
    }

    // Get frame
    const PixelDetector::frame_t &frame(det.frame());

    int r;
    char fieldname[128];
    float *data = (float *)malloc(rows*columns*sizeof(float));
    std::copy(frame.begin(), frame.end(), data);
    snprintf(fieldname, 127, "/data/data%i",i-skipped);
    dims[0] = rows;
    dims[1] = columns;
    dataspace_id = H5Screate_simple( 2, dims, NULL);
    if ( dataspace_id == 0 ) {
      std::cout << "Couldn't create pnCCD dataspace." << std::endl;
    } else {
      dataset_id = H5Dcreate1(fh, fieldname, H5T_NATIVE_SHORT,
                              dataspace_id, H5P_DEFAULT);
      if ( dataset_id == 0 ) {
        std::cout << "Couldn't create pnCCD dataset" << std::endl;
      } else {
        r = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                     H5P_DEFAULT, data);
        free(data);
        if ( r < 0 ) {
          printf("Error when writing data %i...\n",i);
          return;
        }
        H5Dclose(dataset_id);
      }
    }
    H5Sclose(dataspace_id);

  }

  // Save information on number of pnCCD frames saved
  dims[0] = 1;
  dataspace_id = H5Screate_simple( 1, dims, NULL );
  //dataspace_id = H5Screate(H5S_SCALAR);
  int adjusted_nframes = nframes-skipped;
  if ( dataspace_id == 0 ) {
    std::cout << "Couldn't create pnCCD nframes space" << std::endl;
  } else {
    dataset_id = H5Dcreate1(fh, "/data/nframes", H5T_NATIVE_SHORT,
                            dataspace_id, H5P_DEFAULT);
    if ( dataset_id == 0 ) {
      std::cout << "Couldn't create pnCCD nframes dataset" << std::endl;
    } else {
      if ( H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                   H5P_DEFAULT, &adjusted_nframes) < 0 ) {
        std::cout << "Failed to write pnCCD nframes" << std::endl;
      }
      H5Dclose(dataset_id);
      H5Sclose(dataspace_id);
    }
  }

  // Create symbolic link from /data/data0 to /data/data
  // (to maintain our convention of /data/data always containing data)
  hdf_error = H5Lcreate_soft( "/data/data0", fh, "/data/data",0,0);

  // Write pnCCD configurations
  H5Gclose(gid);
  gid = H5Gcreate1(fh, "pnCCD", 0);
  dims[0] = 1;

  dataspace_id = H5Screate_simple(1, dims, NULL );
  dataset_id = H5Dcreate1(fh, "/pnCCD/n_CCDs", H5T_NATIVE_SHORT,
                          dataspace_id, H5P_DEFAULT);
  H5Dwrite(dataset_id, H5T_NATIVE_SHORT, H5S_ALL, H5S_ALL,
           H5P_DEFAULT,&nframes);
  H5Dclose(dataset_id);
  H5Sclose(dataspace_id);

  skipped = 0;
  for ( unsigned int i=0; i<nframes; i++ ) {

    if ( dev->detectors()->size() <= i ) {
      printf("Detector not found\n");
      continue;
    }

    // Rummage around a bit to get the detector
    PixelDetector &det = (*dev->detectors())[i];

    // Get rows/cols
    int rows = det.rows();
    int columns = det.columns();

    char fieldname[100];
    int ccd_index = i-skipped;

    if ( !rows || !columns ) {
      printf("pnCCD frame isn't square %dx%d!\n",columns,rows);
      skipped++;
      continue;
    }

    sprintf(fieldname,"/pnCCD/pnCCD%i", ccd_index);
    hid_t gid = H5Gcreate1(fh,fieldname,0);
    dataspace_id = H5Screate_simple( 1, dims, NULL );

    sprintf(fieldname,"/pnCCD/pnCCD%i/rows", ccd_index);
    dataset_id = H5Dcreate1(fh, fieldname, H5T_NATIVE_SHORT,
                            dataspace_id, H5P_DEFAULT);
    H5Dwrite(dataset_id, H5T_NATIVE_SHORT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
             &rows);
    H5Dclose(dataset_id);

    sprintf(fieldname,"/pnCCD/pnCCD%i/columns", ccd_index);
    dataset_id = H5Dcreate1(fh, fieldname, H5T_NATIVE_SHORT,
                            dataspace_id, H5P_DEFAULT);
    H5Dwrite(dataset_id, H5T_NATIVE_SHORT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
             &columns);
    H5Dclose(dataset_id);

    sprintf(fieldname,"/pnCCD/pnCCD%i/originalrows", ccd_index);
    dataset_id = H5Dcreate1(fh, fieldname, H5T_NATIVE_SHORT,
                            dataspace_id, H5P_DEFAULT);
    H5Dwrite(dataset_id, H5T_NATIVE_SHORT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
             &det.originalrows());
    H5Dclose(dataset_id);

    sprintf(fieldname,"/pnCCD/pnCCD%i/originalcolumns", ccd_index);
    dataset_id = H5Dcreate1(fh, fieldname, H5T_NATIVE_SHORT,
                            dataspace_id, H5P_DEFAULT);
    H5Dwrite(dataset_id, H5T_NATIVE_SHORT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
             &det.originalcolumns());
    H5Dclose(dataset_id);

    sprintf(fieldname,"/pnCCD/pnCCD%i/row_binning", ccd_index);
    int16_t rbin = det.originalrows()/det.rows();
    dataset_id = H5Dcreate1(fh, fieldname, H5T_NATIVE_SHORT,
                            dataspace_id, H5P_DEFAULT);
    H5Dwrite(dataset_id, H5T_NATIVE_SHORT, H5S_ALL, H5S_ALL,
             H5P_DEFAULT, &rbin);
    H5Dclose(dataset_id);

    sprintf(fieldname,"/pnCCD/pnCCD%i/column_binning", ccd_index);
    int16_t cbin = det.originalcolumns()/det.columns();
    dataset_id = H5Dcreate1(fh, fieldname, H5T_NATIVE_SHORT,
                            dataspace_id, H5P_DEFAULT);
    H5Dwrite(dataset_id, H5T_NATIVE_SHORT, H5S_ALL, H5S_ALL,
             H5P_DEFAULT, &cbin);
    H5Dclose(dataset_id);

    sprintf(fieldname,"/pnCCD/pnCCD%i/integral",ccd_index);
    dataset_id = H5Dcreate1(fh, fieldname, H5T_NATIVE_LONG,
                            dataspace_id, H5P_DEFAULT);
    H5Dwrite(dataset_id, H5T_NATIVE_LONG, H5S_ALL, H5S_ALL, H5P_DEFAULT,
             &det.integral());
    H5Dclose(dataset_id);

    H5Sclose(dataspace_id);
    H5Gclose(gid);

  }

  // Write LCLS event information
  MachineData::MachineDataDevice *mdev =
                     dynamic_cast<MachineData::MachineDataDevice *>
                     (cassevent.devices().find(CASSEvent::MachineData)->second);
  gh = H5Gcreate1(fh, "LCLS" ,0);
  dims[0] = 1;
  dataspace_id = H5Screate_simple(1, dims, NULL);

  dataset_id = H5Dcreate1(gh, "/LCLS/machineTime", H5T_NATIVE_UINT32,
                          dataspace_id, H5P_DEFAULT);
  H5Dwrite(dataset_id, H5T_NATIVE_UINT32, H5S_ALL, H5S_ALL,
           H5P_DEFAULT, &eventTime );
  H5Dclose(dataset_id);

  dataset_id = H5Dcreate1(gh, "/LCLS/fiducial",
                          H5T_NATIVE_INT32, dataspace_id, H5P_DEFAULT);
  H5Dwrite(dataset_id, H5T_NATIVE_INT32, H5S_ALL, H5S_ALL,
           H5P_DEFAULT, &eventFiducial);
  H5Dclose(dataset_id);

  int id = cassevent.id();
  dataset_id = H5Dcreate1(gh, "/LCLS/casseventID", H5T_NATIVE_UINT64,
                          dataspace_id, H5P_DEFAULT);
  H5Dwrite(dataset_id, H5T_NATIVE_UINT64, H5S_ALL, H5S_ALL,
           H5P_DEFAULT, &id);
  H5Dclose(dataset_id);

  dataset_id = H5Dcreate1(gh, "/LCLS/energy", H5T_NATIVE_DOUBLE,
                          dataspace_id, H5P_DEFAULT);
  H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
           H5P_DEFAULT, &mdev->energy());
  H5Dclose(dataset_id);

  // Gas detector values
  add_bl_data(gh, dataspace_id, "f_11_ENRC", mdev->BeamlineData());
  add_bl_data(gh, dataspace_id, "f_12_ENRC", mdev->BeamlineData());
  add_bl_data(gh, dataspace_id, "f_21_ENRC", mdev->BeamlineData());
  add_bl_data(gh, dataspace_id, "f_22_ENRC", mdev->BeamlineData());

  // Electron beam parameters
  add_bl_data(gh, dataspace_id, "EbeamCharge", mdev->BeamlineData());
  add_bl_data(gh, dataspace_id, "EbeamL3Energy", mdev->BeamlineData());
  add_bl_data(gh, dataspace_id, "EbeamLTUPosX", mdev->BeamlineData());
  add_bl_data(gh, dataspace_id, "EbeamLTUPosY", mdev->BeamlineData());
  add_bl_data(gh, dataspace_id, "EbeamLTUAngX", mdev->BeamlineData());
  add_bl_data(gh, dataspace_id, "EbeamLTUAngY", mdev->BeamlineData());
  add_bl_data(gh, dataspace_id, "EbeamPkCurrBC2", mdev->BeamlineData());

  // Wavelength (more complicated...)
  double resonantPhotonEnergy = photonEnergy(mdev->BeamlineData());
  double resonantPhotonEnergyNoEnergyLossCorrection =
                        photonEnergyWithoutLossCorrection(mdev->BeamlineData());
  double wavelength_nm = -1;
  double wavelength_A = -1;

  if (! std::isnan(resonantPhotonEnergy) ) {

    wavelength_nm = 1239.8/resonantPhotonEnergy;
    wavelength_A = 10*wavelength_nm;

    dataset_id = H5Dcreate1(fh, "/LCLS/photon_energy_eV",
                            H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
    H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
             H5P_DEFAULT, &resonantPhotonEnergy);
    H5Dclose(dataset_id);

    dataset_id = H5Dcreate1(fh, "/LCLS/photon_wavelength_nm",
                            H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
    H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
             H5P_DEFAULT, &wavelength_nm);
    H5Dclose(dataset_id);

    dataset_id = H5Dcreate1(fh, "/LCLS/photon_wavelength_A",
                             H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
    H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
             H5P_DEFAULT, &wavelength_A);
    H5Dclose(dataset_id);

  }

  if (! std::isnan(resonantPhotonEnergyNoEnergyLossCorrection) ) {
    dataset_id = H5Dcreate1(fh, "/LCLS/photon_energy_eV_no_energy_loss_correction",
                            H5T_NATIVE_DOUBLE, dataspace_id, H5P_DEFAULT);
    H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
             H5P_DEFAULT, &resonantPhotonEnergyNoEnergyLossCorrection);
    H5Dclose(dataset_id);
  }

  // Time in human readable format
  // Strings are a little tricky --> this could be improved!
  char *timestr(new char[128]);
  timestr = ctime_r(&eventTime, timestr);
  dataspace_id = H5Screate(H5S_SCALAR);
  datatype = H5Tcopy(H5T_C_S1);
  H5Tset_size(datatype,strlen(timestr)+1);
  dataset_id = H5Dcreate1(fh, "LCLS/eventTimeString", datatype,
                          dataspace_id, H5P_DEFAULT);
  H5Dwrite(dataset_id, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, timestr);
  H5Dclose(dataset_id);
  H5Sclose(dataspace_id);
  hdf_error = H5Lcreate_soft("/LCLS/eventTimeString", fh,
                             "/LCLS/eventTime", 0, 0);
  delete[](timestr);

  // Put the XTC filename somewhere
  dataspace_id = H5Screate(H5S_SCALAR);
  datatype = H5Tcopy(H5T_C_S1);
  //printf("xtcfile = %s\n",xtcfile.toAscii().constData());
  H5Tset_size(datatype,xtcfile.length()+1);
  dataset_id = H5Dcreate1(fh, "LCLS/xtcFilename", datatype,
                          dataspace_id, H5P_DEFAULT);
  H5Dwrite(dataset_id, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT,
           xtcfile.toAscii().constData());
  H5Dclose(dataset_id);
  H5Sclose(dataspace_id);

  H5Gclose(gid);
  H5Fflush(fh,H5F_SCOPE_LOCAL);

  cleanup(fh);
  H5Fclose(fh);

  _hdf5_lock.unlock();
}


// *** postprocessor 1000 -- Dump pnCCD (and other) data to HDF5 ***

pp1000::pp1000(PostProcessors& pp, const cass::PostProcessors::key_t &key)
    : PostprocessorBackend(pp, key)
{
  // Create dummy histogram to make sure we actually get called
  _result = new Histogram0DFloat();
  createHistList(1);
  std::cout << "Postprocessor " << key << ": set up." << std::endl;
}

void pp1000::process(const cass::CASSEvent &event)
{
  write_HDF5(event);
}


} // end namespace cass



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "Stroustrup"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:
