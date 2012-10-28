#ifndef BLD_DATA_H
#define BLD_DATA_H

#include <stdint.h>

namespace Pds 
{
    
#pragma pack(4)

class BldDataFEEGasDetEnergy
{
    // PV names: GDET:FEE1:11:ENRC,GDET:FEE1:12:ENRC,GDET:FEE1:21:ENRC,GDET:FEE1:22:ENRC
public:
  enum { version=0 };
    double f_11_ENRC;   /* in mJ */ 
    double f_12_ENRC;   /* in mJ */ 
    double f_21_ENRC;   /* in mJ */
    double f_22_ENRC;   /* in mJ */
    
    int print() const;
};

class BldDataEBeamV0
{
public:
  enum { version=0 };
    uint32_t    uDamageMask;
    double      fEbeamCharge;    /* in nC */ 
    double      fEbeamL3Energy;  /* in MeV */ 
    double      fEbeamLTUPosX;   /* in mm */ 
    double      fEbeamLTUPosY;   /* in mm */ 
    double      fEbeamLTUAngX;   /* in mrad */ 
    double      fEbeamLTUAngY;   /* in mrad */  
    
    int print() const;    
};



class BldDataEBeamV1
{
public:
  enum { version=1 };
    uint32_t    uDamageMask;
    double      fEbeamCharge;    /* in nC */
    double      fEbeamL3Energy;  /* in MeV */
    double      fEbeamLTUPosX;   /* in mm */
    double      fEbeamLTUPosY;   /* in mm */
    double      fEbeamLTUAngX;   /* in mrad */
    double      fEbeamLTUAngY;   /* in mrad */
    double      fEbeamPkCurrBC2; /* in Amps */

    int print() const;
};


class BldDataEBeamV2
{
public:
  enum { version=2 };
    uint32_t    uDamageMask;
    double      fEbeamCharge;    /* in nC */
    double      fEbeamL3Energy;  /* in MeV */
    double      fEbeamLTUPosX;   /* in mm */
    double      fEbeamLTUPosY;   /* in mm */
    double      fEbeamLTUAngX;   /* in mrad */
    double      fEbeamLTUAngY;   /* in mrad */
    double      fEbeamPkCurrBC2; /* in Amps */
    double      fEbeamEnergyBC2; /* in MeV */

    int print() const;
};

class BldDataEBeamV3
{
public:
  enum { version=3 };
  enum varname
  {
    NotAssigned,
    EbeamCharge,
    EbeamL3Energy,   /* in MeV */
    EbeamLTUPosX,    /* in mm */
    EbeamLTUPosY,    /* in mm */
    EbeamLTUAngX,    /* in mrad */
    EbeamLTUAngY,    /* in mrad */
    EbeamPkCurrBC2,  /* in Amps */
    EbeamEnergyBC2,  /* in mm */
    EbeamPkCurrBC1, /* Amps */
    EbeamEnergyBC1,  /* in mm */
    nbrOf
  };
    uint32_t    uDamageMask;
    double      fEbeamCharge;     /* in nC */
    double      fEbeamL3Energy;   /* in MeV */
    double      fEbeamLTUPosX;    /* in mm */
    double      fEbeamLTUPosY;    /* in mm */
    double      fEbeamLTUAngX;    /* in mrad */
    double      fEbeamLTUAngY;    /* in mrad */
    double      fEbeamPkCurrBC2;  /* in Amps */
    double      fEbeamEnergyBC2;  /* in mm */
    double      fEbeamPkCurrBC1; /* Amps */
    double      fEbeamEnergyBC1;  /* in mm */

    int print() const;
    static const char* name(varname name);
};

typedef BldDataEBeamV3 BldDataEBeam;

class BldDataPhaseCavity
{
    // PV names: UND:R02:IOC:16:BAT:FitTime1, UND:R02:IOC:16:BAT:FitTime2, 
    //           UND:R02:IOC:16:BAT:Charge1,  UND:R02:IOC:16:BAT:Charge2
public:
  enum { version=0 };
    double fFitTime1;   /* in pico-seconds */ 
    double fFitTime2;   /* in pico-seconds */ 
    double fCharge1;    /* in pico-columbs */ 
    double fCharge2;    /* in pico-columbs */ 
    
    int print() const;
};

#pragma pack()
}
#endif
