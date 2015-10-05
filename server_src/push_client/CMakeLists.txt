cmake_minimum_required(VERSION 2.6)
PROJECT(push_client)

AUX_SOURCE_DIRECTORY(./ SRC_LIST)

#SET(LIBRARY_OUTPUT_PATH ./)
SET(EXECUTABLE_OUTPUT_PATH  ./)
if(APPLE)
    SET(PB_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../base/pb/lib/mac)
elseif(UNIX)
    SET(PB_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../base/pb/lib/linux)
endif()

SET(MYSQL_INCLUDE_DIR /usr/include/mysql)
SET(MYSQL_LIB /usr/lib64/mysql)

ADD_DEFINITIONS( -g -W -Wall -D_REENTRANT -D_FILE_OFFSET_BITS=64 -DAC_HAS_INFO
-DAC_HAS_WARNING -DAC_HAS_ERROR -DAC_HAS_CRITICAL -DTIXML_USE_STL
-DAC_HAS_DEBUG -DLINUX_DAEMON -std=c++11 -DENCRYPT)

INCLUDE_DIRECTORIES(./ ../base ../base/jsoncpp ../base/pb/protocol ../base/slog 
../db_proxy_server/
../base/pb/ ${MYSQL_INCLUDE_DIR})
LINK_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/ ${CMAKE_CURRENT_SOURCE_DIR}/../base ${PB_LIB_DIR} ${MYSQL_LIB} 
${CMAKE_CURRENT_SOURCE_DIR}/../base/slog/lib 
${CMAKE_CURRENT_SOURCE_DIR}/../base/security/lib/linux)

#ADD_XXXX必须在TARGET_LINK_LIBRARIES前面，否则会报错
#ADD_LIBRARY(${PROJECTNAME} SHARED/STATIC ${SRC_LIST})
ADD_EXECUTABLE(push_client ${SRC_LIST} ../db_proxy_server/AutoPool.cpp 
../db_proxy_server/CachePool.cpp ../db_proxy_server/DBPool.cpp)

TARGET_LINK_LIBRARIES(push_client pthread base protobuf-lite mysqlclient_r hiredis curl slog 
crypto security)
 
