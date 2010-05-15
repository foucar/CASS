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


/** Exception thrown when accessing invalid histograms
 *
 * @author Jochen Küpper
 */
class InvalidHistogramError : public std::out_of_range
{
public:

    InvalidHistogramError(size_t id)
        : std::out_of_range("Invalid histogram requested!"), _id(id)
        {};

    virtual const char* what() const throw() {
        std::ostringstream msg;
        msg << "Invalid histogram " << _id << " requested!";
        return msg.str().c_str();
    };


protected:

    size_t _id;
};

/** binary function for averaging.
 * this operator is capable of performing a cumulative moving average and
 * a Exponential moving average.
 * @see http://en.wikipedia.org/wiki/Moving_average
 * @author Lutz Foucar
 */
class Average : std::binary_function<float,float,float>
{
public:
  /** constructor.
   * initializes the \f$\alpha\f$ value
   * @param alpha The \f$\alpha\f$ value
   */
  explicit Average(float alpha)
    :_alpha(alpha)
  {}
  /** the operator calculates the average using the function
   * \f$Y_N = Y_{N-1} + \alpha(y-Y_{N-1})\f$
   * where when \f$\alpha\f$ is equal to N it is a cumulative moving average,
   * otherwise it will be a exponential moving average.
   */
  float operator()(float currentValue, float Average_Nm1)
  {
    return Average_Nm1 + _alpha*(currentValue - Average_Nm1);
  }
protected:
  /** \f$\alpha\f$ for the average calculation */
  float _alpha;
};



/** function to set the 1d histogram properties from the ini file.
 * @param[out] hist pointer to the 1D Histogram whos properties should be updated
 *            (will be deleted and created with new settings)
 * @param[in] id the id of the postprocessor too look up in cass.ini
 * @author Lutz Foucar
 */
void set1DHist(cass::Histogram1DFloat*& hist, size_t id);

/** function to set the 2d histogram properties from the ini file.
 * @param[out] hist pointer to the 2D Histogram whos properties should be updated
 *            (will be deleted and created with new settings)
 * @param[in] id the id of the postprocessor too look up in cass.ini
 * @author Lutz Foucar
 */
void set2DHist(cass::Histogram2DFloat*& hist, size_t id);



