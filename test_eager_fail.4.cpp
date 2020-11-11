#include <iostream>
#include <unistd.h>
#include "vm_app.h"

using namespace std;

int main() {
	char *timothy = (char*)vm_map(nullptr, 0);
	
	timothy[4095] = 't';
    for (size_t i = 0; i < 55; i++) {
    	if (fork() != 0) {
    		cout << "Fork returned 1" << endl;
    	}
		else {
			vm_yield();
		}
    }
    char *de_stroyed = (char*) vm_map(nullptr, 1);
}
