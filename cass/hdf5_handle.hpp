//Copyright (C) 2013 Lutz Foucar

/**
 * @file hdf5_handle.hpp easier api for hdf5 file writing
 *
 * @author Lutz Foucar
 */

#include <vector>
#include <list>
#include <string>
#include <stdexcept>
#include <typeinfo>
#include <utility>

#include <hdf5.h>


namespace hdf5
{

/** traits for matching a build in type with the corresponding h5 type
 *
 * Default @throw logic_error reporting that the type is not supported by the
 * H5 handling.
 */
template <typename T> inline hid_t H5Type()
{
  throw std::logic_error(std::string("H5Type does not exist for '") +
                    typeid(T).name() + "'");
}

/** trait implementation for float */
template <> inline hid_t H5Type<float>() {return H5T_NATIVE_FLOAT;}

/** trait implementation for float */
template <> inline hid_t H5Type<double>() {return H5T_NATIVE_DOUBLE;}

/** trait implementation for float */
template <> inline hid_t H5Type<int>() {return H5T_NATIVE_INT;}

/** trait implementation for float */
template <> inline hid_t H5Type<char>() {return H5T_NATIVE_CHAR;}

/** function for the iterator of the h5 file
 *
 * @param
 */
inline
herr_t iterator_func(hid_t, const char * name, const H5O_info_t *info, void *dlist)
{
  using namespace std;
  list<string>& dsetlist(*reinterpret_cast<list<string>*>(dlist));
  if (info->type == H5O_TYPE_DATASET)
    dsetlist.push_back(name);
  return 0;
}

/** A handler for h5 files
 *
 * @author Lutz Foucar
 */
class Handler
{
public:
  /** default constructor
   *
   */
  Handler() {}

  /** constructor opening the file
   *
   * @param mode Open the file in read "r" or write "w" mode. Default is "w"
   * @param filename the name of the file to open
   */
  Handler(const std::string &filename, const std::string & mode="w")
  {
    open(filename,mode);
  }

  /** destructor
   *
   * flushes and cloeses the file if it is open
   */
  ~Handler()
  {
    H5Fflush(_fileid,H5F_SCOPE_LOCAL);
    H5Fclose(_fileid);
  }

  /** open a file
   *
   * @throw logic_error when file could not be opened
   *
   * @param filename the name of the file to be opened
   * @param mode Open the file in read "r", write "w" or in read write "rw" mode.
   *        Default is "w"
   */
  void open(const std::string &filename, const std::string & mode="w")
  {
    if (mode == "w")
    {
      _fileid = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    }
    else if (mode == "r")
    {
      _fileid = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
    }
    else if (mode == "rw")
    {
      _fileid = H5Fopen(filename.c_str(),H5F_ACC_RDWR, H5P_DEFAULT);
    }
    else
      throw std::invalid_argument("hdf5::Handler::open(): Open mode '" +
                                  mode + "' unknown");
    if (_fileid < 0)
      throw std::invalid_argument("hdf5::Handler::open(): File '" + filename +
                             "' could not be opened");
  }

  /** write an scalar value with a given name as part of a given group
   *
   * create a dataspace and a dataset for writing the scalar value as part of
   * the given group. Then write the value and close all resources later on.
   *
   * @tparam type The scalar type that should be written
   * @param value the value to be written
   * @param valname the name of the value
   */
  template <typename type>
  void writeScalar(const type value, const std::string& valname)
  {
    using namespace std;
    ensureGroupExists(valname);

    hid_t dataspace_id(H5Screate(H5S_SCALAR));
    if (dataspace_id < 0)
      throw runtime_error("writeScalar(float): Could not open the dataspace");

    hid_t dataset_id(H5Dcreate(_fileid, valname.c_str(),H5Type<type>(),
                               dataspace_id, H5P_DEFAULT, H5P_DEFAULT , H5P_DEFAULT));
    if (dataset_id < 0)
      throw runtime_error("writeScalar(float): Could not open the dataset '" + valname +"'");

    herr_t status(H5Dwrite(dataset_id, H5Type<type>(), H5S_ALL, H5S_ALL,
                           H5P_DEFAULT, &value));
    if (status < 0)
      throw runtime_error("writeScalar(): Could not write value");

    H5Dclose(dataset_id);
    H5Sclose(dataspace_id);
  }

