#ifndef BLD_DATA_H
#define BLD_DATA_H

#include <stdint.h>
#include "pdsdata/ipimb/ConfigV1.hh"
#include "pdsdata/ipimb/DataV1.hh"
#include "pdsdata/ipimb/ConfigV2.hh"
#include "pdsdata/ipimb/DataV2.hh"
#include "pdsdata/lusi/IpmFexV1.hh"
#include "pdsdata/lusi/PimImageConfigV1.hh"
#include "pdsdata/pulnix/TM6740ConfigV2.hh"
#include "pdsdata/camera/FrameV1.hh"
#include "pdsdata/acqiris/ConfigV1.hh"
#include "pdsdata/acqiris/DataDescV1.hh"
#include "pdsdata/xtc/DetInfo.hh"

typedef Pds::Ipimb::DataV1   IpimbDataV1;
typedef Pds::Ipimb::ConfigV1 IpimbConfigV1; 
typedef Pds::Ipimb::DataV2   IpimbDataV2;
typedef Pds::Ipimb::ConfigV2 IpimbConfigV2; 
typedef Pds::Lusi::IpmFexV1  IpmFexDataV1;

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

  // uDamageMask bits
  enum { EbeamChargeDamage   = 0x001,
         EbeamL3EnergyDamage = 0x002,
         EbeamLTUPosXDamage  = 0x004,
         EbeamLTUPosYDamage  = 0x008,
         EbeamLTUAngXDamage  = 0x010,
         EbeamLTUAngYDamage  = 0x020 };

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

  // uDamageMask bits
  enum { EbeamChargeDamage    = 0x001,
         EbeamL3EnergyDamage  = 0x002,
         EbeamLTUPosXDamage   = 0x004,
         EbeamLTUPosYDamage   = 0x008,
         EbeamLTUAngXDamage   = 0x010,
         EbeamLTUAngYDamage   = 0x020,
         EbeamPkCurrBC2Damage = 0x040 };

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

  // uDamageMask bits
  enum { EbeamChargeDamage    = 0x001,
         EbeamL3EnergyDamage  = 0x002,
         EbeamLTUPosXDamage   = 0x004,
         EbeamLTUPosYDamage   = 0x008,
         EbeamLTUAngXDamage   = 0x010,
         EbeamLTUAngYDamage   = 0x020,
         EbeamPkCurrBC2Damage = 0x040,
         EbeamEnergyBC2Damage = 0x080 };

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

  // uDamageMask bits
  enum { EbeamChargeDamage    = 0x001,
         EbeamL3EnergyDamage  = 0x002,
         EbeamLTUPosXDamage   = 0x004,
         EbeamLTUPosYDamage   = 0x008,
         EbeamLTUAngXDamage   = 0x010,
         EbeamLTUAngYDamage   = 0x020,
         EbeamPkCurrBC2Damage = 0x040,
         EbeamEnergyBC2Damage = 0x080,
         EbeamPkCurrBC1Damage = 0x100,
         EbeamEnergyBC1Damage = 0x200 };

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


class BldDataIpimbV0
{
public:
  enum { version=0 }; 
    IpimbDataV1    ipimbData;
    IpimbConfigV1  ipimbConfig;
    IpmFexDataV1   ipmFexData;
    
    int print() const;    
};


class BldDataIpimbV1
{
public:
  enum { version=1 }; 
    IpimbDataV2    ipimbData;
    IpimbConfigV2  ipimbConfig;
    IpmFexDataV1   ipmFexData;
    
    int print() const;    
};

typedef BldDataIpimbV1 BldDataIpimb;

class BldDataPimV1
{
public:
  enum { version=1 };
  Pulnix::TM6740ConfigV2   camConfig;
  Lusi::PimImageConfigV1   pimConfig;
  Camera::FrameV1          frame;
};

class BldDataGMDV0
{
public:
  enum    {version = 0};

  char    strGasType[32];         // Gas Type
  double  fPressure;              // Pressure from Spinning Rotor Gauge
  double  fTemperature;           // Temp from PT100
  double  fCurrent;               // Current from Keithley Electrometer
  double  fHvMeshElectron;        // HV Mesh Electron
  double  fHvMeshIon;             // HV Mesh Ion
  double  fHvMultIon;             // HV Mult Ion
  double  fChargeQ;               // Charge Q
  double  fPhotonEnergy;          // Photon Energy
  double  fMultPulseIntensity;    // Pulse Intensity derived from Electron Multiplier
  double  fKeithleyPulseIntensity;// Pulse Intensity derived from ION cup current
  double  fPulseEnergy;           // Pulse Energy derived from Electron Multiplier
  double  fPulseEnergyFEE;        // Pulse Energy from FEE Gas Detector
  double  fTransmission;          // Transmission derived from Electron Multiplier
  double  fTransmissionFEE;       // Transmission from FEE Gas Detector
  double  fSpare6;                // Spare 6

  int print() const;
};

class BldDataGMDV1
{
public:
  enum    {version = 1};
  double  fMilliJoulesPerPulse;    // Shot to shot pulse energy (mJ)
  double  fMilliJoulesAverage;     // Average pulse energy from ION cup current (mJ)
  double  fCorrectedSumPerPulse;   // Bg corrected waveform integrated within limits in raw A/D counts
  double  fBgValuePerSample;       // Avg background value per sample in raw A/D counts
  double  fRelativeEnergyPerPulse; // Shot by shot pulse energy in arbitrary units
  double  fSpare1;                 // Spare value for use as needed
 
  int print() const;
};


typedef BldDataGMDV1 BldDataGMD;

class BldDataAcqADCV1
{
public:
  enum { version=1 };
  Acqiris::ConfigV1   config;
  Acqiris::DataDescV1 data;
};

typedef BldDataAcqADCV1 BldDataAcqADC;

#pragma pack()
}
#endif
