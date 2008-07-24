#include "random.hh"
#include "MersenneTwister.hh"
#include "omnithread.h"

static MTRand     rng;
static omni_mutex mutex;

double randDouble( )
{
	omni_mutex_lock lock(mutex);
	return rng.rand53();
}

double randDouble(
	const double n )
{
	omni_mutex_lock lock(mutex);
	return rng.rand(n);
}

double randDoubleExcl( )
{
	omni_mutex_lock lock(mutex);
	return rng.randExc();
}

double randDoubleExcl(
	const double n )
{
	omni_mutex_lock lock(mutex);
	return rng.randExc(n);
}

unsigned long randInt( )
{
	omni_mutex_lock lock(mutex);
	return rng.randInt();
}

unsigned long randInt(
	unsigned long n )
{
	omni_mutex_lock lock(mutex);
	return rng.randInt(n);
}

double randNorm(
	double mean,
	double variance )
{
	omni_mutex_lock lock(mutex);
	return rng.randNorm(mean, variance);
}
