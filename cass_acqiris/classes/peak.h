//Copyright (C) 2009,2010 Lutz Foucar

#ifndef _PEAK_H_
#define _PEAK_H_

#include <vector>
#include <stdint.h>

#include "cass_acqiris.h"


namespace cass
{
  namespace ACQIRIS
  {
    /*! @brief A Signal found inside a waveform
      This class contains all information one can extract from a singal in the wavefrom.
      The name peak comes historically.
      @todo can make make it such that the properties are calculated only the
            first time they are requested.
      @todo we need to include what kind of analysis should be used to extract the time information
            from the singal.
      @author Lutz Foucar*/
    class CASS_ACQIRISSHARED_EXPORT Peak
    {
    public:
      /** when a peak is created we have to initialize used to false*/
      Peak():_used(false)  {}

    public:
      /** setter / getters*/
      double      time()const         {return _time;}
      double     &time()              {return _time;}

      double      com()const          {return _com;}
      double     &com()               {return _com;}

      double      cfd()const          {return _cfd;}
      double     &cfd()               {return _cfd;}

      double      integral()const     {return _integral;}
      double     &integral()          {return _integral;}

      double      height()const       {return _height;}
      double     &height()            {return _height;}

      double      width()const        {return _width;}
      double     &width()             {return _width;}

      double      fwhm()const         {return _fwhm;}
      double     &fwhm()              {return _fwhm;}

      uint32_t    startpos()const     {return _startpos;}
      uint32_t   &startpos()          {return _startpos;}

      uint32_t    stoppos()const      {return _stoppos;}
      uint32_t   &stoppos()           {return _stoppos;}

      uint32_t    maxpos()const       {return _maxpos;}
      uint32_t   &maxpos()            {return _maxpos;}

      double      maximum()const      {return _maximum;}
      double     &maximum()           {return _maximum;}

      Polarity    polarity()const     {return _polarity;}
      Polarity   &polarity()          {return _polarity;}

      bool        isUsed()const       {return _used;}
      bool       &isUsed()            {return _used;}

    private:
      /** the time of the signal, depending on which kind
          kind of extraction is requested this value will
          either represent the center of mass, or the constant
          fraction.
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
      /** flag showing whether this signal has been used in sorting the detektorhits used for simple sorting*/
      bool        _used;
    };
  }//end namespace remi
}//end namespace cass
#endif
