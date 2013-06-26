#include <stdio.h>
#include <string.h>
#include "pdsdata/bld/bldData.hh"

using namespace Pds;

int BldDataFEEGasDetEnergy::print() const
{    
    printf("GDET:FEE:11:ENRC ( in mJ ): %lf\n", f_11_ENRC );    
    printf("GDET:FEE:12:ENRC ( in mJ ): %lf\n", f_12_ENRC );
    printf("GDET:FEE:21:ENRC ( in mJ ): %lf\n", f_21_ENRC ); 
    printf("GDET:FEE:22:ENRC ( in mJ ): %lf\n", f_22_ENRC );
    
    return 0;
}

int BldDataEBeamV0::print() const
{    
    printf( "ebeamCharge   ( in nC )  : %lf\n", fEbeamCharge  ); 
    printf( "ebeamL3Energy ( in MeV ) : %lf\n", fEbeamL3Energy); 
    printf( "ebeamLTUPosX  ( in mm )  : %lf\n", fEbeamLTUPosX ); 
    printf( "ebeamLTUPosY  ( in mm )  : %lf\n", fEbeamLTUPosY ); 
    printf( "ebeamLTUAngX  ( in mrad ): %lf\n", fEbeamLTUAngX ); 
    printf( "ebeamLTUAngY  ( in mrad ): %lf\n", fEbeamLTUAngY );
    return 0;
}

int BldDataEBeamV1::print() const
{    
    printf( "ebeamCharge   ( in nC )  : %lf\n", fEbeamCharge  ); 
    printf( "ebeamL3Energy ( in MeV ) : %lf\n", fEbeamL3Energy); 
    printf( "ebeamLTUPosX  ( in mm )  : %lf\n", fEbeamLTUPosX ); 
    printf( "ebeamLTUPosY  ( in mm )  : %lf\n", fEbeamLTUPosY ); 
    printf( "ebeamLTUAngX  ( in mrad ): %lf\n", fEbeamLTUAngX ); 
    printf( "ebeamLTUAngY  ( in mrad ): %lf\n", fEbeamLTUAngY );
    printf( "ebeamPkCurrBC2( in A )   : %lf\n", fEbeamPkCurrBC2 );
    return 0;
}

int BldDataEBeamV2::print() const
{    
    printf( "ebeamCharge   ( in nC )  : %lf\n", fEbeamCharge  ); 
    printf( "ebeamL3Energy ( in MeV ) : %lf\n", fEbeamL3Energy); 
    printf( "ebeamLTUPosX  ( in mm )  : %lf\n", fEbeamLTUPosX ); 
    printf( "ebeamLTUPosY  ( in mm )  : %lf\n", fEbeamLTUPosY ); 
    printf( "ebeamLTUAngX  ( in mrad ): %lf\n", fEbeamLTUAngX ); 
    printf( "ebeamLTUAngY  ( in mrad ): %lf\n", fEbeamLTUAngY );
    printf( "ebeamPkCurrBC2( in A )   : %lf\n", fEbeamPkCurrBC2 );
    printf( "ebeamEnergyBC2( in MeV ) : %lf\n", fEbeamEnergyBC2 );
    return 0;
}

int BldDataEBeamV3::print() const
{    
    printf( "ebeamCharge   ( in nC )  : %lf\n", fEbeamCharge  ); 
    printf( "ebeamL3Energy ( in MeV ) : %lf\n", fEbeamL3Energy); 
    printf( "ebeamLTUPosX  ( in mm )  : %lf\n", fEbeamLTUPosX ); 
    printf( "ebeamLTUPosY  ( in mm )  : %lf\n", fEbeamLTUPosY ); 
    printf( "ebeamLTUAngX  ( in mrad ): %lf\n", fEbeamLTUAngX ); 
    printf( "ebeamLTUAngY  ( in mrad ): %lf\n", fEbeamLTUAngY );
    printf( "ebeamPkCurrBC2( in A )   : %lf\n", fEbeamPkCurrBC2 );
    printf( "ebeamEnergyBC2( in MeV ) : %lf\n", fEbeamEnergyBC2 );
    printf( "ebeamPkCurrBC1( in A )   : %lf\n", fEbeamPkCurrBC1 );
    printf( "ebeamEnergyBC1( in MeV ) : %lf\n", fEbeamEnergyBC1 );
    return 0;
}

int BldDataPhaseCavity::print() const
{    
    printf("FitTime1 ( in pico-seconds ): %lf\n", fFitTime1 );
    printf("FitTime2 ( in pico-seconds ): %lf\n", fFitTime2 );
    printf("Charge1  ( in pico-columbs ): %lf\n", fCharge1 );
    printf("Charge2  ( in pico-columbs ): %lf\n", fCharge2 );
    
    return 0;
}

