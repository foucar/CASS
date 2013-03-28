#ifndef __PARTIAL_COVARIANCE_H__
#define __PARTIAL_COVARIANCE_H__

#include "backend.h"
#include "cass_event.h"
#include "histogram.h"


namespace cass
{

/** converts a Electron Time of Flight trace to Energy
 *
 * converts a time of flight trace of electrons to the coresponding energy
 * using function \f$E = (\frac{\alpha}{(t-t_0)})^2-e_0\f$.
 *
 * @note One Energy point is created from many points in the ToF trace. This
 *       means that one has make sure that the Baseline is correct. Therefore
 *       this will determine the baseline first. One has to tell it the ToF
 *       region where no signal will appear, such that it can correctly
 *       determine the baseline.
 *
 * @cassttng PostProcessor/\%name\%/{TofLow|TofUp}\n
 *           The time of flight range within the histogram to convert to energy.
 *           Default is 0|1.
 * @cassttng PostProcessor/\%name\%/{t0}\n
 *           corrects the time of flight
 *           Default is 0.
 * @cassttng PostProcessor/\%name\%/{e0}\n
 *           Retard voltage
 *           Default is 0.
 * @cassttng PostProcessor/\%name\%/{alpha}\n
 *           Coefficient for converting to energy. Default is 0.
 * @cassttng PostProcessor/\%name\%/{NbrBins}\n
 *           number of bins in converted histogram. Default is 0.
 * @cassttng PostProcessor/\%name\%/{tb1}\n
 *           The lower limit in x for calculating the baseline. Default is 0.
 * @cassttng PostProcessor/\%name\%/{tb2}\n
 *           The upper limit in x for calculating the baseline. Default is 0.
 * @cassttng PostProcessor/\%name\%/{HistName}\n
 *           input 1D histogram that contains the time of flight wavetrace.
 *
 * @author Koji Motomura
 */
class pp400 : public PostprocessorBackend
{
public:
  /** constructor */
  pp400(PostProcessors& hist, const PostProcessors::key_t&);

  /** process event */
  virtual void process(const CASSEvent&);

  /** change the histogram, when told the the dependand histogram has changed */
  virtual void histogramsChanged(const HistogramBackend*);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** retrieve the time that corresponses to a given energy
   *
   * calculate the time using function
   * \f$t=\frac{\alpha}{\sqrt{e+e_0}}+t_0\f$.
   *
   * @return the calculated time
   * @param energy the energy that one wants to calc the time for
   */
  double calcEtoTof (double energy);

  /** make the histgram in energy scale.
   *
   * This function makes the histgram in energy scale.
   * Offset value is calculated by average in range of no signal.
   *
   * @param TofHisto  input ToF histgram
   * @param Energy result histgram converted to energy
   * @param offset baseline of wavetrace
   */
  void ToftoEnergy(const HistogramFloatBase& TofHisto,
                   HistogramFloatBase::storage_t& Energy,
                   double offset);

  /** pp containing input histogram */
  PostprocessorBackend *_pHist;

  /** the user requested Tof-axis limits */
  std::pair<float,float> _userTofRange;

  /** corrects the time of flight*/
  double t0;

  /** retardation voltage*/
  double e0;

  /** Coefficient to convert to energy*/
  double alpha;

  /** The lower limit for calculating baseline*/
  double tb1;

  /** The upper limit for calculating baseline*/
  double tb2;

  /** the calculated level of baseline*/
  double offset;

  /** number of bins in converted histgram*/
  size_t NbrBins;

  /** Upper limit converted to bin*/
  size_t binTofUp;

  /** lower limit converted to bin*/
  size_t binTofLow;

  /** t0 converted to bin*/
  size_t bint0;

  /** tb1 converted to bin*/
  size_t bintb1;

