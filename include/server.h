#pragma once

#include <string>
#include <istream>
#include "RequestHandler.h"
#include "request.h"
#include "response.h"
#include "ThreadPool.h"
#include "ptrs.h"

namespace SonaHttp 
{

class ServerImpl;

class Server 
{
public:
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    explicit Server(std::istream& config, size_t thread_pool_size = 10);

    explicit Server(boost::asio::io_service& service, std::istream& config, size_t thread_pool_size = 10);
    ~Server();

    void run(size_t thread_number = 1);

    void stop();

    void addHandler(const std::string& path, RequestHandler* handle) 
    {
        request_handler_.addSubHandler(path, handle);
    }


    void deliverRequest(RequestPtr req) 
    {
        auto res = std::make_shared<Response>(req->connection());
        req->discardConnection();
        request_handler_.handleRequest(req, res);
        if(req->keepAlive() && !res->getHeader("Connection"))
            res->addHeader("Connection", "keep-alive");
    }
   
    boost::asio::io_service& service() 
    {
        return service_;
    }
    
    template<typename _fCallable, typename... _tParams>
    auto enqueue(_fCallable&& f, _tParams&&... args) 
    {
        return thread_pool_.enqueue(std::forward<_fCallable>(f), std::forward<_tParams>(args)...);
    }

private:
    std::unique_ptr<ServerImpl> pimpl_;
    boost::asio::io_service& service_;
    RequestHandler request_handler_;
    size_t thread_pool_size_;
    ThreadPool thread_pool_;
    void startAccept();
    void do_await_stop();
    void handleRequest(ConnectionPtr req);
};

}    /**< namespace SonaHttp */
