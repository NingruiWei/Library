#include <iostream>
#include "vm_app.h"
using namespace std;
int main()
{
    // tests filenames over two blocks

    /* Allocate swap-backed page from the arena */
    char *part_1 = (char *) vm_map(nullptr, 0);
    char *part_2 = (char *) vm_map(nullptr, 0);

    /* Write the name of the file that will be mapped */
    // strcpy(filename1, ('*' * 4090));
    part_1[4091] = 'd';
    part_1[4092] = 'a';
    part_1[4093] = 't';
    part_1[4094] = 'a';
    part_1[4095] = '4';
    part_2[0] = '.';
    part_2[1] = 'b';
    part_2[2] = 'i';
    part_2[3] = 'n';
    part_2[4] = '\0';

    cout << "Filename written" << endl;
    char *p = (char *) vm_map (&part_1[4091], 0);
    cout << "file mapped" << endl;
    for (unsigned int i=0; i<100; i++) {
	cout << p[i];
    }

    cout << endl;
}