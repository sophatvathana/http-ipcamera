#pragma once

#include <functional>
#include "ptrs.h"

namespace SonaHttp 
{

void parseRequest(ConnectionPtr conn, std::function<void(RequestPtr)> handler);

void parseResponse(ConnectionPtr conn, std::function<void(ResponsePtr)> handler);

}    /**< namespace SonaHttp */
