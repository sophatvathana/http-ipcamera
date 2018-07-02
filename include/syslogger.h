#ifndef __DAEMONIZE_SYSLOG_H
#define __DAEMONIZE_SYSLOG_H

#include <sstream>
#include <ostream>

#include <daemonlogger.h>

// This activates vsyslog in syslog.h
#ifndef __USE_BSD
#	define __USE_BSD
#endif

#include <syslog.h>

namespace daemonize
{
	class syslogbuf : public std::stringbuf
	{
		public:
			syslogbuf();
			virtual ~syslogbuf();

			const char* identity();
			void identity(const char* ident);

			int options();
			void options(int opts);

			int facility();
			void facility(int fac);

			int priority_mask();
			void priority_mask(int prio_mask);

			int priority();
			void priority(int prio);

		protected:
			virtual int sync();
			friend class syslog;

		private:
			char* _identity;
			int _options;
			int _facility;
			int _priority_mask;
			int _priority;

			bool reopen;
			// Keep track of which object did logging last,
			// and only run opening if needed.
			static syslogbuf* last;
	};

	class syslog : public daemonize::logger
	{
		public:
			syslog(const char* identity,
				   int options = LOG_CONS | LOG_PID | LOG_NDELAY,
				   int facility = LOG_LOCAL0,
				   int priority_mask = 0,
				   int default_priority = LOG_ERR);

			const char* identity();
			void identity(const char* ident);

			int options();
			void options(int opts);

			int facility();
			void facility(int fac);

			int priority_mask();
			void priority_mask(int prio_mask);

			virtual int priority();
			virtual void priority(int prio);

			virtual bool log();

		private:
			syslogbuf buffer;
	};
}

#endif /* __DAEMONIZE_SYSLOG_H */

