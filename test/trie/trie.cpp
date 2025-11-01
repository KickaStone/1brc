#include "trie.h"

int main(int argc, char const *argv[])
{
    int a = 100;
    int b = 200;
    const char* path1 = "hello world!";
    const char* path2 = "hello";
    
    // Insert data
    insert(&root, path1, &a, 0, strlen(path1));
    insert(&root, path2, &b, 0, strlen(path2));
    
    // Find and test
    TrieNode<int>* found1 = find(&root, path1, 0);
    if(found1 && found1->data){
        std::cout << "Found '" << path1 << "' with value: " << *(found1->data) << std::endl;
    }
    
    TrieNode<int>* found2 = find(&root, path2, 0);
    if(found2 && found2->data){
        std::cout << "Found '" << path2 << "' with value: " << *(found2->data) << std::endl;
    }
    
    TrieNode<int>* notFound = find(&root, "hi", 0);
    if(!notFound){
        std::cout << "String 'hi' not found (correct!)" << std::endl;
    }
    
    std::cout << "\n--- Testing findOrCreate ---" << std::endl;
    
    // Test 1: Find existing path - should return existing value
    TrieNode<int>* existing = findOrCreate(&root, path1, 0, strlen(path1));
    if(existing && existing->data){
        std::cout << "findOrCreate('" << path1 << "') = " << *(existing->data) 
                  << " (existing)" << std::endl;
    }
    
    // Test 2: Create new path with default constructed value (int() = 0)
    const char* newPath = "goodbye";
    TrieNode<int>* created = findOrCreate(&root, newPath, 0, strlen(newPath));
    if(created && created->data){
        std::cout << "findOrCreate('" << newPath << "') = " << *(created->data) 
                  << " (created with default constructor, int() = 0)" << std::endl;
    }
    
    // Test 3: Verify the created path can be found normally
    TrieNode<int>* verify = find(&root, newPath, 0);
    if(verify && verify->data){
        std::cout << "Verification: find('" << newPath << "') = " << *(verify->data) 
                  << " (found!)" << std::endl;
    }
    
    // Test 4: Modify the default value and create another path
    const char* anotherPath = "test";
    TrieNode<int>* another = findOrCreate(&root, anotherPath, 0, strlen(anotherPath));
    if(another && another->data){
        *(another->data) = 42;  // Modify the value
        std::cout << "Created '" << anotherPath << "' and set to: " << *(another->data) << std::endl;
    }
    
    // Test 5: Print all keys in lexicographical order
    std::cout << "\n--- Print All Keys (Lexicographical Order) ---" << std::endl;
    auto print = [](int *a){ std::cout << *a; };
    printAll(&root, print);
    
    return 0;
}
