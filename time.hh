#ifndef TIME_HH_INCLUDED
#define TIME_HH_INCLUDED

// Type is measured in milliseconds.
typedef unsigned long long mstime_t;

// Returns the real (wall clock) time elapsed since startup.
mstime_t now();

#endif //ndef TIME_HH_INCLUDED
