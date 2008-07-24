#ifndef NODE_HH_INCLUDED
#define NODE_HH_INCLUDED

#include <time.h>
#include "main.hh"

#include "ContactTable.hh"
#include "DataTable.hh"
#include "Id.hh"

class Broker_impl;

class Node_impl :
    public POA_kademlia::Node
{

public:
    Node_impl();
    virtual ~Node_impl();
    
    
    // kademlia::Node methods
    
    kademlia::id_t_slice* ping (
        const kademlia::node_ref_t& caller );
    
    void store (
        const kademlia::node_ref_t& caller,
        const kademlia::id_t index,
        const kademlia::value_t& value );
    
    kademlia::seq_value_t* retrieve (
        const kademlia::node_ref_t& caller,
        const kademlia::id_t index );
    
    kademlia::seq_node_ref_t* find_nodes (
        const kademlia::node_ref_t& caller,
        const kademlia::id_t target );

    const Id& id( ) const;

    void update(
        const kademlia::node_ref_t& caller );
		

    // kademlia::Node attributes

    kademlia::seq_node_ref_t* contacts( );
    
    kademlia::seq_entry_t* data( );
    
    CORBA::ULong age( );
    
    
	kademlia::node_ref_t reference( );

	bool add_initial_contact(
	    const char *ior );
	    
	bool initialize(
	    Broker_impl &broker );


private:
    Id           _id;
    DataTable    _dt;
    ContactTable _ct;
    time_t       _startup_time;

	friend class Broker_impl;

}; // class Node

inline const Id& Node_impl::id() const
{
    return _id;
}

#endif //ndef NODE_HH_INCLUDED