/** @brief container and call handler for all registered postprocessors

All currently registered postprocessors are listed here, specifying their id, a description, and (in
parenthesis) the PostProcessor group they belong to (REMI, VMI, pnCCD, other) The postprocessor
classes are named according to their number (or the first number for classes handling multiple
histograms) and placed in the file for the most appropriate group according to their content.

@cassttng PostProcessor/{active}\n
          A list of all active postprocesors. The list does not need to
          be in the right order since dependencies will be resolved.

@section pplist List of Postprocessors
The Classes that implement the postprocessor are pointed out in parenthesis. See
the Class description for information about what parameters are user settable.
(Keep in mind that cases matter)
@todo the classes in parenthesis should be links to the actual classes
@verbatim
00001: Last plain image from Front pnCCD (pp1)
00002: Last plain image from Back pnCCD (pp1)

00003: Last plain image from VMI ccd camera (pp1)

00004: Last waveform of Camp Acqiris Channel 00 (pp4)
00005: Last waveform of Camp Acqiris Channel 01 (pp4)
00006: Last waveform of Camp Acqiris Channel 02 (pp4)
00007: Last waveform of Camp Acqiris Channel 03 (pp4)
00008: Last waveform of Camp Acqiris Channel 04 (pp4)
00009: Last waveform of Camp Acqiris Channel 05 (pp4)
00010: Last waveform of Camp Acqiris Channel 06 (pp4)
00011: Last waveform of Camp Acqiris Channel 07 (pp4)
00012: Last waveform of Camp Acqiris Channel 08 (pp4)
00013: Last waveform of Camp Acqiris Channel 09 (pp4)
00014: Last waveform of Camp Acqiris Channel 10 (pp4)
00015: Last waveform of Camp Acqiris Channel 11 (pp4)
00016: Last waveform of Camp Acqiris Channel 12 (pp4)
00017: Last waveform of Camp Acqiris Channel 13 (pp4)
00017: Last waveform of Camp Acqiris Channel 14 (pp4)
00019: Last waveform of Camp Acqiris Channel 15 (pp4)
00020: Last waveform of Camp Acqiris Channel 16 (pp4)
00021: Last waveform of Camp Acqiris Channel 17 (pp4)
00022: Last waveform of Camp Acqiris Channel 18 (pp4)
00023: Last waveform of Camp Acqiris Channel 19 (pp4)
00024: Last waveform of ITof Acqiris Channel 00 (pp4)
00025: Last waveform of ITof Acqiris Channel 01 (pp4)
00026: Last waveform of ITof Acqiris Channel 02 (pp4)
00027: Last waveform of ITof Acqiris Channel 03 (pp4)

00100: Running average of front pnCCD images with optional condition (pp101)
00101: Running average of front pnCCD images with optional condition (pp101)
00102: Running average of back pnCCD images with optional condition (pp101)
00103: Running average of back pnCCD images with optional condition (pp101)
00104: Running average of commercial ccd images with optional condition (pp101)
00105: Running average of commercial ccd images with optional condition (pp101)

00110: Commercial CCD Image with just the detected photonHits (pp110)
00111: Front pnCCD Image with just the detected photonHits (pp110)
00112: Back pnCCD Image with just the detected photonHits (pp110)
00113: Commercial CCD photonhits in a 1D Histogram (pp113)
00114: Front pnCCD photonhits in a 1D Histogram (pp113)
00115: Back pnCCD photonhits in a 1D Histogram (pp113)
00116: Commercial CCD photonhits in a 1D Histogram energies in eV (pp116)
00117: Front pnCCD photonhits in a 1D Histogram energies in eV (pp116)
00118: Back pnCCD photonhits in a 1D Histogram energies in eV (pp116)

00131: Scalar value of the <cos^2\theta> derived from the 121 image
00141: Sum-intensity of image pp3
00142: Sum-intensity of image pp121
00143: Gaussian width of image pp3
00144: Gaussian height of image pp3
00145: Gaussian width of image pp121
00146: Gaussian height of image pp121
00150: Scalar value of <cos^2\theta> from averaged Opal camera image pp105 (pp150)

00160: photonhits using running average for front pnCCD image, optional condition(pp160)
00161: photonhits using running average for front pnCCD image, optional condition (pp160)
00162: photonhits using running average for back  pnCCD image, optional condition(pp160)
00163: photonhits using running average for back  pnCCD image, optional condition (pp160)
00164: photonhits using running average for commercial CCD image, optional condition(pp160)
00165: photonhits using running average for commercial CCD image, optional condition (pp160)

00166: 1D photonhits using running average for front pnCCD image, optional condition(pp166)
00167: 1D photonhits using running average for front pnCCD image, optional condition (pp166)
00168: 1D photonhits using running average for back  pnCCD image, optional condition(pp166)
00169: 1D photonhits using running average for back  pnCCD image, optional condition (pp166)
00170: 1D photonhits using running average for commercial CCD image, optional condition(pp166)
00171: 1D photonhits using running average for commercial CCD image, optional condition (pp166)

00500: Averaged waveform of Camp Acqiris Channel 00 (pp500)
00501: Averaged waveform of Camp Acqiris Channel 01 (pp500)
00502: Averaged waveform of Camp Acqiris Channel 02 (pp500)
00503: Averaged waveform of Camp Acqiris Channel 03 (pp500)
00504: Averaged waveform of Camp Acqiris Channel 04 (pp500)
00505: Averaged waveform of Camp Acqiris Channel 05 (pp500)
00506: Averaged waveform of Camp Acqiris Channel 06 (pp500)
00507: Averaged waveform of Camp Acqiris Channel 07 (pp500)
00508: Averaged waveform of Camp Acqiris Channel 08 (pp500)
00509: Averaged waveform of Camp Acqiris Channel 09 (pp500)
00510: Averaged waveform of Camp Acqiris Channel 10 (pp500)
00511: Averaged waveform of Camp Acqiris Channel 11 (pp500)
00512: Averaged waveform of Camp Acqiris Channel 12 (pp500)
00513: Averaged waveform of Camp Acqiris Channel 13 (pp500)
00514: Averaged waveform of Camp Acqiris Channel 14 (pp500)
00515: Averaged waveform of Camp Acqiris Channel 15 (pp500)
00516: Averaged waveform of Camp Acqiris Channel 16 (pp500)
00517: Averaged waveform of Camp Acqiris Channel 17 (pp500)
00518: Averaged waveform of Camp Acqiris Channel 18 (pp500)
00519: Averaged waveform of Camp Acqiris Channel 19 (pp500)
00520: Averaged waveform of ITof Acqiris Channel 00 (pp500)
00521: Averaged waveform of ITof Acqiris Channel 01 (pp500)
00522: Averaged waveform of ITof Acqiris Channel 02 (pp500)
00523: Averaged waveform of ITof Acqiris Channel 03 (pp500)

---Hex Anode Postprocessors---
00550: Hex Anode Number of Peaks in MCP (pp550)
00551: Hex Anode Number of Peaks in U1 (pp551)
00552: Hex Anode Number of Peaks in U2 (pp551)
00553: Hex Anode Number of Peaks in V1 (pp551)
00554: Hex Anode Number of Peaks in V2 (pp551)
00555: Hex Anode Number of Peaks in W1 (pp551)
00556: Hex Anode Number of Peaks in W2 (pp551)

00557: Hex Anode Hit Ratio U1 / U2 (pp557)
00558: Hex Anode Hit Ratio U1 / MCP (pp558)
00559: Hex Anode Hit Ratio U2 / MCP (pp558)

00560: Hex Anode Hit Ratio V1 / V2 (pp557)
00561: Hex Anode Hit Ratio V1 / MCP (pp558)
00562: Hex Anode Hit Ratio V2 / MCP (pp558)

00563: Hex Anode Hit Ratio W1 / W2 (pp557)
00564: Hex Anode Hit Ratio W1 / MCP (pp558)
00565: Hex Anode Hit Ratio W2 / MCP (pp558)

00566: Hex Anode Hit Ratio Reconstructed / MCP (pp566)

00567: Hex Anode All Hits on MCP (pp567)

00568: Hex Anode Delayline Timesum U (pp568)
00569: Hex Anode Delayline Timesum V (pp568)
00570: Hex Anode Delayline Timesum W (pp568)
00571: Hex Anode Delayline Timesum U vs Pos U (pp571)
00572: Hex Anode Delayline Timesum V vs Pos V (pp571)
00573: Hex Anode Delayline Timesum W vs Pos W (pp571)

00574: Hex Anode Delayline Picture of First Hit UV Layers in ns (pp574)
00575: Hex Anode Delayline Picture of First Hit UW Layers in ns (pp574)
00576: Hex Anode Delayline Picture of First Hit VW Layers in ns (pp574)

00578: Hex Anode Delayline Picture all Hits in mm (pp578)
00579: Hex Anode Delayline X vs Tof (pp578)
00580: Hex Anode Delayline Y vs Tof (pp578)

00581: Hex Anode Height vs. Fwhm MCP (pp581)
00582: Hex Anode Height vs. Fwhm U1 (pp582)
00583: Hex Anode Height vs. Fwhm U2 (pp582)
00584: Hex Anode Height vs. Fwhm V1 (pp582)
00585: Hex Anode Height vs. Fwhm V2 (pp582)
00586: Hex Anode Height vs. Fwhm W1 (pp582)
00587: Hex Anode Height vs. Fwhm W2 (pp582)

---Quad Anode Postprocessors---
00600: Quad Anode Number of Peaks in MCP (pp550)
00601: Quad Anode Number of Peaks in X1 (pp551)
00602: Quad Anode Number of Peaks in X2 (pp551)
00603: Quad Anode Number of Peaks in Y1 (pp551)
00604: Quad Anode Number of Peaks in Y2 (pp551)

00605: Quad Anode Hit Ratio X1 / X2 (pp557)
00606: Quad Anode Hit Ratio X1 / MCP (pp558)
00607: Quad Anode Hit Ratio X2 / MCP (pp558)

00608: Quad Anode Hit Ratio Y1 / Y2 (pp557)
00609: Quad Anode Hit Ratio Y1 / MCP (pp558)
00610: Quad Anode Hit Ratio Y2 / MCP (pp558)

00611: Quad Anode Hit Ratio Reconstructed / MCP (pp566)

00612: Quad Anode All Hits on MCP (pp567)

00613: Quad Anode Delayline Timesum X (pp568)
00614: Quad Anode Delayline Timesum Y (pp568)
00615: Quad Anode Delayline Timesum X vs Pos X (pp571)
00616: Quad Anode Delayline Timesum Y vs Pos Y (pp571)

00617: Quad Anode Delayline Picture of First Hit in ns (pp574)

00618: Quad Anode Delayline Picture all Hits in mm (pp578)
00619: Quad Anode Delayline X vs Tof (pp578)
00620: Quad Anode Delayline Y vs Tof (pp578)

00621: Quad Anode Height vs. Fwhm MCP (pp581)
00622: Quad Anode Height vs. Fwhm X1 (pp582)
00623: Quad Anode Height vs. Fwhm X2 (pp582)
00624: Quad Anode Height vs. Fwhm Y1 (pp582)
00625: Quad Anode Height vs. Fwhm Y2 (pp582)

---VMI Mcp Postprocessors--
00650: VMIMcp Number of Peaks in Waveform (pp550)
00651: VMIMcp All Hits on Mcp (pp567)
00652: VMIMcp Height vs. Fwhm (pp581)

---Beamdump Postprocessors--
00660: FEL Beam Monitor Number of Peaks in Waveform (pp550)
00661: FEL Beam Monitor All Hits on Mcp (pp567)
00662: FEL Beam Monitor Height vs. Fwhm (pp581)

---YAG Laser Diode Postprocessors--
00670: YAG Laser Photodiode Number of Peaks in Waveform (pp550)
00671: YAG Laser Photodiode All Hits on Mcp (pp567)
00672: YAG Laser Photodiode Height vs. Fwhm (pp581)

---TiSaph Laser Diode Postprocesors--
00680: Femtosecond Laser Photodiode Number of Peaks in Waveform (pp550)
00681: Femtosecond Laser Photodiode All Hits on Mcp (pp567)
00682: Femtosecond Laser Photodiode Height vs. Fwhm (pp581)

00700: PIPICo on Hexdetector (pp700)
00701: PIPICo of Hex and QuadDetector (pp700)

---Operants on histograms--
00106: Difference between choosable averaged CCD images (pp106)
00107: Difference between choosable averaged CCD images (pp106)
00800: Compare two histograms whether first is less than second (pp800)
00801: Compare two histograms whether first is equal to second (pp801)
00802: Divide first histogram by second histogram (pp802)
00803: Multiply first histogram with second histogram (pp803)
00804: Multiply histogram with constant (pp804)
00805: calc integral on 1d histogram between two boarders (pp805)
00806: project 2d histogram to a given axis within chosen boarders (pp806)

---HDF5 output postprocessors--
01001: Dump front and back pnCCD images (and more...) to HDF5
@endverbatim

@section add_pp Howto add custom postprocessors

@subsection nec Things that a postprocessor needs to have

Your postprocessor needs to have the following members
- a constructor that takes the a reference to the histogram container and the processor id
- overloaded void operator()(const cass::CASSEvent&) which gets called for each event
- (optionaly you could have a pointer the histogram in the histogram container)
- you are responsible that the histogram get allocated and destructed.

@subsection steps Register postprocessor

Steps that one has to take in order to have a custom build postprocessor registered to the list of postprocessors:
- add your number to the above list an shortly describe what the postprocessor
will be doing.
- add a describtive enum to the id_t enum
- add your postprossor in the switch statement of cass::PostProcessors::create
- if the Object you are writing is responsible for more than one postprocessor
just follow the example of the last pnccd processor(pp1).

@subsection doc Documentation

Please document what your postprocessor does so that other people now what it does. When documenting
please use doxygen style as then your documentation will be available on the webserver.
*/
class CASSSHARED_EXPORT PostProcessors : public QObject
{
    Q_OBJECT;

public:

