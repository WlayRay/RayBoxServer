aux_source_directory(${CMAKE_SOURCE_DIR}/src/database/mysql/ MYSQL_SOURCE_FILES)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/log/ Log_SOURCE_FILES)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/model/ MODEL_SOURCE_FILES)

add_executable(benchmark benchmark.cpp ${MYSQL_SOURCE_FILES} ${Log_SOURCE_FILES})
target_link_libraries(benchmark mysqlcppconn)