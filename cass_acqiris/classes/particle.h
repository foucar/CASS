//Copyright (C) 2009-2010 Lutz Foucar

/**
 * @file particle.h file contains the classes that describe a particle that hit
 *                       a delayline detector.
 *
 * @author Lutz Foucar
 */

#ifndef __MyParticle_H__
#define __MyParticle_H__

#include <vector>
#include <string>

#include "map.hpp"
#include "delayline_detector.h"
#include "momenta_calculator.h"

namespace cass
{
  class CASSSettings;

  namespace ACQIRIS
  {
    class IsParticleHit;

    /** A Particle
     *
     * class describing a particle
     *
     * @author Lutz Foucar
     */
    class Particle
    {
    public:
      /** constructor
       *
       * set the _listIsCreated flag to false
       */
      Particle()
        :_listIsCreated(false),
         _isParticleHit(0)
      {}

      /** destructor
       *
       * deletes the functors
       */
      ~Particle();

      /** load the settings from .ini file
       *
       * @param s the CASSSettings object to read the info from
       */
      void loadSettings(CASSSettings &s);

      /** retrieve the particle hits
       *
       * goes through the detectorhits and checks whether the hit is a particle
       * of our type. Then calculates the momenta in kartesian and
       * polarcoordinates and puts the new hit into the list. Then returns the
       * list.
       * When the former is already done once, just return the list.
       *
       * @return the list of particle hits
       */
      particleHits_t& hits();

      /** tell which are the detector hits to search through
       *
       * after telling us which detectohit list to go through to search for
       * particle hits, this resets the _listIsCreated flag and clears the
       * _particlehits container.
       *
       * @param pointer to the list of detectorhits
       */
      void associate(detectorHits_t * dethits);

    private:
      /** the list of particle hits */
      particleHits_t _particlehits;

      /** pointer to the list of detectorhits */
      detectorHits_t *_detectorhits;

      /** flag to tell whether we already created the list of particle hits */
      bool _listIsCreated;

      /** identifer for checking detectorhit is a particle hit */
      IsParticleHit *_isParticleHit;

      /** functor for calculating momenta of a particle hit */
      MomentumCalculator _momcalc;

//    public:
//      //checks//
//      bool							 operator==(const MyParticle &p)const		{return (fName.CompareTo(p.fName) == 0);}
//      bool							 operator!=(const MyParticle &p)const		{return (!(*this==p));}
//      bool							 operator==(const char * name)const			{return (fName.CompareTo(name) == 0);}
//      bool							 operator!=(const char * name)const			{return (!(*this==name));}
//      bool							 CheckTof(const MyDetektorHit &dh)			{return ((dh.Tof() > fCondTofFr) && (dh.Tof() < fCondTofTo));}
//      bool							 CheckPos(const MyDetektorHit &dh)			{return fPosFlag?CheckQuad(dh):CheckRad(dh);}
//      bool							 CheckRad(const MyDetektorHit &dh)			{return (TMath::Sqrt((dh.X()-fCondRadX)*(dh.X()-fCondRadX) + (dh.Y()-fCondRadY)*(dh.Y()-fCondRadY)) < fCondRad);}
//      bool							 CheckQuad(const MyDetektorHit &dh)			{return (TMath::Abs(dh.X()-fCondRadX) < fCondWidthX) && (TMath::Abs(dh.Y()-fCondRadY) < fCondWidthY);}
//      bool							 CheckTofAndPos(const MyDetektorHit &dh)	{return (CheckTof(dh) && CheckPos(dh));}
//      //particlehit stuff//
//      const MyParticleHit				&AddHit(const MyDetektorHit&);
//      const size_t					 GetNbrOfParticleHits()const 		{return fPh.size();}
//      const MyParticleHit				&operator[](size_t idx)const		{return fPh[idx];}
//      void							 Clear()							{fPh.clear();}
//      //properties and settings for this particle
//      const MySpectrometer			&GetSpectrometer()const				{return fSp;}
//      const char						*GetName()const						{return fName.Data();}
//      double							 GetXcor()const						{return fXcor;}
//      double							 GetYcor()const						{return fYcor;}
//      double							 GetSfx()const						{return fSfx;}
//      double							 GetSfy()const						{return fSfy;}
//      double							 GetAngle()const					{return fAngle;}
//      double							 GetT0()const						{return fT0;}
//
//      double							 GetCondTofFr()const				{return fCondTofFr;}
//      double							 GetCondTofTo()const				{return fCondTofTo;}
//      double							 GetCondTofRange()const				{return fCondTofTo-fCondTofFr;}
//      double							 GetCondRad()const					{return fCondRad;}
//      double							 GetCondRadX()const					{return fCondRadX;}
//      double							 GetCondRadY()const					{return fCondRadY;}
//      double							 GetCondWidthX()const				{return fCondWidthX;}
//      double							 GetCondWidthY()const				{return fCondWidthY;}
//      bool							 GetPosFlag()const					{return fPosFlag;}
//
//      double							 GetCharge_au()const				{return fCharge_au;}
//      double							 GetMass_au()const					{return fMass_au;}
//
//    private:
//      particleHits					 fPh;								//this contains all Hits on the detektor that belong to this particle
//      double							 fCondTofFr;						//!the lower edge of the time of flight that this particle will be in
//      double							 fCondTofTo;						//!the upper edge of the time of flight that this particle will be in
//      double							 fCondRad;							//!the Radius of the circle on the detektor that this particle will be in
//      double							 fCondRadX;							//!the x-center of the circle on the detektor that this particle will be in
//      double							 fCondRadY;							//!the y-center of the circle on the detektor that this particle will be in
//      double							 fCondWidthX;						//!the width x when choosing a quad condition
//      double							 fCondWidthY;						//!the width y when choosing a quad condition
//      bool							 fPosFlag;							//!flag to tell wether you want a radius or quad condition
//
//      double							 fAngle;							//!the angle to turn the detector in RAD; will be setted in deg but converted to rad while setting it
//      double							 fXcor;								//!to move the center of the distribution to center of raw detektor
//      double							 fYcor;								//!to move the center of the distribution to center of raw detektor
//      double							 fSfx;								//!to scale the x such that it is realy mm
//      double							 fSfy;								//!to scale the y such that it is realy mm
//      double							 fT0;								//!to correct the time of flight
//
//      double							 fMass_au;							//!the Mass of this Particle in a.u.
//      double							 fCharge_au;						//!the Charge of this Particle in a.u.
//      TString							 fName;								//!how is this particle called
    };
  }
}
#endif
