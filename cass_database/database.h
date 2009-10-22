/*
 *  Database.h
 *  diode
 *
 *  Created by lutz foucar, modified by N. Coppola
 *
 */
#include <QtCore/QObject>

namespace cass
{
    class CASSEvent;

    namespace database
    {
        class Database : public QObject
        {
            Q_OBJECT;

        public:
            // I actually already need to know the structure of the CASSEvent...
            // something like
            //Database(cass::CASSEvent*);
            Database();
            ~Database();
            signals:
            void nextEvent();

            //cass::CASSEvent* nextEvent();
           

        public slots:
            void add(cass::CASSEvent*);

        private:
        };
    }//end namespace database
}//end namespace cass
