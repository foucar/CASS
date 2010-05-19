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
    explicit InvalidHistogramError(const std::string &name)
      : std::out_of_range("Invalid histogram requested!"), _name(name)
    {}

    virtual const char* what() const throw()
    {
      std::ostringstream msg;
      msg << "Invalid histogram " << _name << " requested!";
      return msg.str().c_str();
    }

    virtual ~InvalidHistogramError() throw();

  protected:
    std::string _name;
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
@verbatim
---Operations--
00000: Compare 0D histograms for less than constant
00001: Compare 0D histograms for greater than constant
00002: Compare 0D histograms for equal to constant
00003: Apply 0D histograms for boolean XOR
00004: Compare two 0D histograms for boolean AND
00005: Compare two 0D histograms for boolean OR
00006: Compare two histograms whether first is less than second
00007: Compare two histograms whether first is equal to second

00020: Difference between choosable averaged CCD images
00021: Divide first histogram by second histogram
00022: Multiply first histogram with second histogram
00023: Multiply histogram with constant

00050: Project 2D histogram onto a axis
00051: Integral of 1D histogram

00060: Histogram values from a 0D histogram
00061: Average of a histogram
00062: Summing up of histogram

---Data retrieval (Histogram contain only last shot)--
00100: CCD image
00110: Photonhits Spectrum
00111: Photonhits Image
00120: Acqiris Waveform
00130: Beamline data
00140: Epics data
00150: TofDetector number of signals in MCP waveform
00151: TofDetector all signals
00152: TofDetector signal height vs. fwhm
00160: Delayline wireend number of signals
00162: Delayline wireend signal height vs. fwhm
00163: Delayline timesum on anode
00164: Delayline timesum on anode vs. position
00165: Delayline image of first good hit
00167: Delayline reconstructed Number of detectorhits
00168: Delayline data of all reconstructed detectorhits

---Data analysis--
00200: Scalar value of <cos^2\theta> from 2D Histogram
00210: Advanced photonhit finder Image
00211: Advanced photonhit finder Spectrum

---Output
01000: Dump front and back pnCCD images (and more...) to HDF5
02000: Dump all 1d and 2d cass histograms to root file
@endverbatim

@section add_pp Howto add custom postprocessors
@todo updated this section for new layout
@subsection nec Things that a postprocessor needs to have

Your postprocessor needs to have the following members
- a constructor that takes the a reference to the histogram container and the
  processor id
- overloaded void operator()(const cass::CASSEvent&) which gets called for each
  event
- (optionaly you could have a pointer the histogram in the histogram container)
- you are responsible that the histogram get allocated and destructed.

@subsection steps Register postprocessor

Steps that one has to take in order to have a custom build postprocessor registered
to the list of postprocessors:
- add your number to the above list an shortly describe what the postprocessor
  will be doing.
