/**
 * a simple hash table for 1brc challenge
 */

 #include <functional>
 #include <string>
 #include <vector>
 #include <cstring>

namespace hash1 {
    using hashfunc = std::function<size_t(const char*, size_t)>;
    /* APHash */
    size_t APHash(const char* str, size_t len) {
        size_t hash = 0;
        for(size_t i = 0; i < len; i++) {
            hash ^= ((i & 1) == 0) ? (hash << 7) ^ str[i] ^ (hash >> 3) :
                (~hash << 11) ^ str[i] ^ (hash >> 5);
        }
        return hash & 0x7FFFFFFF;
    }

    /* DJB2 */
    size_t DJB2(const char* str, size_t len) {
        size_t hash = 5381;
        for(size_t i = 0; i < len; i++) {
            hash = ((hash << 5) + hash) + str[i];
        }
        return hash & 0x7FFFFFFF;
    }


    /* hashtable using open addressing */
    template<class T>
    struct HashTable {
        size_t size;
        hashfunc hash;
        size_t unique_cnt;
        std::vector<std::pair<const char*, T>> data;
        std::vector<std::pair<const char*, size_t> > keys; // store the keys
        std::vector<size_t> key_lens;  // Store key lengths for quick comparison

        HashTable(size_t size, hashfunc hash = APHash) : size(size), hash(hash), unique_cnt(0) {
            data.resize(size, {nullptr, T{}});
            key_lens.resize(size, 0);
            keys.resize(size, {nullptr, 0});
        }

        ~HashTable() {
            // Clean up allocated keys
            for (auto& pair : data) {
                if (pair.first != nullptr) {
                    delete[] pair.first;
                }
            }
        }

        // Find existing key, returns pointer to value or nullptr if not found
        T* find(const char* key, size_t len) {
            size_t idx = hash(key, len) % size;
            size_t start_idx = idx;

            while (data[idx].first != nullptr) {
                if (key_lens[idx] == len &&
                    std::memcmp(data[idx].first, key, len) == 0) {
                    return &data[idx].second;
                    }
                idx = (idx + 1) % size;
                if (idx == start_idx) {
                    // Table is full and key not found
                    return nullptr;
                }
            }
            return nullptr;
        }

        // Try to emplace: if key exists, return its address; if not, create with default value
        T* try_emplace(const char* key, size_t len) {
            size_t idx = hash(key, len) % size;
            size_t start_idx = idx;

            while (data[idx].first != nullptr) {
                if (key_lens[idx] == len &&
                    std::memcmp(data[idx].first, key, len) == 0) {
                    // Key exists, return address
                    return &data[idx].second;
                    }
                idx = (idx + 1) % size;
                if (idx == start_idx) {
                    // Table is full, cannot insert
                    return nullptr;
                }
            }

            // Key doesn't exist, create it with default value
            char* new_key = new char[len];
            std::memcpy(new_key, key, len);
            data[idx].first = new_key;
            data[idx].second = T{};  // Default construct the value
            key_lens[idx] = len;
            keys[unique_cnt++] = {new_key, len};
            return &data[idx].second;
        }
    };
}