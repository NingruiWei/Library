#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"
#include <deque>
using namespace std;
deque<char *> stuff_idk;
string s;


int main()
{
    int i = 0;
    while( i < 2000){
        cout << i << "ith loop" << endl;
        char *filename = (char *) vm_map(nullptr, 0);
        s = "shakespeare.txt";
        // s += to_string(i);
        strcpy(filename, s.c_str());
        stuff_idk.push_back(filename);
        cout << "stuff is pushed now";
        char *new_check = (char *) vm_map (filename, 0);
        cout << "new one" << endl;
        ++i;
    }
    
}