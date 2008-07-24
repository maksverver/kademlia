#include "time.hh"

#ifdef __WIN32__

#include <sys/types.h>
#include <sys/timeb.h>

static mstime_t global_now()
{
    struct _timeb timebuffer;
    _ftime(&timebuffer);
    return timebuffer.time*1000 + timebuffer.millitm;
}
#else

// UNIX-specific code
#include <sys/time.h>
static mstime_t global_now()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000 + tv.tv_usec/1000;
}

#endif

static mstime_t started_at = global_now();

mstime_t now()
{
    return global_now() - started_at;
};
