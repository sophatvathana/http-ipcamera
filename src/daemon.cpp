#include "daemon.h"
#include <stdexcept>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cstdio>

using namespace daemonize;

daemon::daemon(logger& log,
		const std::string& pid_file,
		const std::string& run_dir)
	:log(log),
	 _pid(0),
	 pid_file_descriptor(0),
	 _pid_file(pid_file),
	 _run_dir(run_dir)
{
	pid_file_descriptor = open(_pid_file.c_str(), O_RDWR | O_CREAT, 0600);

	log << log.debug << "Opening pid file \""
		<< _pid_file << "\"" << std::endl;
	if (pid_file_descriptor == -1)
	{
		log << log.alert << "Unable to open pid file \""
		   << _pid_file << "\""	<< std::endl;
		throw std::runtime_error("Unable to open pid file");
	}

	if (lockf(pid_file_descriptor, F_TLOCK, 0) != 0)
	{
		log << log.alert << "Unable to lock pid file \""
		   << _pid_file << "\""	<< std::endl;
		throw std::runtime_error("Unable to lock pid file");
	}
}

daemon::~daemon()
{
	if (pid_file_descriptor != 0)
	{
		log << logger::notice << "daemonize::daemon::~daemon closing pid file" << std::endl;
		close(pid_file_descriptor);
	}
}

void daemon::get_pid()
{
	_pid = getpid();
	char buffer[10];
	sprintf(buffer, "%d\n", _pid);
	write(pid_file_descriptor, buffer, strlen(buffer));
}

logger& daemon::get_log()
{
	return log;
}

const std::string& daemon::pid_file() const
{
	return _pid_file;
}

const std::string& daemon::run_dir() const
{
	return _run_dir;
}

int daemon::pid() const
{
	return _pid;
}

