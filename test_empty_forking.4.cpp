#include <iostream>
#include <unistd.h>
#include "vm_app.h"
#include <cstring>
#include <algorithm>
#include <functional>
#include <array>
#include <iostream>

using namespace std;

void printing(char* passed) {
    cout << passed << endl;
}

void sorting(char* arr) {
    size_t arraySize = sizeof(arr)/sizeof(*arr);
    std::sort(arr, arr+arraySize);
}

int main(){
    pid_t f1 = fork();
    if (f1 == 0) {
        pid_t f2 = fork();
        if (f2 == 0) {
            char *jean = (char *) vm_map(nullptr, 0);
            char *klee = (char *) vm_map(nullptr, 0);
            jean[4093] = 'd';
            jean[4094] = 'a';
            jean[4095] = 't';
            klee[0] = 'a';
            klee[1] = '3';
            klee[2] = '.';
            klee[3] = 'b';
            klee[4] = 'i';
            klee[5] = 'n';
            klee[6] = '\0';

            char *venti = (char *) vm_map(&jean[4093], 0);
            for (size_t i = 0; i < 123; i++) {
                cout << venti[i];
            }
            cout << endl;
            cout << "INNER FORK" << endl;

            char *mona = (char *) vm_map(&jean[4093], 0);
            strcpy(jean, "data4.bin");
            printing(jean);

            sorting(mona);
            for (size_t i = 0; i < 123; i++) {
                cout << mona[i];
            }
            mona[1] = 'b';
            sorting(mona);
            for (size_t i = 0; i < 123; i++) {
                cout << mona[i];
            }

            char *razor = (char *) vm_map(jean, 3);
            char *cheese = (char *) vm_map(jean, 3);
        }
        else {
            vm_yield();
            char *jean = (char *) vm_map(nullptr, 0);
            char *klee = (char *) vm_map(nullptr, 0);
            jean[4093] = 'd';
            jean[4094] = 'a';
            jean[4095] = 't';
            klee[0] = 'a';
            klee[1] = '3';
            klee[2] = '.';
            klee[3] = 'b';
            klee[4] = 'i';
            klee[5] = 'n';
            klee[6] = '\0';

            char *venti = (char *) vm_map(&jean[4093], 0);
            for (size_t i = 0; i < 123; i++) {
                cout << venti[i];
            }
            cout << endl;
            sorting(venti);
            for (size_t i = 0; i < 123; i++) {
                cout << venti[i];
            }
            cout << "SORTED VENTI INNER INNER" << endl;

            char *mona = (char *) vm_map(&jean[4093], 0);
            sorting(mona);
            for (size_t i = 0; i < 123; i++) {
                cout << mona[i];
            }
            cout << "MONA SORTED" << endl;
            char *razor = (char *) vm_map(jean, 3);
        }
    }
    else {
        vm_yield();
        char *jean = (char *) vm_map(nullptr, 0);
            char *klee = (char *) vm_map(nullptr, 0);
            jean[4093] = 'd';
            jean[4094] = 'a';
            jean[4095] = 't';
            klee[0] = 'a';
            klee[1] = '3';
            klee[2] = '.';
            klee[3] = 'b';
            klee[4] = 'i';
            klee[5] = 'n';
            klee[6] = '\0';

            char *venti = (char *) vm_map(&jean[4093], 0);
            char *razor = (char *) vm_map(jean, 3);
            printing(jean);
            for (size_t i = 0; i < 123; i++) {
                cout << venti[i];
            }
            cout << endl;
            sorting(venti);
            for (size_t i = 0; i < 123; i++) {
                cout << venti[i];
            }
            cout << "SORTED VENTI INNER INNER" << endl;

            char *mona = (char *) vm_map(&jean[4093], 0);
            sorting(mona);
            for (size_t i = 0; i < 123; i++) {
                cout << mona[i];
            }
            cout << "MONA SORTED" << endl;
    }
    cout << "MADE IT TO END" << endl;
}