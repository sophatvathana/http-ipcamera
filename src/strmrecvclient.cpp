/*
* @Author: sophatvathana
* @Date:   2017-01-12 12:58:56
* @Last Modified by:   sophatvathana
* @Last Modified time: 2017-01-12 13:20:53
*/
#include <string>
#include <log4cplus/loggingmacros.h>
#include "loghandler.h"
#include "strmrecvclient.h"
#include <iostream>
#include <libavutil/dict.h>
#include <libavformat/avformat.h>
#include <vector>
#include <dlfcn.h>
#include <stdbool.h>
#include <libavutil/opt.h>
#include <unistd.h>
#include <signal.h>
using namespace loghandlerns;

namespace strmrecvclientns {

#ifndef _WIN32
#define HMODULE void*
#endif

#define MAX_PARSE_BUFFER 4096

#ifndef MYBOOLEAN_H
#define MYBOOLEAN_H

#define false 0
#define true 1
typedef int BOOL; 

#endif

typedef void(*log_write_func)(int level, const char* file, const char* func, int line, const char* msg);

void LogWriteFuncion(int level, const char* file, const char* func, int line, const char* msg) {
    std::string msgs(msg);
    msgs += "\n";
#ifdef _WIN32
    OutputDebugStringA(msgs.c_str());
#endif//_WIN32
    std::cerr << msgs;
}

void logwrite(const char* file, const char* func, int line, int lvl, const char* pszFormat, ...) {
    if (lvl < 0 || lvl > 4)
        lvl = 4;

    va_list args;
    va_start(args, pszFormat);

    size_t n = 0;
    char szBuffer[MAX_PARSE_BUFFER] ={0};

    std::string sFile(file);
    size_t pos = sFile.find_last_of("/\\");
    if (pos != std::string::npos) {
        sFile = sFile.substr(pos + 1);
    }

    std::string sFunc(func);
    pos = sFunc.find_last_of(":.");
    if (pos != std::string::npos) {
        sFunc = sFunc.substr(pos + 1);
    }

    if (vsnprintf(szBuffer + n, MAX_PARSE_BUFFER - n, pszFormat, args) >= 0) {

        LogWriteFuncion(
            (int)lvl,
            sFile.c_str(),
            sFunc.c_str(),
            line,
            szBuffer);

    }
    else {
        std::string msg("ERR(can not make logdata):");
        msg += pszFormat;
        msg += "\n";
        LogWriteFuncion(
            (int)lvl,
            sFile.c_str(),
            sFunc.c_str(),
            line,
            msg.c_str());
    }

    va_end(args);
}

#define LogWrite(...) logwrite(\
    __FILE__, __FUNCTION__, __LINE__, 0, __VA_ARGS__)

void* ALoadLibrary(const char* path) {
    HMODULE handle = NULL;
#ifdef _WIN32
    handle = ::LoadLibraryA(path);
#else
    handle = dlopen(path, RTLD_LAZY);
#endif
    return handle;
}

void  AFreeLibrary(void* context) {
#ifdef _WIN32
    if (context)
        ::FreeLibrary((HMODULE)context);
#else//_WIN32
    if (context)
        dlclose((HMODULE)context);
#endif//_WIN32
}

bool AssignFunction(void** dest, void* context, const char* name) {
    //LogWrite("    Assign Function: %s", name);
#ifdef _WIN32
    *dest = (void*)::GetProcAddress((HMODULE)context, name);
#else//_WIN32
    *dest = (void*)::dlsym((HMODULE)context, name);
#endif//_WIN32
    if (*dest == NULL) {
        LogWrite("    Address mapping fail: %s", name);
    }
    return *dest != NULL;
}

//////////////////////////////////////////
// end of global variables
//////////////////////////////////////////

const char *state_to_string(int state)
{
    switch (state)
    {
    case STRMRECVCLIENT_STATE_ERROR:
        return "ERROR";

    case STRMRECVCLIENT_STATE_EMPTY:
        return "EMPTY";

    case STRMRECVCLIENT_STATE_CLEANED:
        return "CLEANED";

    case STRMRECVCLIENT_STATE_INITIALIZING:
        return "INITIALIZING";

    case STRMRECVCLIENT_STATE_LOOPING:
        return "LOOPING";

    case STRMRECVCLIENT_STATE_STOPPING:
        return "STOPPING";

    case STRMRECVCLIENT_STATE_ABORTING:
        return "ABORTING";

    default:
        return "UNKNOWN";
    }
}

