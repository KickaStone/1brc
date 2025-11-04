/**
 * a simple hash table for 1brc challenge
 */

#include <functional>
#include <vector>
#include <cstring>
#include <cstdio>
#include <iostream>


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


class StringPool {
public:
    static constexpr size_t STR_SIZE = MAXLENTH;

    struct String {
        char str[STR_SIZE];
    };

    StringPool() : next_(0), size_(0) {
        pool_ = nullptr;
    }

    explicit StringPool(size_t size) : next_(0), size_(size) {
        pool_ = new String[size];
    }

    ~StringPool() {
        delete[] pool_;
    }

    // allocate a string space from the pool, return pointer
    char* alloc() {
        if (next_ >= size_) return nullptr; // pool is full
        return pool_[next_++].str;
    }

    void reset() { next_ = 0; }

private:
    String *pool_{};
    size_t next_;
    size_t size_;
};


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
    StringPool *string_pool = nullptr;


    HashTable(size_t size, hashfunc hash = APHash) : size(size), hash(hash), unique_cnt(0)
    {
        data.resize(size, {nullptr, T{}});
        key_lens.resize(size, 0);
        keys.resize(size, {nullptr, 0});
        string_pool = new StringPool(size);
    }

    // 禁止拷贝，防止多个 HashTable 共享同一个 StringPool
    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;

    // 允许移动
    HashTable(HashTable&& other) noexcept
        : size(other.size), hash(other.hash), unique_cnt(other.unique_cnt),
          data(std::move(other.data)), keys(std::move(other.keys)), 
          key_lens(std::move(other.key_lens)), string_pool(other.string_pool)
    {
        other.string_pool = nullptr;
        other.unique_cnt = 0;
    }

    HashTable& operator=(HashTable&& other) noexcept
    {
        if (this != &other) {
            delete string_pool;
            size = other.size;
            hash = other.hash;
            unique_cnt = other.unique_cnt;
            data = std::move(other.data);
            keys = std::move(other.keys);
            key_lens = std::move(other.key_lens);
            string_pool = other.string_pool;
            other.string_pool = nullptr;
            other.unique_cnt = 0;
        }
        return *this;
    }

    ~HashTable()
    {
        delete string_pool;
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

        char *new_key = string_pool->alloc();
        std::strncpy(new_key, key, len);
        new_key[len] = '\0';
        data[idx].first = new_key;
        data[idx].second = T{}; // Default construct the value
        key_lens[idx] = len;
        keys[unique_cnt++] = {new_key, len};
        return &data[idx].second;
    }
};