// Copyright (C) 2010 lmf
// Copyright (C) 2010 Jochen Küpper

#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "serializer.h"

namespace cass
{

/*! Axis properties for histograms

this describes the properties of the axis of the histogram
*/
class CASSSHARED_EXPORT AxisProperty
{
public:
    AxisProperty(size_t nbrBins, float lowerLimit, float upperLimit)
        : _size(nbrBins), _low(lowerLimit), _up(upperLimit)
    {}

    AxisProperty(Serializer &in)
    {
        deserialize(in);
    }
    ~AxisProperty() {}

    void serialize(Serializer&)const;
    void deserialize(Serializer&);

    /*! @return size (nuber of bins) of axis */
    size_t size() const {return _size;}

    /*! Convenience function

    @deprecated Use size() instead    @see size()
    */
    size_t nbrBins() const {return size();}

    /*! Lower limit of axis */
    float lowerLimit()const {return _low;};

    /*! Upper limit of axis */
    float upperLimit()const {return _up;}

    /*! bin-index for position x */
    size_t bin(float x);


protected:

    /*! the number of bins in this axis */
    size_t _size;

    /*! lower limit of the axis */
    float _low;

    /*! upper limit of the axis */
    float _up;


private:

    /*! internal version for de/serializing */
    uint16_t _version;
};



//histogram backend, every type of histogram inherits from here//
class CASSSHARED_EXPORT HistogramBackend
{
public:

    /** constructor */
    explicit HistogramBackend(size_t dim)
        : _dimension(dim), _nbrOfFills(0), _version(1)
    {}

    virtual ~HistogramBackend(){}

    /*! Serialize this object to a string */
    virtual void serialize(Serializer&)const=0;

    /*! Deserialize this object from a string */
    virtual void deserialize(Serializer&)=0;

    typedef std::vector<AxisProperty> axis_t;

    size_t   nbrOfFills()const {return _nbrOfFills;}
    size_t  &nbrOfFills()      {return _nbrOfFills;}
    size_t   dimension()const  {return _dimension;}
    const axis_t  &axis()const {return _axis;}

    /*! possible axes

    convenience type to allow for easier choosing of the axis
    */
    enum Axis{xAxis=0,yAxis,zAxis};

    /*! possible over/underflow quadrants

    convenience type to allow for easier choosing of the over-/underflow quadrant (2D histograms)
    */
    enum Quadrant{UpperLeft=0, UpperMiddle, UpperRight,
                  Left,                     Right,
                  LowerLeft  , LowerMiddle, LowerRight};

    /*! the over/underflow bin (1D hist) */
    enum OverUnderFlow{Overflow=0, Underflow};

protected:
    size_t    _dimension;
    axis_t    _axis;
    size_t    _nbrOfFills;
    uint16_t  _version;
};







// base class for floats from which all float histograms inherit
class CASSSHARED_EXPORT HistogramFloatBase : public HistogramBackend
{
public:

    typedef float value_t;
    typedef std::vector<value_t> storage_t;


    explicit HistogramFloatBase(size_t dim, size_t memory_size=0)
        : HistogramBackend(dim), _memory(memory_size, 0.)
    {}

    HistogramFloatBase(Serializer& in)
        : HistogramBackend(0)
    {
        deserialize(in);
    }
    virtual ~HistogramFloatBase()      {}
    virtual void serialize(Serializer&)const;
    virtual void deserialize(Serializer&);

    /** @return const reference to histogram data */
    const storage_t& memory() const {return _memory;}

    /** @overload */
    storage_t& memory() { return _memory; };

protected:
    //reset the histogram//
    virtual void reset() { _memory.assign(_memory.size(), 0); }

    //the memory contains the histogram in range nbins
    //after that there are some reservered spaces for over/underflow statistics
    storage_t _memory;
};




/*! "0D Histogram" scalar value */
class CASSSHARED_EXPORT Histogram0DFloat : public HistogramFloatBase
{
public:

    /*! Create a 0d histogram of a single float */
    explicit Histogram0DFloat()
        : HistogramFloatBase(0, 1)
    {};

    /*! Constructor for reading a histogram from a stream */
    Histogram0DFloat(Serializer &in)
        : HistogramFloatBase(in)
    {};

