//Copyright (C) 2010 Lutz Foucar

/**
 * @file delayline_detector.h file contains the classes that describe a
 *                            delayline detector.
 *
 * @author Lutz Foucar
 */

#ifndef _DELAYLINE_DETECTOR_H_
#define _DELAYLINE_DETECTOR_H_

#include <vector>
#include <algorithm>
#include <stdexcept>

#include "cass_acqiris.h"
#include "tof_detector.h"
#include "waveform_signal.h"
#include "cass_settings.h"



namespace cass
{
  namespace ACQIRIS
  {
    /** A anode layer of the delayline detector.
     *
     * class containing the properties of a anode layer of the detector
     *
     * @cassttng AcqirisDetectors/\%detectorname\%/\%Layername\%/
     *           {LowerTimesumConditionLimit|UpperTimesumConditionLimit}\n
     *           the timesum condition range for the layer.
     * @cassttng AcqirisDetectors/\%detectorname\%/\%Layername\%/{Scalefactor}\n
     *           scalefactor to convert time => mm:
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT AnodeLayer
    {
    public:
      /** default constructor*/
      AnodeLayer()
        :_tsLow(0.),
         _tsHigh(0.),
         _sf(1.)
      {}

      /** map of signals that form the wireends of the layer*/
      typedef std::map<char,Signal> wireends_t;

      /*! load values from cass.ini, should only be called by the detector*/
      void loadSettings(CASSSettings *p,const char * layername);

      /*! save values to cass.ini, should only be called by the detector*/
      void saveParameters(CASSSettings *p,const char * layername);

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






    /** a hit on the delayline detector.
     *
     * class containing the properties of a Hit on the delayline detector. A
     * hit on a Delaylinedetector consists of  x, y and t values. Where x and
     * y are the position on the detector and t is the time the particle hit
     * the detector. All these values are stored in a map and can be extracted
     * using the appropriate name ('x','y','t').
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT DelaylineDetectorHit
    {
    public:
      /** constructor.
       * @param x,y the position on the detector where the particle hit the detector
       * @param t the time when the particle hit the detector
       */
      DelaylineDetectorHit(double x, double y, double t)
          :_x_mm(x), _y_mm(y), _time(t)
      {
        _values['x']=x;
        _values['y']=y;
        _values['t']=t;
      }

      /** default constructor, initalizing every value to 0*/
      DelaylineDetectorHit()
        :_x_mm(0), _y_mm(0), _time(0)
      {}

    public:
      //@{
      /** getter.
       * use this function to retrieve the properties of a hit.
       */
      double  x()const  {return _x_mm;}
      double  y()const  {return _y_mm;}
      double  t()const  {return _time;}
      //@}
      //@{
      /** setter.
       * use this function to set the properties of a hit.
       * @todo check if we still need to set a detector hits
       *       properties after creating it.
       */
      double &x()       {return _x_mm;}
      double &y()       {return _y_mm;}
      double &t()       {return _time;}
      //@}

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








