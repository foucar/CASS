//Copyright (C) 2013 Lutz Foucar
//Copyright (C) 2013 Stephan Kassemeyer

/**
 * @file fft.cpp containing the class to calculate the fast fourier transform
 *
 * @author Lutz Foucar
 */

#include <algorithm>
#include <functional>
#include <numeric>

#include "fft.h"
#include "result.hpp"
#include "cass_event.h"
#include "cass_settings.h"
#include "log.h"
#include "processor.h"

using namespace cass;
using namespace std;
using tr1::bind;
using tr1::placeholders::_1;
using tr1::placeholders::_2;


pp312::pp312(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

// todo:
//  fftw_destroy_plan(plan);

void pp312::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));

  setupGeneral();
  _hist = setupDependency("InputName");
  bool ret (setupCondition());
  if (!(ret && _hist))
    return;

  const int dim(_hist->result().dim());
  if (dim == 1)
  {
    _Nx = _hist->result().shape().first;
    _func = bind(&pp312::fft1d,this,_1,_2);
    _fft_tmp_padding = (_Nx&1) ? 1 : 2;
    vector<double> fft_tmp;
    fft_tmp.resize(_Nx+_fft_tmp_padding);
    double* tmpdata(&(fft_tmp.front()));
    _fftw_plan = fftw_plan_dft_r2c_1d(_Nx, tmpdata, reinterpret_cast<fftw_complex*>(tmpdata), FFTW_MEASURE);
  }
  else if (dim == 2)
  {
    _Nx = _hist->result().shape().first;
    _Ny = _hist->result().shape().second;
    _func = bind(&pp312::fft2d,this,_1,_2);
    _fft_tmp_padding = (_Nx&1) ? 1 : 2;
    vector<double> fft_tmp;
    fft_tmp.resize(_Ny*(_Nx+_fft_tmp_padding));
    double* tmpdata(&(fft_tmp.front()));
    _fftw_plan = fftw_plan_dft_r2c_2d(_Ny, _Nx, tmpdata, reinterpret_cast<fftw_complex*>(tmpdata), FFTW_MEASURE);
  }
  else
    throw invalid_argument("pp312::loadSettings(): Input Histogram '" + _hist->name() +
                           "' has unsupported dimenstion '" + toString(dim) + "'");

  createHistList(_hist->result().clone());

  Log::add(Log::INFO,"Processor '" + name() +
           "' will calculate the fft of '" + _hist->name() +
           "'. Condition is '" + _condition->name() + "'");
}

void pp312::fft1d(result_t::const_iterator in, result_t::iterator out)
{
  // if fftw was to support arbitrary precision, traits could be used to determine the precision of
  // a given Histogram and it could be cast to fftw types as follows:
  // fftw_plan plan = fftw_plan_dft_r2c_1d(_Nx, in, reinterpret_cast<pp312Traits<HistogramFloatBase::value_t>::fftw_complex_type*> (out), FFTW_MEASURE);
  // unfortunately though, fftw only supports one type of precision determined during compilation.
  // so we have to convert to double, the standard precision.
  std::vector<double> fft_tmp;
  fft_tmp.resize(_Nx+_fft_tmp_padding);
  copy(in, in+_Nx, fft_tmp.begin());
  double* tmpdata(&(fft_tmp.front()));

  fftw_execute_dft_r2c( _fftw_plan, tmpdata, reinterpret_cast<fftw_complex*>(tmpdata));

  // fftw saves some memory for real-valued input data.
  // we have to restore the full data by unpacking hermitian symmetry pairs:

  // shift first half:
  for (size_t xx=0;xx<_Nx/2;++xx)
  {
    result_t::value_t v_real = tmpdata[xx*2];
    result_t::value_t v_imag = tmpdata[xx*2+1];
    out[xx+_Nx/2] = v_real*v_real + v_imag*v_imag;
  }
  // shift second half:
  for (size_t xx=_Nx/2;xx<_Nx;++xx)
  {
    result_t::value_t v_real = tmpdata[xx*2];
    result_t::value_t v_imag = tmpdata[xx*2+1];
    out[xx-_Nx/2] = v_real*v_real + v_imag*v_imag;
  }
}

