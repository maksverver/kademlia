#include "ContactTable.hh"

#include <algorithm>
#include <cstring>
#include <map>

#include "Node.hh"
#include "logging.hh"

using namespace kademlia;
using namespace std;

void *contacttable_thread(void *ct_arg)
{
	trace(10) << "contacttable_thread(): ContactTable maintenance thread started" << endm;
	ContactTable &ct = *reinterpret_cast<ContactTable*>(ct_arg);
	kademlia::node_ref_t node_ref = ct._node.reference();
	for(int pass = 0; ; ++pass)
	{
		omni_thread::self()->sleep(1);
		omni_mutex_lock l(ct._mutex);
		if(ct._destructing)
		{
			trace(10) << "contacttable_thread(): ContactTable maintenance thread exiting" << endm;
			return 0;
		}
		if(pass < 10)
			continue;
		pass = 0;

		// Iterate over all known contacts.
		mstime_t t = now();
		for(unsigned n = 0; n < ContactTable::buckets_size; ++n)
		{
			ContactTable::bucket_t &bucket = ct._buckets[n];
			ContactTable::bucket_t::iterator i, j;
			i = j = bucket.begin();
			while(i != bucket.end())
			{
				++j;
				if( i->second.last_seen == 0 ||
				    i->second.last_seen + ContactTable::ping_interval <= t)
				{
					trace(29) << "contacttable_thread(): pinging contact with id\n" <<
						i->first << endm;
					try
					{
						kademlia::id_t_var raw_id = i->second.node->ping(node_ref);
						Id id(raw_id);
						if(id != i->first)
						{
							trace(20) << "contacttable_thread(): invalid node reference for id\n" <<
								i->first << "\nreassigning node in contact table" << endm;
							kademlia::Node_var node = i->second.node;
							bucket.erase(i);
							ct.insert_unlocked(id, node, true);
						}
						else
						{
							t = now();
							i->second.last_seen = t;
							if(!i->second.first_seen)
								i->second.first_seen = t;
						}
					}
					catch(const CORBA::Exception &)
					{
						trace(20) << "contacttable_thread(): ping failed for node with id\n" <<
							i->first << "\nremoving node from contact table" << endm;
						bucket.erase(i);
					}						
				}
				i = j;
			}
		}
	}
	// should never get here.
}

static Node_impl *const nil_node = 0;
ContactTable::ContactTable(
    const Id  &origin ) :
    _origin(origin),
	_node(*nil_node)
{
}

ContactTable::ContactTable(
    const Id  &origin,
	Node_impl &node) :
    _origin(origin),
	_node(node),
	_destructing(false),
	_thread(new omni_thread(contacttable_thread, this))
{
	_thread->start();
}

ContactTable::~ContactTable( )
{
	_mutex.lock();
	_destructing = true;
	_mutex.unlock();
	_thread->join(0);
}

void ContactTable::insert (
    const Id&                id,
    const kademlia::Node_ptr node,
	bool                     seen )
{
	const omni_mutex_lock l(_mutex);
	trace(25) << "ContactTable::insert()"
		<< "\n\t  Id=" << id
		<< "\n\tSeen=" << seen << endm;
	insert_unlocked(id, node, seen);
}

void ContactTable::insert_unlocked(
    const Id&                id,
    const kademlia::Node_ptr node,
	bool                     seen )
{

	bucket_t &bucket = get_bucket(id);
	bucket_t::iterator i = bucket.find(id);
	if(i == bucket.end() && bucket.size() < max_bucket_size)
	{
		// Insert new contact, only if bucket is not yet full.
		i = bucket.insert(make_pair(id, Contact(node))).first;
	}

	if(seen)
	{
		// Update time last (and maybe first) seen
		mstime_t t = now();
		if(!i->second.first_seen)
			i->second.first_seen = t;
		i->second.last_seen = t;
	}
}

void ContactTable::erase(
	const Id &id )
{
	const omni_mutex_lock l(_mutex);
	trace(25) << "ContactTable::erase()"
		<< "\n\t  Id=" << id << endm;

	bucket_t &bucket = get_bucket(id);
	bucket.erase(id);
}

seq_node_ref_t* ContactTable::retrieve (
    const Id& id )
{
	const omni_mutex_lock l(_mutex);
	trace(25) << "ContactTable::retrieve()"
		<< "\n\t  Id=" << id << endm;

	typedef map<Id, std::pair<Id, Contact> > nearby_map_t;
	nearby_map_t nearby_contacts;
    for(unsigned n = 0; n < buckets_size; ++n)
        for(bucket_t::const_iterator i = _buckets[n].begin(); i != _buckets[n].end(); ++i)
        {
            const Id difference = id ^ i->first;
            if(nearby_contacts.size() == replication_factor)
            {
				nearby_map_t::iterator last_elem = nearby_contacts.end();
                --last_elem;
                if(difference >= last_elem->first)
                    continue;
                nearby_contacts.erase(last_elem);
            }
            nearby_contacts.insert(make_pair(difference, *i));
        }

    seq_node_ref_t_var result = new seq_node_ref_t();
	result->length(nearby_contacts.size());
    nearby_map_t::const_iterator i = nearby_contacts.begin();
	for(unsigned n = 0; n < result->length(); ++n)
    {
        memcpy(result[n].id, i->second.first, sizeof(result[n].id));
        result[n].ref = i->second.second.node;
        ++i;
    }
    return result._retn();
}

ContactTable::bucket_t &ContactTable::get_bucket(const Id &id)
{
	return _buckets[ (id ^ _origin).bitscan() ];
}

seq_node_ref_t* ContactTable::contents( )
{
    seq_node_ref_t_var result = new seq_node_ref_t();
    const omni_mutex_lock l(_mutex);
    unsigned size = 0;
    for(unsigned b = 0; b < buckets_size; ++b)
        size += _buckets[b].size();
    result->length(size);
    unsigned n = 0;
    for(unsigned b = 0; b < buckets_size; ++b)
        for(bucket_t::const_iterator i = _buckets[b].begin(); i != _buckets[b].end(); ++i)
        {
            memcpy(result[n].id, i->first, sizeof(result[n].id));
            result[n].ref = i->second.node;
            ++n;
        }
    return result._retn();
}