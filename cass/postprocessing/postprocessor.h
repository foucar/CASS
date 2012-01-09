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
#include <tr1/memory>

#include <QtCore/QMutex>
#include <QtCore/QReadWriteLock>

#include "cass.h"
#include "serializer.h"
#include "serializable.h"


namespace cass
{
//forward declarations
class CASSEvent;
class PostprocessorBackend;
class HistogramBackend;
class Histogram1DFloat;
class Histogram2DFloat;
class IdList;



/** container and call handler for all registered postprocessors.

All currently registered postprocessors are listed here, specifying their id and
a short description. The postprocessor classes are named according to their
number and placed in the file for the most appropriate group according to their
content.

@section pplist List of Postprocessors
The Classes that implement the postprocessor are pointed out in parenthesis. See
the Class description for information about what parameters are user settable.
(Keep in mind that cases matter)
@verbatim
---Operations--
00001: Compare histograms for less than constant (pp1)
00002: Compare histograms for greater than constant (pp1)
00003: Compare histograms for equal to constant (pp1)
00004: Apply boolean NOT to 0D histograms
00005: Compare two 0D histograms for boolean AND (pp5)
00006: Compare two 0D histograms for boolean OR (pp5)
00007: Compare two histograms whether first is less than second (pp7)
00008: Compare two histograms whether first is equal to second (pp7)
00009: Check whether histogram is in a range
00010: Constant true (pp10)
00011: Constant false (pp10)
00012: Constant Value

00020: Subtract second histogram from first histogram (pp20)
00021: Add first histogram to second histogram (pp20)
00022: Divide first histogram by second histogram (pp20)
00023: Multiply first histogram with second histogram (pp20)
00024: Subtract Constant from histogram (pp23)
00025: Add Constant to histogram (pp23)
00026: Multiply histogram with constant (pp23)
00027: Divide histogram constant constant (pp23)

00030: Subtract Constant from histogram, Constant is taken from 0D histogram (pp30)
00031: Add Constant to histogram, Constant is taken from 0D histogram  (pp30)
00032: Multiply histogram with constant, Constant is taken from 0D histogram  (pp30)
00033: Divide histogram by constant, Constant is taken from 0D histogram  (pp30)

00040: Threshold histogram

00050: Project 2D histogram onto a axis
00051: Integral of 1D histogram
00052: radial average / Project 2D to radius
00053: Angular Distribution / Radar Plot
00054: Convert 2D histogram to Radius - Phi representation (deprectiated; use pp202 instead)

00060: Histogram 0D values to a 1D histogram
00061: Average of a histogram
00062: Summing up of histogram
00063: Time Average of a histogram over given time-intervals
00064: 0d into 1d (append on right end, shifting old values to the left)
00065: Histogram two 0D values to a 2D histogram
00066: Histogram two 1D traces to a 2D histogram
00067: Histogram two 0D values to a 1D histogram, with first=x, second=weight
00068: Histogram 0D and 1D histogram to 2D histogram

00070: Subset a Histogram

00080: nbrOfFills of given histogram
00081: maximum bin of 1D histogram

00085: full width at half maximum for a peak in given range

---Data retrieval (Histogram contain only last shot)--
##CCD data used with old pnccd and ccd devices (select appropriate format converters)##
00100: CCD image
00101: CCD image Integral
00102: CCD image Integral using pixel(s) over user defined Threshold

00140: Photonhits Spectrum (works with old pnccd and ccd device)
00141: Photonhits Image (works with old pnccd and ccd device)
00142: Nbr of Photonhits (works with old pnccd and ccd device)

##Data used with new pixeldetector device (select appropriate format convert to use these)
00105: Pixeldetector Image
00106: Histogram of Pixeldetector pixel values
00107: Display the Map used for correction and pixel detection

00147: detected pixels spectrum (z-values)
00148: 2d image from detected pixels
00149: Number of detected pixels

00143: coalesced detected pixels (hits) spectrum (z-values)
00144: 2d image from coalesced detected pixels (hits)
00145: Number of coalesced detected pixels (hits)
00146: Split level of coalesced detected pixels (hits) (how many detected pixels within coalseced hit)


00120: Beamline data
00121: Eventcode check
00122: EventID retrival
00130: Epics data


00110: Acqiris Waveform

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
00201: Angular distribution from a 2D Histogram (interpolating)
00202: transform 2D Histogram kartesian coordinates to polar coordinates (interpolating)
00212: Advanced photon finder - Dump events to file
00220: PIPICO Spectrum
00230: Photon energy of Shot
00250: Property of particle
00251: 2d hist with two properties of particle
00252: Number of found particle hits per shot

00300: single particle detection
00301: median over last values
00240: Test Image

00400: ToF to Energy conversion
00401: Calculate variance
00402: Square average
00403: Binned 1D histogram
00404: ToF to Mass to Charge ratio conversion
00405: Pulse duration od shot
00406: ToF to Energy conversion from 0D histogram
00407: ToF to Energy conversion by linear interpolation
00408: ToF to Energy conversion by linear interpolation and correction from 0D histogram
00410: calclate covariance map
00420: indicate number of event

---Output--
01000: Dump front and back pnCCD images (and more...) to HDF5
01001: Put selected Histograms to HDF5-File
02000: Dump all selected 0d, 1d and 2d cass histograms to root file
02001: Write Hits of selected Delayline Detectors to ROOT Tree

---Coltrims Analysis--
05000: Electron Energy from Recoil momenta
05001: Tripple Coincidence Spectra from same detector

@endverbatim

@section add_pp Howto add custom postprocessors
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

If your postprocessor needs to have the general available settings call
setupGeneral().

If you want to use the optional condition:
- the pp that contains the conditon is defaulty set by "ConditionName" in
cass.ini
- document that you pp is using the optional condition.
- using setupCondition() one can setup the condition. The retrun value will
tell you whether the dependency is is already on the list

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

Make sure that you document all the different things your postprocessor does.
If you have have settings other than the generaly available that the user needs
to set in cass ini, document them with the doxygen tag cassttng.

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
class CASSSHARED_EXPORT PostProcessors
{
public:

  /** List of all currently registered postprocessors
   *
   * @todo instead of this list use strings as id
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
    AddHistograms=21,
    DivideHistograms=22,
    MultiplyHistograms=23,
    SubtractConstant=24,
    AddConstant=25,
    MultiplyConstant=26,
    DivideConstant=27,

    Subtract0DConstant=30,
    Add0DConstant=31,
    Multiply0DConstant=32,
    Divide0DConstant=33,

    Threshold=40,

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
    ZeroDto2DHistogramming=65,
    OneDto2DHistogramming=66,
    ZeroDto1DHistogramming=67,
    ZeroDand1Dto2DHistogramming=68,

    SubsetHistogram=70,

    nbrOfFills=80,
    maximumBin=81,

    fwhmPeak=85,

    SingleCcdImage=100,
    SingleCcdImageIntegral=101,
    SingleCcdImageIntegralOverThres=102,

    PixelDetectorImage=105,
    PixelDetectorImageHistogram=106,
    CorrectionMaps=107,

    AcqirisWaveform=110,
    BlData=120,
    EvrCode=121,
    EventID=122,
    EpicsData=130,

    CCDPhotonHitsSpectrum=140,
    CCDPhotonHitsImage=141,
    NbrOfCCDPhotonHits=142,

    CCDCoalescedPhotonHitsSpectrum=143,
    CCDCoalescedPhotonHitsImage=144,
    NbrOfCCDCoalescedPhotonHits=145,
    SplitLevelCoalescedPhotonHits=146,

    NewCCDPhotonHitsSpectrum=147,
    NewCCDPhotonHitsImage=148,
    NewNbrOfCCDPhotonHits=149,

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
    RealAngularDistribution=201,
    RealPolarTransformation=202,

    AdvancedPhotonFinderDump=212,

    PIPICO=220,

    PhotonEnergy=230,

    ParticleValue = 250,
    ParticleValues = 251,
    NbrParticles = 252,


    SingleParticleDetection=300,
    medianLastValues=301,
    TestImage=240,

    PnccdHDF5=1000,
    HDF5Converter=1001,
    ROOTDump=2000,
    ROOTTreeDump=2001,

    ElectronEnergy=5000,
    TrippleCoincidence=5001,

    InvalidPP
  };

  /** a shared pointer of this class */
  typedef std::tr1::shared_ptr<PostProcessors> shared_pointer;

