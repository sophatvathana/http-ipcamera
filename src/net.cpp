/*
* @Author: sophatvathana
* @Date:   2017-01-12 12:51:51
* @Last Modified by:   sophatvathana
* @Last Modified time: 2017-01-12 13:19:53
*/
#include "net.h"
#include "TcpConnection.h"

namespace SonaHttp 
{
namespace 
{
void transfer_data(ConnectionPtr conn1, ConnectionPtr conn2)
{
    if(conn1->stoped() || conn2->stoped())
        return;
    conn1->asyncRead(boost::asio::transfer_at_least(1), 
        [=](const boost::system::error_code& err, size_t n)
        { 
            if(err) 
            {
                conn1->stop();
                conn2->stop();
                return;
            }
            std::stringstream ss; 
            ss << &conn1->readBuffer();
            conn2->asyncWrite(ss.str(), [=](const boost::system::error_code& e, size_t n) 
            {
                if(e) 
                {
                    conn1->stop();
                    conn2->stop();
                }
            });
            transfer_data(conn1, conn2);
        }
    );
}

void 
async_connect(ConnectionPtr conn, const std::string& host,
    const std::string& port, std::function<void(ConnectionPtr)> handler)
{
    conn->asyncConnect(host, port, [=](ConnectionPtr c) 
    {
        if(c) 
        {
            handler(c);
        } 
        else 
        {
            Log("ERROR") << "Connect to " << host << ":" << port << " failed";
            handler(nullptr);
        }
    });
}
}

void TcpConnect(boost::asio::io_service& service, const std::string& host,
    const std::string& port, std::function<void(ConnectionPtr)> handler)
{
    async_connect(std::make_shared<TcpConnection>(service), host, port, handler);
}

void tunnel(ConnectionPtr conn1, ConnectionPtr conn2)
{
    transfer_data(conn1, conn2);
    transfer_data(conn2, conn1);
}

}    /**< namespace SonaHttp */
