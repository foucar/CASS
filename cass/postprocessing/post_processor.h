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
class PostprocessorBackend;



/** @brief container and call handler for all registered postprocessors */
class PostProcessors
{
public:

    /** Container of all currently available histograms */
    typedef std::map<size_t, HistogramBackend*> histograms_t;

    /** Container of all currently actice postprocessors */
    typedef std::map<size_t, PostprocessorBackend*> postprocessors_t;

    /** create the instance if not it does not exist already */
    static PostProcessors *instance(const char* OutputFileName);

    /** destroy the instance */
    static void destroy();

    /** process event

    @param event CASSEvent to process by all active postprocessors
    */
    void process(CASSEvent& event);

    void loadSettings(size_t) {}

    void saveSettings() {}

    /** @return Histogram storage */
    const histograms_t  &histograms() const { return _histograms; };

    /** @overload

    @return Histogram storage
    */
    histograms_t &histograms() { return _histograms; };


protected:

    /** container for all histograms */
    histograms_t _histograms;

    /** container for registered (active) postprocessors */
    postprocessors_t _postprocessors;


private:

    PostProcessors(const char* OutputFileName);

    /** Prevent copy-construction */
    PostProcessors(const PostProcessors&);

    /** Prevent assignment (potentially resulting in a copy) */
    PostProcessors& operator=(const PostProcessors&);

    ~PostProcessors() {};

    /** pointer to the singleton instance */
    static PostProcessors *_instance;

    /** Singleton operation locker */
    static QMutex _mutex;

};



/** @brief base class for postprocessors */
class PostprocessorBackend
{
public:

    PostprocessorBackend(HistogramBackend * & backend)
        : _backend(backend) {};

    virtual ~PostprocessorBackend()
        {};

    virtual void operator()(const CASSEvent&) = 0;

protected:

    size_t _id;

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


}


#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
