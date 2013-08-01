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
  QReadLocker lock(&data.lock);
  writeScalar(data.memory().front(),datasetname,filehandle);
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
  QReadLocker lock(&data.lock);
  writeArray(data.memory(),data.axis()[HistogramBackend::xAxis].size(),
      datasetname,filehandle);
  writeScalarAttribute(data.axis()[HistogramBackend::xAxis].lowerLimit(),"xLow",
      datasetname,filehandle);
  writeScalarAttribute(data.axis()[HistogramBackend::xAxis].upperLimit(),"xUp",
      datasetname,filehandle);
}

/** write a 2D value without additional info
 *
 * details
 *
 * @param datasetname Name of the data set including the full absolute group path
 * @param data histogram containing the data to be written
 * @param compresslevel the level of compression to be used
 * @param filehandle the File Handle of the opened hdf5 file
 *
 * @author Lutz Foucar
 */
void writeData(const string& datasetname, const Histogram2DFloat& data, int compresslevel, hid_t filehandle)
{
  QReadLocker lock(&data.lock);
  writeMatrix(data.memory(),data.shape().first,data.shape().second,datasetname,
              filehandle,compresslevel);
  writeScalarAttribute(data.axis()[HistogramBackend::xAxis].lowerLimit(),"xLow",
      datasetname,filehandle);
  writeScalarAttribute(data.axis()[HistogramBackend::xAxis].upperLimit(),"xUp",
      datasetname,filehandle);
  writeScalarAttribute(data.axis()[HistogramBackend::yAxis].lowerLimit(),"yLow",
      datasetname,filehandle);
  writeScalarAttribute(data.axis()[HistogramBackend::yAxis].upperLimit(),"yUp",
      datasetname,filehandle);

//  hid_t dataset_id;
//  if (compress)
//  {
//    // Create dataset creation property list, set the gzip compression filter
//    // and chunck size
//    hsize_t chunk[2] = {40,3};
//    hid_t dcpl (H5Pcreate (H5P_DATASET_CREATE));
//    H5Pset_deflate (dcpl, 9);
//    H5Pset_chunk (dcpl, 2, chunk);
//    dataset_id = (H5Dcreate(filehandle, datasetname.c_str(), H5T_NATIVE_FLOAT,
//                             dataspace_id, H5P_DEFAULT, dcpl, H5P_DEFAULT));
//  }
//  else
//    dataset_id = (H5Dcreate(filehandle, datasetname.c_str(), H5T_NATIVE_FLOAT,
//                            dataspace_id, H5P_DEFAULT,H5P_DEFAULT,H5P_DEFAULT));
//  if (dataset_id == 0 )
//    throw runtime_error("pp1002:process(): Could not open dataset for 2d histogram");
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

  bool compress = s.value("CompressLevel",2).toBool();
  htri_t compavailable (H5Zfilter_avail(H5Z_FILTER_DEFLATE));
  unsigned int filter_info;
  H5Zget_filter_info(H5Z_FILTER_DEFLATE, &filter_info);
  if (!compavailable ||
      !(filter_info & H5Z_FILTER_CONFIG_ENCODE_ENABLED) ||
      !(filter_info & H5Z_FILTER_CONFIG_DECODE_ENABLED))
    throw logic_error("pp1002::loadSettings(): HDF5 library doesn't allow compression. Please use a hdf5 library that allows compression.");

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


