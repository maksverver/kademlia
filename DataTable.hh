#ifndef DATATABLE_HH_INCLUDED
#define DATATABLE_HH_INCLUDED

#include "kademlia.hh"

#include "time.hh"
#include "Id.hh"

#include <omnithread.h>
#include <map>

class DataTable
{
public:

    DataTable();
	~DataTable();
    
    void store(
        const Id &index,
        const CORBA::Any &value,
        mstime_t lifetime );
    
    kademlia::seq_value_t *retrieve(
        const Id &index );

    unsigned purge( );
    
    kademlia::seq_entry_t* contents( );


private:
	unsigned purge_unlocked( );


private:
	
	struct DataEntry
	{
		CORBA::Any value;
		mstime_t   expiration_time;
		mstime_t   republish_time;
	};
	
	omni_mutex _mutex;

    typedef std::multimap<Id, DataEntry> contents_t;
    contents_t _contents;

	bool _destructing;

	omni_thread *_thread;
	friend void *datatable_thread(void *dt);

}; // class DataEntry

#endif //ndef DATATABLE_HH_INCLUDED
