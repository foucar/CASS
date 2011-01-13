// Copyright (C) 2010 Lutz Foucar

/**
 * @file root_converter.cpp file contains definition of postprocessor 2000
 *
 * @author Lutz Foucar
 */

#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>

#include <TObject.h>
#include <TFile.h>
#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>

#include <QtCore/QDateTime>

#include "root_converter.h"
#include "histogram.h"
#include "cass_settings.h"
#include "cass_event.h"


using namespace cass;
using namespace std;

namespace cass
{
  /** namespace for ROOT related functions */
  namespace ROOT
  {
    string eventIdToDirectoryName(uint64_t eventid)
    {
      uint32_t timet(static_cast<uint32_t>((eventid & 0xFFFFFFFF00000000) >> 32));
      uint32_t eventFiducial = static_cast<uint32_t>((eventid & 0x00000000FFFFFFFF) >> 8);
      stringstream name;
      QDateTime time;
      time.setTime_t(timet);
      name << time.toString(Qt::ISODate).toStdString() <<"_"<<eventFiducial;
      VERBOSEOUT(cout<<"eventIdToDirectoryName(): name: "<<name.str()
                 <<endl);
      return name.str();
    }

    /** function that will copy a histogram to file
     *
     * @param casshist the cass histogram that should be written to file
     *
     * @author Lutz Foucar
     */
    void copyHistToRootFile(const HistogramFloatBase &casshist)
    {
      TH1 *roothist(0);
      switch (casshist.dimension())
      {
      case 0:
        VERBOSEOUT(std::cout<<"pp2000:destructor: Can't store a 0D Histogram"<<std::endl);
        break;
      case 1:
        {
          /** create root histogram from cass histogram properties */
          const AxisProperty &xaxis(casshist.axis()[HistogramBackend::xAxis]);
          roothist = new TH1F(casshist.key().c_str(),casshist.key().c_str(),
                              xaxis.nbrBins(), xaxis.lowerLimit(), xaxis.upperLimit());
          /** copy number of fills (how many shots have been accumulated) */
          roothist->SetEntries(casshist.nbrOfFills());
          /** set up axis */
          roothist->GetXaxis()->CenterTitle(true);
          roothist->SetXTitle(xaxis.title().c_str());
          /** copy over / underflow */
          roothist->SetBinContent(roothist->GetBin(0),casshist.memory()[HistogramBackend::Underflow]);
          roothist->SetBinContent(roothist->GetBin(xaxis.nbrBins()+1),casshist.memory()[HistogramBackend::Overflow]);
          /** copy histogram contents */
          for (size_t iX(0); iX<xaxis.nbrBins();++iX)
            roothist->SetBinContent(roothist->GetBin(iX+1),casshist.memory()[iX]);
        }
        break;
      case 2:
        {
          /** create root histogram from cass histogram properties */
          const AxisProperty &xaxis(casshist.axis()[HistogramBackend::xAxis]);
          const AxisProperty &yaxis(casshist.axis()[HistogramBackend::yAxis]);
          roothist = new TH2F(casshist.key().c_str(),casshist.key().c_str(),
                              xaxis.nbrBins(), xaxis.lowerLimit(), xaxis.upperLimit(),
                              yaxis.nbrBins(), yaxis.lowerLimit(), yaxis.upperLimit());
          /** make sure that the histogram is drawn in color and with color bar */
          roothist->SetOption("colz");
          /** set up axis */
          roothist->SetXTitle(xaxis.title().c_str());
          roothist->GetXaxis()->CenterTitle(true);
          roothist->SetYTitle(yaxis.title().c_str());
          roothist->GetYaxis()->CenterTitle(true);
          roothist->GetYaxis()->SetTitleOffset(1.5);
          /** copy over / underflow */
          roothist->SetBinContent(roothist->GetBin(0,0),casshist.memory()[HistogramBackend::LowerLeft]);
          roothist->SetBinContent(roothist->GetBin(xaxis.nbrBins()+1,0),casshist.memory()[HistogramBackend::LowerRight]);
          roothist->SetBinContent(roothist->GetBin(xaxis.nbrBins()+1,yaxis.nbrBins()+1),casshist.memory()[HistogramBackend::UpperRight]);
          roothist->SetBinContent(roothist->GetBin(0,yaxis.nbrBins()+1),casshist.memory()[HistogramBackend::UpperLeft]);
          roothist->SetBinContent(roothist->GetBin(1,0),casshist.memory()[HistogramBackend::LowerMiddle]);
          roothist->SetBinContent(roothist->GetBin(1,yaxis.nbrBins()+1),casshist.memory()[HistogramBackend::UpperMiddle]);
          roothist->SetBinContent(roothist->GetBin(xaxis.nbrBins()+1,1),casshist.memory()[HistogramBackend::Right]);
          roothist->SetBinContent(roothist->GetBin(0,1),casshist.memory()[HistogramBackend::Left]);
          /** copy number of fills (how many shots have been accumulated) */
          roothist->SetEntries(casshist.nbrOfFills());
          /** copy histogram contents */
          for (size_t iY(0); iY<yaxis.nbrBins();++iY)
            for (size_t iX(0); iX<xaxis.nbrBins();++iX)
              roothist->SetBinContent(roothist->GetBin(iX+1,iY+1),casshist.memory()[iX + iY*xaxis.nbrBins()]);
        }
        break;
      default:
        break;
      }
      /** write the histogram to root file */
      roothist->Write(0,TObject::kOverwrite);
    }
  }
}

