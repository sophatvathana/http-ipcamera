#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <server.h>

#include <sys/time.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <sys/fcntl.h>
#include <sys/stat.h>

#include "strmrecvclientapi.h"

#define TEST_ADDRESS "rtsp://admin:12345@192.168.0.168/Streaming/Channels/102"
#define TEST_FRAME_NUM 100000000
#define TEST_FRAME_PER_LOOP 50

static const char error_handler[1024] = 
"<html>\n"
"<b> error code %d cause %s ! </b>"
"</html>";

int bind(int port, int local)
{
	int sl, optval=1;
	struct sockaddr_in sin;

	if ((sl=socket(PF_INET, SOCK_STREAM, 0))<0) {
		
		return -1;
	}

	memset(&sin, 0, sizeof(struct sockaddr_in));
	sin.sin_family=AF_INET;
	sin.sin_port=htons(port);
	
	if (local)
		sin.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
	else
		sin.sin_addr.s_addr=htonl(INADDR_ANY);

	setsockopt(sl, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	if (bind(sl, (struct sockaddr *)&sin, sizeof(struct sockaddr_in))==-1) {
		
		close(sl);
		return -1;
	}

	if (listen(sl, DEF_MAXWEBQUEUE)==-1) {
		
		close(sl);
		return -1;
	}

	return sl;
}

static int accept(int sl)
{
	int sc;
	unsigned long i;
	struct sockaddr_in sin;
	socklen_t addrlen=sizeof(struct sockaddr_in);

	if ((sc=accept(sl, (struct sockaddr *)&sin, &addrlen))>=0) {
		int flags = fcntl(sc, F_GETFL, 0);
		fcntl(sc, F_SETFL, flags | O_NONBLOCK);
		// i=1;
		// ioctl(sc, FIONBIO, &i);
		return sc;
	}
	
	return -1;
}

static struct HttpClient *httpClientTemp(int size)
{
	struct HttpClient *tmpbuffer=mymalloc(sizeof(struct webcam_buffer));
	tmpbuffer->ref=0;
	tmpbuffer->ptr=mymalloc(size);
		
	return tmpbuffer;
}


int main() {
	char http_header[2048] = "HTTP/1.1 200 OK\r\n\n";
	char http_rtsp[2048] = 
	"HTTP/1.0 200 OK\r\n"
			"Server: Sona/0.1\r\n"
			"Connection: close\r\n"
			"Max-Age: 0\r\n"
			"Expires: 0\r\n"
			"Cache-Control: no-cache, private\r\n"
			"Pragma: no-cache\r\n"
			"Content-Type: multipart/x-mixed-replace; boundary=--BoundaryString\r\n\r\n";

	FILE *html_data;

	int server_socket;
	// server_socket = socket(AF_INET, SOCK_STREAM, 0);

	server_socket = bind(8000, 1);

	int client_socket;

	//html_data =  fopen("index.html", "r");
	strmrecvclient_start_log("","");
	strmrecvclient_start(0, TEST_ADDRESS, 1);

	STRMRECVClientData *data = new STRMRECVClientData();
	while(true){
		strmrecvclient_log_state(0);
		data->state = strmrecvclient_get_state(0);

        if (data->state != STRMRECVCLIENT_STATE_LOOPING)
        {
            if (data->state < STRMRECVCLIENT_STATE_INITIALIZING)
                strmrecvclient_start(0, TEST_ADDRESS, 1);

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
            strcat(http_rtsp,(char * ) &data->frameQueue[i * STRMRECVCLIENT_FRAME_BUFFER_SIZE]);
		client_socket = accept(server_socket);
		send(client_socket, http_rtsp, sizeof(http_rtsp), 0);
            // std::string name = "test";
            // name += std::to_string(static_cast<long long>(nframe));
            // name += ".jpg";
            // std::ofstream(name, std::ios::binary).write((char * ) &data->frameQueue[i * STRMRECVCLIENT_FRAME_BUFFER_SIZE], data->frameSizes[i]);

            // nframe++;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));

	}
	close(client_socket);
	strmrecvclient_stop(0);
	char response_data[1024] = "<html><h1>Hello world</h1></html>";
	//fgets(response_data, 1024, "<html><h1>Hello world</h1></html>");

	strcat(http_header, response_data);

	
	strmrecvclient_stop_log();
	return 0;
}




















