//Copyright (C) 2010 lmf

#ifndef _DELAYLINE_DETECTOR_H_
#define _DELAYLINE_DETECTOR_H_

#include <vector>
#include "cass_acqiris.h"
#include "detector_backend.h"
#include "waveform_signal.h"



namespace cass
{
  namespace ACQIRIS
  {
    //----------------------------------------------------------------------------------
    class CASS_ACQIRISSHARED_EXPORT AnodeLayer    //the properties of one layer of the delayline-anode//
    {
    public:
      AnodeLayer()
        :_tsLow(0.),
         _tsHeigh(0.),
         _sf(1.)
      {}
      ~AnodeLayer() {}

    public:
      void loadParameters(QSettings *p,const char * layername)
      {
        //std::cerr <<"loading settings for layer \""<<layername<<"\""<<std::endl;
        p->beginGroup(layername);
        _tsLow   = p->value("LowerTimeSumLimit",0.).toDouble();
        _tsHeigh  = p->value("UpperTimeSumLimit",20000.).toDouble();
        _sf      = p->value("Scalefactor",0.5).toDouble();
        _one.loadParameters(p,"One");
        _two.loadParameters(p,"Two");
        p->endGroup();
        //std::cout <<"done"<<std::endl;
      }
      void saveParameters(QSettings *p,const char * layername)
      {
        p->beginGroup(layername);
        p->setValue("LowerTimeSumLimit",_tsLow);
        p->setValue("UpperTimeSumLimit",_tsHeigh);
        p->setValue("Scalefactor",_sf);
        _one.saveParameters(p,"One");
        _two.saveParameters(p,"Two");
        p->endGroup();
      }

    public:
      //the timesum condition for this anode layer
      double ts()const      {return 0.5*(_tsLow+_tsHeigh);}
      //the timesum of the first good hit of this layer//
      double timesum()const {return _one.firstGood() + _two.firstGood();}
      //the position of the first good hit//
      double position()const{return _one.firstGood() - _two.firstGood();}

    public: //setters/getters
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
    class CASS_ACQIRISSHARED_EXPORT DelaylineDetectorHit
    {
    public:
      DelaylineDetectorHit(double x, double y, double t)
          :_x_mm(x), _y_mm(y), _time(t)        {}
      DelaylineDetectorHit()
        :_x_mm(0), _y_mm(0), _time(0)          {}
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
      DelaylineDetector()
        :DetectorBackend(Delayline),
         _name("Sucker"),
         _runtime(0),
         _wLayerOffset(100),
         _mcpRadius(0),
         _deadMcp(1000),
         _deadAnode(1000),
         _layersToUse(UV),
         _delaylinetype(Hex)
      {_analyzerType=DelaylineSimple;}
      ~DelaylineDetector() {}