  /** tb2 converted to bin*/
  size_t bintb2;
};






/** calculate variance
 *
 * calculate the variance by this on-line algorithm
 * \f$var_n=\frac{1}{n}((n-1)var_{n-1}+(x_n-ave_n)(x_n-ave_{n-1}))\f$
 *
 * @cassttng PostProcessor/\%name\%/{HistName}\n
 *           input 0D or 1D histogram that we calculate variance.
 * @cassttng PostProcessor/\%name\%/{AveHistName}\n
 *           averaged histogram of "HistName"
 *
 * @author Koji Motomura
 */
class pp401 : public PostprocessorBackend
{
public:
  /** constructor */
  pp401(PostProcessors& hist, const PostProcessors::key_t&);

  /** process event */
  virtual void process(const CASSEvent&);

  /** load the settings */
  virtual void loadSettings(size_t);

  /** change the histogram, when told the the dependand histogram has changed */
  virtual void histogramsChanged(const HistogramBackend*);

protected:
  /** pp containing histogram to work on */
  PostprocessorBackend *_pHist;

  /** pp containing input histogram */
  PostprocessorBackend *_ave;

  /** function for variance.
   *
   * calculate by on-line algorithm
   *
   * @param data input histogram
   * @param averageOld averaged histgram at previous event
   * @param averageNew averaged histgram at present event
   * @param variance output histogram calculated variance
   * @param n number of present event
   */
  void calcVariance(const HistogramFloatBase::storage_t& data ,
                    const HistogramFloatBase::storage_t& averageOld,
                    const HistogramFloatBase::storage_t& averageNew,
                    HistogramFloatBase::storage_t& variance,
                    float n);

};






/** Histogram square averaging.
 *
 * Running or cummulative average of a histogram. In this case it will average
 * the squared value.
 *
 * @see PostprocessorBackend for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{NbrOfAverages}\n
 *           how many images should be averaged. When value is 0 its a cummulative
 *           average. Default is 1.
 * @cassttng PostProcessor/\%name\%/{HistName} \n
 *           Postprocessor name containing the histogram that we average.
 *
 * @author Lutz Foucar
 */
class pp402 : public PostprocessorBackend
{
public:
  /** constructor */
  pp402(PostProcessors& hist, const PostProcessors::key_t&);

  /** process event */
  virtual void process(const CASSEvent&);

  /** load the settings */
  virtual void loadSettings(size_t);

  /** change the histogram, when told the the dependand histogram has changed */
  virtual void histogramsChanged(const HistogramBackend*);

protected:
  /** alpha for the running average */
  float _alpha;

  /** pp containing histogram to work on */
  PostprocessorBackend *_pHist;
};








/** subset and rebin a 1d histogram
 *
 * Subsets a 1d histogram and rebins the bins.
 *
 * @cassttng PostProcessor/\%name\%/{XLow|XUp}\n
 *           The subset range one wants to use. Default is 0|1.
 * @cassttng PostProcessor/\%name\%/{BSize}\n
 *           How many bin should be summed together. If this number is 2 then
 *           Always two consecutive bins will be summed together. If this numer
 *           is 1 it will leave the number of bins therefore the binsize like in
 *           the original histogram. Default is 1.
 * @cassttng PostProcessor/\%name\%/{HistName}\n
 *           input histogram name that subset and rebin.
 *
 * @author Koji Motomura
 * @author Marco Siano
 */
class pp403 : public PostprocessorBackend
{
public:
  /** constructor */
  pp403(PostProcessors& hist, const PostProcessors::key_t&);

  /** process event */
  virtual void process(const CASSEvent&);

  /** change the histogram, when told the the dependand histogram has changed */
  virtual void histogramsChanged(const HistogramBackend*);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** pp containing input histogram */
  PostprocessorBackend *_pHist;

  /** offset of first bin in input in Histogram coordinates */
  size_t _inputOffset;

  /** the user requested x-axis limits */
  std::pair<float,float> _userXRange;

  /** the user requested y-axis limits */
  std::pair<float,float> _userYRange;