    /** List of all currently registered postprocessors
     *
     * Keep this fully list synchronized with the documentation in the class header!
     * @todo clean up list once the id for a pp is pair<id_t,size_t>
     */
    enum id_t
    {
        Pnccd1LastImage=1, Pnccd2LastImage=2, VmiCcdLastImage=3,
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
        ITofChannel00LastWaveform=24,
        ITofChannel01LastWaveform=25,
        ITofChannel02LastWaveform=26,
        ITofChannel03LastWaveform=27,
        FirstPnccdFrontBinnedConditionalRunningAverage=100, SecondPnccdFrontBinnedConditionalRunningAverage=101,
        FirstPnccdBackBinnedConditionalRunningAverage=102, SecondPnccdBackBinnedConditionalRunningAverage=103,
        FirstCommercialCCDBinnedConditionalRunningAverage=104, SecondCommercialCCDBinnedConditionalRunningAverage=105,
        FirstImageSubstraction=106, SecondImageSubstraction=107,
        VMIPhotonHits=110, PnCCDFrontPhotonHits=111, PnCCDBackPhotonHits=112,
        VMIPhotonHits1d=113, PnCCDFrontPhotonHits1d=114, PnCCDBackPhotonHits1d=115,
        VMIPhotonHitseV1d=116, PnCCDFrontPhotonHitseV1d=117, PnCCDBackPhotonHitseV1d=118,
        VmiRunningAverage=121, VmiCos2Theta=131,
        Integral3=141, Integral121=142,
        GaussWidth3=143, GaussHeight3=144, GaussWidth121=145, GaussHeight121=146,
        VmiFixedCos2Theta=150,

