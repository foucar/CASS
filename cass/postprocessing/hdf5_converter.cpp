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

using namespace cass;
using namespace std;

namespace cass
{
  hid_t createGroupNameFromEventId(uint64_t eventid, hid_t calibcycle)
  {
    uint32_t timet(static_cast<uint32_t>((eventid & 0xFFFFFFFF00000000) >> 32));
    uint32_t eventFiducial = static_cast<uint32_t>((eventid & 0x00000000FFFFFFFF) >> 8);
    std::stringstream groupname;
    QDateTime time;
    /** @todo make sure that it will be always converted to the timezone in
     *        stanford otherwise people get confused. Timezones are not
     *        yet supported in QDateTime
     */
    time.setTime_t(timet);
    if (timet)
      groupname << time.toString(Qt::ISODate).toStdString() <<"_"<<eventFiducial;
    else
      groupname << "UnknownTime_"<<eventid;
    VERBOSEOUT(std::cout<<"createGroupNameFromEventId(): creating group: "<<groupname.str()
               <<std::endl);
    return H5Gcreate1(calibcycle, groupname.str().c_str(),0);
  }

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

}





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
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _compress = settings.value("Compress",false).toBool();
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
  std::cout <<"PostProcessor "<<_key
      <<" will write all chosen histograms to hdf5 "<<_outfilename
      <<". Condition is '"<<_condition->key()
      <<"'. Compress the file :"<<std::boolalpha << _compress
      <<std::endl;
}

void cass::pp1001::aboutToQuit()
{
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
      VERBOSEOUT(std::cout << "pp1001::aboutToQuit: write '"<<pp.key()
                 << "' which is");
      switch (hist.dimension())
      {
      case 0:
        VERBOSEOUT(std::cout<< " 0D"<<std::endl);
        write0DHist(dynamic_cast<const Histogram0DFloat&>(hist),ppgrouphandle);
        VERBOSEOUT(std::cout<< "pp1001::process: Done writing '"<<pp.key()<<"'"<<std::endl);
        break;
      case 1:
        VERBOSEOUT(std::cout<< " 1D"<<std::endl);
        write1DHist(dynamic_cast<const Histogram1DFloat&>(hist),ppgrouphandle,_compress);
        VERBOSEOUT(std::cout<< "pp1001::process: Done writing '"<<pp.key()<<"'"<<std::endl);
        break;
      case 2:
        VERBOSEOUT(std::cout<< " 2D"<<std::endl);
        write2DHist(dynamic_cast<const Histogram2DFloat&>(hist),ppgrouphandle,_compress);
        VERBOSEOUT(std::cout<< "pp1001::process: Done writing '"<<pp.key()<<"'"<<std::endl);
        break;
      }
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
    VERBOSEOUT(std::cout<<"cass::pp1001::getGroupNameForCalibCycle(): default to root"<<std::endl);
    return _filehandle;
  }
  else 
  {
    QWriteLocker locker(&_calibGroupLock);
    htri_t result = H5Lexists(_filehandle, evt.pvControl.c_str(), 0);
    if (result == TRUE)
    {
      VERBOSEOUT(std::cout<<"cass::pp1001::getGroupNameForCalibCycle(): open "<<evt.pvControl<<std::endl);
      return H5Gopen1(_filehandle, evt.pvControl.c_str());
    }
    else if (result == FALSE)
    {
      VERBOSEOUT(std::cout<<"cass::pp1001::getGroupNameForCalibCycle(): create "<<evt.pvControl<<std::endl);
      return H5Gcreate1(_filehandle, evt.pvControl.c_str(), 0);
    }
    else 
      throw std::runtime_error("should not happen");
  }
}

void cass::pp1001::process(const cass::CASSEvent &evt)
{
  hid_t calibcyclehandle (getGroupNameForCalibCycle(evt)); 
  hid_t eventgrouphandle (createGroupNameFromEventId(evt.id(), calibcyclehandle));
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
      VERBOSEOUT(std::cout << "pp1001::process: write '"<<pp.key()
                 << "' which is");
      switch (hist.dimension())
      {
      case 0:
        VERBOSEOUT(std::cout<< " 0D"<<std::endl);
        write0DHist(dynamic_cast<const Histogram0DFloat&>(hist),ppgrouphandle);
        VERBOSEOUT(std::cout<< "pp1001::process: Done writing '"<<pp.key()<<"'"<<std::endl);
        break;
      case 1:
        VERBOSEOUT(std::cout<< " 1D"<<std::endl);
        write1DHist(dynamic_cast<const Histogram1DFloat&>(hist),ppgrouphandle,_compress);
        VERBOSEOUT(std::cout<< "pp1001::process: Done writing '"<<pp.key()<<"'"<<std::endl);
        break;
      case 2:
        VERBOSEOUT(std::cout<< " 2D"<<std::endl);
        write2DHist(dynamic_cast<const Histogram2DFloat&>(hist),ppgrouphandle,_compress);
        VERBOSEOUT(std::cout<< "pp1001::process: Done writing '"<<pp.key()<<"'"<<std::endl);
        break;
      }
      H5Gclose(ppgrouphandle);
    }
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
  _write = false;
  _hide = true;
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers,true);
  cout <<"PostProcessor '"<<_key
      <<"' will write chosen histograms to hdf5 file with '"<<_basefilename
      <<"' as basename. Condition is '"<<_condition->key()
      <<endl;
}

void pp1002::process(const CASSEvent &evt)
{
  QMutexLocker locker(&_lock);
  const Histogram2DFloat& hist
      (dynamic_cast<const Histogram2DFloat&>((*_pHist)(evt)));
  /** create fielname from base filename + event id */
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
  hid_t dataset_id = (H5Dcreate1(gid, "data",
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
