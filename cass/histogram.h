#ifndef HISTOGRAM_H
#define HISTOGRAM_H

namespace cass
{
  class AxisProperty
  {
  public:
    AxisProperty(size_t nbrBins, float lowerLimit, float upperLimit)
        :_nbrBins(nbrBins),
         _low(lowerLimit),
         _up(upperLimit)
    {
    }
    ~AxisProperty() {}
    size_t nbrBins()const   {return _nbrBins;}
    float  lowerLimit()const{return _low;}
    float  upperLimit()const{return _up;}
  private:
    size_t _nbrBins;
    float  _low;
    float  _up;
  };

  template <typename T>
  class HistogramBase
  {
  public:
    HistogramBase():_dimension(0) {}
    virtual ~HistogramBase()      {}

    virtual size_t dimension()  {return _dimension;}
    virtual void reset()        {_memory.assign(_memory.size(),0);}
    virtual size_t size()
    {
      return _memory.size()*sizeof(T) + //the size of the memory in bytes//
             sizeof(size_t) +   //the dimesion value//
             (_dimension*(sizeof(size_t)+2*sizeof(float))); //the axis properties//
    }
    virtual void serialize(char * buf)=0;
    virtual void deserialize(const char * buf)=0;

  protected:
    enum axis{xAxis=0,yAxis,zAxis};
    std::vector<T>            _memory;
    size_t                    _dimension;
    std::vector<AxisProperty> _axisproperties;
  };



  template <typename T>
  class Histogram1D : public HistogramBase<T>
  {
  public:
    //when creating a histogram//
    Histogram1D(size_t nbrXBins, float xLow, float xUp):
        _memory(nbrXBins,0),
        _dimension(1)
    {
      _axisproperties.push_back(AxisProperty(nbrXBins,xLow,xUp));
    }
    //when reading a histogram from a stream//
    Histogram1D(const char* buf)
    {
      deserialize(buf);
    }

    void fill(float x, T weight=1)
    {
      size_t xBin = static_cast<int>(_axisproperties[xAxis].nbrBins() *
                    (x - _axisproperties[xAxis].lowerLimit()) /
                    (_axisproperties[xAxis].upperLimit()-_axisproperties[xAxis].lowerLimit()));
      //make some sanity check before filling or just use the try - catch mechanism//
      _memory[xBin] += weight;
    }

    void serialize(char *buf)
    {
      //first write the size of the histogram//
      size_t size = size();
      memcpy(buf,reinterpret_cast<char*>(size),sizeof(size_t));
      buf += sizeof(size_t);
      //now write the dimension and the axisproperties//
      memcpy(buf,reinterpret_cast<char*>(_dimension), sizeof(size_t));
      buf += sizeof(size_t);
      for (size_t i=0; i<_axisproperties.size();++i)
      {
        memcpy(buf,reinterpret_cast<char*>(_axisproperties[i]),sizeof(AxisProperty));
        buf+=sizeof(AxisProperty);
      }
      //now write the memory to the buffer//
      memcpy(buf,reinterpret_cast<char*>(&_memory[0]),sizeof(T)*_memory.size());
    }

    void deserialize(const char * buf)
    {
      //find the total size//
      size_t size;
      memcpy(reinterpret_cast<char*>(size),buf,sizeof(size_t));
      buf+=sizeof(size_t);
      size-=sizeof(size_t);
      //read the dimesion//
      memcpy(reinterpret_cast<char*>(_dimension),buf,sizeof(size_t));
      buf+=sizeof(size_t);
      size-=sizeof(size_t);
      //read the axisproperties//
      for (size_t i=0; i<_dimension; ++i)
      {
        AxisProperty *prop = reinterpret_cast<AxisProperty*>(buf);
        _axisproperties.push_back(*prop);
        buf+=sizeof(AxisProperty);
        size-=sizeof(size_t);
      }
      //read the memory//
      size_t nElements = size/sizeof(T);
      _memory.resize(nElements);
      memcpy(reinterpret_cast<char*>(&_memory[0]),buf,size);
    }
  };




  template <typename T>
  class Histogram2D : public HistogramBase<T>
  {
    Histogram2D(size_t nbrXBins, float xLow, float xUp,
                size_t nbrYBins, float yLow, float yUp):
        _memory(nbrXBins*nbrYBins,0),
        _dimension(2)
    {
      _axisproperties.push_back(AxisProperty(nbrXBins,xLow,xUp));
      _axisproperties.push_back(AxisProperty(nbrYBins,yLow,yUp));
    }

    //needs to be fill according to 1d histogram...
  };
}
#endif // HISTOGRAM_H
