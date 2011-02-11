// Copyright (C) Lutz Foucar

#ifndef MACHINEDATACONVERTER_H
#define MACHINEDATACONVERTER_H

#include <map>

#include <QtCore/QMutex>

#include "cass_machine.h"
#include "conversion_backend.h"
#include "machine_device.h"

namespace cass
{
  class CASSEvent;

  namespace MachineData
  {
    /** Converter for Beamline-, Cavity-, Epics- and EVR Data
     *
     * Will convert Beamline data, Cavity data, Epics Data and EVR Data
     *
     * @note maybe split this to several converters for the different data types
     *       will this work. ie is only the epics map copied or the whole
     *       event?
     *
     * @author Lutz Foucar
     */
    class CASS_MACHINEDATASHARED_EXPORT Converter : public cass::ConversionBackend
    {
    public:
      /** create singleton if doesnt exist already */
      static ConversionBackend::shared_pointer instance();

       /** called for appropriate xtc part.
       *
       * @param xtc the xtc that contains evr, epics, beamlinedata info
       * @param evt pointer to the event that we will write the data to.
       */
      void operator()(const Pds::Xtc*xtc, cass::CASSEvent*evt);

    private:
      /** convenience typedef for easier readable code*/
      typedef std::map<int,std::string> indexMap_t;

    private:
      /** constructor
       *
       * sets up the pds types that it is responsible for
       */
      Converter();

      /** prevent copy construction */
      Converter(const Converter&);

      /** prevent assignment */
      Converter& operator=(const Converter&);

      /** the singleton container */
      static ConversionBackend::shared_pointer _instance;

      /** singleton locker for mutithreaded requests */
      static QMutex _mutex;

      /** map to convert epics indexes to strings */
      indexMap_t _index2name;

      /** a container for the machindata values
       *
       * @note this is necessary, since not every shot there is info about the
       *       epics values
       */
      MachineDataDevice _store;
    };
  }//end namespace MachineData
}//end namespace cass

#endif
