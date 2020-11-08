#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"
using namespace std;
int main()
{
    char *filename = (char*) vm_map(nullptr, 0);
    strcpy(filename, "data4.bin");

    char *filebacked_0 = (char*) vm_map(filename, 0);
    char *filebacked_1 = (char*) vm_map(filename, 1);
    char *filebacked_2 = (char*) vm_map(filename, 2);
    //char *filebacked_3 = (char*) vm_map(filename, 3);

    for(int i = 0; i < 512; i++){
        cout << filebacked_0[i] << filebacked_1[i] << filebacked_2[i];// << filebacked_3[i];
    }

    cout << endl << "forking process" << endl;

    if(fork() == 0){

        cout << "child process starting" << endl;
        strcpy(filebacked_0, "data1.bin");
        strcpy(filebacked_1, "data2.bin");
        strcpy(filebacked_2, "data3.bin");
        //strcpy(filebacked_3, "data4.bin");

        for(int i = 0; i < 5; i++){
            char *new_filename = (char*) vm_map(nullptr, 0);
            cout << new_filename[i] << endl;
            cout << filebacked_0[i] << " " << filebacked_1[i] << " " << filebacked_2[i] << endl;// << " " << filebacked_3[i] << endl;

            strcpy(new_filename, "data3.bin");
            cout << new_filename[i] << endl;
        }

        cout << "done with child" << endl;
    }
    else{
        cout << "parent yielding" << endl;
        vm_yield();
        cout << "back to parent" << endl;
        char *filename = (char*) vm_map(nullptr, 0);
        strcpy(filename, "data4.bin");

        char *filebacked_0 = (char*) vm_map(filename, 3);
        char *filebacked_1 = (char*) vm_map(filename, 2);
        char *filebacked_2 = (char*) vm_map(filename, 1);
        //char *filebacked_3 = (char*) vm_map(filename, 0);

        for(int i = 0; i < 512; i++){
            cout << filebacked_0[i] << filebacked_1[i] << filebacked_2[i]; //<< filebacked_3[i];
        }
        cout << endl << "parent done" << endl;
    }

    fork();
    for(int i = 0; i < 512; i++){
        cout << filebacked_0[i] << filebacked_1[i] << filebacked_2[i];// << filebacked_3[i];
    }
    cout << endl;

    cout << "current process finished" << endl;
}