    public:
      virtual void loadParameters(QSettings *p)
      {
        //std::cout <<"loading"<<std::endl;
        //load the parameters for this detector//
        _name         = p->value("Name","IonDetector").toString().toStdString();
        _runtime      = p->value("Runtime",150).toDouble();
        _mcpRadius    = p->value("McpRadius",44.).toDouble();
        _mcp.loadParameters(p,"MCP");
        _analyzerType = static_cast<DetectorAnalyzers>(p->value("AnalysisMethod",DelaylineSimple).toInt());
        //std::cout <<"loaded analyzer type:"<<_analyzerType<<" should be "<<DelaylineSimple<<std::endl;
        //load parameters depending on which analyzer you use to analyze this detector//
        switch(_analyzerType)
        {
        case DelaylineSimple :
          _layersToUse  = static_cast<LayersToUse>(p->value("LayersToUse",UV).toInt());
          break;
        default:
          _deadMcp      = p->value("DeadTimeMcp",10.).toDouble();
          _deadAnode    = p->value("DeadTimeAnode",10.).toDouble();
          _wLayerOffset = p->value("WLayerOffset",0.).toDouble();
          break;
        }
        _delaylinetype   = static_cast<DelaylineType>(p->value("DelaylineType",Quad).toInt());
        //add and load layers according the the delaylinedtype//
        switch (_delaylinetype)
        {
        case Hex:
          _anodelayers.resize(3,AnodeLayer());
          _anodelayers[U].loadParameters(p,"ULayer");
          _anodelayers[V].loadParameters(p,"VLayer");
          _anodelayers[W].loadParameters(p,"WLayer");
          break;
        case Quad:
          _anodelayers.resize(2,AnodeLayer());
          _anodelayers[U].loadParameters(p,"XLayer");
          _anodelayers[V].loadParameters(p,"YLayer");
          break;
        default:
          break;
        }
      }
      virtual void saveParameters(QSettings *p)
      {
        //save the parameters//
        p->setValue("Name",_name.c_str());
        p->setValue("Runtime",_runtime);
        p->setValue("McpRadius",_mcpRadius);
        _mcp.saveParameters(p,"MCP");
        //save according to the method//
        p->setValue("AnalysisMethod",static_cast<int>(_analyzerType));
        switch(_analyzerType)
        {
        case DelaylineSimple :
          p->setValue("LayersToUse",static_cast<int>(_layersToUse));
          break;
        default:
          p->setValue("DeadTimeMcp",_deadMcp);
          p->setValue("DeadTimeAnode",_deadAnode);
          p->setValue("WLayerOffset",_wLayerOffset);
          break;
        }
        //save according to the delayline type//
        p->setValue("DelaylineType",static_cast<int>(_delaylinetype));
        switch (_delaylinetype)
        {
        case Hex:
          _anodelayers[U].saveParameters(p,"ULayer");
          _anodelayers[V].saveParameters(p,"VLayer");
          _anodelayers[W].saveParameters(p,"WLayer");
          break;
        case Quad:
          _anodelayers[U].saveParameters(p,"XLayer");
          _anodelayers[V].saveParameters(p,"YLayer");
          break;
        default:
          break;
        }
      }

    public:
      typedef std::vector<DelaylineDetectorHit> dethits_t;
      typedef std::vector<AnodeLayer> anodelayers_t;

    public:
      //retrieve the timesum of the first good hit for a given layer//
      double timesum(LayerTypes layer) const
      {
        return _anodelayers[layer].timesum() - 2.* _mcp.firstGood();
      }

      //retrieve whether the first "good" hit fullfilles the timesum condition//
      bool timesumcondtion(LayerTypes layer) const
      {
        return (_anodelayers[layer].tsLow() < timesum(layer) && 
                timesum(layer) < _anodelayers[layer].tsHigh());
      }

      //retrieve the position of the first good hit for a given layer//
      double position(LayerTypes layer) const
      {
        return _anodelayers[layer].position();
      }

    public: //setter/getter
      const dethits_t     &hits()const            {return _hits;}
      dethits_t           &hits()                 {return _hits;}

    public: //setter/getter
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
      DelaylineType        delaylineType()const   {return _delaylinetype;}
      DelaylineType       &delaylineType()        {return _delaylinetype;}
      LayersToUse          layersToUse()const     {return _layersToUse;}
      LayersToUse         &layersToUse()          {return _layersToUse;}

    private:
      //variables that the sorters need to find detectorhits, given by the user
      std::string          _name;                 //the name of this detector
      double               _runtime;              //the runtime over the anode
      double               _wLayerOffset;         //the offset of w-layer towards u and v-layer
      double               _mcpRadius;            //the radius of the MCP in mm
      Signal               _mcp;                  //properties of MCP Signals for this detektor
      double               _deadMcp;              //the Deadtime between to Signals on the MCP
      double               _deadAnode;            //the Deadtime between to Signals on the Layers
      LayersToUse          _layersToUse;          //enum telling which Layers should be used to calc the position when using simple sorting
      DelaylineType        _delaylinetype;        //type of the delayline (hex or quad)
      anodelayers_t        _anodelayers;          //properties of layers

      //output of the analysis (sorting) of the peaks
      dethits_t            _hits;                 //Container storing the refrences to the DetektorHits of this Detektor
    };

  }//end namespace remi
}//end namespace cass
#endif





