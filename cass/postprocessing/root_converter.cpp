// Copyright (C) 2010 Lutz Foucar

/**
 * @file root_converter.cpp file contains definition of postprocessor 2000
 *
 * @author Lutz Foucar
 */

#include <sstream>
#include <vector>
#include <stdexcept>

#include <TFile.h>
#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>


#include "root_converter.h"
#include "histogram.h"

using namespace cass;
using namespace cass::ROOT;

namespace cass
{
  /** namespace for ROOT related functions */
  namespace ROOT
  {

  }
}

pp2000::pp2000(PostProcessors& pp, const cass::PostProcessors::key_t &key, std::string filename)
    : PostprocessorBackend(pp, key),
     _rootfile(TFile::Open(filename.c_str(),"RECREATE"))
{
  if (!rootfile)
  {
    stringstream ss;
    ss <<"pp2000 ("<<key<<"): "<<_rootfilename<< " could not be opened, please delete the file!";
    throw invalid_argument(ss.str());
  }
  loadSettings(0);
}

pp2000::~pp2000()
{
  _rootfile->SaveSelf();
  _rootfile->Close();
}

void pp2000::loadSettings(size_t)
{

}

void pp2000::aboutToQuit()
{
  VERBOSEOUT(std::cout << "Histograms will be written to: "<<_rootfilename<<std::endl);
  //create a temporary storage for pointer so that we can delete them later on//
  std::vector<TH1*> tobedeleted;
  //retrieve all active histograms and create according root histogram from them//
  const PostProcessors::postprocessors_t& container(_pp.postprocessors());
  PostProcessors::postprocessors_t::const_iterator it (container.begin());
  for (;it != container.end(); ++it)
  {
    //get our hist//
    HistogramFloatBase *h(it->second->getHist());
    //create hist pointer//
    TH1 * roothist(0);
    //create the histogram according to the dimension of our histogram//
    //fill them with the contents of our histogram//
    switch (h->dimension())
    {
    case 0:
      VERBOSEOUT(std::cout<<"pp2000:destructor: Can't store a 0D Histogram"<<std::endl);
      break;
    case 1:
      roothist = new TH1F(h->axis()[HistogramBackend::xAxis].nbrBins(),
                          h->axis()[HistogramBackend::xAxis].lowerLimit(),
                          h->axis()[HistogramBackend::xAxis].upperLimit());
      float * rootmemory(dynamic_cast<TH1F*>(roothist->GetArray()));
      std::copy(h->memory().begin(), h->memory().end()-2, rootmemory+1);
      roothist->SetEntries(h->nbrOfFills());
      break;
    case 2:
      roothist = new TH2F(h->axis()[HistogramBackend::xAxis].nbrBins(),
                          h->axis()[HistogramBackend::xAxis].lowerLimit(),
                          h->axis()[HistogramBackend::xAxis].upperLimit(),
                          h->axis()[HistogramBackend::yAxis].nbrBins(),
                          h->axis()[HistogramBackend::yAxis].lowerLimit(),
                          h->axis()[HistogramBackend::yAxis].upperLimit());
      float * rootmemory(dynamic_cast<TH1F*>(roothist->GetArray()));
      //because of the stupid memory layout of roothistos we need to copy row by row//
      const size_t nxbins (h->axis()[HistogramBackend::xAxis].nbrBins());
      const size_t nybins (h->axis()[HistogramBackend::yAxis].nbrBins());
      for (size_t row=0; row<nybins; ++row)
        std::copy(h->memory().begin()+row*nxbins,
                  h->memory().begin()+row*nxbins+nxbins,
                  rootmemory+row*(nxbins+2)+1);
      roothist->SetEntries(h->nbrOfFills());
      break;
    default:
      VERBOSEOUT(std::cout<<"pp2000:destructor: Unknown Histogram dimension:"<<h->dimension()<<std::endl);
    }
    //write the histogram to root file//
    roothist->Write(0,TObject::kOverwrite);
    //put pointer to list for deletion later//
    tobedeleted.push_back(roothist);
  }
  //delete all histograms on list//
  for (std::vector<TH1*>::iterator it(tobedeleted.begin()); it != tobedeleted.end(); ++it)
    delete (*it);
  //close rootfile (will automaticly delete it)
  rootfile->Close();

  _pp.histograms_release();
}


