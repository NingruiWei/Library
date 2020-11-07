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
    char* file_1 = (char*) vm_map(nullptr, 0);
    char* file_2 = (char*) vm_map(nullptr, 0);

    file_1[4093] = 'd';
    file_1[4094] = 'a';
    file_1[4095] = 't';
    file_2[0] = 'a';
    file_2[1] = '2';
    file_2[2] = '.';
    file_2[3] = 'b';
    file_2[4] = 'i';
    file_2[5] = 'n';
    file_2[6] = '\0';
    
    char* file_3 = (char*) vm_map(nullptr, 0);
    char* file_4 = (char*) vm_map(nullptr, 0);
    char* venti = (char*) vm_map(&file_1[4093], 0);
    char* lisa = (char*) vm_map(&file_1[4093], 0);
    char* jean = (char*) vm_map(&file_1[4093], 0);
    char* mona = (char*) vm_map(&file_1[4093], 5);

    // printing(venti);
    // printing(lisa);
    // printing(jean);
    // printing(mona);

    jean[0] = 'p';

    for (size_t i = 0; i < 123; i++) {
        cout << venti[i];
    }

    for (size_t i = 0; i < 123; i++) {
        cout << mona[i];
    }

    for (size_t i = 0; i < 123; i++) {
        cout << jean[i];
    }

    pid_t ret = fork();
    if (ret == 0) {
        cout << "WE IN" << endl;
        strcpy(file_1, "data3.bin");
        printing(file_1);

        char* diluc = (char*) vm_map(file_1, 1);
        char* klee = (char*) vm_map(file_1, 1);

        for (size_t i = 0; i < 123; i++) {
            cout << diluc[i];
        }

        pid_t ret1 = fork();
        if (ret1 == 0) {
            sorting(jean);
            sorting(lisa);
            strcpy(mona, "shakespeare.txt");
            printing(mona);

            for (size_t i = 0; i < 123; i++) {
                cout << jean[i];
            }

            for (size_t i = 0; i < 123; i++) {
                cout << lisa[i];
            }

            for (size_t i = 0; i < 123; i++) {
                cout << mona[i];
            }
        }
        else {
            vm_yield();
            sorting(jean);
            sorting(lisa);
            strcpy(mona, "shakespeare.txt");
            printing(mona);

            for (size_t i = 0; i < 123; i++) {
                cout << jean[i];
            }

            for (size_t i = 0; i < 123; i++) {
                cout << lisa[i];
            }

            for (size_t i = 0; i < 123; i++) {
                cout << mona[i];
            }
        }
    }
    else {
        cout << "INSIDE THE LAST ELSE" << endl;
        vm_yield();
        sorting(jean);
        sorting(lisa);
        strcpy(mona, "shakespeare.txt");
        printing(mona);

        for (size_t i = 0; i < 123; i++) {
            cout << jean[i];
        }

        for (size_t i = 0; i < 123; i++) {
            cout << lisa[i];
        }

        for (size_t i = 0; i < 123; i++) {
            cout << mona[i];
        }
    }
    cout << "MADE IT TO END" << endl;
}