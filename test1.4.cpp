#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

//using std::cout;
using namespace std;

int main()
{
    /* Allocate swap-backed page from the arena */
    cout << "APP STARTED" << endl;
    char *filename = (char *) vm_map(nullptr, 0);

    /* Write the name of the file that will be mapped */
    cout << "hi lol adsf" << endl;
    strcpy(filename, "shakespeare.txt");
    cout << "big pp" << endl;

    /* Map a page from the specified file */
    char *p = (char *) vm_map (filename, 0);
    char* matt = (char*) vm_map (filename, 0);
    /* Print the first speech from the file */
    for (unsigned int i=0; i<2561; i++) {
	    cout << p[i];
    }
    for (unsigned int i=0; i<2561; i++) {
	    matt[i] = 'b';
    }

    for (unsigned int i=0; i<2561; i++) {
	    cout << p[i];
    }
    


}