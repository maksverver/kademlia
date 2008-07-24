#include "Id.hh"

#include <string>
#include <cstdio>
#include <cstring>

#include "random.hh"
#include "main.hh"
#include "sha1.h"

Id Id::random( )
{
    Id result;
    for(unsigned n = 0; n < sizeof(result._id); ++n)
        result._id[n] = static_cast<CORBA::Octet>(randInt(255));
    return result;
}

Id Id::hash(
    const void *buffer,
    unsigned length )
{
    Id result;
    sha1_state_s state;
    sha1_init(&state);
    sha1_update(&state, reinterpret_cast<sha1_byte_t*>(const_cast<void*>(buffer)), length);
    sha1_finish(&state, result._id);
    return result;
}

Id::Id()
{
}

Id::Id(
    const kademlia::id_t_slice *id )
{
    *this = id;
}

Id::Id(
    const Id &other )
{
    *this = other;
}

bool Id::operator == (
    const Id &other ) const
{
    return std::memcmp(&_id, &other._id, sizeof(_id)) == 0;
}

bool Id::operator != (
    const Id &other ) const
{
    return std::memcmp(&_id, &other._id, sizeof(_id)) != 0;
}

bool Id::operator < (
    const Id &other ) const
{
    return std::memcmp(&_id, &other._id, sizeof(_id)) < 0;
}

bool Id::operator > (
    const Id &other ) const
{
    return std::memcmp(&_id, &other._id, sizeof(_id)) > 0;
}

bool Id::operator <= (
    const Id &other ) const
{
    return std::memcmp(&_id, &other._id, sizeof(_id)) <= 0;
}

bool Id::operator >= (
    const Id &other ) const
{
    return std::memcmp(&_id, &other._id, sizeof(_id)) >= 0;
}

Id Id::operator ^ (
    const Id &other ) const
{
    Id result = *this;
    result ^= other;
    return result;    
}

Id &Id::operator ^= (
    const Id &other )
{
    for(unsigned n = 0; n < sizeof(_id); ++n)
        _id[n] ^= other._id[n];
    return *this;
}

Id &Id::operator = (
    const Id &other )
{
    std::memcpy(&_id, &other._id, sizeof(_id));
    return *this;
}

Id &Id::operator = (
    const kademlia::id_t_slice *id )
{
    std::memcpy(&_id, id, sizeof(_id));
    return *this;
}

bool Id::str(
    const std::string str )
{
    if(str.size() != 2*sizeof(_id))
        return false;
        
    id_t new_id;
    for(unsigned n = 0; n < sizeof(_id); ++n)
    {
        const char *buffer = str.c_str();
        int digit;
        if(!std::sscanf(&buffer[2*n], "%02X", &digit))
            return false;
        new_id[n] = digit;
    }
    
    *this = (new_id);
    
    return true;
};

// Returns a hexadecimal string representation
std::string Id::str( ) const
{
    char buffer[2*sizeof(_id) + 1];
    for(unsigned n = 0; n < sizeof(_id); ++n)
        std::sprintf(&buffer[2*n], "%02X", (int)_id[n]);
    return std::string(buffer);
}

unsigned Id::bitscan() const
{
    unsigned n = bits;
    while(n--)
        if((*this)[n])
            return n+1;
    return 0;
}

std::ostream &operator<<(std::ostream &os, const Id &id)
{
	return (os << id.str());
}
