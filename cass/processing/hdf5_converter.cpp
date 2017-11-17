//Copyright (C) 2010-2015 Lutz Foucar

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
#include <tr1/functional>

#include "hdf5_converter.h"
#include "result.hpp"
#include "cass_settings.h"
#include "log.h"
#include "convenience_functions.h"
#include "hdf5_handle.hpp"
#include "cass_version.h"

using namespace cass;
using namespace std;
using tr1::bind;
using tr1::function;
using tr1::placeholders::_1;
using tr1::placeholders::_2;

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
   * storage. Add a dataset that describes the cass version with which the file
   * was generated
   *
   *
   * @param filename the name of the h5 file
   * @param id the id of the event to get the data for
   */
  WriteEntry(const string& filename, const CASSEvent::id_t id=0)
    : _fh(filename),
      _id(id),
      _baseGroupname("/")
  {
    _fh.writeString(string("Written with cass version '" + VERSION + "'"),
                    "cass-version");
  }

  /** set the base group name
   *
   * @param name the new base group name
   */
  void setBaseGroup(const string &name)
  {
    _baseGroupname = name;
  }

  /** set the event id
   *
   * @param id the current event id
   */
  virtual void setEventID(const CASSEvent::id_t id)
  {
    _id = id;
  }

  /** retrieve the current file size
   *
   * @return the current file size
   */
  size_t currentFileSize() const
  {
    return _fh.currentFileSize();
  }

  /** write an entry to h5 file using the functions defined above
   *
   * @param entry The entry to put into the h5 file
   */
  virtual void operator()(const pp1002::entry_t& entry)
  {
    const uint32_t &options(entry.options);
    const string &gname(entry.groupname);
    const string &name(entry.name);
    Processor &proc(*entry.proc);

    /** create the requested dataset name */
    const string dataName(_baseGroupname + "/" + gname + "/" + name);

    /** retrieve data from pp and write it to the h5 file */
    const Processor::result_t &data(proc.result(_id));
    QReadLocker lock(&data.lock);
    switch (data.dim())
    {
    case 0:
    {
      _fh.writeScalar(data.front(),dataName);
      break;
    }
    case 1:
    {
      const Processor::result_t::axe_t &xaxis(data.axis(Processor::result_t::xAxis));
      _fh.writeArray(data.storage(), xaxis.nBins, dataName);
      _fh.writeScalarAttribute(xaxis.low, "xLow", dataName);
      _fh.writeScalarAttribute(xaxis.up, "xUp", dataName);
      break;
    }
    case 2:
    {
      const Processor::result_t::axe_t &xaxis(data.axis(Processor::result_t::xAxis));
      const Processor::result_t::axe_t &yaxis(data.axis(Processor::result_t::yAxis));
      _fh.writeMatrix(data.storage(), data.shape(), dataName, options);
      _fh.writeScalarAttribute(xaxis.low, "xLow", dataName);
      _fh.writeScalarAttribute(xaxis.up, "xUp", dataName);
      _fh.writeScalarAttribute(yaxis.low, "yLow", dataName);
      _fh.writeScalarAttribute(yaxis.up, "yUp", dataName);
      break;
    }
    default:
      throw runtime_error("WriteEntry::operator(): data dimension '" +
                          toString(data.dim()) + "' not known");
      break;
    }
  }

protected:
  /** the file handle of the h5 file */
  ::hdf5::Handler _fh;

  /** the eventid to look for */
  CASSEvent::id_t _id;

  /** the base group name */
  string _baseGroupname;
};


/** append an entity to a dataset in h5 file
 *
 * @author Lutz Foucar
 */
class AppendEntry : public WriteEntry
{
public:
  /** constructor
   *
   * create the hdf5 file with the name and the handles to the specific data
   * storage. Add a dataset that describes the cass version with which the file
   * was generated
   *
   * @param filename the name of the h5 file
   * @param id the id of the event to get the data for
   */
  AppendEntry(const string& filename, const CASSEvent::id_t id=0)
    : WriteEntry(filename,id)
  {
    _writeAttributes = bind(&AppendEntry::writeAttib,this,_1,_2);
  }

