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

#define TEST_ADDRESS "rtsp://admin:12345@192.168.0.38/Streaming/Channels/102"
//"rtsp://76.89.206.161/live3.sdp"
//"rtsp://76.89.206.161/live3.sdp"
//"rtsp://admin:12345@192.168.0.38/Streaming/Channels/1"
#define TEST_FRAME_NUM 100000000
#define TEST_FRAME_PER_LOOP 50

using namespace SonaHttp;
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
struct HelloWorldHandler : public RequestHandler {
	void handleRequest(RequestPtr req, ResponsePtr rep) override {
          //std::terminate();
            int clientId = std::stoi((char*)SonaHttp::getParam(req->getQueryString(), "id").c_str());
            std::thread thread_stream([req, rep, clientId]{
                const char * addr = NULL;
                const std::string rg = "(\\baddr=(((?!&).)*))";
                static const std::regex regex(rg);
                std::smatch smt;

                auto search = req->getQueryString();
                if (std::regex_search(search, smt, regex)) {
                      std::string saddr =  smt[2];
                      addr = saddr.c_str();
                      rep->write((char *)HEAD_RESPONSE);
                      strmrecvclient_start_log("","");
                      strmrecvclient_start(clientId, addr, 1);
                      STRMRECVClientData *data = new STRMRECVClientData();
                      int t = 0;
                      while(1){
                            strmrecvclient_log_state(clientId);
                            data->state = strmrecvclient_get_state(clientId);
                            if(data->state = STRMRECVCLIENT_STATE_ERROR){
                                char * t = "សូមដាក់ ឲ្យបានត្រឹមត្រូវ";
                                rep->raw_write(t, strlen(t));
                                rep->flush();
                                break;
                            }

                            if (data->state != STRMRECVCLIENT_STATE_LOOPING){
                                  if (data->state < STRMRECVCLIENT_STATE_INITIALIZING)
                                        strmrecvclient_start(clientId, addr, 1);

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
          }else{
              char * t = "សូមដាក់ ឲ្យបានត្រឹមត្រូវ";
              rep->raw_write(t, strlen(t));
              rep->flush();
            return;
          }
        });
          //thread_stream.join();
          strmrecvclient_stop(clientId);
          thread_stream.detach();

	}
};

int
main(int argc, char *argv[])
{
	std::stringstream config("{\"http port\":\"8888\"}");
	Server server(config);
      server.addHandler("/test", new HelloWorldHandler());
      std::thread thread_runner([&server]{
        server.run(100);
      });
      std::this_thread::sleep_for(std::chrono::seconds(1));
      thread_runner.join();
/*
	char response_data[1024];

	char http_header[2048] = "HTTP/1.1 200 OK\r\nContent-type: multipart/x-mixed-replace;boundary=--boundary\r\n\r\n";
	//strcat(http_header, response_data);

	// create a socket 
	int server_socket;
	server_socket = socket(AF_INET, SOCK_STREAM, 0);

	
	// define the address
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(8001);
	server_address.sin_addr.s_addr = INADDR_ANY;

	bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address));
      std::thread thread_runner([server_socket]{
	   listen(server_socket, 5);
      });
        std::this_thread::sleep_for(std::chrono::seconds(0));
       thread_runner.join();
	int client_socket;

	//while(1) {
		client_socket = accept(server_socket, NULL, NULL);
		send(client_socket, HEAD_RESPONSE, sizeof(HEAD_RESPONSE), 0);
			strmrecvclient_start(0, TEST_ADDRESS, 1);
       	STRMRECVClientData *data = new STRMRECVClientData();
        int t = 0;
        while(1){
        	strmrecvclient_start_log("","");
       	strmrecvclient_log_state(0);
       	data->state = strmrecvclient_get_state(0);

        if (data->state != STRMRECVCLIENT_STATE_LOOPING)
        {
            if (data->state < STRMRECVCLIENT_STATE_INITIALIZING)
                strmrecvclient_start(0, TEST_ADDRESS, 0);

           	 std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            continue;
        }

       if (strmrecvclient_wait(0) != 0)
            continue;

        strmrecvclient_get_data(0, TEST_FRAME_PER_LOOP, 0, data);

        strmrecvclient_resume(0);
  
        for (int i = 0; i < data->framesRead; i++)
        {
            if (data->frameSizes[i] < 1024){
                continue;
            }
            t++;
              unsigned char * image =  &data->frameQueue[i * STRMRECVCLIENT_FRAME_BUFFER_SIZE];
              char buf1[1024];
              sprintf(buf1,"HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\nCache-Control : no-cache, private\r\nContent-Length : %d\r\n\r\n", data->frameSizes[i]);
              write(client_socket,buf1,strlen(buf1));
              fflush(stdout);
              printf("Written: %d \n",write(client_socket,(char * ) &data->frameQueue[i * STRMRECVCLIENT_FRAME_BUFFER_SIZE],data->frameSizes[i]));
              fflush(stdout);
              // std::string name = "test";
              // name += std::to_string(static_cast<long long>(t));
              // name += ".jpg";
             // std::ofstream(name, std::ios::binary).write((char * ) &data->frameQueue[i * STRMRECVCLIENT_FRAME_BUFFER_SIZE], data->frameSizes[i]);
              send(client_socket, SEPARATOR,sizeof(SEPARATOR), 0);
        
         
        }

        	std::this_thread::sleep_for(std::chrono::milliseconds(180));
	}
		close(client_socket);
    */
    
}