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

#include "cass_VMI.h"
#include "AnalysisBackend.h"
#include "VMIEvent.h"

namespace cass
{
    namespace VMI
    {
        class Parameter : cass::BackendParameter
        {
        public:
            uint16_t _threshold;
            uint16_t _xCenterOfMcp;
            uint16_t _yCenterOfMcp;
            uint16_t _maxMcpRadius;
        };


        class CASS_VMISHARED_EXPORT Analysis : cass::AnalysisBackend
        {
        public:
            Analysis(const Parameter &param) {init(param);}
            virtual void init(const Parameter &param);

            //called for every event//
            virtual void operator()(VMIEvent&);

        private:
            uint16_t _threshold;
            uint16_t _xCenterOfMcp;
            uint16_t _yCenterOfMcp;
            uint16_t _maxMcpRadius;
        };
    }//end namespace vmi
}//end namespace cass

#endif
