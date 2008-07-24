#ifndef LOGGING_HH_INCLUDED
#define LOGGING_HH_INCLUDED

#include <iostream>

/*
	Thread-safe portable logging functions.
*/

class endm_t;
extern const endm_t endm;

/*
	Returns an output stream to print errors to. Use this to display messages
	about events that prevent the application from performing its normal
	functions. Set the 'fatal' argument to true only if the application will
	terminate soon after the error has occured.
*/
std::ostream &error(
	bool fatal = false );

/*
	Returns an output stream to print informative messages to. Use this to
	display information about the normal operation of the application, as far
	as it is relevant to the end user of the application. Do no write to this
	stream too often.
*/
std::ostream &info( );

/*
	Returns an output stream to print debug tracing message to. Use this to
	display any information about the internal state of the application that
	might be relevant for the evaluation of the application's performance.
	Writing to this stream frequently is ok.

	Trace levels:
		1 -10	Top-level (one-time per execution) messages
		10-20	Long-living object lifetime messsages
				10	Object construction/destruction
				15	Relevant method invocations
		20-30	Functional debugging
		30+		Memory debugging
*/
std::ostream &trace(
	unsigned short level = 100 );

/*
	Send the end-of-message-marker endm to a stream to finish a log message
	and have it send to the appropriate place (such as the console or the
	system logs).
*/
void operator<<(std::ostream &os, const endm_t &endm);

/*
	Get the current trace level; a higher value means getting more trace
	messages. The default trace level is 0. (Note: not thread safe!)
*/
unsigned short trace_level( );

/*
	Set the trace level. Although error and info messages are always displayed,
	trace messages with a trace level equal to or above the current trace level
	are silently dropped. (Note: not thread safe!)
*/
void trace_level(
	unsigned short level );

#endif //ndef LOGGING_HH_INCLUDED
