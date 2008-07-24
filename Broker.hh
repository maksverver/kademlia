#ifndef BROKER_HH_INCLUDED
#define BROKER_HH_INCLUDED

#include "kademlia.hh"
#include "Id.hh"

class Node_impl;

class Broker_impl :
    public POA_kademlia::Broker
{

public:
    Broker_impl(Node_impl &node);
    virtual ~Broker_impl();
    
    void store (
        const kademlia::id_t index,
        const CORBA::Any &value, 
        CORBA::ULong lifetime );
              
    void erase (
        const kademlia::id_t index,
        const CORBA::Any& value );
        
    kademlia::seq_any_t *retrieve (
        const kademlia::id_t index );

    kademlia::seq_node_ref_t *find_nodes (
        const Id &target );

private:
	Node_impl &_node;
        
}; // class Broker

#endif //ndef BROKER_HH_INCLUDED
