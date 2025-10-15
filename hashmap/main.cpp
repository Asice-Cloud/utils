#include <iostream>
#include <string>
#include "HashMap.h"
#include "HashMap.h"
#include <thread>
#include <vector>

namespace normal
{
    void normal_testHashMap() {
        HashMap<std::string, int> map;

        // 测试插入
        map.put("asice", 17);
        map.put("winter", 19);
        map.put("charlotte", 19);
        map.put("dream", 19);

        // 测试获取
        int value;
        if (map.get("winter", value)) {
            std::cout << "Value for 'winter': " << value << std::endl;
        } else {
            std::cout << "Key 'winter' not found" << std::endl;
        }

        std::cout << "Size: " << map.getSize() << std::endl;

        // 测试删除
        if (map.remove("dream")) {
            std::cout << "Key 'dream' removed successfully" << std::endl;
        } else {
            std::cout << "Failed to remove key 'odream'" << std::endl;
        }

        std::cout << "Size after removal: " << map.getSize() << std::endl;

        // 测试更新
        map.put("winter", 22);
        if (map.get("winter", value)) {
            std::cout << "Updated value for 'winter': " << value << std::endl;
        }

        // 测试大量插入以触发树化和扩容
        for (int i = 0; i < 100; i++) {
            map.put("key" + std::to_string(i), i);
        }

        std::cout << "Size after adding 100 elements: " << map.getSize() << std::endl;

        // 测试获取不存在的键
        if (!map.get("nonexistent", value)) {
            std::cout << "Key 'nonexistent' not found, as expected" << std::endl;
        }
    }
}


namespace safe
{
    void testThreadSafety() {
        HashMap<std::string, int> map;
        const int numThreads = 10;
        const int numOperations = 100;

        auto putTask = [&map](int threadId) {
            for (int i = 0; i < numOperations; ++i) {
                map.put("key" + std::to_string(threadId) + "_" + std::to_string(i), i);
            }
        };

        auto getTask = [&map](int threadId) {
            int value;
            for (int i = 0; i < numOperations; ++i) {
                map.get("key" + std::to_string(threadId) + "_" + std::to_string(i), value);
            }
        };

        auto removeTask = [&map](int threadId) {
            for (int i = 0; i < numOperations; ++i) {
                map.remove("key" + std::to_string(threadId) + "_" + std::to_string(i));
            }
        };

        std::vector<std::thread> threads;
        for (int i = 0; i < numThreads; ++i) {
            threads.emplace_back(putTask, i);
        }
        for (auto& thread : threads) {
            thread.join();
        }

        threads.clear();
        for (int i = 0; i < numThreads; ++i) {
            threads.emplace_back(getTask, i);
        }
        for (auto& thread : threads) {
            thread.join();
        }

        threads.clear();
        for (int i = 0; i < numThreads; ++i) {
            threads.emplace_back(removeTask, i);
        }
        for (auto& thread : threads) {
            thread.join();
        }

        std::cout << "Size after concurrent insert, get, and remove: " << map.getSize() << std::endl;
    }
}



int main() {
    normal::normal_testHashMap();
    std::cout<<'\n';
    safe::testThreadSafety();
    std::cout<<'\n';


    return 0;
}