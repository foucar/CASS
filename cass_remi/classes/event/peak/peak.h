#ifndef __peak_h__
#define __peak_h__

#include <vector>
#include <stdint.h>


#ifndef ROOT_Rtypes
#include "Rtypes.h"
#endif

#ifndef ROOT_tObject
#include "TObject.h"
#endif


namespace cass
{
    namespace REMI
    {
        class Peak
        {
        public:
            Peak()  {}
            ~Peak() {}

        public:
            enum Polarity{Bad=0,Positive,Negative};

        public:
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
            double      _time;              //the time of the peaks, calculated from either cfd or com
            double      _cfd;               //! the time calculated from cfd
            double      _com;               //! the time calculated from com

            Polarity    _polarity;          //the polarity of the peak
            double      _slope;             //the slope of this peak

            uint32_t    _maxpos;            //! the position where the maximum of peak is
            double      _maximum;           //! the height in bits
            double      _height;            //the height in mV
            double      _heightAbziehen;    //! the height when you use the substraction cfd

            double      _fwhm;              //the _whm of the peak
            double      _width;             //! the width at the bottom of the peak
            double      _posHalfLeft;       //! the pos where the left edge crosses the half of the height
            double      _posHalfRight;      //! the pos where the right edge crosses the half of the height

            double      _integral;          //the integral of the peak

            uint32_t    _startpos;          //! the start postion of the peak
            uint32_t    _stoppos;           //! the stop position of the peak

            bool        _used;              //! _lag wether this peak has been used in sorting the detektorhits

            ClassDefNV(Peak,1)              //An identified Signal in the waveform
        };
    }//end namespace remi
}//end namespace cass
#endif
