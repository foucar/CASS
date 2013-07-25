// Copyright (C) 2013 Lutz Foucar
// Copyright (C) 2013 Stephan Kassemeyer

/**
 * @file fft.h containing the class to calculate the fast fourier transform
 *
 * @author Lutz Foucar
 */

#ifndef _FFTW_H_
#define _FFTW_H_

#include <fftw3.h>
#include <tr1/functional>

#include "processor.h"
#include "histogram.h"

namespace cass
{

/* general traits case not implemented: */
template <typename T> struct pp312Traits;

/* float traits case specialization: */
template <>
struct pp312Traits<float>
{
  typedef fftwf_complex fftw_complex_type;
  typedef float fftw_real_type;
};

/* double traits case specialization: */
template <>
struct pp312Traits<double>
{
  typedef fftw_complex fftw_complex_type;
  typedef double fftw_real_type;
};





/** calculate the absolute squared fft of an histogram
 *
 * details
 *
 * @PPList "312": calculate the absolute square fft of an histogram
 *
 * @cassttng PostProcessor/\%name\%/{HistName} \n
 *           Postprocessor name containing the histogram whos fft
 *           should be calculated.
 *
 * @author Stephan Kassemeyer
 * @author Lutz Foucar
 */
class pp312 : public PostProcessor
{
public:
  /** constructor
   *
   * @param pp referPostProcessorence to the postprocessor manager
   * @param key the name of this PostProce
   */
  pp312(const name_t &name);

  /** process the event
   *
   * @param evt the event that contains the id
   * @param result the histogram that will contain the result of the process
   */
  virtual void process(const CASSEvent &evt, HistogramBackend &result);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

protected:
  /** define the shape */
  typedef std::pair<int,int> shape_t;

protected:
  /** calculate the absolute square fft for 1d data
   *
   * @param in the incomming data
   * @param out where the fft will be written to
   */
  void fft1d(const HistogramFloatBase::value_t *in, HistogramFloatBase::value_t *out);

  /** calculate the absolute square fft for 2d data
   *
   * @param in the incomming data
   * @param out where the fft will be written to
   */
  void fft2d(const HistogramFloatBase::value_t *in, HistogramFloatBase::value_t *out);

protected:
  /** pp containing histogram to calculate the autocorrelation for */
  shared_pointer _hist;

  /** the shape of the incomming histogram */
  size_t _Nx;
  size_t _Ny;

  /** fftw plan - stores fftw optimization data for
   * given data type+size */
  fftw_plan _fftw_plan;

  /** info about temporary memory for the fftw calculation */
  int _fft_tmp_padding;

  /** function to call for processing */
  std::tr1::function<void(const HistogramFloatBase::value_t*, HistogramFloatBase::value_t*)> _func;
};


} //end namespace cass
#endif