        AdvancedPhotonFinderFrontPnCCD=160,
        AdvancedPhotonFinderFrontPnCCDTwo=161,
        AdvancedPhotonFinderBackPnCCD=162,
        AdvancedPhotonFinderBackPnCCDTwo=163,
        AdvancedPhotonFinderCommercialCCD=164,
        AdvancedPhotonFinderCommercialCCDTwo=165,

        AdvancedPhotonFinderFrontPnCCD1dHist=166,
        AdvancedPhotonFinderFrontPnCCDTwo1dHist=167,
        AdvancedPhotonFinderBackPnCCD1dHist=168,
        AdvancedPhotonFinderBackPnCCDTwo1dHist=169,
        AdvancedPhotonFinderCommercialCCD1dHist=170,
        AdvancedPhotonFinderCommercialCCDTwo1dHist=171,

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
        ITofChannel00AveragedWaveform=520,
        ITofChannel01AveragedWaveform=521,
        ITofChannel02AveragedWaveform=522,
        ITofChannel03AveragedWaveform=523,

        HexMCPNbrSignals=550,
        HexU1NbrSignals=551,
        HexU2NbrSignals=552,
        HexV1NbrSignals=553,
        HexV2NbrSignals=554,
        HexW1NbrSignals=555,
        HexW2NbrSignals=556,
        HexU1U2Ratio=557,
        HexU1McpRatio=558,
        HexU2McpRatio=559,
        HexV1V2Ratio=560,
        HexV1McpRatio=561,
        HexV2McpRatio=562,
        HexW1W2Ratio=563,
        HexW1McpRatio=564,
        HexW2McpRatio=565,
        HexRekMcpRatio=566,
        HexAllMcp=567,
        HexTimesumU=568,
        HexTimesumV=569,
        HexTimesumW=570,
        HexTimesumUvsU=571,
        HexTimesumVvsV=572,
        HexTimesumWvsW=573,
        HexFirstUV=574,
        HexFirstUW=575,
        HexFirstVW=576,
        HexXY=578,
        HexXT=579,
        HexYT=580,
        HexHeightvsFwhmMcp=581,
        HexHeightvsFwhmU1=582,
        HexHeightvsFwhmU2=583,
        HexHeightvsFwhmV1=584,
        HexHeightvsFwhmV2=585,
        HexHeightvsFwhmW1=586,
        HexHeightvsFwhmW2=587,