  /** write an entry to h5 file using the functions defined above
   *
   * @param entry The entry to put into the h5 file
   */
  virtual void operator()(const pp1002::entry_t& entry)
  {
    const uint32_t &options(entry.options);
    const string &gname(entry.groupname);
    const string &name(entry.name);
    Processor &proc(*entry.proc);

    /** create the requested dataset name */
    const string dataName("/" + gname + "/" + name);

    /** retrieve data from pp and write it to the h5 file */
    const Processor::result_t &data(proc.result(_id));
    QReadLocker lock(&data.lock);
    vector<size_t> shape(data.np_shape());
    ::hdf5::shape_t theShape(shape.begin(),shape.end());
    _fh.appendData(data.storage(),theShape,dataName,options);
    _writeAttributes(data,dataName);
  }

  /** add the event id to the event id dataset
   *
   * in addition switch the attribute writer to nothing
   *
   * @param id the new eventid
   */
  virtual void setEventID(CASSEvent::id_t id)
  {
    if (id)
    {
      if (_id)
        _writeAttributes = bind(&AppendEntry::writeNothing,this,_1,_2);
      ::hdf5::shape_t shape(1,1);
      vector<CASSEvent::id_t> evtid(1,_id);
      _fh.appendData(evtid,shape,"eventIds");
    }
    _id = id;
  }

protected:
  /** write the attributes to the dataset
   *
   * @param data reference to the data
   * @param dsetName the name of the dataset that the attributes should be
   *                 written to
   */
  void writeAttib(const Processor::result_t &data, const string &dsetName)
  {
    switch (data.dim())
    {
    case 1:
    {
      const Processor::result_t::axe_t &xaxis(data.axis(Processor::result_t::xAxis));
      _fh.writeScalarAttribute(xaxis.low, "xLow", dsetName);
      _fh.writeScalarAttribute(xaxis.up, "xUp", dsetName);
      break;
    }
    case 2:
    {
      const Processor::result_t::axe_t &xaxis(data.axis(Processor::result_t::xAxis));
      const Processor::result_t::axe_t &yaxis(data.axis(Processor::result_t::yAxis));
      _fh.writeScalarAttribute(xaxis.low, "xLow", dsetName);
      _fh.writeScalarAttribute(xaxis.up, "xUp", dsetName);
      _fh.writeScalarAttribute(yaxis.low, "yLow", dsetName);
      _fh.writeScalarAttribute(yaxis.up, "yUp", dsetName);
      break;
    }
    default:
      throw runtime_error("WriteEntry::operator(): data dimension '" +
                          toString(data.dim()) + "' not known");
      break;
    }
  }

  /** write nothing
   *
   * @param unused unused reference
   * @param unused not used name
   */
  void writeNothing(const Processor::result_t& /* unused */,
                    const string& /* unused */)
  {

  }

private:
  /** function to write the results axis attributes just once to the dataset */
  function <void(const Processor::result_t&,const string&)> _writeAttributes;
};


}//end namespace hdf5
}//end namespace cass







//*************** h5 output *************************

