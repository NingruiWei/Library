#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using namespace std;

int main()
{
    /* Allocate swap-backed page from the arena */
    char *filename = (char *) vm_map(nullptr, 0);
    cout << filename << endl;

    /* Write the name of the file that will be mapped */
    strcpy(filename, "data2.bin");
    cout << filename << endl;

    /* Map a page from the specified file */
    char *paul = (char *) vm_map (filename, 0);

    if (fork() == 0) {
        cout << "FIRST CHILD" << endl;
        strcpy(paul, "data1.bin");
        for (unsigned int i=0; i<100; i++) {
            cout << paul[i];
        }
    }
    else {
        cout << "INSIDE ELSE" << endl;
        if (fork() == 0) {
            cout << "SECOND CHILD" << endl;
            strcpy(paul, "data2.bin");
            for (unsigned int i=0; i<100; i++) {
                cout << paul[i];
            }
        }
        else {
            cout << "SHAKE AND BAKE" << endl;
            strcpy(paul, "shakespeare.txt");
            for (unsigned int i=0; i<100; i++) {
                cout << paul[i];
            }
        }
    }

    cout << "end" << endl;  
}