    /** A delayline detector.
     *
     * A delayline detector is a tof detector with the ability to also have
     * position information. It can be either a Hex or Quad delayline detector.
     * It contains all information that is needed in order to sort the signals
     * in the waveforms to detector hits.
     *
     * @cassttng AcqirisDetectors/\%detectorname\%/{Runtime}\n
     *           maximum time a signal will run over the complete delayline.
     * @cassttng AcqirisDetectors/\%detectorname\%/{Angle}\n
     *           Angle in degree by which on can rotate the picture around 0,0.
     *           Default is 0.
     * @cassttng AcqirisDetectors/\%detectorname\%/{McpRadius}\n
     *           Radius of the MCP in mm.
     * @cassttng AcqirisDetectors/\%detectorname\%/{AnalysisMethod}\n
     *           Method that is used to reconstruct the detector hits, choises are:
     *           - 0: Simple Analysis
     * @cassttng AcqirisDetectors/\%detectorname\%/{LayersToUse}\n
     *           Layers that should be used (when using the simple reconstruction method).
     *           - if HexAnode:
     *             - 0: Layers U and V
     *             - 1: Layers U and W
     *             - 2: Layers V and W
     *           - if QuadAnode (only one option available):
     *             - 0: Layers X and Y
     * @cassttng AcqirisDetectors/\%detectorname\%/{DeadTimeMcp}\n
     *           Dead time when detecting MCP Signals (used for future more advanced
     *           reconstruction methods)
     * @cassttng AcqirisDetectors/\%detectorname\%/{DeadTimeAnode}\n
     *           Dead time when detecting anode layer Signals (used for future more
     *           advanced reconstruction methods)
     * @cassttng AcqirisDetectors/\%detectorname\%/{WLayerOffset}\n
     *           The W-Layer offset with respect to layers U and V (used for future more
     *           advanced Hex-Detector reconstruction methods)
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT DelaylineDetector : public TofDetector
    {
    public:
      /** constructor.
       * @param[in] type the delayline type is an enum either Hex or Quad
       * @param[in] name the name of this detector
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
      {_analyzerType =DelaylineSimple;}

      /** overwritten assignment operator.
       * this will overwrite the virtual assignment operator of the detectorbackend.
       * we need to do this to be able to easily copy the info of this detector in
       * the acqiris helper @see cass::HelperAcqirisDetectors::validate
       * @return a reference to this
       * @param rhs the right hand side of the operator
       */
      DetectorBackend& operator= (const DetectorBackend& rhs);

    public:
      /** load the values from cass.ini*/
      virtual void loadSettings(CASSSettings *p);

      /** save values to cass.ini */
      virtual void saveParameters(CASSSettings *p);

    public:
      /** a vector of detector hits are the detector hits */
      typedef std::vector<DelaylineDetectorHit> dethits_t;

      /** a map of anodelayers */
      typedef std::map<char,AnodeLayer> anodelayers_t;

