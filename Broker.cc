#include "Broker.hh"
#include "Node.hh"
#include "compare_any.hh"
#include "logging.hh"

using namespace kademlia;

Broker_impl::Broker_impl(Node_impl &node) :
	_node(node)
{
}

Broker_impl::~Broker_impl()
{
}

void Broker_impl::store (
    const kademlia::id_t index_arr,
    const CORBA::Any &value, 
    CORBA::ULong lifetime )
{
	const node_ref_t& node_ref = _node.reference();
	const value_t new_value = { value, lifetime };
	Id index(index_arr);
	trace(20) << "Broker_impl::store(): storing value with index ID " <<
		index << " and lifetime " << lifetime << endm;

	seq_node_ref_t_var nodes = find_nodes(index);
	for(unsigned n = 0; n < nodes->length(); ++n)
	{
		Id node_id(nodes[n].id);
		try
		{
			nodes[n].ref->store(node_ref, index_arr, new_value);
			_node._ct.insert(node_id, nodes[n].ref, true);
			trace(25) << "Broker_impl::store(): succesfully stored value at node ID " <<
				node_id << endm;
		}
		catch(const CORBA::Exception &)
		{
			_node._ct.erase(node_id);
			trace(25) << "Broker_impl::store(): failed to store value at node ID " <<
				node_id << "; erased from contact table." << endm;
		}
	}
}
            
void Broker_impl::erase (
    const kademlia::id_t index,
    const CORBA::Any& value )
{
	store(index, value, 0);
}

seq_any_t* Broker_impl::retrieve (
    const kademlia::id_t index_arr )
{
	const node_ref_t& node_ref = _node.reference();
	Id index(index_arr);
	trace(20) << "Broker_impl::retrieve(): retrieving value for index:\n" <<
		index << endm;

	seq_any_t_var result = new seq_any_t();
	seq_node_ref_t_var nodes = find_nodes(index);
	for(unsigned n = 0; n < nodes->length() && n < replication_factor/5; ++n)
	{
		Id node_id(nodes[n].id);
		try
		{
			// Retrieve nodes at the given index
			seq_value_t_var values = nodes[n].ref->retrieve(node_ref, index_arr);
			_node._ct.insert(node_id, nodes[n].ref, true);
			trace(29) << "Broker_impl::retrieve(): succesfully retrieved values at node ID " <<
				node_id << endm;

			// Add all values to the result set
			for(unsigned m = 0; m < values->length(); ++m)
			{
				// Check wether the result value was already present
				unsigned o, results = result->length();
				for(o = 0; o < results; ++o)
					if(values[m].contents == result[m])
						break;
				if(o == results)
				{
					// Add value to result set
					result->length(results + 1);
					result[results] = values[m].contents;
				}
			}
		}
		catch(const CORBA::Exception &)
		{
			_node._ct.erase(node_id);
			trace(29) << "Broker_impl::retrieve(): failed to retrieve values at node ID " <<
				node_id << "; erased from contact table." << endm;
		}
	}
	return result._retn();
}


seq_node_ref_t* Broker_impl::find_nodes (
	const Id &target )
{
	trace(25) << "Broker_impl::find_nodes(): retrieving nodes for target:\n" << target << endm;
	const node_ref_t& node_ref = _node.reference();

	seq_node_ref_t_var result = _node._ct.retrieve(target);
	bool               queried [replication_factor];
	Id                 distance[replication_factor];

	trace(29) << result->length() << " nodes in local contact table" << endm;
	for(unsigned n = 0; n < result->length(); ++n)
	{
		queried [n] = false;
		distance[n] = _node._id ^ Id(result[n].id);
	}

	while(true)
	{
		// Find an unqueried nearby contact
		unsigned n;
		for(n = 0; n < result->length(); ++n)
			if(!queried[n])
				break;
		if(n == result->length())
			break;	// No unqueried contacts left; we're done.

		// Query a nearby contact, to find even closer ones.
		Id node_id(result[n].id);
		queried[n] = true;
		try
		{
			// Retrieve nodes from remote contact table
			seq_node_ref_t_var nodes = result[n].ref->find_nodes(node_ref, target);
			_node._ct.insert(node_id, nodes[n].ref, true);
			trace(29) << "Broker_impl::find_nodes(): found " << nodes->length() <<
				" nodes at node ID " << node_id << endm;

			// Insert nodes into result set
			for(unsigned n = 0; n < nodes->length(); ++n)
			{
				Id new_id(nodes[n].id), new_distance(new_id ^ _node._id);
				unsigned p;
				for(p = 0; p < result->length(); ++p)
					if(distance[p] == new_distance)
						goto skip_node;	// node was already present!
					else
					if(distance[p] > new_distance)
						break;	// node p is farther away, insert here.

				// Expand the list if it is not yet too large
				if(result->length() < replication_factor)
					result->length(result->length() + 1);

                // Make room for the new entry...
				for(unsigned m = p + 1; m < result->length(); ++m)
				{
					queried [m] = queried [m-1];
					distance[m] = distance[m-1];
					result  [m] = result  [m-1];
        		}

				// and insert it.
				queried [p] = false;
				distance[p] = new_distance;
				result  [p] = nodes[n];

			skip_node: ;
			}
		}
		catch(const CORBA::Exception &)
		{
			_node._ct.erase(node_id);
			trace(29) << "Broker_impl::find_nodes(): failed to find nodes at node ID " <<
				node_id << "; erased from contact table." << endm;
		}
	}

	if(result->length() < replication_factor)
		error() << "Found only " << result->length() << " nodes near ID " << target <<
			"; Kademlia node is not properly connected to the network!" << endm;

	return result._retn();
}
