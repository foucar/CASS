//Copyright (C) 2010 Lutz Foucar

/**
 * @file hdf5_converter.cpp definition of pp1001 (hdf5_converter)
 *
 * @author Lutz Foucar
 */

#include <QtCore/QDateTime>

#include <hdf5.h>
#include <stdint.h>
#include <sstream>
#include <iomanip>

#include "hdf5_converter.h"
#include "histogram.h"
#include "cass_event.h"
#include "cass_settings.h"
#include "machine_device.h"
#include "log.h"
#include "convenience_functions.h"

using namespace cass;
using namespace std;

namespace cass
{

namespace hdf5
{

/** check filesize and open new file if too big
 *
 * check if the current size of the h5 file is bigger than the
 * user set maximum file size. When this is the case, close the
 * current file and open a new file with the same file name, but
 * with an increasing extension.
 *
 * @return new filename
 * @param filehandle the filehandle to the hdf5 file
 * @param maxsize the maximum size of the file before a new file
 *                is opened
 * @param currentfilename the name of the current hdf5 file
 *
 * @author Lutz Foucar
 */
string reopenFile(int & filehandle, size_t maxsize, const string& currentfilename)
{
  hsize_t currentsize;
  H5Fget_filesize(filehandle,&currentsize);
  string newfilename(currentfilename);
  if (maxsize < currentsize)
  {
    H5Fflush(filehandle,H5F_SCOPE_LOCAL);
    H5Fclose(filehandle);

    size_t found =  newfilename.rfind("__");
    if (found == string::npos)
    {
      newfilename.insert(newfilename.find_last_of("."),"__0001");
    }
    else
    {
      int filenumber = atoi(newfilename.substr(found+2,found+6).c_str());
      ++filenumber;
      stringstream ss;
      ss << currentfilename.substr(0,found+2)
         <<setw(4)<< setfill('0')<<filenumber
         << currentfilename.substr(found+6,currentfilename.length());
      newfilename = ss.str();
    }
    filehandle = H5Fcreate(newfilename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    Log::add(Log::INFO,"Maximum filesize is exceeded. Close current file '" + currentfilename +
             "' and open the new file '" + newfilename + "'");
  }
  return newfilename;
}

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
 * @param calibcycle the current calibcycle string
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
  Log::add(Log::VERBOSEINFO,"createGroupNameFromEventId(): creating group: " +
           groupname.str());
  return H5Gcreate1(calibcycle, groupname.str().c_str(),0);
}

/** write an float scalar value with a given name as part of a given group
 *
 * create a dataspace and a dataset for writing the value as part of the given
 * group. Then write the value and close all resources later on.
 *
 * @todo find out whether it is possible to use scalar instead of 2d value.
 *
 * @param value the value to be written
 * @param valname the name of the value
 * @param groupid the id of the group that the value should be part of
 *
 * @author Lutz Foucar
 */
void writeScalar(const float value, const string& valname, hid_t groupid)
{
  hid_t dataspace_id(H5Screate(H5S_SCALAR));
  if (dataspace_id < 0)
    throw runtime_error("writeScalar(float): Could not open the dataspace");

  hid_t dataset_id(H5Dcreate(groupid, valname.c_str(), H5T_NATIVE_FLOAT,
                             dataspace_id, H5P_DEFAULT, H5P_DEFAULT , H5P_DEFAULT));
  if (dataset_id < 0)
    throw runtime_error("writeScalar(float): Could not open the dataset '" + valname +"'");

  H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
           H5P_DEFAULT, &value);

  H5Dclose(dataset_id);
  H5Sclose(dataspace_id);
}

/** write an float 1d arry with a given name as part of a given group
 *
 * create a dataspace and a dataset for writing the value as part of the given
 * group. Then write the value and close all resources later on.
 *
 * @todo find out whether it is possible to use scalar instead of 2d value.
 *
 * @param array the array to be written
 * @param arrayLength the length of the array to be written
 * @param valname the name of the value
 * @param groupid the id of the group that the value should be part of
 *
 * @author Lutz Foucar
 */
void writeArray(const vector<float> &array, const size_t arrayLength,
                const string& valname, hid_t groupid)
{
  hsize_t dims[1] = {arrayLength};

  /** create space and dataset for storing the graph (1D hist) */
  hid_t dataspace_id = H5Screate_simple(1, dims, NULL);
  if (dataspace_id < 0)
    throw runtime_error("writeArray(float): Could not open the dataspace");

  hid_t dataset_id(H5Dcreate(groupid, valname.c_str(), H5T_NATIVE_FLOAT,
                             dataspace_id, H5P_DEFAULT, H5P_DEFAULT , H5P_DEFAULT));
  if (dataset_id < 0)
    throw runtime_error("writeArray(float): Could not open the dataset '"
                        + valname +"'");

  H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
           H5P_DEFAULT, &array.front());

