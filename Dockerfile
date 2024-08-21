FROM ubuntu:22.04
ENV TARGET_APP=server

RUN apt update
RUN apt install -y build-essential cmake libmysqlcppconn-dev


RUN mkdir -p /chat_server
COPY . /chat_server

# build muduo
RUN mkdir -p /chat_server/dependencies/muduo/build
RUN rm -rf /chat_server/dependencies/muduo/build/*
WORKDIR /chat_server/dependencies/muduo/build
RUN cmake -DMUDUO_BUILD_EXAMPLES=OFF ..
RUN make -j2
RUN make install

# build hiredis
RUN mkdir -p /chat_server/dependencies/hiredis/build
RUN rm -rf /chat_server/dependencies/hiredis/build/*
WORKDIR /chat_server/dependencies/hiredis/build
RUN cmake ..
RUN make -j2
RUN make install

# build redis-plus-plus
RUN mkdir -p /chat_server/dependencies/redis-plus-plus/build
RUN rm -rf /chat_server/dependencies/redis-plus-plus/build/*
WORKDIR /chat_server/dependencies/redis-plus-plus/build
RUN cmake -DREDIS_PLUS_PLUS_CXX_STANDARD=11 -DREDIS_PLUS_PLUS_BUILD_STATIC=OFF -DREDIS_PLUS_PLUS_BUILD_TEST=OFF ..
RUN make -j2
RUN make install

# build chat_server
RUN mkdir -p /chat_server/build
RUN rm -rf /chat_server/build/*
RUN rm -rf /chat_server/bin/*
WORKDIR /chat_server/build
RUN cmake -DCMAKE_BUILD_TYPE=release -Dchat_server_DISABLE_EXAMPLES=ON -Dchat_server_DISABLE_TESTS=ON ..
# RUN cmake -DCMAKE_BUILD_TYPE=release ..
RUN make -j2

# 修改容器中的数据库ip地址
RUN sed -i 's/localhost/mysql/g' /chat_server/bin/mysql.cnf
RUN sed -i 's/localhost/redis/g' /chat_server/bin/redis.cnf

WORKDIR /chat_server/bin
RUN chmod +x /chat_server/entrypoint.sh
ENTRYPOINT ["/chat_server/entrypoint.sh"]
# CMD ["/bin/bash", "-i"]