    void fill(value_t value=0.) {_memory[0] = value; };

    /*! Simple assignment ot the single value */
    Histogram0DFloat& operator=(value_t val) { fill(val); return *this; };
};



/*! 1D Histogram for Graphs, ToF's etc... */
class CASSSHARED_EXPORT Histogram1DFloat : public HistogramFloatBase
{
public:

    /*! Create a 1d histogram of floats */
    explicit Histogram1DFloat(size_t nbrXBins, float xLow, float xUp)
        : HistogramFloatBase(1)
    {
        //resize the memory, reserve space for the over/underflow bin
        _memory.resize(nbrXBins+2,0);
        //set up the axis
        _axis.push_back(AxisProperty(nbrXBins,xLow,xUp));
    };

    /*! Constructor for reading a histogram from a stream */
    Histogram1DFloat(Serializer &in)
        : HistogramFloatBase(in)
    {};

    void fill(float x, value_t weight=1.);
};



/** @brief 2D Histogram

for detector images, i.e., pnCCD, VMI CCD, etc...
*/
class CASSSHARED_EXPORT Histogram2DFloat : public HistogramFloatBase
{
public:

    /** create histogram */
    explicit Histogram2DFloat(size_t nbrXBins, float xLow, float xUp,
                              size_t nbrYBins, float yLow, float yUp)
        : HistogramFloatBase(2)
    {
        //create memory, reserve space for under/over quadrants
        _memory.resize(nbrXBins*nbrYBins+8,0);
        //set up the two axis of the 2d hist
        _axis.push_back(AxisProperty(nbrXBins,xLow,xUp));
        _axis.push_back(AxisProperty(nbrYBins,yLow,yUp));
    }

    /** create default histogram

    @overload

    This constructor creates a histogram with integer positions, i.e., it will have positions
    (0,0), (0,1), ... (0,cols-1), (1,0), ... (rows-1,cols-1)

    @param rows, cols Number of bins per row and column.
    */
    explicit Histogram2DFloat(size_t rows, size_t cols)
        : HistogramFloatBase(2)
    {
        // create memory, reserve space for under/over quadrants
        _memory.resize(rows * cols + 8, 0);
        // set up the two axis of the 2d hist
        _axis.push_back(AxisProperty(rows, 0., float(rows-1.)));
        _axis.push_back(AxisProperty(cols, 0., float(cols-1.)));
    }


    Histogram2DFloat(Serializer &in)
        : HistogramFloatBase(in)
    {}

    /** Return histogram bin that contains (x,y) */
    value_t& operator()(float x, float y) { return _memory[_axis[0].bin(x) + _axis[1].bin(y) * _axis[0].size()]; };

    /** Return histogram bin (row,col) */
    value_t& bin(size_t row, size_t col) { return _memory[row + col * _axis[0].size()]; };

    /** Add datum to histogram

    @param x, y Position of datum
    @param weight value of datum
    */
    void fill(float x, float y, value_t weight=1.);
};




//--inlined functions---//

//---------------Axis-------------------------------

inline
void cass::AxisProperty::serialize(cass::Serializer &out) const
{
    //the version//
    out.addUint16(_version);

    //number of bins, lower & upper limit
    out.addSizet(_size);
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
    _size     = in.retrieveSizet();
    _low      = in.retrieveFloat();
    _up       = in.retrieveFloat();
}



inline size_t AxisProperty::bin(float pos)
{
    if(pos < _low)
        throw std::out_of_range("Requested position to low");
    else if(pos > _up)
        throw std::out_of_range("Requested position to high");
    size_t index(static_cast<size_t>(_size * ((pos - _low) / (_up - _low))));
    if(size() == index)
        // be forgiving and compensate for rounding errors
        --index;
    assert(index < size());
    return index;
}



//-----------------Base class-----------------------
inline
void cass::HistogramFloatBase::serialize(cass::Serializer &out) const
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
    for (storage_t::const_iterator it=_memory.begin(); it!=_memory.end();++it)
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
    for (storage_t::iterator it=_memory.begin(); it!=_memory.end();++it)
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

} //end namespace cass


#endif // HISTOGRAM_H



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
