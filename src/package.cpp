/*
* @Author: sophatvathana
* @Date:   2017-01-12 12:51:53
* @Last Modified by:   sophatvathana
* @Last Modified time: 2017-01-12 13:20:35
*/
#include "package.h"
#include "connection.h"
#include <boost/lexical_cast.hpp>

namespace SonaHttp 
{
Package::~Package()
{
}

void
Package::flushPackage()
{
    std::stringstream send_buf;

    if(!send_started_)                      
    {        
        if(chunked_) 
        {
            addHeader("Transfer-Encoding", "chunked");
        } 
        else 
        {
            auto h = getHeader("Content-Length");
            if(h == nullptr) 
                addHeader("Content-Length", boost::lexical_cast<std::string>(contentLength()));
        }
        for(auto h : getHeaderMap())
            send_buf << h.name << ": " << h.value << "\r\n";
        send_buf << "\r\n";
        send_started_ = true;
    }

    if(body.rdbuf()->in_avail()) 
    {
        if(chunked_) 
        {
            send_buf << std::hex << contentLength() << "\r\n";
            send_buf << body.rdbuf() << "\r\n";
        } 
        else 
        {
            send_buf << body.rdbuf();
        }    
    }

    connection()->asyncWrite(send_buf.str());
}

const char* Package::connectionType()
{
    return connection_->type();
}
}    /**< namespace SonaHttp */