  /** read an scalar value with a given name as part of a given group
   *
   * create a dataspace and a dataset for writing the scalar value as part of
   * the given group. Then write the value and close all resources later on.
   *
   * @tparam type The scalar type that should be written
   * @param value the value to be written
   * @param valname the name of the value
   */
  template <typename type>
  type readScalar(const std::string& valname)
  {
    using namespace std;

    /** turn off error output */
    H5Eset_auto(H5E_DEFAULT,0,0);

    /** open the scalar dataset */
    hid_t dataset_id(H5Dopen(_fileid,valname.c_str(),H5P_DEFAULT));
    if (dataset_id < 0)
      throw invalid_argument("readScalar(): Could not open the dataset '" +
                          valname + "'");

    /** read the attribute and close the resources */
    type value;
    herr_t status(H5Dread(dataset_id, H5Type<type>(), H5S_ALL, H5S_ALL, H5P_DEFAULT,
                          &value));
    if (status < 0)
      throw logic_error("readScalar()): Could read the value '" + valname + "'");

    H5Dclose(dataset_id);

    return value;
  }

  /** write a 1d array with a given name
   *
   * create a dataspace and a dataset for writing the value as part of the given
   * group. Then write the value and close all resources later on.
   *
   * The name can contain the group that the value should be written to
   *
   * @todo check out whether the chunck size can be optimized.
   *
   * @tparam type The type that should be written
   * @param array the array to be written
   * @param arrayLength the length of the array to be written
   * @param valname the name of the value
   * @param compressLevel the compression level of the array
   *
   * @author Lutz Foucar
   */
  template<typename type>
  void writeArray(const std::vector<type> &array, const size_t arrayLength,
                  const std::string& valname, int compressLevel=2)
  {
    using namespace std;
    hsize_t dims[1] = {arrayLength};

    ensureGroupExists(valname);

    /** create space and dataset for storing the graph (1D hist) */
    hid_t dataspace_id = H5Screate_simple(1, dims, NULL);
    if (dataspace_id < 0)
      throw runtime_error("writeArray(): Could not open the dataspace");

    /** set up the chunck size and the deflate options */
    hsize_t chunk[1] = {40};
    hid_t dcpl (H5Pcreate (H5P_DATASET_CREATE));
    H5Pset_deflate (dcpl, compressLevel);
    H5Pset_chunk (dcpl, 1, chunk);
    hid_t dataset_id(H5Dcreate(_fileid, valname.c_str(), H5Type<type>(),
                               dataspace_id, H5P_DEFAULT, dcpl , H5P_DEFAULT));
    if (dataset_id < 0)
      throw runtime_error("writeArray(): Could not open the dataset '"
                          + valname +"'");

    herr_t status(H5Dwrite(dataset_id, H5Type<type>(), H5S_ALL, H5S_ALL,
                           H5P_DEFAULT, &array.front()));
    if (status < 0)
      throw runtime_error("writeArray(): Could not write array");

    H5Dclose(dataset_id);
    H5Sclose(dataspace_id);
  }

