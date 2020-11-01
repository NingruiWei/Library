#include "vm_pager.h"
#include <iostream>

using namespace std;

int main(){
    char* paool = (char*) vm_map(nullptr, 0);
    char* joan = (char*) vm_map (nullptr, 0);

    paool[0] = '0';
    cout <<"hmm" << endl;
    paool[1] = '1';
    paool[2] = '2';
    paool[3] = '3';
    paool[4] = '4';
    paool[5] = '5';

    paool[1000] = 'a';
    paool[1001] = 'b';
    paool[1002] = 'c';
    paool[1003] = 'd';

    joan[1] = '!';
    joan[2] = '@';
    joan[3] = '#';
    joan[4] = '%';

    cout << *(paool + 3) << endl;
}