  /** type of postproccessor accessor key */
  typedef std::string key_t;

  /** Container of all currently active postprocessors */
  typedef std::map<key_t, std::tr1::shared_ptr<PostprocessorBackend> > postprocessors_t;

  /** List of all postprocessor keys */
  typedef std::list<key_t> keyList_t;

  /** create the instance if not it does not exist already.
   *
   * @return poiner to this singelton class.
   * @param outputfilename filename of the outputfile
   */
  static shared_pointer instance(std::string outputfilename);

  /** return the already created instance of this
   *
   * check whether static instance has been created and returns if so otherwise
   * throws an exception.
   *
   * @return the instance
   */
  static shared_pointer instance();

  /** return a reference to this instance
   *
   * check whether static instance has been created and returns if so otherwise
   * throws an exception.
   *
   * @return reference to the instance
   */
  static shared_pointer::element_type& reference();

  /** process event
   *
   * This function will call postprocessors operators in the container
   *
   * @param event CASSEvent to process by all active postprocessors
   */
  void operator()(const CASSEvent& event);

  /** retrieve all activated postprocessors keys
   *
   * populate the serializable list of keys with the postprocessors that are
   * not hidden and return it
   *
   * @return shared pointer to the key list
   */
  std::tr1::shared_ptr<IdList> keys();

