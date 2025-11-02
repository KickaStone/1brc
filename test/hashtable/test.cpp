#include "myhashtable.h"
#include <iostream>

using namespace std;
int main(int argc, char const *argv[])
{
    HashTable<int> ht(10000);

    auto it = ht.try_emplace("hello", 5);
    if(it != nullptr){
        *it = 100;
    }

    it = ht.find("hello", 5);
    if(it != nullptr){
        cout << *it << endl;
    }

    it = ht.try_emplace("hello", 5);
    if(it != nullptr){
        *it = 200;
    }

    it = ht.find("hello", 5);
    if(it != nullptr){
        cout << *it << endl;
    }

    return 0;
}