  H5Dclose(dataset_id);
  H5Sclose(dataspace_id);
}

/** write a linearized array as 2d matrix with a given name as part of a given group
 *
 * create a dataspace and a dataset for writing the matrix as part of the given
 * group. Then write the matrix and close all resources later on.
 *
 * @todo find out whether it is possible to use scalar instead of 2d value.
 *
 * @param matrix the matrix to be written
 * @param cols the number of columns in the matrix
 * @param rows the number of rows in the matrix
 * @param valname the name of the value
 * @param groupid the id of the group that the value should be part of
 * @param compresslevel the compression level of how much the data should be compressed
 *
 * @author Lutz Foucar
 */
void writeMatrix(const vector<float> &matrix, const size_t cols, const size_t rows,
                const string& valname, hid_t groupid, int compressLevel)
{
  hsize_t dims[2] = {rows,cols};

  /** create space and dataset for storing the graph (1D hist) */
  hid_t dataspace_id = H5Screate_simple(2, dims, NULL);
  if (dataspace_id < 0)
    throw runtime_error("writeMatrix(float): Could not open the dataspace");

  // Create dataset creation property list, set the gzip compression filter
  // and chunck size
  hsize_t chunk[2] = {40,3};
  hid_t dcpl (H5Pcreate (H5P_DATASET_CREATE));
  H5Pset_deflate (dcpl, compressLevel);
  H5Pset_chunk (dcpl, 2, chunk);
  hid_t dataset_id = (H5Dcreate(groupid, valname.c_str(), H5T_NATIVE_FLOAT,
                                dataspace_id, H5P_DEFAULT, dcpl, H5P_DEFAULT));
  if (dataset_id < 0)
    throw runtime_error("writeMatrix(float): Could not open the dataset '"
                        + valname +"'");

  H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
           H5P_DEFAULT, &matrix.front());

  H5Dclose(dataset_id);
  H5Sclose(dataspace_id);
}

/** write an float scalar attribute with a given name as part of a given dataset
 *
 * @param value the value to be written
 * @param valname the name of the value
 * @param dsetName the Name of the Dataset
 * @param fileId the id of the file that contains the dataset
 *
 * @author Lutz Foucar
 */
void writeScalarAttribute(const float value, const string& valname,
                          const string & dsetName, hid_t fileId)
{
  /** open the dataset that the attribute should be added to */
  hid_t dataset_id(H5Dopen(fileId,dsetName.c_str(),H5P_DEFAULT));
  if (dataset_id < 0)
    throw runtime_error("writeScalarAttribute(float): Could not open the dataset '" +
                        dsetName + "'");

  /** open the attribute space and attribute of the dataset */
  hid_t attributespace_id(H5Screate(H5S_SCALAR));
  if (attributespace_id < 0)
    throw runtime_error("writeScalarAttribute(float): Could not open the dataspace");

  hid_t attribute_id(H5Acreate(dataset_id, valname.c_str(), H5T_NATIVE_FLOAT,
                               attributespace_id, H5P_DEFAULT, H5P_DEFAULT));
  if (attribute_id < 0)
    throw runtime_error("writeScalarAttribute(float): Could not open the attribute '"
                        + valname +"'");

  /** write the attribute and close the resources */
  H5Awrite(attribute_id, H5T_NATIVE_FLOAT, &value);

  H5Aclose(attribute_id);
  H5Sclose(attributespace_id);
  H5Dclose(dataset_id);
}


