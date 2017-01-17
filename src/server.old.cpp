/*
* @Author: sophatvathana
* @Date:   2017-01-12 12:52:47
* @Last Modified by:   sophatvathana
* @Last Modified time: 2017-01-12 13:20:48
*/
#include "strmrecvclientapi.h"
#include <iostream>
#include <chrono>
#include <fstream>
#include <server.h>
#include <sys/time.h>
#include <ctype.h>
#include <sys/stat.h>
#include <cstdio>

/* generic */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

/* networking */
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>


#define STD_HEADER "Connection: close\r\n" \
                   "Server: Sona/0.2\r\n" \
                   "Cache-Control: no-store, no-cache, must-revalidate, pre-check=0, post-check=0, max-age=0\r\n" \
                   "Pragma: no-cache\r\n" \
                   "Expires: Mon, 6 Jan 2020 12:34:56 GMT\r\n"
#define BOUNDARY "boundarydonotcross"

#define TEST_ADDRESS "rtsp://76.89.206.161/live3.sdp"
                   //"rtsp://118.69.187.169:82/live.sdp"
// "rtsp://76.89.206.161/live3.sdp"
                   //"rtsp://admin:12345@192.168.200.168/Streaming/Channels/102"
#define TEST_FRAME_NUM 100000000
#define TEST_FRAME_PER_LOOP 2//50
#define TCP_CORK 3
#if !(defined(LINUX) && defined(__alpha))
#include <netinet/tcp.h> 
#ifdef LINUX
#ifndef TCP_CORK
#define TCP_CORK 3
#endif
#endif
#endif


using namespace std;


