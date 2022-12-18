#ifndef ram_H
#define ram_H
#include<string>
#include<map>
#include<vector>
using namespace std;

class ram
{
private:
    const int memSize = 1000000; //1mb
    vector<string> mem;
public:
    ram();
    string read(int addr);
    bool write(int addr, string data);
    vector<string> getMemory();
};


#endif