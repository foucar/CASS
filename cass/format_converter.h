// Copyright (C) 2009 Jochen Küpper

#ifndef CASS_FORMATCONVERTER_H
#define CASS_FORMATCONVERTER_H

#include <map>
#include <QtCore/QMutex>
#include <QtCore/QObject>
#include "cass.h"
#include "cass_event.h"

namespace cass
{
    class ConversionBackend;

    // @class Format converter container
    //Only one FormatConvert object must exist, therefore this is implemented as a singleton.
    //@author Jochen Küpper,lmf
    //@version 0.2

    class CASSSHARED_EXPORT FormatConverter : public QObject
    {
        Q_OBJECT;

    public:
        // list of known individual format converters //
        enum Converters {pnCCD, REMI, Pulnix, MachineData};
        // Destroy the single FormatConverter instance//
        static void destroy();
        // Return a pointer to the single FormatConverter instance//
        static FormatConverter *instance();
        //function to process a datagram and turn it into a cassevent/
        bool processDatagram(cass::CASSEvent*);

    signals:
        void nextEvent(cass::CASSEvent*);

    protected:
        FormatConverter();
        ~FormatConverter();

        // Available format converters
        std::map<Converters, ConversionBackend *> _converter;
        // pointer to the single instance //
        static FormatConverter *_instance;
        //Singleton operation locker in a multi-threaded environment.//
        static QMutex _mutex;

        //indicator whether process gets called the firsttime//
        static bool _firsttime;
    };

}//end namespace cass


#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
