#include "vm_pager.h"
#include <vector>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"
#include <deque>
using namespace std;
deque<char *> stuff_idk;
char *filename;
char c = 'i';
int main()

{
    for(int i = 0; i < 1000; i++){
        filename = (char *) vm_map(nullptr, 0);
        cout <<  filename << endl;
        string s("shakespeare.txt");
        s.append(to_string(i));
        strcpy(filename, s.c_str());
        cout <<  filename << endl;
        stuff_idk.push_back(filename);
        cout << "end and pushed back" << endl;
    }

    if (fork() != 0) {
        vm_yield();
        cout << "parent" << endl;
        cout <<  filename << endl;
        cout << "fork it fork it" << endl;
    }
    else if(c == 'k'){
        vm_yield();
    }
    else if (c == 'l' || c == 'k'){
        cout << "second";
        vm_yield();
    }
    else {
        cout << "child" << endl;
        cout << "is here" << endl;
        strcpy(filename, "data1.bin");
        cout <<  filename << " is actually out now " << endl;
        cout << endl << endl;
    }  
    return 0; 
}