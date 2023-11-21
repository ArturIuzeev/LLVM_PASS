#include <string>
#include <unordered_map>
#include <iostream>

using Key = std::pair<std::string , std::string>;

struct KeyHash {
    std::size_t operator()(const Key &k) const {
        return std::hash<std::string>()(k.first) + std::hash<std::string>()(k.second);
    }
};

static std::unordered_map<Key, size_t, KeyHash> statistic_map;

extern "C" void logging(char *first,char *second) {
    ++statistic_map[std::pair{std::string{first}, std::string{second}}];
}

extern "C" void func_start_logger(char *name) {
    std::cout << "Name function: " << name << std::endl;
}

extern "C" void get_logs() {
    for (const auto &item : statistic_map) {
        std::cout << item.first.first << "->" << item.first.second << " " << item.second << std::endl;
    }
}
