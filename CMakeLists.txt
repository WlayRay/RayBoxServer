cmake_minimum_required(VERSION 3.10)
project(chat_server C CXX)
option(chat_server_DISABLE_EXAMPLES "Disable build of chat_server examples." OFF)
option(chat_server_DISABLE_TESTS "Disable build of chat_server tests." OFF)

# Enable debug
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g) 

# <------------ copy configuration file to bin --------------->
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# Set the source directory of your configuration files
set(CONFIG_FILES
    ${PROJECT_SOURCE_DIR}/mysql.cnf
    ${PROJECT_SOURCE_DIR}/redis.cnf
    # ${PROJECT_SOURCE_DIR}/config_file3.txt
)

# Set the destination directory of your configuration files in the build directory
set(CONFIG_FILES_DESTINATION ${PROJECT_SOURCE_DIR}/bin)

# Copy the configuration files from the source directory to the build directory
file(COPY ${CONFIG_FILES} DESTINATION ${CONFIG_FILES_DESTINATION})



# <---------- set c++ standard ------------->
# NOTE: you must build redis-plus-plus and your application code with the same standard.
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# <------------ add muduo dependency --------------->
find_path(MUDUO_HEADER muduo REQUIRED)
find_library(MUDUO_BASE_LIB muduo_base REQUIRED)
find_library(MUDUO_NET_LIB muduo_net REQUIRED)

message("Found MUDUO_HEADER: ${MUDUO_HEADER}")
message("Found MUDUO_BASE_LIB: ${MUDUO_BASE_LIB}")
message("Found MUDUO_NET_LIB: ${MUDUO_NET_LIB}")
include_directories(${MUDUO_HEADER})

# <------------ add hiredis dependency --------------->
find_path(HIREDIS_HEADER hiredis REQUIRED)
find_library(HIREDIS_LIB hiredis REQUIRED)

message("Found HIREDIS_HEADER: ${HIREDIS_HEADER}")
message("Found HIREDIS_LIB: ${HIREDIS_LIB}")
include_directories(${HIREDIS_HEADER})

# <------------ add redis-plus-plus dependency -------------->
# NOTE: this should be *sw* NOT *redis++*
find_path(REDIS_PLUS_PLUS_HEADER sw REQUIRED)
find_library(REDIS_PLUS_PLUS_LIB redis++ REQUIRED)

message("Found REDIS_PLUS_PLUS_HEADER: ${REDIS_PLUS_PLUS_HEADER}")
message("Found REDIS_PLUS_PLUS_LIB: ${REDIS_PLUS_PLUS_LIB}")
include_directories(${REDIS_PLUS_PLUS_HEADER})


include_directories(thirdparty)
include_directories(src/include)
include_directories(src/include/database)
include_directories(src/include/database/mysql)
include_directories(src/include/log)
include_directories(src/include/model)
include_directories(src/include/threadpool)
include_directories(src/include/service)
include_directories(src/include/server)
include_directories(src/include/redis)


add_subdirectory(src)
# Add the example subdirectory if the option is not disabled
if (NOT chat_server_DISABLE_EXAMPLES)
endif()

# Add the test subdirectory if the option is not disabled
if (NOT chat_server_DISABLE_TESTS)
    add_subdirectory(test)
endif()
