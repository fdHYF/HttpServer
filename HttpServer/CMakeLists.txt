cmake_minimum_required(VERSION 3.8)
SET(CMAKE_BUILD_TYPE "Debug") 
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb") 
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

project(WangTian)
set(CMAKE_CXX_STANDARD 11)
set(SOURCE_FILES
    lib/Condition.h
    lib/Mutex.h
    lib/Thread.h
    lib/Thread.cpp
    logging/Log.h
    logging/Log.cpp
    logging/LogFile.cpp
    logging/LogFile.h
    logging/LogStream.cpp
    logging/LogStream.h
    logging/LogWrite.cpp
    logging/LogWrite.h
    Epoll.cpp
    Epoll.h
    main.cpp
    Request.cpp
    Request.h
    Server.h
    ThreadPool.h
    ThreadPool.cpp
    Timer.cpp
    Timer.h
    )

add_executable(WangTian ${SOURCE_FILES})
TARGET_LINK_LIBRARIES(WangTian pthread)