  /** the user requested bin size */
  size_t _userBinSize;
};



/** convert time of flight to charge to mass ratio
 *
 * converts a time of flight trace of ions to the coresponding mass to charge
 * ratio.
 *
 * In order to calculate the Mass to Charge ratio one has to provide two known
 * peaks and their Charge to Mass ratio. From this information the rest is
 * calculated.
 *
 * @cassttng PostProcessor/\%name\%/{TofLow|TofLow}\n
 *           The time of flight range within the histogram to convert to mass.
 *           to charge ratio Default is 0|1.
 * @cassttng PostProcessor/\%name\%/{t0}\n
 *           a time of flight of ion that we choosed. Default is 0.
 * @cassttng PostProcessor/\%name\%/{t1}\n
 *           another time of flight of ion that we choosed. Default is 0.
 * @cassttng PostProcessor/\%name\%/{MtC0}\n
 *           mass to charge ratio of t0. Default is 0.
 * @cassttng PostProcessor/\%name\%/{MtC1}\n
 *           mass to charge ratio of t1. Default is 0.
 * @cassttng PostProcessor/\%name\%/{tb1}\n
 *           The lower limit for calculating baseline. Default is 0.
 * @cassttng PostProcessor/\%name\%/{tb2}\n
 *           The upper limit for calculating baseline. Default is 0.
 * @cassttng PostProcessor/\%name\%/{NbrBins}\n
 *           number of bins in converted histogram. Default is 0.
 *
 * @author Marco Siano
 */
class pp404 : public PostprocessorBackend
{
public:
  /** constructor */
  pp404(PostProcessors& hist, const PostProcessors::key_t&);

  /** process event */
  virtual void process(const CASSEvent&);

  /** change the histogram, when told the the dependand histogram has changed */
  virtual void histogramsChanged(const HistogramBackend*);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** pp containing input histogram */
  PostprocessorBackend *_pHist;

  /** the user requested Tof-axis limits */
  std::pair<float,float> _userTofRange;

  /** calculate the tof from the mass to charge ratio
   *
   * \f$t=\alpha\sqrt{\frac{M}{Q}}+\beta\f$
   *
   * @return the time of flight corresponding to the m to q ratio.
   * @param MasstoCharge the m to q ratio
   */
  double calcMtCtoTof (double MasstoCharge);

  /** convert Time of Flight to mass to charge ratio
   *
   * \f$\frac{M}{Q}=(\frac{t}{\alpha}-\beta)^2\f$
   *
   * @return m to q ratio
   * @param Timeoflight
   */
  double calcToftoMtC (double Timeoflight);

  /** create m to q histogram from ToF histogram
   *
   * this is using the function pp404::calcMtCtoTof and pp404::calcToftoMtC to
   * do the conversion.
   *
   * @param hist the tof histogram to calc the m to q ration from
   * @param MtC the resulting m to q histogram storage
   * @param offset the offset of the baseline
   */
  void ToftoMtC(const HistogramFloatBase& hist ,
                HistogramFloatBase::storage_t& MtC,
                double offset);

  /** a time of flight of ion that we choosed */
  double t0;

  /** mass to charge ratio of t0 */
  double MtC0;

  /** another time of flight of ion that we choosed */
  double t1;

  /** mass to charge ratio of t1 */
  double MtC1;

  /** \f$\alpha=\frac{t_1-t_0}{\sqrt{MtC1}-\sqrt{MtC0}}\f$ */
  double alpha;

  /** \f$\beta=t_1-\alpha\sqrt{MtC1}\f$ */
  double beta;

  /** The lower limit for calculating baseline*/
  double tb1;

  /** The upper limit for calculating baseline*/
  double tb2;

  /** the calculated level of baseline*/
  double offset;

  /** number of bins in converted histgram*/
  size_t NbrBins;

  /** Upper limit converted to bin*/
  size_t binTofUp;

  /** lower limit converted to bin*/
  size_t binTofLow;

  /** tb1 converted to bin*/
  size_t bintb1;

  /** tb2 converted to bin*/
  size_t bintb2;
};








/**  calc the pulse duration from the bld
 *
 * get the duration value of FEL pulse from beam line data base.
 *
 * @author Koji Motomura
 */
class pp405 : public PostprocessorBackend
{
public:
  /** constructor */
  pp405(PostProcessors& hist, const PostProcessors::key_t&);

  /** calc the  pulse duration from the bld */
  virtual void process(const CASSEvent&);