    public:
      /** @returns the timesum of the first good hit for a given layer*/
      double timesum(char layer)
      {
//        std::cout<< "del calc "<<layer<<" tsum: "<< _mcp.firstGood()<<std::endl;
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

    public:
      //@{
      /** setter */
      std::string   &name()           {return _name;}
      double        &angle()          {return _angle;}
      double        &runtime()        {return _runtime;}
      double        &wLayerOffset()   {return _wLayerOffset;}
      double        &mcpRadius()      {return _mcpRadius;}
      double        &deadTimeAnode()  {return _deadAnode;}
      double        &deadTimeMCP()    {return _deadMcp;}
      anodelayers_t &layers()         {return _anodelayers;}
      Signal        &mcp()            {return _mcp;}
      DelaylineType &delaylineType()  {return _delaylinetype;}
      LayersToUse   &layersToUse()    {return _layersToUse;}
      //@}
      //@{
      /** getter */
      const std::string   &name()const            {return _name;}
      double               angle()const           {return _angle;}
      double               runtime()const         {return _runtime;}
      double               wLayerOffset()const    {return _wLayerOffset;}
      double               mcpRadius()const       {return _mcpRadius;}
      double               deadTimeAnode()const   {return _deadAnode;}
      double               deadTimeMCP()const     {return _deadMcp;}
      const anodelayers_t &layers()const          {return _anodelayers;}
      const Signal        &mcp()const             {return _mcp;}
      DelaylineType        delaylineType()const   {return _delaylinetype;}
      LayersToUse          layersToUse()const     {return _layersToUse;}
      //@}

    private:
      /** the runtime of a signal over the anode */
      double _runtime;

      /** the angle around which the x and y of detector will be rotated */
      double _angle;

      /** the offset of w-layer towards u and v-layer, only used for hex detectors*/
      double _wLayerOffset;

      /** the radius of the MCP in mm*/
      double _mcpRadius;

      /** the Deadtime between to Signals on the MCP*/
      double _deadMcp;

      /** the deadtime between to Signals on the Layers */
      double _deadAnode;

      /** layer combination.
       * enum telling which Layers should be used to calculate the position when
       * using simple sorting
       */
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
void cass::ACQIRIS::AnodeLayer::loadSettings(CASSSettings *p,const char * layername)
{
  VERBOSEOUT(std::cerr <<"Anode Layer load parameters: loading  for layer \""<<layername<<"\""
      <<" of detector " << p->group().toStdString()<<std::endl);
  p->beginGroup(layername);
  _tsLow  = p->value("LowerTimesumConditionLimit",0.).toDouble();
  _tsHigh = p->value("UpperTimesumConditionLimit",20000.).toDouble();
  _sf     = p->value("Scalefactor",0.5).toDouble();
  _wireend['1'].loadSettings(p,"One");
  _wireend['2'].loadSettings(p,"Two");
  p->endGroup();
  VERBOSEOUT(std::cout <<"Anode Layer load parameters: done loading"<<std::endl);
}
inline
void cass::ACQIRIS::AnodeLayer::saveParameters(CASSSettings *p,const char * layername)
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
void cass::ACQIRIS::DelaylineDetector::loadSettings(CASSSettings *p)
{
  VERBOSEOUT(std::cout<< "Delayline Detector load parameters: loading "<<_name
      <<"'s parameters. It is a "
      <<((_delaylinetype==Hex)?"Hex-":"Quad-")<<"Detector"
      <<std::endl);
  //load the parameters for this detector//
  p->beginGroup(_name.c_str());
  _runtime      = p->value("Runtime",150).toDouble();
  _mcpRadius    = p->value("McpRadius",44.).toDouble();
  _angle        = p->value("Angle",0.).toDouble()*3.1415/180.;
  _mcp.loadSettings(p,"MCP");
  _analyzerType =
      static_cast<DetectorAnalyzers>(p->value("AnalysisMethod",DelaylineSimple).toInt());
  VERBOSEOUT(std::cout <<"Delayline Detector load parameters: loaded analyzer type:"<<_analyzerType
      <<" should be "<<DelaylineSimple
      <<std::endl);
  //load parameters depending on which analyzer you use to analyze this detector//
  switch(_analyzerType)
  {
  case DelaylineSimple :
    VERBOSEOUT(std::cout << "Delayline Detector load parameters: "
        <<"we use Delayline Simple to analyze us"
        <<std::endl);
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
    _anodelayers['U'].loadSettings(p,"ULayer");
    _anodelayers['V'].loadSettings(p,"VLayer");
    _anodelayers['W'].loadSettings(p,"WLayer");
    break;
  case Quad:
    _anodelayers['X'].loadSettings(p,"XLayer");
    _anodelayers['Y'].loadSettings(p,"YLayer");
    break;
  default:
    throw std::invalid_argument("delayline type does not exist");
    break;
  }
  p->endGroup();
  VERBOSEOUT(std::cout << "Delayline Detector load parameters: done loading "<<_name
             << "'s parameters"
             <<std::endl);
}
inline
void cass::ACQIRIS::DelaylineDetector::saveParameters(CASSSettings *p)
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

inline
cass::ACQIRIS::DetectorBackend& cass::ACQIRIS::DelaylineDetector::operator =(const cass::ACQIRIS::DetectorBackend& righthandside)
{
  //if we are not self assigning//
  if (this != &righthandside)
  {
//    std::cout << "copy is using delayline's copying"<<std::endl;
    //for easier writing get the right reference of the right hand side//
    const DelaylineDetector &rhs = dynamic_cast<const DelaylineDetector&>(righthandside);
    //copy all members from the right hand side to us//
    _runtime        = rhs._runtime;
    _wLayerOffset   = rhs._wLayerOffset;
    _mcpRadius      = rhs._mcpRadius;
    _deadMcp        = rhs._deadMcp;
    _deadAnode      = rhs._deadAnode;
    _layersToUse    = rhs._layersToUse;
    _delaylinetype  = rhs._delaylinetype;
    _anodelayers    = rhs._anodelayers;
    _hits           = rhs._hits;
    //tof detectors stuff//
    _mcp            = rhs._mcp;
    //backend's stuff//
    _analyzerType   = rhs._analyzerType;
    _name           = rhs._name;
  }
  return *this;
}

#endif





