add_executable(testmuduo testmuduo.cpp)

target_link_libraries(testmuduo ${MUDUO_NET_LIB} ${MUDUO_BASE_LIB})