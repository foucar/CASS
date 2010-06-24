//Copyright (C) 2010 Lutz Foucar

#include <QtCore/QDateTime>

#include <hdf5.h>
#include <stdint.h>

#include "hdf5_converter.h"
#include "histogram.h"
#include "cass_event.h"

namespace cass
{
  hid_t createGroupNameFromEventId(uint64_t eventid, hid_t filehandler)
  {
    std::stringstream groupname;
    QDateTime time;
    time.setTime_t( (static_cast<uint32_t>(eventid & 0xFFFFFFFF00000000 >> 32) ));
    uint32_t eventFiducial = static_cast<uint32_t>(eventid & 0xFFFFFFFF00000000);
    groupname << time.toString(Qt::ISODate).toStdString() <<" "<<eventFiducial;
    return H5Gcreate1(filehandler, groupname.str().c_str(),0);
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
    H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
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
    H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
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

  void write1DHist(const Histogram1DFloat& hist, hid_t groupid)
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
    dims[0] = 2;
    dims[1] = data.size()-2;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    std::vector<float> table(dims[0]*dims[1]);
    std::vector<float>::iterator tit (table.begin());
    HistogramFloatBase::storage_t::const_iterator dit (data.begin());
    const AxisProperty & xaxis (hist.axis()[HistogramBackend::xAxis]);
    for (size_t ibin(0); ibin<dims[1]; ++ibin)
    {
      *tit++ = xaxis.position(ibin);
      *tit++ = *dit++;
    }

    //Create the dataset.
    dataset_id = (H5Dcreate1(groupid, "HistData",
                             H5T_NATIVE_FLOAT, dataspace_id, H5P_DEFAULT));
    //write data
    H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
             H5P_DEFAULT, &data[0]);
    //End access to the dataset and release resources used by it.
    H5Dclose(dataset_id);
    //Terminate access to the data space.
    H5Sclose(dataspace_id);
  }

  void write2DHist(const Histogram2DFloat& hist, hid_t groupid)
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
    dims[0] = 3;
    dims[1] = data.size()-8;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    std::vector<float> table(dims[0]*dims[1]);
    std::vector<float>::iterator tit (table.begin());
    HistogramFloatBase::storage_t::const_iterator dit (data.begin());
    const AxisProperty & xaxis (hist.axis()[HistogramBackend::xAxis]);
    const AxisProperty & yaxis (hist.axis()[HistogramBackend::yAxis]);
    for (size_t iybin(0); iybin<dims[1]; ++iybin)
    {
      for (size_t ixbin(0); ixbin<dims[1]; ++ixbin)
      {
        *tit++ = xaxis.position(ixbin);
        *tit++ = yaxis.position(iybin);
        *tit++ = *dit++;
      }
    }
    //Create the dataset.
    dataset_id = (H5Dcreate1(groupid, "HistData",
                             H5T_NATIVE_FLOAT, dataspace_id, H5P_DEFAULT));
    //write data
    H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
             H5P_DEFAULT, &data[0]);
    //End access to the dataset and release resources used by it.
    H5Dclose(dataset_id);
    //Terminate access to the data space.
    H5Sclose(dataspace_id);
  }

  void writePostprocessors(PostProcessors::postprocessors_t& ppc,
                           const CASSEvent &evt,
                           hid_t grouphandle)
  {
    PostProcessors::postprocessors_t::iterator it (ppc.begin());
    for (;it != ppc.end(); ++it)
    {
      PostprocessorBackend &pp (*(it->second));
      if (pp.write())
      {
        hid_t ppgrouphandle (H5Gcreate1(grouphandle, pp.key().c_str(),0));
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
        switch (hist.dimension())
        {
        case 0:
          write0DHist(dynamic_cast<const Histogram0DFloat&>(hist),ppgrouphandle);
          break;
        case 1:
          write0DHist(dynamic_cast<const Histogram0DFloat&>(hist),ppgrouphandle);
          break;
        case 2:
          write0DHist(dynamic_cast<const Histogram0DFloat&>(hist),ppgrouphandle);
          break;
        }
        H5Gclose(ppgrouphandle);
      }
    }
  }

}





cass::pp1001::pp1001(cass::PostProcessors &pp, const cass::PostProcessors::key_t &key, const std::string& outfilename)
  :cass::PostprocessorBackend(pp,key),
   _outfilename(outfilename),
   _filehandle(H5Fcreate(_outfilename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT))
{
  loadSettings(0);
}

void cass::pp1001::loadSettings(size_t)
{
  setupGeneral();
  if (!setupCondition(false))
    return;
  _write = false;
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers,true);
  std::cout <<"PostProcessor "<<_key
      <<" will write all chosen histograms to hdf5 "<<_outfilename
      <<". Condition is"<<_condition->key()
      <<std::endl;
}

void cass::pp1001::process(const cass::CASSEvent &evt)
{
  hid_t eventgrouphandle (createGroupNameFromEventId(evt.id(),_filehandle));
  writePostprocessors(_pp.postprocessors(),evt,eventgrouphandle);
  H5Gclose(eventgrouphandle);
}
