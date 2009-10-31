/*
 *  MachineDataAnalysis.h
 *  diode
 *
 *lmf
 */

#ifndef MACHINEDATAANALYSIS_H
#define MACHINEDATAANALYSIS_H


#include "cass_machine.h"
#include "analysis_backend.h"
#include "parameter_backend.h"

namespace cass
{
    class CASSEvent;

    namespace MachineData
    {
        class CASS_MACHINEDATASHARED_EXPORT Parameter : public cass::ParameterBackend
        {
        public:
            Parameter()     {beginGroup("MachineData");}
            ~Parameter()    {endGroup();}
            void load();
            void save();

        public:
        };


        class CASS_MACHINEDATASHARED_EXPORT Analysis : public cass::AnalysisBackend
        {
        public:
            Analysis()            {loadSettings();}
            ~Analysis()           {}
            void loadSettings()   {_param.load();}
            void saveSettings()   {_param.save();}

            //called for every event//
            void operator()(CASSEvent*);

        private:
            Parameter  _param;
        };
    }//end namespace vmi
}//end namespace cass

#endif
