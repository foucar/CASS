//Copyright (C) 2008-2011 Lutz Foucar

/**
 * @file achimsorter_hex.cpp file contains class that uses achims resort routine
 *
 * @author Lutz Foucar
 */

#include <stdexcept>
#include <sstream>
#include <algorithm>

#include "achimsorter_hex.h"

#include "libResort64c/resort64c.h"

using namespace cass::ACQIRIS;
using namespace std;

namespace cass
{
  namespace ACQIRIS
  {
    namespace AchimHex
    {
      /** extract times from signal producer
       *
       * extract the time values from the signal producer and puts it into
       * the corrosponding tdc like array
       *
       * @param _signal container for tdc like array mapped to the corrosponding
       *                signalproducer.
       *
       * @author Lutz Foucar
       */
      void extactTimes(pair<SignalProducer*,vector<double> > & thePair)
      {
        SignalProducer::signals_t &sigs(thePair.first->output());
        vector<double> &tdcarray(thePair.second);
        SignalProducer::signals_t::const_iterator sigsIt(sigs.begin());
        for (;sigsIt !=sigs.end(); ++sigsIt)
          tdcarray.push_back((*sigsIt)["time"]);
      }
    }
  }
}

HexSorter::HexSorter()
  :DetectorAnalyzerBackend(),
   _achims_sorter(new sort_class())
{

}

detectorHits_t& HexSorter::operator()(detectorHits_t &hits)
{
  for_each(_signals.begin(),_signals.end(),AchimHex::extactTimes);
//  if (!_signals[0].second.empty())
//    achims_sorter->tdc[achims_sorter->Cu1]  = &u1d[0];
  return hits;
}

void HexSorter::loadSettings(CASSSettings& s, DelaylineDetector &d)
{
  if(!d.isHex())
  {
    stringstream ss;
    ss << "HexSorter::loadSettings: Error The Hex-Sorter cannot work on '"<<d.name()
        << "' which is a Quad Detector.";
    throw invalid_argument(ss.str());
  }
  _signals.clear();
  _signals.push_back(make_pair(&d.mcp(),vector<double>()));
  _signals.push_back(make_pair(&d.layers()['U'].wireends()['1'],vector<double>()));
  _signals.push_back(make_pair(&d.layers()['U'].wireends()['2'],vector<double>()));
  _signals.push_back(make_pair(&d.layers()['V'].wireends()['1'],vector<double>()));
  _signals.push_back(make_pair(&d.layers()['V'].wireends()['2'],vector<double>()));
  _signals.push_back(make_pair(&d.layers()['W'].wireends()['1'],vector<double>()));
  _signals.push_back(make_pair(&d.layers()['W'].wireends()['2'],vector<double>()));
}