/** write the axis properties to the chosen group
 *
 * details
 *
 * @param axis the axis that contains the info to be written
 * @param groupid the id of the group that the information should be written to
 *
 * @author Lutz Foucar
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
 * @author Lutz Foucar
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
 * @param compress flag whether data should be compressed
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
 * @param compress flag whether data should be compressed
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
 * @param compress flag whether data should be compressed
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
  writeScalar(UpperLeft,"UpperLeft",groupid);

  const float UpperMiddle (data[maxSize+HistogramBackend::UpperMiddle]);
  writeScalar(UpperMiddle,"UpperMiddle",groupid);

  const float UpperRight (data[maxSize+HistogramBackend::UpperRight]);
  writeScalar(UpperRight,"UpperRight",groupid);

  const float Left (data[maxSize+HistogramBackend::Left]);
  writeScalar(Left,"Left",groupid);

  const float Right (data[maxSize+HistogramBackend::Right]);
  writeScalar(Right,"Right",groupid);

  const float LowerLeft (data[maxSize+HistogramBackend::LowerLeft]);
  writeScalar(LowerLeft,"LowerLeft",groupid);

  const float LowerMiddle (data[maxSize+HistogramBackend::LowerMiddle]);
  writeScalar(LowerMiddle,"LowerMiddle",groupid);

  const float LowerRight (data[maxSize+HistogramBackend::LowerRight]);
  writeScalar(LowerRight,"LowerRight",groupid);

  writeHistProperties(hist,groupid);

  // Create the data space for the dataset.
  /** @todo x and y could be wrong and needs to be interchanged -> check */
  hsize_t dims[2];
  dims[0] = nybins;
  dims[1] = nxbins;
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
    writeScalar(bldIt->second,bldIt->first,bldGroup);
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
    writeScalar(epicsIt->second,epicsIt->first,epcsGroup);
  H5Gclose(epcsGroup);
}

/** write all evr codes of the event to file
 *
 * creates a list of all evr codes that were sent with this event and writes it
 * as an array to the group. When the list of evr codes is empty, don't write
 * anything.
 *
 * @param evr the device from the CASSEvent that contains the machine data
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

/** check if a groups exists
 *
 * for now just checks if an absolute path exists in the file. Need to turn
 * off error output, since the command will issue an error when the group does
 * not exist.
 *
 * @todo iterate through everthing to get rid of the disabling of the error
 *       messaging
 *
 * @return true when the group exists, false otherwise
 * @param name the name of the group in absolute path
 * @param filehandle the File Handle of the opened hdf5 file
 *
 * @author Lutz Foucar
 */
bool checkGroup(const string& name, hid_t filehandle)
{
  H5Eset_auto(H5E_DEFAULT,0,0);
  H5G_info_t dummy;
  return (!(H5Gget_info_by_name(filehandle, name.c_str(),&dummy,H5P_DEFAULT) < 0));
}

/** create a group with aboslute path
 *
 * details
 *
 * @param name of group in absolute path
 * @param filehandle the File Handle of the opened hdf5 file
 *
 * @author Lutz Foucar
 */
void createGroupWithAbsolutePath(const string& name, hid_t filehandle)
{
  QString qname(QString::fromStdString(name));
  if (!qname.startsWith('/'))
    qname.prepend('/');
  int occurences(qname.count('/'));
  for (int i=0; i <= occurences ;++i)
  {
    QString groupname = qname.section('/',0,i);
    if (!groupname.isEmpty() && !checkGroup(groupname.toStdString(),filehandle))
    {
      hid_t gh(H5Gcreate(filehandle, groupname.toStdString().c_str() ,H5P_DEFAULT,
                         H5P_DEFAULT, H5P_DEFAULT));
      H5Gclose(gh);
    }
  }
}


/** write a 0D value without additional info
 *
 * details
 *
 * @param datasetname Name of the data set including the full absolute group path
 * @param data histogram containing the data to be written
 * @param filehandle the File Handle of the opened hdf5 file
 *
 * @author Lutz Foucar
 */
void writeData(const string& datasetname, const Histogram0DFloat& data, hid_t filehandle)
{
  hsize_t dims[2];
  dims[0] = 1;

  hid_t dataspace_id = H5Screate_simple(1, dims, NULL);
  if (dataspace_id == 0)
    throw runtime_error("writeData(): Could not open the dataspace");

  hid_t dataset_id (H5Dcreate(filehandle, datasetname.c_str(), H5T_NATIVE_FLOAT,
                              dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT));
  if (dataset_id == 0)
    throw runtime_error("writeData(): Could not open the dataset");

  data.lock.lockForRead();
  H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
           H5P_DEFAULT, &data.memory().front());
  data.lock.unlock();

  H5Dclose(dataset_id);
  H5Sclose(dataspace_id);
}

