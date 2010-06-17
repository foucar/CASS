// Copyright (C) 2010 Lutz Foucar
// Copyright (C) 2010 Jochen Kuepper

#ifndef __POSTPROCESSOR_H__
#define __POSTPROCESSOR_H__

#include <list>
#include <string>
#include <map>
#include <sstream>
#include <stdexcept>
#include <iostream>

#include <QtCore/QMutex>
#include <QtCore/QReadWriteLock>
#include <QtCore/QObject>
#include <QtCore/QSettings>

#include "cass.h"
#include "serializer.h"
#include "serializable.h"


namespace cass
{
  class CASSEvent;
  class PostprocessorBackend;
  class HistogramBackend;
  class Histogram1DFloat;
  class Histogram2DFloat;
  class IdList;



/** container and call handler for all registered postprocessors.

@todo maybe this class should not handle the histograms but only the pp. The
      histograms should be handled by the pp's.

All currently registered postprocessors are listed here, specifying their id and a
short description. The postprocessor classes are named according to their number
and placed in the file for the most appropriate group according to their content.

@section pplist List of Postprocessors
The Classes that implement the postprocessor are pointed out in parenthesis. See
the Class description for information about what parameters are user settable.
(Keep in mind that cases matter)
@verbatim
---Operations--
00001: Compare histograms for less than constant (pp1)
00002: Compare histograms for greater than constant (pp1)
00003: Compare histograms for equal to constant
00004: Apply boolean NOT to 0D histograms
00005: Compare two 0D histograms for boolean AND
00006: Compare two 0D histograms for boolean OR
00007: Compare two histograms whether first is less than second
00008: Compare two histograms whether first is equal to second
00009: Check wether histogram is in a range
00010: Constant true (pp10)
00011: Constant false (pp10)

00020: Difference between choosable averaged CCD images
00021: Divide first histogram by second histogram
00022: Multiply first histogram with second histogram
00023: Multiply histogram with constant
00024: Subtract Constant
00025: Threshold histogram

00050: Project 2D histogram onto a axis
00051: Integral of 1D histogram
00052: Project 2D to radius
00053: Angular Distribution / Radar Plot
00054: Convert 2D histogram to Radius - Phi representation

00060: Histogram values from a 0D histogram
00061: Average of a histogram
00062: Summing up of histogram
00063: Time Average of a histogram over given time-intervals
00064: 0d into 1d (append on right end, shifting old values to the left)

---Data retrieval (Histogram contain only last shot)--
00100: CCD image
00101: CCD image Integral
00102: CCD image Integral using pixel(s) over user defined Threshold
00110: Acqiris Waveform
00120: Beamline data
00130: Epics data

00140: Photonhits Spectrum
00141: Photonhits Image

00150: TofDetector number of signals in MCP waveform
00151: TofDetector all signals
00152: TofDetector signal height vs. fwhm

00160: Delayline wireend number of signals
00161: Delayline wireend signal height vs. fwhm
00162: Delayline timesum on anode
00163: Delayline timesum on anode vs. position
00164: Delayline image of first good hit
00165: Delayline reconstructed Number of detectorhits
00166: Delayline data of all reconstructed detectorhits

---Data analysis--
00200: Scalar value of <cos^2\theta> from 2D Histogram
00212: Advanced photon finder - Dump events to file
00220: PIPICO Spectrum
00230: Photon energy of Shot
00231: Wavelength of photons

---Output--
01000: Dump front and back pnCCD images (and more...) to HDF5
02000: Dump all 1d and 2d cass histograms to root file

@endverbatim

@section add_pp Howto add custom postprocessors
@todo updated this section for new layout
@subsection nec Things that a postprocessor needs to have

Your postprocessor needs to have the following members
- a constructor that takes the a reference to the histogram container and the
  processor key
- overloaded void process()(const cass::CASSEvent&)
- (optionaly you could have a pointer the pp that contains the histogram you
   depend on.)
- the pp handles the histograms and therefore it must set it up.
  - each pp has a list of histograms, which needs to be created.
  - each pp has also a pointer to the most recent one (_result)
  - it will create the list by calling createHistList(size of list).
  - since the above command uses _result to create the other histograms in list
    you need to make sure that you have created a histogram in that _result points
    to.

 If you want to use the optional condition:
 - the pp that contains the conditon is defaulty set by "ConditionName" in
   cass.ini
 - document that you pp is using the optional condition.
 - using setupCondition() one can setup the condition. The retrun value will
   tell you whether the dependency is is already on the list
 - to use the condition, just add the line \n
   if ((*_condition)(evt).isTrue())\n
   in your code

 If you want an additional dependencies do the following
 - create a key
 - use "retrieve_and_validate(_pp,_key,"HistName",keyHist)" to retrieve a pointer
   to the pp that you depend on.
   - _pp : reference to the postprocessors container
   - _key : key of this pp
   - "HistName" : Name of the dependency pp in cass.ini (needs to be documented)
   - keyHist : name of the key created in first step
 - put the key on the dependency list (_dependencies.push_back(keyHist);)

 If you want to retrieve a histogram while you setup your pp (in loadSettings)
 do the following:
 - use the "getHist(0)" member function of the pp that you depend on.

@subsection steps Register postprocessor

Steps that one has to take in order to have a custom build postprocessor registered
to the list of postprocessors:
- add your number to the above list an shortly describe what the postprocessor
  will be doing.
- add a describtive enum to the id_t enum
- add your postprossor in the switch statement of cass::PostProcessors::create
- if the Object you are writing is responsible for more than one postprocessor
  just follow the example of the last ccd processor(pp100).

@subsection doc Documentation

Please document what your postprocessor does so that other people now what it
does. When documenting please use doxygen style as then your documentation will
be available on the webserver. Documenting the parameters in cass.ini can be done
using the custom doxygen tag cassttng.
*/
  class CASSSHARED_EXPORT PostProcessors : public QObject
  {
    Q_OBJECT;

