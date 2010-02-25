//Copyright (C) 2010 lmf

#ifndef _DELAYLINE_DETECTOR_H_
#define _DELAYLINE_DETECTOR_H_

#include <vector>
#include "cass_acqiris.h"
#include "detector_backend.h"
#include "peak.h"


namespace cass
{
  namespace ACQIRIS
  {
    //----------------------------------------------------------------------------------
    class Signal    //the properties of one Wire-end of the Layerwire
    {
    public:
      Signal()    {}
      ~Signal()   {}

    public:
      typedef std::vector<Peak> peaks_t;

    public:
      peaks_t        &peaks()               {return _peaks;}
      const peaks_t  &peaks()const          {return _peaks;}

    public:
      size_t           channelNbr()const    {return _chNbr;}
      size_t          &channelNbr()         {return _chNbr;}
      double           trLow()const         {return _trLow;}
      double          &trLow()              {return _trLow;}
      double           trHigh()const        {return _trHigh;}
      double          &trHigh()             {return _trHigh;}
      Polarity         polarity()const      {return -polarity;}
      Polarity        &polarity()           {return _polarity;}
      int16_t          threshold()const     {return _threshold;}
      int16_t         &threshold()          {return _threshold;}
      int32_t          delay()const         {return _delay;}
      int32_t         &delay()              {return _delay;}
      double           fraction()const      {return _fraction;}
      double          &fraction()           {return _fraction;}
      double           walk()const          {return _walk;}
      double          &walk()               {return _walk;}
      WaveformAnalyzers  analyzerType()const  {return _analyzerType;}
      WaveformAnalyzers &analyzerType()       {return _analyzerType;}

    private:
      //things important to know how to analyze the waveform//
      //set by the user via parameters//
      double          _trLow;         //lower edge of the timerange events can happen in
      double          _trHigh;        //upper edge of the timerange events can happen in
      size_t          _chNbr;         //This Channels Number in the Acqiris Crate
      int16_t         _threshold;     //the Noiselevel for this channel (in adc bytes)
      int32_t         _delay;         //the delay of the cfd
      double          _fraction;      //the fraction of the cfd
      double          _walk;          //the walk of the cfd
      Peak::Polarity  _polarity;      //the Polarity the Signal has
      WaveformAnalyzers _analyzerType;//type of analysis to analyze this channel
      //container for the results of the waveform analysis
      peaks_t         _peaks;         //container for the peaks of the waveform
    };






    //----------------------------------------------------------------------------------
    class AnodeLayer    //the properties of one layer of the delayline-anode//
    {
    public:
      AnodeLayer()  {}
      ~AnodeLayer() {}

    public:
      double         ts()const      {return 0.5*(_tsLow+_tsHeigh);}

    public:
      double         tsLow()const   {return _tsLow;}
      double        &tsLow()        {return _tsLow;}
      double         tsHigh()const  {return _tsHeigh;}
      double        &tsHigh()       {return _tsHeigh;}
      double         sf()const      {return _sf;}
      double        &sf()           {return _sf;}
      const Signal  &one()const     {return _one;}
      Signal        &one()          {return _one;}
      const Signal  &two()const     {return _two;}
      Signal        &two()          {return _two;}

    private:
      double  _tsLow;   //lower edge of the timesum
      double  _tsHeigh; //upper edge of the timesum
      double  _sf;      //scalefactor for layer
      Signal  _one;     //the properties of one end of the wire
      Signal  _two;     //the properties of the other end of the wire
    };






    //----------------------------------------------------------------------------------
    class DelaylineDetectorHit
    {
    public:
      DelaylineDetectorHit(double x, double y, double t)
          :_x_mm(x), _y_mm(y), _time(t)        {}
      DelaylineDetectorHit()  {}
      ~DelaylineDetectorHit() {}

    public:
      double  x()const  {return _x_mm;}
      double &x()       {return _x_mm;}
      double  y()const  {return _y_mm;}
      double &y()       {return _y_mm;}
      double  t()const  {return _time;}
      double &t()       {return _time;}

    private:

      double  _x_mm;    //the x component of the detector in mm
      double  _y_mm;    //the y component of the detector in mm
      double  _time;    //the mcp time of this hit on the detector
    };








    //----------------------------------------------------------------------------------
    class CASS_ACQIRISSHARED_EXPORT DelaylineDetector : public DetectorBackend
    {
    public:
      DelaylineDetector()  {}
      ~DelaylineDetector() {}

    public:
      typedef std::vector<DetectorHit> dethits_t;
      typedef std::vector<AnodeLayer> anodelayers_t;

    public:
      const dethits_t     &hits()const            {return _hits;}
      dethits_t           &hits()                 {return _hits;}

    public:
      const std::string   &name()const            {return _name;}
      std::string         &name()                 {return _name;}
      double               runtime()const         {return _runtime;}
      double              &runtime()              {return _runtime;}
      double               wLayerOffset()const    {return _wLayerOffset;}
      double              &wLayerOffset()         {return _wLayerOffset;}
      double               mcpRadius()const       {return _mcpRadius;}
      double              &mcpRadius()            {return _mcpRadius;}
      double               deadTimeAnode()const   {return _deadAnode;}
      double              &deadTimeAnode()        {return _deadAnode;}
      double               deadTimeMCP()const     {return _deadMcp;}
      double              &deadTimeMCP()          {return _deadMcp;}
      const anodelayers_t &layers()const          {return _anodelayers;}
      anodelayers_t       &layers()               {return _anodelayers;}
      const Signal        &mcp()const             {return _mcp;}
      Signal              &mcp()                  {return _mcp;}
      bool                 isHexAnode()const      {return _isHex;}
      bool                &isHexAnode()           {return _isHex;}
      LayersToUse          LayersToUse()const     {return _layersToUse;}
      LayersToUse         &LayersToUse()          {return _layersToUse;}

    private:
      //variables that the sorters need to find detectorhits, given by the user
      std::string          _name;                 //the name of this detector
      anodelayers_t        _anodelayers;          //properties of layers
      Signal               _mcp;                  //properties of MCP Signals for this detektor
      double               _runtime;              //the runtime over the anode
      double               _wLayerOffset;         //the offset of w-layer towards u and v-layer
      double               _mcpRadius;            //the radius of the MCP in mm
      double               _deadMcp;              //the Deadtime between to Signals on the MCP
      double               _deadAnode;            //the Deadtime between to Signals on the Layers
      bool                 _isHex;                //flag telling wether this is a Hexanode Detektor
      LayersToUse         _layersToUse;          //enum telling which Layers should be used to calc the position when using simple sorting

      //output of the analysis (sorting) of the peaks
      dethits_t            _hits;                 //Container storing the refrences to the DetektorHits of this Detektor
    };

  }//end namespace remi
}//end namespace cass
#endif





