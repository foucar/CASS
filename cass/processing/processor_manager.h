// Copyright (C) 2010,2013 Lutz Foucar
// Copyright (C) 2010 Jochen Kuepper

/**
 * @file processor_manager.h contains the manager for the processors
 *
 * @author Lutz Foucar
 */

#ifndef __PROCESSOR_MANAGER_H__
#define __PROCESSOR_MANAGER_H__

#include <list>
#include <string>
#include <map>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <tr1/memory>

#include <QtCore/QMutex>
#include <QtCore/QReadWriteLock>

#include "processor.h"

namespace cass
{
//forward declarations
class CASSEvent;
class IdList;



/** container and call handler for all registered processors.

All currently registered processors are listed here, specifying their id and
a short description. The processor classes are named according to their
number and placed in the file for the most appropriate group according to their
content.

@section pplist List of Processors
The Classes that implement the processor are pointed out in parenthesis. See
the Class description for information about what parameters are user settable.
(Keep in mind that cases matter)
@verbatim
---Operations--
00001: Operation on 2 Result
00002: Operation on Result with value or 0D Result
00004: Apply boolean NOT to 0D Result
00014: Ternary Operation (output InputOne when condition is true, InputTwo otherwise)

00009: Check whether Result is in a range
00012: Constant Value

00013: Identity Operation (returns the input)

00015: Check whether value of 0d Result has changed

00040: Threshold result
00041: Set bin value in result to user value, when corresponding bin value of another result is in given range

00050: Project 2D result onto a axis
00057: weighted Project 2D result onto a axis
00051: Integral of 1D result

00055: rotate, transpose or invert axis on 2d result

00056: Contains the result of the previous event

00060: Histogram 0D, 1D or 2D values to a 1D histogram, optionally with
       custom weights and remembering the counts per bin
00061: Average of a result
00062: Summing up of result
00063: Time Average of a result over given time-intervals
00064: 0D, 1D or 2D into 1D (append on right end, shifting old values to the left)
00065: Histogram two 0D values to a 2D histogram
00066: Create 2D result from two 1D results
00068: Create 2D result from 0D and 1D results, 0D defines the y-axis
00069: Use two 0D values for a scatter plot

00070: Subset a Result
00071: retrieve a user choosable value of a Result
00081: retrieve user choosable bin of 1D Result

00072: Returns a user choosable column from a table like 2D result
00073: Returns a subset of a table based on a condition on a chossable column
00074: Returns a specific value of a specific row of a table
00079: 2D Result from 2 Columns of a table with optional weight column

00075: Clear a Result
00076: Quit CASS when Condition is met
00077: Checks if eventid is on a user provided list
00078: Count how many times it has been called (Counter)

00082: user choosable statistics value of all values of a result

00085: full width at half maximum for a peak in given range of 1D result
00086: find step in a given range of 1D result
00087: find center of mass in given range of 1D result

00088: retrieve an axis parameter of the result

00089: high or low pass filter on 1D result

00090: Q average of detector image
00091: Find local minima in a 1D result

---Data retrieval (Histogram contain only last shot)--
## Data used with new pixeldetector device
00109: retrieve raw, untreated pixeldetector Image

## Processors below only work when one has set up the pixel detectors
00105: Pixeldetector Image
00107: Display the Map used for correction and pixel detection

00148: 2d image from detected pixels
00149: Number of detected pixels

00144: 2d image from coalesced detected pixels (hits)
00145: Number of coalesced detected pixels (hits)
00146: Split level of coalesced detected pixels (hits) (how many detected pixels within coalseced hit)


## Beamline Data Retrieval
00120: Beamline data
00121: Eventcode check
00122: EventID retrival
00123: Beamline Spectrometer Data
00130: Epics data


## Waveform and detectors recorded via waveform
00110: Acqiris Waveform
00111: CFD Trace from Acqiris Waveform
00112: CFD Analysis of Waveform

00150: TofDetector number of signals in MCP waveform
00151: TofDetector all signals
00152: TofDetector signal height vs. fwhm
00153: TofDetector Deadtime between two consecutive MCP signals

00160: Delayline wireend number of signals
00161: Delayline wireend signal height vs. fwhm
00162: Delayline timesum on anode
00163: Delayline timesum on anode vs. position
00164: Delayline image of first good hit
00165: Delayline reconstructed Number of detectorhits
00166: Delayline data of all reconstructed detectorhits
00167: Delayline Deadtime between two consecutive anode signals

00170: Hex Delayline Calibrator (see Hexcalibrator for parameters)

---Data analysis--
00200: Scalar value of <cos^2\theta> from 2D Histogram
00201: Angular distribution from a 2D Histogram (interpolating)
00202: transform 2D Histogram kartesian coordinates to polar coordinates (interpolating)
00220: PIPICO Spectrum
00230: Photon energy of Shot
00250: Property of particle
00251: 2d hist with two properties of particle
00252: Number of found particle hits per shot

00203: Local image background using median box
00204: Find Bragg peaks in image using signal to noise ratio
00208: Find Bragg peaks in image using signal to noise ratio without outliers
00205: Display Peaks found in an image
00206: Find Pixels that might belong to a bragg peak by looking above threshold

00300: single particle detection
00301: median over last values
00302: binary file 2DHistogram
00310: Autocorrelation of image in polar coordinates
00311: Autocorrelation of image in kartesian coordinates
00312: FFT of an histogram
00313: Convolute a histogram with a kernel
00240: Test Image

00330: Generate Calibration data from raw images
00331: Generate Gain Calibration from images
00332: Generate Hot Pixel Map from images
00333: Generate common mode Background level
00334: Generate common mode Background level for CsPAD

00241: fix distorted offset of pnCCD frames
00242: same as 105, but one can put a user defined value at the masked pixels
00243: apply a mask to an image set the masked values to a user defined value
00244: generate a histogram of the pixelvalues for each pixel in an image

01600: convert cass cspad to cheetah cspad
01601: rearrange cspad to coarsly right orientation (looking from upstream)
01602: rearrange cspad using a geom file

00400: ToF to Energy conversion
00404: ToF to Mass to Charge ratio conversion
00405: Pulse duration of shot
00406: ToF to Energy conversion from 0D histogram
00407: ToF to Energy conversion by linear interpolation
00408: ToF to Energy conversion by linear interpolation and correction from 0D histogram
00410: calculate covariance map
00412: calculate intensity correction

---Output--
01002: Put selectable histograms into HDF5-Files
01500: Put a selectable 2d histogram into CBF Files
02000: Dump all selected 0d, 1d and 2d cass histograms to root file
02001: Write Hits of selected Delayline Detectors to ROOT Tree

---Coltrims Analysis--
05000: Electron Energy from Recoil momenta
05001: Tripple Coincidence Spectra from same detector

---Removed Processors---
00003: removed use pp2 instead
00005: removed use pp1 instead
00006: removed use pp1 instead
00007: removed use pp1 instead
00008: removed use pp1 instead
00010: removed use pp12 instead
00011: removed use pp12 instead
00020: removed used pp1 instead
00021: removed used pp1 instead
00022: removed used pp1 instead
00023: removed used pp1 instead
00024: removed use pp2 instead
00025: removed use pp2 instead
00026: removed use pp2 instead
00027: removed use pp2 instead
00030: removed use pp2 instead
00031: removed use pp2 instead
00032: removed use pp2 instead
00033: removed use pp2 instead
00052: removed use pp202 and pp50 instead
00053: removed use pp202 and pp50 instead
00054: removed use pp202 instead
00067: removed use pp60 instead
00080: removed use pp78 instead
00083: removed use pp82 instead
00084: removed use pp82 instead
00100: removed use pp105 instead
00101: removed use pp105 and pp82 instead
00102: removed use pp105, pp40 and pp82 instead
00106: removed use pp60 instead
00108: removed use pp84 instead
00140: removed use pp105 and pp60 instead
00141: removed use pp148 instead
00142: removed use pp149 instead
00143: removed use pp60 instead
00147: removed use pp60 instead
00155: removed use pp84 instead
00156: removed use pp84 instead
00207: removed use pp79 instead
00212: removed use pp144 or pp148 instead
00401: removed use pp83 instead
00402: removed use pp61 instead
00403: removed use pp70 instead
00420: removed use pp80 instead
01000: removed use pp1002 instead
01001: removed use pp1002 instead
@endverbatim
*/
class ProcessorManager
{
public:

  /** List of all currently registered processors
   *
   * @todo instead of this list use strings as id
   *
   * Keep this fully list synchronized with the documentation in the class header!
   */
  enum id_t
  {
    OperationsOn2Histos=1,
    OperationWithValue=2,
    ConstantEqual=3,
    BooleanNOT=4,
    BooleanAND=5,
    BooleanOR=6,
    CompareForLess=7,
    CompareForEqual=8,
    CheckRange=9,
    ConstantTrue=10,
    ConstantFalse=11,
    ConstantValue=12,
    Identity=13,
    TernaryOperator=14,

    CheckChange=15,

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
    ThresholdImage=41,

    TwoDProjection=50,
    OneDIntergral=51,
    RadalProjection=52,
    AngularDistribution=53,
    R_Phi_Representation=54,

    imageManip=55,
    previousHist=56,

    weightedProject=57,

    ZeroDHistogramming=60,
    HistogramAveraging=61,
    HistogramSumming=62,
    TimeAverage=63,
    running1Dfrom0D=64,
    ZeroDto2DHistogramming=65,
    OneDto2DHistogramming=66,
    ZeroDto1DHistogramming=67,
    ZeroDand1Dto2DHistogramming=68,
    OneDtoScatterPlot=69,

    SubsetHistogram=70,

