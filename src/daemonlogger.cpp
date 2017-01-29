#include "daemonlogger.h"

using namespace daemonize;

logger::priority_t logger::emerge	(0);
logger::priority_t logger::alert	(1);
logger::priority_t logger::critical	(2);
logger::priority_t logger::error	(3);
logger::priority_t logger::warning	(4);
logger::priority_t logger::notice	(5);
logger::priority_t logger::info		(6);
logger::priority_t logger::debug	(7);

logger::logger(std::streambuf* buf)
	:std::ostream(buf)
{
}

logger::~logger()
{
	for (std::list<void(*)(logger*)>::iterator it = destruction_observers.begin();
			it != destruction_observers.end();
			it++)
	{
		(*it)(this);
	}
}

logger& logger::operator<<(logger::priority_t prio)
{
	priority(prio._priority);
	return *this;
}

int logger::priority()
{
	return _priority;
}
void logger::priority(int opts)
{
	_priority = opts;
}

logger::priority_t::priority_t(int prio)
	:_priority(prio)
{
}

void logger::register_destruction_observer(void(*observer)(logger*))
{
	*this << debug << "call to register_destruction_observer: " << observer << std::endl;
	this->destruction_observers.push_back(observer);
}

void logger::unregister_destruction_observer(void(*observer)(logger*))
{
	*this << debug << "call to unregister_destruction_observer: " << observer << std::endl;
	this->destruction_observers.remove(observer);
}