//#include <iostream>
//#include <fstream>
//#include <iomanip>
//#include <TMath.h>
//
//#include "MyDetektorHitSorterAchimHex.h"
//
//#include "../MyRootManager/MyHistos.h"
//#include "../MyEvent/MySortedEvent/MyDetektor/MyDetektorInfo.h"
//#include "../MyEvent/MySortedEvent/MyDetektor/MyDetektor.h"
//#include "../MyEvent/MySortedEvent/MyDetektor/MyDetektorHit.h"
//#include "../MyEvent/MySignalAnalyzedEvent/MyPeak/MyPeak.h"
//
////________________________________________Achims Sorter Hex___________________________________________________________________________________________________________________
//MyDetektorHitSorterAchimHex::MyDetektorHitSorterAchimHex(const MyDetektorInfo &di, MyHistos &rm, int HiOff):
//	MyDetektorHitSorterHex(di,rm,HiOff), MyDetektorHitSorterAchim(di)
//{
//	//create all needed histograms here//
//	const double sfu		= di.GetSfU();
//	const double runtime	= di.GetRunTime();
//	const double maxpos_ns	= runtime+30;
//	const double maxpos_mm	= maxpos_ns*sfu;
//	double maxtof			= di.GetMcpProp().GetTimeRangeHigh(0);
//	for (size_t i=0; i< di.GetMcpProp().GetNbrOfTimeRanges(); ++i)
//		if (maxtof < di.GetMcpProp().GetTimeRangeHigh(i))
//			maxtof = di.GetMcpProp().GetTimeRangeHigh(i);
//
//	//timesums//
//	rm.create2d(fHiOff+kSumVsUShift		,"SumVsUShift"	  ,"u [ns]","usum [ns]",300,-maxpos_ns,maxpos_ns,300,-10,+10,Form("%s/Timesums",di.GetName()));
//	rm.create2d(fHiOff+kSumVsVShift		,"SumVsVShift"	  ,"v [ns]","vsum [ns]",300,-maxpos_ns,maxpos_ns,300,-10,+10,Form("%s/Timesums",di.GetName()));
//	rm.create2d(fHiOff+kSumVsWShift		,"SumVsWShift"	  ,"w [ns]","wsum [ns]",300,-maxpos_ns,maxpos_ns,300,-10,+10,Form("%s/Timesums",di.GetName()));
//	rm.create2d(fHiOff+kSumVsUShiftCorr	,"SumVsUShiftCorr","u [ns]","usum [ns]",300,-maxpos_ns,maxpos_ns,300,-10,+10,Form("%s/Timesums",di.GetName()));
//	rm.create2d(fHiOff+kSumVsVShiftCorr	,"SumVsVShiftCorr","v [ns]","vsum [ns]",300,-maxpos_ns,maxpos_ns,300,-10,+10,Form("%s/Timesums",di.GetName()));
//	rm.create2d(fHiOff+kSumVsWShiftCorr	,"SumVsWShiftCorr","w [ns]","wsum [ns]",300,-maxpos_ns,maxpos_ns,300,-10,+10,Form("%s/Timesums",di.GetName()));
//	//det//
//	rm.create2d(fHiOff+kDetShi_ns	,"DetShift_ns"	,"x [ns]","y [ns]",300,-maxpos_ns,maxpos_ns,300,-maxpos_ns,maxpos_ns,Form("%s/DetPictures",di.GetName()));
//	rm.create2d(fHiOff+kDetUVShi_ns ,"DetUVShift_ns","x [ns]","y [ns]",300,-maxpos_ns,maxpos_ns,300,-maxpos_ns,maxpos_ns,Form("%s/DetPictures",di.GetName()));
//	rm.create2d(fHiOff+kDetVWShi_ns ,"DetVWShift_ns","x [ns]","y [ns]",300,-maxpos_ns,maxpos_ns,300,-maxpos_ns,maxpos_ns,Form("%s/DetPictures",di.GetName()));
//	rm.create2d(fHiOff+kDetUWShi_ns ,"DetUWShift_ns","x [ns]","y [ns]",300,-maxpos_ns,maxpos_ns,300,-maxpos_ns,maxpos_ns,Form("%s/DetPictures",di.GetName()));
//	rm.create2d(fHiOff+kDetShi_mm	,"DetShift_mm"	,"x [mm]","y [mm]",300,-maxpos_mm,maxpos_mm,300,-maxpos_mm,maxpos_mm,Form("%s/DetPictures",di.GetName()));
//	rm.create2d(fHiOff+kDetUVShi_mm ,"DetUVShift_mm","x [mm]","y [mm]",300,-maxpos_mm,maxpos_mm,300,-maxpos_mm,maxpos_mm,Form("%s/DetPictures",di.GetName()));
//	rm.create2d(fHiOff+kDetVWShi_mm ,"DetVWShift_mm","x [mm]","y [mm]",300,-maxpos_mm,maxpos_mm,300,-maxpos_mm,maxpos_mm,Form("%s/DetPictures",di.GetName()));
//	rm.create2d(fHiOff+kDetUWShi_mm ,"DetUWShift_mm","x [mm]","y [mm]",300,-maxpos_mm,maxpos_mm,300,-maxpos_mm,maxpos_mm,Form("%s/DetPictures",di.GetName()));
//	//output//
//	rm.create1d(fHiOff+kUsedMethod, "UsedMethod",	"Reconstruction Method Number" ,60,0,30,Form("%s/SorterOutput",di.GetName()));
//	rm.create2d(fHiOff+kDetAll,		"DetAll",		"x [mm]","y [mm]",300,-maxpos_mm,maxpos_mm,300,-maxpos_mm,maxpos_mm,Form("%s/SorterOutput",di.GetName()));
//	rm.create2d(fHiOff+kDetRisky,	"DetRisky",		"x [mm]","y [mm]",300,-maxpos_mm,maxpos_mm,300,-maxpos_mm,maxpos_mm,Form("%s/SorterOutput",di.GetName()));
//	rm.create2d(fHiOff+kDetNonRisky,"DetNonRisky",	"x [mm]","y [mm]",300,-maxpos_mm,maxpos_mm,300,-maxpos_mm,maxpos_mm,Form("%s/SorterOutput",di.GetName()));
//	rm.create1d(fHiOff+kTime,		"Time",			"MCP [ns]",1000,0,maxtof,Form("%s/SorterOutput",di.GetName()));
//	rm.create2d(fHiOff+kPosXVsTime,	"PosXVsTofAll",	"MCP [ns]","x [mm]",500,0,maxtof,300,-maxpos_mm,maxpos_mm,Form("%s/SorterOutput",di.GetName()));
//	rm.create2d(fHiOff+kPosYVsTime,	"PosYVsTofAll",	"MCP [ns]","y [mm]",500,0,maxtof,300,-maxpos_mm,maxpos_mm,Form("%s/SorterOutput",di.GetName()));
//	//calibration//
//	rm.create2d(fHiOff+kNonLinearityMap,"NonlinearityMap","x [mm]","y [mm]",200,-100-0.5,100-0.5,200,-100-0.5,100-0.5,Form("%s/SorterOutput",di.GetName()));
//}
//
//
////___________________________________________________________________________________________________________________________________________________________
//void MyDetektorHitSorterAchimHex::SortImpl(MySignalAnalyzedEvent &sae, MyDetektor &d, MyHistos &rm)
//{
//	ExtractTimes(sae,d);
//	CreateTDCArrays();
//	FillHistosBeforeShift(d,rm);
//	Shift(d);
//	FillHistosAfterShift(d,rm);
//	FillDeadTimeHistos(rm);
//	if(d.ActivateSorter()) SortWithAchimSorter();
//	CreateDetHits(d,rm);
//	FillRatioHistos(d,rm);
//}
//
//
//
//
//
////___________________________________________________________________________________________________________________________________________________________
//void MyDetektorHitSorterAchimHex::CreateTDCArrays()
//{
//	//assign vectors to tdc array//
//	if (u1d.size())fAs->tdc[fAs->Cu1]  = &u1d[0];
//	if (u2d.size())fAs->tdc[fAs->Cu2]  = &u2d[0];
//	if (v1d.size())fAs->tdc[fAs->Cv1]  = &v1d[0];
//	if (v2d.size())fAs->tdc[fAs->Cv2]  = &v2d[0];
//	if (w1d.size())fAs->tdc[fAs->Cw1]  = &w1d[0];
//	if (w2d.size())fAs->tdc[fAs->Cw2]  = &w2d[0];
//	if (mcpd.size())fAs->tdc[fAs->Cmcp] = &mcpd[0];
//
//	//fill the counter array//
//	fCnt[fAs->Cu1] = u1vec.size();
//	fCnt[fAs->Cu2] = u2vec.size();
//	fCnt[fAs->Cv1] = v1vec.size();
//	fCnt[fAs->Cv2] = v2vec.size();
//	fCnt[fAs->Cw1] = w1vec.size();
//	fCnt[fAs->Cw2] = w2vec.size();
//	fCnt[fAs->Cmcp] = mcpvec.size();
//}
//
//
//
//
//
////___________________________________________________________________________________________________________________________________________________________
//void MyDetektorHitSorterAchimHex::CreateDetHits(MyDetektor &d, MyHistos &rm)
//{
//	for (int i=0;i<fNRecHits;++i)
//	{
//		//add a hit to the detektor//
//		MyDetektorHit &hit = d.AddHit();
//
//		//set infos from achims routine//
//		hit.SetXmm(fAs->output_hit_array[i]->x);
//		hit.SetYmm(fAs->output_hit_array[i]->y);
//		hit.SetTime(fAs->output_hit_array[i]->time);
//		hit.SetRekMeth(fAs->output_hit_array[i]->method);
//
//		//set which peaks have been used//
//		if (fAs->output_hit_array[i]->iCu1 != -1) hit.SetU1Nbr(u1vec[fAs->output_hit_array[i]->iCu1]->GetPeakNbr());
//		if (fAs->output_hit_array[i]->iCu2 != -1) hit.SetU2Nbr(u2vec[fAs->output_hit_array[i]->iCu2]->GetPeakNbr());
//		if (fAs->output_hit_array[i]->iCv1 != -1) hit.SetV1Nbr(v1vec[fAs->output_hit_array[i]->iCv1]->GetPeakNbr());
//		if (fAs->output_hit_array[i]->iCv2 != -1) hit.SetV2Nbr(v2vec[fAs->output_hit_array[i]->iCv2]->GetPeakNbr());
//		if (fAs->output_hit_array[i]->iCw1 != -1) hit.SetW1Nbr(w1vec[fAs->output_hit_array[i]->iCw1]->GetPeakNbr());
//		if (fAs->output_hit_array[i]->iCw2 != -1) hit.SetW2Nbr(w2vec[fAs->output_hit_array[i]->iCw2]->GetPeakNbr());
//		if (fAs->output_hit_array[i]->iCmcp!= -1) hit.SetMcpNbr(mcpvec[fAs->output_hit_array[i]->iCmcp]->GetPeakNbr());
//
//		//fill the output histograms//
//		rm.fill1d(fHiOff+kUsedMethod,fAs->output_hit_array[i]->method);
//
//		rm.fill1d(fHiOff+kTime,fAs->output_hit_array[i]->time);
//		rm.fill2d(fHiOff+kPosXVsTime,fAs->output_hit_array[i]->time,fAs->output_hit_array[i]->x);
//		rm.fill2d(fHiOff+kPosYVsTime,fAs->output_hit_array[i]->time,fAs->output_hit_array[i]->y);
//
//		rm.fill2d(fHiOff+kDetAll,fAs->output_hit_array[i]->x,fAs->output_hit_array[i]->y);
//		if(fAs->output_hit_array[i]->method < 15)
//			rm.fill2d(fHiOff+kDetNonRisky,fAs->output_hit_array[i]->x,fAs->output_hit_array[i]->y);
//		if(fAs->output_hit_array[i]->method >= 15)
//			rm.fill2d(fHiOff+kDetRisky,fAs->output_hit_array[i]->x,fAs->output_hit_array[i]->y);
//	}
//}
//
//
//
////___________________________________________________________________________________________________________________________________________________________
//void MyDetektorHitSorterAchimHex::Calibrate(const MyDetektor &d, MyHistos &rm)
//{
//	const double tsU = d.GetTsu();
//	const double tsV = d.GetTsv();
//	const double tsW = d.GetTsw();
//
//	const double u1 = (u1d.size())?u1d[0]:0;
//	const double u2 = (u2d.size())?u2d[0]:0;
//	const double v1 = (v1d.size())?v1d[0]:0;
//	const double v2 = (v2d.size())?v2d[0]:0;
//	const double w1 = (w1d.size())?w1d[0]:0;
//	const double w2 = (w2d.size())?w2d[0]:0;
//	const double mcp = (mcpd.size())?mcpd[0]:0;
//
//	const double u_ns = u1-u2;
//	const double v_ns = v1-v2;
//	const double w_ns = w1-w2;
//
//	const bool csu = (TMath::Abs( u1+u2-2.*mcp) < 10) && u1d.size() && u2d.size() && mcpd.size();
//	const bool csv = (TMath::Abs( v1+v2-2.*mcp) < 10) && v1d.size() && v2d.size() && mcpd.size();
//	const bool csw = (TMath::Abs( w1+w2-2.*mcp) < 10) && w1d.size() && w2d.size() && mcpd.size();
//
//	if (csu && csv && csw)
//	{
//		fSfc->feed_calibration_data(u_ns,v_ns,w_ns,w_ns-d.GetWOffset());
//		rm.fill2d(fHiOff+kNonLinearityMap, fSfc->binx,fSfc->biny, fSfc->detector_map_fill);
//	}
//	fSwc->fill_sum_histograms();
//}
//
//
////___________________________________________________________________________________________________________________________________________________________
//void MyDetektorHitSorterAchimHex::WriteCalibData(const MyDetektorInfo &di)
//{
//	ofstream file;
//	file.open(Form("%s_CalibrationData.txt",di.GetName()));
//	file.fill('0');
//	file <<std::fixed<<std::setprecision(4);
//
//	file<<"-----Calibration Data for "<<di.GetName()<<"--------------"<<std::endl;
//	file<<"-----Add the part that you want to parameter input txt-file-----"<<std::endl;
//	file<<std::endl;
//	file<<std::endl;
//	file<<std::endl;
//
//	//calibrate the scalefactors and the w-offset
//	if (fSfc)
//	{
//		file << "######ScaleFactorsStuff################"<<std::endl;
//		file << std::endl;
//		fSfc->do_auto_calibration(di.GetWOffset());
//		file << di.GetName() << "_ScalefactorU=" << fAs->fu             << std::endl;
//		file << di.GetName() << "_ScalefactorV=" << fSfc->best_fv       << std::endl;
//		file << di.GetName() << "_ScalefactorW=" << fSfc->best_fw       << std::endl;
//		file << di.GetName() << "_WLayerOffset=" << fSfc->best_w_offset << std::endl;
//	}
//	file<<std::endl;
//	file<<std::endl;
//	file<<std::endl;
//
//
//	//calibrate the timesum
//	if (fSwc)
//	{
//		file<<"######TimesumWalkCorrectionStuff################"<<std::endl;
//		file<<std::endl;
//		fSwc->generate_sum_walk_profiles();
//
//		//U-Layer//
//		if (fSwc->sumu_profile)
//		{
//			file<<"#U-Layer"<<std::endl;
//			file<< di.GetName() << "_UNbrOfCorrPts=" <<  fSwc->sumu_profile->number_of_columns << std::endl;
//			for (int binx=0;binx<fSwc->sumu_profile->number_of_columns;++binx)
//			{
//				file << di.GetName() << "_PosU"<<std::setw(3)<<binx<<"="<<fSwc->sumu_profile->get_bin_center_x(static_cast<double>(binx))<< "\t";
//				file << di.GetName() << "_CorU" <<std::setw(3)<<binx<<"="<<fSwc->sumu_profile->get_y(binx)     << std::endl;
//			}
//			file<<std::endl;
//		}
//
//		//V-Layer//
//		if (fSwc->sumv_profile)
//		{
//			file<<"#V-Layer"<<std::endl;
//			file<< di.GetName() << "_VNbrOfCorrPts=" << fSwc->sumv_profile->number_of_columns << std::endl;
//			for (int binx=0;binx<fSwc->sumv_profile->number_of_columns;++binx)
//			{
//				file << di.GetName() << "_PosV"<<std::setw(3)<<binx<<"="<<fSwc->sumv_profile->get_bin_center_x(static_cast<double>(binx))<< "\t";
//				file << di.GetName() << "_CorV" <<std::setw(3)<<binx<<"="<<fSwc->sumv_profile->get_y(binx)    << std::endl;
//			}
//			file<<std::endl;
//		}
//
//		//W-Layer//
//		if (fSwc->sumw_profile)
//		{
//			file<<"#W-Layer"<<std::endl;
//			file<< di.GetName() << "_WNbrOfCorrPts=" << fSwc->sumw_profile->number_of_columns << std::endl;
//			for (int binx=0;binx<fSwc->sumw_profile->number_of_columns;++binx)
//			{
//				file << di.GetName() << "_PosW"<<std::setw(3)<<binx<<"="<<fSwc->sumw_profile->get_bin_center_x(static_cast<double>(binx))<< "\t";
//				file << di.GetName() << "_CorW" <<std::setw(3)<<binx<<"="<<fSwc->sumw_profile->get_y(binx)     << std::endl;
//			}
//			file<<std::endl;
//		}
//	}
//	file<<std::endl;
//	file<<std::endl;
//	file<<std::endl;
//
//}
//
////___________________________________________________________________________________________________________________________________________________________
//void MyDetektorHitSorterAchimHex::Shift(const MyDetektor &d)
//{
//	fAs->shift_sums(-1,d.GetTsu(),d.GetTsv(),d.GetTsw());				// shift all time sums to zero
//	fAs->shift_layer_w(+1,d.GetWOffset());								// shift layer w so that all center lines of the layers meet in one point
//	fAs->shift_position_origin(-1,d.GetDetCenterX(),d.GetDetCenterY());	// shift all layers so that the position picture is centered around X=zero,Y=zero
//}
//
//
//
//
////___________________________________________________________________________________________________________________________________________________________
//void MyDetektorHitSorterAchimHex::FillHistosAfterShift(const MyDetektor &d, MyHistos &rm)
//{
//	//get some infos first//
//	const double tsU = d.GetTsu();
//	const double tsV = d.GetTsv();
//	const double tsW = d.GetTsw();
//
//	const double sfu = d.GetSfU();
//	const double sfv = d.GetSfV();
//	const double sfw = d.GetSfW();
//
//	//get shifted things//
//	const double u1 = (u1d.size())?u1d[0]:0;
//	const double u2 = (u2d.size())?u2d[0]:0;
//	const double v1 = (v1d.size())?v1d[0]:0;
//	const double v2 = (v2d.size())?v2d[0]:0;
//	const double w1 = (w1d.size())?w1d[0]:0;
//	const double w2 = (w2d.size())?w2d[0]:0;
//	const double mcp = (mcpd.size())?mcpd[0]:0;
//
//	//draw shifted Sums//
//	if (u1d.size() || u2d.size() || mcpd.size())
//		rm.fill2d(fHiOff+kSumVsUShift,u1-u2,u1+u2-2.*mcp);
//	if (v1d.size() || v2d.size() || mcpd.size())
//		rm.fill2d(fHiOff+kSumVsVShift,v1-v2,v1+v2-2.*mcp);
//	if (w1d.size() || w2d.size() || mcpd.size())
//		rm.fill2d(fHiOff+kSumVsWShift,w1-w2,w1+w2-2.*mcp);
//
//	//draw shifted and corrected Sums//
//	if (u1d.size() || u2d.size() || mcpd.size())
//		rm.fill2d(fHiOff+kSumVsUShiftCorr,u1-u2,fAs->correct_sum(u1,u2,0)-2.*mcp);
//	if (v1d.size() || v2d.size() || mcpd.size())
//		rm.fill2d(fHiOff+kSumVsVShiftCorr,v1-v2,fAs->correct_sum(v1,v2,1)-2.*mcp);
//	if (w1d.size() || w2d.size() || mcpd.size())
//		rm.fill2d(fHiOff+kSumVsWShiftCorr,w1-w2,fAs->correct_sum(w1,w2,2)-2.*mcp);
//
//	//--calc Pos and Timesum from shifted first hits--//
//	const double u_ns = u1-u2;
//	const double v_ns = v1-v2;
//	const double w_ns = w1-w2;
//
//	//without scalefactors//
//	const double Xuv_ns = u_ns;
//    const double Yuv_ns = 1/TMath::Sqrt(3.) * (u_ns - 2.*v_ns);
//    const double Xuw_ns = Xuv_ns;
//    const double Yuw_ns = 1/TMath::Sqrt(3.) * (2.*w_ns - u_ns);
//	const double Xvw_ns = (v_ns + w_ns);
//    const double Yvw_ns = 1/TMath::Sqrt(3.) * (w_ns - v_ns);
//
//	//with scalefactors//
//	const double Xuv_mm = u_ns * sfu;
//    const double Yuv_mm = 1/TMath::Sqrt(3.) * (u_ns * sfu - 2.*v_ns * sfv);
//    const double Xuw_mm = Xuv_mm;
//    const double Yuw_mm = 1/TMath::Sqrt(3.) * (2.*w_ns * sfw  - u_ns * sfu);
//    const double Xvw_mm = (v_ns * sfv + w_ns * sfw);
//    const double Yvw_mm = 1/TMath::Sqrt(3.) * (w_ns * sfw - v_ns * sfv);
//
//	//check for right timesum//
//	const bool csu = (TMath::Abs( u1+u2-2.*mcp) < 10) && u1d.size() && u2d.size() && mcpd.size();
//	const bool csv = (TMath::Abs( v1+v2-2.*mcp) < 10) && v1d.size() && v2d.size() && mcpd.size();
//	const bool csw = (TMath::Abs( w1+w2-2.*mcp) < 10) && w1d.size() && w2d.size() && mcpd.size();
//
//	if (csu && csv)
//	{
//		rm.fill2d(fHiOff+kDetUVShi_ns,Xuv_ns,Yuv_ns);
//		rm.fill2d(fHiOff+kDetShi_ns  ,Xuv_ns,Yuv_ns);
//
//		rm.fill2d(fHiOff+kDetUVShi_mm,Xuv_mm,Yuv_mm);
//		rm.fill2d(fHiOff+kDetShi_mm  ,Xuv_mm,Yuv_mm);
//	}
//	if (csw && csv)
//	{
//		rm.fill2d(fHiOff+kDetVWShi_ns,Xvw_ns,Yvw_ns);
//		rm.fill2d(fHiOff+kDetShi_ns  ,Xvw_ns,Yvw_ns);
//
//		rm.fill2d(fHiOff+kDetVWShi_mm,Xvw_mm,Yvw_mm);
//		rm.fill2d(fHiOff+kDetShi_mm  ,Xvw_mm,Yvw_mm);
//	}
//	if (csu && csw)
//	{
//		rm.fill2d(fHiOff+kDetUWShi_ns,Xuw_ns,Yuw_ns);
//		rm.fill2d(fHiOff+kDetShi_ns  ,Xuw_ns,Yuw_ns);
//
//		rm.fill2d(fHiOff+kDetUWShi_mm,Xuw_mm,Yuw_mm);
//		rm.fill2d(fHiOff+kDetShi_mm  ,Xuw_mm,Yuw_mm);
//	}
//}
