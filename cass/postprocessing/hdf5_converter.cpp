//Copyright (C) 2010 Lutz Foucar

/**
 * @file hdf5_converter.cpp definition of pp1001 (hdf5_converter)
 *
 * @author Lutz Foucar
 */

#include <QtCore/QDateTime>

#include <hdf5.h>
#include <stdint.h>

#include "hdf5_converter.h"
#include "histogram.h"
#include "cass_event.h"
#include "cass_settings.h"
#include "machine_device.h"
#include "log.h"

using namespace cass;
using namespace std;

namespace cass
{

namespace hdf5
{

/** create group name for an event from its ID
 *
 * details
 *
 * @todo make sure that it will be always converted to the timezone in
 *        stanford otherwise people get confused. Timezones are not
 *        yet supported in QDateTime
 *
 * @return id pointing to the groupname created
 * @param eventid the event id
 * @param calibcylce
 *
 * @author Lutz Foucar
 */
hid_t createGroupNameFromEventId(uint64_t eventid, hid_t calibcycle)
{
  uint32_t timet(static_cast<uint32_t>((eventid & 0xFFFFFFFF00000000) >> 32));
  uint32_t eventFiducial = static_cast<uint32_t>((eventid & 0x00000000FFFFFFFF) >> 8);
  std::stringstream groupname;
  QDateTime time;
  time.setTime_t(timet);
  if (timet)
    groupname << time.toString(Qt::ISODate).toStdString() <<"_"<<eventFiducial;
  else
    groupname << "UnknownTime_"<<eventid;
  VERBOSEOUT(std::cout<<"createGroupNameFromEventId(): creating group: "<<groupname.str()
             <<std::endl);
  return H5Gcreate1(calibcycle, groupname.str().c_str(),0);
}

/** write an float value with a given name as part of a given group
 *
 * create a dataspace and a dataset for writing the value as part of the given
 * group. Then write the value and close all resources later on. The dataspace
 * needs to be a 2d matrix with 1x1 contents.
 *
 * @todo find out whether it is possible to use scalar instead of 2d value.
 *
 * @param value the value to be written
 * @param valname the name of the value
 * @param groupid the id of the group that the value should be part of
 *
 * @author Lutz Foucar
 */
void writeFloatValue(const float value, const string& valname, hid_t groupid)
{
  // hid_t dataspace_id = (H5Screate(H5S_SCALAR));

  hsize_t dims[2];
  dims[0] = 1;
  hid_t dataspace_id = H5Screate_simple(1, dims, NULL);
  if (dataspace_id == 0)
    throw runtime_error("writeFloatValue(): Could not open the dataspace");

  hid_t dataset_id (H5Dcreate1(groupid, valname.c_str(),
                               H5T_NATIVE_FLOAT, dataspace_id, H5P_DEFAULT));
  if (dataset_id == 0)
    throw runtime_error("writeFloatValue(): Could not open the dataset '" + valname +"'");

  H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
           H5P_DEFAULT, &value);

  H5Dclose(dataset_id);
  H5Sclose(dataspace_id);

}

/** write the axis properties to the chosen group
 *
 * details
 *
 * @param axis the axis that contains the info to be written
 * @param groupid the id of the group that the information should be written to
 *
 * @param Lutz Foucar
 */
void writeAxisProperty(const AxisProperty& axis, hid_t groupid)
{
  // Create the data space for the dataset.
  hsize_t dims[2];
  dims[0] = 1;
  hid_t dataspace_id = H5Screate_simple(1, dims, NULL);

  //Create the dataset.
  hid_t dataset_id (H5Dcreate1(groupid, "NumberOfBins",
                               H5T_NATIVE_INT, dataspace_id, H5P_DEFAULT));
  //get copy of value to write
  const int nbins (axis.nbrBins());
  //write value
  H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
           H5P_DEFAULT, &nbins);
  //End access to the dataset and release resources used by it.
  H5Dclose(dataset_id);

  //Create the dataset.
  dataset_id = (H5Dcreate1(groupid, "Low",
                           H5T_NATIVE_FLOAT, dataspace_id, H5P_DEFAULT));
  //get copy of value to write
  const float low (axis.lowerLimit());
  //write value
  H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
           H5P_DEFAULT, &low);
  //End access to the dataset and release resources used by it.
  H5Dclose(dataset_id);

  //Create the dataset.
  dataset_id = (H5Dcreate1(groupid, "Up",
                           H5T_NATIVE_FLOAT, dataspace_id, H5P_DEFAULT));
  //get copy of value to write
  const float up (axis.upperLimit());
  //write value
  H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
           H5P_DEFAULT, &up);
  //End access to the dataset and release resources used by it.
  H5Dclose(dataset_id);

  //Terminate access to the data space.
  H5Sclose(dataspace_id);

  //write title//
  dataspace_id = (H5Screate(H5S_SCALAR));
  hid_t datatype (H5Tcopy(H5T_C_S1));
  H5Tset_size(datatype,axis.title().size()+1);
  dataset_id = (H5Dcreate1(groupid, "Title", datatype,
                           dataspace_id, H5P_DEFAULT));
  H5Dwrite(dataset_id, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT,
           axis.title().c_str());
  H5Dclose(dataset_id);
  H5Sclose(dataspace_id);
}

