#include "logger.h"

#include <time.h>

#include <iostream>

Logger &Logger::GetInstance()
{
    static Logger logger;
    return logger;
}

void Logger::Log(std::string msg)
{
    _logQue.Push(msg);
}

Logger::Logger()
{
    std::thread writeLogTask([&]()
                             {
        for (;;) {
            time_t now = time(nullptr);
            tm *nowtm = localtime(&now);

            char file_name[128];
            snprintf(file_name, sizeof(file_name), "../logs/%d-%d-%d-log.txt",
                     nowtm->tm_year + 1900, nowtm->tm_mon + 1, nowtm->tm_mday);

            // 存在则追加(a)，不存在则创建该文件(+)
            FILE *pf = fopen(file_name, "a+");
            if (pf == nullptr) {
                std::cout << "logger file : " << file_name << " open error!"
                          << std::endl;
                exit(EXIT_FAILURE);
            }

            std::string msg = _logQue.Pop();

            char time_buf[128] = {0};
            snprintf(time_buf, sizeof(time_buf), "%d:%d:%d - ", nowtm->tm_hour,
                     nowtm->tm_min, nowtm->tm_sec);
            msg.insert(0, time_buf);
            msg.append("\n");

            fputs(msg.c_str(), pf);
            fclose(pf);
        } });
    writeLogTask.detach();
}
