#include "vm_pager.h"
#include "global_general.h"
#include "clock.h"
#include <vector>
#include <unordered_map>
#include <cassert>
#include <queue>
#include <bits/stdc++.h>
#include <iostream>
#include <deque>
using namespace std;

struct process{
    pager_page_table_t *page_table = new pager_page_table_t;
    page_table_t *infrastructure_page_table = new page_table_t;
    pid_t process_id;
    uintptr_t arena_start = uintptr_t(VM_ARENA_BASEADDR);
    uintptr_t arena_valid_end = uintptr_t(VM_ARENA_BASEADDR);
};

struct pager_page_table_t{
    pager_page_t pager_page_table[VM_ARENA_SIZE/VM_PAGESIZE];
};

struct pager_page_t{
    page_table_entry_t* base;
    bool reference_bit = false;
    bool resident_bit = false;
    bool dirty_bit = false;
    const char *filename;
    unsigned int block;
    bool swap_backed;
};

deque<pager_page_t*> clocker;
deque<unsigned int> phys_index;
deque<unsigned int> swap_index;
unordered_map<pid_t, process*> processes;
int curr_pid = -1;
unsigned int physmem_size;
unsigned int swap_size;
unsigned int swap_counter = 0;
unsigned int phys_counter = 1;
const unsigned int buff_index = 0;

void vm_init(unsigned int memory_pages, unsigned int swap_blocks){  
    physmem_size = memory_pages;
    for(int i = 0; i < physmem_size; i++){
        phys_index.push_back(i);
    }
    swap_size = swap_blocks;
    for(int i = 0; i < swap_size; i++){
        swap_index.push_back(i);
    }
    char *buff = new char[VM_PAGESIZE];
    memset (buff, 0, VM_PAGESIZE);
    ((char*)vm_physmem)[buff_index] = *buff;
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
    page_table_base_register = processes[pid]->infrastructure_page_table;
    curr_pid = pid;
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

void evict(){
    while(true){
        if(clocker.front()->reference_bit == true){ //Clock hand pointing at "1"
            clocker.front()->reference_bit = false;
            clocker.push_back(clocker.front());
            clocker.pop_front();
        }
        else{                                       //Clock hand pointing at "0"
            enable_page_protection(clocker.front());
            clocker.front()->resident_bit = 0;
            phys_index.push_back(clocker.front()->base->ppage);
            clocker.pop_front();
            return;
        }
    }
}

void clock_insert(pager_page_t* insert_page){
    if(clocker.size() < physmem_size){
        insert_page->reference_bit = true;
        insert_page->resident_bit = true;
        clocker.push_back(insert_page); //Push back is inserting the element and then moving the hand one place forward
    }
    else{
        evict();
        insert_page->reference_bit = true;
        insert_page->resident_bit = true;
        clocker.push_back(insert_page);
    }
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
        if(swap_counter < swap_size){ //Eager swap reservation check
            assert(!swap_index.empty()); //Double checks that there is still swap space available

            int first_invalid_page = arena_valid_page_size() + 1;
            pager_page_t* temp_page = new pager_page_t;
            temp_page->swap_backed = true;
            temp_page->filename = filename;
            temp_page->block = swap_index.front();
            swap_index.pop_front();
            enable_page_protection(temp_page);
            page_table_base_register->ptes[first_invalid_page] = *temp_page->base;
            processes[curr_pid]->arena_valid_end += VM_PAGESIZE;

            if (phys_counter < physmem_size){
                phys_counter++;
                clock_insert(temp_page);
                return  (void *) (processes[curr_pid]->arena_valid_end - VM_PAGESIZE);
            }
            else{
                swap_counter++;
                clock_insert(temp_page);
                return  (void *) (processes[curr_pid]->arena_valid_end - VM_PAGESIZE);
            }
        }
        else{
            return nullptr;
        }
    }
    return nullptr;
}

int vm_fault(const void* addr, bool write_flag){
    if((unsigned int) addr - processes[curr_pid]->arena_start >= (processes[curr_pid]->arena_valid_end)){ //(Address - Start of Addresss Space) >= End of Valid Address space is an illegal call
        return -1;
    }

    pager_page_t* curr_page = &processes[curr_pid]->page_table->pager_page_table[ ((unsigned int) addr - processes[curr_pid]->arena_start) / VM_PAGESIZE]; //Page address is trying to access
    curr_page->reference_bit = true;

    if(write_flag){ //Trying to write to page

    }
    else{ //Trying to read page
        if(curr_page->resident_bit == false){ //Page is not already resident
            clock_insert(curr_page); //Bring page into residency (within clock)
            curr_page->base->ppage = phys_index.front();
            phys_index.pop_front();

            
        }
        else{ //Page is already resident
            //
        }
    }
}