// Copyright (C) 2009 Jochen Küpper

#ifndef CASS_FORMATCONVERTER_H
#define CASS_FORMATCONVERTER_H

#include <map>
#include <QtCore/QMutex>
#include <QtCore/QObject>
#include "cass.h"
#include "EventQueue.h"

namespace cass
{

    /** @class Format converter container

    Only one FormatConvert object must exist, therefore this is implemented as a singleton.

    @author Jochen Küpper
    @version 0.2
    */
    class CASSSHARED_EXPORT FormatConverter : public QObject
    {
        Q_OBJECT;

    public:

        /** list of known individual format converters */
        enum Converters {pnCCD, REMI, Pulnix, GMD, YAGPOWER};

        /** Destroy the single FormatConverter instance */
        static void destroy();

        /** Return a pointer to the single FormatConverter instance */
        static FormatConverter *instance(EventQueue *);

    public slots:
        void processDatagram(uint32_t index);

    protected:

        /** Constructor */
        FormatConverter();

        /** Destructor */
        ~FormatConverter();



        /** Available format converters

        Adjust type for superclass of Format converters (FormatBackend)
        */
        std::map<Converters, FormatConverter *> _converter;

        /** pointer to the single instance */
        static FormatConverter *_instance;

        /** @brief Singleton operation locker in a multi-threaded environment. */
        static QMutex _mutex;

        static EventQueue *_eventqueue;

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
