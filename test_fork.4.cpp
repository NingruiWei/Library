#include <iostream>
#include <unistd.h>
#include "vm_app.h"

using namespace std;

void pretty_print(char* passed_in){
    cout << passed_in[0] << " ";
}

int main(){
    char* file_1 = (char*) vm_map(nullptr, 0);
    char* file_2 = (char*) vm_map(nullptr, 0);

    // file_1[0] = '0';
    // file_2[0] = '0';

    pretty_print(file_1);
    pretty_print(file_2);
    cout << endl;

    file_2[0] = '2';

    char* file_3 = (char*) vm_map(nullptr, 0);

    // file_3[0] = '0';

    file_1[0] = '1';
    file_2[0] = 'c';

    pretty_print(file_1);
    pretty_print(file_2);
    pretty_print(file_3);
    cout << endl;

    file_3[0] = file_2[0];

    pretty_print(file_1);
    pretty_print(file_2);
    pretty_print(file_3);
    cout << endl;

    char* file_4 = (char*) vm_map(nullptr, 0);
    char* file_5 = (char*) vm_map(nullptr, 0);

    // file_4[0] = '0';
    // file_5[0] = '0';

    file_4[0] = '4';

    pretty_print(file_1);
    pretty_print(file_2);
    pretty_print(file_3);
    pretty_print(file_4);
    pretty_print(file_5);
    cout << endl;

    file_5[0] = '5';

    pretty_print(file_1);
    pretty_print(file_2);
    pretty_print(file_3);
    pretty_print(file_4);
    pretty_print(file_5);
    cout << endl;

    file_5[0] = '9';

    char* file_6 = (char*) vm_map(nullptr, 0);

    // file_6[0] = '0';

    //fork();
    //vm_yield();

    char* file_7 = (char*) vm_map(nullptr, 0);

    // file_7[0] = '0';

    while(1){
        if(file_1[0] < file_5[0]){
            file_1[0]++;
            file_2[0]--;
            file_1[0]++;
            file_1[0]++;
        }
        else if(file_1[0] > file_5[0]){
            file_3[0] += file_4[0];
            file_1[0]--;
        }
        else{
            break;
        }
        pretty_print(file_1);
        pretty_print(file_2);
        pretty_print(file_3);
        pretty_print(file_4);
        pretty_print(file_5);
        cout << endl;
    }

    pretty_print(file_1);
    pretty_print(file_2);
    pretty_print(file_3);
    pretty_print(file_4);
    pretty_print(file_5);
    cout << endl;

    cout << "done" << endl;
}