    RetrieveValue=71,
    RetrieveColFromTable=72,
    SubsetTable=73,
    RetrieveValOfRow=74,

    ClearHistogram=75,
    QuitCASS=76,
    IdIsOnList=77,
    Counter=78,

    Table2TwoDHist=79,

    nbrOfFills=80,
    maximumBin=81,
    meanvalue=82,
    standartDev=83,
    sumbins=84,

    fwhmPeak=85,
    step=86,
    centerofmass=87,
    axisparameter=88,
    highlowpassfilter=89,
    qaverage=90,
    nodes=91,

    SingleCcdImage=100,
    SingleCcdImageIntegral=101,
    SingleCcdImageIntegralOverThres=102,

    PixelDetectorImage=105,
    PixelDetectorImageHistogram=106,
    CorrectionMaps=107,
    SumPixels=108,

    RAWPixeldetectorFrame=109,

    AcqirisWaveform=110,
    CFDTraceFromWaveform=111,
    CFDAnalysis=112,

    BlData=120,
    EvrCode=121,
    EventID=122,
    BldSpecData=123,
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
    TofDetDeadtime=153,

    SumFoundPixels=155,
    SumPhotonHits=156,

    WireendNbrSignals=160,
    WireendHeightvsFwhm=161,
    AnodeTimesum=162,
    AnodeTimesumVsPos=163,
    DelaylineFirstGoodHit=164,
    DelaylineNbrReconstructedHits=165,
    DelaylineAllReconstuctedHits=166,
    DelaylineAnodeSigDeadtime=167,

    HEXCalibrator = 170,

    Cos2Theta=200,
    RealAngularDistribution=201,
    RealPolarTransformation=202,

    MedianBoxBackground=203,
    BraggPeakSNR=204,
    DrawPeaks=205,
    BraggPeakThreshold=206,
    ImageFromTable=207,
    BraggPeakSNRWOOutliers=208,

    AdvancedPhotonFinderDump=212,

    PIPICO=220,

    PhotonEnergy=230,
    TestImage=240,

