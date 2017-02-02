/*
* @Author: sophatvathana
* @Date:   2017-01-12 13:14:01
* @Last Modified by:   sophatvathana
* @Last Modified time: 2017-01-12 13:17:02
*/
#include "SonaHttp.h"
#include "utils.h"
#include <fstream>
#include <chrono>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <strmrecvclientapi.h>
#include <regex>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>

#define OPT1 "Operation1"
#define OPT2 "Operation2"

struct global_args_t {
    const char *program_name;
    const char *config_file;
    const char *operation_name;
    int verbose;
} _args;

#define TEST_ADDRESS "rtsp://admin:12345@192.168.0.38/Streaming/Channels/102"
//"rtsp://76.89.206.161/live3.sdp"
//"rtsp://76.89.206.161/live3.sdp"
//"rtsp://admin:12345@192.168.0.38/Streaming/Channels/1"
#define TEST_FRAME_NUM 100000000
#define TEST_FRAME_PER_LOOP 2//20
#define SOCK_PATH "echo_socket"
using namespace SonaHttp;

#define DAEMON_NAME "strmrecv"

void process(){
    syslog (LOG_NOTICE, "Writing to my Syslog");
}   


const char HEAD_RESPONSE[] =
{
    "HTTP/1.1 200 OK\r\n" 
    "Server: sonadev.com.kh sona server\r\n" 
    "Cache-Control: no-cache\r\n" 
    "Cache-Control: private\r\n" 
    "Content-Type: multipart/x-mixed-replace;boundary=myboundary\r\n\r\n"
    "--myboundary\r\n"
};

const char SEPARATOR[] =
{
    "\r\n"
  "--myboundary\r\n"
};

int  t = false;
struct StopClient : public RequestHandler {
  void handleRequest(RequestPtr req, ResponsePtr rep) override {
    int clientId = std::stoi((char*)SonaHttp::getParam(req->getQueryString(), "id").c_str());
    int de = std::stoi((char*)SonaHttp::getParam(req->getQueryString(), "rm").c_str());
      strmrecvclient_stop(clientId);
    if(de == 1){
      strmrecvclient_destroy(clientId);
    }
  }
};

