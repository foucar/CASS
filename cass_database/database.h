/*
 *  Database.h
 *  diode
 *
 *  Created by lutz foucar
 *
 */
#include <QtCore/QObject>
//#include <map>
//#include "cass_event.h"

namespace cass
{
    class CASSEvent;

    namespace database
    {
        class Database : public QObject
        {
            Q_OBJECT;

        public:
            Database() {}
            ~Database() {}


            cass::CASSEvent* nextEvent() {return 0;}
           

        public slots:
            void add(cass::CASSEvent*) {}

        private:
        };
    }//end namespace database
}//end namespace cass
