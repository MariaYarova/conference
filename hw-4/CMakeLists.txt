cmake_minimum_required(VERSION 3.2)

project(hl_event_server C CXX)

SET (EXAMPLE_BINARY "hl_mai_lab_01")

find_package(OpenSSL)
find_package(GTest REQUIRED)
find_package(Threads)
find_package(ZLIB)
find_package(CppKafka REQUIRED)
find_package(Poco REQUIRED COMPONENTS Foundation Util Net XML JSON Crypto NetSSL)
find_package(Boost COMPONENTS filesystem system program_options regex REQUIRED)


if(NOT ${Poco_FOUND})
    message(FATAL_ERROR "Poco C++ Libraries not found.")
endif()
include_directories(${Poco_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIR})

add_executable(${EXAMPLE_BINARY} main.cpp 
                                 config/config.cpp 
                                 database/database.cpp
                                 database/author.cpp
                                 database/article.cpp)
target_include_directories(${EXAMPLE_BINARY} PRIVATE "${CMAKE_BINARY_DIR}")
target_compile_options(${EXAMPLE_BINARY} PRIVATE -Wall -Wextra -pedantic -Werror )
target_link_libraries(${EXAMPLE_BINARY} PRIVATE 
                             ${CMAKE_THREAD_LIBS_INIT} 
                             ${Poco_LIBRARIES}
                             "PocoData"
                             "PocoDataMySQL"
                             "mysqlclient"
                             "rdkafka++"
                             CppKafka::cppkafka
                             ZLIB::ZLIB)
set_target_properties(${EXAMPLE_BINARY} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${EXAMPLE_BINARY} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

# event_writer
SET (WRITER_BINARY "event_writer")
add_executable(${WRITER_BINARY} writer_main.cpp 
                                config/config.cpp 
                                database/database.cpp
                                database/author.cpp)
target_include_directories(${WRITER_BINARY} PRIVATE "${CMAKE_BINARY_DIR}")
target_compile_options(${WRITER_BINARY} PRIVATE -Wall -Wextra -pedantic -Werror )
target_link_libraries(${WRITER_BINARY} PRIVATE 
                             ${CMAKE_THREAD_LIBS_INIT} 
                             ${Poco_LIBRARIES}
                             "PocoData"
                             "PocoDataMySQL"
                             "rdkafka++"
                             CppKafka::cppkafka
                             ZLIB::ZLIB
                             ${Boost_LIBRARIES})
set_target_properties(${WRITER_BINARY} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${WRITER_BINARY} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
                                                          