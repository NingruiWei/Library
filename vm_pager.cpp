#include "vm_pager.h"
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
    bool pinned = false;
};

struct pager_page_table_t {
    //vector<pager_page_t*> entries(VM_ARENA_SIZE/VM_PAGESIZE);
    pager_page_t* entries[VM_ARENA_SIZE/VM_PAGESIZE];
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
    pager_page_t* zero = new pager_page_t;
    zero->resident_bit = true;
    zero->pinned = true;
    clocker.push_back(zero);
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
    //cout << "INSIDE SWITCH" << endl;
    page_table_base_register = processes[pid]->infrastructure_page_table;
    curr_pid = pid;
}

void destroy_clock_page(pager_page_t* destroy_page){
    while(true){
        if(clocker.front() != destroy_page){
            clocker.push_back(clocker.front());
            clocker.pop_front();
        }
        else{
            clocker.pop_front(); //remove page that is about to be deleted from clock
            return;
        }
    }
}

void vm_destroy(){
    for(size_t i = processes[curr_pid]->arena_start; i < processes[curr_pid]->arena_valid_end; i += VM_PAGESIZE){
        pager_page_t* curr_page = processes[curr_pid]->page_table->entries[(i-processes[curr_pid]->arena_start)/VM_PAGESIZE];
        if(curr_page->resident_bit == true){ //If currently resident, remove from physmem and clock
            phys_index.push_back(curr_page->base->ppage);
            phys_counter--;
            destroy_clock_page(curr_page);
        }
        if(curr_page->swap_backed == true){ //Free up swap block for something new in swap space
            swap_index.push_back(curr_page->block);
            swap_counter--;
        }
        delete curr_page; //Delete the page (since it was dynamically allocated)
    }
    page_table_base_register = nullptr; //No current page table since we're deleting the one we're currently running
    delete processes[curr_pid]->page_table; //Not sure why, but this line doesn't work. We're deleting everything else that's dynamically allocated, but this line causes issues
    delete processes[curr_pid]->infrastructure_page_table; //Delete dynamically allocated memebers of process and dynamically allocated process

    // can only call delete[] if you call new[]

    delete processes[curr_pid];
    processes.erase(curr_pid); //Remove process from map
}

int arena_valid_page_size(){
    int arena_size = processes[curr_pid]->arena_valid_end - processes[curr_pid]->arena_start;
    return arena_size / VM_PAGESIZE;
}

void enable_page_protection(pager_page_t* disable_page){
    disable_page->base->ppage = 0;
    disable_page->base->read_enable = 0;
    disable_page->base->write_enable = 0;
}

void evict_page(pager_page_t* reset_page){
    enable_page_protection(reset_page);
    reset_page->resident_bit = false;
    reset_page->dirty_bit = false;
    reset_page->reference_bit = false;
    reset_page->privacy_bit = false;
}

void evict(){
    while(1){
        if(clocker.front()->reference_bit == true || clocker.front()->pinned == true){ //Clock hand pointing at "1"
            clocker.front()->reference_bit = false;
            clocker.push_back(clocker.front());
            clocker.pop_front();
        }
        else{                                       //Clock hand pointing at "0"
            enable_page_protection(clocker.front());
            phys_index.push_back(clocker.front()->base->ppage);
            phys_counter--;

            /*
                Some kind of check for dirty and privacy bit
                should call file_write to write the information back to the correct file
            */
            if(clocker.front()->dirty_bit == true && clocker.front()->privacy_bit == true){
               file_write(clocker.front()->filename, clocker.front()->block, &((char *)vm_physmem)[clocker.front()->base->ppage]);
            }
            evict_page(clocker.front());
            
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

bool invalid_filename_address(const char *filename) {
    return processes[curr_pid]->arena_start > (uintptr_t)filename || (uintptr_t)filename >= processes[curr_pid]->arena_valid_end;
}

void *vm_map(const char *filename, unsigned int block){
    // cout << "hi map here" << endl;
    if(processes[curr_pid]->arena_valid_end == uintptr_t(VM_ARENA_BASEADDR) + VM_ARENA_SIZE){
        //arena is full
        return nullptr;
    }
    if(filename){ // file-backed
        unsigned int filename_offset = 0;
        string filename_str;
        while (1) {
            const char * id = filename + filename_offset;
            if (invalid_filename_address(id)) {
                return nullptr;
            }
            vm_fault(id, false);
            unsigned int ppage = page_table_base_register->ptes[((uintptr_t)id - processes[curr_pid]->arena_start)/VM_PAGESIZE].ppage;
            ppage = (ppage * VM_PAGESIZE) + (filename_offset % VM_PAGESIZE);
            filename_str += ((char*) vm_physmem)[ppage];
            filename_offset += 1;
            if(((char*) vm_physmem)[ppage] == '\0'){
                break;
            }
        }
        char * copy_str = new char[filename_offset];
        filename_str.copy(copy_str, filename_offset);

        // if(fileback_map.find(copy_string+block) == fileback_map.end()){
        //     //create new page to add to fileback map
        // }
        // return fileback_map[copy_string+block];

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
            processes[curr_pid]->page_table->entries[first_invalid_page] = temp_page;
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
   
    pager_page_t* curr_page = processes[curr_pid]->page_table->entries[ ((uintptr_t) addr - processes[curr_pid]->arena_start) / VM_PAGESIZE]; //Page address is trying to access
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
                curr_page->privacy_bit = true;
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
                curr_page->privacy_bit = true;
            }
        }
        
        // if(curr_page->dirty_bit == true){
        //     curr_page->base->write_enable = true;
        // }
        // else{
        //     curr_page->base->write_enable = false;
        // }
        curr_page->base->read_enable = true;
    }

    return 0;
}