/** write the properties of the histogram to the chosen group
 *
 * details
 *
 * @param hist the result whos values should be added to the group
 * @param groupid the id of the group that the contents should be written to
 *
 * @param Lutz Foucar
 */
void writeHistProperties(const HistogramBackend& hist, hid_t groupid)
{
  // Create the data space for the dataset.
  hsize_t dims[2];
  dims[0] = 1;
  hid_t dataspace_id = H5Screate_simple(1, dims, NULL);

  //Create the dataset.
  hid_t dataset_id (H5Dcreate1(groupid, "NumberOfFills",
                               H5T_NATIVE_INT, dataspace_id, H5P_DEFAULT));
  //get copy of value to write
  const int nfill (hist.nbrOfFills());
  //write value
  H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
           H5P_DEFAULT, &nfill);
  //End access to the dataset and release resources used by it.
  H5Dclose(dataset_id);

  //Terminate access to the data space.
  H5Sclose(dataspace_id);
}

/** write a 0d histogram to the hdf5 file
 *
 * details
 *
 * @param hist the 0d histogram to be written
 * @param groupid the id of the group where the contents should be added to
 *
 * @author Lutz Foucar
 */
void write0DHist(const Histogram0DFloat& hist, hid_t groupid)
{
  // Create the data space for the dataset.
  hsize_t dims[2];
  dims[0] = 1;
  hid_t dataspace_id = H5Screate_simple(1, dims, NULL);

  //Create the dataset.
  hid_t dataset_id (H5Dcreate1(groupid, "Value",
                               H5T_NATIVE_FLOAT, dataspace_id, H5P_DEFAULT));
  //get copy of value to write
  const float value (hist.getValue());
  //write value
  H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
           H5P_DEFAULT, &value);
  //End access to the dataset and release resources used by it.
  H5Dclose(dataset_id);

  writeHistProperties(hist,groupid);

  //Terminate access to the data space.
  H5Sclose(dataspace_id);
}

/** write a 1d histogram to the hdf5 file
 *
 * details
 *
 * @param hist the 1d histogram to be written
 * @param groupid the id of the group where the contents should be added to
 *
 * @author Lutz Foucar
 */
void write1DHist(const Histogram1DFloat& hist, hid_t groupid, bool compress)
{
  hid_t axisgrouphandle (H5Gcreate1(groupid, "xAxis",0));
  writeAxisProperty(hist.axis()[HistogramBackend::xAxis],axisgrouphandle);
  H5Gclose(axisgrouphandle);

  const HistogramFloatBase::storage_t &data (hist.memory());
  const size_t nxbins (hist.axis()[HistogramBackend::xAxis].nbrBins());

  // Create the data space for the dataset.
  hsize_t dims[2];
  dims[0] = 1;
  hid_t dataspace_id = H5Screate_simple(1, dims, NULL);

  //Create the dataset.
  hid_t dataset_id (H5Dcreate1(groupid, "Underflow",
                               H5T_NATIVE_FLOAT, dataspace_id, H5P_DEFAULT));
  //get copy of value to write
  const float underflow (data[nxbins+HistogramBackend::Underflow]);
  //write value
  H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
           H5P_DEFAULT, &underflow);
  //End access to the dataset and release resources used by it.
  H5Dclose(dataset_id);

  //Create the dataset.
  dataset_id = (H5Dcreate1(groupid, "Overflow",
                           H5T_NATIVE_FLOAT, dataspace_id, H5P_DEFAULT));
  //get copy of value to write
  const float overflow (data[nxbins+HistogramBackend::Overflow]);
  //write value
  H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
           H5P_DEFAULT, &overflow);
  //End access to the dataset and release resources used by it.
  H5Dclose(dataset_id);

  writeHistProperties(hist,groupid);

  //Terminate access to the data space.
  H5Sclose(dataspace_id);

  // Create the data space for the dataset.
  dims[0] = data.size()-2;
  dims[1] = 2;
  dataspace_id = H5Screate_simple(2, dims, NULL);

  std::vector<float> table(dims[0]*dims[1]);
  std::vector<float>::iterator tableIt (table.begin());
  HistogramFloatBase::storage_t::const_iterator dataIt (data.begin());
  const AxisProperty & xaxis (hist.axis()[HistogramBackend::xAxis]);
  for (size_t ibin(0); ibin<xaxis.nbrBins(); ++ibin)
  {
    *tableIt++ = xaxis.position(ibin);
    *tableIt++ = *dataIt++;
  }

  //Create the dataset.
  if (compress)
  {
    // Create dataset creation property list, set the gzip compression filter
    // and chunck size
    hsize_t chunk[2] = {40,2};
    hid_t dcpl (H5Pcreate (H5P_DATASET_CREATE));
    H5Pset_deflate (dcpl, 9);
    H5Pset_chunk (dcpl, 2, chunk);
    dataset_id = (H5Dcreate(groupid, "1DHistData",
                            H5T_NATIVE_FLOAT, dataspace_id, H5P_DEFAULT, dcpl, H5P_DEFAULT));
  }
  else
    dataset_id = (H5Dcreate1(groupid, "1DHistData",
                             H5T_NATIVE_FLOAT, dataspace_id, H5P_DEFAULT));
  //write data
  H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
           H5P_DEFAULT, &table[0]);
  //End access to the dataset and release resources used by it.
  H5Dclose(dataset_id);
  //Terminate access to the data space.
  H5Sclose(dataspace_id);
}

