add_executable(testredis testredis.cpp)
target_link_libraries(testredis ${HIREDIS_LIB} ${REDIS_PLUS_PLUS_LIB})

aux_source_directory(${CMAKE_SOURCE_DIR}/src/redis/ REDIS_SOURCE_FILES)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/log/ Log_SOURCE_FILES)
add_executable(testredispool testredispool.cpp ${REDIS_SOURCE_FILES} ${Log_SOURCE_FILES})
target_link_libraries(testredispool ${HIREDIS_LIB} ${REDIS_PLUS_PLUS_LIB})