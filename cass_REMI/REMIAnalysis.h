// Copyright (C)2009 Jochen Küpper


#ifndef REMIANALYSIS_H
#define REMIANALYSIS_H

#include <map>
#include <string>
#include <vector>
#include "cass_REMI.h"
#include "AnalysisBackend.h"
#include "SignalAnalyzer.h"
#include "DetektorHitSorter.h"
#include "Detector.h"
#include "REMIEvent.h"

namespace cass 
{
    namespace REMI
    {

        /** @class REMI backend parameter sets

        @author Jochen Küpper
        @version 0.1
        */


        class DetectorParameter
        {
        public:
            //the settings for one detector//
            double            fRuntime;            //the runtime over the anode
            double            fWLayerOffset;        //the offset of w-layer towards u and v-layer
            double            fMcpRadius;            //the radius of the MCP in mm
            double            fDeadMcp;            //the Deadtime between to Signals on the MCP
            double            fDeadAnode;            //the Deadtime between to Signals on the Layers
            AnodeLayer        fULayer;            //the properties of the U-Layer
            AnodeLayer        fVLayer;            //the properties of the V-Layer
            AnodeLayer        fWLayer;            //the properties of the W-Layer
            Signal            fMcp;                //properties of MCP Signal for this detektor
            long              fSortMethod;        //way how peaks are sorted for detectorhits
            bool              fIsHex;                //flag that tells
            std::string       fName;                //a name for this detector (ie. Electrondetector, Iondetector)
        };
        typedef std::vector<DetectorParameter> detparameters_t;

        class ChannelParameter
        {
        public:
            //the settings for one channel//
            double            fThreshold;            //the threshold of the channel
            double            fOffset;            //the offset
            int               fBacksize;            //the backsize
            int               fStepsize;            //the stepsize
            int               fDelay;                //the delay of the cfd
            double            fFraction;            //the fraction of the cfd
            double            fWalk;                //the walk of the cfd
        };
        typedef std::vector<ChannelParameter> chanparameters_t;

        class Parameter : public cass::ParameterBackend
        {
        public:
            size_t nbrOfDetectors()const    {return fDetectorParameters.size();}
            //a dictionary of all user settings

            //The following entries (keys) must be present:
            detparameters_t     fDetectorParameters;            //we have the option to have 1 or 2 Detectors
            chanparameters_t    fChannelParameters;            //settings to extract peaks of the channels
            int                 fPeakfindingMethod;        //way how peaks are identified
        };




        /** @class REMI analysis backend

        @author Jochen Küpper
        @version 0.1
        */


        class CASS_REMISHARED_EXPORT Analysis : public cass::AnalysisBackend
        {
        public:

            Analysis(const cass::ParameterBackend* param)        {init(param);}

            /** initialize AnalysisBackend with new set of parameters */
            void init(const cass::ParameterBackend* param);
            //called for every event//
            void operator()(CASSEvent*);


        private:
            SignalAnalyzer       fSiganalyzer;
            DetektorHitSorter    fSorter;
            Parameter            fParam;
        };


    } //end namespace REMI
} //end namespace CASS

#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