/** write a 2d histogram to the hdf5 file
 *
 * details
 *
 * @param hist the 2d histogram to be written
 * @param groupid the id of the group where the contents should be added to
 *
 * @author Lutz Foucar
 */
void write2DHist(const Histogram2DFloat& hist, hid_t groupid, bool compress)
{
  hid_t axisgrouphandle (H5Gcreate1(groupid, "xAxis",0));
  writeAxisProperty(hist.axis()[HistogramBackend::xAxis],axisgrouphandle);
  H5Gclose(axisgrouphandle);

  axisgrouphandle = (H5Gcreate1(groupid, "yAxis",0));
  writeAxisProperty(hist.axis()[HistogramBackend::yAxis],axisgrouphandle);
  H5Gclose(axisgrouphandle);

  const HistogramFloatBase::storage_t &data (hist.memory());
  const size_t nxbins (hist.axis()[HistogramBackend::xAxis].nbrBins());
  const size_t nybins (hist.axis()[HistogramBackend::yAxis].nbrBins());
  const size_t maxSize  = nxbins*nybins;


  // Create the data space for the dataset.
  hsize_t dims[2];
  dims[0] = 1;
  hid_t dataspace_id = H5Screate_simple(1, dims, NULL);

  //Create the dataset.
  hid_t dataset_id (H5Dcreate1(groupid, "UpperLeft",
                               H5T_NATIVE_FLOAT, dataspace_id, H5P_DEFAULT));
  //get copy of value to write
  const float UpperLeft (data[maxSize+HistogramBackend::UpperLeft]);
  //write value
  H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
           H5P_DEFAULT, &UpperLeft);
  //End access to the dataset and release resources used by it.
  H5Dclose(dataset_id);

  //Create the dataset.
  dataset_id = (H5Dcreate1(groupid, "UpperMiddle",
                           H5T_NATIVE_FLOAT, dataspace_id, H5P_DEFAULT));
  //get copy of value to write
  const float UpperMiddle (data[maxSize+HistogramBackend::UpperMiddle]);
  //write value
  H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
           H5P_DEFAULT, &UpperMiddle);
  //End access to the dataset and release resources used by it.
  H5Dclose(dataset_id);

  //Create the dataset.
  dataset_id = (H5Dcreate1(groupid, "UpperRight",
                           H5T_NATIVE_FLOAT, dataspace_id, H5P_DEFAULT));
  //get copy of value to write
  const float UpperRight (data[maxSize+HistogramBackend::UpperRight]);
  //write value
  H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
           H5P_DEFAULT, &UpperRight);
  //End access to the dataset and release resources used by it.
  H5Dclose(dataset_id);

  //Create the dataset.
  dataset_id = (H5Dcreate1(groupid, "Left",
                           H5T_NATIVE_FLOAT, dataspace_id, H5P_DEFAULT));
  //get copy of value to write
  const float Left (data[maxSize+HistogramBackend::Left]);
  //write value
  H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
           H5P_DEFAULT, &Left);
  //End access to the dataset and release resources used by it.
  H5Dclose(dataset_id);

  //Create the dataset.
  dataset_id = (H5Dcreate1(groupid, "Right",
                           H5T_NATIVE_FLOAT, dataspace_id, H5P_DEFAULT));
  //get copy of value to write
  const float Right (data[maxSize+HistogramBackend::Right]);
  //write value
  H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
           H5P_DEFAULT, &Right);
  //End access to the dataset and release resources used by it.
  H5Dclose(dataset_id);

  //Create the dataset.
  dataset_id = (H5Dcreate1(groupid, "LowerLeft",
                           H5T_NATIVE_FLOAT, dataspace_id, H5P_DEFAULT));
  //get copy of value to write
  const float LowerLeft (data[maxSize+HistogramBackend::LowerLeft]);
  //write value
  H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
           H5P_DEFAULT, &LowerLeft);
  //End access to the dataset and release resources used by it.
  H5Dclose(dataset_id);

  //Create the dataset.
  dataset_id =(H5Dcreate1(groupid, "LowerMiddle",
                          H5T_NATIVE_FLOAT, dataspace_id, H5P_DEFAULT));
  //get copy of value to write
  const float LowerMiddle (data[maxSize+HistogramBackend::LowerMiddle]);
  //write value
  H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
           H5P_DEFAULT, &LowerMiddle);
  //End access to the dataset and release resources used by it.
  H5Dclose(dataset_id);

  //Create the dataset.
  dataset_id = (H5Dcreate1(groupid, "LowerRight",
                           H5T_NATIVE_FLOAT, dataspace_id, H5P_DEFAULT));
  //get copy of value to write
  const float LowerRight (data[maxSize+HistogramBackend::LowerRight]);
  //write value
  H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
           H5P_DEFAULT, &LowerRight);
  //End access to the dataset and release resources used by it.
  H5Dclose(dataset_id);

  writeHistProperties(hist,groupid);

  //Terminate access to the data space.
  H5Sclose(dataspace_id);

  // Create the data space for the dataset.
  dims[0] = data.size()-8;
  dims[1] = 3;
  dataspace_id = H5Screate_simple(2, dims, NULL);

  std::vector<float> table(dims[0]*dims[1]);
  std::vector<float>::iterator tableIt (table.begin());
  HistogramFloatBase::storage_t::const_iterator dataIt (data.begin());
  const AxisProperty & xaxis (hist.axis()[HistogramBackend::xAxis]);
  const AxisProperty & yaxis (hist.axis()[HistogramBackend::yAxis]);
  for (size_t iybin(0); iybin<yaxis.nbrBins(); ++iybin)
  {
    for (size_t ixbin(0); ixbin<xaxis.nbrBins(); ++ixbin)
    {
      *tableIt++ = xaxis.position(ixbin);
      *tableIt++ = yaxis.position(iybin);
      *tableIt++ = *dataIt++;
    }
  }
  //Create the dataset.
  if (compress)
  {
    // Create dataset creation property list, set the gzip compression filter
    // and chunck size
    hsize_t chunk[2] = {40,3};
    hid_t dcpl (H5Pcreate (H5P_DATASET_CREATE));
    H5Pset_deflate (dcpl, 9);
    H5Pset_chunk (dcpl, 2, chunk);
    dataset_id = (H5Dcreate(groupid, "HistData",
                            H5T_NATIVE_FLOAT, dataspace_id, H5P_DEFAULT, dcpl, H5P_DEFAULT));
  }
  else
    dataset_id = (H5Dcreate1(groupid, "HistData",
                             H5T_NATIVE_FLOAT, dataspace_id, H5P_DEFAULT));
  //write data
  H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
           H5P_DEFAULT, &table[0]);
  //End access to the dataset and release resources used by it.
  H5Dclose(dataset_id);
  //Terminate access to the data space.
  H5Sclose(dataspace_id);
}

