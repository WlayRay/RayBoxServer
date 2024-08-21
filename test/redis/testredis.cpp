#include <sw/redis++/redis++.h>

#include <cstdio>
#include <iostream>
#include <thread>

using namespace sw::redis;

// see https://github.com/sewenew/redis-plus-plus#api-reference
// for more details.

void connection_pool() {
    ConnectionOptions connection_options;
    connection_options.host = "127.0.0.1";  // Required.
    connection_options.port = 6666;  // Optional. The default port is 6379.
    connection_options.password = "auth";  // Optional. No password by default.
    connection_options.db = 1;  // Optional. Use the 0th database by default.

    // Optional. Timeout before we successfully send request to or receive
    // response from redis. By default, the timeout is 0ms, i.e. never timeout
    // and block until we send or receive successfuly. NOTE: if any command is
    // timed out, we throw a TimeoutError exception.
    connection_options.socket_timeout = std::chrono::milliseconds(200);

    // Connect to Redis server with a single connection.
    Redis redis1(connection_options);

    ConnectionPoolOptions pool_options;
    pool_options.size = 3;  // Pool size, i.e. max number of connections.

    // Optional. Max time to wait for a connection. 0ms by default, which means
    // wait forever. Say, the pool size is 3, while 4 threads try to fetch the
    // connection, one of them will be blocked.
    pool_options.wait_timeout = std::chrono::milliseconds(100);

    // Optional. Max lifetime of a connection. 0ms by default, which means never
    // expire the connection. If the connection has been created for a long
    // time, i.e. more than `connection_lifetime`, it will be expired and
    // reconnected.
    pool_options.connection_lifetime = std::chrono::minutes(10);

    // Connect to Redis server with a connection pool.
    Redis redis2(connection_options, pool_options);
}

void type_string() {
    // Create an Redis object, which is movable but NOT copyable.
    auto redis = Redis("tcp://127.0.0.1:6379");

    redis.set("hello", "hello, redis!");

    auto val1 = redis.get("invalid-key");
    auto val2 = redis.get("hello");
    if (val1) {
        // Dereference val to get the returned value of std::string type.
        std::cout << *val1 << std::endl;
    } else {
        // else key doesn't exist
        std::cout << "key doesn't exist" << std::endl;
    }

    if (val2) {
        // Dereference val to get the returned value of std::string type.
        std::cout << *val2 << std::endl;
    } else {
        // else key doesn't exist
        std::cout << "key doesn't exist" << std::endl;
    }
}

void type_list() {
    auto redis = Redis("tcp://127.0.0.1:6379");

    // std::vector<std::string> ==>> Redis LIST.
    std::vector<std::string> vec = {"c", "cpp", "rust"};
    redis.rpush("languages", vec.begin(), vec.end());

    // std::initializer_list ==>> Redis LIST.
    redis.rpush("languages", {"java", "python", "go"});

    // Redis LIST ==> std::vector<std::string>.
    vec.clear();
    redis.lrange("languages", 0, -1, std::back_inserter(vec));
    for (auto& s : vec) {
        std::cout << s << std::endl;
    }
}

void type_hash() {
    auto redis = Redis("tcp://127.0.0.1:6379");
    redis.hset("student", "name", "Eric Ling");
    redis.hset("student", std::make_pair("age", "23"));

    // std::unordered_map<std::string, std::string> ==>> Redis HASH.
    std::unordered_map<std::string, std::string> m = {{"gender", "male"},
                                                      {"hobby", "linux"}};
    redis.hmset("student", m.begin(), m.end());

    // Redis HASH ==>> std::unordered_map<std::string, std::string>.
    m.clear();
    redis.hgetall("student", std::inserter(m, m.begin()));
    for (auto& i : m) {
        std::cout << i.first << ":" << i.second << std::endl;
    }

    // Get value only.
    // NOTE: since field might NOT exist, so we need to parse it to
    // OptionalString.
    std::vector<OptionalString> vals;
    redis.hmget("student", {"name", "invalid-field"}, std::back_inserter(vals));
    for (auto& i : vals) {
        if (i) {  // OptionalString
            // Dereference to get the returned value of std::string type.
            std::cout << *i << std::endl;
        }
    }
}

