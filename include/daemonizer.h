#ifndef __DAEMONIZE_DAEMONIZER_H
#define __DAEMONIZE_DAEMONIZER_H

#include <daemonlogger.h>
#include <daemon.h>

namespace daemonize
{
	void daemonizer(daemonize::daemon& daemon);
}

#endif /* __DAEMONIZE_DAEMONIZER_H */