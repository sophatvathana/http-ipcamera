#pragma once

#include <string>
#include <memory>
#include <vector>

#include "request.h"
#include "response.h"

namespace SonaHttp 
{

class Server;

class RequestHandler 
{
public:
    RequestHandler() = default;
    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;
    virtual ~RequestHandler();

    virtual void handleRequest(RequestPtr req, ResponsePtr res);


    bool deliverRequest(RequestPtr req, ResponsePtr res);

    void addSubHandler(const std::string& path, RequestHandler * handler) 
    {
        sub_handlers_.push_back(std::make_tuple(path, handler));
    }

private:
    std::vector<std::tuple<const std::string, RequestHandler *>> sub_handlers_;
};

}    /**< namespace SonaHttp */
