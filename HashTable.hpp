#ifndef HASHTABLE_HPP
# define HASHTABLE_HPP

#include <iostream>
#include <string>
#include <exception>
#include <utility>

namespace artemreyt {
    const size_t start_size = 1024;

    /*
        ** HASH CLASSES
        ** in operator() m is size of array in HashTable
        ** and it's guaranteed is power of 2
    */
    template<typename T>
    struct DefaultHash {
        size_t operator()(const T &key, size_t m) {
            return static_cast<size_t>(key) % m;
        }
    };

    template<>
    struct DefaultHash<int> {
    private:
        const size_t s = 2654435769;
        const size_t power2_32 = 1 << 31;

    public:
        int     find_pow(size_t m);
        size_t  operator()(const int &key, size_t m);
    }; 

    template<>
    struct DefaultHash<std::string> {
    private:
        const size_t a = 53;

    public:
        size_t operator()(const std::string &str, size_t m);
    };

    /*
        ** DEFAULT COMPARATOR
    */
    template<typename T>
    struct DefaultComparator {
        bool operator()(const T &lhs, const T &rhs) {
            return lhs == rhs;
        }
    };

    /*
        ** Condition of node in table
    */
    enum class condition {EMPTY, DELETED, BUSY};
    
    /*
        ** node in table
    */
    template<typename Key, typename Value>
    struct node_t {
        condition               state = condition::EMPTY;
        std::pair<Key, Value>   pair;
    };

    /*
        ** Functor generating proba sequence
    */
    class ProbPolicy {
    private:
        size_t current_proba;
        size_t next_proba;
        size_t m;
        size_t i;

    public:
        ProbPolicy(size_t hash, size_t m_);
        size_t  operator()();
    };

    /*
        ** HASHTABLE CLASS
        ** Key - type of keys in table
        ** T - type of values in table
        ** Hash - class where operator() is hash function
        ** Comparator class where operator() is equal function
    */
    template <
        typename    Key,
        typename    T,
        class       Hash = DefaultHash<Key>,
        class       Comparator = DefaultComparator<Key>> 
    class HashTable {
    public:
        class iterator;
    private:
        node_t<Key, T>  *table;
        size_t          tableSize;
        size_t          elementsCount;
        double          maxLoadFactor;
        Hash            hasher;
        Comparator      cmp;

        bool        isFree(const node_t<Key, T> &node);
        iterator    insert_key(const Key &key);
        iterator    fillNode(size_t pos, const Key &key);

    public:
        /*
            ** Simple iterator implementation
        */
        class iterator {
        private:
            node_t<Key, T>   *ptr;
            node_t<Key, T>   *end;

        public:
            iterator(node_t<Key, T> *ptr_, node_t<Key, T> *end_): ptr(ptr_), end(end_) {}
            iterator            operator++() {
                iterator tmp = *this;
                ptr++;
                while (ptr < end && ptr->state != condition::BUSY) ptr++;
                if (ptr > end) ptr = end;
                return tmp;
            }
            iterator&           operator++(int) {
                ptr++;
                while ((size_t)ptr < (size_t)end && ptr->state != condition::BUSY) ptr++;
                if (ptr > end) ptr = end; 
                return *this;
            }
            std::pair<Key, T>&  operator*() { return {ptr->pair}; }
            bool                operator!=(const iterator& rhs) { return (this->ptr != rhs.ptr); }

            friend class HashTable;
        };

        explicit    HashTable(double maxLoadFactor=0.75);
                    ~HashTable();
        iterator    insert(const std::pair<Key, T> &insert_pair);
        iterator    find(const Key &key);
        bool        erase(const Key &key);
        T&          operator[](const Key &key);
        void        rehash();
        iterator    begin();
        iterator    end();
    };

    /*
        ** HashTable methods implementations
    */
    template <typename Key, typename T, class Hash, class Comparator>
    HashTable<Key, T, Hash, Comparator>::HashTable(double maxLoadFactor_):
            maxLoadFactor(maxLoadFactor_), tableSize(start_size), elementsCount(0),
            hasher(Hash()) {
        table = new node_t<Key, T> [start_size];
    }

    template <typename Key, typename T, class Hash, class Comparator>
    HashTable<Key, T, Hash, Comparator>::~HashTable() {
        delete [] table;
    }

