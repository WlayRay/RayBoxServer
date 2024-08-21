#include <bits/types/clock_t.h>

#include <ctime>
#include <memory>
#include <thread>

#include "mysqlconnection.h"
#include "mysqlconnectionpool.h"
using namespace std;
// 压力测试连接池的使用效果

int main() {
    try {
        MysqlConnection conn("localhost", 3306, "root", "123456", "test",
                             "utf8mb4");
        conn.update("delete from user");

        std::cout << "===================" << std::endl;
        // 不使用连接池
        clock_t begin1 = clock();
        auto worker = []() {
            for (int i = 0; i < 100; ++i) {
                MysqlConnection conn("localhost", 3306, "root", "123456",
                                     "test", "utf8mb4");
                conn.update(
                    "insert into user( name, age, sex) values( 'alice', 1, "
                    "'male')");
            }
        };
        std::thread t1(worker);
        std::thread t2(worker);
        std::thread t3(worker);
        std::thread t4(worker);
        std::thread t5(worker);

        t1.join();
        t2.join();
        t3.join();
        t4.join();
        t5.join();
        clock_t end1 = clock();
        std::cout << "不使用连接池：" << (end1 - begin1) / CLOCKS_PER_SEC
                  << " seconds" << std::endl;

        // 使用连接池
        conn.update("delete from user");
        clock_t begin2 = clock();
        auto worker2 = []() {
            MysqlConnectionPool& pool =
                MysqlConnectionPool::getInstance("mysql_test.cnf");
            for (int i = 0; i < 100; ++i) {
                std::shared_ptr<MysqlConnection> conn = pool.getConnection();
                conn->update(
                    "insert into user( name, age, sex) values( '测试名字', 1, "
                    "'male')");
            }
        };
        std::thread tt1(worker2);
        std::thread tt2(worker2);
        std::thread tt3(worker2);
        std::thread tt4(worker2);
        std::thread tt5(worker2);

        tt1.join();
        tt2.join();
        tt3.join();
        tt4.join();
        tt5.join();
        clock_t end2 = clock();
        std::cout << "使用连接池：" << (end2 - begin2) / CLOCKS_PER_SEC
                  << " seconds" << std::endl;
    } catch (sql::SQLException& e) {
        cout << "# ERR: SQLException in " << __FILE__;
        cout << "(" << __func__ << ") on line " << __LINE__ << endl;
        cout << "# ERR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << " )" << endl;
    }
    return 0;
}
