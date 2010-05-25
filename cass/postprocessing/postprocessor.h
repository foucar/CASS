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
    explicit InvalidHistogramError(const std::string &key)
      : std::out_of_range("Invalid histogram requested!"), _key(key)
    {}

    virtual const char* what() const throw()
    {
      std::ostringstream msg;
      msg << "Invalid histogram " << _key << " requested!";
      return msg.str().c_str();
    }

    virtual ~InvalidHistogramError() throw(){}

  protected:
    std::string _key;
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



/** container and call handler for all registered postprocessors.
 * @todo create function that will find all pp that depend on a given pp

All currently registered postprocessors are listed here, specifying their id and a
short description. The postprocessor classes are named according to their number
and placed in the file for the most appropriate group according to their content.

@section pplist List of Postprocessors
The Classes that implement the postprocessor are pointed out in parenthesis. See
the Class description for information about what parameters are user settable.
(Keep in mind that cases matter)
@verbatim
---Operations--
00001: Compare 0D histograms for less than constant
00002: Compare 0D histograms for greater than constant
00003: Compare 0D histograms for equal to constant
00004: Apply boolean NOT to 0D histograms
00005: Compare two 0D histograms for boolean AND
00006: Compare two 0D histograms for boolean OR
00007: Compare two histograms whether first is less than second
00008: Compare two histograms whether first is equal to second

00020: Difference between choosable averaged CCD images
00021: Divide first histogram by second histogram
00022: Multiply first histogram with second histogram
00023: Multiply histogram with constant
00024: Substract Constant

00050: Project 2D histogram onto a axis
00051: Integral of 1D histogram
00052: Project 2d to Radius
00053: Angular Distribution / Radar Plot

00060: Histogram values from a 0D histogram
00061: Average of a histogram
00062: Summing up of histogram

---Data retrieval (Histogram contain only last shot)--
00100: CCD image
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
00210: Advanced photonhit finder Image
00211: Advanced photonhit finder Spectrum
00220: PIPICO Spectrum
00230: Photon energy of Shot
00231: Wavelength of photons

<<<<<<< .working
---Output
01000: Dump front and back pnCCD images (and more...) to HDF5
=======
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

---TiSaph Laser Diode Postprocessors--
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
00807: Compare two 0D histograms for boolean AND (pp807)
00808: Compare two 0D histograms for boolean OR  (pp808)
00809: Compare 0D histograms for less than constant  (pp809)
00810: Compare 0D histograms for greater than constant  (pp810)
00811: Compare 0D histograms for equal to constant  (pp811)

---Machindata--
00850: access for beamline data
00851: access for epics data
00852: photonenergy from bld

---HDF5 output postprocessors--
01001: Dump front and back pnCCD images (and more...) to HDF5

---ROOT file ouput postprocessor--
>>>>>>> .merge-right.r2412
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
      ConstantLess=1,
      ConstantGreater=2,
      ConstantEqual=3,
      Xor=4,
      BooleanAND=5,
      BooleanOR=6,
      CompareForLess=7,
      CompareForEqual=8,

      SubstractHistograms=20,
      DivideHistograms=21,
      MultiplyHistograms=22,
      MultiplyConstant=23,
      SubstractConstant=24,

      TwoDProjection=50,
      OneDIntergral=51,

      ZeroDHistogramming=60,
      HistogramAveraging=61,
      HistogramSumming=62,

      SingleCcdImage=100,
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

      AdvancedPhotonFinder=210,
      AdvancedPhotonFinderSpectrum=211,

      PIPICO=220,

      PhotonEnergy=230,
      PhotonWavelength=231,

      PnccdHDF5=1000,
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
    void histograms_delete(const key_t &key)
    {
      _histlock.lockForWrite();
      _delete(key);
      _histlock.unlock();
    }

    /** Remove histogram from storage
     *
     * @param type Histogram to remove
     */
    void histograms_replace(const key_t &key, HistogramBackend *hist)
    {
      _histlock.lockForWrite();
      _replace(key, hist);
      _histlock.unlock();
    }

    /** make sure a specific histogram exists and is not 0
     *
     * This requires that locking is done outside!
     */
    void validate(const key_t &key)
    {
      if((_histograms.end() == _histograms.find(key)) || (0 == _histograms[key]))
        throw InvalidHistogramError(key);
    }

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

    /** defining an invalid mimetype */
    std::string _invalidMime;

    /** container for all histograms */
    histograms_t _histograms;

    /** container for registered (active) postprocessors */
    postprocessors_t _postprocessors;

    /** Create new Postprocessor for specified id and using the specified histogram container
     *
     * @param[in] key the key of the postprocessor
     */
    PostprocessorBackend * create(const key_t &key);

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
        _list.push_back(in->retrieveString());
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

  /** function to retrieve and validate a postprocessors dependency
   * @return true when the dependcy exists
   * @param[in] pp reference to the postprocessor instance that contains the histograms
   * @param[in] key the key of the postprocessor asking for another postprocessors id
   * @param[in] param_name paramenter name of the dependency in qsettings
   * @param[out] dependid reference to the pp id that we retrieve from qsettings
   */
  bool retrieve_and_validate(cass::PostProcessors &pp,
                             cass::PostProcessors::key_t key,
                             const char * param_name,
                             cass::PostProcessors::key_t &dependid);
} // namespace cass


#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "gnu"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:
