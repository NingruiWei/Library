#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"
using namespace std;
int main()
{
    fork();
    //fork();
    
    /* Allocate swap-backed page from the arena */
    cout << "APP STARTED" << endl;
    char *filename = (char *) vm_map(nullptr, 0);
    cout << "after initial map with nullptr" << endl;
    /* Write the name of the file that will be mapped */
    strcpy(filename, "shakespeare.txt");
    cout << "strcpy called" << endl;

    /* Map a page from the specified file */
    char *p = (char *) vm_map (filename, 0);

    /* Print the first speech from the file */
    for (unsigned int i=0; i<2561; i++) {
	    cout << p[i];
    }

    if(fork() == 0){
        cout << "Child process begin" << endl;
        cout << filename << endl;

        strcpy(filename, "data4.bin");

        cout << filename << endl;

        vm_yield();

        p = (char *) vm_map (filename, 0);

        for (unsigned int i=0; i<2561; i++) {
            cout << p[i];
        }

        cout << "Child process end" << endl;
    }
    else{
        cout << "Parent process begin" << endl;
        cout << filename << endl;
        vm_yield();
        cout << filename << endl;
        cout << "Parent process end" << endl;
    }
}