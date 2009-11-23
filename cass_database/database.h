/*
 *  Database.h
 *  diode
 *
 *  Created by lutz foucar, modified by N. Coppola
 *
 */
#include <QtCore/QObject>
#include <TNtuple.h>
#include "analysis_backend.h"
#include "parameter_backend.h"

#define CASS_DATABASE_GLOBAL_H

#include <QtCore/qglobal.h>
#  define CASS_DATABASESHARED_EXPORT Q_DECL_EXPORT

namespace cass
{
    class CASSEvent;

    namespace database
    {
        class CASS_DATABASESHARED_EXPORT Parameter : public cass::ParameterBackend
        {
        public:
            Parameter()     {beginGroup("Database");}
            ~Parameter()    {endGroup();}
            void load();
            void save();

        public:
            uint   _updatefrequency;
            uint   _number_ofevents;
            uint16_t   _useREMI;
            uint16_t   _useVMI;
            uint16_t   _usepnCCD;
        };

        class CASS_DATABASESHARED_EXPORT Database : public QObject
        {
            Q_OBJECT;

        public:
            // I actually already need to know the structure of the CASSEvent...
            // something like
            //Database(cass::CASSEvent*);
            Database();
            ~Database();
            void loadSettings()   {_param.load();}
            void saveSettings()   {_param.save();}
	    /*        public:
		      Name_set();*/

            signals:
            void nextEvent();

        public slots:
            void add(cass::CASSEvent*);

        private:
            TTree *T;
            Parameter  _param;
        };
    }//end namespace database
}//end namespace cass