- add a describtive enum to the id_t enum
- add your postprossor in the switch statement of cass::PostProcessors::create
- if the Object you are writing is responsible for more than one postprocessor
  just follow the example of the last pnccd processor(pp1).

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

      ROOTDump=2000,

      InvalidPP
    };
    /** type of postproccessor accessor key */
    typedef std::string key_t;

    /** Container of all currently available histograms */
    typedef std::map<key_t, HistogramBackend*> histograms_t;

    /** Container of all currently actice postprocessors */
    typedef std::map<key_t, PostprocessorBackend*> postprocessors_t;

    /** List of active postprocessors */
    typedef std::list<key_t> active_t;


    /** create the instance if not it does not exist already.
     * @param outputfilename filename of the outputfile
     */
    static PostProcessors *instance(std::string outputfilename);

    /** destroy the instance */
    static void destroy();

    /** process event
     *
     * @param event CASSEvent to process by all active postprocessors
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
    void histograms_delete(key_t key) { _histlock.lockForWrite(); _delete(key); _histlock.unlock(); };

    /** Remove histogram from storage
     *
     * @param type Histogram to remove
     */
    void histograms_replace(key_t key, HistogramBackend *hist)
    {
      _histlock.lockForWrite();
      _replace(key, hist);
      _histlock.unlock();
    };

    /** make sure a specific histogram exists and is not 0
     *
     * This requires that locking is done outside!
     */
    void validate(std::string name)
    {
      if((_histograms.end() == _histograms.find(name)) || (0 == _histograms[name]))
        throw InvalidHistogramError(name);
    };

    IdList* getIdList();
    const std::string& getMimeType(key_t);

  public slots:
    /** Load active postprocessors and histograms
     *
     * Reset set of active postprocessors/histograms based on cass.ini
     */
    void loadSettings(size_t);

    /** Save active postprocessors and histograms */
    void saveSettings() {}

    /** clear the histogram that has id */
    void clear(key_t);

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
     * @param[in] key the key of the postprocessor
     */
    PostprocessorBackend * create(key_t key);

    /** Set up _histograms and _postprocessors using current _active*/
    void setup();

    /** Internal method to actually remove histogram from storage
     *
     * This requires that locking is done outside!
     *
     * @param key Histogram to remove
     */
    void _delete(key_t key);

    /** Internal method to actually replace histogram from storage
     *
     * This requires that locking is done outside!
     *
     * @param type Histogram to replace
     * @param hist New histogram to store
     */
    void _replace(key_t key, HistogramBackend *hist);

    /** histogram container lock */
    QReadWriteLock _histlock;

    /** filename of the output file */
    std::string _outputfilename;

  private:
    /** Private constructor of singleton
     * @param outputfilename filename of the file containing the results. Used
     *         in offline mode.
     */
    PostProcessors(std::string outputfilename);

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


  /** id-list
   *
   * used for SOAP communication of id-lists
   *
   * @todo document this class
   * @todo if possible put this class into a separate file.
   * @note do all these function need to be inlines?
   */
  class IdList : public Serializable
  {
  public:

    IdList()
      : Serializable(1), _size(0)
    {}

    IdList(PostProcessors::active_t& list)
      : Serializable(1), _list(list), _size(list.size())
    {
      std::cerr << "Initial list size = " << _size << std::endl;
    }

    IdList(SerializerBackend* in) : Serializable(1)
    {
      deserialize(in);
    }

    IdList( SerializerBackend &in) : Serializable(1)
    {
      deserialize(in);
    }

    void clear()
    {
      _list.clear();
      _size=0;
    }

    void setList(PostProcessors::active_t& list)
    {
      clear();
      _list = list;
      _size = list.size();
    }

    PostProcessors::active_t& getList()
    {
      return _list;
    }

    bool deserialize(SerializerBackend& in)
    {
      return deserialize(&in);
    }

    bool deserialize(SerializerBackend *in)
    {
      _list.clear();
      //check whether the version fits//
      in->startChecksumGroupForRead();
      uint16_t ver = in->retrieveUint16();
      if(ver != _version)
      {
        std::cerr<<"version conflict in IdList: "<<ver<<" "<<_version<<std::endl;
        return false;
      }
      //number of bins, lower & upper limit
      _size = in->retrieveSizet();
      std::cerr << "list size " << _size << std::endl;
      if (!in->endChecksumGroupForRead())
      {
        std::cerr<<"wrong checksum IdList"<<std::endl;
        return false;
      }
      for(size_t ii=0; ii<_size; ++ii)
        _list.push_back("temp");
      std::cerr << "list is done " << std::endl;
      return true;
    }

    void serialize(SerializerBackend &out)
    {
      serialize(&out);
    }

    void serialize(SerializerBackend *out)
    {
      out->startChecksumGroupForWrite();
      out->addUint16(_version);
      out->addSizet(_size);
      out->endChecksumGroupForWrite();
      for (PostProcessors::active_t::iterator it=_list.begin(); it!=_list.end(); it++)
        out->addString(*it);
    }

  private:
    PostProcessors::active_t _list;
    size_t _size;
  };






  /** function to set the 1d histogram properties from the ini file.
   * @param[out] hist pointer to the 1D Histogram whos properties should be updated
   *            (will be deleted and created with new settings)
   * @param[in] key the key of the postprocessor too look up in cass.ini
   * @author Lutz Foucar
   */
  void set1DHist(cass::Histogram1DFloat*& hist, PostProcessors::key_t key);

  /** function to set the 2d histogram properties from the ini file.
   * @param[out] hist pointer to the 2D Histogram whos properties should be updated
   *            (will be deleted and created with new settings)
   * @param[in] key the key of the postprocessor too look up in cass.ini
   * @author Lutz Foucar
   */
  void set2DHist(cass::Histogram2DFloat*& hist, PostProcessors::key_t key);



} // namespace cass


#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "gnu"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:
