#include "compare_any.hh"
#include <omnithread.h>

extern CORBA::ORB_var orb;
static DynamicAny::DynAnyFactory_var daf;
static omni_mutex daf_mutex;

bool operator==(const CORBA::Any &a, const CORBA::Any &b)
{
	{
		omni_mutex_lock l(daf_mutex);
		if(CORBA::is_nil(daf))
		{
			CORBA::Object_var obj = orb->resolve_initial_references("DynAnyFactory");
			daf = DynamicAny::DynAnyFactory::_narrow(obj);
			
			if(CORBA::is_nil(daf))
				throw "DynAnyFactory reference was nil";
		}
	}
	
	CORBA::TypeCode_var ta = a.type(), tb = b.type();
	if(ta->kind() == CORBA::tk_null || tb->kind() == CORBA::tk_null)
		return ta->kind() == tb->kind();

	bool result;
	DynamicAny::DynAny_var da, db;
	try
	{
		da = daf->create_dyn_any(a),
		db = daf->create_dyn_any(b);
		result = da->equal(db);
	}
	catch (const DynamicAny::DynAnyFactory::InconsistentTypeCode &itc)
	{
	        if(!CORBA::is_nil(da))
			da->destroy();
		if(!CORBA::is_nil(db))
			db->destroy();
		throw itc;
	}
	da->destroy();
	db->destroy();

	return result;
}
