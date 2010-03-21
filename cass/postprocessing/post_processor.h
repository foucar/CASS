// Copyright (C) 2010 lmf
// Copyright (C) 2010 Jochen Küpper

#ifndef __POSTPROCESSOR_H__
#define __POSTPROCESSOR_H__

#include <cassert>
#include <map>
#include <utility>

#include <QtCore/QMutex>

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

    /** List of all currently registered postprocessors

    Keep this list synchronized with the documntation of the TCP server in tcpserver.h!
    */
    enum id_t {PnccdLastImage1=1, PnccdLastImage2=2,
    };

    /** Container of all currently available histograms */
    typedef std::map<id_t, HistogramBackend*> histograms_t;

    /** Container of all currently actice postprocessors */
    typedef std::map<id_t, PostprocessorBackend*> postprocessors_t;

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
    const histograms_t &histograms() const { return _histograms; };

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

    PostprocessorBackend(PostProcessors::histograms_t& hs, PostProcessors::id_t id)
        : _id(id), _histograms(hs)
        {};

    virtual ~PostprocessorBackend()
        {};

    virtual void operator()(const CASSEvent&) = 0;

protected:

    /** @return histogram of the actual postprocessor we call this for */
    virtual HistogramBackend *histogram() { return _histograms[_id]; };

    /** @overload

    @return histogram of the requested postprocessor */
    virtual HistogramBackend *histogram(PostProcessors::id_t id) { return _histograms[id]; };

    PostProcessors::id_t _id;

    PostProcessors::histograms_t& _histograms;
};



class PostprocessorPnccdLastImage : public PostprocessorBackend
{
public:

    PostprocessorPnccdLastImage(PostProcessors::histograms_t& hist, PostProcessors::id_t id)
        : PostprocessorBackend(hist, id),
          _image(new Histogram2DFloat(1024, 0, 1023, 1024, 0, 1023))
        {
            switch(id) {
            case PostProcessors::PnccdLastImage1:
                _detector = 0;
                break;
            case PostProcessors::PnccdLastImage2:
                _detector = 1;
                break;
            };
            // save storage in PostProcessors container
            assert(hist == _histograms);
            _histograms[_id] = _image;
        };

    /** Free _image spcae */
    virtual ~PostprocessorPnccdLastImage()
        { delete _image; _image = 0; };

    /** copy image from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);


protected:

    size_t _detector;

    Histogram2DFloat *_image;
};



} // namespace cass


#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
