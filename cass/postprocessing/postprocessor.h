// Copyright (C) 2010 lmf
// Copyright (C) 2010 Jochen Kuepper

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

    All currently registered postprocessors are listed here, specifying their id, a description, and
    (in parenthesis) the PostProcessor group they belong to (REMI, VMI, pnCCD, other) The
    postprocessor classes are named according to their number (or the first number for classes
    handling multiple histograms) and placed in the file for the most appropriate group according to
    their content.

    @verbatim
    00001: Last plain image from pnCCD-1 (pnCCD)
    00002: Last plain image from pnCCD-2 (pnCCD)
    00003: Last plain image from VMI ccd camera (VMI)
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
    00121: Running average of VMI (???) camera
    00131: Scalar value of the <cos^2\theta>_{2D} derived from the 121 image
    00141: Sum-intensity of image pp3
    00142: Sum-intensity of image pp121
    00143: Gaussian width of image pp3
    00144: Gaussian height of image pp3
    00145: Gaussian width of image pp121
    00146: Gaussian height of image pp121
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
    00550: Hex Anode Number of Peaks in MCP
    00551: Hex Anode Number of Peaks in U1
    00552: Hex Anode Number of Peaks in U2
    00553: Hex Anode Number of Peaks in V1
    00554: Hex Anode Number of Peaks in V2
    00555: Hex Anode Number of Peaks in W1
    00556: Hex Anode Number of Peaks in W2
    00557: Hex Anode Hit Ratio U1 / U2
    00558: Hex Anode Hit Ratio U1 / MCP
    00559: Hex Anode Hit Ratio U2 / MCP
    00560: Hex Anode Hit Ratio V1 / V2
    00561: Hex Anode Hit Ratio V1 / MCP
    00562: Hex Anode Hit Ratio V2 / MCP
    00563: Hex Anode Hit Ratio W1 / W2
    00564: Hex Anode Hit Ratio W1 / MCP
    00565: Hex Anode Hit Ratio W2 / MCP
    00566: Hex Anode Hit Ratio Reconstructed / MCP
    00567: Hex Anode Delayline Timesum U
    00568: Hex Anode Delayline Timesum V
    00569: Hex Anode Delayline Timesum W
    00570: Hex Anode Delayline Timesum U vs Pos U
    00571: Hex Anode Delayline Timesum V vs Pos V
    00572: Hex Anode Delayline Timesum W vs Pos W
    00573: Hex Anode Delayline Picture of First Hit UV Layers in ns
    00574: Hex Anode Delayline Picture of First Hit UW Layers in ns
    00575: Hex Anode Delayline Picture of First Hit VW Layers in ns
    00576: Hex Anode Delayline Picture of First Hit All Layers in ns
    00578: Hex Anode Delayline Picture all Hits in mm
    00579: Hex Anode Delayline X vs Tof
    00580: Hex Anode Delayline Y vs Tof
    00600: Quad Anode Number of Peaks in MCP
    00601: Quad Anode Number of Peaks in X1
    00602: Quad Anode Number of Peaks in X2
    00603: Quad Anode Number of Peaks in Y1
    00604: Quad Anode Number of Peaks in Y2
    00605: Quad Anode Hit Ratio X1 / X2
    00606: Quad Anode Hit Ratio X1 / MCP
    00607: Quad Anode Hit Ratio X2 / MCP
    00608: Quad Anode Hit Ratio Y1 / Y2
    00609: Quad Anode Hit Ratio Y1 / MCP
    00610: Quad Anode Hit Ratio Y2 / MCP
    00611: Quad Anode Hit Ratio Reconstructed / MCP
    00612: Quad Anode All Hits on MCP
    00613: Quad Anode Delayline Timesum X
    00614: Quad Anode Delayline Timesum Y
    00615: Quad Anode Delayline Timesum X vs Pos X
    00616: Quad Anode Delayline Timesum Y vs Pos Y
    00617: Quad Anode Delayline Picture of First Hit in ns
    00618: Quad Anode Delayline Picture all Hits in mm
    00619: Quad Anode Delayline X vs Tof
    00620: Quad Anode Delayline Y vs Tof
    @endverbatim
    */
    class CASSSHARED_EXPORT PostProcessors : public QObject
    {
        Q_OBJECT;

    public:

        /** List of all currently registered postprocessors

        Keep this fully list synchronized with the documentation in the class header!
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
            Pnccd1BinnedRunningAverage=101, Pnccd1BackgroundCorrectedBinnedRunnngAverage=102,
            VmiRunningAverage=121, VmiCos2Theta=131,
            Integral3=141, Integral121=142,
            GaussWidth3=143, GaussHeight3=144, GaussWidth121=145, GaussHeight121=146,
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

        /*! Load active postprocessors and histograms

        Reset set of active postprocessors/histograms based on cass.ini */
        void loadSettings(size_t);

        /*! Save active postprocessors and histograms */
        void saveSettings() {}


    protected:

        /*! @brief (ordered) list of active postprocessors/histograms

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
// c-file-style: "Stroustrup"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:
