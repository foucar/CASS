/*
 *  MachineDataAnalysis.h
 *  diode
 *
 *lmf
 */

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
    class CASS_MACHINEDATASHARED_EXPORT Parameter : public cass::ParameterBackend
    {
    public:
      Parameter()     {beginGroup("MachineData");}
      ~Parameter()    {save();endGroup();}
      void load();
      void save();

     public:
      double _lambda; //the undulator period
      double _K;      //the undulator K
    };


    class CASS_MACHINEDATASHARED_EXPORT Analysis : public cass::AnalysisBackend
    {
    public:
      Analysis()            {loadSettings();}
      ~Analysis()           {}
      void loadSettings()   {_param.load();}
      void saveSettings()   {_param.save();}

      //called for every event//
      void operator()(CASSEvent*);

    private:
      Parameter  _param;
    };
  }//end namespace MachineData
}//end namespace cass

#endif