  /** read a array with a given name into a linearized array
   *
   * reads a array from the h5 file. The dimensions of the matrix will be
   * returned in the arrayLength and the vector will be resized to fit the
   * data before copying the data into the vector.
   *
   * @tparam type The type that should be read
   * @param[out] array the array that will be read
   * @param[out] arrayLength the length of the array
   * @param[in] valname the name of the value
   */
  template<typename type>
  void readArray(std::vector<type> &array, size_t &arrayLength,
                 const std::string& valname)
  {
    using namespace std;
    hsize_t dims[1];

    /** turn off error output */
    H5Eset_auto(H5E_DEFAULT,0,0);

    hid_t dataset_id(H5Dopen(_fileid, valname.c_str(), H5P_DEFAULT));
    if (dataset_id < 0)
      throw invalid_argument("readArray(): Could not open Dataset '"+ valname +"'");

    hid_t dataspace_id(H5Dget_space (dataset_id));
    if (dataspace_id < 0)
      throw logic_error("readArray(): Could not open the dataspace");

    int ndims(H5Sget_simple_extent_dims (dataspace_id, dims, NULL));
    if (ndims < 0)
      throw logic_error("readMatrix(): Could not read the dimensions");

    arrayLength = dims[0];

    array.resize(arrayLength,0);

    herr_t status(H5Dread(dataset_id, H5Type<type>(), H5S_ALL, H5S_ALL, H5P_DEFAULT,
                          &array.front()));
    if (status < 0 )
      throw logic_error("readArray: Something went wrong reading matrix data");

    H5Sclose(dataspace_id);
    H5Dclose(dataset_id);
  }


  /** write a linearized matrix with a given name
   *
   * create a dataspace and a dataset for writing the matrix as part of the given
   * group. Then write the matrix and close all resources later on.
   *
   * The name can contain the group that the value should be written to
   *
   * @todo check out whether the chunck size can be optimized.
   *
   * @tparam type The type that should be written
   * @param matrix the matrix to be written
   * @param cols the number of columns in the matrix
   * @param rows the number of rows in the matrix
   * @param valname the name of the value
   * @param compressLevel the compression level of the matrix
   */
  template<typename type>
  void writeMatrix(const std::vector<type> &matrix, std::pair<size_t,size_t> shape,
                   const std::string& valname, int compressLevel=2)
  {
    using namespace  std;
    hsize_t dims[2] = {shape.second,shape.first};

    ensureGroupExists(valname);

    /** create space and dataset for storing the graph (1D hist) */
    hid_t dataspace_id = H5Screate_simple(2, dims, NULL);
   if (dataspace_id < 0)
      throw runtime_error("writeMatrix(): Could not open the dataspace");

    hsize_t chunk[2] = {40,3};
    hid_t dcpl (H5Pcreate (H5P_DATASET_CREATE));
    H5Pset_deflate (dcpl, compressLevel);
    H5Pset_chunk (dcpl, 2, chunk);
    hid_t dataset_id = (H5Dcreate(_fileid, valname.c_str(), H5Type<type>(),
                                  dataspace_id, H5P_DEFAULT, dcpl, H5P_DEFAULT));
    if (dataset_id < 0)
      throw runtime_error("writeMatrix(): Could not open the dataset '"
                          + valname +"'");

    herr_t status(H5Dwrite(dataset_id, H5Type<type>(), H5S_ALL, H5S_ALL,
                           H5P_DEFAULT, &matrix.front()));
    if (status < 0)
      throw runtime_error("writeMatrix(): Could not write array");

    H5Dclose(dataset_id);
    H5Sclose(dataspace_id);
  }