pp1002::pp1002(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp1002::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();

  int compresslevel(s.value("CompressLevel",2).toBool());
  htri_t compavailable (H5Zfilter_avail(H5Z_FILTER_DEFLATE));
  unsigned int filter_info;
  H5Zget_filter_info(H5Z_FILTER_DEFLATE, &filter_info);
  if (!compavailable ||
      !(filter_info & H5Z_FILTER_CONFIG_ENCODE_ENABLED) ||
      !(filter_info & H5Z_FILTER_CONFIG_DECODE_ENABLED))
    throw logic_error("pp1002::loadSettings(): HDF5 library doesn't allow compression. Please use a hdf5 library that allows compression.");

  bool allDepsAreThere(true);
  int size = s.beginReadArray("Processor");
  for (int i = 0; i < size; ++i)
  {
    s.setArrayIndex(i);
    string procname(s.value("Name","Unknown").toString().toStdString());
    if (procname == "Unknown")
      continue;
    shared_pointer proc(setupDependency("",procname));
    allDepsAreThere = proc && allDepsAreThere;
    string groupname(s.value("GroupName","/").toString().toStdString());
    string name = proc ? s.value("ValName",QString::fromStdString(proc->name())).toString().toStdString() : "";
    _procList.push_back(entry_t(name,groupname,compresslevel,proc));
  }
  s.endArray();

  size = s.beginReadArray("ProcessorSummary");
  for (int i = 0; i < size; ++i)
  {
    s.setArrayIndex(i);
    string procname(s.value("Name","Unknown").toString().toStdString());
    if (procname == "Unknown")
      continue;
    shared_pointer proc(setupDependency("",procname));
    allDepsAreThere = proc && allDepsAreThere;
    string groupname(s.value("GroupName","/").toString().toStdString());
    string name = proc ? s.value("ValName",QString::fromStdString(proc->name())).toString().toStdString() : "";
    _procSummaryList.push_back(entry_t(name,groupname,compresslevel,proc));
  }
  s.endArray();

  bool ret (setupCondition());
  if (!(ret && allDepsAreThere))
  {
    _procList.clear();
    _procSummaryList.clear();
    return;
  }

  _basefilename = s.value("FileBaseName",QString::fromStdString(_basefilename)).toString().toStdString();

  /** when requested add the first subdir to the filename and make sure that the
   *  directory exists.
   */
  _maxFilePerSubDir = s.value("MaximumNbrFilesPerDir",-1).toInt();
  _filecounter = 0;

  /** set up the maximum file size and convert to Bytes */
  _maxFileSize = s.value("MaximumFileSize_GB",200).toUInt();
  _maxFileSize *= 1024*1024*1024;

  /** set up which kind of file should be written */
  /** set up the dir or the filename, depending on the case */
  bool multipleevents(s.value("WriteMultipleEventsInOneFile",false).toBool());
  bool singleDataset(s.value("WriteToSingleDatasets",false).toBool());
  if (multipleevents)
  {
    if (singleDataset)
    {
      _writeEvent = bind(&pp1002::appendEventToMultipleEventsFile,this,_1);
      _writeSummary = bind(&pp1002::writeSummaryToMultipleEventsFile,this);
      _basefilename = AlphaCounter::intializeFile(_basefilename);
      _entryWriter = entryWriter_t(new hdf5::AppendEntry(_basefilename));
    }
    else
    {
      _writeEvent = bind(&pp1002::writeEventToMultipleEventsFile,this,_1);
      _writeSummary = bind(&pp1002::writeSummaryToMultipleEventsFile,this);
      _basefilename = AlphaCounter::intializeFile(_basefilename);
      _entryWriter = entryWriter_t(new hdf5::WriteEntry(_basefilename));
    }
  }
  else
  {
    _writeEvent = bind(&pp1002::writeEventToSingleFile,this,_1);
    _writeSummary = bind(&pp1002::writeSummaryToSingleFile,this);
    if(_maxFilePerSubDir != -1)
      _basefilename = AlphaCounter::intializeDir(_basefilename);
  }

  _hide = true;
  string output("Processor '" + name() + "' will write histogram ");
  for (list<entry_t>::const_iterator it(_procList.begin());
       it != _procList.end(); ++it)
    output += ("'" + it->proc->name() + "' to Group '" + it->groupname +
               "' with dataname '" + it->name +"',");
  output += (" of a hdf5 file with '" + _basefilename +
             "' as basename. 2D File will" + (compresslevel ? "" : " NOT") +
             " be compressed. Files will" + (_maxFilePerSubDir != -1 ? "" : " NOT") +
             " be distributed. Events will"+ (multipleevents ? " NOT" : "") +
             " be written to single files. In which case the data of the" +
             " individual processors will" + (singleDataset ? "" : " NOT") +
             " be put into a single dataset. Maximum file size is '" +
             toString(_maxFileSize) + "' bytes. Condition is '" +
             _condition->name() + "'");
  Log::add(Log::INFO,output);
}

