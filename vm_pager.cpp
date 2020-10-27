#include "vm_pager.h"
#include <vector>
#include <unordered_map>
#include <cassert>
using namespace std;

unsigned int physmem_size;
unsigned int swap_size;
unsigned int swap_counter = 0;

struct pager_page_t{
    page_table_entry_t base;
    bool reference_bit = 0;
    bool swap_back = 0;
};

struct arena{

    page_table_t page_table;
    pid_t process_id;
    uintptr_t arena_start = uintptr_t(VM_ARENA_BASEADDR);
    uintptr_t arena_valid_end = uintptr_t(VM_ARENA_BASEADDR);

};

unordered_map<pid_t, arena*> arenas;
int curr_pid = -1;
bool first_time(){ // used the first time to know we set curr_pid
    return curr_pid == -1;
}
void vm_init(unsigned int memory_pages, unsigned int swap_blocks){  
    physmem_size = memory_pages;
    swap_size = swap_blocks;
};

int vm_create(pid_t parent_pid, pid_t child_pid){
    if(parent_pid != 0){// 498 part
        assert(false);
        return 0;
    }
    else{
        arena* temp_arena = new arena;
        temp_arena->process_id = child_pid;
        if(first_time()){
            curr_pid = child_pid;
        }
        assert(arenas.find(curr_pid) == arenas.end());
        arenas[curr_pid] = temp_arena;
        return 0;
    }
};

void vm_switch(pid_t pid){
    //search arena table for arena with matching pid
    //evict and writeback any necessary information from the page table leaving
    //switch page_table_base_register to be a pointer to the new page_table_t from the new arena
};



int vm_fault(const void* addr, bool write_flag){



};


void vm_destroy(){



};

void *vm_map(const char *filename, unsigned int block){
    if(arenas[curr_pid]->arena_valid_end == uintptr_t(VM_ARENA_BASEADDR) + VM_ARENA_SIZE){
        //arena is full
        return nullptr;
    }
    if(filename){

    }
    else{
        if(swap_counter < swap_size){ //We need some way to track arena's use of swap blocks
            int first_invalid_page = arena_valid_page_size() + 1;
            page_table_entry_t temp_page;
            temp_page.read_enable = 0;
            temp_page.write_enable = 0;
            page_table_base_register->ptes[first_invalid_page] = temp_page;
            arenas[curr_pid]->arena_valid_end += VM_PAGESIZE;
            return  (void *) arenas[curr_pid]->arena_valid_end;
        }
        else{
            //Not enough swap blocks to hold all swap-backed virtual pages
            return nullptr;
        }
    }
}

int file_read(const char* filename, unsigned int block, void* buf){




};

int file_write(const char* filename, unsigned int block, const void* buf){





};

void* const vm_physmem{


};

int arena_valid_page_size(){
    int arena_size = arena_table[curr_arena_index].arena_valid_end - arena_table[curr_arena_index].arena_start;
    return arena_size / VM_PAGESIZE;
}