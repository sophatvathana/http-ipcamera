/*
* @Author: sophatvathana
* @Date:   2017-01-12 12:52:44
* @Last Modified by:   sophatvathana
* @Last Modified time: 2017-01-12 13:20:49
*/
#include "server.h"
#include "parser.h"
#include "TcpConnection.h"
#include "log.h"
#include <thread>
#include <vector>
#include <csignal>
#include <utility>
#include <random>
#include <ctime>
#include <cstring>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace SonaHttp 
{
namespace 
{
using boost::property_tree::read_json;
using boost::property_tree::ptree;

template<typename T>
T parse_config(const ptree& t, const std::string& key, 
    std::function<void(std::exception& e, T&)> exception_callback = 
    [](std::exception& e, T& retval) 
    {
        retval = T{};
    })
{
    T ret{};
    try 
    {
        ret = t.get<T>(key);
    } 
    catch(std::exception& e) 
    {
        exception_callback(e, ret);
    }
    return ret;
}

std::string generateId(size_t max_size)
{
    std::string ret;
    std::default_random_engine e(time(nullptr));
    std::uniform_int_distribution<char> c('0', '9');
    size_t length = e();
    length %= max_size;
    for(size_t i = 0; i < length; ++i)
        ret.push_back(c(e));
    return ret;
}    
}

class ServerImpl 
{
    friend class Server;
public:
    ServerImpl(boost::asio::io_service& service) 
        : service_(service), signals_(service), tcp_acceptor_(service), 
        ssl_acceptor_(service)
    {}

    static boost::asio::io_service& common_service()
    {
        static std::unique_ptr<boost::asio::io_service> service_ptr = std::make_unique<boost::asio::io_service>();
        service_ptr->stop();
        while(!service_ptr->stopped())
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        service_ptr = std::make_unique<boost::asio::io_service>();
        return *service_ptr;
    }

private:
    boost::asio::io_service& service_;
    boost::asio::signal_set signals_;
    boost::asio::ip::tcp::acceptor tcp_acceptor_;
    boost::asio::ip::tcp::acceptor ssl_acceptor_;
    static boost::asio::ssl::context ssl_context_;
    TcpConnectionPtr new_tcp_connection_;
    //SslConnectionPtr new_ssl_connection_;

    void handleTcpAccept(std::function<void(ConnectionPtr)> request_handler, const boost::system::error_code& ec);
    void handleSslAccept(std::function<void(ConnectionPtr)> request_handler, const boost::system::error_code& ec);
};
    
//boost::asio::ssl::context ServerImpl::ssl_context_{boost::asio::ssl::context::sslv23};

void ServerImpl::handleTcpAccept(std::function<void(ConnectionPtr)> request_handler, const boost::system::error_code& ec)
{
    if(!ec) 
    {
        request_handler(new_tcp_connection_);
        new_tcp_connection_.reset(new TcpConnection(service_));
        tcp_acceptor_.async_accept(new_tcp_connection_->nativeSocket(),
            std::bind(&ServerImpl::handleTcpAccept, this, 
            request_handler, std::placeholders::_1));
    } 
    else 
    {
        if(ec != boost::asio::error::operation_aborted)
            Log("ERROR") << ec.message();
    }
}

Server::Server(std::istream& config, size_t thread_pool_size)
    : Server(ServerImpl::common_service(), config, thread_pool_size)
{
}

Server::Server(boost::asio::io_service& service, std::istream& config, size_t thread_pool_size)
    : pimpl_(std::make_unique<ServerImpl>(service)), service_(service),
    thread_pool_size_(thread_pool_size), thread_pool_(thread_pool_size)
{
    pimpl_->signals_.add(SIGINT);
    pimpl_->signals_.add(SIGTERM);
#if defined(SIGQUIT)
    pimpl_->signals_.add(SIGQUIT);
#endif

    do_await_stop();
    ptree conf;
    read_json(config, conf);
    std::string http_port = parse_config<std::string>(conf, "http port",
        [](std::exception&, std::string&) 
        {
            Log("NOTE") << "no http port provide in config, disable http";
        }
    );
    std::string https_port = parse_config<std::string>(conf, "https port",
        [](std::exception&, std::string&) 
        {
            Log("NOTE") << "no https port provide in config";
        }
    );
    if(http_port != "") 
    {
        std::string http_server = parse_config<std::string>(conf, "http server",
            [](std::exception&, std::string& server) 
            {
                server = "0.0.0.0";
            }
        );
        boost::asio::ip::tcp::resolver resolver(service_);
        boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve({http_server, http_port});
        pimpl_->tcp_acceptor_.open(endpoint.protocol());
        pimpl_->tcp_acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        pimpl_->tcp_acceptor_.bind(endpoint);
        pimpl_->tcp_acceptor_.listen();
        pimpl_->new_tcp_connection_.reset(new TcpConnection(service_));
    }

    startAccept();
}

Server::~Server() {}

void Server::run(size_t thread_number)
{
    if(!thread_number)
        return;

    std::vector<std::thread> threads;
    for(size_t i = 0; i < thread_number; ++i) 
    {
        threads.push_back(std::thread([this] 
        {
            service_.run();
        }));
    }
    for(auto&& th : threads)
        th.join();
    service_.reset();
}

void Server::stop()
{
    Log("NOTE") << "សូមជំរាបលា";
    service_.stop();       
}

void Server::handleRequest(ConnectionPtr conn)
{
    assert(conn != nullptr);
    parseRequest(conn, [=](RequestPtr request) 
    {
        if(request) 
        {
            if(request->keepAlive()) 
            {
                handleRequest(conn);
            }
            thread_pool_.wait_to_enqueue([this](std::unique_lock<std::mutex>&) 
            {
                if(thread_pool_.size_unlocked() > thread_pool_size_)
                    Log("WARNING") << "thread pool overload";
            }, &Server::deliverRequest, this, request);
        } 
        else 
        {
            conn->stop();
        }
    });
}

void Server::startAccept()
{
    if(pimpl_->new_tcp_connection_) 
    {
        pimpl_->tcp_acceptor_.async_accept(pimpl_->new_tcp_connection_->nativeSocket(),
            [this](const boost::system::error_code& ec) 
            {
                pimpl_->handleTcpAccept(
                    std::bind(&Server::handleRequest, this, std::placeholders::_1),
                    ec
                );
            }
        );
    }

}    

void Server::do_await_stop()
{
    pimpl_->signals_.async_wait([this](boost::system::error_code /*ec*/, int /*signo*/) 
    {
        stop();
    });
}

}    /**< namespace SonaHttp */