void type_set() {
    auto redis = Redis("tcp://127.0.0.1:6379");
    redis.sadd("skills", "c");

    // std::unordered_set<std::string> ==>> Redis SET.
    std::unordered_set<std::string> set = {"c++", "rust"};
    redis.sadd("skills", set.begin(), set.end());

    // std::initializer_list ==>> Redis SET.
    redis.sadd("skills", {"java", "python"});

    // Redis SET ==>> std::unordered_set<std::string>.
    set.clear();
    redis.smembers("skills", std::inserter(set, set.begin()));

    for (auto& i : set) {
        std::cout << i << std::endl;
    }

    if (redis.sismember("skills", "cpp")) {
        std::cout << "cpp exists" << std::endl;
    }  // else NOT exist.
}

void type_sorted_set() {
    auto redis = Redis("tcp://127.0.0.1:6379");
    redis.zadd("sorted_set", "m1", 1.3);

    // std::unordered_map<std::string, double> ==>> Redis SORTED SET.
    std::unordered_map<std::string, double> scores = {{"m2", 2.3}, {"m3", 4.5}};
    redis.zadd("sorted_set", scores.begin(), scores.end());

    // Redis SORTED SET ==>> std::vector<std::pair<std::string, double>>.
    // NOTE: The return results of zrangebyscore are ordered, if you save
    // the results in to `std::unordered_map<std::string, double>`, you'll
    // lose the order.
    std::vector<std::pair<std::string, double>> zset_result;
    redis.zrangebyscore("sorted_set",
                        UnboundedInterval<double>{},  // (-inf, +inf)
                        std::back_inserter(zset_result));
    for (auto& i : zset_result) {
        std::cout << i.first << ":" << i.second << " ";
    }
    std::cout << std::endl;

    // Only get member names:
    // pass an inserter of std::vector<std::string> type as output
    // parameter.
    std::vector<std::string> without_score;
    redis.zrangebyscore(
        "sorted_set",
        BoundedInterval<double>(1.5, 3.4, BoundType::CLOSED),  // [1.5, 3.4]
        std::back_inserter(without_score));
    for (auto& i : without_score) {
        std::cout << i << " ";
    }
    std::cout << std::endl;

    // Get both member names and scores:
    // pass an back_inserter of std::vector<std::pair<std::string, double>>
    // as output parameter.
    std::vector<std::pair<std::string, double>> with_score;
    redis.zrangebyscore(
        "sorted_set",
        BoundedInterval<double>(1.5, 3.4, BoundType::LEFT_OPEN),  // (1.5, 3.4]
        std::back_inserter(with_score));
    for (auto& i : with_score) {
        std::cout << i.first << ":" << i.second << " ";
    }
    std::cout << std::endl;
}

void publish() {
    ConnectionOptions opts;
    opts.host = "127.0.0.1";
    opts.port = 6379;
    opts.socket_timeout = std::chrono::milliseconds(100);

    auto redis = Redis(opts);
    redis.publish("test", "This is a test message!");
}

void subscribe() {
    ConnectionOptions opts;
    opts.host = "127.0.0.1";
    opts.port = 6379;
    opts.socket_timeout = std::chrono::seconds(1);

    auto redis = Redis(opts);

    // Create a Subscriber.
    auto sub = redis.subscriber();

    // Set callback functions.
    sub.on_message([](std::string channel, std::string msg) {
        // Process message of MESSAGE type.
        std::cout << "channel:" << channel << std::endl;
        std::cout << "msg:" << msg << std::endl;
    });

    sub.on_pmessage(
        [](std::string pattern, std::string channel, std::string msg) {
            // Process message of PMESSAGE type.
        });

    sub.on_meta(
        [](Subscriber::MsgType type, OptionalString channel, long long num) {
            // Process message of META type.
        });

    // Subscribe to channels and patterns.
    sub.subscribe("test");
    sub.subscribe({"channel2", "channel3"});

    sub.psubscribe("pattern1*");

    // Consume messages in a loop.
    while (true) {
        try {
            sub.consume();
        } catch (const Error& err) {
            // Handle exceptions.
        }
    }
}

void scripts() {
    auto redis = Redis("tcp://127.0.0.1:6379");
    // Script returns a single element.
    auto num = redis.eval<long long>("return 1", {}, {});

    // Script returns an array of elements.
    std::vector<std::string> nums;
    redis.eval("return {ARGV[1], ARGV[2]}", {}, {"1", "2"},
               std::back_inserter(nums));

    // mset with TTL
    auto mset_with_ttl_script = R"(
        local len = #KEYS
        if (len == 0 or len + 1 ~= #ARGV) then return 0 end
        local ttl = tonumber(ARGV[len + 1])
        if (not ttl or ttl <= 0) then return 0 end
        for i = 1, len do redis.call("SET", KEYS[i], ARGV[i], "EX", ttl) end
        return 1
    )";

    // Set multiple key-value pairs with TTL of 60 seconds.
    auto keys = {"key1", "key2", "key3"};
    std::vector<std::string> args = {"val1", "val2", "val3", "60"};
    redis.eval<long long>(mset_with_ttl_script, keys.begin(), keys.end(),
                          args.begin(), args.end());
}

