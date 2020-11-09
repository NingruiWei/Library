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
    char *filename;
    filename = (char *) vm_map(nullptr, 0);
    filename[0] = 'h';
    cout << filename[0];
    cout << "here" << endl;
    strcpy(filename, "shakespeare.txt");
    filename = (char*) vm_map(filename, 0);
    cout << "again" << endl;
    cout << filename[0];
    filename[0] = '@';
    cout << "wtf" << endl;
    cout << filename[0];
    filename = (char*) vm_map(nullptr, 0);
    cout << filename[0];
    filename[0] = '[';
    cout << "uh hello" << endl;
    cout << filename[0];
    cout << "end it all " << endl;
   
}