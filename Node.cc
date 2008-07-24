#include "Node.hh"
#include "Broker.hh"
using namespace kademlia;

#include "logging.hh"

extern CORBA::ORB_var orb;

Node_impl::Node_impl() :
    _id(Id::random()), 
    _ct(_id, *this),
    _startup_time(time(NULL))
{
	trace(10) << "Node_impl::Node_impl(): initialized node with ID " << _id << endm;
}

Node_impl::~Node_impl()
{
}

id_t_slice* Node_impl::ping(
    const node_ref_t& caller )
{
	trace(20) << "Node_impl::ping()" <<
			     "\n\tCaller=" << Id(caller.id).str() << endm;
    update(caller);
    return id_t_dup(_id);
}

void Node_impl::store(
    const node_ref_t& caller,
    const kademlia::id_t index,
    const value_t &value )
{
	trace(20) << "Node_impl()::store()\n"
			  << "\n\t  Caller=" << Id(caller.id).str()
              << "\n\t   Index=" << Id(index).str()
              << "\n\tLifetime=" << value.lifetime/1000.0 << endm;
    update(caller);
    _dt.store(index, value.contents, value.lifetime);
}

seq_value_t* Node_impl::retrieve(
    const node_ref_t& caller,
    const kademlia::id_t index )
{
    trace(20) << "Node_impl()::retrieve()"
			  << "\n\t  Caller=" << Id(caller.id).str()
              << "\n\t   Index=" << Id(index).str() << endm;
    update(caller);
    return _dt.retrieve(index);
}

seq_node_ref_t* Node_impl::find_nodes(
    const node_ref_t& caller,
    const kademlia::id_t target )
{
    trace(20) << "Node_impl()::find_nodes()"
			  << "\n\t  Caller=" << Id(caller.id).str()
              << "\n\t  Target=" << Id(target).str() << endm;
    update(caller);
    return _ct.retrieve(Id(target));
}

void Node_impl::update(const node_ref_t &caller)
{
    _ct.insert(Id(caller.id), caller.ref, true);
}

seq_node_ref_t* Node_impl::contacts( )
{
    return _ct.contents();
}
    
seq_entry_t* Node_impl::data( )
{
    return _dt.contents();
}    

CORBA::ULong Node_impl::age( )
{
    return static_cast<CORBA::ULong>( time(NULL) - _startup_time );
}

kademlia::node_ref_t Node_impl::reference( )
{
	kademlia::node_ref_t result;
	memcpy(result.id, _id, sizeof(result.id));
	result.ref = _this();
	return result;
}

bool Node_impl::add_initial_contact(const char *ior)
{
	info() << "Initial contact: " << ior << endm;
	try
	{
		CORBA::Object_var  obj  = orb->string_to_object(ior);
		kademlia::Node_var node = kademlia::Node::_narrow(obj);
		kademlia::id_t_var id   = node->ping(reference());
		_ct.insert(Id(id), node, true);
	    trace(20) << "Node_impl::add_initial_contact(): added contact with IOR\n"
			<< ior << endm;
		return true;
	}
	catch(const CORBA::Exception &)
	{
        trace(20) << "Node_impl::add_initial_contact(): failed to add contact with IOR " << ior << endm;
        return false;
	}
}

bool Node_impl::initialize( Broker_impl &broker )
{
    seq_node_ref_t_var nodes = broker.find_nodes(_id);
    if(nodes->length() == 0)
        return false;
    for(unsigned n = 0; n < nodes->length(); ++n)
    {
        Id node_id(nodes[n].id);
        info() << "Added neighbour node ID " << node_id << endm;
	    _ct.insert(node_id, nodes[n].ref);
    }
    return true;
}
