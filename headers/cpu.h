#ifndef CPU_H
#define CPU_H
#include<string>
#include<queue>
#include<utility>
using namespace std; 

class cpu
{
private:

    queue<pair<int, string>> addressList;
    int addressLength;

public:
    cpu(string file);
    pair<int, string> getAddress();
    int getAddressCount() { return addressLength; };
};
#endif
