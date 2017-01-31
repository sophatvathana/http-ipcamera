#ifndef STRMRECVCLIENTAPI_H
#define STRMRECVCLIENTAPI_H

#include <chrono>
#include <thread>
#include "strmrecvdef.h"

#ifdef _MSC_VER
#define STRMRECV_EXPORTS

#ifdef STRMRECV_EXPORTS
#define STRMRECVAPI __declspec(dllexport)
#else
#define STRMRECVAPI __declspec(dllimport)
#endif

#define STRMRECVCALL __cdecl
#else
#define STRMRECVAPI
#define STRMRECVCALL
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    struct _STRMRECVClientData
    {
        int isRuning;
        int clientId;
        int state;
        int isWaiting;
        int framesRead;
        int next;
        unsigned char frameQueue[STRMRECVCLIENT_FRAME_QUEUE_LEN * STRMRECVCLIENT_FRAME_BUFFER_SIZE];
        int frameSizes[STRMRECVCLIENT_FRAME_QUEUE_LEN];
    };

    typedef struct _STRMRECVClientData STRMRECVClientData;

    STRMRECVAPI void STRMRECVCALL strmrecvclient_start_log(const char *outputLogFile, const char *errorLogFile);
    STRMRECVAPI void STRMRECVCALL strmrecvclient_stop_log();
    STRMRECVAPI int STRMRECVCALL strmrecvclient_get_log_level();
    STRMRECVAPI void STRMRECVCALL strmrecvclient_set_log_level(int logLevel);

    STRMRECVAPI void STRMRECVCALL strmrecvclient_log_state(int client);

    STRMRECVAPI int STRMRECVCALL strmrecvclient_start(int clientId, const char *address, int asyncCall);
    STRMRECVAPI int STRMRECVCALL strmrecvclient_wait(int clientId);
    STRMRECVAPI int STRMRECVCALL strmrecvclient_resume(int clientId);
    STRMRECVAPI int STRMRECVCALL strmrecvclient_stop(int clientId);
    STRMRECVAPI int STRMRECVCALL strmrecvclient_destroy(int clientId);

    STRMRECVAPI std::string STRMRECVCALL strmrecvclient_get_address(int clientId);
    STRMRECVAPI int STRMRECVCALL strmrecvclient_get_data(int clientId, int frames, int fps, STRMRECVClientData *data);
    STRMRECVAPI int STRMRECVCALL strmrecvclient_get_state(int clientId);

    STRMRECVAPI void * STRMRECVCALL  StrmCreateContext (const char* path);

    STRMRECVAPI int STRMRECVCALL StrmOpenRtsp(void* context, const char* url);

    STRMRECVAPI unsigned char* STRMRECVCALL StrmReadFrame(int handler, const char* url);
    STRMRECVAPI void STRMRECVCALL StrmReleaseContext(void* context) ;

#ifdef __cplusplus
} // __cplusplus defined.
#endif

#endif