void pp312::fft2d(result_t::const_iterator in, result_t::iterator out)
{
  // if fftw was to support arbitrary precision, traits could be used to determine the precision of
  // a given Histogram and it could be cast to fftw types as follows:
  // fftw_plan plan = fftw_plan_dft_r2c_2d(_Nx, _Ny, in, reinterpret_cast<pp312Traits<HistogramFloatBase::value_t>::fftw_complex_type*> (out), FFTW_MEASURE);
  // unfortunately though, fftw only supports one type of precision determined during compilation.
  // so we have to convert to double, the standard precision.


  vector<double> fft_tmp;
  fft_tmp.resize(_Ny*(_Nx+_fft_tmp_padding));
  double* tmpdata(&(fft_tmp.front()));
  //std::copy(in, in+_Nx*_Ny, fft_tmp.begin());
  for (size_t yy=0;yy<_Ny;++yy)
    for (size_t xx=0;xx<_Nx;++xx)
      tmpdata[yy*(_Nx+_fft_tmp_padding)+xx]=in[yy*_Nx+xx];

//  for (int yy=0;yy<_Ny;++yy)
//    for (int xx=0;xx<_Nx;++xx)
//      tmpdata[yy*(_Nx+_fft_tmp_padding)+xx]=0;
//  for (int yy=_Ny/2-30;yy<_Ny/2+30;++yy)
//    for (int xx=_Nx/2-30;xx<_Nx/2+30;++xx)
//      tmpdata[yy*(_Nx+_fft_tmp_padding)+xx]=10;


  fftw_execute_dft_r2c( _fftw_plan, tmpdata, reinterpret_cast<fftw_complex*>(tmpdata));

  // fftw saves some memory for real-valued input data.
  // we have to restore the full data by unpacking hermitian symmetry pairs:
  for (size_t xx=0;xx<_Nx/2;++xx)
    for (size_t yy=0;yy<_Ny/2;++yy)
      out[_Ny*xx+yy]=0;
  for (size_t ii=0;ii<_Nx/2*_Ny/2;++ii)
    out[ii]=tmpdata[ii*2];

  // shift first quadrant:
  for (size_t xx=_Nx/2;xx<_Nx;++xx)
  {
    for (size_t yy=_Ny/2;yy<_Ny;++yy)
    {
      result_t::value_t v_real = tmpdata[(yy-_Ny/2)*(_Nx+_fft_tmp_padding)+(xx-_Nx/2)*2];
      result_t::value_t v_imag = tmpdata[(yy-_Ny/2)*(_Nx+_fft_tmp_padding)+(xx-_Nx/2)*2+1];
      out[_Nx*(yy)+xx] = v_real*v_real + v_imag*v_imag;
    }
  }
  // shift second quadrant:
  for (size_t xx=_Nx/2;xx<_Nx;++xx)
  {
    for (size_t yy=0;yy<_Ny/2;++yy)
    {
      result_t::value_t v_real = tmpdata[(yy+_Ny/2)*(_Nx+_fft_tmp_padding)+(xx-_Nx/2)*2];
      result_t::value_t v_imag = tmpdata[(yy+_Ny/2)*(_Nx+_fft_tmp_padding)+(xx-_Nx/2)*2+1];
      out[_Nx*(yy)+xx] = v_real*v_real + v_imag*v_imag;
    }
  }
  // fill in hermitian symmetry:
  for (size_t xx=0;xx<_Nx/2;++xx)
  {
    for (size_t yy=0;yy<_Ny;++yy)
    {
      out[_Nx*(yy)+xx] = out[ _Nx*(_Ny-yy) + _Nx-xx ];
    }
  }
  // special treatment for (0,0):
  out[_Nx/2*_Ny+_Ny/2] = tmpdata[0]*tmpdata[0] + tmpdata[1]*tmpdata[1];
}

void pp312::process(const CASSEvent &evt, result_t &result)
{
  const result_t& hist(_hist->result(evt.id()));
  QReadLocker lock(&hist.lock);

  _func(hist.begin(),result.begin());
}

