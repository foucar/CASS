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

#include "resorter/resort64c.h"

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
        vector<double> &tdcarray(thePair.second);
        tdcarray.clear();
        SignalProducer::signals_t &sigs(thePair.first->output());
        SignalProducer::signals_t::const_iterator sigsIt(sigs.begin());
        for (;sigsIt !=sigs.end(); ++sigsIt)
          tdcarray.push_back((*sigsIt)["time"]);
      }
    }
  }
}

HexSorter::HexSorter()
  :DetectorAnalyzerBackend(),
   _achims_sorter(new sort_class()),
   _count(7)
{
  _achims_sorter->Cmcp = 0;
  _achims_sorter->Cu1  = 1;
  _achims_sorter->Cu2  = 2;
  _achims_sorter->Cv1  = 3;
  _achims_sorter->Cv2  = 4;
  _achims_sorter->Cw1  = 5;
  _achims_sorter->Cw2  = 6;
  _achims_sorter->count = &_count.front();
  _achims_sorter->TDC_resolution_ns = 0.1;
  _achims_sorter->tdc_array_row_length = 1000;
  _achims_sorter->dont_overwrite_original_data = true;
  _achims_sorter->use_pos_correction = false;
  //this is needed to tell achims routine that we care for our own arrays//
  for (size_t i=0;i<7;++i)
    _achims_sorter->tdc[i] = (double*)(0x1);

  _tsum_calibrator =
      tsumcalibratorPtr_t(sum_walk_calibration_class::new_sum_walk_calibration_class(_achims_sorter.get(),49));
  _scalefactor_calibrator =
      scalefactorcalibratorPtr_t(new scalefactors_calibration_class(true,
                                                                    _achims_sorter->max_runtime*0.78,
                                                                    0,
                                                                    _achims_sorter->fu,
                                                                    _achims_sorter->fv,
                                                                    _achims_sorter->fw));

}

detectorHits_t& HexSorter::operator()(detectorHits_t &hits)
{
  for_each(_signals.begin(),_signals.end(),AchimHex::extactTimes);
  //assign the tdc arrays and copy the number of found signals
  for (size_t i(0); i<7;++i)
  {
    if (!_signals[i].second.empty())
      _achims_sorter->tdc[i]  = &_signals[i].second.front();
    _count[i] = _signals[i].second.size();
  }
  // shift all time sums to zero
  _achims_sorter->shift_sums(-1,_timesums[0],_timesums[1],_timesums[2]);
  // shift layer w so that all center lines of the layers meet in one point
  _achims_sorter->shift_layer_w(+1,_wLayerOffset);
  // shift all layers so that the position picture is centered around X=zero,Y=zero
  _achims_sorter->shift_position_origin(-1,_center.first,_center.second);
  int32_t nbrOfRecHits = _achims_sorter->sort();
  //copy the reconstructed hits to our dethits//
  for (int i(0);i<nbrOfRecHits;++i)
  {
    detectorHit_t hit;
    hit["x"] = _achims_sorter->output_hit_array[i]->x;
    hit["y"] = _achims_sorter->output_hit_array[i]->y;
    hit["t"] = _achims_sorter->output_hit_array[i]->time;
    hit["method"] = _achims_sorter->output_hit_array[i]->method;
    hits.push_back(hit);
  }

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
  _signals.resize(7);
  _signals.push_back(make_pair(&d.mcp(),vector<double>()));
  _signals.push_back(make_pair(&d.layers()['U'].wireends()['1'],vector<double>()));
  _signals.push_back(make_pair(&d.layers()['U'].wireends()['2'],vector<double>()));
  _signals.push_back(make_pair(&d.layers()['V'].wireends()['1'],vector<double>()));
  _signals.push_back(make_pair(&d.layers()['V'].wireends()['2'],vector<double>()));
  _signals.push_back(make_pair(&d.layers()['W'].wireends()['1'],vector<double>()));
  _signals.push_back(make_pair(&d.layers()['W'].wireends()['2'],vector<double>()));

  //	fAs->use_sum_correction					= static_cast<bool>(di.GetNbrSumUCorrPoints);
  //	//set the variables that we get from the detektor//
  //	fAs->uncorrected_time_sum_half_width_u	= di.GetTsuWidth();
  //	fAs->uncorrected_time_sum_half_width_v	= di.GetTsvWidth();
  //	fAs->uncorrected_time_sum_half_width_w	= di.GetTswWidth();
  //	fAs->fu									= di.GetSfU();
  //	fAs->fv									= di.GetSfV();
  //	fAs->fw									= di.GetSfW();
  //	fAs->max_runtime						= di.GetRunTime();
  //	fAs->dead_time_anode					= di.GetDeadTimeAnode();
  //	fAs->dead_time_mcp						= di.GetDeadTimeMCP();
  //	fAs->MCP_radius							= di.GetMCPRadius();
  //	fAs->use_HEX							= di.IsHexAnode();
  //	fAs->use_MCP							= di.UseMCP();
  //	//set the sum walk correction points, Achims Routine will internaly find out how many we gave it//
  //	for (int i=0;i<di.GetNbrSumUCorrPoints();++i)
  //		fAs->sum_corrector_U->set_point(di.GetUCorrPos(i),di.GetUCorrCorr(i));
  //
  //	for (int i=0;i<di.GetNbrSumVCorrPoints();++i)
  //		fAs->sum_corrector_V->set_point(di.GetVCorrPos(i),di.GetVCorrCorr(i));
  //
  //	for (int i=0;i<di.GetNbrSumWCorrPoints();++i)
  //		fAs->sum_corrector_W->set_point(di.GetWCorrPos(i),di.GetWCorrCorr(i));
  //
  //	//init() must be called only once
  //	int error_code = fAs->init();
  //	if (error_code != 0)
  //	{
  //		char error_text[500];
  //		fAs->get_error_text(error_code,500,error_text);
  //		std::cout << "Achims Sorter: "<<error_text<<std::endl;
  //		exit(1);
  //	}
  //	else
  //	{
  //		fAlreadyInitialized=true;
  //		fSwc = sum_walk_calibration_class::new_sum_walk_calibration_class(fAs,49);
  //		fSfc = new scalefactors_calibration_class(true,fAs->max_runtime*0.78,fAs->fu,fAs->fv,fAs->fw);
  //	}

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
//
//
//
//
//
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