/** write a 2d histogram to the hdf5 file
 *
 * the 2d histogram data will be added as matrix in the hdf5 instead of table
 * like in write2dHist().
 *
 * @param hist the 2d histogram to be written
 * @param groupid the id of the group where the contents should be added to
 *
 * @author Lutz Foucar
 */
void write2DMatrix(const Histogram2DFloat& hist, hid_t groupid, bool compress)
{
  hid_t axisgrouphandle (H5Gcreate1(groupid, "xAxis",0));
  writeAxisProperty(hist.axis()[HistogramBackend::xAxis],axisgrouphandle);
  H5Gclose(axisgrouphandle);

  axisgrouphandle = (H5Gcreate1(groupid, "yAxis",0));
  writeAxisProperty(hist.axis()[HistogramBackend::yAxis],axisgrouphandle);
  H5Gclose(axisgrouphandle);

  const HistogramFloatBase::storage_t &data (hist.memory());
  const size_t nxbins (hist.axis()[HistogramBackend::xAxis].nbrBins());
  const size_t nybins (hist.axis()[HistogramBackend::yAxis].nbrBins());
  const size_t maxSize  = nxbins*nybins;


  const float UpperLeft (data[maxSize+HistogramBackend::UpperLeft]);
  writeFloatValue(UpperLeft,"UpperLeft",groupid);

  const float UpperMiddle (data[maxSize+HistogramBackend::UpperMiddle]);
  writeFloatValue(UpperMiddle,"UpperMiddle",groupid);

  const float UpperRight (data[maxSize+HistogramBackend::UpperRight]);
  writeFloatValue(UpperRight,"UpperRight",groupid);

  const float Left (data[maxSize+HistogramBackend::Left]);
  writeFloatValue(Left,"Left",groupid);

  const float Right (data[maxSize+HistogramBackend::Right]);
  writeFloatValue(Right,"Right",groupid);

  const float LowerLeft (data[maxSize+HistogramBackend::LowerLeft]);
  writeFloatValue(LowerLeft,"LowerLeft",groupid);

  const float LowerMiddle (data[maxSize+HistogramBackend::LowerMiddle]);
  writeFloatValue(LowerMiddle,"LowerMiddle",groupid);

  const float LowerRight (data[maxSize+HistogramBackend::LowerRight]);
  writeFloatValue(LowerRight,"LowerRight",groupid);

  writeHistProperties(hist,groupid);

  // Create the data space for the dataset.
  hsize_t dims[2];
  dims[0] = nxbins;
  dims[1] = nybins;
  hid_t dataspace_id = H5Screate_simple(2, dims, NULL);
  if (dataspace_id == 0)
    throw runtime_error("pp1001::write2dMatrix(): Could not open the dataspace for the 2d matrix");

  //Create the dataset.
  hid_t dataset_id;
  if (compress)
  {
    // Create dataset creation property list, set the gzip compression filter
    // and chunck size
    hsize_t chunk[2] = {40,3};
    hid_t dcpl (H5Pcreate (H5P_DATASET_CREATE));
    H5Pset_deflate (dcpl, 9);
    H5Pset_chunk (dcpl, 2, chunk);
    dataset_id = (H5Dcreate(groupid, "HistData",
                            H5T_NATIVE_FLOAT, dataspace_id, H5P_DEFAULT, dcpl, H5P_DEFAULT));
  }
  else
    dataset_id = (H5Dcreate1(groupid, "HistData",
                             H5T_NATIVE_FLOAT, dataspace_id, H5P_DEFAULT));
  if (dataset_id == 0 )
    throw runtime_error("pp1001:write2dMatrix(): Could not open dataset for 2d matrix");

  //write data
  H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
           H5P_DEFAULT, &data.front());
  //End access to the dataset and release resources used by it.
  H5Dclose(dataset_id);
  //Terminate access to the data space.
  H5Sclose(dataspace_id);
}