int BldDataIpimbV0::print() const 
{   
    printf("BLD Shared IPIMB Data:\n");
    printf("  Trig Count      : %llu \n", (long long) ipimbData.triggerCounter());
    printf("  IpimbDataCh-0   : %f   \n",ipimbData.channel0Volts());
    printf("  IpimbDataCh-1   : %f   \n",ipimbData.channel1Volts());      
    printf("  IpimbDataCh-2   : %f   \n",ipimbData.channel2Volts());  
    printf("  IpimbDataCh-3   : %f   \n",ipimbData.channel3Volts());

    printf("  IpmFexDataCh-0  : %f   \n",ipmFexData.channel[0]);  
    printf("  IpmFexDataCh-1  : %f   \n",ipmFexData.channel[1]);
    printf("  IpmFexDataCh-2  : %f   \n",ipmFexData.channel[2]);
    printf("  IpmFexDataCh-3  : %f   \n",ipmFexData.channel[3]);
    printf("  IpmFexDataSum   : %f   \n",ipmFexData.sum);
    printf("  IpmFexDataXpos  : %f   \n",ipmFexData.xpos);
    printf("  IpmFexDataYpos  : %f   \n",ipmFexData.ypos);
    
    return 0;
}

int BldDataIpimbV1::print() const 
{   
    printf("BLD Shared IPIMB Data:\n");
    printf("  Trig Count      : %llu \n", (long long) ipimbData.triggerCounter());
    printf("  IpimbDataCh-0   : %f   \n",ipimbData.channel0Volts());
    printf("  IpimbDataCh-1   : %f   \n",ipimbData.channel1Volts());      
    printf("  IpimbDataCh-2   : %f   \n",ipimbData.channel2Volts());  
    printf("  IpimbDataCh-3   : %f   \n",ipimbData.channel3Volts());
    printf("  IpimbDataCh-0ps : %f   \n",ipimbData.channel0psVolts());
    printf("  IpimbDataCh-1ps : %f   \n",ipimbData.channel1psVolts());      
    printf("  IpimbDataCh-2ps : %f   \n",ipimbData.channel2psVolts());  
    printf("  IpimbDataCh-3ps : %f   \n",ipimbData.channel3psVolts());

    printf("  IpmFexDataCh-0  : %f   \n",ipmFexData.channel[0]);  
    printf("  IpmFexDataCh-1  : %f   \n",ipmFexData.channel[1]);
    printf("  IpmFexDataCh-2  : %f   \n",ipmFexData.channel[2]);
    printf("  IpmFexDataCh-3  : %f   \n",ipmFexData.channel[3]);
    printf("  IpmFexDataSum   : %f   \n",ipmFexData.sum);
    printf("  IpmFexDataXpos  : %f   \n",ipmFexData.xpos);
    printf("  IpmFexDataYpos  : %f   \n",ipmFexData.ypos);
    
    return 0;
}

int BldDataGMDV0::print() const 
{   
  printf("BLD GMD Data V0:\n");
  
  char strGasTypeTemp[sizeof(strGasType)+1];
  strncpy(strGasTypeTemp, strGasType, sizeof(strGasType));
  strGasTypeTemp[sizeof(strGasTypeTemp)-1] = 0;
    
  printf("  Gas Type                        : %s\n", strGasTypeTemp);
  printf("  Pressure (Spinning Rotor Gauge) : %lf\n", fPressure);
  printf("  Temperature (PT100)             : %lf\n", fTemperature);
  printf("  Current (Keithley Electrometer) : %lf\n", fCurrent);
  printf("  Voltage (HV Mesh Electron)      : %lf\n", fHvMeshElectron);
  printf("  Voltage (HV Mesh Ion)           : %lf\n", fHvMeshIon);
  printf("  Voltage (HV Mult Ion)           : %lf\n", fHvMultIon);
  printf("  Charge Q                        : %lf\n", fChargeQ);
  printf("  Photon Energy                   : %lf\n", fPhotonEnergy);
  printf("  Pulse Intensity (Electron Multiplier) : %lf\n", fMultPulseIntensity);
  printf("  Pulse Intensity (ION cup current)     : %lf\n", fKeithleyPulseIntensity);
  printf("  Pulse Energy (Electron Multiplier)    : %lf\n", fPulseEnergy);
  printf("  Pulse Energy (FEE Gas Detector)       : %lf\n", fPulseEnergyFEE);
  printf("  Transmission (Electron Multiplier)    : %lf\n", fTransmission);
  printf("  Transmission (FEE Gas Detector)       : %lf\n", fTransmissionFEE);
  printf("  Spare 6                         : %lf\n", fSpare6);
  
  return 0;
}

int BldDataGMDV1::print() const
{
  printf("BLD GMD Data V1:\n");
  
  printf("  Shot to shot pulse energy (mJ)                 : %lf\n", fMilliJoulesPerPulse);
  printf("  Average pulse energy from ION cup current (mJ) : %lf\n", fMilliJoulesAverage);
  printf("  Bg corrected waveform (counts)                 : %lf\n", fCorrectedSumPerPulse);
  printf("  Avg background value per sample (counts)       : %lf\n", fBgValuePerSample);
  printf("  Shot by shot pulse energy (no units)           : %lf\n", fRelativeEnergyPerPulse);
  printf("  Spare value 1                                  : %lf\n", fSpare1);
  
  return 0;
}





