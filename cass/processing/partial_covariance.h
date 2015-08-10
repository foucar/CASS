// Copyright (C) 2012 Koji Motomura

/**
 * @file partial_covariance.h processors to calculate partical covariance
 *
 * @author Koji Motomura
 */

#ifndef __PARTIAL_COVARIANCE_H__
#define __PARTIAL_COVARIANCE_H__

#include "processor.h"
#include "cass_event.h"


namespace cass
{

/** converts a Electron Time of Flight trace to Energy
 *
 * @PPList "400": converts a Electron Time of Flight trace to Energy
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
 * @cassttng Processor/\%name\%/{TofLow|TofUp}\n
 *           The time of flight range within the histogram to convert to energy.
 *           Default is 0|1.
 * @cassttng Processor/\%name\%/{t0}\n
 *           corrects the time of flight
 *           Default is 0.
 * @cassttng Processor/\%name\%/{e0}\n
 *           Retard voltage
 *           Default is 0.
 * @cassttng Processor/\%name\%/{alpha}\n
 *           Coefficient for converting to energy. Default is 0.
 * @cassttng Processor/\%name\%/{NbrBins}\n
 *           number of bins in converted histogram. Default is 0.
 * @cassttng Processor/\%name\%/{tb1}\n
 *           The lower limit in x for calculating the baseline. Default is 0.
 * @cassttng Processor/\%name\%/{tb2}\n
 *           The upper limit in x for calculating the baseline. Default is 0.
 * @cassttng Processor/\%name\%/{HistName}\n
 *           input 1D histogram that contains the time of flight wavetrace.
 *
 * @author Koji Motomura
 */
class pp400 : public Processor
{
public:
  /** constructor */
  pp400(const name_t &);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

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
  void ToftoEnergy(const result_t& TofHisto,
                   result_t& Energy,
                   double offset);

