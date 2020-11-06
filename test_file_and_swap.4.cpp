#include <iostream>
#include <cstring>
#include "vm_app.h"
using namespace std;
int main()
{
    // tests filenames over two blocks

    /* Allocate swap-backed page from the arena */
    char *swap_1 = (char *) vm_map(nullptr, 0);
    char *swap_2 = (char *) vm_map(nullptr, 0);

    strcpy(swap_1, "data1.bin");
    swap_2[0] = 'd';
    swap_2[1] = 'a';
    swap_2[2] = 't';
    swap_2[3] = 'a';
    swap_2[4] = '2';
    swap_2[5] = '.';
    swap_2[6] = 'b';
    swap_2[7] = 'i';
    swap_2[8] = 'n';

    char *file_1 = (char *) vm_map(swap_1, 0);
    char *file_2 = (char *) vm_map(swap_2, 0);

    swap_1[0] = 'd';
    swap_1[1] = 'a';
    swap_1[2] = 't';
    swap_1[3] = 'a';
    swap_1[4] = '3';
    swap_1[5] = '.';
    swap_1[6] = 'b';
    swap_1[7] = 'i';
    swap_1[8] = 'n';
    strcpy(swap_2, "data4.bin");

    char *file_3 = (char *) vm_map(swap_1, 0);
    char *file_4 = (char *) vm_map(swap_2, 0);

    for(int i = 0; i < 15; i++){
        cout << file_1[i] << endl;
        file_1[i] = file_2[i];
        cout << file_1[i] << endl;
        file_2[i+1] = file_3[i];
        file_1[i] = file_4[i+1];
        cout << swap_1[i] << " " << swap_2[i] << endl;
    }
}