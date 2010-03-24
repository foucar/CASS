// Copyright (C) 2009 Jochen Küpper,lmf

#ifndef CASS_FORMATCONVERTER_H
#define CASS_FORMATCONVERTER_H

#include <map>
#include <QtCore/QMutex>
#include <QtCore/QObject>
#include "cass.h"
#include "conversion_backend.h"
#include "parameter_backend.h"
#include "pdsdata/xtc/TypeId.hh"

namespace cass
{
  class CASSEvent;
  class ConversionBackend;

  // @class Format converter container
  //Only one FormatConvert object must exist, therefore this is implemented as a singleton.
  //@author Jochen Küpper,lmf
  //@version 0.2

  //the parameters for the converter//
  class CASSSHARED_EXPORT ConverterParameter : public cass::ParameterBackend
  {
  public:
    ConverterParameter()     {beginGroup("Converter");}
    ~ConverterParameter()    {endGroup();}
    void load();
    void save();

  public:
    bool _useCCD;
    bool _useAcqiris;
    bool _usepnCCD;
    bool _useMachine;
  };

  //the formatconverter that contains all converters
  class CASSSHARED_EXPORT FormatConverter : public QObject
  {
    Q_OBJECT;

  public:
    // list of known individual format converters //
    enum Converters {pnCCD, Acqiris, ccd, MachineData, Blank};
    // Destroy the single FormatConverter instance//
    static void destroy();
    // Return a pointer to the single FormatConverter instance//
    static FormatConverter *instance();
    //function to process a datagram and turn it into a cassevent/
    bool processDatagram(cass::CASSEvent*);
    //functions to load / save the settings for the format converter//
    void loadSettings(size_t what);
    void saveSettings();

  protected:
    FormatConverter();
    ~FormatConverter();

  public:
    typedef std::map<Converters, ConversionBackend *> availableConverters_t;
    typedef std::map<Pds::TypeId::Type, ConversionBackend *> usedConverters_t;

  protected:
    //functions to add / remove analyzers from the list//
    void addConverter(Converters);
    void removeConverter(Converters);
    //the parameters//
    ConverterParameter _param;
    // Available format converters
    availableConverters_t _availableConverters;
    //map of all xtc types there are, we add the appropriate
    //converters at the right position
    usedConverters_t _usedConverters;
    // pointer to the single instance //
    static FormatConverter *_instance;
    //Singleton operation locker in a multi-threaded environment.//
    static QMutex _mutex;
  };

}//end namespace cass


#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