/** write a 1D value
 *
 * Write the data in a 1D histogram to a given file. Add the metadata of the
 * Histogram as Attributes of the dataset.
 *
 * @param datasetname Name of the data set including the full absolute group path
 * @param data histogram containing the data to be written
 * @param filehandle the File Handle of the opened hdf5 file
 *
 * @author Lutz Foucar
 */
void writeData(const string& datasetname, const Histogram1DFloat& data, hid_t filehandle)
{
  hsize_t dims[2];
  dims[0] = data.axis()[HistogramBackend::xAxis].nbrBins();
  dims[1] = 1;

  /** create space and dataset for storing the graph (1D hist) */
  hid_t dataspace_id = H5Screate_simple( 2, dims, NULL);
  if (dataspace_id == 0)
    throw runtime_error("writeData(): Could not open the dataspace for the 1d histogram");

  hid_t dataset_id (H5Dcreate(filehandle, datasetname.c_str(), H5T_NATIVE_FLOAT,
                              dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT));
  if (dataset_id == 0)
    throw runtime_error("writeData(): Could not open the dataset");

  /** Create scalar attributes for storing up and low limit of the xaxis */
  hid_t xLowattspace_id(H5Screate(H5S_SCALAR));
  if (xLowattspace_id == 0)
    throw runtime_error("writeData(): Could not open the dataspace for the xlow Attribute");
  hid_t xLowattr_id(H5Acreate(dataset_id, "xLow", H5T_NATIVE_FLOAT,
                              xLowattspace_id, H5P_DEFAULT,H5P_DEFAULT));
  if (xLowattr_id == 0)
    throw runtime_error("writeData(): Could not open the dataspace for the xlow Attribute");

  hid_t xUpattspace_id(H5Screate(H5S_SCALAR));
  if (xUpattspace_id == 0)
    throw runtime_error("writeData(): Could not open the dataspace for the xUp Attribute");
  hid_t xUpattr_id(H5Acreate(dataset_id, "xUp", H5T_NATIVE_FLOAT,
                              xUpattspace_id, H5P_DEFAULT,H5P_DEFAULT));
  if (xUpattr_id == 0)
    throw runtime_error("writeData(): Could not open the dataspace for the xUp Attribute");

  data.lock.lockForRead();
  H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
           H5P_DEFAULT, &data.memory().front());
  float xlow(data.axis()[HistogramBackend::xAxis].lowerLimit());
  H5Awrite(xLowattr_id, H5T_NATIVE_FLOAT, &xlow);
  float xUp(data.axis()[HistogramBackend::xAxis].upperLimit());
  H5Awrite(xUpattr_id, H5T_NATIVE_FLOAT, &xUp);
  data.lock.unlock();

  /** close what has been opened before */
  H5Aclose(xLowattr_id);
  H5Aclose(xUpattr_id);

  H5Sclose(xLowattspace_id);
  H5Sclose(xUpattspace_id);

  H5Dclose(dataset_id);
  H5Sclose(dataspace_id);
}

/** write a 2D value without additional info
 *
 * details
 *
 * @param datasetname Name of the data set including the full absolute group path
 * @param data histogram containing the data to be written
 * @param compress flag whether the data should be compressed
 * @param filehandle the File Handle of the opened hdf5 file
 *
 * @author Lutz Foucar
 */