  /** load the settings from cass.ini */
  virtual void loadSettings(size_t);
};









/** Tof to Energy correct from 0D histogram
 *
 * converts a time of flight trace of electrons to the coresponding energy with
 * correction from 0D histogram, using function
 * \f$e = (\frac{\alpha}{t-t_0})^2-e_0\f$.
 *
 * @cassttng PostProcessor/\%name\%/{TofLow|TofUp}\n
 *           The time of flight range within the histogram to convert to energy.
 *           Default is 0|1.
 * @cassttng PostProcessor/\%name\%/{t0}\n
 *           corrects the time of flight. Default is 0.
 * @cassttng PostProcessor/\%name\%/{e0}\n
 *           Retardation voltage. Default is 0.
 * @cassttng PostProcessor/\%name\%/{alpha}\n
 *           Coefficient for converting to energy. Default is 0.
 * @cassttng PostProcessor/\%name\%/{NbrBins}\n
 *           number of bins in converted histogram. Default is 0.
 * @cassttng PostProcessor/\%name\%/{tb1}\n
 *           The lower limit for calculating baseline. Default is 0.
 * @cassttng PostProcessor/\%name\%/{tb2}\n
 *           The upper limit for calculating baseline. Default is 0.
 * @cassttng PostProcessor/\%name\%/{HistName}\n
 *           input 1D histogram that is the time of flight wavetrace.
 * @cassttng PostProcessor/\%name\%/{HistZeroD}\n
 *           input 0D histogram for enegy correction.
 *
 * @author Koji Motomura
 * @author Marco Siano
 */
class pp406 : public PostprocessorBackend
{
public:
  /** constructor */
  pp406(PostProcessors& hist, const PostProcessors::key_t&);

  /** process event */
  virtual void process(const CASSEvent&);

  /** change the histogram, when told the the dependand histogram has changed */
  virtual void histogramsChanged(const HistogramBackend*);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** pp containing input histogram */
  PostprocessorBackend *_pHist;

  /** pp containing input OD histogram */
  PostprocessorBackend *_constHist;

  /** the user requested Tof-axis limits */
  std::pair<float,float> _userTofRange;

  /** corrects the time of flight*/
  double t0;

  /** retardation voltage*/
  double e0;

  /** Coefficient to convert to energy*/
  double alpha;

  /** The lower limit for calculating baseline*/
  double tb1;

  /** The upper limit for calculating baseline*/
  double tb2;

  /** the calculated level of baseline*/
  double offset;

  /** corrects from photon energy value*/
  double ediff;

  /** number of bins in converted histgram*/
  size_t NbrBins;

  /** Upper limit converted to bin*/
  size_t binTofUp;

  /** lower limit converted to bin*/
  size_t binTofLow;

  /** t0 converted to bin*/
  size_t bint0;

  /** tb1 converted to bin*/
  size_t bintb1;

  /** tb2 converted to bin*/
  size_t bintb2;

  /** retrieve the time that corresponses to a given energy
   *
   * calculate the time using function \f$t=\frac{\alpha}{\sqrt{e+e_0}}+t_0\f$.
   *
   * @return the calculated time
   * @param energy the energy that one wants to calc the time for
   */
  double calcEtoTof(double energy);

