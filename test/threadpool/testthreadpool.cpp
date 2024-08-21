#include <chrono>
#include <iostream>
#include <thread>

#include "threadpool.h"

int sum1(int a, int b) {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return a + b;
}
int sum2(int a, int b, int c) {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return a + b + c;
}

int main() {
    ThreadPool pool;
    pool.start(2);

    std::future<int> r1 = pool.submit(sum1, 1, 2);
    std::future<int> r2 = pool.submit(sum2, 1, 2, 3);
    std::future<int> r3 = pool.submit(
        [](int b, int e) -> int {
            int sum = 0;
            for (int i = b; i <= e; i++) sum += i;
            return sum;
        },
        1, 100);

    std::cout << r1.get() << std::endl;
    std::cout << r2.get() << std::endl;
    std::cout << r3.get() << std::endl;
}
