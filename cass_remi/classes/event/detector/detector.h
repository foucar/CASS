#ifndef __Detector_H_
#define __Detector_H_

#include <vector>
#include "peak.h"
#include "channel.h"


namespace cass
{
    namespace REMI
    {
        //----------------------------------------------------------------------------------
        class Signal	//the properties of one Wire-end of the Layerwire
        {
        public:
            void		  extractFromChannels(const channels_t&);

        public:
            double		  t(size_t idx)const	{return fPeaks.size()?fPeaks[idx].time():0;}
            double		  t()const				{return fPeaks.size()?fPeaks[0].time():0;}

        public:
            size_t		  nbrPeaks()const		{return fPeaks.size();}
            Peak		 &peak(size_t idx)		{return fPeaks[idx];}
            const Peak   &peak(size_t idx)const	{return fPeaks[idx];}

        public:
            long		  polarity()const		{return fPolarity;}
            void		  polarity(long in)		{fPolarity = in;}
            long		  chanNbr()const		{return fChNbr;}
            void		  chanNbr(long in)		{fChNbr = in;}
            double		  trLow()const			{return fTrLow;}
            void		  trLow(double in)		{fTrLow = in;}
            double		  trHigh()const			{return fTrHigh;}
            void		  trHigh(double in)		{fTrHigh = in;}

        private:
            long		  fPolarity;			//the Polarity the Signal has
            long		  fChNbr;				//the channel that we will find this Layer Signal in
            double		  fTrLow;				//lower edge of the timerange events can happen in
            double		  fTrHigh;				//upper edge of the timerange events can happen in
            peaks_t		  fPeaks;				//container for the peaks, that fit the conditions for this Signal
        };

        //----------------------------------------------------------------------------------
        class AnodeLayer	//the properties of one layer of the delayline-anode//
        {
        public:
            void			extractFromChannels(const channels_t&);

        public:
            double			pos_ns(size_t idxFi, size_t idxSe)const	{return  fOne.t(idxFi) - fTwo.t(idxSe);}
            double			pos_mm(size_t idxFi, size_t idxSe)const	{return (fOne.t(idxFi) - fTwo.t(idxSe))*fSf;}
            double			sum(size_t idxFi, size_t idxSe)const	{return  fOne.t(idxFi) + fTwo.t(idxSe);}

            double			pos_ns(size_t idx)const	{return  fOne.t(idx) - fTwo.t(idx);}
            double			pos_mm(size_t idx)const	{return (fOne.t(idx) - fTwo.t(idx))*fSf;}
            double			sum(size_t idx)const	{return  fOne.t(idx) + fTwo.t(idx);}

            double			pos_ns()const			{return  fOne.t() - fTwo.t();}
            double			pos_mm()const			{return (fOne.t() - fTwo.t())*fSf;}
            double			sum()const				{return  fOne.t() + fTwo.t();}

            double			ts()const				{return 0.5*(fTsLow+fTsHeigh);}

        public:
            double			tsLow()const			{return fTsLow;}
            void			tsLow(double in)		{fTsLow = in;}
            double			tsHigh()const			{return fTsHeigh;}
            void			tsHigh(double in)		{fTsHeigh = in;}
            double			sf()const				{return fSf;}
            void			sf(double in)			{fSf = in;}
            Signal		   &one()					{return fOne;}
            const Signal   &one()const				{return fOne;}
            void			one(Signal in)			{fOne = in;}
            Signal		   &two()					{return fTwo;}
            const Signal   &two()const				{return fTwo;}
            void			two(Signal in)			{fTwo = in;}

        private:
            double			fTsLow;					//lower edge of the timesum
            double			fTsHeigh;				//upper edge of the timesum
            double			fSf;					//scalefactor for layer
            Signal			fOne;					//the properties of one end of the wire
            Signal			fTwo;					//the properties of the other end of the wire
        };

        //----------------------------------------------------------------------------------
        class DetectorHit
        {
        public:
            DetectorHit(double x, double y, double t):
              fX_mm(x), fY_mm(y), fTime(t)		{}

        public:
            double	 x()const		{return fX_mm;}
            double	 y()const		{return fY_mm;}
            double	 t()const		{return fTime;}

        private:
            double	 fX_mm;			//the x component of the detector in mm
            double	 fY_mm;			//the y component of the detector in mm
            double	 fTime;			//the mcp time of this hit on the detector
        };
        typedef std::vector<DetectorHit> dethits_t;

        //----------------------------------------------------------------------------------
        class DetectorParameter;
        class Detector
        {
        public:
            Detector(const DetectorParameter&);
            void					 extractFromChannels(const std::vector<Channel>&);

        public:
            DetectorHit				&addHit(double x, double y, double t);
            size_t					 nbrOfHits() const				{return fHits.size();}
            DetectorHit				&hit(const int idx)				{return fHits[idx];}
            const DetectorHit		&hit(const int idx) const		{return fHits[idx];}

        public:
            const char *			 name()const					{return fName.c_str();}
            double					 runtime()const					{return fRuntime;}
            double					 wOff()const					{return fWLayerOffset;}
            double					 mcpRadius()const				{return fMcpRadius;}
            double					 deadTimeAnode()const			{return fDeadAnode;}
            double					 deadTimeMCP()const				{return fDeadMcp;}
            AnodeLayer				&u()							{return fULayer;}
            AnodeLayer				&v()							{return fVLayer;}
            AnodeLayer				&w()							{return fWLayer;}
            Signal					&mcp()							{return fMcp;}
            bool					 isHexAnode()const				{return fHex;}

        private:
            dethits_t				 fHits;							//Container storing the refrences to the DetektorHits of this Detektor
            std::string				 fName;							//the name of this detector
            AnodeLayer				 fULayer;						//properties of U1 Signals for this detektor
            AnodeLayer				 fVLayer;						//properties of V1 Signals for this detektor
            AnodeLayer				 fWLayer;						//properties of W1 Signals for this detektor
            Signal					 fMcp;							//properties of MCP Signals for this detektor
            double					 fRuntime;						//the runtime over the anode
            double					 fWLayerOffset;					//the offset of w-layer towards u and v-layer
            double					 fMcpRadius;					//the radius of the MCP in mm
            double					 fDeadMcp;						//the Deadtime between to Signals on the MCP
            double					 fDeadAnode;					//the Deadtime between to Signals on the Layers
            bool					 fHex;							//flag telling wether this is a Hexanode Detektor
            short					 fSortMethod;					//flag telling which Method to sort the times is used 0=Simple Sorting, 1=Achims Sorting
        };
        typedef std::vector<Detector> detectors_t;
    }//end namespace remi
}//end namespace cass
#endif





