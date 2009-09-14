// Copyright (C) 2009 Jochen Küpper

#ifndef CASS_EVENT_H
#define CASS_EVENT_H

#include <map>
#include <QtGui/QImage>
#include "cass.h"
#include "REMIEvent.h"

namespace cass {

/** @class Event container

@author Jochen Küpper
@version 0.1
*/
    class CASSSHARED_EXPORT Event {
    public:
        //Event(XTC &);   //constructor for LCLS Dataformat
        //Event(FLASH &); //constructor for FLASH Dataformat
        uint64_t id()   {return _id;}
    private:
        //unique id to describe this event.
        //This is a montonically increasing series of positive integer values
        uint64_t _id;

        //an event conists of the detectorevents and machinedataevent//
        cass::REMI::REMIEvent _remievent;
//        PNCCDEvent pnccdevent;
//        VMIEvent vmievent;
//        MashineEvent mashineevent;


        //original
//        /** a type for all known powermeters */
//        enum PowerMeter {LCLS,                 // LCLS beam diagnostics power
//                         LCLS_CAMP_GAS,        //
//                         YAG};                 // Spectra Physics YAG power meter
//
//        /** List of all acquired power meter readings for this event */
//        std::map<PowerMeter, double> power;
//
//        /** a type for all known cameras */
//        enum CameraType {pnCCD,
//                         Pulnix};
//
//        /** List of all acquired camera images for this event */
//        std::map<std::pair<CameraType, size_t>, QImage> images;

    };
} //end namespace cass

#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
