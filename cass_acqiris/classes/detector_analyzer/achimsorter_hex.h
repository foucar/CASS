#ifndef __MyDetektorHitSorterAchimHex_H_
#define __MyDetektorHitSorterAchimHex_H_

#include "MyDetektorHitSorterAchim.h"
#include "MyDetektorHitSorterHex.h"

//______________________MyDetektorHitSorter Achim Hex______________________
class MyDetektorHitSorterAchimHex : public MyDetektorHitSorterHex, public MyDetektorHitSorterAchim
{
public:
	MyDetektorHitSorterAchimHex(const MyDetektorInfo&, MyHistos&, int HiOff);

public:
	virtual void Sort(MySignalAnalyzedEvent &sae, MyDetektor &d, MyHistos &rm)	{SortImpl(sae,d,rm);}
	void WriteCalibData(const MyDetektorInfo&);

protected:
	void SortImpl(MySignalAnalyzedEvent&, MyDetektor&, MyHistos&);
	void Shift(const MyDetektor&);
	void CreateDetHits(MyDetektor&, MyHistos&);
	void FillHistosAfterShift(const MyDetektor&, MyHistos&);	
	void CreateTDCArrays();
	void Calibrate(const MyDetektor&, MyHistos&);
};

class MyDetektorHitSorterAchimHexCalib : public MyDetektorHitSorterAchimHex
{
public:
	MyDetektorHitSorterAchimHexCalib(const MyDetektorInfo &di, MyHistos &rm, int HiOff):
	  MyDetektorHitSorterAchimHex(di,rm,HiOff) {}

public:
	void Sort(MySignalAnalyzedEvent &sae, MyDetektor &d, MyHistos &rm)	{SortImpl(sae,d,rm); Calibrate(d,rm);}
};
#endif