STRMRECVClient* STRMRECVClient::_instance = NULL;

static int interrupt_cb(void *pClient)
{
    //if av_read_frame takes more than timeout then interrupt it
    if (((STRMRECVClientStruct * ) pClient)->state == STRMRECVCLIENT_STATE_LOOPING
        && (clock() - ((STRMRECVClientStruct * ) pClient)->av_frame_read_ticks) / CLOCKS_PER_SEC > STRMRECVCLIENT_AV_FRAME_READ_TIMEOUT)
        return 1;

    return ((STRMRECVClientStruct * ) pClient)->state == STRMRECVCLIENT_STATE_ABORTING
        || ((STRMRECVClientStruct * ) pClient)->state == STRMRECVCLIENT_STATE_STOPPING ? 1 : 0;
}

static const AVIOInterruptCB int_cb = { interrupt_cb, NULL };

STRMRECVClient::STRMRECVClient()
{
    av_register_all();
    avformat_network_init();
    for (int i = 0; i < STRMRECVCLIENT_MAX_CLIENT_NUM; i++)
        clients[i] = NULL;
}


STRMRECVClient* STRMRECVClient::getInstance()
{
    if (!_instance)
        _instance = new STRMRECVClient;

    return _instance;
}

void STRMRECVClient::_logAVError(STRMRECVClientStruct *pClient, int error)
{
    Logger loggerError = Logger::getInstance(LOG4CPLUS_TEXT(DEFAULT_ERROR_LOGGER));
    Logger logger = Logger::getInstance(LOG4CPLUS_TEXT(DEFAULT_OUTPUT_LOGGER));
    if (av_strerror(error, pClient->errbuf, STRMRECVCLIENT_ERRBUF_SIZE) == 0)
    {
        printf("LibAV error: %s\n",pClient->errbuf );
        
        LOG4CPLUS_ERROR(loggerError, "[CLIENT " << pClient->clientId << "] LibAV error: " << pClient->errbuf);
        LOG4CPLUS_ERROR(logger, "[CLIENT " << pClient->clientId << "] LibAV error: " << pClient->errbuf);
    }
    else
    {
        LOG4CPLUS_ERROR(loggerError, "[CLIENT " << pClient->clientId << "] LibAV error: Unknown error...");
        LOG4CPLUS_ERROR(logger, "[CLIENT " << pClient->clientId << "] LibAV error: Unknown error...");
    }
}

