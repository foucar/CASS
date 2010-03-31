// Copyright (C) 2010 Jochen Küpper

#ifndef _ALGINMENT_POSTPROCESSOR_H_
#define _ALGINMENT_POSTPROCESSOR_H_

#include "postprocessing/alignment.h"
#include "postprocessing/backend.h"
#include "postprocessing/postprocessor.h"

namespace cass
{
class Histogram0DFloat;

/*! Gaussian width of CCD image

This reduced the running average of the CCD image (pp121) to a scalar
that represents the FWHM of a Gaussain fit to the column histogram

@author Jochen Küpper
*/
class pp143 : public PostprocessorBackend
{
public:

    /*! Construct postprocessor for Gaussian height of image */
    pp143(PostProcessors::histograms_t&, PostProcessors::id_t);

    /** Free _image space */
    virtual ~pp143();

    /*! Determine Gaussian width of image */
    virtual void operator()(const CASSEvent&);

    /*! Define postprocessor dependency on pp121 (averaged VMI image) */
    virtual std::list<PostProcessors::id_t> dependencies() {
        return std::list<PostProcessors::id_t>(1, PostProcessors::VmiRunningAverage); };


protected:

    Histogram0DFloat *_value;
};



/*! Gaussian height of CCD image

This reduced the running average of the CCD image (pp121) to a scalar
that represents the FWHM of a Gaussain fit to the row histogram

@author Jochen Küpper
@todo Uncouple 143 and 144 -- I have only done that for demonstration puposes
*/
class pp144 : public PostprocessorBackend
{
public:

    /*! Construct postprocessor for Gaussian width of image */
    pp144(PostProcessors::histograms_t&, PostProcessors::id_t);

    /*! Free _image space */
    virtual ~pp144();

    /*! Determine Gaussian height of image */
    virtual void operator()(const CASSEvent&);

    /*! Define postprocessor dependency on pp121 (averaged VMI image) */
    virtual std::list<PostProcessors::id_t> dependencies() {
        return std::list<PostProcessors::id_t>(1, PostProcessors::VmiRunningAverage); };


protected:

    Histogram0DFloat *_value;
};


}
#endif




// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "Stroustrup"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:
