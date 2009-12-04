// Copyright (C) 2009 Jochen Küpper , Nils Kimmel,lmf

#ifndef PNCCDANALYSIS_H
#define PNCCDANALYSIS_H

#include <map>
#include <string>
#include <vector>
#include "cass_pnccd.h"
#include "analysis_backend.h"
#include "parameter_backend.h"

//#include <QtGui/QImage>


namespace cass
{
  class CASSEvent;
  namespace pnCCD
  {


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

      //rebin factors for each detector//
      std::vector<uint32_t> _rebinfactors;
      //offsets for each detector//
      std::vector<std::vector<double> > _offsets;
      //noise for each detector//
      std::vector<std::vector<double> > _noise;
      //the number of fills for each detector//
      std::vector<size_t> _nbrDarkframes;
      //the multiplier how much times the sigma is the noise level//
      std::vector<double> _sigmaMultiplier;
      //the conversion factor to convert "adu's" to eV//
      std::vector<double> _adu2eV;
      //the remichannel to that will tell us whether there is light in the chamber//
      uint32_t _lightIndicatorChannel;
      // Dark frame calibration file names for each detector//
      std::vector<std::string> _darkcal_fnames;
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
      basic parameters are reacorded, e.g. the number of detected events
      in the frame.
      */
      void operator() (cass::CASSEvent*);
    private:
      Parameter _param;
      // The frame analysis object:
      std::vector<pnCCDFrameAnalysis *> _pnccd_analyzer;
      //temporary storage for rebinning frames//
      //!!! this is not thread safe if this is a singleton//
      std::vector<uint64_t> _tmp;
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