void writeData(const string& datasetname, const Histogram2DFloat& data, bool compress, hid_t filehandle)
{
  hsize_t dims[2];
  dims[0] = data.axis()[HistogramBackend::yAxis].nbrBins();
  dims[1] = data.axis()[HistogramBackend::xAxis].nbrBins();

  hid_t dataspace_id = H5Screate_simple( 2, dims, NULL);
  if (dataspace_id == 0)
    throw runtime_error("writeData(): Could not open the dataspace for the 2d histogram");

  hid_t dataset_id;
  if (compress)
  {
    // Create dataset creation property list, set the gzip compression filter
    // and chunck size
    hsize_t chunk[2] = {40,3};
    hid_t dcpl (H5Pcreate (H5P_DATASET_CREATE));
    H5Pset_deflate (dcpl, 9);
    H5Pset_chunk (dcpl, 2, chunk);
    dataset_id = (H5Dcreate(filehandle, datasetname.c_str(), H5T_NATIVE_FLOAT,
                             dataspace_id, H5P_DEFAULT, dcpl, H5P_DEFAULT));
  }
  else
    dataset_id = (H5Dcreate(filehandle, datasetname.c_str(), H5T_NATIVE_FLOAT,
                            dataspace_id, H5P_DEFAULT,H5P_DEFAULT,H5P_DEFAULT));
  if (dataset_id == 0 )
    throw runtime_error("pp1002:process(): Could not open dataset for 2d histogram");

  data.lock.lockForRead();
  H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
           H5P_DEFAULT, &data.memory().front());
  data.lock.unlock();

  H5Dclose(dataset_id);
  H5Sclose(dataspace_id);
}


/** write an entity to a h5 file
 *
 * @author Lutz Foucar
 */
class WriteEntry
{
public:
  /** constructor
   *
   * create the hdf5 file with the name and the handles to the specific data
   * storage
   *
   * @param filename the name of the h5 file
   * @param id the id of the event to get the data for
   */
  WriteEntry(const string& filename, const CASSEvent::id_t id=0)
    : _fh(H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT)),
      _id(id)
  {
    if (_fh == 0)
      throw runtime_error("WriteEntity(): Could not open the hdf5 file '" + filename +"'");
  }

  /** destructor
   *
   * flush and close file
   */
  ~WriteEntry()
  {
    H5Fflush(_fh,H5F_SCOPE_LOCAL);
    H5Fclose(_fh);
  }

  /** write an entry to h5 file using the functions defined above
   *
   * @param entry The entry to put into the h5 file
   */
  void operator()(const pp1002::entry_t& entry)
  {
    typedef pp1002::entry_t entry_t;
    const uint32_t &options(entry.options);
    const string &gname(entry.groupname);
    const string &name(entry.name);
    PostProcessor &pp(*entry.pp);

    /** create the requested group and data name*/
    createGroupWithAbsolutePath(gname,_fh);
    const string dataName(gname + "/" + name);

    /** retrieve data from pp and write it to the h5 file */
    const HistogramBackend &data(pp.result(_id));
    switch (data.dimension())
    {
    case 0:
      hdf5::writeData(dataName, dynamic_cast<const Histogram0DFloat&>(data), _fh);
      break;
    case 1:
      hdf5::writeData(dataName, dynamic_cast<const Histogram1DFloat&>(data), _fh);
      break;
    case 2:
      hdf5::writeData(dataName, dynamic_cast<const Histogram2DFloat&>(data), options, _fh);
      break;
    default:
      throw runtime_error("WriteEntity::operator(): data dimension '" +
                          toString(data.dimension()) + "' not known");
      break;
    }
  }

private:
  /** the file handle of the h5 file */
  hid_t _fh;

  /** the eventid to look for */
  CASSEvent::id_t _id;
};

}//end namespace hdf5
}//end namespace cass







//*************** h5 output *************************

