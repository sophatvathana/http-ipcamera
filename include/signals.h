#ifndef __DAEMONIZE_SIGNALS_H
#define __DAEMONIZE_SIGNALS_H

#include <daemonlogger.h>

namespace daemonize
{
	namespace signals
	{
		void enable(daemonize::logger*);
		void disable();

		void handler(int signal);
	}
}

#endif