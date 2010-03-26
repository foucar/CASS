// Copyright (C) 2010 lmf
// Copyright (C) 2010 Jochen Küpper

#ifndef __POSTPROCESSOR_H__
#define __POSTPROCESSOR_H__

#include <list>
#include <map>

#include <QtCore/QMutex>
#include <QtCore/QObject>

#include "cass.h"

namespace cass
{
  class CASSEvent;
  class PostprocessorBackend;
  class HistogramBackend;


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

  class CASSSHARED_EXPORT PostProcessors
  {
    Q_OBJECT;

  public:

    /** List of all currently registered postprocessors

    Keep this fully list synchronized with the documentation in the class header!
    */
    enum id_t
    {
      Pnccd1LastImage=1,
      Pnccd2LastImage=2,
      CampChannel00LastWaveform=4,
      CampChannel01LastWaveform=5,
      CampChannel02LastWaveform=6,
      CampChannel03LastWaveform=7,
      CampChannel04LastWaveform=8,
      CampChannel05LastWaveform=9,
      CampChannel06LastWaveform=10,
      CampChannel07LastWaveform=11,
      CampChannel08LastWaveform=12,
      CampChannel09LastWaveform=13,
      CampChannel10LastWaveform=14,
      CampChannel11LastWaveform=15,
      CampChannel12LastWaveform=16,
      CampChannel13LastWaveform=17,
      CampChannel14LastWaveform=18,
      CampChannel15LastWaveform=19,
      CampChannel16LastWaveform=20,
      CampChannel17LastWaveform=21,
      CampChannel18LastWaveform=22,
      CampChannel19LastWaveform=23,
      Pnccd1BinnedRunningAverage=101,
      Pnccd1BackgroundCorrectedBinnedRunnngAverage=102,
      VmiRunningAverage=121,
      VmiAlignment=201,
      CampChannel00AveragedWaveform=500,
      CampChannel01AveragedWaveform=501,
      CampChannel02AveragedWaveform=502,
      CampChannel03AveragedWaveform=503,
      CampChannel04AveragedWaveform=504,
      CampChannel05AveragedWaveform=505,
      CampChannel06AveragedWaveform=506,
      CampChannel07AveragedWaveform=507,
      CampChannel08AveragedWaveform=508,
      CampChannel09AveragedWaveform=509,
      CampChannel10AveragedWaveform=510,
      CampChannel11AveragedWaveform=511,
      CampChannel12AveragedWaveform=512,
      CampChannel13AveragedWaveform=513,
      CampChannel14AveragedWaveform=514,
      CampChannel15AveragedWaveform=515,
      CampChannel16AveragedWaveform=516,
      CampChannel17AveragedWaveform=517,
      CampChannel18AveragedWaveform=518,
      CampChannel19AveragedWaveform=519,
    };

    /** Container of all currently available histograms */
    typedef std::map<id_t, HistogramBackend*> histograms_t;

    /** Container of all currently actice postprocessors */
    typedef std::map<id_t, PostprocessorBackend*> postprocessors_t;

    /** create the instance if not it does not exist already */
    static PostProcessors *instance();

    /** destroy the instance */
    static void destroy();

    /** process event

    @param event CASSEvent to process by all active postprocessors
    */
    void process(CASSEvent& event);


    /** @return Histogram storage */
    const histograms_t &histograms() const { return _histograms; };

    /** @overload

    @return Histogram storage
    */
    histograms_t &histograms() { return _histograms; };


  public slots:

    /** @brief reset set of active postprocessors/histograms based on cass.ini */
    void loadSettings(size_t);
    void saveSettings() {}


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
    PostProcessors();

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
} // namespace cass


#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
