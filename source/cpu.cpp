#include "cpu.h"
#include<string>
#include <sstream>
#include <fstream>
#include<iostream>
using namespace std;

cpu::cpu(string file) {
    ifstream addresses;
    string line;
    addresses.open(file);

    if (!addresses.is_open()) {
        cout << "File cannot Open!\n";
    }
    string delimiter = " ";
    while (getline(addresses, line)) {
        
        pair<int, string> pair = make_pair(-1, "");
        
        if (line.find(" ") != string::npos) {
            pair.first = stoi(line.substr(0, line.find(" ")));
            pair.second = line.substr(line.find(" "));
        }
        else {
            pair.first = stoi(line);
        }
        addressList.push(pair);
    }
    addressLength = addressList.size();
    addresses.close();
}

pair<int, string> cpu::getAddress() {
    pair<int, string> p = addressList.front();
    addressList.pop();
    return p;
}









