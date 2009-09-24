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

    //todo make singleton??

    class CASSSHARED_EXPORT Analyzer : public QObject
    {
        Q_OBJECT;

    public:
        Analyzer();
        ~Analyzer();

        /** list of known individual analyzers */
        enum Analyzers {pnCCD, REMI, VMI, GMD, YAGPOWER};

    public slots:
        void processEvent(cass::CASSEvent*);

    signals:
        void nextEvent(cass::CASSEvent*);

    protected:
        std::map<Analyzers, AnalysisBackend*> _analyzer;
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
