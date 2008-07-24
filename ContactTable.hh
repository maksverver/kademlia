#ifndef CONTACTTABLE_HH_INCLUDED
#define CONTACTTABLE_HH_INCLUDED

#include "kademlia.hh"

#include "Id.hh"
#include "time.hh"

#include <map>

class Node_impl;

class ContactTable
{
public:
    ContactTable( // exists for testing only!
		const Id        &origin );
			
    ContactTable(
		const Id        &origin,
		      Node_impl &node );
	
	~ContactTable( );

	void insert (
        const Id&                id,
        const kademlia::Node_ptr node,
		bool                     seen = false );

	void erase (
		const Id& id );

	kademlia::seq_node_ref_t* ContactTable::retrieve (
		const Id& id );
		
    kademlia::seq_node_ref_t* contents( );

private:
    struct Contact
    {
		Contact(const kademlia::Node_ptr node) :
			node(kademlia::Node::_duplicate(node))
		{
		}

        kademlia::Node_var node;
        mstime_t           first_seen,
                           last_seen;
    };
    
    typedef std::map<Id, Contact> bucket_t;

	static const unsigned ping_interval = 600*1000;	// 10 minutes

	static const unsigned max_bucket_size =
		kademlia::replication_factor + 2;

    static const unsigned buckets_size = Id::bits+1;

	bucket_t &get_bucket(
		const Id &id );
    
    void insert_unlocked (
        const Id&                id,
        const kademlia::Node_ptr node,
		bool                     seen = false);

private:
	omni_mutex _mutex;
	
	const Id  _origin;
	Node_impl &_node;    
	
    bucket_t _buckets[buckets_size];
	
	bool _destructing;

	omni_thread *_thread;
	friend void *contacttable_thread(void *dt);	
    
}; // class ContactTable

#endif //ndef CONTACTTABLE_HH_INCLUDED
