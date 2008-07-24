#ifndef ID_HH_INCLUDED
#define ID_HH_INCLUDED

#include "kademlia.hh"
#include <string>
#include <iostream>

class Id
{
public:

    typedef kademlia::id_t id_t;
    
    static const unsigned bits = sizeof(id_t)*8;

            
    static Id random( );

    static Id hash(
        const void *buffer,
        size_t length );

                    
    Id();
    
    Id(
        const Id &other );
        
    Id(
        const kademlia::id_t_slice * );

    
    bool str(
        const std::string str );
        
    std::string str( ) const;

        
    operator const kademlia::id_t & ( ) const;
    
    operator kademlia::id_t &( );

    
    bool operator == (
        const Id &other ) const;
    
	bool operator != (
        const Id &other ) const;
        
    bool operator < (
        const Id &other ) const;
        
    bool operator > (
        const Id &other ) const;
        
    bool operator <= (
        const Id &other ) const;
    
    bool operator >= (
        const Id &other ) const;
        
    
    Id operator ^ (
        const Id &other ) const;
        
    Id &operator ^= (
        const Id &other );
        
    
    Id &operator = (
        const Id &other );
        
    Id &operator = (
        const kademlia::id_t_slice *id );
        
    
    bool operator [] (
        unsigned bit ) const;
        
    unsigned bitscan( ) const;
    

private:

    // for bitstring access: msb comes first! (big-endian)
    id_t _id;
          
}; // class Id

inline Id::operator const kademlia::id_t &() const
{
    return _id;
}

inline Id::operator kademlia::id_t &()
{
    return _id;
}

inline bool Id::operator[](unsigned bit) const
{
    return ( _id[sizeof(_id) - 1 - (bit/8)] & (1 << (bit%8)) ) != 0;
}

std::ostream &operator<<(std::ostream &os, const Id &id);

#endif //ndef ID_HH_INCLUDED
