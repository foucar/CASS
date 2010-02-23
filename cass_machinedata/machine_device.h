#ifndef _MACHINEDATADEVICE_H_
#define _MACHINEDATADEVICE_H_

#include <map>
#include <string>

#include "device_backend.h"
#include "cass_machine.h"

namespace cass
{
  namespace MachineData
  {
    class CASS_MACHINEDATASHARED_EXPORT MachineDataDevice : public cass::DeviceBackend
    {
    public:
      MachineDataDevice()
        :_f_11_ENRC(0),
        _f_12_ENRC(0),
        _f_21_ENRC(0),
        _f_22_ENRC(0),
        _EbeamCharge(0),
        _EbeamL3Energy(0),
        _EbeamLTUPosX(0),
        _EbeamLTUPosY(0),
        _EbeamLTUAngX(0),
        _EbeamLTUAngY(0),
        _FitTime1(0),
        _FitTime2(0),
        _Charge1(0),
        _Charge2(0),
        _energy(0),
        _wavelength(0)
      {
      }

      ~MachineDataDevice()  {}

    public:
      typedef std::map<std::string,double> epicsDataMap_t;

    public:
      double f_11_ENRC()const    {return _f_11_ENRC;}
      double& f_11_ENRC()        {return _f_11_ENRC;}

      double f_12_ENRC()const    {return _f_12_ENRC;}
      double& f_12_ENRC()        {return _f_12_ENRC;}

      double f_21_ENRC()const    {return _f_21_ENRC;}
      double& f_21_ENRC()        {return _f_21_ENRC;}

      double f_22_ENRC()const    {return _f_22_ENRC;}
      double& f_22_ENRC()        {return _f_22_ENRC;}

      double energy()const       {return _energy;}
      double& energy()           {return _energy;}

      double wavelength()const   {return _energy;}
      double& wavelength()       {return _energy;}

      double EbeamCharge()const  {return _EbeamCharge;}
      double& EbeamCharge()      {return _EbeamCharge;}

      double EbeamL3Energy()const{return _EbeamL3Energy;}
      double& EbeamL3Energy()    {return _EbeamL3Energy;}

      double EbeamLTUPosX()const {return _EbeamLTUPosX;}
      double& EbeamLTUPosX()     {return _EbeamLTUPosX;}

      double EbeamLTUPosY()const {return _EbeamLTUPosY;}
      double& EbeamLTUPosY()     {return _EbeamLTUPosY;}

      double EbeamLTUAngX()const {return _EbeamLTUAngX;}
      double& EbeamLTUAngX()     {return _EbeamLTUAngX;}

      double EbeamLTUAngY()const {return _EbeamLTUAngY;}
      double& EbeamLTUAngY()     {return _EbeamLTUAngY;}

      double EbeamPkCurrBC2()const {return _EbeamPkCurrBC2;}
      double& EbeamPkCurrBC2()     {return _EbeamPkCurrBC2;} 

      double FitTime1()const     {return _FitTime1;}
      double& FitTime1()         {return _FitTime1;}

      double FitTime2()const     {return _FitTime2;}
      double& FitTime2()         {return _FitTime2;}

      double Charge1()const      {return _Charge1;}
      double& Charge1()          {return _Charge1;}

      double Charge2()const      {return _Charge2;}
      double& Charge2()          {return _Charge2;}

      const epicsDataMap_t& EpicsData()const {return _epicsdata;}
      epicsDataMap_t& EpicsData() {return _epicsdata;}

    private:
      //data comming from machine//
      double _f_11_ENRC;      //pulsenergy in mJ
      double _f_12_ENRC;      //pulsenergy in mJ
      double _f_21_ENRC;      //pulsenergy in mJ
      double _f_22_ENRC;      //pulsenergy in mJ

      double _EbeamCharge;    // in nC
      double _EbeamL3Energy;  // in MeV
      double _EbeamLTUPosX;   // in mm
      double _EbeamLTUPosY;   // in mm
      double _EbeamLTUAngX;   // in mrad
      double _EbeamLTUAngY;   // in mrad
      double _EbeamPkCurrBC2; // in Amps

      double _FitTime1;       //cavity property in pico-seconds
      double _FitTime2;       //cavity property in pico-seconds
      double _Charge1;        //cavity property in pico-columbs
      double _Charge2;        //cavity property in pico-columbs

      //epics data//
      epicsDataMap_t _epicsdata;//a map containing all epics data in the xtc stream

      //data that gets calculated in Analysis//
      double _energy;         //the calculated puls energy
      double _wavelength;     //the corrosponding wavelength

    };
  }//end namespace machinedata
}//end namespace cass

#endif
