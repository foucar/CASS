// Copyright (C) 2009 jk, lmf
#include <iostream>
#include "pnccd_analysis.h"
#include "pnccd_event.h"
#include "cass_event.h"

void cass::pnCCD::Analysis::loadSettings()
{
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
        //get the dimesions of the detector//
        const uint16_t NbrOfRows = pnccdevent.detectors()[i].rows();
        const uint16_t NbrOfCols = pnccdevent.detectors()[i].columns();

        //resize the corrected frame container to the size of the raw frame container//
        pnccdevent.detectors()[i].correctedFrame().resize(pnccdevent.detectors()[i].rawFrame().size());

        //get the pointers to the first datapoint in the frames//
        const uint16_t* rawData = &pnccdevent.detectors()[i].rawFrame()[0];
        uint16_t* corData = &pnccdevent.detectors()[i].correctedFrame()[0];

        //do the "massaging" of the detector here//

        //calc the integral (the sum of all bins)//
        pnccdevent.detectors()[i].integral() = 0;
        for (size_t j=0; j<pnccdevent.detectors()[i].correctedFrame().size();++j)
            pnccdevent.detectors()[i].integral() += pnccdevent.detectors()[i].correctedFrame()[j];

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
