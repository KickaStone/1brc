#include <iostream>

using namespace std;

void parse(char*& data, char*& name, int& len, int& v){  // data也改为引用
    int neg=1;
    name = data;
    len = 0;
    v = 0;
    
    // 解析名称
    while(*data != ';'){ len++; data++; }
    data++; // 跳过 ';'
    
    // 解析温度值
    while(*data != '\n' && *data != '\0'){
        if(*data == '-'){ neg = -1; data++; continue; }
        if(*data == '.'){ data++; continue; }
        v = v * 10 + (*data - '0');
        data++;
    }
    v *= neg;
    
    // 跳过当前行的 '\n'，移动到下一行
    if(*data == '\n') data++;
}
int main(int argc, char *argv[]){
    char* name = nullptr;
    int len = 0;
    int v = 0;

    char *data = "New York;10.5\nLos Angeles;15.2\nChicago;12.3\n";
    char *ptr = data;
    
    // 循环解析多行，直到遇到字符串结尾
    while(*ptr != '\0'){
        parse(ptr, name, len, v);
        cout << string(name, len) << "=" << v << endl;
    }
    
    return 0;
}