/** write all beamline data variables to file
 *
 * when there is no beamline data to be written return immediately,
 * otherwise write all beamline data available to the "Beamline Data"
 * group after creating it.
 *
 * @param bld the beamline data contained in the event
 * @param groupid the id of the group that the data shoudl be added to
 *
 * @author Lutz Foucar
 */
void writeBeamlineData(const MachineData::MachineDataDevice::bldMap_t &bld, hid_t groupid)
{
  if (bld.empty())
    return;
  hid_t bldGroup(H5Gcreate1(groupid,"Beamline Data",0));
  MachineData::MachineDataDevice::bldMap_t::const_iterator bldIt(bld.begin());
  MachineData::MachineDataDevice::bldMap_t::const_iterator bldEnd(bld.end());
  for(; bldIt != bldEnd; ++bldIt)
    writeFloatValue(bldIt->second,bldIt->first,bldGroup);
  H5Gclose(bldGroup);
}

/** write all epics  data variables to file
 *
 * when there is no epics data to be written return immediately,
 * otherwise write all epics data available to the "Epics Data"
 * group after creating it.
 *
 * @param epics the epics data contained in the event
 * @param groupid the id of the group that the data shoudl be added to
 *
 * @author Lutz Foucar
 */
void writeEpicsData(const MachineData::MachineDataDevice::epicsDataMap_t &epics, hid_t groupid)
{
  if (epics.empty())
    return;
  hid_t epcsGroup (H5Gcreate1(groupid,"Epics Data",0));
  MachineData::MachineDataDevice::epicsDataMap_t::const_iterator epicsIt(epics.begin());
  MachineData::MachineDataDevice::epicsDataMap_t::const_iterator epicsEnd(epics.end());
  for(; epicsIt != epicsEnd; ++epicsIt)
    writeFloatValue(epicsIt->second,epicsIt->first,epcsGroup);
  H5Gclose(epcsGroup);
}

