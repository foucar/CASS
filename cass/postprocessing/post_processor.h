// Copyright (C) 2010 lmf
// Copyright (C) 2010 Jochen Küpper

#ifndef __POSTPROCESSOR_H__
#define __POSTPROCESSOR_H__

#include <cassert>
#include <stdexcept>
#include <list>
#include <map>
#include <utility>
#include <algorithm>

#include <QtCore/QMutex>
#include <QtCore/QObject>

#include "cass.h"
#include "histogram.h"

namespace cass
{
class CASSEvent;
class PostprocessorBackend;



/** @brief container and call handler for all registered postprocessors

The currently registered postprocessors are:

00001: Last plain image from pnCCD-1
00002: Last plain image from pnCCD-2
00003: Last plain image from VMI ccd
00004: Last waveform of Camp Acqiris Channel 00
00005: Last waveform of Camp Acqiris Channel 01
00006: Last waveform of Camp Acqiris Channel 02
00007: Last waveform of Camp Acqiris Channel 03
00008: Last waveform of Camp Acqiris Channel 04
00009: Last waveform of Camp Acqiris Channel 05
00010: Last waveform of Camp Acqiris Channel 06
00011: Last waveform of Camp Acqiris Channel 07
00012: Last waveform of Camp Acqiris Channel 08
00013: Last waveform of Camp Acqiris Channel 09
00014: Last waveform of Camp Acqiris Channel 10
00015: Last waveform of Camp Acqiris Channel 11
00016: Last waveform of Camp Acqiris Channel 12
00017: Last waveform of Camp Acqiris Channel 13
00017: Last waveform of Camp Acqiris Channel 14
00019: Last waveform of Camp Acqiris Channel 15
00020: Last waveform of Camp Acqiris Channel 16
00021: Last waveform of Camp Acqiris Channel 17
00022: Last waveform of Camp Acqiris Channel 18
00023: Last waveform of Camp Acqiris Channel 19
00101: Running average of pnCCD-1 images with
       - geometric binning (x and y) of postprocessors/101/binning
       - an average length of postprocessors/101/average
00102: Histogram 101 with
       - background subtraction of the image file specified in postprocessors/102/background
00121: Running average of VMI camera
00131: Scalar value of the <cos^2\theta>_{2D} derived from the 121 image
00500: Averaged waveform of Camp Acqiris Channel 00
00501: Averaged waveform of Camp Acqiris Channel 01
00502: Averaged waveform of Camp Acqiris Channel 02
00503: Averaged waveform of Camp Acqiris Channel 03
00504: Averaged waveform of Camp Acqiris Channel 04
00505: Averaged waveform of Camp Acqiris Channel 05
00506: Averaged waveform of Camp Acqiris Channel 06
00507: Averaged waveform of Camp Acqiris Channel 07
00508: Averaged waveform of Camp Acqiris Channel 08
00509: Averaged waveform of Camp Acqiris Channel 09
00510: Averaged waveform of Camp Acqiris Channel 10
00511: Averaged waveform of Camp Acqiris Channel 11
00512: Averaged waveform of Camp Acqiris Channel 12
00513: Averaged waveform of Camp Acqiris Channel 13
00514: Averaged waveform of Camp Acqiris Channel 14
00515: Averaged waveform of Camp Acqiris Channel 15
00516: Averaged waveform of Camp Acqiris Channel 16
00517: Averaged waveform of Camp Acqiris Channel 17
00518: Averaged waveform of Camp Acqiris Channel 18
00519: Averaged waveform of Camp Acqiris Channel 19

*/
class PostProcessors
{
    // Q_OBJECT;

public:

    /** List of all currently registered postprocessors

    Keep this fully list synchronized with the documentation in the class header!
    */
    enum id_t {
        Pnccd1LastImage=1, Pnccd2LastImage=2,
        Pnccd1BinnedRunningAverage=101, Pnccd1BackgroundCorrectedBinnedRunnngAverage=102,
        VmiRunningAverage=121, VmiAlignment=201,
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


public slots:

    /** @brief reset set of active postprocessors/histograms based on cass.ini */
    void readIni();


protected:

    /** @brief (ordered) list of active postprocessors/histograms

    This list has order, i.e., postprocessors are called in the specified order. You can rely on the
    result of a postprocessor earlier in the list, but not on one that only occurs further back...
    */
    std::list<id_t> _active;

    /** container for all histograms */
    histograms_t _histograms;

    /** container for registered (active) postprocessors */
    postprocessors_t _postprocessors;

    /** Create new Postprocessor for specified id and using the specified histogram container */
    PostprocessorBackend * create(histograms_t hs, id_t id);

    /** Set up _histograms and _postprocessors using current _active*/
    void setup();


private:

    /** Private constructor of singleton */
    PostProcessors(const char* OutputFileName);

    /** Prevent copy-construction of singleton */
    PostProcessors(const PostProcessors&);

    /** Prevent assignment (potentially resulting in a copy) of singleton */
    PostProcessors& operator=(const PostProcessors&);

    /** Prevent destruction unless going through destroy */
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

    /** @brief Provide default implementation of readIni that does nothing */
    virtual void readIni() {};



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
            case PostProcessors::Pnccd1LastImage:
                _detector = 0;
                break;
            case PostProcessors::Pnccd2LastImage:
                _detector = 1;
                break;
            default:
                throw std::invalid_argument("Impossible postprocessor id for PostprocessorPnccdLastImage");
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



class PostprocessorPnccdBinnedRunningAverage : public PostprocessorBackend
{
public:

    PostprocessorPnccdBinnedRunningAverage(PostProcessors::histograms_t& hist, PostProcessors::id_t id);

    /** Free _image spcae */
    virtual ~PostprocessorPnccdBinnedRunningAverage()
        { delete _image; _image = 0; };

    /** copy image from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

    virtual void readIni();

protected:

    /** how many pixels to bin in vertical and horizontal direction */
    std::pair<unsigned, unsigned> _binning;

    /** pnCCD detector to work on */
    size_t _detector;

    /** current image */
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
