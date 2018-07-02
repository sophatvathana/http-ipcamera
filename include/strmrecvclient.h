#ifndef STRMRECVCLIENT_H
#define STRMRECVCLIENT_H

#include <string>
#include <ctime>
#include "strmrecvdef.h"

#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#ifdef __cplusplus
} // __cplusplus defined.
#endif

namespace strmrecvclientns {

    const char *state_to_string(int state);

    // input
    struct _STRMRECVClientParameters
    {
        const char *address;
        int clientId;
    };

    typedef struct _STRMRECVClientParameters STRMRECVClientParameters;

    // struct
    struct _STRMRECVClientStruct
    {
        // private
        AVFormatContext *_pFormatCtx;
        AVCodecContext *_pCodecCtxOrig;
        AVCodecContext *_pCodecCtx;
        AVCodec *_pCodec;
        AVPacket _packet;

        char errbuf[STRMRECVCLIENT_ERRBUF_SIZE];

        int _videoStream;

        // public
        int clientId;
        std::string address;
        bool       connected;
        int           videoIndex;
        int state;
        bool isWaiting;
        int newFramesNum;
        int next;
        uint8_t *frameQueue[STRMRECVCLIENT_FRAME_QUEUE_LEN];
        int frameSizes[STRMRECVCLIENT_FRAME_QUEUE_LEN];

        clock_t av_frame_read_ticks;

    };

    typedef struct _STRMRECVClientStruct STRMRECVClientStruct;

    class STRMRECVClient
    {
    private:
        static STRMRECVClient *_instance;
        STRMRECVClient();
        void _logAVError(STRMRECVClientStruct *pClient, int error);
        void _clean(STRMRECVClientStruct *pClient, int stateToSet = STRMRECVCLIENT_STATE_CLEANED);
        int _init(STRMRECVClientStruct *pClient);
        int _readFrame(STRMRECVClientStruct *pClient);

    public:
        STRMRECVClientStruct *clients[STRMRECVCLIENT_MAX_CLIENT_NUM];
        static STRMRECVClient *getInstance();

        static void threadLoop(STRMRECVClientParameters *parameters);
        void * createContext(const char* path);
        void releaseContext(void* context);
        unsigned char* readFrame(void* context, int* bufferLen);
        int openRtsp(void* context, const char* url);
        int wait(int clientId);
        int resume(int clientId);
        void stop(int clientId);
        void abort(int clientId);
        int destroy(int clientId);

        void logState(int clientId);
    };

}
#endif
