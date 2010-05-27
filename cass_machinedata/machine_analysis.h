// Copyright (C) 2009,2010 Lutz Foucar

#ifndef _MACHINEDATAANALYSIS_H
#define _MACHINEDATAANALYSIS_H


#include "cass_machine.h"
#include "analysis_backend.h"
#include "parameter_backend.h"

namespace cass
{
  class CASSEvent;

  namespace MachineData
  {
    /*! Parameters for the Machine Data Analysis.
      @author Lutz Foucar
    */
    class CASS_MACHINEDATASHARED_EXPORT Parameter : public cass::ParameterBackend
    {
    public:
      /** constructor creating group "MachineData"*/
      Parameter()     {beginGroup("MachineData");}
      /** destructor closing group "MachineData"*/
      ~Parameter()    {endGroup();}
      /** load the parameters from cass.ini*/
      void load();
      /** save parameters to cass.ini*/
      void save();

     public:
      double _lambda; //!< the undulator period
      double _K;      //!< the undulator K
    };


    /*! The Machine Data Analyzis.
      functor doing calculations on the machine data.
      This uses Rick K. code at psexport.slac.stanford.edu:/reg/neh/home/rkirian/ana2
      to calculate the photon energy and wavelength from the Beamlinedata
      @author Lutz Foucar
      */
    class CASS_MACHINEDATASHARED_EXPORT Analysis : public cass::AnalysisBackend
    {
    public:
      /** constructor loading the settings*/
      Analysis()            {}
      /** load settings from file*/
      void loadSettings()   {_param.load();}
      /** save settings to file*/
      void saveSettings()   {}
      /** called for every event, does the analysis */
      void operator()(CASSEvent*);

    private:
      Parameter  _param; //!< the parameters
    };
  }//end namespace MachineData
}//end namespace cass

#endif
