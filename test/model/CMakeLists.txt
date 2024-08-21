aux_source_directory(${CMAKE_SOURCE_DIR}/src/database/mysql/ MYSQL_SOURCE_FILES)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/log/ Log_SOURCE_FILES)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/model/ MODEL_SOURCE_FILES)

add_executable(test_friendmodel test_friendmodel.cpp ${MYSQL_SOURCE_FILES} ${Log_SOURCE_FILES} ${MODEL_SOURCE_FILES})
add_executable(test_groupmodel test_groupmodel.cpp ${MYSQL_SOURCE_FILES} ${Log_SOURCE_FILES} ${MODEL_SOURCE_FILES})
add_executable(test_offlinemessagemodel test_offlinemessagemodel.cpp ${MYSQL_SOURCE_FILES} ${Log_SOURCE_FILES} ${MODEL_SOURCE_FILES})
add_executable(test_usermodel test_usermodel.cpp ${MYSQL_SOURCE_FILES} ${Log_SOURCE_FILES} ${MODEL_SOURCE_FILES})

target_link_libraries(test_friendmodel mysqlcppconn)
target_link_libraries(test_groupmodel mysqlcppconn)
target_link_libraries(test_offlinemessagemodel mysqlcppconn)
target_link_libraries(test_usermodel mysqlcppconn)