int STRMRECVClient::_init(STRMRECVClientStruct *pClient)
{
     Logger logger = Logger::getInstance(LOG4CPLUS_TEXT(DEFAULT_OUTPUT_LOGGER));
     Logger loggerError = Logger::getInstance(LOG4CPLUS_TEXT(DEFAULT_ERROR_LOGGER));
     pClient->_pCodecCtxOrig = NULL;
     pClient->_pCodecCtx = NULL;
     pClient->_pCodec = NULL;
     //pClient->_pFormatCtx = NULL;
    // state is set to ERROR by the caller
    if (pClient->address.empty())
        return -1;

    pClient->state = STRMRECVCLIENT_STATE_INITIALIZING;
    LOG4CPLUS_TRACE(LOG4CPLUS_TEXT(DEFAULT_OUTPUT_LOGGER), "[CLIENT " << pClient->clientId << "] STATE = " << state_to_string(pClient->state));

    LOG4CPLUS_DEBUG(logger, "[CLIENT " << pClient->clientId << "] Trying to open address " << pClient->address);
    
     // open rtsp
    //AVFormatContext* container = NULL;
    pClient->_pFormatCtx = avformat_alloc_context();
    pClient->_pFormatCtx->interrupt_callback = int_cb;
    pClient->_pFormatCtx->interrupt_callback.opaque = pClient;

    int ret = 0;

    // open input
    LOG4CPLUS_TRACE(logger, "[CLIENT " << pClient->clientId << "] avformat_open_input() [TCP]... ");

    // try with TCP
      AVDictionary *options = 0;
    try{
        //av_dict_set(&options, "rtsp_transport", "tcp", 0);
        std::size_t pos = pClient->address.find("://"); 
        std::string str3 = pClient->address.substr (0,pos);
        printf("%s\n", str3.c_str());
        if (str3 == "mms") {
            av_dict_set(&options, "rtsp_transport", "udp_multicast", 0);
        } else if (str3 == "rtsp") {
            av_dict_set(&options, "rtsp_transport", "tcp", 0);
        } else if (str3 == "http") {
            av_dict_set(&options, "rtsp_transport", "http", 0);
        }
        ret = avformat_open_input(&(pClient->_pFormatCtx), pClient->address.c_str(), 0, &options);
        av_dict_free(&options);
    }catch(...){

    }

    if (ret < 0)
    {     
        pClient->_pFormatCtx = NULL;
        LOG4CPLUS_ERROR(loggerError, "[CLIENT " << pClient->clientId << "] Could not open address [TCP] " << pClient->address << "!");
        LOG4CPLUS_ERROR(logger, "[CLIENT " << pClient->clientId << "] Could not open address [TCP] " << pClient->address << "!");
        _logAVError(pClient, ret);

        // this to avoid to try UDP while stopping or aborting
        if (pClient->state != STRMRECVCLIENT_STATE_INITIALIZING){
            pClient->state = STRMRECVCLIENT_STATE_INITIALIZING;
            return -1;
        }

        // try with UDP
        LOG4CPLUS_TRACE(logger, "[CLIENT " << pClient->clientId << "] avformat_open_input() [UDP]... ");
        try{
          ret = avformat_open_input(&(pClient->_pFormatCtx), pClient->address.c_str(), 0, 0);
        }catch(...){}

        if (ret < 0)
        {
            pClient->_pFormatCtx = NULL;
            LOG4CPLUS_ERROR(loggerError, "[CLIENT " << pClient->clientId << "] Could not open address [UDP] " << pClient->address << "!");
            LOG4CPLUS_ERROR(logger, "[CLIENT " << pClient->clientId << "] Could not open address [UDP] " << pClient->address << "!");
            _logAVError(pClient, ret);

            return -1;
        }
    }

    // find stream info
    LOG4CPLUS_TRACE(logger, "[CLIENT " << pClient->clientId << "] avformat_find_stream_info()... ");

    // see https://ffmpeg.zeranoe.com/forum/viewtopic.php?t=1211
    int64_t max_analyze_duration = pClient->_pFormatCtx->max_analyze_duration;

    pClient->_pFormatCtx->max_analyze_duration = 0;
    ret = avformat_find_stream_info(pClient->_pFormatCtx, 0);
    pClient->_pFormatCtx->max_analyze_duration = max_analyze_duration;

    if (ret < 0)
    {
        LOG4CPLUS_ERROR(loggerError, "[CLIENT " << pClient->clientId << "] Could not get stream info! ");
        LOG4CPLUS_ERROR(logger, "[CLIENT " << pClient->clientId << "] Could not get stream info! ");
        _logAVError(pClient, ret);

        return -1;
    }

    // search video stream
    pClient->_videoStream = -1;

    for (int i = 0; i < (int ) pClient->_pFormatCtx->nb_streams; i++)
    {
        if (pClient->_pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO &&
         pClient->_pFormatCtx->streams[i]->codec->codec_id == AV_CODEC_ID_MJPEG
         || pClient->_pFormatCtx->streams[i]->codec->codec_id == AV_CODEC_ID_MPEG2VIDEO ||
          pClient->_pFormatCtx->streams[i]->codec->codec_id == AV_CODEC_ID_MPEG4 ||
          pClient->_pFormatCtx->streams[i]->codec->codec_id == AV_CODEC_ID_H264 ||
          pClient->_pFormatCtx->streams[i]->codec->codec_id == AV_CODEC_ID_VC1 || 
          pClient->_pFormatCtx->streams[i]->codec->codec_id == AV_CODEC_ID_WMV3
         )
        {
            pClient->_videoStream = i;
            break;
        }
    }

    if (pClient->_videoStream == -1)
    {
        LOG4CPLUS_ERROR(loggerError, "[CLIENT " << pClient->clientId << "] Could not find any format stream!");
        LOG4CPLUS_ERROR(logger, "[CLIENT " << pClient->clientId << "] Could not find any format stream!");

        return - 1;
    }

    pClient->_pCodecCtxOrig = pClient->_pFormatCtx->streams[pClient->_videoStream]->codec;
    LOG4CPLUS_TRACE(logger, "[CLIENT " << pClient->clientId << "] avcodec_alloc_context3()... ");
    pClient->_pCodecCtx = avcodec_alloc_context3(NULL);

    // find the decoder for the video stream

    LOG4CPLUS_TRACE(logger, "[CLIENT " << pClient->clientId << "] avcodec_find_decoder()... ");
    pClient->_pCodec = avcodec_find_decoder(pClient->_pCodecCtxOrig->codec_id);

    if (pClient->_pCodec == NULL)
    {
        LOG4CPLUS_ERROR(loggerError, "[CLIENT " << pClient->clientId << "] Unsupported codec!");
        LOG4CPLUS_ERROR(logger, "[CLIENT " << pClient->clientId << "] Unsupported codec!");

        return -1; // codec not found
    }

    // copy context
    LOG4CPLUS_TRACE(logger, "[CLIENT " << pClient->clientId << "] avcodec_copy_context()... ");
    ret = avcodec_copy_context(pClient->_pCodecCtx, pClient->_pCodecCtxOrig);

    if (ret != 0)
    {
        LOG4CPLUS_ERROR(loggerError, "[CLIENT " << pClient->clientId << "] Could not copy codec context!");
        LOG4CPLUS_ERROR(logger, "[CLIENT " << pClient->clientId << "] Could not copy codec context!");
        _logAVError(pClient, ret);

        return - 1; // error copying codec context
    }

    // open codec
    LOG4CPLUS_TRACE(logger, "[CLIENT " << pClient->clientId << "] avcodec_open2()... ");
    ret = avcodec_open2(pClient->_pCodecCtx, pClient->_pCodec, NULL);

    if (ret < 0)
    {
        LOG4CPLUS_ERROR(loggerError, "[CLIENT " << pClient->clientId << "] Could not open codec!");
        LOG4CPLUS_ERROR(logger, "[CLIENT " << pClient->clientId << "] Could not open codec!");
        _logAVError(pClient, ret);

        return - 1;
    }

    return 0;
}

