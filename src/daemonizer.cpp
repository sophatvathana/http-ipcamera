#include "daemonizer.h"

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

void daemonize::daemonizer(daemonize::daemon& daemon)
{
	daemon.get_log() << logger::notice << "Starting..." << std::endl;

	/* Fork off the parent process */
	pid_t pid = fork();	// child process get 0 as return value
	if (pid < 0)
	{
		daemon.get_log() << logger::error << "Unable to fork off the parent process: %m" << std::endl;
		throw EXIT_FAILURE;
	}
	/* If we got a good PID, then
	 * we can exit the parent process. */
	if (pid > 0)
	{
		daemon.get_log() << logger::debug << "Exiting parent process" << std::endl;
		throw EXIT_SUCCESS;
	}

	daemon.get_log() << logger::debug << "Change file mode umask to 0" << std::endl;
	umask(0);

	daemon.get_log() << logger::debug << "Create a new SID for the child process" << std::endl;
	pid_t sid = setsid();
	if (sid < 0)
	{
		daemon.get_log() << logger::error << "Unable to create a new SID for the child process: %m" << std::endl;
		throw EXIT_FAILURE;
	}

	daemon.get_log() << logger::debug << "Change current working directory to \"" << daemon.run_dir() << "\"" << std::endl;
	if ((chdir(daemon.run_dir().c_str())) < 0)
	{
		daemon.get_log() << logger::error << "Unable to change the current working directory: %m" << std::endl;
		throw EXIT_FAILURE;
	}

	daemon.get_log() << logger::debug << "Closing standard file descriptor STDIN" << std::endl;
	if (close(STDIN_FILENO) < 0)
	{
		daemon.get_log() << logger::error << "Unable to close standard file descriptor STDIN: %m" << std::endl;
		throw EXIT_FAILURE;
	}
	daemon.get_log() << logger::debug << "Closing standard file descriptor STDOUT" << std::endl;
	if (close(STDOUT_FILENO) < 0)
	{
		daemon.get_log() << logger::error << "Unable to close standard file descriptor STDOUT: %m" << std::endl;
		throw EXIT_FAILURE;
	}
	daemon.get_log() << logger::debug << "Closing standard file descriptor STDERR" << std::endl;
	if (close(STDERR_FILENO) < 0)
	{
		daemon.get_log() << logger::error << "Unable to close standard file descriptor STDERR: %m" << std::endl;
		throw EXIT_FAILURE;
	}

	daemon.get_pid();

	daemon.get_log() << logger::notice << "Launching daemon" << std::endl;
	daemon.run();
}

