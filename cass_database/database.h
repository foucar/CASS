/*
 *  Database.h
 *  diode
 *
 *  Created by lutz foucar
 *
 */
#include <map>
#include "Event.h"

namespace cass
{
	namespace database
	{
		class Database
		{
        public:
            size_t size()const                 {return _db.size();}
            cass::Event* event(uint64_t id)    {return _db[id];}
            void add(cass::Event* e)           {_db[e->id()]=e;}
        private:
            std::map<uint64_t,cass::Event*> _db;
		};
	}//end namespace database
}//end namespace cass
