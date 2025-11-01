#include <map>
#include <cstring>
#include <iostream>

using std::map;

template<class T>
struct TrieNode{
    bool isEnd = false;
    T *data = nullptr;
    map<char, TrieNode*> mp;
};

TrieNode<int> root;

template <class T>
using cb = void(T *t);

/**
 * insert the data with path. If path not exists create it.
 */
template<class T>
inline void insert(TrieNode<T>* root, const char* path, T *data, int l, size_t size){
    if(root == nullptr || path == nullptr) return;
    
    // If we've reached the end of the path
    if(l >= size || path[l] == '\0'){
        root->isEnd = true;
        root->data = data;
        return;
    }
    
    // Find or create the child node for current character
    auto it = root->mp.find(path[l]);
    if(it == root->mp.end()){
        // Create new node if it doesn't exist
        TrieNode<T>* newNode = new TrieNode<T>();
        root->mp[path[l]] = newNode;
        insert(newNode, path, data, l + 1, size);
    } else {
        // Continue traversing if node exists
        insert(it->second, path, data, l + 1, size);
    }
}

/**
 * Find a node by path. Returns the node if found, nullptr otherwise.
 */
template<class T>
TrieNode<T>* find(TrieNode<T>* root, const char* str, int i){
    if(root == nullptr || str == nullptr) return nullptr;
    
    // If we've reached the end of the string, return current node if it's an end node
    if(str[i] == '\0'){
        return root->isEnd ? root : nullptr;
    }
    
    // Find the child node for current character
    auto it = root->mp.find(str[i]);
    if(it == root->mp.end()){
        return nullptr;  // Character not found in trie
    }
    
    // Continue searching in the child node
    return find(it->second, str, i + 1);
};


/**
 * Find a node by path. If path doesn't exist, create it with default constructed value.
 * This is like a "get or create" operation using T's default constructor.
 */
template<class T>
TrieNode<T>* findOrCreate(TrieNode<T>* root, const char* path, int l, size_t size){
    if(root == nullptr || path == nullptr) return nullptr;
    
    // If we've reached the end of the path
    if(l >= size || path[l] == '\0'){
        // If not marked as end, initialize it with default constructed value
        if(!root->isEnd){
            root->isEnd = true;
            root->data = new T();  // Use default constructor
        }
        return root;
    }
    
    // Find or create the child node for current character
    auto it = root->mp.find(path[l]);
    if(it == root->mp.end()){
        // Create new node if it doesn't exist (like insert)
        TrieNode<T>* newNode = new TrieNode<T>();
        root->mp[path[l]] = newNode;
        return findOrCreate(newNode, path, l + 1, size);
    } else {
        // Continue traversing
        return findOrCreate(it->second, path, l + 1, size);
    }
}

/**
 * Print all key in trie tree with lexicographical order
 */
template<class T, class Func>
void printAll(TrieNode<T>* root, Func f, std::string currentPath = ""){
    if(root == nullptr) return;
    
    // If this is an end node, print the path and its value
    if(root->isEnd){
        // std::cout << "Key: \"" << currentPath << "\" -> Value: ";
        if(root->data != nullptr){
            // print value using callback
            f(root->data);
        }
        std::cout << std::endl;
    }
    
    // Traverse children in lexicographical order (map maintains sorted order)
    for(auto& pair : root->mp){
        char ch = pair.first;
        TrieNode<T>* child = pair.second;
        printAll(child, f, currentPath + ch);
    }
}