pp2000::pp2000(PostProcessors& pp, const cass::PostProcessors::key_t &key, std::string filename)
    : PostprocessorBackend(pp, key),
     _rootfile(TFile::Open(filename.c_str(),"RECREATE"))
{
  if (!_rootfile)
  {
    stringstream ss;
    ss <<"pp2000 ("<<key<<"): "<<_rootfilename<< " could not be opened! Maybe deleting the file helps.";
    throw invalid_argument(ss.str());
  }
  loadSettings(0);
}

void pp2000::loadSettings(size_t)
{
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  setupGeneral();
  if (!setupCondition(false))
    return;
  _write = false;
  _hide = true;
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers,true);
  cout<<endl<<"PostProcessor '"<<_key
      <<"' will write all cass histograms with the write flag set "
      <<"to rootfile '"<<_rootfile->GetName()
      <<"'. Condition is '"<<_condition->key()<<"'"
      <<endl;
}

void pp2000::aboutToQuit()
{
  VERBOSEOUT(cout << "pp2000::aboutToQuit() ("<<key
             <<"): Histograms will be written to: "
             <<_rootfilename
             <<endl);
  /** create the summary directory and cd into it */
  _rootfile->cd("/");
  _rootfile->mkdir("Summary")->cd();
  /** retrieve postprocessor container */
  const PostProcessors::postprocessors_t& container(_pp.postprocessors());
  /** go through all contents of the container */
  PostProcessors::postprocessors_t::const_iterator it(container.begin());
  for (;it != container.end(); ++it)
  {
    /** check if histograms of postprocessor should be written */
    PostprocessorBackend &pp(*(it->second));
    if (pp.write())
    {
      /** if so write it to the root file */
      const HistogramBackend &cassbackend(pp.getHist(0));
      const HistogramFloatBase &casshist(dynamic_cast<const HistogramFloatBase&>(cassbackend));
      ROOT::copyHistToRootFile(casshist);
    }
  }
  /** go back to original directory and save file */
  _rootfile->cd("/");
  _rootfile->SaveSelf();
  _rootfile->Close();
}

void pp2000::process(const cass::CASSEvent &evt)
{
  /** create directory from eventId and cd into it */
  _rootfile->cd("/");
  string dirname(ROOT::eventIdToDirectoryName(evt.id()));
  _rootfile->mkdir(dirname.c_str())->cd();
  /** retrieve postprocessor container */
  PostProcessors::postprocessors_t &ppc(_pp.postprocessors());
  /** go through all contents of the container */
  PostProcessors::postprocessors_t::iterator it (ppc.begin());
  for (;it != ppc.end(); ++it)
  {
    PostprocessorBackend &pp (*(it->second));
    if (pp.write())
    {
      /** if so write it to the root file */
      const HistogramBackend &cassbackend(pp(evt));
      const HistogramFloatBase &casshist(dynamic_cast<const HistogramFloatBase&>(cassbackend));
      ROOT::copyHistToRootFile(casshist);
    }
  }
  /** go back to original directory and save file */
  _rootfile->cd("/");
  _rootfile->SaveSelf();
}

