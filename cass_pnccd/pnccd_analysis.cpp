// Copyright (C) 2009 jk, lmf
#include <iostream>
#include "pnccd_analysis.h"
#include "pnccd_event.h"
#include "cass_event.h"

void cass::pnCCD::Analysis::loadSettings()
{
    //sync before loading//
    sync();

    //initialize your analyzer here using param//
}

void cass::pnCCD::Analysis::saveSettings()
{
    //save your settings here//
}

void cass::pnCCD::Analysis::operator ()(cass::CASSEvent* cassevent)
{
    //extract a reference to the pnccdevent in cassevent//
    cass::pnCCD::pnCCDEvent &pnccdevent = cassevent->pnCCDEvent();
    //clear the event//
    for (size_t i=0; i<pnccdevent.detectors().size();++i)
    {
        pnccdevent.detectors()[i].recombined().clear();
        pnccdevent.detectors()[i].nonrecombined().clear();
    }
    //go through all detectors//
    for (size_t i=0; i<pnccdevent.detectors().size();++i)
    {
        //retrieve a reference to the detector we are working on right now//
        cass::pnCCD::pnCCDDetector &det = pnccdevent.detectors()[i];
        //get the dimesions of the detector//
        const uint16_t NbrOfRows = det.rows();
        const uint16_t NbrOfCols = det.columns();

        //resize the corrected frame container to the size of the raw frame container//
//        pnccdevent.detectors()[i].correctedFrame().resize(pnccdevent.detectors()[i].rawFrame().size());
        det.correctedFrame().assign(det.rawFrame().begin(), det.rawFrame().end()); //for testing copy the contents of raw to cor

        //get the pointers to the first datapoint in the frames//
//        const uint16_t* rawData = &pnccdevent.detectors()[i].rawFrame()[0];
//        uint16_t* corData = &pnccdevent.detectors()[i].correctedFrame()[0];

        //do the "massaging" of the detector here//

        //calc the integral (the sum of all bins)//
        det.integral() = 0;
        for (size_t j=0; j<det.correctedFrame().size();++j)
            det.integral() += det.correctedFrame()[j];

        //find the photon hits here//
    }
}


// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
