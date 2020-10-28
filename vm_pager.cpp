#include "vm_pager.h"
#include <vector>
#include <unordered_map>
#include <cassert>
#include <queue>
#include <bits/stdc++.h>
#include <iostream>
using namespace std;

struct pager_page_t{
    page_table_entry_t base;
    bool reference_bit = 0;
    bool swap_back = 0;
};

struct arena{
    page_table_t *page_table = new page_table_t;
    pid_t process_id;
    uintptr_t arena_start = uintptr_t(VM_ARENA_BASEADDR);
    uintptr_t arena_valid_end = uintptr_t(VM_ARENA_BASEADDR);

};

priority_queue<int> phys_pq;
unordered_map<pid_t, arena*> arenas;
int curr_pid = -1;
unsigned int physmem_size;
unsigned int swap_size;
unsigned int swap_counter = 0;
unsigned int phys_counter = 1;

void vm_init(unsigned int memory_pages, unsigned int swap_blocks){  
    physmem_size = memory_pages;
    swap_size = swap_blocks;
}

int find_next_physmem_index(){
    if (phys_pq.empty()){
        phys_pq.push(1);
        return 0;
    }
    int temp = phys_pq.top();
    phys_pq.pop();
    if(phys_pq.empty()){

        phys_pq.push(temp + 1);
    }
    return temp;
}

int vm_create(pid_t parent_pid, pid_t child_pid){
    if(arenas.find(parent_pid) != arenas.end()){ // 498 part
        assert(false);
        return 0;
    }
    else{
        arena* temp_arena = new arena;
        temp_arena->process_id = child_pid;
        assert(arenas.find(curr_pid) == arenas.end());
        arenas[child_pid] = temp_arena;
        return 0;
    }
}

void vm_switch(pid_t pid){
    //search arena table for arena with matching pid
    //evict and writeback any necessary information from the page table leaving
    //switch page_table_base_register to be a pointer to the new page_table_t from the new arena
    cout << "INSIDE SWITCH" << endl;
    page_table_base_register = arenas[pid]->page_table;
    curr_pid = pid;
}



int vm_fault(const void* addr, bool write_flag){


    return -1;
}


void vm_destroy(){



}

int arena_valid_page_size(){
    int arena_size = arenas[curr_pid]->arena_valid_end - arenas[curr_pid]->arena_start;
    return arena_size / VM_PAGESIZE;
}

void enable_page_protection(page_table_entry_t* disable_page){
    disable_page->read_enable = 0;
    disable_page->write_enable = 0;
}

void *vm_map(const char *filename, unsigned int block){
    if(arenas[curr_pid]->arena_valid_end == uintptr_t(VM_ARENA_BASEADDR) + VM_ARENA_SIZE){
        //arena is full
        return nullptr;
    }
    if(filename){
        //
        //
    }
    else{
        if (phys_counter < physmem_size){
            int first_invalid_page = arena_valid_page_size() + 1;
            page_table_entry_t* temp_page = new page_table_entry_t;
            enable_page_protection(temp_page);
            page_table_base_register->ptes[first_invalid_page] = *temp_page;
            arenas[curr_pid]->arena_valid_end += VM_PAGESIZE;
            ((page_table_entry_t*)vm_physmem)[phys_counter] = *temp_page;
            phys_counter++;
            return  (void *) arenas[curr_pid]->arena_valid_end - VM_PAGESIZE;
        }
        else if(swap_counter < swap_size){ //We need some way to track arena's use of swap blocks
            int first_invalid_page = arena_valid_page_size() + 1;
            page_table_entry_t* temp_page = new page_table_entry_t;
            enable_page_protection(temp_page);
            page_table_base_register->ptes[first_invalid_page] = *temp_page;
            arenas[curr_pid]->arena_valid_end += VM_PAGESIZE;
            ((page_table_entry_t*)vm_physmem)[0] = *temp_page;
            file_write(nullptr, swap_counter, temp_page); //I AM CONCERNED ABOUT THIS LINE. TEMP_PAGE BAD, VM_PHYSMEM BUFFER GOOD. FIX THIS SHIT
            swap_counter++;
            return  (void *) arenas[curr_pid]->arena_valid_end - VM_PAGESIZE;
        }
        else{
            //Not enough swap blocks to hold all swap-backed virtual pages
            return nullptr;
        }
    }
    return nullptr;
}