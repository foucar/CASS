//Copyright (C) 2009,2010 Lutz Foucar

/**
 * @file peak.h file contains the classes that describe a signal within an
 *              acqiris channels waveform
 *
 * @author Lutz Foucar
 */

#ifndef _PEAK_H_
#define _PEAK_H_

#include <vector>
#include <stdint.h>

#include "cass_acqiris.h"


namespace cass
{
  namespace ACQIRIS
  {
    /** Properties of a signal found inside a waveform.
     *
     * This class contains all information one can extract from a singal in the wavefrom.
     * The name peak comes historically.
     *
     * @note can make make it such that the properties are calculated only the
     *       first time they are requested. Then we need to include what kind of
     *       analysis should be used to extract the time information from the singal.
     * @note to be able to calculate the values lazy, one could create pair<bool, value>
     *       where the bool indicates whether the value has already been calculated for
     *       the event.
     * @todo find a more meaningful name for this class (maybe this one should be called signal,
     *       since it represents a signal in the waveform
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT Peak
    {
    public:
      /** when a peak is created we have to initialize used to false*/
      Peak():_used(false)  {}

    public:
      /** compare two peaks
       *
       * a peak is smaller or equal when time of the peak is smaller or equal
       * than the input
       *
       * @return true when the time is smaller than the value
       * @param rhs the right hand side of the comparison
       */
      bool operator<=(double rhs)const {return _time <= rhs;}

      /** compare two peaks.
       *
       * a peak is greater when its not smaller
       *
       * @return negate smaller or equal than operator
       * @param rhs the right hand side of the comparison
       */
      bool operator>(double rhs)const {return _time > rhs;}

    public:
      //@{
      /** setter */
      double     &time()              {return _time;}
      double     &com()               {return _com;}
      double     &cfd()               {return _cfd;}
      double     &integral()          {return _integral;}
      double     &height()            {return _height;}
      double     &width()             {return _width;}
      double     &fwhm()              {return _fwhm;}
      uint32_t   &startpos()          {return _startpos;}
      uint32_t   &stoppos()           {return _stoppos;}
      uint32_t   &maxpos()            {return _maxpos;}
      double     &maximum()           {return _maximum;}
      Polarity   &polarity()          {return _polarity;}
      bool       &isUsed()            {return _used;}
      //@}
      //@{
      /** getter */
      double      time()const         {return _time;}
      double      com()const          {return _com;}
      double      cfd()const          {return _cfd;}
      double      height()const       {return _height;}
      bool        isUsed()const       {return _used;}
      Polarity    polarity()const     {return _polarity;}
      double      maximum()const      {return _maximum;}
      uint32_t    maxpos()const       {return _maxpos;}
      uint32_t    stoppos()const      {return _stoppos;}
      uint32_t    startpos()const     {return _startpos;}
      double      fwhm()const         {return _fwhm;}
      double      width()const        {return _width;}
      double      integral()const     {return _integral;}
      //@}

    private:
      /** position of singal in time
       *
       * the time of the signal, depending on which kind
       * kind of extraction is requested this value will
       * either represent the center of mass, or the constant
       * fraction.
       */
      double _time;

      /** the time calculated from cfd */
      double _cfd;

      /** the time calculate from the center of mass*/
      double _com;

      /** the polarity of the signal */
      Polarity _polarity;

      /** the slope of the front of this signal*/
      double _slope;

      /** the position where the maximum value of the signal is*/
      uint32_t    _maxpos;

      /** the height in digitizer units */
      double _maximum;

      /** the height in V*/
      double _height;

      /** the height when you use the substraction cfd*/
      double _heightAbziehen;

      /** the full width at half maximum of the signal*/
      double _fwhm;

      /** the width at the bottom of the peak*/
      double _width;

      /** the pos where the left edge crosses the half of the height (historical)*/
      double _posHalfLeft;

      /** the pos where the right edge crosses the half of the height*/
      double _posHalfRight;

      /** the integral of the signal*/
      double _integral;

      /** the index where the signal starts*/
      uint32_t    _startpos;

      /** the index where the signal stops*/
      uint32_t    _stoppos;

      /** flag showing whether this signal has been used for sorting
       *
       * right now this only used in sorting the detektorhits by simple sorting
       */
      bool        _used;
    };
  }//end namespace acqiris
}//end namespace cass
#endif