const Processor::result_t &pp1002::result(const CASSEvent::id_t)
{
  throw logic_error("pp1002::result: '"+name()+"' should never be called");
}

void pp1002::aboutToQuit()
{
  /** check if something to be written */
  if (_procSummaryList.empty())
    return;

  QMutexLocker locker(&_lock);
  _writeSummary();
}

void pp1002::writeSummaryToSingleFile()
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
  list<entry_t>::const_iterator it(_procSummaryList.begin());
  list<entry_t>::const_iterator last(_procSummaryList.end());
  while(it != last)
    writeEntry(*it++);
}

void pp1002::writeSummaryToMultipleEventsFile()
{
  _entryWriter->setEventID(0);
  _entryWriter->setBaseGroup("Summary");

  /** write all entries to file using the writer
   *
   * @note we can't use for_each here, since we need to ensure that the
   *       entries are written sequentially and for_each can potentially use
   *       omp to parallelize the execution.
   */
  hdf5::WriteEntry &writeEntry(*_entryWriter);
  list<entry_t>::const_iterator it(_procSummaryList.begin());
  list<entry_t>::const_iterator last(_procSummaryList.end());
  while(it != last)
    writeEntry(*it++);
}

void pp1002::processEvent(const CASSEvent &evt)
{
  /** check if there is something to be written or if it should be written */
  if (_procList.empty() || !_condition->result(evt.id()).isTrue())
    return;

  QMutexLocker locker(&_lock);
  _writeEvent(evt);
}

void pp1002::writeEventToSingleFile(const CASSEvent &evt)
{
  /** increment subdir in filename when they should be distributed and the
   *  counter exeeded the maximum amount of files per subdir
   */
  if (_maxFilePerSubDir == _filecounter)
  {
    _filecounter = 0;
    _basefilename = AlphaCounter::increaseDirCounter(_basefilename);
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
  list<entry_t>::const_iterator it(_procList.begin());
  list<entry_t>::const_iterator last(_procList.end());
  while(it != last)
    writeEntry(*it++);
}

void pp1002::writeEventToMultipleEventsFile(const CASSEvent &evt)
{
  /** check the current file size, create a new file with increase ending,
   *  if too big
   */
  if (_entryWriter->currentFileSize() > _maxFileSize)
  {
    _basefilename = AlphaCounter::increaseFileCounter(_basefilename);
    _entryWriter = entryWriter_t(new hdf5::WriteEntry(_basefilename));
  }

  /** tell the writer which id to use and the corresponding base group */
  _entryWriter->setEventID(evt.id());
  _entryWriter->setBaseGroup(toString(evt.id()));

  /** write all entries to file using the writer
   *
   * @note we can't use for_each here, since we need to ensure that the
   *       entries are written sequentially and for_each can potentially use
   *       omp to parallelize the execution.
   */
  hdf5::WriteEntry &writeEntry(*_entryWriter);
  list<entry_t>::const_iterator it(_procList.begin());
  list<entry_t>::const_iterator last(_procList.end());
  while(it != last)
    writeEntry(*it++);
}

void pp1002::appendEventToMultipleEventsFile(const CASSEvent &evt)
{
  /** check the current file size, create a new file with increase ending,
   *  if too big
   */
  if (_entryWriter->currentFileSize() > _maxFileSize)
  {
    _basefilename = AlphaCounter::increaseFileCounter(_basefilename);
    _entryWriter = entryWriter_t(new hdf5::AppendEntry(_basefilename));
  }

  /** tell the writer which id to use and the corresponding base group */
  _entryWriter->setEventID(evt.id());

  /** write all entries to file using the writer
   *
   * @note we can't use for_each here, since we need to ensure that the
   *       entries are written sequentially and for_each can potentially use
   *       omp to parallelize the execution.
   */
  list<entry_t>::const_iterator it(_procList.begin());
  list<entry_t>::const_iterator last(_procList.end());
  while(it != last)
    (*_entryWriter)(*it++);
}