  /** make the histogram in energy scale.
   *
   * This function makes the histgram in energy scale. Offset value is
   * calculated by average in range of no signal.
   *
   * @param TofHisto input ToF histgram
   * @param Energy result histgram converted to energy
   * @param offset baseline of wavetrace
   */
  void ToftoEnergy(const HistogramFloatBase& TofHisto,
                   HistogramFloatBase::storage_t& Energy,
                   double offset );
};








/** Tof to Energy linear interpolation
 *
 * converts a time of flight trace of electrons to the coresponding energy
 * using fuction \f$e=(\frac{\alpha}{t-t_0})^2-e_0\f$ and apply linear
 * interpolation to energy spectrum.
 *
 * @cassttng PostProcessor/\%name\%/{TofLow|TofUp}\n
 *           The time of flight range within the histogram to convert to energy.
 *           Default is 0|1.
 * @cassttng PostProcessor/\%name\%/{t0}\n
 *           corrects the time of flight. Default is 0.
 * @cassttng PostProcessor/\%name\%/{e0}\n
 *           Retardation voltage. Default is 0.
 * @cassttng PostProcessor/\%name\%/{alpha}\n
 *           Coefficient for converting to energy. Default is 0.
 * @cassttng PostProcessor/\%name\%/{NbrBins}\n
 *           number of bins in converted histogram. Default is 0.
 * @cassttng PostProcessor/\%name\%/{tb1}\n
 *           The lower limit for calculating baseline. Default is 0.
 * @cassttng PostProcessor/\%name\%/{tb2}\n
 *           The upper limit for calculating baseline. Default is 0.
 * @cassttng PostProcessor/\%name\%/{HistName}\n
 *           input 1D histogram that is the time of flight wavetrace.
 * @cassttng PostProcessor/\%name\%/{HistZeroD}\n
 *           input 0D histogram for enegy correction.
 *
 * @author Koji Motomura
 * @author Marco Siano
 */
class pp407 : public PostprocessorBackend
{
public:
  /** constructor */
  pp407(PostProcessors& hist, const PostProcessors::key_t&);

  /** process event */
  virtual void process(const CASSEvent&);

  /** change the histogram, when told the the dependand histogram has changed */
  virtual void histogramsChanged(const HistogramBackend*);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** pp containing input histogram */
  PostprocessorBackend *_pHist;

  /** the user requested Tof-axis limits */
  std::pair<float,float> _userTofRange;

  /** retrieve the time that corresponses to a given energy
   *
   * calculate the time using function. \f$t=\frac{\alpha}{\sqrt{e+e_0}}+t_0\f$.
   *
   * @return the calculated time
   * @param energy the energy that one wants to calc the time for
   */
  double calcEtoTof (double energy);

  /** retrieve the linear interpolation value
   *
   * calculate the signal height at tofPos when it is interpolated
   *
   * @return the signal height
   * @param tofPos a time that we want to know interpolated value
   * @param binlow bin value at lower side
   * @param bin_size the bin size of energy histogram
   * @param TofHisto ToF wavetrace
   */
  double calcTofValue(const double tofPos, const size_t binlow ,
                      const double bin_size, const HistogramFloatBase& TofHisto);

  /** make the histgram in energy scale.
   *
   * This function makes the histgram in energy scale. Offset value is
   * calculated by average in range of no signal.
   *
   * @param TofHisto input ToF histgram
   * @param Energy result histgram converted to energy
   * @param offset baseline of wavetrace
   */
  void ToftoEnergy(const HistogramFloatBase& TofHisto,
                   HistogramFloatBase::storage_t& Energy,
                   double offset);

  /** corrects the time of flight*/
  double t0;

  /** retardation voltage*/
  double e0;

  /** Coefficient to convert to energy*/
  double alpha;

  /** The lower limit for calculating baseline*/
  double tb1;

  /** The upper limit for calculating baseline*/
  double tb2;

  /** the calculated level of baseline*/
  double offset;

  /** number of bins in converted histgram*/
  size_t NbrBins;

  /** Upper limit converted to bin*/
  size_t binTofUp;

  /** lower limit converted to bin*/
  size_t binTofLow;

  /** t0 converted to bin*/
  size_t bint0;

  /** tb1 converted to bin*/
  size_t bintb1;

