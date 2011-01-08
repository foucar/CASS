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
#include "signal_producer.h"
#include "map.hpp"

namespace cass
{
  class CASSSettings;

  namespace ACQIRIS
  {
    //forward declarations
    class DetectorAnalyzerBackend;

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
      typedef std::map<char,SignalProducer> wireends_t;

      /** load values from cass.ini, should only be called by the detector*/
      void loadSettings(CASSSettings&);

      /** associate the event with this anodelayers signal producers */
      void associate(const CASSEvent&);

      /** returns the timesum condition for this anode layer*/
      double ts()const      {return 0.5*(_tsLow+_tsHigh);}

      /** returns the timesum of the first good hit of this layer*/
      double timesum() {return _wireend['1'].firstGood() + _wireend['2'].firstGood();}

      /** returns the position of the first good hit*/
      double position(){return _wireend['1'].firstGood() - _wireend['2'].firstGood();}

    public:
      //@{
      /** setter */
      double            &tsLow()        {return _tsLow;}
      double            &tsHigh()       {return _tsHigh;}
      double            &sf()           {return _sf;}
      wireends_t        &wireend()      {return _wireend;}
      //@}
      //@{
      /** getter */
      double             tsLow()const   {return _tsLow;}
      double             tsHigh()const  {return _tsHigh;}
      double             sf()const      {return _sf;}
      const wireends_t  &wireend()const {return _wireend;}
      //@}

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
      {
        _values['x']=x;
        _values['y']=y;
        _values['t']=t;
      }

      /** default constructor*/
      DelaylineDetectorHit() {}

      /** get the values of a hit*/
      std::map<char,double> &values() {return _values;}

    private:
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
       *
       * @param[in] type the delayline type is an enum either Hex or Quad
       * @param[in] name the name of this detector
       */
      DelaylineDetector(const std::string name)
        :TofDetector(name), _analyzer(0), _newEventAssociated(false)
      {}

    public:
      /** load the values from cass.ini*/
      virtual void loadSettings(CASSSettings&);

      /** associate the event with all of this detectors signal producers */
      void associate (const CASSEvent&);


    public:
      typedef Map<std::string,double> hit_t;
      typedef std::vector<hit_t> hits_t;

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
      /** return the list of detector hits */
      hits_t &hits();

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
      SignalProducer&mcp()            {return _mcp;}
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
      const SignalProducer&mcp()const             {return _mcp;}
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
      hits_t _hits;

      /** the analyzer that will sort the signal to hits */
      DetectorAnalyzerBackend * _analyzer;

      /** flag to show whether there is a new event associated whith this */
      bool _newEventAssociated;

    };

  }//end namespace remi
}//end namespace cass


#endif





