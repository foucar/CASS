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

            ClassDefNV(PhotonHit,1)     //a photon hit on the detector
        };








        class pnCCDDetector
        {
        public:
            pnCCDDetector():_rows(1024),_columns(1024)     {}
            ~pnCCDDetector()    {}

        public: //typedefs for better readable code
            typedef std::vector<int16_t>   frame_t;
            typedef std::vector<PhotonHit>  photonHits_t;

        public:
            const frame_t       &rawFrame()const        {return _rawFrame;}
            frame_t             &rawFrame()             {return _rawFrame;}

            const frame_t       &correctedFrame()const  {return _correctedFrame;}
            frame_t             &correctedFrame()       {return _correctedFrame;}

            const photonHits_t  &recombined()const      {return _recombined;}
            photonHits_t        &recombined()           {return _recombined;}

            const photonHits_t  &nonrecombined()const   {return _nonrecombined;}
            photonHits_t        &nonrecombined()        {return _nonrecombined;}

            uint16_t             rows()const            {return _rows;}
            uint16_t            &rows()                 {return _rows;}

            uint16_t             columns()const         {return _columns;}
            uint16_t            &columns()              {return _columns;}

            uint32_t             integral()const        {return _integral;}
            uint32_t            &integral()             {return _integral;}

        private:
            //infos from the xtc file
            frame_t              _rawFrame;             //! the raw frame
            uint16_t             _rows;                 //number of rows of the detector
            uint16_t             _columns;              //number of columns of the detector

            //results of the analysis
            frame_t              _correctedFrame;       //the "massaged" frame
            photonHits_t         _recombined;           //vector containing recombined X-ray photon hits
            photonHits_t         _nonrecombined;        //vector containing non-recombined X-ray photon hits
            uint32_t             _integral;             //the sum of all pixels in the frame

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

