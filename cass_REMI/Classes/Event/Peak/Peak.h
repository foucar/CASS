#ifndef __Peak_H__
#define __Peak_H__

namespace cass
{
	namespace REMI
	{

		class Peak
		{
		public:
			Peak();

		public:
			void   Clear();
			enum EPolarity{kBad=0,kPositive,kNegative};

		public:
			void   time(double in)				{fTime = in;}
			double time() const					{return fTime;}

			void   com(double in)				{fCom = in;}
			double com() const					{return fCom;}

			void   cfd(double in)				{fCfd = in;}
			double cfd() const					{return fCfd;}
			
			void   integral(double in)			{fIntegral = in;}
			double integral() const				{return fIntegral;}

			void   height(double in)			{fHeight = in;}
			double height() const				{return fHeight;}
			
			void   width(double in)				{fWidth = in;}
			double width() const				{return fWidth;}

			void   fwhm(double in)				{fFwhm = in;}
			double fwhm() const					{return fFwhm;}

			void   startpos(long in)			{fStartpos = in;}
			long   startpos() const				{return fStartpos;}

			void   stoppos(long in)				{fStoppos = in;}
			long   stoppos() const				{return fStoppos;}

			void   maxpos(long in)				{fMaxpos = in;}
			long   maxpos() const				{return fMaxpos;}

			void   maximum(double in)			{fMaximum = in;}
			double maximum() const				{return fMaximum;}

			void   polarity(long in)			{fPolarity = in;}
			long   polarity() const				{return fPolarity;}

			void   isUsed(bool in)				{fUsed = in;}
			bool   isUsed() const				{return fUsed;}

		private:
			double fTime;						//the time of the peaks, calculated from either cfd or com
			double fCfd;						//the time calculated from cfd
			double fCom;						//the time calculated form com

			long   fPolarity;					//the polarity of the peak
			double fSlope;						//the slope of this peak

			long   fMaxpos;						//the position where the maximum of peak is
			double fMaximum;					//the height in bits
			double fHeight;						//the height in mV
			double fHeightAbziehen;				//the height when you use the substraction cfd

			double fFwhm;						//the fwhm of the peak
			double fWidth;						//the width at the bottom of the peak
			double fPosHalfLeft;				//the pos where the left edge crosses the half of the height
			double fPosHalfRight;				//the pos where the right edge crosses the half of the height

			double fIntegral;					//the integral of the peak

			long   fStartpos;					//the start postion of the peak
			long   fStoppos;					//the stop position of the peak

			bool   fUsed;						//flag wether this peak has been used in sorting the detektorhits

		};
	}//end namespace REMI
}//end namespace cass
#endif