        QuadMCPNbrSignals=600,
        QuadX1NbrSignals=601,
        QuadX2NbrSignals=602,
        QuadY1NbrSignals=603,
        QuadY2NbrSignals=604,
        QuadX1X2Ratio=605,
        QuadX1McpRatio=606,
        QuadX2McpRatio=607,
        QuadY1Y2Ratio=608,
        QuadY1McpRatio=609,
        QuadY2McpRatio=610,
        QuadRekMcpRatio=611,
        QuadAllMcp=612,
        QuadTimesumX=613,
        QuadTimesumY=614,
        QuadTimesumXvsX=615,
        QuadTimesumYvsY=616,
        QuadFirstXY=617,
        QuadXY=618,
        QuadXT=619,
        QuadYT=620,
        QuadHeightvsFwhmMcp=621,
        QuadHeightvsFwhmX1=622,
        QuadHeightvsFwhmX2=623,
        QuadHeightvsFwhmY1=624,
        QuadHeightvsFwhmY2=625,

        VMIMcpNbrSignals=650,
        VMIMcpAllMcp=651,
        VMIMcpHeightvsFwhmMcp=652,

        FELBeamMonitorNbrSignals=660,
        FELBeamMonitorAllMcp=661,
        FELBeamMonitorHeightvsFwhmMcp=662,