  /** pp containing input histogram */
  shared_pointer _pHist;

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







/** convert time of flight to charge to mass ratio
 *
 * @PPList "404": convert time of flight to charge to mass ratio
 *
 * converts a time of flight trace of ions to the coresponding mass to charge
 * ratio.
 *
 * In order to calculate the Mass to Charge ratio one has to provide two known
 * peaks and their Charge to Mass ratio. From this information the rest is
 * calculated.
 *
 * @cassttng Processor/\%name\%/{TofLow|TofLow}\n
 *           The time of flight range within the histogram to convert to mass.
 *           to charge ratio Default is 0|1.
 * @cassttng Processor/\%name\%/{t0}\n
 *           a time of flight of ion that we choosed. Default is 0.
 * @cassttng Processor/\%name\%/{t1}\n
 *           another time of flight of ion that we choosed. Default is 0.
 * @cassttng Processor/\%name\%/{MtC0}\n
 *           mass to charge ratio of t0. Default is 0.
 * @cassttng Processor/\%name\%/{MtC1}\n
 *           mass to charge ratio of t1. Default is 0.
 * @cassttng Processor/\%name\%/{tb1}\n
 *           The lower limit for calculating baseline. Default is 0.
 * @cassttng Processor/\%name\%/{tb2}\n
 *           The upper limit for calculating baseline. Default is 0.
 * @cassttng Processor/\%name\%/{NbrBins}\n
 *           number of bins in converted histogram. Default is 0.
 *
 * @author Marco Siano
 */
class pp404 : public Processor
{
public:
  /** constructor */
  pp404(const name_t &);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** pp containing input histogram */
  shared_pointer _pHist;

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
  void ToftoMtC(const result_t& hist ,
                result_t& MtC,
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
 * @PPList "405": pulse duration from the bld
 *
 * get the duration value of FEL pulse from beam line data base.
 *
 * @author Koji Motomura
 */
class pp405 : public Processor
{
public:
  /** constructor */
  pp405(const name_t &);

  /** calc the  pulse duration from the bld */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings from cass.ini */
  virtual void loadSettings(size_t);
};









/** Tof to Energy correct from 0D histogram
 *
 * @PPList "406": Tof to Energy correct from 0D histogram
 *
 * converts a time of flight trace of electrons to the coresponding energy with
 * correction from 0D histogram, using function
 * \f$e = (\frac{\alpha}{t-t_0})^2-e_0\f$.
 *
 * @cassttng Processor/\%name\%/{TofLow|TofUp}\n
 *           The time of flight range within the histogram to convert to energy.
 *           Default is 0|1.
 * @cassttng Processor/\%name\%/{t0}\n
 *           corrects the time of flight. Default is 0.
 * @cassttng Processor/\%name\%/{e0}\n
 *           Retardation voltage. Default is 0.
 * @cassttng Processor/\%name\%/{alpha}\n
 *           Coefficient for converting to energy. Default is 0.
 * @cassttng Processor/\%name\%/{NbrBins}\n
 *           number of bins in converted histogram. Default is 0.
 * @cassttng Processor/\%name\%/{tb1}\n
 *           The lower limit for calculating baseline. Default is 0.
 * @cassttng Processor/\%name\%/{tb2}\n
 *           The upper limit for calculating baseline. Default is 0.
 * @cassttng Processor/\%name\%/{HistName}\n
 *           input 1D histogram that is the time of flight wavetrace.
 * @cassttng Processor/\%name\%/{HistZeroD}\n
 *           input 0D histogram for enegy correction.
 *
 * @author Koji Motomura
 * @author Marco Siano
 */
class pp406 : public Processor
{
public:
  /** constructor */
  pp406(const name_t &);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** pp containing input histogram */
  shared_pointer _pHist;

  /** pp containing input OD histogram */
  shared_pointer _constHist;

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
  void ToftoEnergy(const result_t& TofHisto,
                   result_t& Energy,
                   double offset );
};








/** Tof to Energy linear interpolation
 *
 * @PPList "407":  Tof to Energy linear interpolation
 *
 * converts a time of flight trace of electrons to the coresponding energy
 * using fuction \f$e=(\frac{\alpha}{t-t_0})^2-e_0\f$ and apply linear
 * interpolation to energy spectrum.
 *
 * @cassttng Processor/\%name\%/{TofLow|TofUp}\n
 *           The time of flight range within the histogram to convert to energy.
 *           Default is 0|1.
 * @cassttng Processor/\%name\%/{t0}\n
 *           corrects the time of flight. Default is 0.
 * @cassttng Processor/\%name\%/{e0}\n
 *           Retardation voltage. Default is 0.
 * @cassttng Processor/\%name\%/{alpha}\n
 *           Coefficient for converting to energy. Default is 0.
 * @cassttng Processor/\%name\%/{NbrBins}\n
 *           number of bins in converted histogram. Default is 0.
 * @cassttng Processor/\%name\%/{tb1}\n
 *           The lower limit for calculating baseline. Default is 0.
 * @cassttng Processor/\%name\%/{tb2}\n
 *           The upper limit for calculating baseline. Default is 0.
 * @cassttng Processor/\%name\%/{HistName}\n
 *           input 1D histogram that is the time of flight wavetrace.
 * @cassttng Processor/\%name\%/{HistZeroD}\n
 *           input 0D histogram for enegy correction.
 *
 * @author Koji Motomura
 * @author Marco Siano
 */
class pp407 : public Processor
{
public:
  /** constructor */
  pp407(const name_t &);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** pp containing input histogram */
  shared_pointer _pHist;

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
                      const double bin_size, const result_t& TofHisto);

  /** make the histgram in energy scale.
   *
   * This function makes the histgram in energy scale. Offset value is
   * calculated by average in range of no signal.
   *
   * @param TofHisto input ToF histgram
   * @param Energy result histgram converted to energy
   * @param offset baseline of wavetrace
   */
  void ToftoEnergy(const result_t& TofHisto,
                   result_t& Energy,
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
 * @PPList "408": Tof to Energy linear interpolation and correct from 0d histogram
 *
 * converts a time of flight trace of electrons to the coresponding energy and
 * apply linear interpolation to energy spectrum using fuction
 * \f$e=(\frac{\alpha}{t-t_0})^2-e_0\f$.
 *
 * @cassttng Processor/\%name\%/{TofLow|TofUp}\n
 *           The time of flight range within the histogram to convert to energy.
 *           Default is 0|1.
 * @cassttng Processor/\%name\%/{t0}\n
 *           corrects the time of flight. Default is 0.
 * @cassttng Processor/\%name\%/{e0}\n
 *           Retardation voltage. Default is 0.
 * @cassttng Processor/\%name\%/{alpha}\n
 *           Coefficient for converting to energy. Default is 0.
 * @cassttng Processor/\%name\%/{NbrBins}\n
 *           number of bins in converted histogram. Default is 0.
 * @cassttng Processor/\%name\%/{tb1}\n
 *           The lower limit for calculating baseline. Default is 0.
 * @cassttng Processor/\%name\%/{tb2}\n
 *           The upper limit for calculating baseline. Default is 0.
 * @cassttng Processor/\%name\%/{HistName}\n
 *           input 1D histogram that is the time of flight wavetrace.
 * @cassttng Processor/\%name\%/{HistZeroD}\n
 *           input 0D histogram for enegy correction.
 *
 * @author Koji Motomura
 * @author Marco Siano
 */
class pp408 : public Processor
{
public:
  /** constructor */
  pp408(const name_t &);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** pp containing input histogram */
  shared_pointer _pHist;

  /** pp containing input OD histogram */
  shared_pointer _constHist;

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
                      const double bin_size, const result_t& TofHisto);

  /** make the histogram in energy scale.
   *
   * This function makes the histgram in energy scale. Offset value is
   * calculated by average in range of no signal.
   *
   * @param TofHisto input ToF histgram
   * @param Energy result histgram converted to energy
   * @param offset baseline of wavetrace
   */
  void ToftoEnergy(const result_t& TofHisto,
                   result_t& Energy,
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
 * @PPList "410":Covariance map
 *
 * calculate the covariance by this on-line algorithm
 * \f$cov_n=((n-1)cov_{n-1}+(x_n-aveX_n))(y_n-aveY_{n-1})))\f$
 * and makes 2d map from 1d histogram.
 *
 * @cassttng Processor/\%name\%/{HistName}\n
 *           input 1D histogram that we calculate covariance map.
 * @cassttng Processor/\%name\%/{AveHistName}\n
 *           averaged histogram of "HistName"
 *
 * @author Koji Motomura
 */
class pp410 : public AccumulatingProcessor
{
public:
  /** constructor */
  pp410(const name_t &);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings */
  virtual void loadSettings(size_t);

protected:
  /** pp containing histogram to work on */
  shared_pointer _pHist;

  /** pp containing input histogram */
  shared_pointer _ave;

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
  void calcCovariance(const result_t& data ,
                      const result_t::storage_t &averageOld,
                      const result_t& averageNew,
                      result_t& variance, float n);

};





/** calculate intensity correction
 *
 * @PPList "412": calculate intensity correction of covariance map
 *
 * calculate the covariance between wavetrace and intensity
 * by on-line algorithm
 * \f$cov_n=\frac{1}{n}((n-1)cov_{n-1}+(x_n-aveX_n))(y_n-aveY_{n-1}))\f$
 *
 * @cassttng Processor/\%name\%/{HistName1D}\n
 *           input 1D histogram that is the time of flight wavetrace.
 * @cassttng Processor/\%name\%/{AveHistName1D}\n
 *           input 1D averaged histogram that is the time of flight wavetrace.
 * @cassttng Processor/\%name\%/{HistName0D}\n
 *           input 1D histogram that is intensity.
 * @cassttng Processor/\%name\%/{AveHistName0D}\n
 *           input 0D averaged histogram that is intensity.
 *
 * @author Koji Motomura
 */
class pp412 : public AccumulatingProcessor
{
public:
  /** constructor */
  pp412(const name_t &);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings */
  virtual void loadSettings(size_t);

protected:

  /** pp containing input histogram 1D*/
  shared_pointer _hist1D;

  /** pp containing input histogram 0D*/
  shared_pointer _hist0D;

  /** pp containing input histogram 1D it should be averaged _hist1D*/
  shared_pointer _ave1D;

  /** pp containing input histogram 0D it should be averaged _hist0D*/
  shared_pointer _ave0D;

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
  void calcCovariance(const result_t& waveTrace ,
                      const result_t& waveTraceAve ,
                      const float intensity ,
                      const float intensityAveOld,
                      result_t& correction,
                      float n);

};

}//end namespace cass


#endif // __PARTIAL_COVARIANCE_H__