void STRMRECVClient::_clean(STRMRECVClientStruct *pClient, int stateToSet)
{
    // close the codecs
    if (pClient->_pCodecCtx != NULL)
    {
        avcodec_close(pClient->_pCodecCtx);
        pClient->_pCodecCtx = NULL;
    }

    if (pClient->_pCodecCtxOrig != NULL)
    {
        avcodec_close(pClient->_pCodecCtxOrig);
        pClient->_pCodecCtxOrig = NULL;
    }

    // close the video input
    if (pClient->_pFormatCtx != NULL)
    {
        avformat_close_input(&pClient->_pFormatCtx);
        pClient->_pFormatCtx = NULL;
    }

    pClient->state = stateToSet;

    pClient->isWaiting = false;
    pClient->newFramesNum = 0;
    Logger logger = Logger::getInstance(LOG4CPLUS_TEXT(DEFAULT_OUTPUT_LOGGER));
    LOG4CPLUS_TRACE(logger, "[CLIENT " << pClient->clientId << "] STATE = " << state_to_string(pClient->state));
}

int STRMRECVClient::_readFrame(STRMRECVClientStruct *pClient)
{
    //printf("This is address of ther client: %s\n", pClient->address);
    int ret = 0;
    Logger logger = Logger::getInstance(LOG4CPLUS_TEXT(DEFAULT_OUTPUT_LOGGER));
    LOG4CPLUS_TRACE(logger, "[CLIENT " << pClient->clientId << "] av_read_frame()...");

    pClient->av_frame_read_ticks = clock();

    // is this a packet from the video stream?
    if (av_read_frame(pClient->_pFormatCtx, &pClient->_packet) < 0 )
      //|| pClient->_packet.stream_index != pClient->_videoStream)
    {
        if (pClient->state == STRMRECVCLIENT_STATE_ABORTING || pClient->state == STRMRECVCLIENT_STATE_STOPPING){
            Logger logger = Logger::getInstance(LOG4CPLUS_TEXT(DEFAULT_OUTPUT_LOGGER));
            LOG4CPLUS_WARN(logger, "[CLIENT " << pClient->clientId << "] frame not read because STOPPING or ABORTING");
        }
        else
        {
            Logger logger = Logger::getInstance(LOG4CPLUS_TEXT(DEFAULT_OUTPUT_LOGGER));
            LOG4CPLUS_WARN(logger, "[CLIENT " << pClient->clientId << "] frame read failed or stream not valid!");
            _logAVError(pClient, ret);

            pClient->state = STRMRECVCLIENT_STATE_ABORTING;
        }

        ret = -1;
    } else if (!pClient->isWaiting && pClient->_packet.size <= STRMRECVCLIENT_FRAME_BUFFER_SIZE) {

        // copy the packet (jpg) in the next frame buffer position
        memcpy(pClient->frameQueue[pClient->next], pClient->_packet.data, pClient->_packet.size);
        pClient->frameSizes[pClient->next] = pClient->_packet.size;

        pClient->next = (pClient->next + 1) % STRMRECVCLIENT_FRAME_QUEUE_LEN;

        pClient->newFramesNum = std::min(pClient->newFramesNum + 1, STRMRECVCLIENT_FRAME_QUEUE_LEN);
    }
    
    LOG4CPLUS_DEBUG(logger, "[CLIENT " << pClient->clientId << "] new frames num = " << pClient->newFramesNum << "\tnext = " << pClient->next);

    // free the packet that was allocated by av_read_frame
    av_free_packet(&pClient->_packet);

    return ret;


}

void STRMRECVClient::threadLoop(STRMRECVClientParameters *parameters)
{
    int clientId = parameters->clientId;

    STRMRECVClient *instance = getInstance();
    STRMRECVClientStruct *pClient = instance->clients[clientId];

    if (pClient == NULL || (pClient != NULL && pClient->address.compare(parameters->address) != 0)){
        pClient = new STRMRECVClientStruct;

        pClient->clientId = clientId;
        pClient->state = STRMRECVCLIENT_STATE_CLEANED;
        pClient->isWaiting = false;
        pClient->newFramesNum = 0;
        pClient->next = 0;

        for (int i = 0; i < STRMRECVCLIENT_FRAME_QUEUE_LEN; i++)
        {
            pClient->frameQueue[i] = (uint8_t * ) av_malloc(STRMRECVCLIENT_FRAME_BUFFER_SIZE);
            pClient->frameSizes[i] = 0;
        }

        instance->clients[clientId] = pClient;
        Logger logger = Logger::getInstance(LOG4CPLUS_TEXT(DEFAULT_OUTPUT_LOGGER));
        LOG4CPLUS_DEBUG(logger,  "[CLIENT " << pClient->clientId << "] allocated space for a new client...");
    }
    Logger logger = Logger::getInstance(LOG4CPLUS_TEXT(DEFAULT_OUTPUT_LOGGER));
    LOG4CPLUS_TRACE(logger, "[CLIENT " << pClient->clientId << "] threadLoop()...");

    pClient->address = ((STRMRECVClientParameters * ) parameters)->address;

    delete parameters;

    if (instance->_init(pClient) < 0)
    {
        if(pClient->state < 5 && pClient->state != 1)
            instance->_clean(pClient, STRMRECVCLIENT_STATE_CLEANED);
        Logger logger = Logger::getInstance(LOG4CPLUS_TEXT(DEFAULT_OUTPUT_LOGGER));
        Logger loggerError = Logger::getInstance(LOG4CPLUS_TEXT(DEFAULT_ERROR_LOGGER));
        LOG4CPLUS_ERROR(loggerError, "[CLIENT " << pClient->clientId << "] init failed! with current state:" <<pClient->state );
        LOG4CPLUS_ERROR(logger, "[CLIENT " << pClient->clientId << "] init failed!");
        
        return;
    }

    pClient->state = STRMRECVCLIENT_STATE_LOOPING;

    while (pClient->state == STRMRECVCLIENT_STATE_LOOPING && instance->_readFrame(pClient) == 0){
        Logger logger = Logger::getInstance(LOG4CPLUS_TEXT(DEFAULT_OUTPUT_LOGGER));
        LOG4CPLUS_DEBUG(logger, "[CLIENT " << pClient->clientId << "] while()...");
    }


    instance->_clean(pClient, pClient->state == STRMRECVCLIENT_STATE_STOPPING ? STRMRECVCLIENT_STATE_CLEANED : STRMRECVCLIENT_STATE_ERROR);
}

int STRMRECVClient::wait(int clientId)
{
    if (clients[clientId]->state != STRMRECVCLIENT_STATE_LOOPING)
        return -1;

    clients[clientId]->isWaiting = true;
    Logger logger = Logger::getInstance(LOG4CPLUS_TEXT(DEFAULT_OUTPUT_LOGGER));
    LOG4CPLUS_TRACE(logger , "[CLIENT " << clientId << "] STATE = " << state_to_string(clients[clientId]->state));

    return 0;
}

int STRMRECVClient::resume(int clientId)
{
    if (clients[clientId]->state != STRMRECVCLIENT_STATE_LOOPING)
        return -1;

    clients[clientId]->isWaiting = false;
     Logger logger = Logger::getInstance(LOG4CPLUS_TEXT(DEFAULT_OUTPUT_LOGGER));
    LOG4CPLUS_TRACE(logger , "[CLIENT " << clientId << "] STATE = " << state_to_string(clients[clientId]->state));

    return 0;
}

void STRMRECVClient::stop(int clientId)
{
    if (clients[clientId]->state <= STRMRECVCLIENT_STATE_CLEANED)
        return;

    clients[clientId]->state = STRMRECVCLIENT_STATE_STOPPING;

    Logger logger = Logger::getInstance(LOG4CPLUS_TEXT(DEFAULT_OUTPUT_LOGGER));
    LOG4CPLUS_TRACE(logger, "[CLIENT " << clientId << "] STATE = " << state_to_string(clients[clientId]->state));
}

void STRMRECVClient::abort(int clientId)
{
    if (clients[clientId]->state <= STRMRECVCLIENT_STATE_CLEANED)
        return;

    clients[clientId]->state = STRMRECVCLIENT_STATE_ABORTING;
    Logger logger = Logger::getInstance(LOG4CPLUS_TEXT(DEFAULT_OUTPUT_LOGGER));
    LOG4CPLUS_TRACE(logger , "[CLIENT " << clientId << "] STATE = " << state_to_string(clients[clientId]->state));
}

int STRMRECVClient::destroy(int clientId)
{
    STRMRECVClientStruct *pClient = clients[clientId];

    // WARNING! Not thread safe!
    if (pClient->state > STRMRECVCLIENT_STATE_CLEANED)
        return -1;

    clients[clientId] = NULL;

    for (int i = 0; i < STRMRECVCLIENT_FRAME_QUEUE_LEN; i++)
    {
        av_free(pClient->frameQueue[i]);
        pClient->frameQueue[i] = NULL;
    }

    delete pClient;

    return 0;
}

void STRMRECVClient::logState(int clientId)
{
    if (clientId > 0)
    {
        Logger logger = Logger::getInstance(LOG4CPLUS_TEXT(DEFAULT_OUTPUT_LOGGER));
        LOG4CPLUS_INFO(logger , "[CLIENT " << clientId << "] STATE = " << state_to_string(clients[clientId]->state));
        printf("STATE: %s\n", state_to_string(clients[clientId]->state));
        return;
    }

    // print all instantiated client's state
    for (int i = 0; i < STRMRECVCLIENT_MAX_CLIENT_NUM; i++)
    {
        if (clients[i] != NULL){
            Logger outputLogger = Logger::getInstance(LOG4CPLUS_TEXT(DEFAULT_OUTPUT_LOGGER));
            LOG4CPLUS_INFO(outputLogger, "[CLIENT " << i << "] STATE = " << state_to_string(clients[i]->state));
        }
    }

    // dump information about file onto standard error
    //av_dump_format(_pFormatCtx, 0, address.c_str(), 0);
}

}