struct HelloWorldHandler : public RequestHandler {
	void handleRequest(RequestPtr req, ResponsePtr rep) override {
          auto search = req->getQueryString();
          std::string streamAddr = "";
          std::string address = SonaHttp::getByPattern(search, "(\\baddr=(((?!&).)*))",2);
          if(address.size() == 0){
            rep->setStatus(403);
            rep->setMessage("Error hery");
            rep->flush();
            return;
          } 

          std::string domain = address.substr(address.find("://",0)+3, address.size());
          if((int)domain.find("@", 0) < 0){
            if((int)domain.find_last_of(":") < 0){
              streamAddr = domain.substr(0,domain.find_last_of("/"));
            }
            else
              streamAddr = domain.substr(0,domain.find_last_of(":"));
          }else{
            if((int)domain.find_last_of(":") < 0){
              streamAddr = domain.substr(domain.find("@", 0)+1,domain.find_last_of("/")-10);
            }
            else
              streamAddr = domain.substr(domain.find("@", 0)+1,domain.find_last_of(":")-10);
          }

          printf("%s\n", streamAddr.c_str());
          //std::string streamAddr = SonaHttp::getByPattern(address,"([^@:,\\/rtsphttp]+[1-9a-zA-Z])",1);
          std::string::size_type loc = streamAddr.find( ".", 0 );
          if(SonaHttp::getByPattern(streamAddr, "([a-zA-Z])", 0).size() == 0){
            if(!SonaHttp::isValidIPAddress(streamAddr)){
              rep->setStatus(403);
              rep->setMessage("Error hery");
              rep->flush();
              return;
            }
          }else{
            if(!SonaHttp::isDomainMatch(streamAddr, streamAddr.substr(loc, streamAddr.size()))){
              rep->setStatus(403);
              rep->setMessage("Error hery");
              rep->flush();
              return;
            }
          }
          if (streamAddr.size() != 0){
            printf("This is stream address: %s\n", streamAddr.c_str());
          }
          //rep->
          //std::terminate();
            int clientId = std::stoi((char*)SonaHttp::getParam(req->getQueryString(), "id").c_str());
            //strmrecvclient_stop(clientId);
            std::thread thread_stream([req, rep, clientId, address]{
                      const char * addr = address.c_str();
                      rep->write((char *)HEAD_RESPONSE);
                      strmrecvclient_start_log("","");
                    
                      // if(strmrecvclient_get_address(clientId).size() != 0){
                      //   printf("Should work\n");
                      //   printf("New: %s Old: %s\n",addr, strmrecvclient_get_address(clientId).c_str());
                      //   if (strmrecvclient_get_address(clientId).compare(addr) !=0){
                      //     printf("1202 This is different");
                      //     //strmrecvclient_stop(clientId);
                      //     strmrecvclient_destroy(clientId);
                      //     //strmrecvclient_start(clientId, addr, 1);
                      //   }
                      // }
                      strmrecvclient_start(clientId, addr, 1);
                      STRMRECVClientData *data = new STRMRECVClientData();
                      int t = 0;
                      while(1){
                        //strmrecvclient_stop_log();
                        strmrecvclient_log_state(clientId);
                        data->state = strmrecvclient_get_state(clientId);
                        
                        if (data->state == STRMRECVCLIENT_STATE_ERROR){
                            strmrecvclient_start(clientId, addr, 1);
                            continue;
                        }

                        if (data->state != STRMRECVCLIENT_STATE_LOOPING){
                            if (data->state < STRMRECVCLIENT_STATE_INITIALIZING && data->state != STRMRECVCLIENT_STATE_STOPPING){
                              strmrecvclient_start(clientId, addr, 1);
                            }
                            std::this_thread::sleep_for(std::chrono::milliseconds(180));

                            continue;
                        }


       if (strmrecvclient_wait(clientId) != 0)
            continue;

        strmrecvclient_get_data(clientId, TEST_FRAME_PER_LOOP, 0, data);

        strmrecvclient_resume(clientId);
  
        for (int i = 0; i < data->framesRead; i++)
        {
            if (data->frameSizes[i] < 1024){
                continue;
            }
            t++;  
              char buf1[1024] = {0};
              sprintf(buf1,"HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\nCache-Control : no-cache, private\r\nContent-Length : %d\r\n\r\n", data->frameSizes[i]);
              rep->raw_write(buf1, strlen(buf1));
              fflush(stdout);
              rep->raw_write((char * ) &data->frameQueue[i * STRMRECVCLIENT_FRAME_BUFFER_SIZE], data->frameSizes[i]);
              fflush(stdout);
              rep->raw_write((char*)SEPARATOR, strlen(SEPARATOR));
              fflush(stdout);
              
        }
         std::this_thread::sleep_for(std::chrono::milliseconds(180));
      }
         //  rep->flush();
            strmrecvclient_stop(clientId);
         // strmrecvclient_destroy(clientId);
            delete data;
            strmrecvclient_stop_log();
        });
          //printf("%d\n", std::this_thread::get_id());
          //thread_stream.join();
          thread_stream.detach();
          //exit(0);
	}
};
void runner(){
  char* conf = "{\"http port\":\"9000\"}";
  std::stringstream config(conf);
  Server server(config);
        server.addHandler("/test", new HelloWorldHandler());
        server.addHandler("/stop", new StopClient());
        //std::thread thread_runner([&server]{
          server.run(1);
        //});
        // std::this_thread::sleep_for(std::chrono::milliseconds(500));
        // thread_runner.join();
}

