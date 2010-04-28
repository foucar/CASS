// Copyright (C) 2010 lmf
// Copyright (C) 2010 Jochen Küpper

#ifndef _CCD_POSTPROCESSOR_H_
#define _CCD_POSTPROCESSOR_H_

#include "postprocessing/backend.h"
#include "cass_event.h"

namespace cass
{

// forward declaration
class Histogram0DFloat;
class Histogram1DFloat;
class Histogram2DFloat;



/** @brief Last pnCCD image */
class pp1 : public PostprocessorBackend
{
public:

    pp1(PostProcessors&, PostProcessors::id_t);

    /** Free _image spcae */
    virtual ~pp1();

    /** copy image from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);


protected:

    size_t _detector;

    Histogram2DFloat *_image;
};




/** @brief Last CCD image

@todo This should be merged with pp1
*/
class pp3 : public PostprocessorBackend
{
public:

    pp3(PostProcessors&, PostProcessors::id_t);

    /** Free _image spcae */
    virtual ~pp3();

    /** copy image from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

    /*! Define all postprocessors we depend on

    The dependencies must be run before the actual postprocessor is run by itself.
    */
    virtual std::list<PostProcessors::id_t> dependencies() {return std::list<PostProcessors::id_t>(); };

protected:

    Histogram2DFloat *_image;
};



/** @brief Averaged binned pnCCD / commercial ccd image

Running average of pnCCD or commercial ccd images with
- an averaging length of postprocessors/101/average
- geometric binning (x and y) of postprocessors/101/{bin_horizontal|bin_vertical}.
Binning must be a fraction of 1024.
*/
class pp101 : public PostprocessorBackend
{
public:

    pp101(PostProcessors& hist, PostProcessors::id_t id);

    /** Free _image spcae */
    virtual ~pp101();

    /** copy image from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

    virtual void loadSettings(size_t);

protected:

    /** Length of average */
    unsigned _average;

    /** Scaling factor of new data to approximate running average */
    float _scale;

    /** how many pixels to bin in horizontal and vertical direction */
    std::pair<unsigned, unsigned> _binning;

    /** pnCCD detector to work on */
    size_t _detector;

    /** device the ccd image comes from*/
    cass::CASSEvent::Device _device;

    /** current image */
    Histogram2DFloat *_image;
};






/** PhotonHits of CCD's.
 * This postprocessor will fill a 2d histogram with the detected Photonhits.
 * Photonhits will be detected in the commercial Pre Analyzer. They will
 * be just summed up. One needs to clear this histogram, when something has changed
 */
class pp110 : public PostprocessorBackend
{
public:

    pp110(PostProcessors& hist, PostProcessors::id_t id);

    /** Free _image spcae */
    virtual ~pp110();

    /** copy image from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

protected:
    /** device the ccd image comes from*/
    cass::CASSEvent::Device _device;

    /** detector to work on */
    size_t _detector;

    /** current image */
    Histogram2DFloat *_image;
};








/** @brief Integral of last CCD image (pp3) */
class pp141 : public PostprocessorBackend
{
public:

    pp141(PostProcessors&, PostProcessors::id_t);

    /** Free _image space */
    virtual ~pp141();

    /** copy image from CASS event to histogram storage

    @todo confirm that the simple sum is good enough or whether we need something more accurate
    (i.e., Kahan summation, Shewchuk, or similar) (JK, 2010-03-29)
    */
    virtual void operator()(const CASSEvent&);

    /*! Define postprocessor dependency on pp3 (last VMI image) */
    virtual std::list<PostProcessors::id_t> dependencies() {
        return std::list<PostProcessors::id_t>(1, PostProcessors::VmiCcdLastImage); };


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
