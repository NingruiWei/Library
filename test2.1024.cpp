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
    /* Print the first speech from the file */
    for (unsigned int i=0; i<1000; i++) {
	    cout << paul[i];
    }
    cout << endl;

    char *ningrui = (char *) vm_map (filename, 10);
    for (unsigned int i=0; i<1000; i++) {
	    cout << ningrui[i];
    }
    cout << endl;

    strcpy(paul, "shakespeare.txt");
    for (unsigned int i=0; i<1000; i++) {
        cout << paul[i];
    }
    cout << endl;

    ningrui = (char *) vm_map(nullptr, 0);
    for (unsigned int i=0; i<1000; i++) {
	    cout << ningrui[i];
    }
    cout << endl;
    
}