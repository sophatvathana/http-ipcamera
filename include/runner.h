#ifndef RUNNER_H
#define RUNNER_H
#include <daemon.h>
class Runner:daemonize::daemon{
	public:
		Runner();
		~Runner();
		virtual void run(){
        	daemonize::daemon::run();
    	}
};
#endif