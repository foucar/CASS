// pnccd_photon_hit.h
// Declaration of a structure which describes a photon hit on
// a pnCCD. For use in the in the pnCCD library for cass.

#ifndef _PNCCDDETECTOR_H
#define _PNCCDDETECTOR_H

#include <vector>
#include <stdint.h>

#ifndef ROOT_Rtypes
#include "Rtypes.h"
#endif

#ifndef ROOT_TObject
#include "TObject.h"
#endif

#ifndef bit32
#define bit32
#endif

namespace cass
{
    namespace pnCCD
    {
        class PhotonHit
        {
        public:
            PhotonHit()     {}
            ~PhotonHit()    {}

        public:
            uint16_t    x()const            {return _x;}
            uint16_t   &x()                 {return _x;}

            uint16_t    y()const            {return _y;}
            uint16_t   &y()                 {return _y;}

            uint16_t    amplitude()const    {return _amplitude;}
            uint16_t   &amplitude()         {return _amplitude;}

            float       energy()const       {return _energy;}
            float      &energy()            {return _energy;}

        private:
            uint16_t _x;                    // x coordinate
            uint16_t _y;                    // y coordinate
            uint16_t _amplitude;            // signal amplitude in adu
            float    _energy;               // energy in eV

            ClassDefNV(PhotonHit,1)         //a photon hit on the detector
        };








        class pnCCDDetector
        {
        public:
            pnCCDDetector():_originalrows(0),_originalcolumns(0),_rows(0),_columns(0)    {}
            ~pnCCDDetector()    {}

        public: //typedefs for better readable code
            typedef std::vector<int32_t>   frame_i32_t;
            typedef std::vector<uint16_t>   frame_t;
            typedef std::vector<PhotonHit>  photonHits_t;

        public:
            const frame_t       &rawFrame()const        {return _rawFrame;}
            frame_t             &rawFrame()             {return _rawFrame;}

#ifdef bit32
            const frame_i32_t       &correctedFrame()const  {return _correctedFrame;}
            frame_i32_t             &correctedFrame()       {return _correctedFrame;}
#else
            const frame_t       &correctedFrame()const  {return _correctedFrame;}
            frame_t             &correctedFrame()       {return _correctedFrame;}
#endif
            const photonHits_t  &recombined()const      {return _recombined;}
            photonHits_t        &recombined()           {return _recombined;}

            const photonHits_t  &nonrecombined()const   {return _nonrecombined;}
            photonHits_t        &nonrecombined()        {return _nonrecombined;}

            bool                 calibrated()const      {return _calibrated;}
            bool                &calibrated()           {return _calibrated;}

            uint16_t             rows()const            {return _rows;}
            uint16_t            &rows()                 {return _rows;}

            uint16_t             columns()const         {return _columns;}
            uint16_t            &columns()              {return _columns;}

            uint16_t             originalrows()const    {return _originalrows;}
            uint16_t            &originalrows()         {return _originalrows;}

            uint16_t             originalcolumns()const {return _originalcolumns;}
            uint16_t            &originalcolumns()      {return _originalcolumns;}

            int64_t              integral()const        {return _integral;}
            int64_t             &integral()             {return _integral;}

        private:
            //infos from the xtc file
            frame_t              _rawFrame;             //! the raw frame
            uint16_t             _originalrows;         //number of rows of the detector
            uint16_t             _originalcolumns;      //number of columns of the detector

            //results of the analysis
#ifdef bit32
            frame_i32_t          _correctedFrame;       //the "massaged" frame
#else
            frame_t              _correctedFrame;       //the "massaged" frame
#endif
            photonHits_t         _recombined;           //vector containing recombined X-ray photon hits
            photonHits_t         _nonrecombined;        //vector containing non-recombined X-ray photon hits
            int64_t              _integral;             //the sum of all pixels in the frame
            uint16_t             _rows;                 //number of rows of the rebinned detector
            uint16_t             _columns;              //number of columns of the rebinned detector
            bool                 _calibrated;           //true if the calibration procedure was run

            ClassDefNV(pnCCDDetector,1)  //a pnccd detector
        };


    } // end of scope of namespace pnCCD
} // end of scope of namespace cass

#endif // PNCCD_PHOTON_HIT_H

// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:

// end of file pnCCDEvent.h

