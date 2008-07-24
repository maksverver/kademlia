#include <omniORB4/CORBA.h>
#include <iostream>
#include <cstring>
#include "logging.hh"
using namespace std;

CORBA::ORB_var orb;

#include <omnithread.h>

void test_thread()
{
	omni_mutex mutex;
	omni_mutex_lock lock(mutex);
}


#include "Id.hh"

void test_Id()
{
    cout << "Testing Id's..." << endl;
    Id a = Id::random(), b = Id::random(), c;
    cout << "A: " << a.str() << endl;
    cout << "B: " << b.str() << endl;
    c = a; a = b; b = c;
    cout << "Swapped A and B..." << endl;
    cout << "A: " << a.str() << endl;
    cout << "B: " << b.str() << endl;
    cout << "C: " << c.str() << endl;
    cout << "A==C: " << (a==c) << endl;
    cout << "B==C: " << (b==c) << endl;
    cout << "A<B: " << (a<b) << endl;
    cout << "B<A: " << (b<a) << endl;
    cout << "A^B: " << (a^b).str() << endl;
    cout << "B^A: " << (b^a).str() << endl;

    c.str("0123456789ABCDEF0123456789ABCDEF01234567");
    cout << "Bits for " << c.str() << ": ";
    {
        unsigned n = c.bits;
        while(--n)
            cout << (c[n] ? '1' : '0');
    }
    
    cout << endl;

    cout << "bitscan of A^B: " << (a^b).bitscan() << endl;
    c.str("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");    // 160
    cout << "bitscan of " << c.str() << ": " << c.bitscan() << endl;
    c.str("0000000000000000000000000000000000000000");    // 0
    cout << "bitscan of " << c.str() << ": " << c.bitscan() << endl;
    c.str("0000000000000000000000000000000000000001");    // 1
    cout << "bitscan of " << c.str() << ": " << c.bitscan() << endl;
    c.str("8080808080808080808080808080808080808080");    // 160
    cout << "bitscan of " << c.str() << ": " << c.bitscan() << endl;
    c.str("4080808080808080808080808080808080808080");    // 159
    cout << "bitscan of " << c.str() << ": " << c.bitscan() << endl;
    c.str("00000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");    // 128
    cout << "bitscan of " << c.str() << ": " << c.bitscan() << endl;
    c.str("00000000000000000000FFFFFFFFFFFFFFFFFFFF");    // 80    
    cout << endl;
}


#include "time.hh"

void test_time()
{
    string line;
    cout << "Testing time functions..." << endl;
    cout << "Current time in seconds: " << now()/1000.0 << " (press enter) " << flush;
    getline(cin, line);
    cout << "Current time in seconds: " << now()/1000.0 << " (press enter) " << flush;
    getline(cin, line);
    cout << "Current time in seconds: " << now()/1000.0 << endl;
    cout << endl;
}


#include "DataTable.hh"

using namespace kademlia;

void test_DataTable()
{
    DataTable dt;
    
    Id index;
    CORBA::Any value;
    
    cout << "Storing 'fooKey' ==> 123 (expires in 10 seconds)" << endl;
    index = Id::hash("fooKey", strlen("fooKey")); value <<= (long)123;
    dt.store(index, value, 10*1000);
    cout << "Storing 'barKey' ==> 555 (expires in 10 seconds)" << endl;
    index = Id::hash("barKey", strlen("barKey")); value <<= (long)555;
    dt.store(index, value, 10*1000);
    cout << "Storing 'fooKey' ==> 345 (expires in 1 second)" << endl;
    index = Id::hash("fooKey", strlen("fooKey")); value <<= (long)345;
    dt.store(index, value, 1*1000);
    cout << "Storing 'barKey' ==> 666 (expires in 50 seconds)" << endl;
    index = Id::hash("barKey", strlen("barKey")); value <<= (long)666;
    dt.store(index, value, 50*1000);
    cout << "Storing 'barKey' ==> 666 (expires in 5 seconds)" << endl;
    index = Id::hash("barKey", strlen("barKey")); value <<= (long)666;
    dt.store(index, value, 5*1000);
   
    cout << "Delaying... " << " (press enter) " << flush;
    string line; getline(cin, line);
    cout << dt.purge() << " entries purged!" << endl;
    
    for(int n = 0; n < 3; ++n)
    {
        const char *keys[] = { "fooKey", "barKey", "bazKey" };
        cout << "Retrieving entries for " << keys[n] << ":" << endl;
        index = Id::hash(keys[n], strlen(keys[n]));
    
        kademlia::seq_value_t_var values;
        values = dt.retrieve(index);
        for(unsigned n = 0; n < values->length(); ++n)
        {
            long number;
            if( values[n].contents >>= number)
                cout << "\t" << number;
            else
                cout << "\tValue extraction failed!";
            cout << " (lifetime: " << values[n].lifetime << ")" << endl;
        }
    }
    
}


#include "ContactTable.hh"

void test_ContactTable()
{
    Id origin = Id::random();
    cout << "Origin ID: " << origin.str() << endl;
    ContactTable ct(origin);

    cout << "Updating 50 ID's:" << endl;
    for(unsigned n = 0; n < 50; ++n)
    {
        Id id = Id::random();
        cout << "\t" << n << "\t" << id.str() << endl;
        ct.insert(id, Node::_nil());
    }
    
    {
        Id id = Id::random();
        cout << "Retrieving nearest nodes for ID: " << id.str() << endl;
        seq_node_ref_t_var nodes = ct.retrieve(id);
        for(unsigned n = 0; n < nodes->length(); ++n)
            cout << "\t" << n << "\t" <<Id(nodes[n].id).str() << endl;
    }
    
    cout << endl;
}

int main(int argc, char *argv[])
{
	orb = CORBA::ORB_init(argc, argv);
	trace_level(1000);
	test_thread();
	test_Id();
    test_time();
    test_DataTable();
    test_ContactTable();
}
