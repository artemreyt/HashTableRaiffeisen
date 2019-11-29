#include <iostream>
#include <string>
#include <cstdlib>
#include <cctype>
#include <ctime>
#include <unordered_map>
#include <unordered_set>
#include "HashTable.hpp"

std::string randomWordGenerator(size_t max_len) {
    size_t len = std::rand() % max_len + 1;
    std::string str = "";
    for (size_t i = 0; i < len; i++) {
        int c = std::rand() % ('z' - 'a' - 1) + 'a';
        if (std::rand() % 2) {
            c = toupper(c);
        }
        str.push_back(c);
    }
    return str;
}

int main() {
    srand(time(NULL));
    size_t elementsCount = 200000;

    std::unordered_map<std::string, int> std_table;
    artemreyt::HashTable<std::string, int> my_table(0.75);
    std::unordered_set<std::string> key_set;

    std::clock_t std_time_insert = 0;
    std::clock_t my_time_insert = 0;
    std::clock_t start, end;
    for (size_t i = 0; i < elementsCount; i++) {
        auto key = randomWordGenerator(20);
        key_set.insert(key);
        int value = std::rand() % 100;

        start = clock();
        std_table.insert({key, value});
        end = clock();
        std_time_insert += end - start;


        start = clock();
        my_table.insert({key, value});
        end = clock();
        my_time_insert += end - start;
    }

    std::clock_t std_time_delete = 0;
    std::clock_t my_time_delete = 0;
    size_t i = 0;
    for (const auto &key: key_set) {
        if (i == static_cast<size_t>(elementsCount * 0.7))
            break;
        
        start = clock();
        std_table.erase(key);
        end = clock();
        std_time_delete += end - start;

        start = clock();
        my_table.erase(key);
        end = clock();
        my_time_delete += end - start;

        i++;
    }

    for (const auto &key: key_set) {
        if (i == 0)
            break;
        auto new_value = std::rand() % 100;

        std_table[key] = new_value;
        my_table[key] = new_value;
        i++;
    }

    std::clock_t std_time_find= 0;
    std::clock_t my_time_find = 0;
    size_t total_err = 0;
    for (const auto &str : key_set) {
        start = clock();
        int std_val = std_table[str];
        end = clock();
        std_time_find += end - start;

        start = clock();
        int my_val = my_table[str];
        end = clock();
        my_time_find += end - start;

        std::cout << "\"" << str << "\": " << "std: " << std_val << " | mine: " << my_val << " ";
        if (std_val != my_val) {
            std::cout << "OH NO!" << std::endl;
            total_err++;
        } else {
            std::cout << "YES!" << std::endl;
        }
    }

    if (!total_err) {
        std::cout << "!----SUCCESS----!" << std::endl;
        std::cout << "INSERT std runtime: " << std::fixed << (double)std_time_insert / CLOCKS_PER_SEC << 's' << std::endl;
        std::cout << "INSERT my runtime: " << std::fixed << (double)my_time_insert / CLOCKS_PER_SEC << 's' << std::endl;
        std::cout << "FIND std runtime: " << std::fixed << (double)std_time_find / CLOCKS_PER_SEC << 's' << std::endl;
        std::cout << "FIND my runtime: " << std::fixed << (double)my_time_find / CLOCKS_PER_SEC << 's' << std::endl;
        std::cout << "DELETE std runtime: " << std::fixed << (double)std_time_delete / CLOCKS_PER_SEC << 's' << std::endl;
        std::cout << "DELETE my runtime: " << std::fixed << (double)my_time_delete / CLOCKS_PER_SEC << 's' << std::endl;

    } else {
        std::cout << "TOTAL ERRORS: " << total_err << " FAIL:(" << std::endl;
        return 1;
    }
    return 0;
}
