#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

#include "logger.h"

int main() {
    LOG_INFO("this is a INFO log, %d", 42);
    LOG_ERROR("this is a ERROR log, %d", 42);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    exit(EXIT_SUCCESS);
}