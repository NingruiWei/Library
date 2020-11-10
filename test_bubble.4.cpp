#include <iostream>
#include <cstring>
#include <unistd.h>
#include <vector>
#include <string>
#include "vm_app.h"

using namespace std;


//Adapted from geeksforgeeks bubble sort for ease of use
//Source: https://www.geeksforgeeks.org/bubble-sort/
void bubbleSort(char* arr, int n)  
{  
    for (int i = 0; i < n-1; i++) { 
        // Last i elements are already in place  
        for (int j = 0; j < n-i-1; j++){
            if (arr[j] > arr[j+1]){
                swap(arr[j], arr[j+1]);
            }
        }
    }
}  

int main(){
    if(fork() == 0){
        if(fork() == 0){
            char* filename_p1 = (char *) vm_map(nullptr, 0);
            char* filename_p2 = (char *) vm_map(nullptr, 0);

            filename_p1[4091] = 'd';
            filename_p1[4092] = 'a';
            filename_p1[4093] = 't';
            filename_p1[4094] = 'a';
            filename_p1[4095] = '4';
            filename_p2[0] = '.';
            filename_p2[1] = 'b';
            filename_p2[2] = 'i';
            filename_p2[3] = 'n';
            filename_p2[4] = '\0';

            char* file_p1 = (char *) vm_map(filename_p1 + 4091, 0);
            char *file_p2 = (char *) vm_map(filename_p1 + 4091, 0);


            cout << "Uppermost IF. File_p1 sorted output" << endl;
            bubbleSort(file_p1, VM_PAGESIZE);
            for(size_t i = 0; i < 25; i++){
                cout << file_p1[i];
            }
            cout << endl;

            cout << "Uppermost IF. File_p2 sorted output" << endl;
            bubbleSort(file_p2, VM_PAGESIZE);
            for(size_t i = 0; i < 25; i++){
                cout << file_p2[i];
            }
            cout << endl;

            cout << "Outputs should match" << endl;
        }
        else{
            char* filename_p1 = (char *) vm_map(nullptr, 0);
            char* filename_p2 = (char *) vm_map(nullptr, 0);

            filename_p1[4091] = 'd';
            filename_p1[4092] = 'a';
            filename_p1[4093] = 't';
            filename_p1[4094] = 'a';
            filename_p1[4095] = '4';
            filename_p2[0] = '.';
            filename_p2[1] = 'b';
            filename_p2[2] = 'i';
            filename_p2[3] = 'n';
            filename_p2[4] = '\0';

            char* file_p1 = (char *) vm_map(filename_p1 + 4091, 0);
            char *file_p2 = (char *) vm_map(filename_p1 + 4091, 0);
            
            strcpy(filename_p1, "data3.bin");

            char* file_p3 = (char *) vm_map(filename_p1, 1);

            cout << "Inner ELSE. File_p1 sorted output" << endl;
            bubbleSort(file_p1, VM_PAGESIZE);
            for(size_t i = 0; i < 25; i++){
                cout << file_p1[i];
            }
            cout << endl;

            cout << "Inner ELSE. File_p2 sorted output" << endl;
            bubbleSort(file_p2, VM_PAGESIZE);
            for(size_t i = 0; i < 25; i++){
                cout << file_p2[i];
            }
            cout << endl;

            cout << "Outputs should match" << endl;

            file_p3[0] = 'z';
            file_p3[1] = 'y';
            file_p3[2] = 'x';
            file_p3[3] = 'w';

            char* file_p4 = (char *) vm_map(filename_p1, 1);

            cout << "Inner ELSE. Modified File_p3 sorted output" << endl;
            bubbleSort(file_p3, VM_PAGESIZE);
            for(size_t i = 0; i< 25; i++){
                cout << file_p3[i];
            }
            cout << endl;

            cout << "Inner ELSE. File_p4 sorted output" << endl;
            bubbleSort(file_p4, VM_PAGESIZE);
            for(size_t i = 0; i< 25; i++){
                cout << file_p4[i];
            }
            cout << endl;

            cout << "Outputs should NOT match" << endl;
        }
    }
    else{
        char* filename_p1 = (char *) vm_map(nullptr, 0);
        char* filename_p2 = (char *) vm_map(nullptr, 0);

        filename_p1[4091] = 'd';
        filename_p1[4092] = 'a';
        filename_p1[4093] = 't';
        filename_p1[4094] = 'a';
        filename_p1[4095] = '4';
        filename_p2[0] = '.';
        filename_p2[1] = 'b';
        filename_p2[2] = 'i';
        filename_p2[3] = 'n';
        filename_p2[4] = '\0';

        char* file_p1 = (char *) vm_map(filename_p1 + 4091, 0);
        char *file_p2 = (char *) vm_map(filename_p1 + 4091, 0);
        char *file_p3 = (char *) vm_map(filename_p1, 1);

        cout << "Outer ELSE. File_p1 sorted output" << endl;
        bubbleSort(file_p1, VM_PAGESIZE);
        for(size_t i = 0; i < 25; i++){
            cout << file_p1[i];
        }
        cout << endl;

        cout << "Outer ELSE. File_p2 sorted output" << endl;
        bubbleSort(file_p2, VM_PAGESIZE);
        for(size_t i = 0; i < 25; i++){
            cout << file_p2[i];
        }
        cout << endl;

        cout << "Outer ELSE. File_p3 sorted output" << endl;
        bubbleSort(file_p3, VM_PAGESIZE);
        for(size_t i = 0; i< 25; i++){
            cout << file_p3[i];
        }
        cout << endl;

        cout << "First two ouputs should match, 3rd Output should NOT match" << endl;
    }

    cout << "Process finished" << endl;
}