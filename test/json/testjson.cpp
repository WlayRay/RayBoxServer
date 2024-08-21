#include "json.hpp"
using json = nlohmann::json;
using namespace nlohmann::literals;
#include <iostream>
#include <map>
#include <string>
#include <vector>
using namespace std;

int main() {
    json js;
    js["int"] = 42;
    js["double"] = 3.14;
    js["string"] = "hello, world";
    js["array"] = {1, 2, 3, 4, 5};
    js["object"]["name"] = "eric";
    js["object"]["pwd"] = "password";
    js["object2"] = {{"name", "eva"}, {"pwd", "another password"}};

    vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    js["vector"] = vec;

    map<int, string> m;
    m[1] = "cpp";
    m[2] = "rust";

    js["map"] = m;

    // 序列化
    string jsStr = js.dump();
    std::cout << jsStr << std::endl;

    // 反序列化
    json js2 = json::parse(jsStr);
    std::cout << js2["int"] << std::endl;
    vector<int> vec2 = js2["vector"];
    std::cout << vec2[0] << std::endl;

    std::map<int, string> map2 = js2["map"];
    std::cout << map2[1] << std::endl;

    // 以下代码抛出异常
    // int num = js["invalid-key"];
    // std::cout << num << std::endl;

    js["null"] = "null";
    std::cout << js["null"] << std::endl;
    std::cout << js.contains("null") << std::endl;
    std::cout << js["invalid"];
}