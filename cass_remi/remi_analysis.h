// Copyright (C)2009 Jochen Küpper,lmf


#ifndef REMIANALYSIS_H
#define REMIANALYSIS_H

#include <string>
#include <vector>
#include "cass_remi.h"
#include "analysis_backend.h"
#include "parameter_backend.h"
#include "waveform_analyzer.h"
#include "detektorhitsorter.h"
#include "remi_event.h"

namespace cass 
{
    namespace REMI
    {

        class ChannelParameter
        {
        public:
            //the settings for one channel//
            int16_t     _threshold;   //the threshold of the channel
            uint32_t    _delay;       //the delay of the cfd
            double      _fraction;    //the fraction of the cfd
            double      _walk;        //the walk of the cfd
            WaveformAnalyzer::WaveformAnalyzerTypes     _analyzetyp;     //way how peaks are identified
        };

        class CASS_REMISHARED_EXPORT Parameter : public cass::ParameterBackend
        {
        public:
            Parameter()  {beginGroup("REMI");}
            ~Parameter() {endGroup();}
        public:
            typedef std::vector<ChannelParameter> chanparameters_t;

            void load();
            void save();

            REMIEvent::detectors_t  _detectors;
            chanparameters_t        _channelParameters;     //settings to extract peaks of the channels
        };




        class CASS_REMISHARED_EXPORT Analysis : public cass::AnalysisBackend
        {
        public:
            Analysis();
            ~Analysis()         {}
            void loadSettings();
            void saveSettings() {_parameter.save();}
            //called for every event//
            void operator()(CASSEvent*);

        private:
            typedef std::map<WaveformAnalyzer::WaveformAnalyzerTypes, WaveformAnalyzer*> waveformanalyzers_t;
            typedef std::map<DetectorHitSorter::SorterTypes, DetectorHitSorter*> dethitsorter_t;

        private:
            waveformanalyzers_t _waveformanalyzer;
            dethitsorter_t      _sorter;
            Parameter           _parameter;
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
