#include "logging.hh"

#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <vector>

#include <omnithread.h>

/*
	Local type declarations.
*/

class ologstream : public std::ostringstream
{
public:
	enum type_t { error, info, trace } type;
	union {
		bool           fatal;
		unsigned short level;
	};
};

/*
	Global variables.
*/

// End-of-message marker.
const class endm_t { } endm = { };

// Stream pool with mutex.
static std::vector<ologstream *> stream_pool;
static omni_mutex stream_pool_mutex;

// Mutex for controlling console output.
static omni_mutex console_mutex;

// The current trace level (not synchronized!).
static unsigned short _trace_level = 0;


#ifdef __WIN32__  // Windows NT event logging

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static class Logger {
private:
	HANDLE     _handle;

public:
    Logger()
	{
		_handle = RegisterEventSource(NULL, "Kademlia");
	};

	~Logger()
	{
		if(_handle)
			DeregisterEventSource(_handle);
	};

	bool valid()
	{
		return (_handle != NULL);
	};

	bool report(
		const ologstream &ols )
	{
		WORD type = ( (ols.type == ologstream::error) ?
			(ols.fatal ? EVENTLOG_ERROR_TYPE : EVENTLOG_WARNING_TYPE) :
			EVENTLOG_INFORMATION_TYPE );
		std::string text = "\n\n"; text += ols.str();
		LPCSTR messages[] = { text.c_str(), NULL };
		return ReportEvent( _handle, type, 0, 0, NULL,
			1, 0, messages, NULL ) != FALSE;
	}

} logger;
#else

#if !defined(NOSYSLOG)  // UNIX-style syslog

#include <syslog.h>
#include <stdarg.h>

static class Logger
{
public:
	Logger()
	{
		openlog("kademlia", LOG_CONS, LOG_USER);
	}
	
	~Logger()
	{
		closelog();
	}

	bool valid()
	{
		return true;
	}
	
	bool report( 
		const ologstream &ols )
	{
		syslog(
			(ols.type == ologstream::error) ? (ols.fatal ? LOG_ERR : LOG_WARNING ) : LOG_INFO,
			"%s", ols.str().c_str() );
		return true;
	}
	
} logger;

	// TODO: syslog implementation

#else  // No system specific logging facility available.
static struct Logger
{
	bool valid()
	{
		return false;
	}

	bool report(
		const ologstream &ols )
	{
		return false;
	};
	
} logger;
#endif // !defined(NOSYSLOG)
#endif // def __WIN32__


static ologstream *acquire_stream()
{
    omni_mutex_lock l(stream_pool_mutex);
	if(stream_pool.empty())
	{
		// Allocate a new stream object
		return new ologstream();
	}
	else
	{
		// Take an stream object from the pool
		ologstream *result = stream_pool.back();
		stream_pool.pop_back();
		return result;
	}
}

static void release_stream(ologstream *stream)
{
	// Reset the stream object to it's initial state, so it is ready for reuse.
	stream->str("");
	stream->clear();

	// Add the stream object to the pool
	omni_mutex_lock l(stream_pool_mutex);
	stream_pool.push_back(stream);
}

std::ostream &error(
	bool fatal)
{
	ologstream *ols = acquire_stream();
	ols->type  = ologstream::error;
	ols->fatal = fatal;
	return *ols;
}

std::ostream &info( )
{
	ologstream *ols = acquire_stream();
	ols->type  = ologstream::info;
	return *ols;
}

std::ostream &trace(
	unsigned short level )
{
	ologstream *ols = acquire_stream();
	ols->type  = ologstream::trace;
	ols->level = level;
	return *ols;
}

void operator<<(std::ostream &os, const endm_t &endm)
{
	// Beware: this better be a real ologstream object, or things will go wrong!
	ologstream *ols = reinterpret_cast<ologstream*>(&os);

	if((ols->type != ologstream::trace) || (ols->level < _trace_level))
		switch(ols->type)
		{
		case ologstream::error:
		case ologstream::info:
			if(logger.valid() && logger.report(*ols) && (_trace_level == 0))
				break;

		default:
			// Extract log message from stream.
			const std::string &message = ols->str();

			// Select destination stream
			std::ostream &dst = ((ols->type == ologstream::error) ? std::cerr : std::cout);
			omni_mutex_lock l( console_mutex );

			// Write timestamp
			time_t t = time(NULL);
			char *time_buf = ctime(&t);
			dst << '['; dst.write(time_buf, 24); dst << "] ";

			// Write message type
			if(ols->type == ologstream::trace)
				dst << std::setw(3) << ols->level << ' ';
			else
			if(ols->type == ologstream::error)
				dst << (ols->fatal ? "FAT " : "ERR ");
			else
				dst << "--> ";

			// Write the actual message
			dst << message << std::endl;
		}

	// Release stream so it can be used by other threads.
	release_stream(ols);
}

unsigned short trace_level( )
{
	return _trace_level;
}

void trace_level(
	unsigned short level )
{
	_trace_level = level;
}