  /** read a matrix with a given name into a linearized array
   *
   * reads a matrix from the h5 file. The dimensions of the matrix will be
   * returned in the shape parameter and the vector will be resized to fit the
   * data before copying the data into the vector.
   *
   * @tparam type The type that should be written
   * @param matrix the matrix to be written
   * @param cols the number of columns in the matrix
   * @param rows the number of rows in the matrix
   * @param valname the name of the value
   * @param compressLevel the compression level of the matrix
   */
  template<typename type>
  void readMatrix(std::vector<type> &matrix, std::pair<size_t,size_t> &shape,
                   const std::string& valname)
  {
    using namespace std;
    hsize_t dims[2];

    /** turn off error output */
    H5Eset_auto(H5E_DEFAULT,0,0);

    hid_t dataset_id(H5Dopen (_fileid, valname.c_str(), H5P_DEFAULT));
    if (dataset_id < 0)
      throw invalid_argument("readMatrix(): Could not open Dataset '"+ valname +"'");

    hid_t dataspace_id(H5Dget_space (dataset_id));
    if (dataspace_id < 0)
      throw logic_error("readMatrix(): Could not open the dataspace");

    int ndims(H5Sget_simple_extent_dims (dataspace_id, dims, NULL));
    if (ndims < 0)
      throw logic_error("readMatrix(): Could not read the dimensions");

    shape.first = dims[1];
    shape.second = dims[0];

    matrix.resize(shape.first*shape.second,0);

    herr_t status(H5Dread(dataset_id, H5Type<type>(), H5S_ALL, H5S_ALL, H5P_DEFAULT,
                          &matrix.front()));
    if (status < 0 )
      throw logic_error("readMatrix: Something went wrong reading matrix data");

    H5Dclose(dataset_id);
    H5Sclose(dataspace_id);
  }

  /** write an float scalar attribute with a given name as part of a given dataset
   *
   * @tparam type The type that should be written
   * @param value the value to be written
   * @param valname the name of the value
   * @param dsetName the Name of the Dataset
   */
  template<typename type>
  void writeScalarAttribute(const type value, const std::string& valname,
                            const std::string & dsetName)
  {
    using namespace std;

    /** open the dataset that the attribute should be added to */
    hid_t dataset_id(H5Dopen(_fileid,dsetName.c_str(),H5P_DEFAULT));
    if (dataset_id < 0)
      throw runtime_error("writeScalarAttribute(): Could not open the dataset '" +
                          dsetName + "'");

    /** open the attribute space and attribute of the dataset */
    hid_t attributespace_id(H5Screate(H5S_SCALAR));
    if (attributespace_id < 0)
      throw runtime_error("writeScalarAttribute(): Could not open the dataspace");
    hid_t attribute_id(H5Acreate(dataset_id, valname.c_str(), H5Type<type>(),
                                 attributespace_id, H5P_DEFAULT, H5P_DEFAULT));
    if (attribute_id < 0)
      throw runtime_error("writeScalarAttribute(): Could not open the attribute '"
                          + valname +"'");

    /** write the attribute and close the resources */
    herr_t status(H5Awrite(attribute_id, H5Type<type>(), &value));
    if (status < 0 )
      throw logic_error("writeScalarAttribute: Something went wrong reading matrix data");

    H5Aclose(attribute_id);
    H5Sclose(attributespace_id);
    H5Dclose(dataset_id);
  }

  /** read an scalar attribute with a given name as part of a given dataset
   *
   * @throws invalid_argument when the requested parameter is not present
   *
   * @tparam type The type of the scalar value
   * @return the value of the scalar attribute
   * @param valname the name of the value
   * @param dsetName the Name of the Dataset
   */
  template<typename type>
  type readScalarAttribute(const std::string& valname, const std::string & dsetName)
  {
    using namespace std;

    /** turn off error output */
    H5Eset_auto(H5E_DEFAULT,0,0);

    /** open the dataset that the attribute should be added to */
    hid_t dataset_id(H5Dopen(_fileid,dsetName.c_str(),H5P_DEFAULT));
    if (dataset_id < 0)
      throw invalid_argument("readScalarAttribute(): Could not open the dataset '" +
                          dsetName + "'");

    /** attach to the scalar attribute of the dataset and read it */
    hid_t attribute_id(H5Aopen(dataset_id, valname.c_str(), H5P_DEFAULT));
    if (attribute_id < 0)
      throw invalid_argument("readScalarAttribute(): Could not open the attribute '"
                          + valname +"'");

    /** read the attribute and close the resources */
    type value;
    herr_t status(H5Aread(attribute_id, H5Type<type>(), &value));
    if (status < 0)
      throw logic_error("readScalarAttribute(): Could read the attribute '"
                        + valname +"' of dataset '" + dsetName + "'");

    H5Aclose(attribute_id);
    H5Dclose(dataset_id);

    return value;
  }


