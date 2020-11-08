#include <iostream>
#include <cstring>
#include <string>
#include <unistd.h>
#include "vm_app.h"

//using std::cout;
using namespace std;

void pushing(string &str, char *file) {
    for (size_t i = 0; i < (VM_PAGESIZE + 1); i++) {
        str.push_back(file[0]);
    }
}

void pretty_printing() {
    for (unsigned int i=0; i<10; i++) {
        char *pito = (char *) vm_map (nullptr, 0);
	    cout << *pito << endl;
        strcpy(pito, "data2.bin");
    }
}

int main()
{
    /* Allocate swap-backed page from the arena */
    cout << "APP STARTED" << endl;
    char *filename = (char *) vm_map(nullptr, 0);
    char *diff = (char *) vm_map(nullptr, 0);
    strcpy(diff, "d");
    /* Write the name of the file that will be mapped */
    string change = "";
    pushing(change, diff);
    const char *changed = change.c_str();
    strcpy(filename, changed);
    cout << "strcpy called" << endl;
    cout << filename << endl;

    /* Map a page from the specified file */
    char *p = (char *) vm_map (filename, 0);
    for (size_t i = 0; i < 100; i++) {
        cout << p[i] << endl;
    }

    pretty_printing();
}