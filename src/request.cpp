/*
* @Author: sophatvathana
* @Date:   2017-01-11 13:18:39
* @Last Modified by:   sophatvathana
* @Last Modified time: 2017-01-12 13:20:51
*/
#include "request.h"
#include "connection.h"
#include "utils.h"
#include "base64.h"
#include <boost/regex.hpp>

namespace SonaHttp {

namespace {

std::string auth_info(const std::string auth)
{
    static const boost::regex basic_auth_reg("[Bb]asic ([[:print:]]*)");
    boost::smatch results;
    if(boost::regex_search(auth, results, basic_auth_reg)) 
    {
        return Base64::decode(results.str(1));
    } 
    else 
    {
        return {};
    }
}
}

void Request::flush()
{

    if(connection() == nullptr)
        return;
    if(!chunked()) 
    {        
    
        setChunked();
        if(query_ == "") 
        {
            Log("NOTE") << method_ << " " << path_ << " " << version_;
            connection()->asyncWrite(method_ + " " + path_ + " " + version_ + "\r\n");
        } 
        else 
        {
            Log("NOTE") << method_ << " " << path_ << "?" << query_ << " " << version_;
            connection()->asyncWrite(method_ + " " + path_ + "?" + query_ + " " + version_ + "\r\n");
        }
    }
    flushPackage();
}

std::string Request::basicAuthInfo()
{
    auto auth = getHeader("Authorization"); 
    if(!auth) return {};
    return auth_info(*auth);
}

std::string Request::proxyAuthInfo()
{
    auto auth = getHeader("Proxy-Authorization"); 
    if(!auth) return {};
    return auth_info(*auth);
}

void Request::basicAuth(const std::string& auth)
{
    setHeader("Authorization", "Basic " + Base64::encode(auth));
}

Request::~Request()
{
    if(connection() == nullptr)
        return;
    try 
    {
        if(!chunked()) 
        {
            if(query_ == "") 
            {
                Log("NOTE") << method_ << " " << path_ << " " << version_;
                connection()->asyncWrite(method_ + " " + path_ + " " + version_ + "\r\n");
            } 
            else 
            {
                Log("NOTE") << method_ << " " << path_ << "?" << query_ << " " << version_;
                connection()->asyncWrite(method_ + " " + path_ + "?" + query_ + " " + version_ + "\r\n");
            }
        } 
        else 
        {
            flushPackage();
            connection()->asyncWrite("0\r\n\r\n");
        }
        flushPackage();
    } 
    catch(std::exception& e) 
    {
        fprintf(stderr, "%s\n", e.what());
    }
}

}    /**< namespace SonaHttp */        