/** write all evr codes of the event to file
 *
 * creates a list of all evr codes that were sent with this event and writes it
 * as an array to the group. When the list of evr codes is empty, don't write
 * anything.
 *
 * @param bld the device from the CASSEvent that contains the machine data
 * @param groupid the id of the group that the data shoudl be added to
 *
 * @author Lutz Foucar
 */
void writeEvrCodes(const MachineData::MachineDataDevice::evrStatus_t &evr, hid_t groupid)
{
  vector<int> list;
  for(size_t i(0);i < evr.size(); ++i)
    if (evr[i])
      list.push_back(i);
  if(list.empty())
  {
    Log::add(Log::WARNING,"writeEvrCodes(): The list of EVR Codes is empty. Nothing will be written");
  }
  else
  {
    hsize_t dims[2];
    dims[0] = list.size();
    dims[1] = 1;
    hid_t dataspace_id(H5Screate_simple(2, dims, NULL));

    hid_t dataset_id(H5Dcreate1(groupid, "EventCodes",
                                H5T_NATIVE_INT, dataspace_id, H5P_DEFAULT));
    H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
             H5P_DEFAULT, &list.front());

    H5Dclose(dataset_id);
    H5Sclose(dataspace_id);
  }
}

}//end namespace hdf5
}//end namespace cass





cass::pp1001::pp1001(cass::PostProcessors &pp, const cass::PostProcessors::key_t &key, const std::string& outfilename)
  :cass::PostprocessorBackend(pp,key),
    _outfilename(outfilename),
    _filehandle(H5Fcreate(_outfilename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT)),
    _events_root_is_filehandle(false)
{
  loadSettings(0);
}

void cass::pp1001::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));
  _store2dAsMatrix = s.value("Write2dAsMatrix",false).toBool();
  _dumpBLD = s.value("WriteBeamlineData",false).toBool();
  _dumpEpics = s.value("WriteEpicsData",false).toBool();
  _dumpEVR = s.value("WriteEvrCodes",false).toBool();
  _dumpMachineData = _dumpBLD || _dumpEpics || _dumpEVR;
  _writeSummary = s.value("WriteSummary",true).toBool();
  _compress = s.value("Compress",false).toBool();
  if (_compress)
  {
    htri_t compavailable (H5Zfilter_avail(H5Z_FILTER_DEFLATE));
    unsigned int filter_info;
    H5Zget_filter_info(H5Z_FILTER_DEFLATE, &filter_info);
    if (!compavailable ||
        !(filter_info & H5Z_FILTER_CONFIG_ENCODE_ENABLED) ||
        !(filter_info & H5Z_FILTER_CONFIG_DECODE_ENABLED))
      _compress = false;
  }
  setupGeneral();
  if (!setupCondition(false))
    return;
  _write = false;
  _hide = true;
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers,true);
  Log::add(Log::INFO,"PostProcessor '"+ _key +
           "' will write all chosen histograms to hdf5 '" + _outfilename +
           "'. Compress the file '"+ (_compress?"true":"false") +
           "'. Write2dAsMatrix '"+ (_store2dAsMatrix?"true":"false") +
           "'. WriteBeamlineData '"+ (_dumpBLD?"true":"false") +
           "'. WriteEpicsData '"+ (_dumpEpics?"true":"false") +
           "'. WriteEvrCodes '"+ (_dumpEVR?"true":"false") +
           "'. WriteSummary '"+ (_writeSummary?"true":"false") +
           "'. Condition is '" + _condition->key());
}

