// Copyright (C) 2010 Lutz Foucar
// Copyright (C) 2010 Jochen Küpper

#ifndef _POSTPROCESSOR_BACKEND_H_
#define _POSTPROCESSOR_BACKEND_H_

#include <list>

#include "cass.h"
#include "postprocessing/postprocessor.h"

namespace cass
{
//forward declaration
class CASSEvent;

/** @brief base class for postprocessors */
class CASSSHARED_EXPORT PostprocessorBackend
{
public:

    PostprocessorBackend(PostProcessors& pp, PostProcessors::id_t id)
        : _id(id), _pp(pp)
    {}

    virtual ~PostprocessorBackend() { }

    virtual void operator()(const CASSEvent&) = 0;

    /** @brief Provide default implementation of loadSettings that does nothing */
    virtual void loadSettings(size_t) {};

    /*! Define all postprocessors we depend on

    The dependencies must be run before the actual postprocessor is run by itself.
    */
    virtual std::list<PostProcessors::id_t> dependencies() { return std::list<PostProcessors::id_t>(); };


protected:

    /** @return histogram of the actual postprocessor we call this for */
    virtual HistogramBackend *histogram_checkout() { return histogram_checkout(_id); };

    /** @overload

    @return histogram of the requested postprocessor */
    virtual HistogramBackend *histogram_checkout(PostProcessors::id_t id) {
        try {
            PostProcessors::histograms_t hist(_pp.histograms_checkout());
            _pp.validate(id);
            return hist[id];
        } catch (InvalidHistogramError *) {
            return 0;
        }
    };

    void histogram_release() { _pp.histograms_release(); };

    // the postprocessors id (see post_processor.h for an list of ids)//
    PostProcessors::id_t _id;

    // reference to the PostProcessors container
    PostProcessors& _pp;
};

} //end namespace cass

#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "Stroustrup"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:
