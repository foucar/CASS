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
#include <iostream>

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
#include "rootfile_helper.h"
#include "log.h"
#include "processor_manager.h"


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
  if (timet)
  {
    QDateTime time;
    time.setTime_t(timet);
    name << time.toString(Qt::ISODate).toStdString() <<"_"<<eventFiducial;
  }
  else
  {
    name << "UnknownTime_"<<eventid;
  }
  Log::add(Log::DEBUG4,"eventIdToDirectoryName(): name: '"+ name.str() + "'");
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
  {
    roothist = new TH1F(casshist.key().c_str(),casshist.key().c_str(),
                        1,0,1);
    roothist->SetBinContent(1,casshist.memory()[0]);
    roothist->SetEntries(casshist.nbrOfFills());
  }
    break;
  case 1:
  {
    /** create root histogram from cass histogram properties */
    const AxisProperty &xaxis(casshist.axis()[HistogramBackend::xAxis]);
    roothist = new TH1F(casshist.key().c_str(),casshist.key().c_str(),
                        xaxis.nbrBins(), xaxis.lowerLimit(), xaxis.upperLimit());
    /** set up axis */
    roothist->GetXaxis()->CenterTitle(true);
    roothist->SetXTitle(xaxis.title().c_str());
    /** copy histogram contents */
    for (size_t iX(0); iX<xaxis.nbrBins();++iX)
      roothist->SetBinContent(roothist->GetBin(iX+1),casshist.memory()[iX]);
    /** copy over / underflow */
    size_t OverUnderFlowStart (xaxis.nbrBins());
    roothist->SetBinContent(roothist->GetBin(0),casshist.memory()[OverUnderFlowStart+HistogramBackend::Underflow]);
    roothist->SetBinContent(roothist->GetBin(xaxis.nbrBins()+1),casshist.memory()[OverUnderFlowStart+HistogramBackend::Overflow]);
    /** copy number of fills (how many shots have been accumulated) */
    roothist->SetEntries(casshist.nbrOfFills());
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
    /** copy histogram contents */
    for (size_t iY(0); iY<yaxis.nbrBins();++iY)
      for (size_t iX(0); iX<xaxis.nbrBins();++iX)
        roothist->SetBinContent(roothist->GetBin(iX+1,iY+1),casshist.memory()[iX + iY*xaxis.nbrBins()]);
    /** copy over / underflow */
    size_t OverUnderFlowStart (xaxis.nbrBins()*yaxis.nbrBins());
    roothist->SetBinContent(roothist->GetBin(0,0),casshist.memory()[OverUnderFlowStart+HistogramBackend::LowerLeft]);
    roothist->SetBinContent(roothist->GetBin(xaxis.nbrBins()+1,0),casshist.memory()[OverUnderFlowStart+HistogramBackend::LowerRight]);
    roothist->SetBinContent(roothist->GetBin(xaxis.nbrBins()+1,yaxis.nbrBins()+1),casshist.memory()[OverUnderFlowStart+HistogramBackend::UpperRight]);
    roothist->SetBinContent(roothist->GetBin(0,yaxis.nbrBins()+1),casshist.memory()[OverUnderFlowStart+HistogramBackend::UpperLeft]);
    roothist->SetBinContent(roothist->GetBin(1,0),casshist.memory()[OverUnderFlowStart+HistogramBackend::LowerMiddle]);
    roothist->SetBinContent(roothist->GetBin(1,yaxis.nbrBins()+1),casshist.memory()[OverUnderFlowStart+HistogramBackend::UpperMiddle]);
    roothist->SetBinContent(roothist->GetBin(xaxis.nbrBins()+1,1),casshist.memory()[OverUnderFlowStart+HistogramBackend::Right]);
    roothist->SetBinContent(roothist->GetBin(0,1),casshist.memory()[OverUnderFlowStart+HistogramBackend::Left]);
    /** copy number of fills (how many shots have been accumulated) */
    roothist->SetEntries(casshist.nbrOfFills());
  }
    break;
  default:
    break;
  }
  /** write the histogram to root file */
  roothist->Write(0,TObject::kOverwrite);
}
}//end namespace root
}//end namespace cass

pp2000::pp2000(const name_t &name, std::string filename)
  : PostProcessor(name),
    _rootfile(ROOTFileHelper::create(filename))
{
  if (!_rootfile)
    throw invalid_argument("pp2000 (" + name + "): '" + filename +
                           "' could not be opened! Maybe deleting the file helps.");
  loadSettings(0);
}

const HistogramBackend& pp2000::result(const CASSEvent::id_t)
{
  throw logic_error("pp2000::result: '"+name()+"' should never be called");
}

void pp2000::loadSettings(size_t)
{
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  if (!setupCondition(false))
    return;
  _hide = true;
  Log::add(Log::INFO,"PostProcessor '" + name() +
           "' will write all cass histograms with the write flag set " +
           "to rootfile '" + _rootfile->GetName() + "'. Condition is '" +
           _condition->name() + "'");
}

void pp2000::aboutToQuit()
{
  Log::add(Log::VERBOSEINFO,"pp2000::aboutToQuit() (" + name() +
           "): Histograms will be written to: '" + _rootfile->GetName() + "'");
  /** create the summary directory and cd into it */
  _rootfile->cd("/");
  _rootfile->mkdir("Summary")->cd();
  /** retrieve postprocessor container */
  const PostProcessors::postprocessors_t& container(PostProcessors::reference().postprocessors());
  /** go through all contents of the container */
  PostProcessors::postprocessors_t::const_iterator it(container.begin());
  for (;it != container.end(); ++it)
  {
    /** check if histograms of postprocessor should be written */
    PostProcessor &pp(*(it->get()));
    if (false)
    {
      /** if so write it to the root file */
      const HistogramBackend &cassbackend(pp.result());
      const HistogramFloatBase &casshist(dynamic_cast<const HistogramFloatBase&>(cassbackend));
      ROOT::copyHistToRootFile(casshist);
    }
  }
  /** go back to original directory and save file */
  _rootfile->cd("/");
  ROOTFileHelper::close(_rootfile);
}

void pp2000::processEvent(const cass::CASSEvent &evt)
{
  if (!_condition->result(evt.id()).isTrue())
    return;
  QMutexLocker locker(&_lock);
  /** create directory from eventId and cd into it */
  _rootfile->cd("/");
  string dirname(ROOT::eventIdToDirectoryName(evt.id()));
  _rootfile->mkdir(dirname.c_str())->cd();
  /** retrieve postprocessor container */
  PostProcessors::postprocessors_t &ppc(PostProcessors::reference().postprocessors());
  /** go through all contents of the container */
  PostProcessors::postprocessors_t::iterator it (ppc.begin());
  for (;it != ppc.end(); ++it)
  {
    PostProcessor &pp (*(it->get()));
    if (false)
    {
      /** if so write it to the root file */
      const HistogramBackend &cassbackend(pp.result(evt.id()));
      const HistogramFloatBase &casshist(dynamic_cast<const HistogramFloatBase&>(cassbackend));
      casshist.lock.lockForRead();
      ROOT::copyHistToRootFile(casshist);
      casshist.lock.unlock();
    }
  }
  /** go back to original directory and save file */
  _rootfile->cd("/");
  _rootfile->SaveSelf();
}