        YAGPhotodiodeNbrSignals=670,
        YAGPhotodiodeAllMcp=671,
        YAGPhotodiodeHeightvsFwhmMcp=672,

        FsPhotodiodeNbrSignals=680,
        FsPhotodiodeAllMcp=681,
        FsPhotodiodeHeightvsFwhmMcp=682,

        HexPIPICO=700,
        HexQuadPIPICO=701,

        PnccdHDF5=1001,

        InvalidPP
    };

    /** Container of all currently available histograms */
    typedef std::map<id_t, HistogramBackend*> histograms_t;

    /** Container of all currently actice postprocessors */
    typedef std::map<id_t, PostprocessorBackend*> postprocessors_t;

    /** List of active postprocessors */
    typedef std::list<id_t> active_t;

    /** create the instance if not it does not exist already.
     * @todo add a string or const char * to pass the outputfilename to the pp
     */
    static PostProcessors *instance();

    /** destroy the instance */
    static void destroy();

    /** process event
     *
     *@param event CASSEvent to process by all active postprocessors
     */
    void process(CASSEvent& event);

    /** Histogram storage access
     *
     * We only allow read access to the histograms container. Obtaining acces will immediately put a
     * lock on the container. You must release this with histograms_release.
     *
     * @return Histogram storage
     */
    const histograms_t &histograms_checkout()
    {
      if(! _histlock.tryLockForRead(100))
        _histlock.lockForWrite();
      return _histograms;
    };

    /** release read-lock for histograms container */
    void histograms_release() { _histlock.unlock(); };

    /** Replace histogram in storage
     *
     * @param type Histogram to replace
     * @param hist New histogram to store
     */
    void histograms_delete(id_t type) { _histlock.lockForWrite(); _delete(type); _histlock.unlock(); };

