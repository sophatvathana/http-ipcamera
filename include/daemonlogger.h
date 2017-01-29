#ifndef __DAEMONIZE_LOGGER_H
#define __DAEMONIZE_LOGGER_H

#include <ostream>
#include <functional>
#include <list>

namespace daemonize
{
	class logger : public std::ostream
	{
		public:
			logger(std::streambuf* buf);
			virtual ~logger();

			// must sync()
			virtual bool log() = 0;

			virtual int priority();
			virtual void priority(int prio);


			struct priority_t
			{
				public:
					priority_t(int prio);

					int _priority;
			};

			// Values for priority defined in logger.cpp
			// are taken from syslog.h
			// You can updated these values if you use another
			// log system.
			static priority_t emerge;
			static priority_t alert;
			static priority_t critical;
			static priority_t error;
			static priority_t warning;
			static priority_t notice;
			static priority_t info;
			static priority_t debug;

			logger& operator<<(priority_t prio);

			void register_destruction_observer(void(*observer)(logger*));
			void unregister_destruction_observer(void(*observer)(logger*));

		private:
			int _priority;
			std::list<void(*)(logger*)> destruction_observers;
	};
}

#endif /* __DAEMONIZE_LOGGER_H */