void cass::pp1001::aboutToQuit()
{
  if(!_writeSummary)
    return;
  hid_t grouphandle ( H5Gcreate1(_filehandle, "Summary",0));
  PostProcessors::postprocessors_t &ppc(_pp.postprocessors());
  PostProcessors::postprocessors_t::iterator it (ppc.begin());
  for (;it != ppc.end(); ++it)
  {
    PostprocessorBackend &pp (*(it->second));
    if (pp.write_summary())
    {
      hid_t ppgrouphandle (H5Gcreate1(grouphandle, pp.key().c_str(),0));
      const HistogramBackend &hist (pp.getHist(0));
      //write comment//
      hid_t dataspace_id (H5Screate(H5S_SCALAR));
      hid_t datatype (H5Tcopy(H5T_C_S1));
      H5Tset_size(datatype,pp.comment().size()+1);
      hid_t dataset_id (H5Dcreate1(ppgrouphandle, "Comment", datatype,
                                   dataspace_id, H5P_DEFAULT));
      H5Dwrite(dataset_id, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT,
               pp.comment().c_str());
      H5Dclose(dataset_id);
      H5Sclose(dataspace_id);
      Log::add(Log::DEBUG4, "pp1001::aboutToQuit(): write '" + pp.key() +
               "' which is " + toString(hist.dimension()) +"D");
      switch (hist.dimension())
      {
        case 0:
          hdf5::write0DHist(dynamic_cast<const Histogram0DFloat&>(hist),ppgrouphandle);
          break;
        case 1:
          hdf5::write1DHist(dynamic_cast<const Histogram1DFloat&>(hist),ppgrouphandle,_compress);
          break;
        case 2:
          if (_store2dAsMatrix)
            hdf5::write2DMatrix(dynamic_cast<const Histogram2DFloat&>(hist),ppgrouphandle,_compress);
          else
            hdf5::write2DHist(dynamic_cast<const Histogram2DFloat&>(hist),ppgrouphandle,_compress);
          break;
      }
      Log::add(Log::DEBUG4, "pp1001::aboutToQuit(): done writing '" + pp.key() + "'");
      H5Gclose(ppgrouphandle);
    }
  }
  H5Gclose(grouphandle);

  // close file//
  H5Fflush(_filehandle,H5F_SCOPE_LOCAL);
  H5Fclose(_filehandle);
}

hid_t cass::pp1001::getGroupNameForCalibCycle(const cass::CASSEvent &evt)
{
  if (evt.pvControl.empty())
  {
    _events_root_is_filehandle = true;
    Log::add(Log::DEBUG0,"pp1001::getGroupNameForCalibCycle(): default to root");
    return _filehandle;
  }
  else
  {
    QWriteLocker locker(&_calibGroupLock);
    htri_t result = H5Lexists(_filehandle, evt.pvControl.c_str(), 0);
    if (result == TRUE)
    {
      Log::add(Log::DEBUG0,"pp1001::getGroupNameForCalibCycle():open" +
               evt.pvControl);
      return H5Gopen1(_filehandle, evt.pvControl.c_str());
    }
    else if (result == FALSE)
    {
      Log::add(Log::DEBUG0,"pp1001::getGroupNameForCalibCycle():create" +
               evt.pvControl);
      return H5Gcreate1(_filehandle, evt.pvControl.c_str(), 0);
    }
    else
      throw std::runtime_error("pp1001::getGroupNameForCalibCycle(): should not happen");
  }
}

void cass::pp1001::process(const cass::CASSEvent &evt)
{
  hid_t calibcyclehandle (getGroupNameForCalibCycle(evt));
  hid_t eventgrouphandle (hdf5::createGroupNameFromEventId(evt.id(), calibcyclehandle));
  PostProcessors::postprocessors_t &ppc(_pp.postprocessors());
  PostProcessors::postprocessors_t::iterator it (ppc.begin());
  for (;it != ppc.end(); ++it)
  {
    PostprocessorBackend &pp (*(it->second));
    if (pp.write())
    {
      hid_t ppgrouphandle (H5Gcreate1(eventgrouphandle, pp.key().c_str(),0));
      const HistogramBackend &hist (pp(evt));
      //write comment//
      hid_t dataspace_id (H5Screate(H5S_SCALAR));
      hid_t datatype (H5Tcopy(H5T_C_S1));
      H5Tset_size(datatype,pp.comment().size()+1);
      hid_t dataset_id (H5Dcreate1(ppgrouphandle, "comment", datatype,
                                   dataspace_id, H5P_DEFAULT));
      H5Dwrite(dataset_id, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT,
               pp.comment().c_str());
      H5Dclose(dataset_id);
      H5Sclose(dataspace_id);
      Log::add(Log::DEBUG4, "pp1001::process(): write '" + pp.key() +
               "' which is " + toString(hist.dimension()) +"D");
      switch (hist.dimension())
      {
        case 0:
          hdf5::write0DHist(dynamic_cast<const Histogram0DFloat&>(hist),ppgrouphandle);
          break;
        case 1:
          hdf5::write1DHist(dynamic_cast<const Histogram1DFloat&>(hist),ppgrouphandle,_compress);
          break;
        case 2:
          if (_store2dAsMatrix)
            hdf5::write2DMatrix(dynamic_cast<const Histogram2DFloat&>(hist),ppgrouphandle,_compress);
          else
            hdf5::write2DHist(dynamic_cast<const Histogram2DFloat&>(hist),ppgrouphandle,_compress);
          break;
      }
      Log::add(Log::DEBUG4, "pp1001::process(): done writing '" + pp.key() + "'");
      H5Gclose(ppgrouphandle);
    }
  }
  if(_dumpMachineData)
  {
    hid_t mdatagroup(H5Gcreate1(eventgrouphandle,"MachineData",0));
    const MachineData::MachineDataDevice& mdev(dynamic_cast<const MachineData::MachineDataDevice&>(*(evt.devices().find(CASSEvent::MachineData)->second)));
    if(_dumpBLD)
      hdf5::writeBeamlineData(mdev.BeamlineData(), mdatagroup);
    if(_dumpEpics)
      hdf5::writeEpicsData(mdev.EpicsData(),mdatagroup);
    if(_dumpEVR)
      hdf5::writeEvrCodes(mdev.EvrData(),mdatagroup);
    H5Gclose(mdatagroup);
  }
  H5Gclose(eventgrouphandle);
  if (!_events_root_is_filehandle) H5Gclose(calibcyclehandle);
}


