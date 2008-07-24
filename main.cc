#include "main.hh"

#include <ctime>
#include <cstring>
#include <utility>
#include <string>
#include <vector>

#include <omnithread.h>

#include "kademlia.hh"
#include "Node.hh"
#include "Broker.hh"
#include "logging.hh"

CORBA::ORB_var orb;

Node_impl   *node_servant;
Broker_impl *broker_servant;

PortableServer::ObjectId objectid(const char *str)
{
	unsigned len = strlen(str);
	return PortableServer::ObjectId( len, len,
		reinterpret_cast<unsigned char*>((const_cast<char*>(str))) );
}

static void run_bootstrap_thread(void *contacts_ptr)
{
    const std::vector<std::string> &contacts =
        *static_cast<const std::vector<std::string>*>(contacts_ptr);
        
	// FIXME: Wait 3 secs so the ORB has time to start up. This is a dirty hack!
	omni_thread::self()->sleep(3);

	// Bootstrap node
    info() << "Bootstrapping local node ID " << node_servant->id() << endm;
    for(std::vector<std::string>::const_iterator i = contacts.begin();
        i != contacts.end(); ++i)
    {
    	node_servant->add_initial_contact(i->c_str());
    }

    if(!node_servant->initialize(*broker_servant))
	{
		error(true) << "Node initialization failed! Shutting down." << endm;
		orb->shutdown(true);
	}
}

static void initialize_servants()
{
    try {
    
        // Obtain a reference to the root POA
		PortableServer::POA_var root_poa;
		{
			CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
			root_poa = PortableServer::POA::_narrow(obj);
		}
        
		// Obtain a reference to the root POA manager
		PortableServer::POAManager_var poa_manager = root_poa->the_POAManager();
		
		// Create child POA for our servants
		PortableServer::POA_var child_poa;
		{	
			CORBA::PolicyList policies(3);
			policies.length(3);
			policies[0] = root_poa->create_lifespan_policy(PortableServer::PERSISTENT);
			policies[1] = root_poa->create_id_assignment_policy(PortableServer::USER_ID);
			policies[2] = root_poa->create_thread_policy(PortableServer::ORB_CTRL_MODEL);
			child_poa = root_poa->create_POA("Kademlia", poa_manager, policies);
		}

		// Create and activate servant objects
		node_servant   = new Node_impl();
		broker_servant = new Broker_impl(*node_servant);
		
		child_poa->activate_object_with_id(objectid("Node"),   node_servant),
        child_poa->activate_object_with_id(objectid("Broker"), broker_servant);

		// Print out object id's
        {
            CORBA::Object_var obj;
			CORBA::String_var ior;
            
            obj = node_servant->_this();
			ior = orb->object_to_string(obj); 
			info() << "Node servant can be reached at \n" << (const char*)ior << endm;

			obj = broker_servant->_this();
            ior = orb->object_to_string(obj);
			info() << "Broker servant can be reached at \n" << (const char*)ior << endm;
        }

		// Tell the POA manager to start accepting requests on its objects.
        poa_manager->activate();
       
    }
    catch(CORBA::SystemException &) {
		error(true) << "Caught CORBA::SystemException while initializing servants" << std::endl;
    }
    catch(CORBA::Exception &) {
        error(true) << "Caught CORBA::Exception while initializing servants" << std::endl;
    }
    catch(...) {
        error(true) << "Caught unknown exception while initializing servants" << std::endl;
    }
}

#ifdef __WIN32__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static SERVICE_STATUS        service_status; 
static SERVICE_STATUS_HANDLE h_server_status;

static bool install_service()
{
	SC_HANDLE h_manager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(!h_manager)
		return false;

	TCHAR filepath[2048];
	GetModuleFileName(NULL, &filepath[1], sizeof(filepath) - 1);
	filepath[0] = '"'; strcat(filepath, "\"");

	SC_HANDLE h_service = CreateService( h_manager, "Kademlia", "Kademlia",
		SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
		SERVICE_AUTO_START /* SERVICE_DEMAND_START */ , // auto start during beta!
		SERVICE_ERROR_NORMAL, filepath, NULL, NULL, NULL, NULL, NULL );

	if(!h_service)
		return false;

	CloseServiceHandle(h_service);
	return true;
}

static bool uninstall_service()
{
	SC_HANDLE h_manager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(!h_manager)
		return false;

	SC_HANDLE h_service = OpenService( h_manager, "Kademlia", SERVICE_ALL_ACCESS );
	if(!h_service)
		return false;

	if(DeleteService(h_service)==0)
		return false;

	CloseServiceHandle(h_service);
    return true;
}

static void WINAPI service_control_handler(DWORD opcode)
{
	switch(opcode)
	{
	case SERVICE_CONTROL_STOP: 
		service_status.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(h_server_status, &service_status);
		orb->shutdown(true);
	break;
	}
}

static void WINAPI service_main(DWORD argc, LPTSTR *argv)
{
	service_status.dwServiceType      = SERVICE_WIN32_OWN_PROCESS;
	service_status.dwCurrentState     = SERVICE_RUNNING;
	service_status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	h_server_status = RegisterServiceCtrlHandler( "Kademlia", service_control_handler );
	SetServiceStatus(h_server_status, &service_status);
};

static void run_service_thread(void *unused)
{
	SERVICE_TABLE_ENTRY service_table[] = {
		{ "Kademlia", service_main },
		{ NULL,       NULL } };

	StartServiceCtrlDispatcher(service_table);
	error() << "returned from StartServiceCtrlDispatcher() call" << endm;
}

#endif //def __WIN32__

int main(int argc, char* argv[])
{
	// Initialise the ORB
    orb = CORBA::ORB_init(argc, argv);
	trace_level(1000); // display all messages.

    std::vector<std::string> contacts;
    contacts.push_back("corbaloc::1.2@130.89.161.226:4200/%ffKademlia%00Node");

    // Process command line arguments
    for(int n = 1; n < argc; ++n)
        if(argv[n][0] == '-')
        {
#ifdef __WIN32__
            if(strcmp(argv[n], "-install") == 0)
            {
                if(install_service())
                    info() << "Service installation succeeded" << endm;
                else
                    error() << "Service installation failed" << endm;
            }
            else
            if(strcmp(argv[n], "-uninstall") == 0)
            {
                if(uninstall_service())
                    info() << "Service uninstallation succeeded" << endm;
                else
                    error() << "Service uninstallation failed" << endm;
            }
            else
#endif //def __WIN32__
               error() << "Unrecognized command line argument \"" << argv[n] << "\"" << endm;
        }           
        else
            contacts.push_back(argv[n]);
    
	info() << "Kademlia service starting" << endm;

#	ifdef __WIN32__
	omni_thread *service_thread = new omni_thread(run_service_thread);
	service_thread->start();
#	endif

    // Initialize Broker and Node servants
    initialize_servants();

	// Start the bootstrapping thread.
	omni_thread *bootstrap_thread = new omni_thread(run_bootstrap_thread, &contacts);
	bootstrap_thread->start();

    // Start running; only returns after ORB has been shut down.
    orb->run();
    
    // Exit cleanly.
	info() << "Kademlia service exiting" << endm;
	orb->destroy();
	delete node_servant;
	delete broker_servant;
	
}
