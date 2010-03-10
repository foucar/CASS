//Copyright (C) 2010 lmf

#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <vector>
#include <iostream>

#include "serializer.h"

namespace cass
{

  //this describes the properties of the axis of the histogram
  class CASSSHARED_EXPORT AxisProperty
  {
  public:
    AxisProperty(size_t nbrBins, float lowerLimit, float upperLimit)
        :_nbrBins(nbrBins),
         _low(lowerLimit),
         _up(upperLimit)
    {}
    AxisProperty(Serializer &in)
    {
      deserialize(in);
    }
    ~AxisProperty() {}

    void serialize(Serializer&)const;
    void deserialize(Serializer&);

    size_t nbrBins()const   {return _nbrBins;}
    float  lowerLimit()const{return _low;}
    float  upperLimit()const{return _up;}

  private:
    size_t    _nbrBins; //the number of bins in this axis
    float     _low;     //lower limit of the axis
    float     _up;      //upper limit of the axis
    uint16_t  _version; //the version for de/serializing
  };

  //histogram backend, every type of histogram inherits from here//
  class CASSSHARED_EXPORT HistogramBackend
  {
  public:
    explicit HistogramBackend(size_t dim)
      :_dimension(dim),
      _nbrOfFills(0),
       _version(1)
    {}
    virtual ~HistogramBackend(){}
    virtual void serialize(Serializer&)const=0;
    virtual void deserialize(Serializer&)=0;
  protected: //setters , getters
    size_t   nbrOfFills()const {return _nbrOfFills;}
    size_t  &nbrOfFills()      {return _nbrOfFills;}
    size_t   dimension()       {return _dimension;}
  protected:
    typedef std::vector<AxisProperty> axis_t;
  protected:
    enum Axis{xAxis=0,yAxis,zAxis}; //choose easier the axis
    enum Quadrant{UpperLeft=0, UpperMiddle, UpperRight, //choose easier the over/
                  Left,                     Right,      //underflow quadrant (2D hist)
                  LowerLeft  , LowerMiddle, LowerRight};
    enum OverUnderFlow{Overflow=0, Underflow};    //the over/underflow bin (1D hist)
  protected:
    size_t    _dimension;
    axis_t    _axis;
    size_t    _nbrOfFills;
    uint16_t  _version;
  };







  //base class for floats from which all float histograms inherit
  class CASSSHARED_EXPORT HistogramFloatBase : public HistogramBackend
  {
  public:
    explicit HistogramFloatBase(size_t dim)
      :HistogramBackend(dim)
    {}
    HistogramFloatBase(Serializer& in)
      :HistogramBackend(0)
    {
      deserialize(in);
    }
    virtual ~HistogramFloatBase()      {}
    virtual void serialize(Serializer&)const;
    virtual void deserialize(Serializer&);
  protected:
    typedef std::vector<float> histo_t;
  protected:
    //reset the histogram//
    virtual void   reset()           {_memory.assign(_memory.size(),0);}
    //setters and getters
    const histo_t &memory()const     {return _memory;}
    histo_t       &memory()          {return _memory;}
  protected:
    //the memory contains the histogram in range nbins
    //after that there are some reservered spaces for over/underflow statistics
    histo_t        _memory;
  };


  //1D Histogram for Graphs, ToF's etc...
  class CASSSHARED_EXPORT Histogram1DFloat : public HistogramFloatBase
  {
  public:
    //constructor for creating a histogram//
    explicit Histogram1DFloat(size_t nbrXBins, float xLow, float xUp)
      :HistogramFloatBase(1)
    {
      //resize the memory, reserve space for the over/underflow bin
      _memory.resize(nbrXBins+2,0);
      //set up the axis
      _axis.push_back(AxisProperty(nbrXBins,xLow,xUp));
    }
    //Constructor for reading a histogram from a stream//
    Histogram1DFloat(Serializer &in)
      :HistogramFloatBase(in)
    {}

    void fill(float x, float weight=1);
  };



  //2D Histogram for Detector Pictures etc...
  class CASSSHARED_EXPORT Histogram2DFloat : public HistogramFloatBase
  {
    //constructor creating histo//
    explicit Histogram2DFloat(size_t nbrXBins, float xLow, float xUp,
                              size_t nbrYBins, float yLow, float yUp)
                                :HistogramFloatBase(2)
    {
      //create memory, reserve space for under/over quadrants
      _memory.resize(nbrXBins*nbrYBins+8,0);
      //set up the two axis of the 2d hist
      _axis.push_back(AxisProperty(nbrXBins,xLow,xUp));
      _axis.push_back(AxisProperty(nbrYBins,yLow,yUp));
    }
    Histogram2DFloat(Serializer &in)
      :HistogramFloatBase(in)
    {}

    void fill(float x, float y, float weight=1);
  };
}//end namespace cass


//--inlined functions---//