pp1002::pp1002(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp1002::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();

  bool compress = s.value("Compress",true).toBool();
  if (compress)
  {
    htri_t compavailable (H5Zfilter_avail(H5Z_FILTER_DEFLATE));
    unsigned int filter_info;
    H5Zget_filter_info(H5Z_FILTER_DEFLATE, &filter_info);
    if (!compavailable ||
        !(filter_info & H5Z_FILTER_CONFIG_ENCODE_ENABLED) ||
        !(filter_info & H5Z_FILTER_CONFIG_DECODE_ENABLED))
      compress = false;
  }

  bool allDepsAreThere(true);
  int size = s.beginReadArray("PostProcessor");
  for (int i = 0; i < size; ++i)
  {
    s.setArrayIndex(i);
    string ppname(s.value("Name","Unknown").toString().toStdString());
    if (ppname == "Unknown")
      continue;
    shared_pointer pp(setupDependency("",ppname));
    allDepsAreThere = pp && allDepsAreThere;
    string groupname(s.value("GroupName","/").toString().toStdString());
    string name = pp ? s.value("ValName",QString::fromStdString(pp->name())).toString().toStdString() : "";
    _ppList.push_back(entry_t(name,groupname,compress,pp));
  }
  s.endArray();

  size = s.beginReadArray("PostProcessorSummary");
  for (int i = 0; i < size; ++i)
  {
    s.setArrayIndex(i);
    string ppname(s.value("Name","Unknown").toString().toStdString());
    if (ppname == "Unknown")
      continue;
    shared_pointer pp(setupDependency("",ppname));
    allDepsAreThere = pp && allDepsAreThere;
    string groupname(s.value("GroupName","/").toString().toStdString());
    string name = pp ? s.value("ValName",QString::fromStdString(pp->name())).toString().toStdString() : "";
    _ppSummaryList.push_back(entry_t(name,groupname,compress,pp));
  }
  s.endArray();

  bool ret (setupCondition());
  if (!(ret && allDepsAreThere))
  {
    _ppList.clear();
    _ppSummaryList.clear();
    return;
  }

  _basefilename = s.value("FileBaseName",QString::fromStdString(_basefilename)).toString().toStdString();

  /** when requested add the first subdir to the filename and make sure that the
   *  directory exists.
   */
  _maxFilePerSubDir = s.value("MaximumNbrFilesPerDir",-1).toInt();
  _filecounter = 0;
  if(_maxFilePerSubDir != -1)
    _basefilename = AlphaCounter::intializeDir(_basefilename);

  _hide = true;
  string output("PostProcessor '" + name() + "' will write histogram ");
  for (list<entry_t>::const_iterator it(_ppList.begin());
       it != _ppList.end(); ++it)
    output += ("'" + it->pp->name() + "' to Group '" + it->groupname +
               "' with dataname '" + it->name +"',");
  output += (" of a hdf5 file with '" + _basefilename +
             "' as basename. 2D File will " + (compress ? "" : "NOT") +
             " be compressed. Files will " + (_maxFilePerSubDir != -1 ? "" : "NOT") +
             " be distributed. Condition is '" + _condition->name() + "'");
  Log::add(Log::INFO,output);
}

const HistogramBackend& pp1002::result(const CASSEvent::id_t)
{
  throw logic_error("pp1002::result: '"+name()+"' should never be called");
}

void pp1002::aboutToQuit()
{
  QMutexLocker locker(&_lock);

  /** check if something to be written */
  if (!_ppSummaryList.empty())
  {
    /** remove subdir from filename when they should be distributed */
    if (_maxFilePerSubDir != -1)
      _basefilename = AlphaCounter::removeAlphaSubdir(_basefilename);

    /** create filename from base filename and write entries to file */
    hdf5::WriteEntry writeEntry(_basefilename + "_Summary.h5");

    /** write all entries to file using the writer
     *
     * @note we can't use for_each here, since we need to ensure that the
     *       entries are written sequentially and for_each can potentially use
     *       omp to parallelize the execution.
     */
    list<entry_t>::const_iterator it(_ppSummaryList.begin());
    list<entry_t>::const_iterator last(_ppSummaryList.end());
    while(it != last)
      writeEntry(*it++);
  }
}

void pp1002::processEvent(const CASSEvent &evt)
{

  if (!_condition->result(evt.id()).isTrue())
    return;

  QMutexLocker locker(&_lock);

  /** check if there is something to be written */
  if (!_ppList.empty())
  {
    /** increment subdir in filename when they should be distributed and the
     *  counter exeeded the maximum amount of files per subdir
     */
    if (_maxFilePerSubDir == _filecounter)
    {
      _filecounter = 0;
      _basefilename = AlphaCounter::increaseCounter(_basefilename);
    }
    ++_filecounter;

    /** create entry writer with filename using basefilename and event id */
    hdf5::WriteEntry writeEntry(_basefilename + "_" + toString(evt.id()) + ".h5",evt.id());

    /** write all entries to file using the writer
     *
     * @note we can't use for_each here, since we need to ensure that the
     *       entries are written sequentially and for_each can potentially use
     *       omp to parallelize the execution.
     */
    list<entry_t>::const_iterator it(_ppList.begin());
    list<entry_t>::const_iterator last(_ppList.end());
    while(it != last)
      writeEntry(*it++);
  }
}


