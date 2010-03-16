// Copyright (C) 2010 lmf
// Copyright (C) 2010 Jochen Küpper

#ifndef __POSTPROCESSOR_H__
#define __POSTPROCESSOR_H__

#include <QtCore/QMutex>

#include <map>
#include <utility>

#include "cass.h"
#include "histogram.h"

namespace cass
{
    class CASSEvent;


    class PostprocessorBackend
    {
    public:

        PostprocessorBackend(HistogramBackend * & backend)
            : _backend(backend) {};

            virtual void operator()(const CASSEvent&) = 0;

    protected:

            HistogramBackend *_backend;
    };


    class PostprocessorAveragePnCCD : public PostprocessorBackend
    {
    public:

        PostprocessorAveragePnCCD(HistogramBackend * & backend)
            : PostprocessorBackend(backend)
        {
            _backend = new Histogram2DFloat(1024, 0, 1023, 1024, 0, 1023);
        };

            virtual void operator()(const CASSEvent&);

    protected:

            Histogram2DFloat *_backend;
    };




    class PostProcessor
    {
    public:

        //creates an instace if not it does not exist already//
        static PostProcessor *instance(const char* OutputFileName);
        //this destroys the the instance//
        static void destroy();

    public:
        void postProcess(CASSEvent&);
        void loadSettings(size_t) {}
        void saveSettings() {}

    public:

        typedef std::map<std::pair<size_t, size_t>, HistogramBackend*> histograms_t;

        typedef std::map<std::pair<size_t, size_t>, PostprocessorBackend*> postprocessors_t;

    public://setters/getters

        /** @return Histogram storage */
        const histograms_t  &histograms() const  {return _histograms;};

        /** @return Histogram storage */
        histograms_t        &histograms()        {return _histograms;};

    protected:

        PostProcessor(const char* OutputFileName);

        ~PostProcessor() {};

        //pointer to the instance//
        static PostProcessor *_instance;
        //Singleton operation locker in a multi-threaded environment.//
        static QMutex _mutex;

        //container for all histograms//
        histograms_t _histograms;

        // container for postrocessors
        postprocessors_t _postprocessors;
    };


}


#endif