pp1002::pp1002(PostProcessors &pp, const PostProcessors::key_t &key, const string& outfilename)
  : PostprocessorBackend(pp,key),
    _basefilename(outfilename)
{
  loadSettings(0);
}

void pp1002::loadSettings(size_t)
{
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString::fromStdString(_key));
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  const HistogramBackend &hist(_pHist->getHist(0));
  if (hist.dimension() != 2)
    throw invalid_argument("pp1002: The histogram that should be written to hdf5 is not a 2d histogram");
  _compress = settings.value("Compress",true).toBool();
  if (_compress)
  {
    htri_t compavailable (H5Zfilter_avail(H5Z_FILTER_DEFLATE));
    unsigned int filter_info;
    H5Zget_filter_info(H5Z_FILTER_DEFLATE, &filter_info);
    if (!compavailable ||
        !(filter_info & H5Z_FILTER_CONFIG_ENCODE_ENABLED) ||
        !(filter_info & H5Z_FILTER_CONFIG_DECODE_ENABLED))
      _compress = false;
  }
  _write = false;
  _hide = true;
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers,true);
  Log::add(Log::INFO,"PostProcessor '" + _key +
           + "' will write chosen histograms to hdf5 file with '" + _basefilename +
           + "' as basename. Condition is '" + _condition->key() + "'");
}

void pp1002::process(const CASSEvent &evt)
{
  QMutexLocker locker(&_lock);
  const Histogram2DFloat& hist
      (dynamic_cast<const Histogram2DFloat&>((*_pHist)(evt)));
  /** create filename from base filename + event id */
  string filename(_basefilename + "_" + toString(evt.id()) + ".h5");
  /** create the hdf5 file with the name and the handles to the specific data storage*/
  hid_t fh = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  if (fh == 0)
    throw runtime_error("pp1002::process(): Could not open the hdf5 file '" + filename +"'");

  hid_t gid = H5Gcreate1(fh, "data", 0);
  hsize_t dims[2];
  dims[0] = hist.axis()[HistogramBackend::yAxis].nbrBins();
  dims[1] = hist.axis()[HistogramBackend::xAxis].nbrBins();
  hid_t dataspace_id = H5Screate_simple( 2, dims, NULL);
  if (dataspace_id == 0)
    throw runtime_error("pp1002::process(): Could not open the dataspace for the 2d histogram");
  hid_t dataset_id;
  if (_compress)
  {
    // Create dataset creation property list, set the gzip compression filter
    // and chunck size
    hsize_t chunk[2] = {40,3};
    hid_t dcpl (H5Pcreate (H5P_DATASET_CREATE));
    H5Pset_deflate (dcpl, 9);
    H5Pset_chunk (dcpl, 2, chunk);
    dataset_id = (H5Dcreate(gid, "data",
                            H5T_NATIVE_FLOAT, dataspace_id, H5P_DEFAULT, dcpl, H5P_DEFAULT));
  }
  else
    dataset_id = (H5Dcreate1(gid, "data",
                             H5T_NATIVE_FLOAT, dataspace_id, H5P_DEFAULT));
  if (dataset_id == 0 )
    throw runtime_error("pp1002:process(): Could not open dataset for 2d histogram");
  /** write data */
  hist.lock.lockForRead();
  H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
           H5P_DEFAULT, &hist.memory().front());
  hist.lock.unlock();
  /** close hdf5 handles and file */
  H5Dclose(dataset_id);
  H5Sclose(dataspace_id);
  H5Gclose(gid);
  H5Fflush(fh,H5F_SCOPE_LOCAL);
  H5Fclose(fh);
}
