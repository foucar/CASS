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


using namespace cass;
using namespace std;

namespace cass
{
/** namespace for ROOT related functions */
namespace ROOT
{
/** function to create a human readable directory name from the eventid
 *
 * @param eventid the event id to create the human readable name from
 *
 * @author Lutz Foucar
 */
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

/** create a directory if it does not exist and cd into it
 *
 * @param file the file in which the directory should be created
 * @param dirname the directory name that is realtiv to the current directory
 *
 * @author Lutz Foucar
 */
void changeDir(TFile* file,const string &dirname)
{
  /** find out wether directory exists */
  file->cd("/");
  TDirectory * direc = file->GetDirectory(dirname.c_str());
  /** if not, create it */
  if (!direc)
  {
    string lhs;
    string rhs;
    string tmp = dirname;
    while (1)
    {
      /** if there is no '/' then this is the last sub dir */
      if (tmp.find("/") == string::npos)
      {
        lhs = tmp;
      }
      /** otherwise split the string to lefthandside and righthandside of "/" */
      else
      {
        lhs = tmp.substr(0,tmp.find("/"));
        rhs = tmp.substr(tmp.find("/")+1,tmp.length());
      }

      /** check wether subdir exits */
      direc = gDirectory->GetDirectory(lhs.c_str());
      /** if so, cd into it */
      if (direc)
        gDirectory->cd(lhs.c_str());
      /** otherwise create it and then cd into it */
      else
      {
        direc = gDirectory->mkdir(lhs.c_str());
        gDirectory->cd(lhs.c_str());
      }

      /** when there is no "/" anymore break */
      if (tmp.find("/") == string::npos)
        break;

      /** the new temp is all that is on the right hand side */
      tmp = rhs;
    }
  }
  /** make the requested dir the current global dir */
  direc->cd();
}

/** function that will copy a histogram to file
 *
 * @param casshist the cass histogram that should be written to file
 * @param valname the name of the histogram in the root file
 *
 * @author Lutz Foucar
 */
void copyHistToRootFile(const HistogramFloatBase &casshist,const string &valname)
{
  TH1 *roothist(0);
  switch (casshist.dimension())
  {
  case 0:
  {
    roothist = new TH1F(valname.c_str(),valname.c_str(),
                        1,0,1);
    roothist->SetBinContent(1,casshist.memory()[0]);
    roothist->SetEntries(casshist.nbrOfFills());
  }
    break;
  case 1:
  {
    /** create root histogram from cass histogram properties */
    const AxisProperty &xaxis(casshist.axis()[HistogramBackend::xAxis]);
    roothist = new TH1F(valname.c_str(),valname.c_str(),
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
    roothist = new TH2F(valname.c_str(),valname.c_str(),
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

pp2000::pp2000(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

const HistogramBackend& pp2000::result(const CASSEvent::id_t)
{
  throw logic_error("pp2000::result: '"+name()+"' should never be called");
}

void pp2000::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
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
    _ppList.push_back(entry_t(name,groupname,pp));
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
    _ppSummaryList.push_back(entry_t(name,groupname,pp));
  }
  s.endArray();

  bool ret (setupCondition());
  if (!(ret && allDepsAreThere))
  {
    _ppList.clear();
    _ppSummaryList.clear();
    return;
  }
  _hide = true;

  string filename(s.value("FileName","output.root").toString().toStdString());
  _rootfile= ROOTFileHelper::create(filename);
  if (!_rootfile)
    throw invalid_argument("pp2000 (" + name() + "): '" + filename +
                             "' could not be opened! Maybe deleting the file helps.");
  Log::add(Log::INFO,"PostProcessor '" + name() +
           "' will write all cass histograms with the write flag set " +
           "to rootfile '" + _rootfile->GetName() + "'. Condition is '" +
           _condition->name() + "'");
}

void pp2000::aboutToQuit()
{
  Log::add(Log::VERBOSEINFO,"pp2000::aboutToQuit() (" + name() +
           "): Histograms will be written to: '" + _rootfile->GetName() + "'");

  /** check if something to be written */
  if (!_ppSummaryList.empty())
  {
    /** write all entries to file */
    list<entry_t>::const_iterator it(_ppSummaryList.begin());
    list<entry_t>::const_iterator last(_ppSummaryList.end());
    while(it != last)
    {
      /** create and change into dir given by user then write hist with given
       *  name into the file
       */
      string folder("/Summary/" + it->groupname);
      ROOT::changeDir(_rootfile,folder);
      ROOT::copyHistToRootFile(dynamic_cast<const HistogramFloatBase&>(it->pp->result()),it->name);
      ++it;
    }

    /** go back to original directory and save file */
    _rootfile->cd("/");
    ROOTFileHelper::close(_rootfile);
  }
}

void pp2000::processEvent(const cass::CASSEvent &evt)
{
  if (!_condition->result(evt.id()).isTrue())
    return;

  /** check if there is something to be written */
  if (!_ppList.empty())
  {
    QMutexLocker locker(&_lock);

    /** create the directory name from eventId */
    string dirname(ROOT::eventIdToDirectoryName(evt.id()));

    /** write all entries to file */
    list<entry_t>::const_iterator it(_ppList.begin());
    list<entry_t>::const_iterator last(_ppList.end());
    while(it != last)
    {
      /** create and change into dir given by user then write hist with given
       *  name into the file
       */
      string folder("/" + dirname + "/" + it->groupname);
      ROOT::changeDir(_rootfile,folder);
      ROOT::copyHistToRootFile(dynamic_cast<const HistogramFloatBase&>(it->pp->result(evt.id())),it->name);
    }

    /** go back to original directory and save file */
    _rootfile->cd("/");
    _rootfile->SaveSelf();
  }
}

