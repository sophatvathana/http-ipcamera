/*
* @Author: sophatvathana
* @Date:   2017-01-12 13:10:45
* @Last Modified by:   sophatvathana
* @Last Modified time: 2017-01-12 13:17:02
*/
#include <cstring>
#include "loghandler.h"
#include "strmrecvclient.h"
#include "strmrecvclientapi.h"

using namespace loghandlerns;
using namespace strmrecvclientns;




STRMRECVAPI void STRMRECVCALL strmrecvclient_start_log(const char *outputLogFile, const char *errorLogFile)
{
    LogHandler::startLog(std::string(outputLogFile), std::string(errorLogFile));
}

STRMRECVAPI void STRMRECVCALL strmrecvclient_stop_log()
{
    LogHandler::stopLog();
}

STRMRECVAPI int STRMRECVCALL strmrecvclient_get_log_level()
{
    return LogHandler::getLogLevel();
}

STRMRECVAPI void STRMRECVCALL strmrecvclient_set_log_level(int logLevel)
{
    LogHandler::setLogLevel(logLevel);
}

STRMRECVAPI void STRMRECVCALL strmrecvclient_log_state(int clientId)
{
    STRMRECVClient::getInstance()->logState(clientId);
}

bool check_client(int clientId)
{
    if (clientId < 0 || clientId >= STRMRECVCLIENT_MAX_CLIENT_NUM)
    {
        LOG4CPLUS_ERROR("","");
        Logger loggerError = Logger::getInstance(LOG4CPLUS_TEXT(DEFAULT_ERROR_LOGGER));
        LOG4CPLUS_ERROR(loggerError, "[CLIENT " << clientId << "] client id NOT valid!");
        return false;
    }

    return true;
}

void startup_thread_loop(STRMRECVClientParameters *parameters)
{
    STRMRECVClient *instance = STRMRECVClient::getInstance();
    Logger logger = Logger::getInstance(LOG4CPLUS_TEXT(DEFAULT_OUTPUT_LOGGER));
    Logger loggerError = Logger::getInstance(LOG4CPLUS_TEXT(DEFAULT_ERROR_LOGGER));
    // wait initialization
    int wait = STRMRECVCLIENT_TIMEOUT;
    int clientId = ((STRMRECVClientParameters * ) parameters)->clientId;
    //if(instance->clients[clientId] != NULL){
      // start rtsp thread
      std::thread(STRMRECVClient::threadLoop, parameters).detach();
    //}
       
    LOG4CPLUS_DEBUG(logger, "[CLIENT " << clientId << "] thread started, waiting initialization...");

    while (wait-- > 0
           && (instance->clients[clientId] == NULL
           || (instance->clients[clientId]->state != STRMRECVCLIENT_STATE_LOOPING
           && instance->clients[clientId]->state != STRMRECVCLIENT_STATE_ERROR)))
    {
        if (instance->clients[clientId] != NULL)
            LOG4CPLUS_DEBUG(LOG4CPLUS_TEXT(DEFAULT_OUTPUT_LOGGER), "[CLIENT " << clientId << "] STATE = " << state_to_string(instance->clients[clientId]->state));
        LOG4CPLUS_TRACE(logger, "[CLIENT " << clientId << "] wait = " << wait);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // if not initialized quit
    if (instance->clients[clientId] == NULL
        || instance->clients[clientId]->state != STRMRECVCLIENT_STATE_LOOPING)
    {
        
        LOG4CPLUS_ERROR(loggerError, "[CLIENT " << clientId << "] rtsp thread not looping... something has gone wrong :(");
        LOG4CPLUS_ERROR(logger, "[CLIENT " << clientId << "] rtsp thread not looping... something has gone wrong :(");

        if (instance->clients[clientId] != NULL && instance->clients[clientId]->state > STRMRECVCLIENT_STATE_CLEANED)
        {
            instance->abort(clientId);

            wait = STRMRECVCLIENT_TIMEOUT;
            LOG4CPLUS_ERROR(loggerError, "[CLIENT " << clientId << "] aborting crazy...");
            LOG4CPLUS_ERROR(logger, "[CLIENT " << clientId << "] aborting crazy...");
            //STRMRECVClient::getInstance()->stop(clientId);
            while (wait-- > 0 && instance->clients[clientId]->state == STRMRECVCLIENT_STATE_ABORTING)
            {
                printf("INdex -> %d\n", wait);
                LOG4CPLUS_DEBUG(logger, "[CLIENT " << clientId << "] STATE = " << state_to_string(instance->clients[clientId]->state));
                LOG4CPLUS_TRACE(logger, "[CLIENT " << clientId << "] wait = " << wait);

                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        }

    }
}
STRMRECVAPI std::string STRMRECVCALL strmrecvclient_get_address(int clientId){
  if(STRMRECVClient::getInstance()->clients[clientId] != NULL)
    return STRMRECVClient::getInstance()->clients[clientId]->address;
  return "";
}

STRMRECVAPI int STRMRECVCALL strmrecvclient_start(int clientId, const char *address, int asyncCall)
{
    if (address == NULL)
    {
        LOG4CPLUS_ERROR(LOG4CPLUS_TEXT(DEFAULT_ERROR_LOGGER), "[CLIENT " << clientId << "] address is empty!");
        LOG4CPLUS_ERROR(LOG4CPLUS_TEXT(DEFAULT_OUTPUT_LOGGER), "[CLIENT " << clientId << "] address is empty!");

        return -1;
    }

    if (!check_client(clientId))
        return -1;

    // check that the client is not already started and is not doing anything
    if (STRMRECVClient::getInstance()->clients[clientId] != NULL && STRMRECVClient::getInstance()->clients[clientId]->state > STRMRECVCLIENT_STATE_CLEANED
      && STRMRECVClient::getInstance()->clients[clientId]->address == address)
    {
        LOG4CPLUS_WARN(LOG4CPLUS_TEXT(DEFAULT_OUTPUT_LOGGER), "[CLIENT " << clientId << "] client already started.");
        return 0;
    }

    STRMRECVClientParameters *parameters = new STRMRECVClientParameters;
    parameters->clientId = clientId;
    parameters->address = address;
    LOG4CPLUS_DEBUG(LOG4CPLUS_TEXT(DEFAULT_OUTPUT_LOGGER), "[CLIENT " << clientId << "] starting " << (asyncCall == 1 ? "asynchronously" : "synchronously") << "...");

    if (asyncCall == 1)
    {
        // start a startup thread
        std::thread(startup_thread_loop, parameters).detach();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    else
        startup_thread_loop(parameters);

    return 0;
}

STRMRECVAPI int STRMRECVCALL strmrecvclient_wait(int clientId)
{
    if (!check_client(clientId))
        return -1;

    return STRMRECVClient::getInstance()->wait(clientId);
}

STRMRECVAPI int STRMRECVCALL strmrecvclient_resume(int clientId)
{
    if (!check_client(clientId))
        return -1;

    return STRMRECVClient::getInstance()->resume(clientId);
}

STRMRECVAPI int STRMRECVCALL strmrecvclient_stop(int clientId)
{
    printf("Stop is working\n");
    if (!check_client(clientId))
        return -1;

    STRMRECVClient *instance = STRMRECVClient::getInstance();
    STRMRECVClientStruct *pClient = instance->clients[clientId];

    if (pClient == NULL || pClient->state <= STRMRECVCLIENT_STATE_CLEANED)
        return 0;

    pClient->state = STRMRECVCLIENT_STATE_STOPPING;

    LOG4CPLUS_DEBUG(LOG4CPLUS_TEXT(DEFAULT_OUTPUT_LOGGER), "[CLIENT " << clientId << "] waiting rtsp thread terminating...");

    int wait = STRMRECVCLIENT_TIMEOUT;

    while (wait-- > 0 && (pClient->state != STRMRECVCLIENT_STATE_CLEANED && pClient->state != STRMRECVCLIENT_STATE_ERROR))
        std::this_thread::sleep_for(std::chrono::seconds(1));

    if (pClient->state == STRMRECVCLIENT_STATE_CLEANED || pClient->state == STRMRECVCLIENT_STATE_ERROR)
        LOG4CPLUS_DEBUG(LOG4CPLUS_TEXT(DEFAULT_OUTPUT_LOGGER), "[CLIENT " << clientId << "] thread stopped");
    else
        LOG4CPLUS_ERROR(LOG4CPLUS_TEXT(DEFAULT_ERROR_LOGGER), "[CLIENT " << clientId << "] stop FAILED!");

    LOG4CPLUS_DEBUG(LOG4CPLUS_TEXT(DEFAULT_OUTPUT_LOGGER), "[CLIENT " << clientId << "] STATE = " << state_to_string(pClient->state));

    return pClient->state > STRMRECVCLIENT_STATE_CLEANED ? -1 : 0;
}

STRMRECVAPI int STRMRECVCALL strmrecvclient_destroy(int clientId)
{
    if (!check_client(clientId))
        return -1;

    STRMRECVClient *instance = STRMRECVClient::getInstance();
    STRMRECVClientStruct *pClient = instance->clients[clientId];

    if (pClient == NULL)
        return 0;

    return instance->destroy(clientId);
}

STRMRECVAPI int STRMRECVCALL strmrecvclient_get_data(int clientId, int frames, int fps, STRMRECVClientData *data)
{
    if (!check_client(clientId))
        return -1;

    STRMRECVClient *instance = STRMRECVClient::getInstance();
    STRMRECVClientStruct *pClient = instance->clients[clientId];

    if (pClient == NULL)
        return 0;

    frames = frames <= 0 ? pClient->newFramesNum : std::min(frames, STRMRECVCLIENT_FRAME_QUEUE_LEN);

    data->clientId = pClient->clientId;
    data->state = pClient->state;
    data->isWaiting = pClient->isWaiting == true ? 1 : 0;
    data->next = pClient->next;

    int cur = pClient->next - frames;
    if (cur < 0)
        cur += STRMRECVCLIENT_FRAME_QUEUE_LEN;

    int step = 1;

    if (fps > 0)
        step = (int ) std::max(ceil((double ) frames / (double ) fps), 1.0);

    data->framesRead = 0;

    for (int i = 0; i * step < frames; i++)
    {
        std::memcpy((char * ) &data->frameQueue[i * STRMRECVCLIENT_FRAME_BUFFER_SIZE], (char * ) pClient->frameQueue[(cur + i * step) % STRMRECVCLIENT_FRAME_QUEUE_LEN], pClient->frameSizes[(cur + i * step) % STRMRECVCLIENT_FRAME_QUEUE_LEN]);
        data->frameSizes[i] = pClient->frameSizes[(cur + i * step) % STRMRECVCLIENT_FRAME_QUEUE_LEN];
        data->framesRead++;
    }

    pClient->newFramesNum = 0;

    return 0;
}

STRMRECVAPI int STRMRECVCALL strmrecvclient_get_state(int clientId)
{
    if (!check_client(clientId))
        return -1;

    STRMRECVClientStruct *pClient = STRMRECVClient::getInstance()->clients[clientId];

    if (pClient == NULL)
        return STRMRECVCLIENT_STATE_EMPTY;

    return pClient->state;
}