  /** tb2 converted to bin*/
  size_t bintb2;
};








/** Tof to Energy linear interpolation and correct from 0d histogram
 *
 * converts a time of flight trace of electrons to the coresponding energy and
 * apply linear interpolation to energy spectrum using fuction
 * \f$e=(\frac{\alpha}{t-t_0})^2-e_0\f$.
 *
 * @cassttng PostProcessor/\%name\%/{TofLow|TofUp}\n
 *           The time of flight range within the histogram to convert to energy.
 *           Default is 0|1.
 * @cassttng PostProcessor/\%name\%/{t0}\n
 *           corrects the time of flight. Default is 0.
 * @cassttng PostProcessor/\%name\%/{e0}\n
 *           Retardation voltage. Default is 0.
 * @cassttng PostProcessor/\%name\%/{alpha}\n
 *           Coefficient for converting to energy. Default is 0.
 * @cassttng PostProcessor/\%name\%/{NbrBins}\n
 *           number of bins in converted histogram. Default is 0.
 * @cassttng PostProcessor/\%name\%/{tb1}\n
 *           The lower limit for calculating baseline. Default is 0.
 * @cassttng PostProcessor/\%name\%/{tb2}\n
 *           The upper limit for calculating baseline. Default is 0.
 * @cassttng PostProcessor/\%name\%/{HistName}\n
 *           input 1D histogram that is the time of flight wavetrace.
 * @cassttng PostProcessor/\%name\%/{HistZeroD}\n
 *           input 0D histogram for enegy correction.
 *
 * @author Koji Motomura
 * @author Marco Siano
 */
class pp408 : public PostprocessorBackend
{
public:
  /** constructor */
  pp408(PostProcessors& hist, const PostProcessors::key_t&);

  /** process event */
  virtual void process(const CASSEvent&);

  /** change the histogram, when told the the dependand histogram has changed */
  virtual void histogramsChanged(const HistogramBackend*);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** pp containing input histogram */
  PostprocessorBackend *_pHist;

  /** pp containing input OD histogram */
  PostprocessorBackend *_constHist;

  /** the user requested Tof-axis limits */
  std::pair<float,float> _userTofRange;

  /** retrieve the time that corresponses to a given energy
   *
   * calculate the time using function \f$t=\frac{\alpha}{\sqrt{e+e_0}}+t_0\f$.
   *
   * @return the calculated time
   * @param energy the energy that one wants to calc the time for
   */
  double calcEtoTof (double energy);

  /** retrieve the linear interpolation value
   *
   * calculate the signal height at tofPos when it is interpolated
   *
   * @return height of signal
   * @param tofPos a time that we want to know interpolated value
   * @param binlow bin value at lower side
   * @param bin_size the bin size of energy histogram
   * @param TofHisto ToF wavetrace
   */
  double calcTofValue(const double tofPos, const size_t binlow ,
                      const double bin_size, const HistogramFloatBase& TofHisto);

  /** make the histogram in energy scale.
   *
   * This function makes the histgram in energy scale. Offset value is
   * calculated by average in range of no signal.
   *
   * @param TofHisto input ToF histgram
   * @param Energy result histgram converted to energy
   * @param offset baseline of wavetrace
   */
  void ToftoEnergy(const HistogramFloatBase& TofHisto,
                   HistogramFloatBase::storage_t& Energy,
                   double offset );

  /** corrects the time of flight*/
  double t0;

  /** retardation voltage*/
  double e0;

  /** Coefficient to convert to energy*/
  double alpha;

  /** The lower limit for calculating baseline*/
  double tb1;

  /** The upper limit for calculating baseline*/
  double tb2;

  /** the calculated level of baseline*/
  double offset;

  /** difference from expected energy*/
  double ediff;

  /** number of bins in converted histgram*/
  size_t NbrBins;

  /** Upper limit converted to bin*/
  size_t binTofUp;

  /** lower limit converted to bin*/
  size_t binTofLow;

  /** t0 converted to bin*/
  size_t bint0;

  /** tb1 converted to bin*/
  size_t bintb1;

  /** tb2 converted to bin*/
  size_t bintb2;
};










/** Covariance map
 *
 * calculate the covariance by this on-line algorithm
 * \f$cov_n=((n-1)cov_{n-1}+(x_n-aveX_n))(y_n-aveY_{n-1})))\f$
 * and makes 2d map from 1d histogram.
 *
 * @cassttng PostProcessor/\%name\%/{HistName}\n
 *           input 1D histogram that we calculate covariance map.
 * @cassttng PostProcessor/\%name\%/{AveHistName}\n
 *           averaged histogram of "HistName"
 *
 * @author Koji Motomura
 */
class pp410 : public PostprocessorBackend
{
public:
  /** constructor */
  pp410(PostProcessors& hist, const PostProcessors::key_t&);