bool daemonize() {
  pid_t pid = fork();
  if (pid == -1) {
    std::stringstream ss;
    ss << "Could not become a daemon: fork #1 failed: " << errno;
    throw std::logic_error(ss.str());
  }
  if (pid != 0) {
    _exit(0); // exit parent
  }

  pid_t sid = setsid();
  if (sid == -1) {
    std::stringstream ss;
    ss << "Could not become a daemon: setsid failed: " << errno;;
    throw std::logic_error(ss.str());
  }

  // check fork for child
  pid = fork();
  if (pid == -1) {
    std::stringstream ss;
    ss << "Could not become a daemon: fork #2 failed: " << errno;
    throw std::logic_error(ss.str());
  }
  if (pid != 0) {
    _exit(0); // exit session leader
  }

  for (int i = getdtablesize(); i--; ) {
    close(i);
  }
  umask(0002); // disable: S_IWOTH
  chdir("/");

  const char *devnull = "/dev/null";
  stdin = fopen(devnull, "a+");
  if (stdin == NULL) {
    return false;
  }
  stdout = fopen(devnull, "w");
  if (stdout == NULL) {
    return false;
  }
  stderr = fopen(devnull, "w");
  if (stderr == NULL) {
    return false;
  }
  return true;
}

// void signalHandler(int signo) {
//   if ((SIGINT == signo || SIGTERM == signo) && ::server != NULL) {
//     server->stop();
//   }
// }   

// void setUpSignalHandlers() {
//   if (SIG_ERR == signal(SIGINT, signalHandler)) {
//     throw std::runtime_error("Cannot set up SIGINT handler");
//   } 
//   if (SIG_ERR == signal(SIGTERM, signalHandler)) {
//     throw std::runtime_error("Cannot set up SIGTERM handler");
//   }
// }


void print_usage(FILE* stream, int exit_code) {
    fprintf(stream, "Usage: %s <operation name> [other options] \n", _args.program_name);
    fprintf(stream,
            "  -h  --help               Display usage information.\n"
            "  -d  --daemon    Run with daemon.\n"
            "  -a  --debug    Run with debug.\n");
    exit(exit_code);
}

void select_option(int argc, char* argv[]) {

    int next_option;
    const char* const short_options = "ho:c:v";
    const struct option long_options[] = {
        { "help", no_argument, NULL, 'h'},
        { "debug", no_argument, NULL, 'a'},
        { "daemon", no_argument, NULL, 'd'},
        { NULL, no_argument, NULL, 0}
    };

    _args.config_file = NULL;
    _args.operation_name = NULL;
    _args.verbose = 0;
    _args.program_name = argv[0];

    //do {
        next_option = getopt_long(argc, argv, short_options, long_options, NULL);
        switch (next_option) {
            case 'a':
              runner();
              break;
            case 'd':
                  if (!daemonize()) {
                    exit(EXIT_FAILURE);
                  }
                    runner();
                _args.config_file = optarg;
                break;
            case 'h':
            case '?':
                print_usage(stdout, 0);
            case -1: 
                break;
            default: 
                exit(EXIT_FAILURE);
        }
    //} while (next_option != -1);

    if (argc < 2 || _args.operation_name == NULL) {
        print_usage(stdout, 0);
    }

    // remained arguments
    if (_args.verbose) {
        for (int i = optind; i < argc; ++i) {
            printf("Argument: %s\n", argv[i]);
        }
    }

    
    if(strcmp(_args.operation_name, OPT1) == 0) {
        printf("operation: %s\n",_args.operation_name);
    } else if(strcmp(_args.operation_name, OPT2) == 0) {
        printf("operation: %s\n",_args.operation_name);
    } else {
        printf("Unknown operation\n");
        print_usage(stdout, 0);
    }
    
}

int main(int argc, char* argv[]) {
  // if (!daemonize()) {
  //     exit(EXIT_FAILURE);
  // }
  //   runner();
  select_option(argc, argv);
}
// int main(int argc, char* argv[])
// {
//   printf("%s\n", argv[0]);
//   if(argv[0] == 0){
//     daemon(0,0);
//     runner();
//   }else{
//     runner();
//   }
    
// }