    fixOffset=241,
    MaskValue=242,
    MaskImageValue=243,
    PixelHistogram=244,

    ParticleValue = 250,
    ParticleValues = 251,
    NbrParticles = 252,

    SingleParticleDetection=300,
    medianLastValues=301,
    binaryFile2D=302,
    Autocorrelation=310,
    Autocorrelation2=311,
    fft=312,
    convoluteKernel=313,

    calibration=330,
    gaincalibration=331,
    hotpixmap=332,
    commonmodecalc=333,
    commonmodecalcCsPad=334,

    tof2energy=400,
    calcVariance=401,
    squaredaverage=402,
    Bin1DHist=403,
    TofToMTC=404,
    PulseDuration=405,
    tof2energy0D=406,
    tof2energylinear=407,
    tof2energylinear0D=408,
    calcCovarianceMap=410,
    calcCorrection=412,
    EventNumber=420,

    PnccdHDF5=1000,
    HDF5Converter=1001,
    HDF52dConverter=1002,
    CBFOutput=1500,
    ChetahConv=1600,
    CoarseCsPadAligment=1601,
    GeomFileCsPadAligment=1602,
    ROOTDump=2000,
    ROOTTreeDump=2001,

    ElectronEnergy=5000,
    TrippleCoincidence=5001,

    InvalidPP
  };

  /** a shared pointer of this class */
  typedef std::tr1::shared_ptr<ProcessorManager> shared_pointer;

  /** type of proccessor accessor key */
  typedef Processor::name_t key_t;

  /** Container of all currently active processors */
  typedef std::list<Processor::shared_pointer> processors_t;

  /** List of all processor keys */
  typedef Processor::names_t keyList_t;

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
   * This function will call processors operators in the container
   *
   * @param event CASSEvent to process by all active processors
   */
  void operator()(const CASSEvent& event);

  /** retrieve all activated processors keys
   *
   * populate the serializable list of keys with the processors that are
   * not hidden and return it
   *
   * @return shared pointer to the key list
   */
  std::tr1::shared_ptr<IdList> keys();

  /** retreive pp with key */
  Processor& getProcessor(const Processor::name_t &name);

  /** retreive pp with name
   *
   * @param name The name of the Processor to retrive
   */
  Processor::shared_pointer getProcessorSPointer(const Processor::name_t &name);

  /** retrieve pp container */
  processors_t& processors() {return _processors;}

  /** will be called when program will quit */
  void aboutToQuit();

  /** retrieve the list of active processors */
  const keyList_t &activeList() {return _active;}

  /** a read write lock
   *
   * read write for making sure that reload is not called when someone
   * wants retrieve a list or retrieve a processor.
   */
  QReadWriteLock lock;

  /** Load active processors and histograms
   *
   * Reset set of active processors/histograms based on cass.ini
   */
  void loadSettings(size_t);

protected:
  /** factory to create new processor with the name key.
   *
   * The ID which processor should be used for the name is extracted from
   * the settings and has the property ID.
   *
   * @return instance the newly created processor
   * @param[in] key the key of the processor
   */
  Processor::shared_pointer create(const key_t &key);

protected:
  /** the list of keys.
   *
   * used to create the combobox in cassview
   */
  std::tr1::shared_ptr<IdList> _keys;

  /** container for user selected and registered processors */
  processors_t _processors;

  /** filename of the output file */
  std::string _outputfilename;

  /** list of active keys */
  keyList_t _active;

private:
  /** Private constructor of singleton
   *
   * @param outputfilename filename of the file containing the results. Used
   *                       by special processors.
   */
  ProcessorManager(std::string outputfilename);

  /** Prevent copy-construction of singleton */
  ProcessorManager(const ProcessorManager&);

  /** Prevent assignment (potentially resulting in a copy) of singleton */
  ProcessorManager& operator=(const ProcessorManager&);

  /** pointer to the singleton instance */
  static shared_pointer _instance;

  /** Singleton operation locker */
  static QMutex _mutex;
};

} // namespace cass


#endif