  /** process event */
  virtual void process(const CASSEvent&);

  /** load the settings */
  virtual void loadSettings(size_t);

  /** change the histogram, when told the the dependand histogram has changed */
  virtual void histogramsChanged(const HistogramBackend*);

protected:
  /** pp containing histogram to work on */
  PostprocessorBackend *_pHist;

  /** pp containing input histogram */
  PostprocessorBackend *_ave;

  /** calculate the covariance map
   *
   * calculate by on-line algorithm and makes 2d map
   *
   * @param data input histogram
   * @param averageOld averaged histgram at previous event
   * @param averageNew averaged histgram at present event
   * @param variance output histogram calculated variance
   * @param n number of present event
   */
  void calcCovariance(const HistogramFloatBase::storage_t& data ,
                      const HistogramFloatBase::storage_t& averageOld,
                      const HistogramFloatBase::storage_t& averageNew,
                      HistogramFloatBase::storage_t& variance,float n);

};





/** calculate intensity correction
 *
 * calculate the covariance between wavetrace and intensity
 * by on-line algorithm
 * \f$cov_n=\frac{1}{n}((n-1)cov_{n-1}+(x_n-aveX_n))(y_n-aveY_{n-1}))\f$
 *
 * @cassttng PostProcessor/\%name\%/{HistName1D}\n
 *           input 1D histogram that is the time of flight wavetrace.
 * @cassttng PostProcessor/\%name\%/{AveHistName1D}\n
 *           input 1D averaged histogram that is the time of flight wavetrace.
 * @cassttng PostProcessor/\%name\%/{HistName0D}\n
 *           input 1D histogram that is intensity.
 * @cassttng PostProcessor/\%name\%/{AveHistName0D}\n
 *           input 0D averaged histogram that is intensity.
 *
 * @author Koji Motomura
 */
class pp412 : public PostprocessorBackend
{
public:
  /** constructor */
  pp412(PostProcessors& hist, const PostProcessors::key_t&);

  /** process event */
  virtual void process(const CASSEvent&);

  /** load the settings */
  virtual void loadSettings(size_t);

  /** change the histogram, when told the the dependand histogram has changed */
  virtual void histogramsChanged(const HistogramBackend*);

protected:

  /** pp containing input histogram 1D*/
  PostprocessorBackend *_hist1D;

  /** pp containing input histogram 0D*/
  PostprocessorBackend *_hist0D;

  /** pp containing input histogram 1D it should be averaged _hist1D*/
  PostprocessorBackend *_ave1D;

  /** pp containing input histogram 0D it should be averaged _hist0D*/
  PostprocessorBackend *_ave0D;

  /** calculate covariance
   *
   * calculate covariance between wavetrace (1d) and intensity (0d).
   *
   * @param waveTrace input wavetrace
   * @param waveTraceAve averaged wavetrace
   * @param intensity 0d intensity value
   * @param intensityAveOld averaged intensity value
   * @param correction output histogram calculated covariance
   * @param n number of present event
   */
  void calcCovariance(const HistogramFloatBase::storage_t& waveTrace ,
                      const HistogramFloatBase::storage_t& waveTraceAve ,
                      const float intensity ,
                      const float intensityAveOld,
                      HistogramFloatBase::storage_t& correction,
                      float n);

};






/** indicate number of event
 *
 * write to the stdout how the number of fills of the this
 *
 * @cassttng PostProcessor/\%name\%/{Frequency}\n
 *           frequency for show the number of event
 *
 * @author Koji Motomura
 */
class pp420 : public PostprocessorBackend
{
public:
  /** constructor */
  pp420(PostProcessors& hist, const PostProcessors::key_t&);

  /** retrieve the nbr of fills */
  virtual void process(const CASSEvent&);

  /** load the settings from cass.ini */
  virtual void loadSettings(size_t);

protected:
  /** pp containing input histogram */
  PostprocessorBackend *_pHist;

  /** frequency */
  size_t freq;
};


}//end namespace cass


#endif // __PARTIAL_COVARIANCE_H__
