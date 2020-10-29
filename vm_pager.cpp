#include "vm_pager.h"
#include "global_general.h"
#include "clock.h"
#include <vector>
#include <unordered_map>
#include <cassert>
#include <queue>
#include <bits/stdc++.h>
#include <iostream>
using namespace std;

struct process{
    page_table_t *page_table = new page_table_t;
    pid_t process_id;
    uintptr_t arena_start = uintptr_t(VM_ARENA_BASEADDR);
    uintptr_t arena_valid_end = uintptr_t(VM_ARENA_BASEADDR);
};

Clock clocker;
priority_queue<int> phys_pq;
unordered_map<pid_t, process*> processes;
int curr_pid = -1;
unsigned int physmem_size;
unsigned int swap_size;
unsigned int swap_counter = 0;
unsigned int phys_counter = 1;
const unsigned int buff_index = 0;

void vm_init(unsigned int memory_pages, unsigned int swap_blocks){  
    physmem_size = memory_pages;
    swap_size = swap_blocks;
    char *buff = new char[VM_PAGESIZE];
    memset (buff, 0, VM_PAGESIZE);
    ((char*)vm_physmem)[buff_index] = *buff;
    clocker = Clock();
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
    if(processes.find(parent_pid) != processes.end()){ // 498 part
        assert(false);
        return 0;
    }
    else{
        process* temp_process = new process;
        temp_process->process_id = child_pid;
        assert(processes.find(curr_pid) == processes.end());
        processes[child_pid] = temp_process;
        return 0;
    }
}

void vm_switch(pid_t pid){
    //search arena table for arena with matching pid
    //evict and writeback any necessary information from the page table leaving
    //switch page_table_base_register to be a pointer to the new page_table_t from the new arena
    cout << "INSIDE SWITCH" << endl;
    page_table_base_register = processes[pid]->page_table;
    curr_pid = pid;
}



int vm_fault(const void* addr, bool write_flag){


    return -1;
}


void vm_destroy(){



}

int arena_valid_page_size(){
    int arena_size = processes[curr_pid]->arena_valid_end - processes[curr_pid]->arena_start;
    return arena_size / VM_PAGESIZE;
}

void enable_page_protection(pager_page_t* disable_page){
    disable_page->base->read_enable = 0;
    disable_page->base->write_enable = 0;
}

void *vm_map(const char *filename, unsigned int block){
    if(processes[curr_pid]->arena_valid_end == uintptr_t(VM_ARENA_BASEADDR) + VM_ARENA_SIZE){
        //arena is full
        return nullptr;
    }
    if(filename){ // file-backed
        //
        //
    }
    else{ // swap-backed
        if (phys_counter < physmem_size){ // physical memory is full
            int first_invalid_page = arena_valid_page_size() + 1;
            pager_page_t* temp_page = new pager_page_t;
            temp_page->swap_backed = true;
            enable_page_protection(temp_page);
            page_table_base_register->ptes[first_invalid_page] = *temp_page->base; // 
            processes[curr_pid]->arena_valid_end += VM_PAGESIZE;

            // SHOULD ONLY CAST TO (char *) WHEN UTILIZING vm_physmem!
            // ((page_table_entry_t*)vm_physmem)[phys_counter] = *temp_page;

            phys_counter++;
            clocker.insert(temp_page->base, filename, block);
            return  (void *) (processes[curr_pid]->arena_valid_end - VM_PAGESIZE);
        }
        // check if there are enough swap blocks to hold all swap-backed virtual pages (maybe should check both every time?)
        else if(swap_counter < swap_size){ // We need some way to track arena's use of swap blocks
            int first_invalid_page = arena_valid_page_size() + 1;
            pager_page_t* temp_page = new pager_page_t;
            enable_page_protection(temp_page);
            page_table_base_register->ptes[first_invalid_page] = *temp_page->base;
            processes[curr_pid]->arena_valid_end += VM_PAGESIZE;
            //((page_table_entry_t*)vm_physmem)[0] = *temp_page;
            file_write(nullptr, swap_counter, temp_page); //I AM CONCERNED ABOUT THIS LINE. TEMP_PAGE BAD, VM_PHYSMEM BUFFER GOOD. FIX THIS SHIT
            swap_counter++;
            clocker.insert(temp_page->base, filename, block);
            return  (void *) (processes[curr_pid]->arena_valid_end - VM_PAGESIZE);
        }
        else{
            //Not enough swap blocks to hold all swap-backed virtual pages
            return nullptr;
        }
    }
    return nullptr;
}