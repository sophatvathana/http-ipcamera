#ifndef __DAEMONIZE_DAEMON_H
#define __DAEMONIZE_DAEMON_H

#include <daemonlogger.h>

namespace daemonize
{
	class daemon
	{
		public:
			// Throws if pid file can't be opened or locked
			daemon(daemonize::logger& log,
					const std::string& pid_file,
					const std::string& run_dir = "/");
			virtual ~daemon();

			virtual void run() = 0;

			daemonize::logger& get_log();

			const std::string& pid_file() const;
			const std::string& run_dir() const;
			int pid() const;

			void get_pid();

		protected:
			daemonize::logger& log;

		private:
			int _pid;
			int	pid_file_descriptor;
			std::string _pid_file;
			std::string _run_dir;
	};
}

#endif /* __DAEMONIZE_DAEMON_H */

