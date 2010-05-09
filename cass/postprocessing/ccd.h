// Copyright (C) 2010 Lutz Foucar
// Copyright (C) 2010 Jochen Küpper

#ifndef _CCD_POSTPROCESSOR_H_
#define _CCD_POSTPROCESSOR_H_

#include "postprocessing/backend.h"
#include "cass_event.h"
#include "cass_acqiris.h"

namespace cass
{

// forward declaration
class Histogram0DFloat;
class Histogram1DFloat;
class Histogram2DFloat;



/** Last CCD image.
 *
 * Postprocessor will get the last image from all kinds of ccd's.
 * Will work for postprocessors 1-3.
 *
 * @author Jochen Kuepper
 * @author Lutz Foucar
 */
class pp1 : public PostprocessorBackend
{
public:
    /** constructor.*/
    pp1(PostProcessors&, PostProcessors::id_t);

    /** Free _image spcae */
    virtual ~pp1();

    /** copy image from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

protected:

    /** CCD detector that contains the requested image*/
    size_t _detector;

    /** device the ccd image comes from*/
    cass::CASSEvent::Device _device;

    /** current image */
    Histogram2DFloat *_image;
};





/** Averaged binned pnCCD or commercial ccd image.
 *
 * Running average of pnCCD or commercial ccd images.
 *
 * @cassttng PostProcessor/p\%id\%/{average} \n
 *           averaging length
 * @cassttng PostProcessor/p\%id\%/{ConditionDetector} \n
 *           Detector that you want to have the condition on. If the detector
 *           sees a signal than this condition evaluates true. If
 *           "InvalidDetector" or no Detector is chosen, than the condition
 *           is not evaluated at all.
 * @cassttng PostProcessor/p\%id\%/Invert \n
 *           Invert the Condition, when there is a valid detector condition chosen.
 *           default is "false".
 * @cassttng PostProcessor/%pp_Number%/{bin_horizontal|bin_vertical}\n
 *           geometric binning (x and y). Binning must be a fraction of 1024 (in
 *           case of pnccd's) (unused for now)
 *
 *
 * Implements postprocessors 101, 103, 105
 *
 * @author Jochen Kuepper
 * @author Lutz Foucar
 */
class pp101 : public PostprocessorBackend
{
public:

    pp101(PostProcessors& hist, PostProcessors::id_t id);

    /** Free _image space */
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

    /** CCD detector to work on */
    size_t _detector;

    /** the Detector that we make the condition on*/
    ACQIRIS::Detectors _conditionDetector;

    /** flag that will invert the condition */
    bool _invert;

    /** device the ccd image comes from*/
    cass::CASSEvent::Device _device;

    /** current image */
    Histogram2DFloat *_image;
};






/** PhotonHits of CCD's.
 *
 * This postprocessor will fill a 2d histogram with the detected Photonhits.
 * Photonhits will be detected in the commercial Pre Analyzers. Set the Parameters
 * for detecting photonhits there. (PNCCD::Analyzer or CCD::Analyzer)
 *
 * Photonhits will be just summed up in a 2d Histogram.
 * One needs to clear this histogram, when something has changed.
 *
 * implements Postprocessors 110,111,112
 *
 * User settable parameters in CASS.ini
 * - properties of the 2d histogram:
 *   PostProcessor/p%id%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}
 *
 * @author Lutz Foucar
 */
class pp110 : public PostprocessorBackend
{
public:
    /** constructor.
     * setting the appropriate device and detector
     */
    pp110(PostProcessors& hist, PostProcessors::id_t id);

    /** Free _image spcae */
    virtual ~pp110();

    /** copy image from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

    /** set the histogram size */
    virtual void loadSettings(size_t);

protected:
    /** device the ccd image comes from*/
    cass::CASSEvent::Device _device;

    /** detector to work on */
    size_t _detector;

    /** averaged image */
    Histogram2DFloat *_image;
};








/** PhotonHits of CCD's in a 1D histogram.
 *
 * This postprocessor will fill a 1d histogram with the z values detected Photonhits.
 * Photonhits will be detected in the commercial Pre Analyzer. They will
 * be just summed up. One needs to clear this histogram, when something has changed.
 *
 * Implements Postprocessor id's: 113, 114, 115
 *
 * User settable parameters in cass.ini:
 * - properties of the 1D Histogram:
 *   PostProcessor/p%id%/{XNbrBins|XLow|Xup}
 *
 * @author Lutz Foucar
 */
class pp113 : public PostprocessorBackend
{
public:
    /** constructor.*/
    pp113(PostProcessors& hist, PostProcessors::id_t id);

    /** Free _image spcae */
    virtual ~pp113();

    /** copy image from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

    /** set the histogram size */
    virtual void loadSettings(size_t);

protected:
    /** device the ccd image comes from*/
    cass::CASSEvent::Device _device;

    /** detector to work on */
    size_t _detector;

    /** current image */
    Histogram1DFloat * _hist;
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