void pipeline() {
    auto redis = Redis("tcp://127.0.0.1:6379");
    // Create a pipeline.
    auto pipe = redis.pipeline();

    // Send mulitple commands and get all replies.
    auto pipe_replies = pipe.set("key", "value")
                            .get("key")
                            .rename("key", "new-key")
                            .rpush("list", {"a", "b", "c"})
                            .lrange("list", 0, -1)
                            .exec();

    // Parse reply with reply type and index.
    auto set_cmd_result = pipe_replies.get<bool>(0);

    auto get_cmd_result = pipe_replies.get<OptionalString>(1);

    // rename command result
    pipe_replies.get<void>(2);

    auto rpush_cmd_result = pipe_replies.get<long long>(3);

    std::vector<std::string> lrange_cmd_result;
    pipe_replies.get(4, back_inserter(lrange_cmd_result));
}

void transaction() {
    auto redis = Redis("tcp://127.0.0.1:6379");
    // Create a transaction.
    auto tx = redis.transaction();

    // Run multiple commands in a transaction, and get all replies.
    auto tx_replies =
        tx.incr("num0").incr("num1").mget({"num0", "num1"}).exec();

    // Parse reply with reply type and index.
    auto incr_result0 = tx_replies.get<long long>(0);

    auto incr_result1 = tx_replies.get<long long>(1);

    std::vector<OptionalString> mget_cmd_result;
    tx_replies.get(2, back_inserter(mget_cmd_result));
}

void genetic_command_interface() {
    auto redis = Redis("tcp://127.0.0.1:6379");
    // There's no *Redis::client_getname* interface.
    // But you can use *Redis::command* to get the client name.
    auto val = redis.command<OptionalString>("client", "getname");
    if (val) {
        std::cout << *val << std::endl;
    }

    // Same as above.
    auto getname_cmd_str = {"client", "getname"};
    val = redis.command<OptionalString>(getname_cmd_str.begin(),
                                        getname_cmd_str.end());

    // There's no *Redis::sort* interface.
    // But you can use *Redis::command* to send sort the list.
    std::vector<std::string> sorted_list;
    redis.command("sort", "list", "ALPHA", std::back_inserter(sorted_list));

    // Another *Redis::command* to do the same work.
    auto sort_cmd_str = {"sort", "list", "ALPHA"};
    redis.command(sort_cmd_str.begin(), sort_cmd_str.end(),
                  std::back_inserter(sorted_list));
}

void cluster() {
    // Create a RedisCluster object, which is movable but NOT copyable.
    auto redis_cluster = RedisCluster("tcp://127.0.0.1:7000");

    // RedisCluster has similar interfaces as Redis.
    redis_cluster.set("key", "value");
    auto val = redis_cluster.get("key");
    if (val) {
        std::cout << *val << std::endl;
    }  // else key doesn't exist.

    // Keys with hash-tag.
    redis_cluster.set("key{tag}1", "val1");
    redis_cluster.set("key{tag}2", "val2");
    redis_cluster.set("key{tag}3", "val3");

    std::vector<OptionalString> hash_tag_res;
    redis_cluster.mget({"key{tag}1", "key{tag}2", "key{tag}3"},
                       std::back_inserter(hash_tag_res));
}

int main() {
    // 五大基本数据类型
    // std::cout << "===========string类型=============" << std::endl;
    // type_string();

    // std::cout << "===========list类型=============" << std::endl;
    // type_list();

    // std::cout << "===========hash类型=============" << std::endl;
    // type_hash();

    // std::cout << "===========set类型=============" << std::endl;
    // type_set();

    // std::cout << "===========zset类型=============" << std::endl;
    // type_sorted_set();

    // 消息队列
    std::thread t(subscribe);
    t.detach();
    publish();
    getchar();

    // 其它功能
    std::cout << "===========脚本=============" << std::endl;
    scripts();
    std::cout << "===========pipeline=============" << std::endl;
    pipeline();
    std::cout << "===========事务=============" << std::endl;
    transaction();
    std::cout << "===========通用命令接口=============" << std::endl;
    genetic_command_interface();
    // cluster();
}