  /** retreive pp with key */
  PostprocessorBackend& getPostProcessor(const key_t &key);

  /** retrieve pp container */
  postprocessors_t& postprocessors() {return _postprocessors;}

  /** will be called when program will quit */
  void aboutToQuit();

  /** find all postprocessors that depend on the given one
   *
   * @return list of postprocessor key that depend on requested one
   * @param[in] key key of postprocessor that we find the dependants for
   */
  keyList_t find_dependant(const key_t& key);

  /** retrieve the list of active postprocessors */
  const keyList_t &activeList() {return _active;}

  /** a read write lock
   *
   * read write for making sure that reload is not called when someone
   * wants retrieve a list or retrieve a postprocessor.
   */
  QReadWriteLock lock;

  /** Load active postprocessors and histograms
   *
   * Reset set of active postprocessors/histograms based on cass.ini
   */
  void loadSettings(size_t);

  /** Save active postprocessors and histograms */
  void saveSettings();

protected:
  /** factory to create new Postprocessor with the name key.
   *
   * The ID which postprocessor should be used for the name is extracted from
   * the settings and has the property ID.
   *
   * @return instance the newly created postprocessor
   * @param[in] key the key of the postprocessor
   */
  std::tr1::shared_ptr<PostprocessorBackend> create(const key_t &key);

  /** Set up _postprocessors using the user requested pp in active
   *
   * Make sure that all PostProcessors on active list are in the pp container.
   * When the PostProcessor has a dependency resolve it, add it to the active
   * list if it's not already on it.
   *
   * Delete all postprocessors that are not on the active list.
   *
   * @param active reference to list of all postprocessors that should be in
   *               container.
   */
  void setup(keyList_t& active);

protected:
  /** the list of keys.
   *
   * used to create the combobox in cassview
   */
  std::tr1::shared_ptr<IdList> _keys;

  /** container for user selected and registered postprocessors */
  postprocessors_t _postprocessors;

  /** filename of the output file */
  std::string _outputfilename;

  /** list of active keys */
  keyList_t _active;

private:
  /** Private constructor of singleton
   *
   * @param outputfilename filename of the file containing the results. Used
   *                       by special postprocessors.
   */
  PostProcessors(std::string outputfilename);

  /** Prevent copy-construction of singleton */
  PostProcessors(const PostProcessors&);

  /** Prevent assignment (potentially resulting in a copy) of singleton */
  PostProcessors& operator=(const PostProcessors&);

  /** pointer to the singleton instance */
  static shared_pointer _instance;

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
