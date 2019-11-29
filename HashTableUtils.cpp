#include "HashTable.hpp"

artemreyt::ProbPolicy::ProbPolicy(size_t hash, size_t m_): 
    next_proba(hash), current_proba(hash), m(m_), i(0) {}

size_t  artemreyt::ProbPolicy::operator()() {
    current_proba = next_proba;
    next_proba = (current_proba + i++ + 1) % m;
    return current_proba;
}

int    artemreyt::DefaultHash<int>::find_pow(size_t m) {
    size_t i = 0;
    size_t pow_of_2 = 1;

    while (pow_of_2 < m) {
        i++;
        pow_of_2 <<= 1;
    }
    return i;
}

size_t  artemreyt::DefaultHash<int>::operator()(const int &key, size_t m) {
    return (((key * s) % power2_32) >> (32 - find_pow(m))) % m;
}

size_t  artemreyt::DefaultHash<std::string>::operator()(const std::string &str, size_t m) {
    size_t hash = 0;

    for (const auto &c: str) {
        hash = (hash * a + c) % m;
    }
    return hash;
}
