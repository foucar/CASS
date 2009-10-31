#ifndef _MACHINEDATAEVENT_H_
#define _MACHINEDATAEVENT_H_

#include <map>
#include <string>

#ifndef ROOT_Rtypes
#include "Rtypes.h"
#endif

#ifndef ROOT_TObject
#include "TObject.h"
#endif

namespace Pds
{
    class BldDataFEEGasDetEnergy;
}


namespace cass
{
    namespace MachineData
    {
        class MachineDataEvent
        {
        public:
            MachineDataEvent():
                    _isFilled(false),
                    _f_11_ENRC(0),
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
                    _energy(0)
                    {}

            ~MachineDataEvent(){}
        private:
            typedef std::map<std::string,double> EpicsDataMap;

        public:
            bool  isFilled()const       {return _isFilled;}
            bool& isFilled()            {return _isFilled;}

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

            double FitTime1()const     {return _FitTime1;}
            double& FitTime1()         {return _FitTime1;}

            double FitTime2()const     {return _FitTime2;}
            double& FitTime2()         {return _FitTime2;}

            double Charge1()const      {return _Charge1;}
            double& Charge1()          {return _Charge1;}

            double Charge2()const      {return _Charge2;}
            double& Charge2()          {return _Charge2;}

            const EpicsDataMap& EpicsData()const {return _epicsdata;}
            EpicsDataMap& EpicsData() {return _epicsdata;}

        private:
            bool   _isFilled;
            //data comming from machine//
            double _f_11_ENRC;   /* in mJ */
            double _f_12_ENRC;   /* in mJ */
            double _f_21_ENRC;   /* in mJ */
            double _f_22_ENRC;   /* in mJ */

            double _EbeamCharge;    /* in nC */
            double _EbeamL3Energy;  /* in MeV */
            double _EbeamLTUPosX;   /* in mm */
            double _EbeamLTUPosY;   /* in mm */
            double _EbeamLTUAngX;   /* in mrad */
            double _EbeamLTUAngY;   /* in mrad */

            double _FitTime1;   /* in pico-seconds */
            double _FitTime2;   /* in pico-seconds */
            double _Charge1;    /* in pico-columbs */
            double _Charge2;    /* in pico-columbs */

            //epics data//
            EpicsDataMap _epicsdata;


            //data that gets calculated in Analysis//
            double _energy;

            ClassDefNV(MachineDataEvent,1)
        };
    }//end namespace machinedata
}//end namespace cass

#endif
