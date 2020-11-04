#include <vector>
#include "vm_app.h"
#include <iostream>
#include <deque>

using namespace std;

int main()
{
    deque<char*> Paool;

    for (unsigned int i = 0; i < 4097; ++i) {
        Paool.push_back((char *)vm_map(nullptr, 0));

        cout << i << " : " << Paool.back() << endl;

        if (i % 2 == 0) {
            vm_yield();
        }
    }
}