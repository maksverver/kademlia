#ifndef COMPARE_ANY_HH
#define COMPARE_ANY_HH

#include "kademlia.hh"

// Tests two CORBA::Any's for structural equality
bool operator==(const CORBA::Any &a, const CORBA::Any &b);

#endif //ndef COMPARE_ANY_HH
