/*
* @Author: sophatvathana
* @Date:   2017-01-12 12:51:24
* @Last Modified by:   sophatvathana
* @Last Modified time: 2017-01-12 13:20:00
*/
#include "connection.h"

namespace SonaHttp 
{
using boost::system::error_code;

#define ASYNC_APPLY(op, func, handler, ...)                             \
enqueue##op([=, ptr = shared_from_this()]                               \
{                                                                       \
    auto handle = [this, handler, ptr](const error_code& e, size_t n)   \
    {                                                                   \
        handler(e, n);                                                  \
        dequeue##op();                                                  \
    };                                                                  \
    func(__VA_ARGS__, handle);                                          \
})

Connection::~Connection() 
{
}

void 
Connection::asyncRead(std::function<size_t(const error_code &, size_t)> completion,
    std::function<void(const error_code &, size_t)> handler) 
{
    ASYNC_APPLY(Read, async_read, handler, completion);
}

void 
Connection::asyncReadUntil(const std::string& delim, 
    std::function<void(const error_code &, size_t)> handler)
{
    ASYNC_APPLY(Read, async_read_until, handler, delim);
}
    
void 
Connection::asyncWrite(const std::string& msg, 
    std::function<void(const error_code&, size_t)> handler)
{
    ASYNC_APPLY(Write, async_write, handler, msg);
}

void
Connection::rawWrite(char * data, int size, std::function<void(const error_code&, size_t)> handler){
    //ASYNC_APPLY(Write, raw_write, handler, data, size);
    raw_write(data, size, handler);
}

}    /**< namespace SonaHttp */
