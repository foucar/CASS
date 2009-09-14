/*
 *  VMIAnalysis.h
 *  diode
 *
 *  Created by Jochen Küpper on 20.05.09.
 *  Copyright 2009 Fritz-Haber-Institut der MPG. All rights reserved.
 *
 */

#ifndef VMIANALYSIS_H
#define VMIANALYSIS_H

#include "cass_vmi.h"
#include "analysis_backend.h"
#include "vmi_event.h"

namespace cass
{
    namespace VMI
    {
        class Parameter : public cass::ParameterBackend
        {
        public:
            uint16_t _threshold;
            uint16_t _xCenterOfMcp;
            uint16_t _yCenterOfMcp;
            uint16_t _maxMcpRadius;
        };


        class CASS_VMISHARED_EXPORT Analysis : public cass::AnalysisBackend
        {
        public:
            Analysis(const cass::ParameterBackend *param)            {init(param);}
            void init(const cass::ParameterBackend*);

            //called for every event//
            void operator()(CASSEvent*);

        private:
            uint16_t _threshold;
            uint16_t _xCenterOfMcp;
            uint16_t _yCenterOfMcp;
            uint16_t _maxMcpRadius;
        };
    }//end namespace vmi
}//end namespace cass

#endif
