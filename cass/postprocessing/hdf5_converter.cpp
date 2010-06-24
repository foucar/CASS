//Copyright (C) 2010 Lutz Foucar

#include <hdf5.h>
#include <stdint.h>

#include "hdf5_converter.h"
#include "histogram.h"
#include "cass_event.h"

namespace cass
{
  hid_t createGroupNameFromEventId(uint64_t eventid, hid_t filehandler)
  {
    return 0;
  }

  void write0DHist(const Histogram0DFloat& hist, hid_t groupid)
  {

  }

  void write1DHist(const Histogram1DFloat& hist, hid_t groupid)
  {

  }

  void write2DHist(const Histogram2DFloat& hist, hid_t groupid)
  {

  }
}

cass::pp1001::pp1001(cass::PostProcessors &pp, const cass::PostProcessors::key_t &key, const std::string& outfilename)
  :cass::PostprocessorBackend(pp,key),_outfilename(outfilename)
{
  loadSettings(0);
}

void cass::pp1001::loadSettings(size_t)
{
  setupGeneral();
  if (!setupCondition())
    return;
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers,true);
  _filehandle = H5Fcreate(_outfilename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  std::cout <<"PostProcessor "<<_key
      <<" will write all chosen histograms to hdf5 "<<_outfilename
      <<". Condition is"<<_condition->key()
      <<std::endl;
}

void cass::pp1001::process(const cass::CASSEvent &evt)
{
  hid_t eventgrouphandle (createGroupNameFromEventId(evt.id(),_filehandle));
  PostProcessors::postprocessors_t& ppc(_pp.postprocessors());
  PostProcessors::postprocessors_t::iterator it (ppc.begin());
  for (;it != ppc.end(); ++it)
  {
    PostprocessorBackend &pp (*(it->second));
    if (pp.write())
    {
      hid_t ppgrouphandle (H5Gcreate1(eventgrouphandle, pp.key().c_str(),0));
      /** @todo write metadata to group */
      const HistogramBackend &hist (pp(evt));
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
