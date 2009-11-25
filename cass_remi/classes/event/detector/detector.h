#ifndef __Detector_H_
#define __Detector_H_

#ifndef ROOT_Rtypes
#include <Rtypes.h>
#endif

#ifndef ROOT_TObject
#include <TObject.h>
#endif

#include <vector>
#include "peak.h"
#include "channel.h"


namespace cass
{
    namespace REMI
    {
        //----------------------------------------------------------------------------------
        class Signal    //the properties of one Wire-end of the Layerwire
        {
        public:
            Signal()    {}
            ~Signal()   {}

        public:
            typedef std::vector<Peak*> peaks_t;

        public:
            peaks_t        &peaks()         {return fPeaks;}
            const peaks_t  &peaks()const    {return fPeaks;}

        public:
            Peak::Polarity  polarity()const {return fPolarity;}
            Peak::Polarity &polarity()      {return fPolarity;}
            long            chanNbr()const  {return fChNbr;}
            size_t         &chanNbr()       {return fChNbr;}
            double          trLow()const    {return fTrLow;}
            double         &trLow()         {return fTrLow;}
            double          trHigh()const   {return fTrHigh;}
            double         &trHigh()        {return fTrHigh;}

        private:
            Peak::Polarity  fPolarity;      //the Polarity the Signal has
            size_t          fChNbr;         //the channel that we will find this Layer Signal in
            double          fTrLow;         //lower edge of the timerange events can happen in
            double          fTrHigh;        //upper edge of the timerange events can happen in
            peaks_t         fPeaks;         //container for the peaks, that fit the conditions for this Signal
        };






        //----------------------------------------------------------------------------------
        class AnodeLayer    //the properties of one layer of the delayline-anode//
        {
        public:
            AnodeLayer()    {}
            ~AnodeLayer()   {}

        public:
            double           ts()const                                  {return 0.5*(fTsLow+fTsHeigh);}

        public:
            double           tsLow()const                               {return fTsLow;}
            double          &tsLow()                                    {return fTsLow;}
            double           tsHigh()const                              {return fTsHeigh;}
            double          &tsHigh()                                   {return fTsHeigh;}
            double           sf()const                                  {return fSf;}
            double          &sf()                                       {return fSf;}
            const Signal    &one()const                                 {return fOne;}
            Signal          &one()                                      {return fOne;}
            const Signal    &two()const                                 {return fTwo;}
            Signal          &two()                                      {return fTwo;}

        private:
            double           fTsLow;                                    //lower edge of the timesum
            double           fTsHeigh;                                  //upper edge of the timesum
            double           fSf;                                       //scalefactor for layer
            Signal           fOne;                                      //the properties of one end of the wire
            Signal           fTwo;                                      //the properties of the other end of the wire
        };






        //----------------------------------------------------------------------------------
        class DetectorHit
        {
        public:
            DetectorHit(double x, double y, double t):
              fX_mm(x), fY_mm(y), fTime(t)        {}
            DetectorHit()  {}
            ~DetectorHit() {}


        public:
            double  x()const    {return fX_mm;}
            double &x()         {return fX_mm;}
            double  y()const    {return fY_mm;}
            double &y()         {return fY_mm;}
            double  t()const    {return fTime;}
            double &t()         {return fTime;}

        private:

            double  fX_mm;      //the x component of the detector in mm
            double  fY_mm;      //the y component of the detector in mm
            double  fTime;      //the mcp time of this hit on the detector
        };








        //----------------------------------------------------------------------------------
        class DetectorParameter;
        class Detector
        {
        public:
            Detector()  {}
            ~Detector() {}

        public:
            typedef std::vector<DetectorHit> dethits_t;

        public:
            const dethits_t     &hits()const            {return _hits;}
            dethits_t           &hits()                 {return _hits;}

        public:
            const std::string   &name()const            {return _name;}
            std::string         &name()                 {return _name;}
            double               runtime()const         {return _runtime;}
            double              &runtime()              {return _runtime;}
            double               wLayerOffset()const    {return _wLayerOffset;}
            double              &wLayerOffset()         {return _wLayerOffset;}
            double               mcpRadius()const       {return _mcpRadius;}
            double              &mcpRadius()            {return _mcpRadius;}
            double               deadTimeAnode()const   {return _deadAnode;}
            double              &deadTimeAnode()        {return _deadAnode;}
            double               deadTimeMCP()const     {return _deadMcp;}
            double              &deadTimeMCP()          {return _deadMcp;}
            const AnodeLayer    &u()const               {return _uLayer;}
            AnodeLayer          &u()                    {return _uLayer;}
            const AnodeLayer    &v()const               {return _vLayer;}
            AnodeLayer          &v()                    {return _vLayer;}
            const AnodeLayer    &w()const               {return _wLayer;}
            AnodeLayer          &w()                    {return _wLayer;}
            const Signal        &mcp()const             {return _mcp;}
            Signal              &mcp()                  {return _mcp;}
            bool                 isHexAnode()const      {return _isHex;}
            bool                &isHexAnode()           {return _isHex;}
            int32_t              sorterType()const      {return _sorterType;}
            int32_t             &sorterType()           {return _sorterType;}

        private:
            //variables that the sorters need to find detectorhits, given by the user
            std::string          _name;                 //the name of this detector
            AnodeLayer           _uLayer;               //properties of U1 Signals for this detektor
            AnodeLayer           _vLayer;               //properties of V1 Signals for this detektor
            AnodeLayer           _wLayer;               //properties of W1 Signals for this detektor
            Signal               _mcp;                  //properties of MCP Signals for this detektor
            double               _runtime;              //the runtime over the anode
            double               _wLayerOffset;         //the offset of w-layer towards u and v-layer
            double               _mcpRadius;            //the radius of the MCP in mm
            double               _deadMcp;              //the Deadtime between to Signals on the MCP
            double               _deadAnode;            //the Deadtime between to Signals on the Layers
            bool                 _isHex;                //flag telling wether this is a Hexanode Detektor
            int32_t              _sorterType;           //flag telling which Method to sort the times is used 0=Simple Sorting, 1=Achims Sorting

            //output of the analysis (sorting) of the peaks
            dethits_t            _hits;                     //Container storing the refrences to the DetektorHits of this Detektor
            ClassDefNV(Detector,1)                          //A Delayline Detector
        };

    }//end namespace remi
}//end namespace cass
#endif





