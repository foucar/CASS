// Copyright (C) 2009 Jochen Küpper

#include "pnccd_analysis.h"
#include "pnccd_event.h"
#include "cass_event.h"

void cass::pnCCD::Analysis::init()
{
    //initialize your analyzer here using param//
}

void cass::pnCCD::Analysis::operator ()(cass::CASSEvent* cassevent)
{
    cass::pnCCD::pnCCDEvent &pnccdevent = cassevent->pnCCDEvent();
    //analyze your event here//
}


// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
