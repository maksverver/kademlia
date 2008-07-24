#ifndef RANDOM_HH_INCLUDED
#define RANDOM_HH_INCLUDED

/*
Functions for portably generating high quality random numbers. No
initialization is required. The returned numbers reflect the random number
generator's internal state and are therefore not suitable for application
for cryptographic purposes.
*/

/*
	Returns a random real number between 0 and 1 (inclusive).
*/
double randDouble( );

/*
	Returns a random real number between 0 and n (inclusive).
*/
double randDouble(
	const double n );

/*
	Returns a random real number between 0 and 1 (exclusive).
*/
double randDoubleExcl( );

/*
	Returns a random real number between 0 and n (exclusive).
*/
double randDoubleExcl(
	const double n );

/*
	Returns a random integer between 0 and 2^32-1 (inclusive).
*/
unsigned long randInt( );

/*
	Returns a random integer between 0 and n (inclusive).
*/
unsigned long randInt(
	unsigned long n );

/*
	Returns a random real number from a normal distribution with the given
	parameters for mean and variance.
*/
double randNorm(
	double mean,
	double variance );

#endif //ndef RANDOM_HH_INCLUDED
