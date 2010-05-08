// Copyright (C) 2009 Jochen Kuepper
// Copyright (C) 2009, 2010 Lutz Foucar

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
  //forward delarations
  class CASSEvent;
  class ConversionBackend;

  /** Parameteres for Converter.
   * The Parameters used by the Format Converter
   * @author Lutz Foucar
   */
  class CASSSHARED_EXPORT ConverterParameter : public cass::ParameterBackend
  {
  public:
    /** constructor starts the group called "Converter"*/
    ConverterParameter()     {beginGroup("Converter");}
    /** destructor closes group "Converter"*/
    ~ConverterParameter()    {endGroup();}
    /** load the parameters from cass.ini*/
    void load();
    /** save the parameters to cass.ini*/
    void save();

  public:
    bool _useCCD;     //!< switch whether to convert the commmercial ccd
    bool _useAcqiris; //!< switch whether to convert the Acqiris Data
    bool _usepnCCD;   //!< switch whether to convert the pnCCD
    bool _useMachine; //!< switch whether to convert the Data provided by the Machine
  };

  /** Format converter container.
   *
   * Only one FormatConvert object must exist, therefore this is implemented
   * as a singleton. It contains all available Format Converters and calls
   * all requested ones.
   *
   * The user can set which converters should be running, options are true or false:
   * - Converter/{useCommercialCCDConverter}
   * - Converter/{useAcqirisConverter}
   * - Converter/{useMachineConverter}
   * - Converter/{usepnCCDConverter}
   *
   * @author Jochen Kuepper
   * @author Lutz Foucar
   */
  class CASSSHARED_EXPORT FormatConverter : public QObject
  {
    Q_OBJECT;

  public:
    //! list of known individual format converters
    enum Converters {pnCCD, Acqiris, ccd, MachineData, Blank};
    //! Destroy the single FormatConverter instance
    static void destroy();
    //! Return a pointer to the single FormatConverter instance
    static FormatConverter *instance();
    /** function to process a datagram and turn it into a cassevent.
     * this function will iterate through the xtc's contained in the datagram
     * extract the information and put into the right device of the CASSEvent
     * @return returns true if the datagram was a L1Accept (an event) transition
     * @param evt pointer to the CASSEvent which also contains the datagram
     */
    bool processDatagram(cass::CASSEvent*evt);
    //! function to load  the settings for the format converter
    void loadSettings(size_t what);
    //! function to  save the settings for the format converter
    void saveSettings();

  protected:
    /** constructor is made protected, should only be called through instance*/
    FormatConverter();
    /** destructor is made protected, should only be called through destroy*/
    ~FormatConverter();

  public:
    /** typdef describing the map of available converters for easier readable code*/
    typedef std::map<Converters, ConversionBackend *> availableConverters_t;
    /** typdef describing the map of used converters for easier readable code */
    typedef std::map<Pds::TypeId::Type, ConversionBackend *> usedConverters_t;

  protected:
    //! functions to add converters from the list
    void addConverter(Converters);
    //! function to remove converters from the list
    void removeConverter(Converters);
    //! the parameters//
    ConverterParameter _param;
    /** status whether a configure has already been seen */
    bool _configseen;
    //! Available format converters
    availableConverters_t _availableConverters;
    /** map that contains all type id's of all known xtc in a transition.
     * the converter used by the xtc is the second. We add the appropriate
     * converters at the right position
     */
    usedConverters_t _usedConverters;
    //! pointer to the single instance //
    static FormatConverter *_instance;
    //! Singleton operation locker in a multi-threaded environment.//
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
