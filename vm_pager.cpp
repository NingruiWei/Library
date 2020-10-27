#include "vm_pager.h"
#include <vector>
using namespace std;

unsigned int physmem_size;
unsigned int swap_size;
struct pager_page_t{
    page_table_entry_t base;
    bool reference_bit = 0;
};
struct arena{

    page_table_t page_table;
    pid_t process_id;
    pid_t parent_id;
};
vector<arena> arena_table;

void vm_init(unsigned int memory_pages, unsigned int swap_blocks){  
    physmem_size = memory_pages;
    swap_size = swap_blocks;
};

int vm_create(pid_t parent_pid, pid_t child_pid){
    if(parent_pid){

    }
    else{
    arena temp_arena;
    temp_arena.parent_id = parent_pid; // null or w/e
    temp_arena.process_id = child_pid;
    arena_table.push_back(temp_arena);
    }
};

void vm_switch(pid_t pid){



};



int vm_fault(const void* addr, bool write_flag){



};


void vm_destroy(){



};

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