#include "vm_pager.h"
#include <iostream>

using namespace std;

int main(){



    char* p;
    p = (char*) vm_map(nullptr, 0);
    p[0] = 'm';
    p[1] = 'a';
    p[2] = 't';
    p[3] = 't';
    p[4] = 'h';
    p[5] = 'i';
    p[6] = 'a';
    p[7] = 's';
    p[8] = p[7];
    p[9] = p[6];
    p[10] = p[5];
    p[11] = p[4];
    p[12] = p[3];
    p[13] = p[2];
    p[14] = p[1];
    p[15] = p[0];
    // p[16] = p[10];
    // p[17] = p[3];
    // p[18] = p[14];
    // p[19] = p[3];
    // p[11] = p[19];
    // p[13] = p[13];
    // p[15] = p[0];
    // p[15] = p[1];
    cout << "hello peoples" << endl;
    for (int i = 16 ; i < 5000; ++i){
        p[i] = 'h';
    }

    for (int i = 0 ; i < 5000; ++i){
        cout << p[i];
    }

    
}
