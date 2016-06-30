// Copyright (C) 2010 Lutz Foucar

/**
 * @file waveform.h file contains acqiris data retrieval processor
 *                  declaration
 *
 * @author Lutz Foucar
 */

#ifndef _WAVEFORM__H_
#define _WAVEFORM__H_

#include "processor.h"

namespace cass
{
//forward declarations
class CASSEvent;
class Histogram1DFloat;

/** acqiris channel waveform.
 *
 * @PPList "110": retrieve wavefrom of a channel
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{InstrumentId} \n
 *           The instrument id of the acqiris instrument that contains the
 *           channel. Default is 8. Options are:
 *           - 8: Camp Acqiris
 *           - 4: AMO ITof Acqiris
 *           - 2: AMO GD Acqiris
 *           - 5: AMO Mbes Acqiris
 *           - 22: XPP Acqiris
 * @cassttng Processor/\%name\%/{ChannelNbr} \n
 *           The channel number of the acqiris instrument. Default is 0.
 * @cassttng Processor/\%name\%/{NbrSamples} \n
 *           Number of samples in the waveform. Default is 40000
 * @cassttng Processor/\%name\%/{SampleInterval} \n
 *           Sample Interval (Time between to samples in s. Default is 1e-9
 *
 * @author Lutz Foucar
 */
class pp110 : public Processor
{
public:
  /** constructor */
  pp110(const name_t &name);

  /** copy the last waveform from the channel*/
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

protected:
  /** the instrument that contains the channel this processor will work on */
  uint32_t _instrument;

  /** the Acqiris channel number of this processor */
  size_t _channel;

  /** the sample interval */
  double _sampleInterval;
};



/** cfd trace from waveform
 *
 * @PPList "111": convert wavefrom to cfd trace
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{Waveform} \n
 *           The name of the Processor containing the waveform that should
 *           be converted. Default is 'Unknown'
 * @cassttng Processor/\%name\%/{Delay_ns}\n
 *           Delay in ns used. Default is 5.
 * @cassttng Processor/\%name\%/{Fraction}\n
 *           Fraction used. Default is 0.6
 * @cassttng Processor/\%name\%/{Walk_V}\n
 *           walk in V used. Default is 0.
 *
 * @author Lutz Foucar
 */
class pp111 : public Processor
{
public:
  /** constructor */
  pp111(const name_t &name);

  /** copy the last waveform from the channel*/
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

protected:
  /** the Processor that contains the waveform to convert */
  shared_pointer _waveform;

  /** the delay in bins */
  size_t _delay;

  /** the fraction */
  float _fraction;

  /** the walk in volts */
  float _walk;
};



/** cfd analysis of waveform
 *
 * @PPList "112": cfd analysis of waveform
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{Waveform} \n
 *           The name of the Processor containing the waveform that should
 *           be converted. Default is 'Unknown'
 * @cassttng Processor/\%name\%/{Delay}\n
 *           Delay in units of the input x-axis is used. Default is 5.
 * @cassttng Processor/\%name\%/{Fraction}\n
 *           Fraction used. Default is 0.6
 * @cassttng Processor/\%name\%/{Threshold}\n
 *           threshold in units of the y-axis. Default is 0.1
 * @cassttng Processor/\%name\%/{Walk}\n
 *           walk in units of the y-axis. Default is 0.
 *
 * @author Lutz Foucar
 */
class pp112 : public Processor
{
public:
  /** constructor */
  pp112(const name_t &name);

  /** copy the last waveform from the channel*/
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

protected:
  /** definition of the table */
  typedef result_t::storage_t table_t;

  /** enum describing the contents of the resulting table */
  enum ColumnNames
  {
    position  =  0,
    height    =  1,
    maxpos    =  2,
    fwhm      =  3,
    startpos  =  4,
    endpos    =  5,
    integral  =  6,
    width     =  7,
    polarity  =  8,
    CoM       =  9,
    nbrOf
  };

protected:
  /** define the fitparameters */
  typedef std::pair<float,float> fitparam_t;

  /** define a point */
  typedef std::pair<float,float> point_t;

  /** define points */
  typedef std::vector<point_t> points_t;

  /** make a linear regression through points
   *
   * @return the constant and the slope
   * @param points vector containing the x,y coordintas of the points
   */
  fitparam_t linearRegression(points_t::const_iterator first, points_t::const_iterator last);

  /** create Newton Polynomial
   *
   * This function creates the coefficients for Newton interpolating Polynomials.
   * Newton Polynomials are created from n Points and have the form
   * \f$p(x) = c_0 + c_1(x-x_0) + c_2(x-x_0)(x-x_1)+...+c_{n-1}(x-x_0)(x-x_1)...(x-x_{n-2})\f$
   * given that you have n Points
   * \f$(x_0,y_0), (x_1,y_1), ..., (x_{n-1},y_{n-1})\f$
   * Here we do it for 4 Points.
   *
   * @param[in] x the x-values of the points
   * @param[in] y the y-values of the points
   * @param[out] coeff the coefficients of the newton polynomial
   */
  void createNewtonPolynomial(const float * x, const float * y, float * coeff);

  /** evaluate Newton Polynomial
   *
   * this function evaluates the Newton Polynomial that was created from n Points
   * \f$(x_0,y_0),..., (x_{n-1},y_{n-1}) with coefficients (c_0,...,c_{n-1})\f$
   * using Horner's Rule. This is done for an polynomial with 4 entries
   *
   * @return the newton polynomial
   * @param[in] x array of x values
   * @param[in] coeff array of coefficients
   * @param[in] X
   */
  float evalNewtonPolynomial(const float * x, const float * coeff, float X);

  /** Achims Numerical Approximation
   *
   * this function should find x value corrosponding to a given y value
   * in a newton polynomial. It does it the following way:
   * -# create a lower and upper boundary point
   * -# create an interating x-value and initialize it with the Start value
   * -# evaluate the y-value at the x-value
   * -# if the y value is bigger than the requested value the start point
   *    is defines the new upper or lower boundary depending on the slope.
   * -# the new x-point is defined by the arithmetic mean between the tow
   *    boundary points.
   * -# do points 3-5 until the new x-value does not differ more than 0.005
   *    from the old one.
   *
   *@param[in] x two points describing upper and lower boundaries
   *@param[in] coeff the newton polynomial coefficents
   *@param[in] Y the requested y-values to find the x-value for
   *@param[in] Start the x-value we start the search with
   */
  float findXForGivenY(const float * x, const float * coeff, const float Y, const float Start);

protected:
  /** the Processor that contains the waveform to convert */
  shared_pointer _waveform;

  /** the delay in bins */
  size_t _delay;

  /** the fraction */
  float _fraction;

  /** the walk in volts */
  float _walk;

  /** the threshold */
  float _threshold;

  /** the baseline */
  float _baseline;
};
}

#endif
