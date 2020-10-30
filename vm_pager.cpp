#include "vm_pager.h"
#include "global_general.h"
#include <vector>
#include <unordered_map>
#include <cassert>
#include <queue>
#include <bits/stdc++.h>
#include <iostream>
#include <deque>
using namespace std;

struct pager_page_t{
    page_table_entry_t* base = nullptr;
    bool reference_bit = false;
    bool resident_bit = false;
    bool dirty_bit = false;
    bool privacy_bit = false;
    const char *filename = nullptr;
    unsigned int block = 0;
    bool swap_backed = false;
};

struct pager_page_table_t {
    pager_page_t ptes[VM_ARENA_SIZE/VM_PAGESIZE];
};

struct process{
    pager_page_table_t *page_table;
    page_table_t *infrastructure_page_table;
    pid_t process_id;
    uintptr_t arena_start = uintptr_t(VM_ARENA_BASEADDR);
    uintptr_t arena_valid_end = uintptr_t(VM_ARENA_BASEADDR);
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
    for(size_t i = 1; i < physmem_size; i++){
        phys_index.push_back(i);
    }
    swap_size = swap_blocks;
    for(size_t i = 0; i < swap_size; i++){
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
        temp_process->infrastructure_page_table = new page_table_t;
        temp_process->page_table = new pager_page_table_t;
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

            /*
                Some kind of check for dirty and privacy bit
                should call file_write to write the information back to the correct file
            */

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

            int first_invalid_page = arena_valid_page_size();
            pager_page_t* temp_page = new pager_page_t;
            //page_table_base_register->ptes[first_invalid_page] = new page_table_entry_t;
            temp_page->base = &(page_table_base_register->ptes[first_invalid_page]);

            temp_page->swap_backed = true;
            temp_page->filename = filename;
            temp_page->block = swap_index.front();
            swap_index.pop_front();

            enable_page_protection(temp_page);
            temp_page->dirty_bit = false;
            temp_page->resident_bit = false;
            temp_page->reference_bit = false;
            temp_page->privacy_bit = false;

            processes[curr_pid]->arena_valid_end += VM_PAGESIZE;
            processes[curr_pid]->page_table->ptes[first_invalid_page] = *temp_page;
            processes[curr_pid]->infrastructure_page_table->ptes[first_invalid_page] = *temp_page->base;

            swap_counter++;
            return  (void *) (processes[curr_pid]->arena_valid_end - VM_PAGESIZE);
        }
        else{
            return nullptr;
        }
    }
    return nullptr;
}

int vm_fault(const void* addr, bool write_flag){
    if((uintptr_t) addr - processes[curr_pid]->arena_start >= (processes[curr_pid]->arena_valid_end)){ //(Address - Start of Addresss Space) >= End of Valid Address space is an illegal call
        return -1;
    }

    pager_page_t* curr_page = &processes[curr_pid]->page_table->ptes[ ((uintptr_t) addr - processes[curr_pid]->arena_start) / VM_PAGESIZE]; //Page address is trying to access
    curr_page->reference_bit = true;

    if(write_flag){ //Trying to write to page
        if(curr_page->resident_bit == false){
            clock_insert(curr_page);
            curr_page->base->ppage = phys_index.front();
            phys_index.pop_front();
            phys_counter++;

            if(curr_page->swap_backed == true && curr_page->privacy_bit == false){
                ((char *)vm_physmem)[curr_page->base->ppage] = ((char *) vm_physmem)[buff_index];
                curr_page->privacy_bit = true;
            }
            else if(curr_page->swap_backed == true && curr_page->privacy_bit == true){
                int result = file_read(curr_page->filename, curr_page->block, &((char *)vm_physmem)[curr_page->base->ppage]);
                if(result == -1){
                    //file_read was a failure
                    assert(false);
                }
            }
            else{
                //File backed write
            }
        }

        curr_page->base->read_enable = true;
        curr_page->base->write_enable = true;
        curr_page->dirty_bit = true;
    }
    else{ //Trying to read page
        if(curr_page->resident_bit == false){ //Page is not already resident
            clock_insert(curr_page); //Bring page into residency (within clock)
            curr_page->base->ppage = phys_index.front();
            phys_index.pop_front();
            phys_counter++;
            
            if(curr_page->swap_backed == true && curr_page->privacy_bit == false){ //Swapped back page that original has not been written to
                ((char *)vm_physmem)[curr_page->base->ppage] = ((char *) vm_physmem)[buff_index];
                curr_page->dirty_bit = false;
            }
            else if (curr_page->swap_backed == true && curr_page->privacy_bit == true){ //Swapped back page that original has had some write to (you read what was newly written to it)
                int result = file_read(curr_page->filename, curr_page->block, &((char *)vm_physmem)[curr_page->base->ppage]);
                if(result == -1){
                    //file_read was a failure
                    assert(false);
                }
                curr_page->dirty_bit = false;
            }
            else{
                //File backed read
            }
        }
        
        // if(curr_page->dirty_bit == true){
        //     curr_page->base->write_enable = true;
        // }
        // else{
        //     curr_page->base->write_enable = false;
        // }
        assert(curr_page->base->write_enable == false);
        curr_page->base->read_enable = true;
    }

    return 0;
}