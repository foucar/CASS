// Copyright (C) 2010 lmf
// Copyright (C) 2010 Jochen Küpper

#ifndef _CCD_POSTPROCESSOR_H_
#define _CCD_POSTPROCESSOR_H_

#include "postprocessing/backend.h"

namespace cass
{
    //forward declaration
    class Histogram2DFloat;

    /** @brief Last pnCCD image */
    class pp1 : public PostprocessorBackend
    {
    public:

        pp1(PostProcessors::histograms_t&, PostProcessors::id_t);

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

        pp3(PostProcessors::histograms_t&, PostProcessors::id_t);

        /** Free _image spcae */
        virtual ~pp3();

        /** copy image from CASS event to histogram storage */
        virtual void operator()(const CASSEvent&);


    protected:

        size_t _detector;

        Histogram2DFloat *_image;
    };



    /** @brief Averaged binned pnCCD image

    Running average of pnCCD-1 images with
    - an averaging length of postprocessors/101/average
    - geometric binning (x and y) of postprocessors/101/{bin_horizontal|bin_vertical}.
    Binning must be a fraction of 1024.
    */
    class pp101 : public PostprocessorBackend
    {
    public:

        pp101(PostProcessors::histograms_t& hist, PostProcessors::id_t id);

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

        /** current image */
        Histogram2DFloat *_image;
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
