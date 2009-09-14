// Copyright (C) 2009 Jochen Küpper

#ifndef CASS_ANALYZER_H
#define CASS_ANALYZER_H

#include <map>
#include <QtCore/QObject>
#include "cass.h"

namespace cass
{
    class CASSEvent;
    class AnalysisBackend;
    class ParameterBackend;

    /** @class Format converter container

    @author Jochen Küpper
    @version 0.1

    @todo Make Singleton
    */
    class CASSSHARED_EXPORT Analyzer : public QObject
    {
        Q_OBJECT;

    public:
        Analyzer();

        /** list of known individual analyzers */
        enum Analyzers {pnCCD, REMI, VMI, GMD, YAGPOWER};

    public slots:
        void processEvent(CASSEvent*);

    signals:
        void nextEvent(CASSEvent*);

    protected:
        std::map<Analyzers, AnalysisBackend*> _analyzer;
        std::map<Analyzers, ParameterBackend*> _parameter;
    };
}

#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
