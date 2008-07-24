#include "DataTable.hh"
#include "random.hh"
#include "logging.hh"
#include "compare_any.hh"
using namespace kademlia;

void *datatable_thread(void *dt_arg)
{
	trace(10) << "datatable_thread(): DataTable maintenance thread started" << endm;
	DataTable *dt = reinterpret_cast<DataTable*>(dt_arg);
	for(int pass = 0; ; ++pass)
	{
		omni_thread::self()->sleep(1);
		omni_mutex_lock l(dt->_mutex);
		if(dt->_destructing)
		{
			trace(10) << "datatable_thread(): DataTable maintenance thread exiting" << endm;
			return 0;
		}
		if(pass < 10)
			continue;
		pass =0;

		// Purge old data entries.
		unsigned count = dt->purge_unlocked();
		if(count)
			trace(29) << "datatable_thread(): " << count << " data entries purged" << endm;

		// TODO: Republish entries that are due for republishing.
	}
	// should never get here.
}

DataTable::DataTable() :
	_destructing(false),
	_thread(new omni_thread(datatable_thread, this))
{
	_thread->start();
}

DataTable::~DataTable()
{
	_mutex.lock();
	_destructing = true;
	_mutex.unlock();
	_thread->join(0);
}

void DataTable::store(
    const Id &index,
    const CORBA::Any &value,
    mstime_t lifetime )
{
	omni_mutex_lock l(_mutex);
	std::ostream &log = trace(25);
	log << "DataTable::store(): storing value at index:\n" << index;

	// Create a new storage entry
    DataEntry entry;
	mstime_t t = now();
    entry.value           = value;
    entry.expiration_time = t + lifetime;
    entry.republish_time  = t + kademlia::republish_interval + randInt(kademlia::republish_interval / 10); 
	
	// See if an existing entry is available
	for(contents_t::iterator i = _contents.find(index);
		i != _contents.end() && i->first == index; ++i)
	{
		if(i->second.value == value)
		{
			// Update existing entry.
			i->second = entry;
			return;
		}
	}
    
	// Add new entry
	_contents.insert( std::pair<const Id, DataEntry>(index, entry) );
}

kademlia::seq_value_t *DataTable::retrieve(
    const Id &index )
{
	omni_mutex_lock l(_mutex);

	mstime_t t = now();
    
    contents_t::const_iterator i = _contents.find(index), j = i;
    unsigned count = 0;
    while(j != _contents.end() && j->first == i->first)
    {
        if(j->second.expiration_time > t)
            ++count;
        ++j;
    };

	trace(25) << "DataTable::store(): retrieving values at index:\n" << index <<
			"\n" << count << " entries found." << endm;

    kademlia::seq_value_t *values = new kademlia::seq_value_t(count);
    values->length(count);
    for(int n = 0; i != j; ++i, ++n)
        if(i->second.expiration_time > t)
        {
            (*values)[n].contents = i->second.value;
            (*values)[n].lifetime = static_cast<lifetime_t>( i->second.expiration_time - t );
        }
    return values;
}

unsigned DataTable::purge( )
{
	omni_mutex_lock l(_mutex);
	trace(25) << "DataTable::purge()" << endm;
	
	return purge_unlocked();
}

unsigned DataTable::purge_unlocked( )
{
	unsigned purged = 0;
    mstime_t t = now();
    contents_t::iterator i, j;
    i = j = _contents.begin();
    while(i != _contents.end())
    {
        ++j;
        if(i->second.expiration_time <= t)
        {
            ++purged;
            _contents.erase(i);
        }
        i = j;
    }
    return purged;
}

seq_entry_t* DataTable::contents( )
{
    seq_entry_t_var result = new seq_entry_t();

	omni_mutex_lock l(_mutex);
	mstime_t t = now();
	result->length(_contents.size());
	unsigned n = 0;
    for(contents_t::const_iterator i = _contents.begin(); i != _contents.end(); ++i)
        if(i->second.expiration_time > t)
        {
            memcpy(result[n].index, i->first, sizeof(result[n].index));
            result[n].value.lifetime = static_cast<lifetime_t>( i->second.expiration_time - t );
            result[n].value.contents = i->second.value;
            ++n;
        }
    result->length(n);
    return result._retn();
}