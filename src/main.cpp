/*
* @Author: sophatvathana
* @Date:   2017-01-12 12:51:32
* @Last Modified by:   sophatvathana
* @Last Modified time: 2017-01-12 13:19:56
*/
#include "server_http.h"
#include <strmrecvclientapi.h>
#include <fstream>
#include <vector>
#include <algorithm>
#ifdef HAVE_OPENSSL
#include "crypto.h"
#endif

using namespace std;
#define TEST_ADDRESS "rtsp://admin:12345@192.168.0.38/Streaming/Channels/102"
//"rtsp://76.89.206.161/live3.sdp"
//"rtsp://76.89.206.161/live3.sdp"
//"rtsp://admin:12345@192.168.0.38/Streaming/Channels/1"
#define TEST_FRAME_NUM 100000000
#define TEST_FRAME_PER_LOOP 50
const char HEAD_RESPONSE[] =
{
    "HTTP/1.1 200 OK\r\n" 
    "Server: en.code-bude.net example server\r\n" 
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


typedef StrmrecvWeb::Server<StrmrecvWeb::HTTP> HttpServer;

void default_resource_send(const HttpServer &server, const shared_ptr<HttpServer::Response> &response,
                           const shared_ptr<ifstream> &ifs);

int main() {
    HttpServer server;
    server.config.port=8080;

    server.resource["^/info$"]["GET"]=[](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
        stringstream content_stream;
        content_stream << "<h1>Request from " << request->remote_endpoint_address << " (" << request->remote_endpoint_port << ")</h1>";
        content_stream << request->method << " " << request->path << " HTTP/" << request->http_version << "<br>";
        for(auto& header: request->header) {
            content_stream << header.first << ": " << header.second << "<br>";
        }
        
        //find length of content_stream (length received using content_stream.tellp())
        content_stream.seekp(0, ios::end);
        
        *response <<  "HTTP/1.1 200 OK\r\nContent-Length: " << content_stream.tellp() << "\r\n\r\n" << content_stream.rdbuf();
    };

    server.resource["^/work$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request ) {
        * response << HEAD_RESPONSE;
        *response << "0\r\n\r\n";
        thread work_thread([response] {
        strmrecvclient_start(0, TEST_ADDRESS, 1);
        STRMRECVClientData *data = new STRMRECVClientData();
        int t = 0;
        while(t< 1000000){
         strmrecvclient_start_log("","");
         strmrecvclient_log_state(0);
         data->state = strmrecvclient_get_state(0);

        if (data->state != STRMRECVCLIENT_STATE_LOOPING)
        {
            if (data->state < STRMRECVCLIENT_STATE_INITIALIZING)
                strmrecvclient_start(0, TEST_ADDRESS, 0);

              std::this_thread::sleep_for(std::chrono::seconds(1));

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
              //write(rep,buf1,strlen(buf1));
              *response << buf1;
               //rep->out() << buf1 << std::endl;
               fflush(stdout);
               //rep->flush();
             // printf("Written: %d \n",write(rep,(char * ) &data->frameQueue[i * STRMRECVCLIENT_FRAME_BUFFER_SIZE],data->frameSizes[i]));
               //rep->raw_write((char * ) &data->frameQueue[i * STRMRECVCLIENT_FRAME_BUFFER_SIZE],data->frameSizes[i]);
               //rep->out() << (char * ) &data->frameQueue[i * STRMRECVCLIENT_FRAME_BUFFER_SIZE] << std::endl;
              fflush(stdout);
              //rep->flush();
              std::string name = "test";
              name += std::to_string(static_cast<long long>(t));
              name += ".jpg";
             // std::ofstream(name, std::ios::binary).write((char * ) &data->frameQueue[i * STRMRECVCLIENT_FRAME_BUFFER_SIZE], data->frameSizes[i]);


              //send(rep, SEPARATOR,sizeof(SEPARATOR), 0);
              //rep->out() << SEPARATOR << std::endl;
         
        }
         std::this_thread::sleep_for(std::chrono::seconds(0));
        }
        });

        work_thread.detach();
    };

    
    thread server_thread([&server](){
        server.start();
    });

    this_thread::sleep_for(chrono::seconds(1));

    server_thread.join();
    
    return 0;
}

void default_resource_send(const HttpServer &server, const shared_ptr<HttpServer::Response> &response,
                           const shared_ptr<ifstream> &ifs) {
    static vector<char> buffer(131072); // Safe when server is running on one thread
    streamsize read_length;
    if((read_length=ifs->read(&buffer[0], buffer.size()).gcount())>0) {
        response->write(&buffer[0], read_length);
        if(read_length==static_cast<streamsize>(buffer.size())) {
            server.send(response, [&server, response, ifs](const boost::system::error_code &ec) {
                if(!ec)
                    default_resource_send(server, response, ifs);
                else
                    cerr << "Connection interrupted" << endl;
            });
        }
    }
}