    /** Remove histogram from storage
     *
     * @param type Histogram to remove
     */
    void histograms_replace(id_t type, HistogramBackend *hist)
    {
      _histlock.lockForWrite();
      _replace(type, hist);
      _histlock.unlock();
    };

    /** make sure a specific histogram exists and is not 0
     *
     * This requires that locking is done outside!
     */
    void validate(id_t type)
    {
        if((_histograms.end() == _histograms.find(type)) || (0 == _histograms[type]))
            throw InvalidHistogramError(type);
    };

    IdList* getIdList();
    std::string& getMimeType(id_t type);

public slots:
    /** Load active postprocessors and histograms
     *
     * Reset set of active postprocessors/histograms based on cass.ini
     */
    void loadSettings(size_t);

    /** Save active postprocessors and histograms */
    void saveSettings() {}

protected:
    /** @brief (ordered) list of active postprocessors/histograms
     *
     * This list has order, i.e., postprocessors are called in the specified order. You can rely on the
     * result of a postprocessor earlier in the list, but not on one that only occurs further back...
     */
    active_t _active;

    /** the list of id's */
    IdList * _IdList;

    /** a string @todo document what this is */
    std::string _invalidMime;

    /** container for all histograms */
    histograms_t _histograms;

    /** container for registered (active) postprocessors */
    postprocessors_t _postprocessors;

    /** Create new Postprocessor for specified id and using the specified histogram container
     *
     * @param[in] hs reference to the histogram container
     * @param[in] id the id of the postprocessor
     */
    PostprocessorBackend * create(id_t id);

    /** Set up _histograms and _postprocessors using current _active*/
    void setup();

    /** Internal method to actually remove histogram from storage
     *
     * This requires that locking is done outside!
     *
     * @param type Histogram to remove
     */
    void _delete(id_t type);

    /** Internal method to actually replace histogram from storage
     *
     * This requires that locking is done outside!
     *
     * @param type Histogram to replace
     * @param hist New histogram to store
     */
    void _replace(id_t type, HistogramBackend *hist);

    /** histogram container lock */
    QReadWriteLock _histlock;

    /** filename of the output file */
    string _outputfilename;

private:
    /** Private constructor of singleton
     * @todo enable passing the filename to the pp.
     */
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


/** idlist
 *
 * @todo document this class
 * @todo if possible put this class into a separate file.
 * @note do all these function need to be inlines?
 */
class IdList : public Serializable
{
public:
   IdList() : Serializable(1), _size(0) {}
   IdList( PostProcessors::active_t & list) : Serializable(1), _list(list), _size(list.size()) {}
   IdList( SerializerBackend* in) : Serializable(1) {
      deserialize(in);
   }
   IdList( SerializerBackend &in) : Serializable(1) {
      deserialize(in);
   }

   void clear() {
      _list.clear();
      _size=0;
   }

   void setList(PostProcessors::active_t& list) {
      clear();
      _list=list;
      _size=list.size();
   }

   PostProcessors::active_t& getList() {
      return _list;
   }

   void deserialize(SerializerBackend& in) {
      deserialize(&in);
   }

   void deserialize(SerializerBackend *in) {
      //check whether the version fits//
      uint16_t ver = in->retrieveUint16();
      if(ver!=_version)
      {
        std::cerr<<"version conflict in IdList: "<<ver<<" "<<_version<<std::endl;
        return;
      }
      //number of bins, lower & upper limit
      _size     = in->retrieveSizet();
      _list.clear();
      for (size_t ii=0;ii<_size;ii++)
         _list.push_back(static_cast<PostProcessors::id_t>(in->retrieveUint16()));
   }

   void serialize(SerializerBackend &out) {
      serialize(&out);
   }

   void serialize(SerializerBackend *out) {
      //
      out->addUint16(_version);
      out->addSizet(_size);
      for (PostProcessors::active_t::iterator it=_list.begin(); it!=_list.end(); it++)
         out->addUint16(*it);
   }

private:
   PostProcessors::active_t _list;
   size_t _size;
};






} // namespace cass


#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "Stroustrup"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:
