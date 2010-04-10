//Copyright (C) 2010 Lutz Foucar

#ifndef _DELAYLINE_DETECTOR_H_
#define _DELAYLINE_DETECTOR_H_

#include <vector>
#include <algorithm>

#include "cass_acqiris.h"
#include "tof_detector.h"
#include "waveform_signal.h"



namespace cass
{
  namespace ACQIRIS
  {
    /*! The Anode Layer of the delayline detector.
      class containing the properties of a
      anode layer of the detector
      @author Lutz Foucar
    */
    class CASS_ACQIRISSHARED_EXPORT AnodeLayer
    {
    public:
      /*! default constructor*/
      AnodeLayer()
        :_tsLow(0.),
         _tsHigh(0.),
         _sf(1.)
      {}

    public:
      /** map of signals that form the wireends of the layer*/
      typedef std::map<char,Signal> wireends_t;
    public:
      /*! load values from cass.ini, should only be called by the detector*/
      void loadParameters(QSettings *p,const char * layername);
      /*! save values to cass.ini, should only be called by the detector*/
      void saveParameters(QSettings *p,const char * layername);
    public:
      /*! returns the timesum condition for this anode layer*/
      double ts()const      {return 0.5*(_tsLow+_tsHigh);}
      /*! returns the timesum of the first good hit of this layer*/
      double timesum() {return _wireend['1'].firstGood() + _wireend['2'].firstGood();}
      /*! returns the position of the first good hit*/
      double position() {return _wireend['1'].firstGood() + _wireend['2'].firstGood();}

    public:
      /*! setters/getters */
      double             tsLow()const   {return _tsLow;}
      double            &tsLow()        {return _tsLow;}
      double             tsHigh()const  {return _tsHigh;}
      double            &tsHigh()       {return _tsHigh;}
      double             sf()const      {return _sf;}
      double            &sf()           {return _sf;}
      const wireends_t  &wireend()const {return _wireend;}
      wireends_t        &wireend()      {return _wireend;}

    private:
      /*! lower edge of the timesum condition*/
      double  _tsLow;
      /*! upper edge of the timesum condition*/
      double  _tsHigh;
      /*! scalefactor which converts ns -> mm */
      double  _sf;
      /*! the properties of the wireends, they are singals */
      wireends_t  _wireend;
    };






    /*! Detector Hits.
      class containing the properties of a Hit on the
      delayline detector. A Hit on a Delaylinedetector consists
      of a x, y and t value. Where x and y are the position on
      the detector and t is the time the particle hit the detector.
      All these values are stored in a map and can be extracted using
      the appropriate name ('x','y','t').
      @author Lutz Foucar
    */
    class CASS_ACQIRISSHARED_EXPORT DelaylineDetectorHit
    {
    public:
      /*! initiailzing constructor*/
      DelaylineDetectorHit(double x, double y, double t)
          :_x_mm(x), _y_mm(y), _time(t)
      {
        _values['x']=x;
        _values['y']=y;
        _values['t']=t;
      }
      /*! default constructor, initalizing every value to 0*/
      DelaylineDetectorHit()
        :_x_mm(0), _y_mm(0), _time(0)
      {}

    public: /*! getters & setters*/
      double  x()const  {return _x_mm;}
      double &x()       {return _x_mm;}
      double  y()const  {return _y_mm;}
      double &y()       {return _y_mm;}
      double  t()const  {return _time;}
      double &t()       {return _time;}
      /** get the values of a hit*/
      std::map<char,double> &values() {return _values;}


    private:
      /*! the x component of the detector in mm*/
      double  _x_mm;
      /*! the y component of the detector in mm*/
      double  _y_mm;
      /*! the mcp time of this hit on the detector*/
      double  _time;
      /** a map containing the three coordiantes of the hit*/
      std::map<char,double> _values;
    };








    /*! The delayline detector

    A delayline detector is a tof detector with the ability to also
    have position information.
    class that can be a Hex or Quad delayline detector. it contains
    all information that is needed in order to sort the signals in the
    waveforms to detector hits

    @author Lutz Foucar
    */
    class CASS_ACQIRISSHARED_EXPORT DelaylineDetector : public TofDetector
    {
    public:
      /** constructor
      @param[in] type the delayline type
      @param[in] name the name of this detector
      */
      DelaylineDetector(DelaylineType type, const std::string name)
        :TofDetector(name),
         _runtime(0),
         _wLayerOffset(100),
         _mcpRadius(0),
         _deadMcp(1000),
         _deadAnode(1000),
         _layersToUse(UV),
         _delaylinetype(type)
      { _analyzerType =DelaylineSimple;}

    public:
      /** load the values from cass.ini
      @todo make sure we create a unificaly group for each detector (maybe using a name?)
      */
      virtual void loadParameters(QSettings *p);
      /** save values to cass.ini */
      virtual void saveParameters(QSettings *p);

    public:
      /** a vector of detector hits are the detector hits */
      typedef std::vector<DelaylineDetectorHit> dethits_t;
      /** a map of anodelayers */
      typedef std::map<char,AnodeLayer> anodelayers_t;

    public:
      /** @returns the timesum of the first good hit for a given layer*/
      double timesum(char layer)
      {
        return _anodelayers[layer].timesum() - 2.* _mcp.firstGood();
      }

      /** @returns whether the first "good" hit fullfilles the timesum condition*/
      bool timesumcondtion(char layer)
      {
        return (_anodelayers[layer].tsLow() < timesum(layer) && 
                timesum(layer) < _anodelayers[layer].tsHigh());
      }

      /** @returns the position of the first good hit for a given layer*/
      double position(char layer) 
      {
        return _anodelayers[layer].position();
      }

    public:
      /** @return the found detector hits*/
      const dethits_t     &hits()const            {return _hits;}
      /** @overload hits()const*/
      dethits_t           &hits()                 {return _hits;}

    public: /** setters & getters */
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
      /** the runtime of a signal over the anode */
      double _runtime;
      /** the offset of w-layer towards u and v-layer, only used for hex detectors*/
      double _wLayerOffset;
      /** the radius of the MCP in mm*/
      double _mcpRadius;
      /** the Deadtime between to Signals on the MCP*/
      double _deadMcp;
      /** the deadtime between to Signals on the Layers */
      double _deadAnode;
      /** enum telling which Layers should be used to calculate the position when using simple sorting*/
      LayersToUse _layersToUse;
      /** type of the delayline (hex or quad)*/
      DelaylineType _delaylinetype;
      /** properties of layers*/
      anodelayers_t _anodelayers;

      /** constainer for all reconstructed detector hits*/
      dethits_t _hits;
    };

  }//end namespace remi
}//end namespace cass



//----function definition-------
//-----------Anode Layer--------
inline
void cass::ACQIRIS::AnodeLayer::loadParameters(QSettings *p,const char * layername)
{
  //std::cerr <<"loading settings for layer \""<<layername<<"\""<<std::endl;
  p->beginGroup(layername);
  _tsLow  = p->value("LowerTimesumConditionLimit",0.).toDouble();
  _tsHigh = p->value("UpperTimesumConditionLimit",20000.).toDouble();
  _sf     = p->value("Scalefactor",0.5).toDouble();
  _wireend['1'].loadParameters(p,"One");
  _wireend['2'].loadParameters(p,"Two");
  p->endGroup();
  //std::cout <<"done"<<std::endl;
}
inline
void cass::ACQIRIS::AnodeLayer::saveParameters(QSettings *p,const char * layername)
{
  p->beginGroup(layername);
  p->setValue("LowerTimesumConditionLimit",_tsLow);
  p->setValue("UpperTimesumConditionLimit",_tsHigh);
  p->setValue("Scalefactor",_sf);
  _wireend['1'].saveParameters(p,"One");
  _wireend['2'].saveParameters(p,"Two");
  p->endGroup();
}


//-----------Detector--------
inline
void cass::ACQIRIS::DelaylineDetector::loadParameters(QSettings *p)
{
  //std::cout <<"loading"<<std::endl;
  //load the parameters for this detector//
  p->beginGroup(_name.c_str());
  _runtime      = p->value("Runtime",150).toDouble();
  _mcpRadius    = p->value("McpRadius",44.).toDouble();
  _mcp.loadParameters(p,"MCP");
  _analyzerType =
      static_cast<DetectorAnalyzers>(p->value("AnalysisMethod",DelaylineSimple).toInt());
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
  //add and load layers according the the delayline type//
  switch (_delaylinetype)
  {
  case Hex:
    _anodelayers['U'].loadParameters(p,"ULayer");
    _anodelayers['V'].loadParameters(p,"VLayer");
    _anodelayers['W'].loadParameters(p,"WLayer");
    break;
  case Quad:
    _anodelayers['X'].loadParameters(p,"XLayer");
    _anodelayers['Y'].loadParameters(p,"YLayer");
    break;
  default:
    break;
  }
  p->endGroup();
}
inline
void cass::ACQIRIS::DelaylineDetector::saveParameters(QSettings *p)
{
  p->beginGroup(_name.c_str());
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
  switch (_delaylinetype)
  {
  case Hex:
    _anodelayers['U'].saveParameters(p,"ULayer");
    _anodelayers['V'].saveParameters(p,"VLayer");
    _anodelayers['W'].saveParameters(p,"WLayer");
    break;
  case Quad:
    _anodelayers['X'].saveParameters(p,"XLayer");
    _anodelayers['Y'].saveParameters(p,"YLayer");
    break;
  default:
    break;
  }
  p->endGroup();
}

#endif