    template <typename Key, typename T, class Hash, class Comparator>
    typename HashTable<Key, T, Hash, Comparator>::iterator
    HashTable<Key, T, Hash, Comparator>::fillNode(size_t pos, const Key &key) {
        table[pos].pair.first = key;
        table[pos].state = condition::BUSY;
        return iterator(this->table + pos, this->table + tableSize);
    }

    template <typename Key, typename T, class Hash, class Comparator>
    typename HashTable<Key, T, Hash, Comparator>::iterator
    HashTable<Key, T, Hash, Comparator>::insert_key(const Key &key) {
        elementsCount++;
        if ((double)elementsCount / tableSize >= maxLoadFactor) {
            rehash();
        }

        size_t hash = hasher(key, this->tableSize);
        size_t i = 0;
        size_t pos;
        std::pair<bool, size_t> first_deleted = {false, 0};
        ProbPolicy proba(hash, tableSize);
        while (i < tableSize) {
            pos = proba();
            if (this->table[pos].state == condition::DELETED) {
                if (!first_deleted.first) {
                    first_deleted.first = true;
                    first_deleted.second = pos;
                }
            } 
            else if (this->table[pos].state == condition::EMPTY) {
                if (first_deleted.first)
                    pos = first_deleted.second;
                return fillNode(pos, key);
            }
            else if (cmp(table[pos].pair.first, key))
                return this->end();
            i++;
        }
        if (!first_deleted.first)
            throw std::logic_error("No free space in table!");
        return fillNode(first_deleted.second, key);
    }

    template <typename Key, typename T, class Hash, class Comparator>
    typename HashTable<Key, T, Hash, Comparator>::iterator
    HashTable<Key, T, Hash, Comparator>::insert(const std::pair<Key, T> &insert_pair) {
        auto iter = this->insert_key(insert_pair.first);
        if (iter != this->end()) {
            *iter = insert_pair;
            return iter;
        }
        return this->end();
    }

    template <typename Key, typename T, class Hash, class Comparator>
    bool    HashTable<Key, T, Hash, Comparator>::isFree(const node_t<Key, T> &node) {
        return (node.state != condition::BUSY);
    }

    template <typename Key, typename T, class Hash, class Comparator>
    void    HashTable<Key, T, Hash, Comparator>::rehash() {
        node_t<Key, T>  *old_table = this->table;
        size_t          old_size = this->tableSize;

        this->tableSize *= 2;
        this->elementsCount = 0;
        this->table = new node_t<Key, T> [tableSize];

        for (size_t i = 0; i < old_size; i++) {
            if (!isFree(old_table[i]))
                this->insert(old_table[i].pair);
        }

        delete [] old_table;
    }

    template <typename Key, typename T, class Hash, class Comparator>
    typename HashTable<Key, T, Hash, Comparator>::iterator
    HashTable<Key, T, Hash, Comparator>::find(const Key &key) {
        size_t  hash = hasher(key, this->tableSize);
        size_t  pos;
        ProbPolicy proba(hash, tableSize);

        for (size_t i = 0; i < this->tableSize; i++) {
            pos = proba();
            if (this->table[pos].state == condition::EMPTY) {
                return this->end();
            } 
            else if (this->table[pos].state == condition::BUSY
                     && cmp(table[pos].pair.first, key)) {
                return iterator(this->table + pos, this->table + tableSize);
            }
        }
        return this->end();
    }

    template <typename Key, typename T, class Hash, class Comparator>
    T&          HashTable<Key, T, Hash, Comparator>::operator[](const Key &key) {
        iterator key_iter = this->find(key);

        if (key_iter != this->end()) {
            return (*key_iter).second;
        }
        return (*this->insert_key(key)).second;
    }

    template <typename Key, typename T, class Hash, class Comparator>
    typename HashTable<Key, T, Hash, Comparator>::iterator
    HashTable<Key, T, Hash, Comparator>::begin() {
        return iterator(this->table, this->table + this->tableSize);
    }

    template <typename Key, typename T, class Hash, class Comparator>
    typename HashTable<Key, T, Hash, Comparator>::iterator
    HashTable<Key, T, Hash, Comparator>::end() {
        return iterator(this->table + this->tableSize, this->table + this->tableSize);
    }

    template <typename Key, typename T, class Hash, class Comparator>
    bool     HashTable<Key, T, Hash, Comparator>::erase(const Key &key) {
        iterator key_iter = this->find(key);
        if (key_iter != this->end()) {
            key_iter.ptr->state = condition::DELETED;
            return true;
        }
        return false;
    }
};

#endif
