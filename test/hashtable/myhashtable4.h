/**
 * a simple hash table for 1brc challenge
 */
#ifndef MAP3
#define MAP3

#include <functional>
#include <vector>
#include <cstring>
#include <cstdio>
#include <iostream>


namespace hash4 {
#define MAXLENTH 32 // name of city is less than 32 characters
    using hashfunc = std::function<size_t(const char *, size_t)>;
    /* APHash */
    size_t APHash(const char *str, size_t len)
    {
        size_t hash = 0;
        for (size_t i = 0; i < len; i++)
        {
            hash ^= ((i & 1) == 0) ? (hash << 7) ^ str[i] ^ (hash >> 3) : (~hash << 11) ^ str[i] ^ (hash >> 5);
        }
        return hash & 0x7FFFFFFF;
    }

    size_t APHash_unroll(const char *str, size_t len)
    {
        size_t hash = 0;
        size_t i = 0;

        for (; i + 3 < len; i += 4)
        {
            hash ^= (hash << 7) ^ str[i] ^ (hash >> 3);
            hash ^= (~hash << 11) ^ str[i + 1] ^ (hash >> 5);
            hash ^= (hash << 7) ^ str[i + 2] ^ (hash >> 3);
            hash ^= (~hash << 11) ^ str[i + 3] ^ (hash >> 5);
        }

        for (; i < len; i++)
            hash ^= ((i & 1) == 0)
                        ? (hash << 7) ^ str[i] ^ (hash >> 3)
                        : (~hash << 11) ^ str[i] ^ (hash >> 5);

        return hash & 0x7FFFFFFF;
    }

    /* DJB2 */
    size_t DJB2(const char *str, size_t len)
    {
        size_t hash = 5381;
        for (size_t i = 0; i < len; i++)
        {
            hash = ((hash << 5) + hash) + str[i];
        }
        return hash & 0x7FFFFFFF;
    }


    /* hashtable using open addressing */
    template <class T>
    struct HashTable
    {
        size_t size;
        hashfunc hash;
        size_t unique_cnt;
        std::vector<std::pair<const char *, T>> data;
        std::vector<std::pair<const char *, size_t>> keys; // store the keys and the length of the key
        std::vector<size_t> key_lens;                      // Store key lengths for quick comparison


        HashTable(size_t size, hashfunc hash = APHash_unroll) : size(size), hash(hash), unique_cnt(0)
        {
            data.resize(size, {nullptr, T{}});
            key_lens.resize(size, 0);
            keys.resize(size, {nullptr, 0});
        }

        // 禁止拷贝，防止多个 HashTable 共享同一个 StringPool
        HashTable(const HashTable&) = delete;
        HashTable& operator=(const HashTable&) = delete;

        // 允许移动
        HashTable(HashTable&& other) noexcept
            : size(other.size), hash(other.hash), unique_cnt(other.unique_cnt),
              data(std::move(other.data)), keys(std::move(other.keys)),
              key_lens(std::move(other.key_lens))
        {
            other.unique_cnt = 0;
        }

        HashTable& operator=(HashTable&& other) noexcept
        {
            if (this != &other) {
                size = other.size;
                hash = other.hash;
                unique_cnt = other.unique_cnt;
                data = std::move(other.data);
                keys = std::move(other.keys);
                key_lens = std::move(other.key_lens);
                other.unique_cnt = 0;
            }
            return *this;
        }

        // Find existing key, returns pointer to value or nullptr if not found
        T *find(const char *key, size_t len)
        {
            size_t idx = hash(key, len) % size;
            size_t start_idx = idx;

            while (data[idx].first != nullptr)
            {
                if (key_lens[idx] == len &&
                    std::memcmp(data[idx].first, key, len) == 0)
                {
                    return &data[idx].second;
                }
                idx = (idx + 1) % size;
                if (idx == start_idx)
                {
                    // Table is full and key not found
                    return nullptr;
                }
            }
            return nullptr;
        }

        // Try to emplace: if key exists, return its address; if not, create with default value
        T *try_emplace(const char *key, size_t len)
        {
            size_t idx = hash(key, len) % size;
            size_t start_idx = idx;

            while (data[idx].first != nullptr)
            {
                if (key_lens[idx] == len &&
                    std::memcmp(data[idx].first, key, len) == 0)
                {
                    return &data[idx].second;
                }
                idx = (idx + 1) % size;
                if (idx == start_idx)
                {
                    // Table is full, cannot insert
                    return nullptr;
                }
            }

            if(unique_cnt >= size){
                printf("HashTable is full\n");
                return nullptr;
            }

            data[idx].first = key;
            key_lens[idx] = len;
            keys[unique_cnt++] = {key, len};
            return &data[idx].second;
        }
    };
}
#endif
