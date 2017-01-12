/*
* @Author: sophatvathana
* @Date:   2017-01-12 12:52:04
* @Last Modified by:   sophatvathana
* @Last Modified time: 2017-01-12 13:20:38
*/
#include "RequestHandler.h"
#include "response.h"
#include "request.h"
#include "server.h"
#include "utils.h"
#include "log.h"
#include "connection.h"
#include <fstream>
#include <sstream>
#include <string>

namespace SonaHttp 
{

namespace 
{
const char *allowed_method[] = 
{
    "GET",
    "POST",
    "PUT",
    "DELETE",
    "CONNECT"
};

bool method_implemented(const std::string& method)
{
    for(const char* m : allowed_method) 
    {
        if(m == method)
            return true;
    }
    return false;
}
}

RequestHandler::~RequestHandler()
{
}

void RequestHandler::handleRequest(RequestPtr req, ResponsePtr rep)
{
    try 
    {
        if(!method_implemented(req->getMethod())) 
        {
            rep->setStatus(Response::Not_Implemented);
            return;
        }
        if(!deliverRequest(req, rep)) 
        {
            rep->setStatus(Response::Not_Found);
            return;
        }
    } 
    catch(std::exception& e) 
    {
        Log("WARNING") << "EXCEPTION FROM USER HANDLER: " << e.what();
    }
}

bool RequestHandler::deliverRequest(RequestPtr req, ResponsePtr rep)
{
    std::tuple<std::string, RequestHandler*> best;

    for(auto handler : sub_handlers_) 
    {
        if(std::get<0>(handler).size() >= std::get<0>(best).size() &&
            isPathMatch(req->getPath(), std::get<0>(handler)))
            best = handler;
    }

    if(std::get<1>(best) == nullptr)
        return false;

    std::get<1>(best)->handleRequest(req, rep);
    return true;
}

}    /**< namespace SonaHttp */
