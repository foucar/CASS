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
    {
    }
    ~AxisProperty() {}

    void serialize(Serializer&);
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

  //base class from which all histograms inherit
  template <typename T>
  class CASSSHARED_EXPORT HistogramBase
  {
  public:
    HistogramBase():_dimension(0),_version(1),_nbrOfFills(0) {}
    virtual ~HistogramBase()      {}

    //reset the histogram//
    virtual void          reset()           {_memory.assign(_memory.size(),0);}
    //setters and getters
    const std::vector<T> &memory()const     {return _memory;}
    std::vector<T>       &memory()          {return _memory;}
    size_t                nbrOfFills()const {return _nbrOfFills;}
    size_t               &nbrOfFills()      {return _nbrOfFills;}
    size_t                dimension()       {return _dimension;}

    virtual void serialize(Serializer&)const;
    virtual void deserialize(Serializer&);

  protected:
    enum Axis{xAxis=0,yAxis,zAxis}; //choose easier the axis
    enum Quadrant{UpperLeft=0, UpperMiddle, UpperRight, //choose easier the over/
                  Left,                     Right,      //underflow quadrant (2D hist)
                  LowerLeft  , LowerMiddle, LowerRight};
    enum OverUnderFlow{Overflow=0, Underflow};    //the over/underflow bin (1D hist)

  protected:
    //the memory contains the histogram in range nbins
    //after that there are some reservered spaces for over/underflow statistics
    std::vector<T>            _memory;
    size_t                    _dimension;
    std::vector<AxisProperty> _axis;
    size_t                    _nbrOfFills;
    uint16_t                  _version;
  };


  //1D Histogram for Graphs, ToF's etc...
  template <typename T>
  class CASSSHARED_EXPORT Histogram1D : public HistogramBase<T>
  {
  public:
    //constructor for creating a histogram//
    Histogram1D(size_t nbrXBins, float xLow, float xUp)
    {
      //the dimension of the 1d Hist
      _dimension=1;
      //resize the memory, reserve space for the over/underflow bin
      _memory.resize(nbrXBins+2,0);
      //set up the axis
      _axis.push_back(AxisProperty(nbrXBins,xLow,xUp));
    }
    //Constructor for reading a histogram from a stream//
    Histogram1D(Serializer &in)   {deserialize<T>(in);}

    void fill(float x, T weight=1);
  };



  //2D Histogram for Detector Pictures etc...
  template <typename T>
  class CASSSHARED_EXPORT Histogram2D : public HistogramBase<T>
  {
    //constructor creating histo//
    Histogram2D(size_t nbrXBins, float xLow, float xUp,
                size_t nbrYBins, float yLow, float yUp)
    {
      //create memory, reserve space for under/over quadrants
      _memory.resize(nbrXBins*nbrYBins+8,0);
      _dimension=2;
      //set up the two axis of the 2d hist
      _axis.push_back(AxisProperty(nbrXBins,xLow,xUp));
      _axis.push_back(AxisProperty(nbrYBins,yLow,yUp));
    }

    Histogram2D(Serializer &in)     {deserialize<T>(in);}
    void fill(float x, float y, T weight=1);
  };
}

//---------------Axis-------------------------------
inline
void cass::AxisProperty::serialize(Serializer &out)const
{
  //the version//
  out.addUint16(_version);

  //number of bins, lower & upper limit
  out.addSizet(_nbrBins);
  out.addFloat(_low);
  out.addFloat(_up);
}
inline
void cass::AxisProperty::deserialize(Serializer &in)
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
template <typename T>
inline
void cass::HistogramBase<T>::serialize(Serializer &out)const
{
  //the version//
  out.addUint16(_version);
  //the dimension//
  out.addSizet(_dimension);
  //the axis properties//
  std::vector<AxisProperty>::const_iterator it;
  for (it=_axisproperties.begin(); it !=_axisproperties.end();++it)
    it->serialize(out);
  //size of the memory//
  size_t size = _memory.size();
  out.addSizet(size);
  //the memory//
  for (std::vector<T>::const_iterator it=_memory.begin(); it!=_memory.end();++it)
    out.add(*it);
}

template <typename T>
inline
void cass::HistogramBase<T>::deserialize(Serializer &in)
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
  //make sure the axis container is big enough for all dimensions//
  _axisproperties.resize(_dimension);
  std::vector<AxisProperty>::iterator it;
  for (it=_axisproperties.begin(); it !=_axisproperties.end();++it)
    it->deserialize(in);
  //the memory//
  size_t size = in.retrieveSizet();
  _memory.resize(size);
  for (std::vector<T>::iterator it=_memory.begin(); it!=_memory.end();++it)
    *it = in.retrieve<T>();
}


//-----------------1D Hist--------------------------
template <typename T>
inline
void cass::Histogram1D<T>::fill(float x, T weight)
{
  //calc the bin//
  const size_t nxBins = _axis[xAxis].nbrBins();
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
  ++_nbrOfFills;
}

//-----------------2D Hist--------------------------
template <typename T>
inline
void cass::Histogram2D<T>::fill(float x, float y, T weight)
{
  //calc the xbin//
  const size_t nxBins = _axis[xAxis].nbrBins();
  const float xlow    = _axis[xAxis].lowerLimit();
  const float xup     = _axis[xAxis].upperLimit();
  const int xBin = static_cast<int>( nxBins * (x - xlow) / (xup-xlow));

  //calc the ybin//
  const size_t nyBins = _axis[yAxis].nbrBins();
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
  ++_nbrOfFills;
}


#endif // HISTOGRAM_H
