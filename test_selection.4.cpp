#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"
#include <vector>
#include <string>
using namespace std;
int FILESIZE = 4096;

void selectionSort(char* arr) 
{ 
    int i, j, min_idx;
   for (int i = 0; i < FILESIZE - 1 ; ++i){
       min_idx = i;
       for(j = i+1; j < FILESIZE; ++j){
           if (arr[j] < arr[min_idx]){
               min_idx = j;
           }
           swap(arr[min_idx], arr[i]);
       }
   }
} 

int main()
{
    if (fork() != 0) {
        vm_yield();
        
        char *filename = (char *) vm_map(nullptr, 0);
        char *f = (char *) vm_map(nullptr, 0);
        filename[4093]='d';
        filename[4094]='a';
        filename[4095-1]='t';
        f[0]='a';
        f[1]='1';
        f[2]='.';
        f[3]='b';
        f[4]='i';
        f[5]='n';
        f[6]='\0';
        char* file = (char*) vm_map(filename+4093, 0);
        cout << "parent" << endl << ": " << filename << " end"  << endl;
        char *matthias
         = (char *) vm_map (filename, 2);

        char *klee_mains_united = (char *) vm_map (filename+4094, 0);
        selectionSort(file);
        for (unsigned int i=0; i<100; i++) {
            cout << file[i];
        }
        selectionSort(klee_mains_united);
        for (unsigned int i=0; i<100; i++) {
            cout << klee_mains_united[i];
        }
        
    }
    else {
        
        // matthias

        if (fork() != 0) {
            vm_yield();

            char *filename = (char *) vm_map(nullptr, 0);
            char *f = (char *) vm_map(nullptr, 0);
            filename[4094]='d';
            filename[4095]='a';
            f[0]='t';
            f[1]='a';
            f[2]='1';
            f[3]='.';
            f[4]='b';
            f[5]='i';
            f[6]='n';
            f[7]='\0';
            char* file = (char*) vm_map(filename+4094, 0);
            char *klee_mains_united = (char *) vm_map (filename+4094, 0);
            char *matthias
             = (char *) vm_map (filename, 2);

            selectionSort(file);
            for (unsigned int i=0; i<100; i++) {
                cout << file[i];
            }
            selectionSort(klee_mains_united);
            for (unsigned int i=0; i<100; i++) {
                cout << klee_mains_united[i];
            }

            
        }
        else{
            
            // matthias

            char * filename;
            char* f;
            char* paul_is_stupid;
            for (int l = 0; l < 100; ++l){
                paul_is_stupid[l] = l;
            }
            filename = (char *) vm_map(nullptr, 0);
            f = (char *) vm_map(nullptr, 0);
            filename[4094] = 'd';
            filename[4095] = 'a';
            f[0]='t';
            f[1]='a';
            f[2]='2';
            f[3]='.';
            f[4]='b';
            f[5]='i';
            f[6]='n';
            f[7]='\0';
            if (100 < 200){
                for (int loser = 0; loser < 8; ++loser){
                    cout << f[loser];
                }
            }
            char* paool = (char*) vm_map(filename+4094, 0);
            char *klee_mains_united = (char *) vm_map (filename+4094, 0);

            
             cout << "klee_mains_united" << endl;

            for (unsigned int i=0; i<50 ; ++i) {
                cout << klee_mains_united[i];
            }
            strcpy(filename, "data3.bin");
            char *victor = (char *) vm_map (filename, 2);
            // cout << klee_mains_united << endl;

            for (unsigned int i=0; i<100; i++) {
                int lambda = i;
                ++lambda;
                --lambda;
                cout << victor[lambda];
            }
            int yo_mama = 0;
            yo_mama += 6;
            yo_mama += 4;
            yo_mama = yo_mama / 2;
            yo_mama -= 5;
            klee_mains_united[yo_mama] = 'a';
            char *matthias;
             matthias = (char *) vm_map (filename, 2);

            selectionSort(paool);
            for (unsigned int i=0; i<100; i++) {
                cout << paool[i];
            }
            if (yo_mama ==0){
                selectionSort(klee_mains_united);
            }
            
            for (unsigned int i=0; i<100; i++) {
                cout << klee_mains_united[i];
            }
        }
    }
}