//---------------Axis-------------------------------
inline
void cass::AxisProperty::serialize(cass::Serializer &out)const
{
  //the version//
  out.addUint16(_version);

  //number of bins, lower & upper limit
  out.addSizet(_nbrBins);
  out.addFloat(_low);
  out.addFloat(_up);
}
inline
void cass::AxisProperty::deserialize(cass::Serializer &in)
{
  //check whether the version fits//
  uint16_t ver = in.retrieveUint16();
  if(ver!=_version)
  {
    std::cerr<<"version conflict in Axisproperty of Histogram: "<<ver<<" "<<_version<<std::endl;
    return;
  }
  //number of bins, lower & upper limit
  _nbrBins  = in.retrieveSizet();
  _low      = in.retrieveFloat();
  _up       = in.retrieveFloat();
}



//-----------------Base class-----------------------
inline
void cass::HistogramFloatBase::serialize(cass::Serializer &out)const
{
  //the version//
  out.addUint16(_version);
  //the dimension//
  out.addSizet(_dimension);
  //the axis properties//
  for (axis_t::const_iterator it=_axis.begin(); it !=_axis.end();++it)
    it->serialize(out);
  //size of the memory//
  size_t size = _memory.size();
  out.addSizet(size);
  //the memory//
  for (histo_t::const_iterator it=_memory.begin(); it!=_memory.end();++it)
    out.addFloat(*it);
}

inline
void cass::HistogramFloatBase::deserialize(cass::Serializer &in)
{
  //check whether the version fits//
  uint16_t ver = in.retrieveUint16();
  if(ver!=_version)
  {
    std::cerr<<"version conflict in histogram: "<<ver<<" "<<_version<<std::endl;
    return;
  }
  //the dimension//
  _dimension = in.retrieveSizet();
  //initialize axis for all dimensions//
  for (size_t i=0; i<_dimension;++i)
    _axis.push_back(AxisProperty(in));
  //the memory//
  size_t size = in.retrieveSizet();
  _memory.resize(size);
  for (histo_t::iterator it=_memory.begin(); it!=_memory.end();++it)
    *it = in.retrieveFloat();
}


//-----------------1D Hist--------------------------
inline
void cass::Histogram1DFloat::fill(float x, float weight)
{
  //calc the bin//
  const int nxBins    = static_cast<const int>(_axis[xAxis].nbrBins());
  const float xlow    = _axis[xAxis].lowerLimit();
  const float xup     = _axis[xAxis].upperLimit();
  const int xBin = static_cast<int>( nxBins * (x - xlow) / (xup-xlow));

  //check whether the fill is in the right range//
  const bool xInRange = 0<=xBin && xBin<nxBins;
  //if in range fill the memory otherwise figure out//
  //whether over of underflow occured//
  if (xInRange)
    _memory[xBin] += weight;
  else if (xBin >= nxBins)
    _memory[nxBins+Overflow] += 1;
  else if (xBin < 0)
    _memory[nxBins+Underflow] += 1;

  //increase the number of fills//
  ++(_nbrOfFills);
}

//-----------------2D Hist--------------------------
inline
void cass::Histogram2DFloat::fill(float x, float y, float weight)
{
  //calc the xbin//
  const int nxBins    = static_cast<const int>(_axis[xAxis].nbrBins());
  const float xlow    = _axis[xAxis].lowerLimit();
  const float xup     = _axis[xAxis].upperLimit();
  const int xBin = static_cast<int>( nxBins * (x - xlow) / (xup-xlow));

  //calc the ybin//
  const int nyBins    = static_cast<const int>(_axis[yAxis].nbrBins());
  const float ylow    = _axis[yAxis].lowerLimit();
  const float yup     = _axis[yAxis].upperLimit();
  const int yBin = static_cast<int>( nyBins * (y - ylow) / (yup-ylow));

  //fill the memory or the over/underflow quadrant bins//
  const size_t maxSize = nyBins*nxBins;
  const bool xInRange = 0<=xBin && xBin<nxBins;
  const bool yInRange = 0<=yBin && yBin<nyBins;
  //if both are in range fill the memory other wise figure out which qadrant
  //needs to be filled//
  if (xInRange && yInRange)
    _memory[yBin*nxBins + yBin]+= weight;
  if (xBin <0 && yBin <0)
    _memory[maxSize+LowerLeft]+=1;
  else if (xInRange && yBin <0)
    _memory[maxSize+LowerMiddle]+=1;
  else if (xBin >= nxBins && yBin >=nyBins)
    _memory[maxSize+LowerRight]+=1;
  else if (xBin < 0 && yInRange)
    _memory[maxSize+Left]+=1;
  else if (xBin >= nxBins && yInRange)
    _memory[maxSize+Right]+=1;
  else if (xBin < 0 && yBin >= nyBins)
    _memory[maxSize+UpperLeft]+=1;
  else if (xInRange && yBin >= nyBins)
    _memory[maxSize+UpperMiddle]+=1;
  else if (xBin >= nxBins && yBin >= nyBins)
    _memory[maxSize+UpperRight]+=1;

  //increase the number of fills//
  ++(_nbrOfFills);
}


#endif // HISTOGRAM_H