  /** get the dimension of a value with a given name
   *
   * retrieve the dataset with the given name, then the dataspace for the
   * dataset. Judge by the type and the number of dimension what the dimensions
   * are.
   *
   * @return the dimension of the value
   * @param valname the name of the value
   */
  size_t dimension(const std::string &valname) const
  {
    using namespace std;
    size_t dimension;
    hid_t dataset_id(H5Dopen (_fileid, valname.c_str(), H5P_DEFAULT));
    if (dataset_id < 0)
      throw invalid_argument("dimension(): Could not open Dataset '"+ valname +"'");

    hid_t dataspace_id(H5Dget_space (dataset_id));
    switch(H5Sget_simple_extent_type(dataspace_id))
    {
    case H5S_SCALAR:
      dimension = 0;
      break;
    case H5S_SIMPLE:
      switch(H5Sget_simple_extent_ndims(dataspace_id))
      {
      case 1:
        dimension = 1;
        break;
      case 2:
        dimension = 2;
        break;
      default:
        throw logic_error("dimension(): Unkown dataspace dimension");
        break;
      }
      break;
    default:
      throw logic_error("dimension(): Unknown dataspace type");
      break;
    }

    H5Dclose(dataset_id);
    H5Sclose(dataspace_id);

    return dimension;
  }

  /** get the list of datasets in the file
   *
   * @return list of strings that point to valid datasets
   */
  std::list<std::string> datasets() const
  {
    using namespace std;
    list<string> dsetlist;
    hid_t status(H5Ovisit(_fileid,H5_INDEX_NAME,H5_ITER_NATIVE,iterator_func,&dsetlist));
    if (status < 0)
      throw logic_error("datasets(): Error when iterating trough the h5 file");

    return dsetlist;
  }

private:
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
  std::string reopenFile(int & filehandle, size_t maxsize,
                         const std::string& currentfilename)
  {
    using namespace std;
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
//        int filenumber = atoi(newfilename.substr(found+3,found+6).c_str());
//        ++filenumber;
//        stringstream ss;
//        ss << currentfilename.substr(0,found+2)
//           <<setw(4)<< setfill('0')<<filenumber
//          << currentfilename.substr(found+6,currentfilename.length());
//        newfilename = ss.str();
      }
      filehandle = H5Fcreate(newfilename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    }
    return newfilename;
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
   * @param groupname the name of the group in absolute path
   */
  bool groupExists(const std::string& groupname)
  {
    H5Eset_auto(H5E_DEFAULT,0,0);
    H5G_info_t dummy;
    return (!(H5Gget_info_by_name(_fileid, groupname.c_str(),&dummy,H5P_DEFAULT) < 0));
  }

  /** make sure that the requested group for the datset exists
   *
   * strip off the datasetname and check if the group exists. If it doesn't
   * create the group once. Do this with each subgroup until the full path
   * is checked.
   *
   * @param name absolute path containing also the datasetname
   */
  void ensureGroupExists(const std::string& name)
  {
    using namespace std;
    string wholename(name);
    /** prepend a '/' if its not there to ensure that it is a absolute path */
    wholename = "/" + name;
    string gname(wholename.substr(0, wholename.find_last_of('/')+1));
    for (int i=0; i < static_cast<int>(gname.length()); ++i)
    {
      if(gname[i] == '/')
      {
        string groupname(gname.substr(0,i));
        if (!groupname.empty() && !groupExists(groupname))
        {
          hid_t gh(H5Gcreate(_fileid, groupname.c_str() ,H5P_DEFAULT,
                             H5P_DEFAULT, H5P_DEFAULT));
          H5Gclose(gh);
        }
      }
    }
  }


private:
  /** the file handle */
  hid_t _fileid;
};
}//end namespace hdf5
