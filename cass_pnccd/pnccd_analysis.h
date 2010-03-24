// Copyright (C) 2009 Jochen Küpper , Nils Kimmel,lmf, Nicola Coppola

#ifndef PNCCDANALYSIS_H
#define PNCCDANALYSIS_H

#include <map>
#include <string>
#include <vector>
#include "cass_pnccd.h"
#include "analysis_backend.h"
#include "parameter_backend.h"

//#include <QtGui/QImage>
#include <QtCore/QMutex>
#include <QtCore/QObject>


namespace cass
{
  class CASSEvent;
  namespace pnCCD
  {

    class ROIsimple
    {
    /* inserting the "definition" of a ROI
       each ROI need the following "attributes": shape, xsize, ysize, xcenter, ycenter
       shapes:=circ(==circle),triangle(isosceles),square <=Do I need many squares per frame?
       AAAA there is a problem with a triangular shape... the orientation!!!
       the orientation is used only in the case of a triangular shape

       I think also a "double triangle bottle-like shape could be helpful

       xsize,ysize and center are in pixel units

       Do I need to shrink the ROI if I am rebinning??
    */
    public:
      // the shape name(s)
      // or   enum NameTypes {circ=0, triangle=1, square=2};
      std::string name;
      // the size(s) along the x axis
      uint32_t xsize;
      // the size(s) along the y axis
      uint32_t ysize;
      // the centre(s) along the x axis
      uint32_t xcentre;
      // the centre(s) along the y axis
      uint32_t ycentre;
      /*
           the orientation is used only in the case of a triangular shape
            /\           ----         |\           /|
           /  \  ==+1    \  /  ==-1   | \  ==+2   / | == -2
           ----           \/          | /         \ |
                                      |/           \|
         if I rotate the plane by -pi/2: -2=>+1 1=>+2 -1=>-2  +2=>-1
         please remember to use the rotated frame wrt standard-natural frame
         orientation!!
      */
      // the orientation
      int32_t orientation;
    };

    class detROI_t
    {
    public:
      std::vector<ROIsimple> _ROI;
      //_ROI_t _ROI;
      // to which detector the ROI belongs
      uint32_t detID;
    };


    class CASS_PNCCDSHARED_EXPORT Parameter : public cass::ParameterBackend
    {
      /*
      Parameters needed for the pnCCDs. CAMP will typically use
      two one-megapixel detector modules consisting of two pnCCDs
      with 512 x 1024 pixels each.
      */
    public:
      /*
      Constructor: assign the parameters with safe default values:
      */
      Parameter(void) {beginGroup("pnCCD");}
      ~Parameter()    {endGroup();}
      void load();
      void save();

      //flag to set the dark/not-dark run condition
      bool _This_is_a_dark_run;
        //std::vector<size_t> _pnCCDOriginalRows;
        //std::vector<size_t> _pnCCDOriginalCols;

      //rebin factors for each detector//
      std::vector<uint32_t> _rebinfactors;
      //offsets for each detector//
      std::vector<std::vector<double> > _offsets;
      //noise for each detector//
      std::vector<std::vector<double> > _noise;
      //the damping coefficient//
      std::vector<double> _dampingCoefficient;
      //the number of fills for each detector//
      std::vector<size_t> _nbrDarkframes;
      //the multiplier how much times the sigma is the noise level//
      std::vector<double> _sigmaMultiplier;
      //the conversion factor to convert "adu's" to eV//
      std::vector<double> _adu2eV;
      //need to know if I need to use a common mode subtraction scheme//
      std::vector<uint32_t> _useCommonMode;
      //the remichannel to that will tell us whether there is light in the chamber//
      uint32_t _lightIndicatorChannel;
      // Dark frame calibration file names for each detector//
      std::vector<std::string> _darkcal_fnames;
      // Dark frame calibration save file names for each detector//
      std::vector<std::string> _save_darkcal_fnames;

      typedef std::vector<detROI_t> _detROI;
      _detROI detROI;
      //the ROI mask for each detector//
      std::vector<std::vector<uint16_t> > _ROImask;
      //the ROI index-pointer-mask for each detector//
      std::vector<std::vector<uint32_t> > _ROIiterator;
    };


    class pnCCDFrameAnalysis;

    class CASS_PNCCDSHARED_EXPORT Analysis : public cass::AnalysisBackend
    {
    public:
      Analysis(void);
      ~Analysis();
      /*
      initialize AnalysisBackend with new set of parameters
      */
      void loadSettings();
      void saveSettings();
      /*
      Put the pnCCDEvent object through the analysis chain. The original data
      remain unchanged, a new corrected pnCCD image is generated and X-ray
      photon hits are extracted if the user wishes to so. In addition, some
      basic parameters are recorded, e.g. the number of detected events
      in the frame.
      */
      void operator() (cass::CASSEvent*);
    protected:
        /** @brief Singleton operation locker in a multi-threaded environment. */
        static QMutex _mutex;

    private:
      Parameter _param;

      // The frame analysis object:
      std::vector<pnCCDFrameAnalysis *> _pnccd_analyzer;
      //temporary storage for rebinning frames//
      //!!! this is not thread safe if this is a singleton//
      std::vector<uint64_t> _tmp;
      std::vector<float> _tmpf;

        //is this the best place??
      typedef std::vector<uint16_t>   maskframe_t;
      std::vector<maskframe_t>       mask_frame;

      //inherited from fformat.h
      typedef  int16_t        pxType;
      //inherited from xonline_data_types.h
      typedef struct
      {
        //    int          count;       // internal use
        double sum;         // internal use
        //    pxType mean;      // offset mean value of pixel (common mode corrected)
        double offset;      // offset mean value of pixel (raw)
        double sigma;       // noise sigma value of pixel
        double sumSq;       // internal use
        int    count;       // internal use
        pxType mean;        // offset mean value of pixel (common mode corrected)

      } staDataType;



    };


  } // end of scope of namespace pnCCD
} // end of scope of namespace cass

#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
