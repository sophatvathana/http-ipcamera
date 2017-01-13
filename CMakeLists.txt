cmake_minimum_required(VERSION 3.6)
project(cameralib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    include/AsyncWrapper.h
    include/base64.h
    include/buffer.h
    include/client.h
    include/connection.h
    include/cookie.h
    include/crypto.h
    include/exception.h
    include/header.h
    include/log.h
    include/loghandler.h
    include/MimeType.h
    include/net.h
    include/package.h
    include/parser.h
    include/ptrs.h
    include/request.h
    include/RequestHandler.h
    include/response.h
    include/server.h
    include/server.old.h
    include/server_http.h
    include/SonaHttp.h
    include/strmrecvclient.h
    include/strmrecvclientapi.h
    include/strmrecvdef.h
    include/TcpConnection.h
    include/ThreadPool.h
    include/utils.h
    src/base64.cpp
    src/client.cpp
    src/connection.cpp
    src/exception.cpp
    src/loghandler.cpp
    src/main.cpp
    src/MimeType.cpp
    src/net.cpp
    src/package.cpp
    src/parser.cpp
    src/request.cpp
    src/RequestHandler.cpp
    src/response.cpp
    src/runner.cpp
    src/server.cpp
    src/server.old.cpp
    src/strmjni.cpp
    src/strmrecvclient.cpp
    src/strmrecvclientapi.cpp
    src/TcpConnection.cpp
    src/ThreadPool.cpp
    src/utils.cpp
    config.h)

add_executable(cameralib ${SOURCE_FILES})