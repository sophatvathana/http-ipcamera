#include <cstring>
#include <cstdlib>

#include "syslogger.h"

using namespace daemonize;

syslogbuf* syslogbuf::last(0);

syslog::syslog(const char* identity,
			   int options,
			   int facility,
			   int priority_mask,
			   int priority)
	:buffer(),
	 logger(&buffer)
{
	buffer.identity(identity);
	buffer.options(options);
	buffer.facility(facility);
	buffer.priority_mask(priority_mask);
	buffer.priority(priority);
}

bool syslog::log()
{
	return buffer.sync() == 0;
}

const char* syslog::identity()
{
	return buffer.identity();
}
void syslog::identity(const char *ident)
{
	buffer.identity(ident);
}

int syslog::options()
{
	return buffer.options();
}
void syslog::options(int opts)
{
	buffer.options(opts);
}

int syslog::facility()
{
	return buffer.facility();
}
void syslog::facility(int fac)
{
	buffer.facility(fac);
}

int syslog::priority_mask()
{
	return buffer.priority_mask();
}
void syslog::priority_mask(int opts)
{
	buffer.priority_mask(opts);
}

int syslog::priority()
{
	return buffer.priority();
}
void syslog::priority(int opts)
{
	buffer.priority(opts);
}

syslogbuf::syslogbuf()
	:_identity(0),
	 reopen(false)
{
}

syslogbuf::~syslogbuf()
{
	if (_identity != 0)
		std::free(_identity);
}

int syslogbuf::sync()
{
	if (std::stringbuf::sync() == -1)
		return -1;

	if (reopen || last == this)
	{
		::setlogmask(_priority_mask);
		::openlog(_identity, _options, _facility);
	}

	::syslog(_priority, "%s", str().c_str());

	reopen = false;
	last = this;

	str("");

	return 0;
}

const char* syslogbuf::identity()
{
	return _identity;
}
void syslogbuf::identity(const char* ident)
{
	if (_identity != 0)
		std::free(_identity);

	_identity = strdup(ident);
	reopen = true;
}

int syslogbuf::options()
{
	return _options;
}
void syslogbuf::options(int opts)
{
	_options = opts;
	reopen = true;
}

int syslogbuf::facility()
{
	return _facility;
}
void syslogbuf::facility(int opts)
{
	_facility = opts;
	reopen = true;
}

int syslogbuf::priority_mask()
{
	return _priority_mask;
}
void syslogbuf::priority_mask(int opts)
{
	_priority_mask = opts;
	reopen = true;
}

int syslogbuf::priority()
{
	return _priority;
}
void syslogbuf::priority(int opts)
{
	_priority = opts;
}

