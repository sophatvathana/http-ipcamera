#ifndef STRMRECVDEF_H
#define STRMRECVDEF_H

#define STRMRECVCLIENT_FRAME_BUFFER_SIZE 16777216 //2^24
#define STRMRECVCLIENT_FRAME_QUEUE_LEN 1//5
#define STRMRECVCLIENT_TIMEOUT 10//5
#define STRMRECVCLIENT_MAX_CLIENT_NUM 10000000

// STATES
#define STRMRECVCLIENT_STATE_ERROR -1
#define STRMRECVCLIENT_STATE_EMPTY 0
#define STRMRECVCLIENT_STATE_CLEANED 1
#define STRMRECVCLIENT_STATE_INITIALIZING 2
#define STRMRECVCLIENT_STATE_ABORTING 3
#define STRMRECVCLIENT_STATE_STOPPING 4
#define STRMRECVCLIENT_STATE_LOOPING 5

#define STRMRECVCLIENT_ERRBUF_SIZE 512
#define STRMRECVCLIENT_AV_FRAME_READ_TIMEOUT 10 // seconds

#endif
