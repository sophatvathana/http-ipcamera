/**
 * signals
 * Author:		Geoffroy Planquart <geoffroy@aethelflaed.com>
 * Created:		January 23 2013
 * Last Change:	January 23 2013
 */

#include "signals.h"
#include <csignal>
#include <cstdlib>


namespace daemonize
{
	namespace signals
	{
		daemonize::logger* logger(0);

		void reset_logger(daemonize::logger* _logger)
		{
			if (logger == _logger)
			{
				logger = 0;
			}
		}
	}
}


void daemonize::signals::enable(daemonize::logger* _logger)
{
	if (logger != 0 && _logger != logger)
	{
		logger = 0;
		logger->unregister_destruction_observer(daemonize::signals::reset_logger);
	}
	if (_logger != 0)
	{
		logger = _logger;
		*logger << logger->info << "daemonize::signals::enable: " << _logger << std::endl;
		logger->register_destruction_observer(daemonize::signals::reset_logger);
	}

	sigset_t sigset;
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGCHLD);			// ignore child
	sigaddset(&sigset, SIGTSTP);			// Tty stop signal
	sigaddset(&sigset, SIGTTOU);			// Tty background writes
	sigaddset(&sigset, SIGTTIN);			// Tty background reads
	pthread_sigmask(SIG_BLOCK, &sigset, 0);	// block above signals

	struct sigaction action;
	sigemptyset(&action.sa_mask);
	action.sa_handler = daemonize::signals::handler;
	action.sa_flags = 0;

	sigaction(SIGHUP,	&action, 0);
	sigaction(SIGINT,	&action, 0);
	sigaction(SIGTERM,	&action, 0);
	sigaction(SIGFPE,	&action, 0);
	sigaction(SIGILL,	&action, 0);
	sigaction(SIGSEGV,	&action, 0);
}

void daemonize::signals::disable()
{
	if (logger != 0)
	{
		*logger << logger->info << "daemonize::signals::disable" << std::endl;
		logger->unregister_destruction_observer(daemonize::signals::reset_logger);
		logger = 0;
	}
}

void daemonize::signals::handler(int signal)
{
	switch(signal)
	{
		case SIGHUP:
			*logger << logger->notice	<< "daemonize::signals: SIGHUP" << std::endl;
			exit(EXIT_SUCCESS);
		case SIGINT:
			*logger << logger->notice	<< "daemonize::signals: SIGINT" << std::endl;
			exit(EXIT_SUCCESS);
		case SIGTERM:
			*logger << logger->notice	<< "daemonize::signals: SIGTERM" << std::endl;
			exit(EXIT_SUCCESS);
		case SIGFPE:
			*logger << logger->error	<< "daemonize::signals: SIGFPE" << std::endl;
			abort();
		case SIGILL:
			*logger << logger->error	<< "daemonize::signals: SIGILL" << std::endl;
			abort();
		case SIGSEGV:
			*logger << logger->error	<< "daemonize::signals: SIGSEGV" << std::endl;
			abort();
		default:
			*logger << logger->warning	<< "daemonize::signals: unknown signal caught: " << signal << std::endl;
			break;
	}
}