  public:

    /** List of all currently registered postprocessors
     *
     * Keep this fully list synchronized with the documentation in the class header!
     */
    enum id_t
    {
      ConstantLess=1,
      ConstantGreater=2,
      ConstantEqual=3,
      BooleanNOT=4,
      BooleanAND=5,
      BooleanOR=6,
      CompareForLess=7,
      CompareForEqual=8,
      CheckRange=9,
      ConstantTrue=10,
      ConstantFalse=11,

      SubtractHistograms=20,
      DivideHistograms=21,
      MultiplyHistograms=22,
      MultiplyConstant=23,
      SubtractConstant=24,
      Threshold=25,

      TwoDProjection=50,
      OneDIntergral=51,
      RadalProjection=52,
      AngularDistribution=53,
      R_Phi_Representation=54,

      ZeroDHistogramming=60,
      HistogramAveraging=61,
      HistogramSumming=62,
      TimeAverage=63,
      running1Dfrom0D=64,

      SingleCcdImage=100,
      SingleCcdImageIntegral=101,
      SingleCcdImageIntegralOverThres=102,
      AcqirisWaveform=110,
      BlData=120,
      EpicsData=130,

      CCDPhotonHitsSpectrum=140,
      CCDPhotonHitsImage=141,

      TofDetNbrSignals=150,
      TofDetAllSignals=151,
      TofDetMcpHeightVsFwhm=152,

      WireendNbrSignals=160,
      WireendHeightvsFwhm=161,
      AnodeTimesum=162,
      AnodeTimesumVsPos=163,
      DelaylineFirstGoodHit=164,
      DelaylineNbrReconstructedHits=165,
      DelaylineAllReconstuctedHits=166,

      Cos2Theta=200,

      AdvancedPhotonFinderDump=212,

      PIPICO=220,

      PhotonEnergy=230,
      PhotonWavelength=231,

      TestImage=240,

      PnccdHDF5=1000,
      ROOTDump=2000,


      SingleParticleDetection=589,


      InvalidPP
    };

    /** type of postproccessor accessor key */
    typedef std::string key_t;

    /** Container of all currently active postprocessors */
    typedef std::map<key_t, PostprocessorBackend*> postprocessors_t;

    /** List of all postprocessor keys */
    typedef std::list<key_t> keyList_t;

    /** create the instance if not it does not exist already.
     * @param outputfilename filename of the outputfile
     */
    static PostProcessors *instance(std::string outputfilename);

    /** destroy the instance */
    static void destroy();

    /** process event
     *
     * This function will call postprocessors operator that are on the leave list
     * @param event CASSEvent to process by all active postprocessors
     */
    void process(const CASSEvent& event);

    /** retrieve all activated postprocessors keys */
    IdList* getIdList();

    /** retreive pp with key */
    PostprocessorBackend& getPostProcessor(const key_t &key);

    /** retrieve pp container */
    const postprocessors_t& postprocessors() {return _postprocessors;}

    /** will be called when program will quit */
    void aboutToQuit();

  public slots:
    /** Load active postprocessors and histograms
     *
     * Reset set of active postprocessors/histograms based on cass.ini
     */
    void loadSettings(size_t);

    /** Save active postprocessors and histograms */
    void saveSettings() {}

    /** clear the histogram that has id */
    void clear(const key_t&);

  protected:
    /** Create new Postprocessor with key.
     * Create Postprocessor with user definded id.
     *
     * @param[in] key the key of the postprocessor
     */
    PostprocessorBackend * create(const key_t &key);

    /** Set up _postprocessors using the user requested pp in active*/
    void setup(keyList_t&);

    /** find all postprocessors that depend on the given one
     *
     * @return list of postprocessor key that depend on requested one
     * @param[in] key key of postprocessor that we find the dependants for
     */
    keyList_t find_dependant(const key_t& key);

  protected:
    /** list of postprocessors that noone depends on */
    keyList_t _leave;

    /** the list of id's */
    IdList *_IdList;

    /** container for user selected and registered postprocessors */
    postprocessors_t _postprocessors;

    /** filename of the output file */
    std::string _outputfilename;

  private:
    /** Private constructor of singleton
     * @param outputfilename filename of the file containing the results. Used
     *                       by special postprocessors.
     */
    PostProcessors(std::string outputfilename);

    /** Prevent copy-construction of singleton */
    PostProcessors(const PostProcessors&);

    /** Prevent assignment (potentially resulting in a copy) of singleton */
    PostProcessors& operator=(const PostProcessors&);

    /** Prevent destruction unless going through destroy */
    ~PostProcessors() {}

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
// c-file-style: "gnu"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:
