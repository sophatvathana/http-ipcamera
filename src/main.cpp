#include <fstream>
#include <string>
#include <iostream>
#include <chrono>
#include <thread>
#include "strmrecvclientapi.h"
#include "server.h"


#define TEST_ADDRESS "rtsp://admin:12345@192.168.0.168/Streaming/Channels/102"
#define TEST_FRAME_NUM 100000000
#define TEST_FRAME_PER_LOOP 50

// int main()
// {

    // printf("Hello\n");
    // strmrecvclient_start_log("", "");

    // strmrecvclient_start(0, TEST_ADDRESS, 1);

    // int nframe = 0;
    // STRMRECVClientData *data = new STRMRECVClientData;

    // while (nframe < TEST_FRAME_NUM)
    // {
    //     strmrecvclient_log_state(0);

    //     data->state = strmrecvclient_get_state(0);

    //     if (data->state != STRMRECVCLIENT_STATE_LOOPING)
    //     {
    //         if (data->state < STRMRECVCLIENT_STATE_INITIALIZING)
    //             strmrecvclient_start(0, TEST_ADDRESS, 1);

    //         std::this_thread::sleep_for(std::chrono::seconds(1));

    //         continue;
    //     }

    //     if (strmrecvclient_wait(0) != 0)
    //         continue;

    //     strmrecvclient_get_data(0, TEST_FRAME_PER_LOOP, 0, data);

    //     strmrecvclient_resume(0);

    //     for (int i = 0; i < data->framesRead; i++)
    //     {
    //         if (data->frameSizes[i] < 1024){
    //             continue;
    //         }
    //         std::string name = "test";
    //         name += std::to_string(static_cast<long long>(nframe));
    //         name += ".jpg";
    //         std::ofstream(name, std::ios::binary).write((char * ) &data->frameQueue[i * STRMRECVCLIENT_FRAME_BUFFER_SIZE], data->frameSizes[i]);

    //         nframe++;
    //     }

    //     std::this_thread::sleep_for(std::chrono::seconds(1));
    // }

    // strmrecvclient_stop(0);

    // //delete data;

    // strmrecvclient_stop_log();

//     return 0;
// }
