#include "vm_pager.h"
using namespace std;


void vm_init(unsigned int memory_pages, unsigned int swap_blocks){


}

int vm_create(pid_t parent_pid, pid_t child_pid){



}

void vm_switch(pid_t pid){



}



int vm_fault(const void* addr, bool write_flag){



}


void vm_destroy(){



}

void *vm_map(const char *filename, unsigned int block){


    
}

int file_read(const char* filename, unsigned int block, void* buf){




};

int file_write(const char* filename, unsigned int block, const void* buf){





};

void* const vm_physmem{


};


page_table_t* page_table_base_register{

}