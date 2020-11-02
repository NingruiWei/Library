#include <iostream>
#include <cstring>
#include "vm_app.h"

using namespace std;
void print_block(char* stuff){
    for (unsigned int i = 0; i < 30; i++){
        cout << stuff[i];
    }
    cout << endl;

}

int main() {
	/* Allocate swap-backed page from the arena */
	char *filename = (char *)vm_map(nullptr, 0);
	strcpy(filename, "data1.bin");
	char *mattias = (char *)vm_map(filename, 0);
	char *victor = (char *)vm_map(filename, 1);
	char *superman = (char *)vm_map(filename, 2);
	char *paul = (char *)vm_map(filename, 2);

	/* Print the first speech from the file */
	for (unsigned int i = 0; i < 30; i++) {
		cout << mattias[i] << victor[i] << superman[i] << paul[i] << endl;
	}

	for (unsigned int i = 0; i < 30; i++) {
		paul[i] = '&';
		cout << "0";
	}
	cout << endl;

	for (unsigned int i = 0; i < 30; i++) {
		cout << mattias[i] << victor[i] << superman[i] << paul[i] << endl;
	}

    cout << "stuff happens" << endl;

	for (unsigned int i = 0; i < 30; i++) {
        mattias[i] = '<';
		superman[i] = 'h';
        victor[i] = '>';
	}
    print_block(superman);
    print_block(mattias);
    print_block(victor);
    cout << "break" << endl;
	for (unsigned int i = 0; i < 30; i++) {
		cout << mattias[i] << victor[i] << superman[i] << paul[i] << endl;
	}
    cout << "break2" << endl;

    
	for (unsigned int i = 0; i < 30; i++) {
		if (i % 2 == 1)
			cout << superman[i];
		else
			superman[i] = '0';
	}

    print_block(superman);

}