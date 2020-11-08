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
    pager_page_t(){}
    ~pager_page_t(){}
    vector<pair<int, page_table_entry_t*>> page_table_entries;
    //page_table_entry_t* base = nullptr;
    bool reference_bit = false;
    bool resident_bit = false;
    bool dirty_bit = false;
    bool privacy_bit = false;
    const char *filename = nullptr;
    unsigned int block = 0;
    bool swap_backed = false;
    bool pinned = false;
    bool in_physmem = false;
};

struct pager_page_table_t {
    pager_page_table_t(){}
    ~pager_page_table_t(){}
    //vector<pager_page_t*> entries(VM_ARENA_SIZE/VM_PAGESIZE);
    pager_page_t* entries[VM_ARENA_SIZE/VM_PAGESIZE];
};

struct process{
    process(){
        page_table = new pager_page_table_t;
        infrastructure_page_table = new page_table_t;
        for (size_t i = 0; i < VM_ARENA_SIZE/VM_PAGESIZE; i++) {
            infrastructure_page_table->ptes[i].ppage = 0;
            infrastructure_page_table->ptes[i].read_enable = 0;
            infrastructure_page_table->ptes[i].write_enable = 0;
        }
    }
    ~process(){
        page_table->~pager_page_table_t(); //Not sure why, but this line doesn't work. We're deleting everything else that's dynamically allocated, but this line causes issues
        delete infrastructure_page_table;
    }
    pager_page_table_t *page_table;
    page_table_t *infrastructure_page_table;
    pid_t process_id;
    uintptr_t arena_start = uintptr_t(VM_ARENA_BASEADDR);
    uintptr_t arena_valid_end = uintptr_t(VM_ARENA_BASEADDR);
    int num_swap_pages = 0;
};

pager_page_t* zero;

deque<pager_page_t*> clocker;
deque<unsigned int> phys_index;
deque<unsigned int> swap_index;
deque<unsigned int> reserved_swap_index; //swap indices that are currently reserved for copy-on-write swap backed pages
unordered_map<pid_t, process*> processes;
unordered_map<string, pager_page_t*> filebacked_map;
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
    //char *buff = new char[VM_PAGESIZE];
    memset (vm_physmem, 0, VM_PAGESIZE);
    //((char*)vm_physmem)[buff_index] = *buff;
    zero = new pager_page_t;
    zero->resident_bit = true;
    zero->pinned = true;
    zero->reference_bit = true;
    page_table_entry_t * zero_entry = new page_table_entry_t;
    zero_entry->ppage = 0;
    zero->page_table_entries.push_back(make_pair(curr_pid, zero_entry));
    clocker.push_back(zero);
}

int vm_create(pid_t parent_pid, pid_t child_pid){
    if(processes.find(parent_pid) != processes.end()){ // 498 part
        process* parent_process = processes[parent_pid];
        process* child_process = new process;
        child_process->process_id = child_pid;
        //child_process->infrastructure_page_table = new page_table_t;
        //child_process->page_table = new pager_page_table_t;

        for(size_t i = 0; i < sizeof(parent_process->page_table->entries) / sizeof(parent_process->page_table->entries)[0]; i++){
            pager_page_t* curr_parent_entry = parent_process->page_table->entries[i];
            if(!curr_parent_entry){
                break;
            }
            if(curr_parent_entry->swap_backed == true){ //Swapbacked page (THIS IS WHERE THE WORKED IS NEEDED)
                if(swap_counter + 1 >= swap_size){ //No space for eager swap reservation
                    return -1;
                }
                assert(!swap_index.empty()); //Double check that we still have swap space left

                child_process->num_swap_pages++;
                reserved_swap_index.push_back(swap_index.front()); //reserve swap index for potential copy-on-write
                swap_index.pop_front();
                swap_counter++;
            }

            page_table_entry_t* temp_page = &child_process->infrastructure_page_table->ptes[i];
            //page_table_entry_t* temp_page_parent = &parent_process->infrastructure_page_table->ptes[i];
            if(!curr_parent_entry->page_table_entries.empty()){
                temp_page->read_enable = curr_parent_entry->page_table_entries.front().second->read_enable;
                temp_page->write_enable = curr_parent_entry->page_table_entries.front().second->write_enable;
                temp_page->ppage = curr_parent_entry->page_table_entries.front().second->ppage;
                //temp_page_parent->ppage = curr_parent_entry->page_table_entries.front().second->ppage;
            }
            if(curr_parent_entry->pinned){
                temp_page->ppage = 0;
                //temp_page_parent->ppage = 0;
                temp_page->read_enable = true;
                //temp_page_parent->read_enable = true;
                temp_page->write_enable = false;
                //temp_page_parent->write_enable = false;
            }

            curr_parent_entry->page_table_entries.push_back(make_pair(child_pid, temp_page));
            child_process->page_table->entries[i] = curr_parent_entry;

            if (curr_parent_entry->swap_backed == true) {
                for(pair<pid_t, page_table_entry_t*> entry : curr_parent_entry->page_table_entries){
                    entry.second->write_enable = false; //Guarantee write is false so that we can copy on write if necessary
                }
            }
        }

        child_process->arena_valid_end = parent_process->arena_valid_end;
        assert(child_process->num_swap_pages == parent_process->num_swap_pages); //Parent and child should have the same number of swap pages

        processes[child_pid] = child_process;

        return 0;
    }
    else{
        process* temp_process = new process;
        temp_process->process_id = child_pid;
        //temp_process->infrastructure_page_table = new page_table_t;
        //temp_process->page_table = new pager_page_table_t;
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
    page_table_base_register = nullptr; //No current page table since we're deleting the one we're currently running
    for(size_t i = processes[curr_pid]->arena_start; i < processes[curr_pid]->arena_valid_end; i += VM_PAGESIZE){
        pager_page_t* curr_page = processes[curr_pid]->page_table->entries[(i-processes[curr_pid]->arena_start)/VM_PAGESIZE];
        processes[curr_pid]->page_table->entries[(i-processes[curr_pid]->arena_start)/VM_PAGESIZE] = nullptr;
        unsigned int temp_ppage = curr_page->page_table_entries.front().second->ppage;
        if(!curr_page->swap_backed){ //File backed only removes if the pid matches
            //THIS ACTUALLY WROTE TO THE FILE AND DIDN'T SEEM SAFE. Commenting for now, I don't know if this is necessary or not?
            // if(curr_page->page_table_entries.size() == 1 && curr_page->dirty_bit == true && curr_page->privacy_bit == true){ //Last thing pointing to that file and block, write back its content if dirty
            //     file_write(curr_page->filename, curr_page->block, &((char *)vm_physmem)[VM_PAGESIZE * curr_page->page_table_entries.front().second->ppage]);
            // }

            curr_page->page_table_entries.erase(remove_if(curr_page->page_table_entries.begin(), curr_page->page_table_entries.end(), [curr_page](pair <int, page_table_entry_t*> entry){
                return entry.first == curr_pid;
            }));
        }
        else{ //Swap backed should only ever have 1 in its page table entries vector
            //assert(curr_page->page_table_entries.size() == 1); //No longer true, since a copy-on-write swap backed page would be in its page_table_entries vector

            /*
                NEED SOME WAY TO FREE UP COPY ON WRITE SWAP BACKED PAGES in the case they were never written to to separate them
                This is a TEMPORARY solution, but we may need to rethink it
            */
           if(curr_page->page_table_entries.size() > 1){ //Swap backed page with something that had a reserved copy-on-write swap block, but never wrote (so did not copy)
                curr_page->page_table_entries.erase(remove_if(curr_page->page_table_entries.begin(), curr_page->page_table_entries.end(), [curr_page](pair <int, page_table_entry_t*> entry){
                    return entry.first == curr_pid;
                }));
                swap_index.push_back(reserved_swap_index.front());
                reserved_swap_index.pop_front();
                if (curr_page->page_table_entries.size() == 1 && curr_page->dirty_bit == true && curr_page->reference_bit == true) {
                    curr_page->page_table_entries.front().second->write_enable = true;
                }
            }
            else{ //Swap backed with nothing reserved for copy-on-write
                curr_page->page_table_entries.pop_back();
            }
        }

        if(curr_page->page_table_entries.size() == 0){
            if(curr_page->resident_bit == true){
                phys_index.push_back(temp_ppage);
                phys_counter--;
                destroy_clock_page(curr_page);
            }
            if(curr_page->swap_backed == true){
                swap_index.push_back(curr_page->block);
                swap_counter--;
            }
            else{
                string to_erase(curr_page->filename);
                to_erase += "-" + to_string(curr_page->block);
                filebacked_map.erase(to_erase);
                delete curr_page->filename;
            }
            curr_page->~pager_page_t();
        }
    }

    
    //processes[curr_pid]->page_table->~pager_page_table_t(); //Not sure why, but this line doesn't work. We're deleting everything else that's dynamically allocated, but this line causes issues
    //delete processes[curr_pid]->infrastructure_page_table; //Delete dynamically allocated memebers of process and dynamically allocated process

    // can only call delete[] if you call new[]
    sort(phys_index.begin(), phys_index.end());
    sort(swap_index.begin(), swap_index.end());
    processes[curr_pid]->~process();
    processes.erase(curr_pid); //Remove process from map
}

int arena_valid_page_size(){
    int arena_size = processes[curr_pid]->arena_valid_end - processes[curr_pid]->arena_start;
    return arena_size / VM_PAGESIZE;
}

void enable_page_protection(page_table_entry_t* disable_page){
    disable_page->ppage = 0;
    disable_page->read_enable = 0;
    disable_page->write_enable = 0;
}

void evict_page(pager_page_t* reset_page){
    for(pair<int, page_table_entry_t*> entry : reset_page->page_table_entries){
        enable_page_protection(entry.second);
    }
    reset_page->resident_bit = false;
    reset_page->dirty_bit = false;
    reset_page->reference_bit = false;
    //reset_page->privacy_bit = false; //Privacy bit will tell us if swap-backed has ever been written back to before, meaning we can't just access a zero page for it
    reset_page->in_physmem = false;
}

void evict(){
    while(1){ 
        if(clocker.front()->reference_bit == true || clocker.front()->pinned == true){ //Clock hand pointing at page with reference of "1"
            clocker.front()->reference_bit = false;
            clocker.front()->page_table_entries.front().second->read_enable = false; //Lose privelages when you lose your reference
            clocker.front()->page_table_entries.front().second->write_enable = false;
            
            for(pair<int, page_table_entry_t*> entry : clocker.front()->page_table_entries){ //Echo loss of reference status to all page table entries managed together (for file-backed)
                entry.second->ppage = clocker.front()->page_table_entries.front().second->ppage;
                entry.second->read_enable = clocker.front()->page_table_entries.front().second->read_enable;
                entry.second->write_enable = clocker.front()->page_table_entries.front().second->write_enable;
            }

            clocker.push_back(clocker.front());
            clocker.pop_front();
        }
        else{  //Clock hand pointing at page with reference of "0"
            if(clocker.front()->dirty_bit == true && (clocker.front()->filename != nullptr || clocker.front()->privacy_bit == true)){
               file_write(clocker.front()->filename, clocker.front()->block, &((char *)vm_physmem)[VM_PAGESIZE * clocker.front()->page_table_entries.front().second->ppage]);
            }
            phys_index.push_back(clocker.front()->page_table_entries.front().second->ppage);
            phys_counter--;
            evict_page(clocker.front());

            for(pair<int, page_table_entry_t*> entry : clocker.front()->page_table_entries){ //Echo evicition to all page table entries managed together (mostly for file-backed that have the same filename and block)
                entry.second->ppage = clocker.front()->page_table_entries.front().second->ppage;
                entry.second->read_enable = clocker.front()->page_table_entries.front().second->read_enable;
                entry.second->write_enable = clocker.front()->page_table_entries.front().second->write_enable;
            }
            
            clocker.pop_front();
            return;
        }
    }
}

void clock_insert(pager_page_t* insert_page){
    if(clocker.size() >= physmem_size){
        evict();
    }

    insert_page->reference_bit = true;
    insert_page->resident_bit = true;
    // we might need to do the memcpy stuff here after bringing the page into resident (bring into physical mem)
    insert_page->page_table_entries.front().second->ppage = phys_index.front();
    phys_index.pop_front();
    phys_counter++;

    for(pair<int, page_table_entry_t*> entry : insert_page->page_table_entries){
        entry.second->ppage = insert_page->page_table_entries.front().second->ppage;
        entry.second->read_enable = insert_page->page_table_entries.front().second->read_enable;
        entry.second->write_enable = insert_page->page_table_entries.front().second->write_enable;
    }

    clocker.push_back(insert_page); //Push back is inserting the element and then moving the hand one place forward
}

bool invalid_filename_address(const char *filename) {
    return processes[curr_pid]->arena_start > (uintptr_t)filename || (uintptr_t)filename >= processes[curr_pid]->arena_valid_end;
}

pager_page_t* new_pager_page(const char* filename, unsigned int block){
    pager_page_t* return_pager_page = new pager_page_t;

    return_pager_page->filename = filename;
    return_pager_page->block = block;

    return_pager_page->resident_bit = false;
    return_pager_page->dirty_bit = false;
    return_pager_page->reference_bit = false;
    return_pager_page->pinned = false;
    return_pager_page->in_physmem = false;

    return return_pager_page;
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
            unsigned int ppage_offset = ((uintptr_t)id - processes[curr_pid]->arena_start) % VM_PAGESIZE;
            filename_str += ((char*) vm_physmem)[((VM_PAGESIZE * ppage) + ppage_offset)];
            filename_offset += 1;
            if(((char*) vm_physmem)[((VM_PAGESIZE * ppage) + ppage_offset)] == '\0'){
                break;
            }
        }
        char * copy_str = new char[filename_offset];
        filename_str.copy(copy_str, filename_offset);
        string addition(copy_str);
        int first_invalid_page = arena_valid_page_size();
        addition += '-' + to_string(block);
        if(filebacked_map.find(addition) == filebacked_map.end()){
            //create new page to add to fileback map

            pager_page_t* temp_page = new_pager_page(copy_str, block);

            filebacked_map[addition] = temp_page;
        }

        page_table_entry_t* temp_page_table_entry = &(page_table_base_register->ptes[first_invalid_page]);
        if(!filebacked_map[addition]->page_table_entries.empty()){
            temp_page_table_entry->ppage = filebacked_map[addition]->page_table_entries.front().second->ppage;
            temp_page_table_entry->read_enable = filebacked_map[addition]->page_table_entries.front().second->read_enable;
            temp_page_table_entry->write_enable = filebacked_map[addition]->page_table_entries.front().second->write_enable;
        }
        else{
            enable_page_protection(temp_page_table_entry);
        }
        filebacked_map[addition]->page_table_entries.push_back(make_pair(curr_pid, temp_page_table_entry));

        //Add page, from filebacked_map, to current process page table
        processes[curr_pid]->arena_valid_end += VM_PAGESIZE;
        processes[curr_pid]->page_table->entries[first_invalid_page] = filebacked_map[addition];
        processes[curr_pid]->infrastructure_page_table->ptes[first_invalid_page] = *temp_page_table_entry;

        //processes[curr_pid]->page_table->entries[ ((uintptr_t) addr - processes[curr_pid]->arena_start) / VM_PAGESIZE]
        //clock_insert(processes[curr_pid]->page_table->entries[first_invalid_page]); // adding it to 
        return  (void *) (processes[curr_pid]->arena_valid_end - VM_PAGESIZE);

    }
    else{ // swap-backed
        if(swap_counter < swap_size){ //Eager swap reservation check
            assert(!swap_index.empty()); //Double checks that there is still swap space available

            int first_invalid_page = arena_valid_page_size();
            pager_page_t* temp_page = new_pager_page(filename, block);

            page_table_entry_t* temp_page_table_entry = &(page_table_base_register->ptes[first_invalid_page]);
            temp_page_table_entry->ppage = 0; //see each byte of a newly mapped swap-backed virtual page as initialized with the value 0.
            temp_page_table_entry->read_enable = true; //So the application can see that they're all zeroes
            temp_page_table_entry->write_enable = false; //So that we can defer assigning it a real virtual page of all zeroes until it's written to

            temp_page->page_table_entries.push_back(make_pair(curr_pid, temp_page_table_entry));

            temp_page->swap_backed = true;
            temp_page->filename = filename;
            temp_page->block = swap_index.front();
            swap_index.pop_front();

            processes[curr_pid]->arena_valid_end += VM_PAGESIZE;
            processes[curr_pid]->page_table->entries[first_invalid_page] = temp_page;
            processes[curr_pid]->infrastructure_page_table->ptes[first_invalid_page] = *temp_page_table_entry;
            processes[curr_pid]->num_swap_pages++;

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

    if(curr_page->swap_backed && write_flag == true && curr_page->page_table_entries.size() > 1){ //Swap backed page that has a copy-on-write, so we must copy before writing
        unsigned int copy_on_ppage = curr_page->page_table_entries.front().second->ppage;
        page_table_entry_t* copy_on_page;
        for(size_t i = 0; i < curr_page->page_table_entries.size(); i++){
            if(curr_page->page_table_entries[i].first == curr_pid && curr_page->page_table_entries[i].second == &page_table_base_register->ptes[((uintptr_t) addr - processes[curr_pid]->arena_start) / VM_PAGESIZE]){
                copy_on_page = curr_page->page_table_entries[i].second;
                curr_page->page_table_entries.erase(curr_page->page_table_entries.begin() + i);
                break;
            }
        }

        vm_fault(addr, false); //Hint: Writing to a virtual page that is being shared via copy-on-write should have the same effect on the system as reading it, then writing it.
        pager_page_t* copy_on_write_page = new pager_page_t;
        copy_on_write_page->block = reserved_swap_index.front();
        copy_on_write_page->swap_backed = true;
        copy_on_write_page->filename = nullptr;
        copy_on_write_page->in_physmem = curr_page->in_physmem;
        copy_on_write_page->privacy_bit = curr_page->in_physmem;
        copy_on_write_page->page_table_entries.push_back(make_pair(curr_pid, copy_on_page));
        reserved_swap_index.pop_front();
        processes[curr_pid]->page_table->entries[((uintptr_t) addr - processes[curr_pid]->arena_start) / VM_PAGESIZE] = copy_on_write_page;
        //vm_fault(addr, false); //Hint: Writing to a virtual page that is being shared via copy-on-write should have the same effect on the system as reading it, then writing it.
        char* copy_on_page_buffer = new char[VM_PAGESIZE];
        memcpy(copy_on_page_buffer, &((char*)vm_physmem)[VM_PAGESIZE * copy_on_ppage], VM_PAGESIZE);
        //copy_on_write_page->page_table_entries.push_back(make_pair(curr_pid, &page_table_base_register->ptes[((uintptr_t) addr - processes[curr_pid]->arena_start) / VM_PAGESIZE]));
        curr_page = copy_on_write_page;
    }

    if(curr_page->resident_bit == false){
        clock_insert(curr_page);
    }
    
    if(curr_page->swap_backed == true && curr_page->privacy_bit == false && !curr_page->in_physmem){
        memcpy(&((char *)vm_physmem)[VM_PAGESIZE * (curr_page->page_table_entries.front().second->ppage)], &((char *) vm_physmem)[VM_PAGESIZE * buff_index], VM_PAGESIZE);
        //file_read(curr_page->filename, curr_page->block, &((char *)vm_physmem)[VM_PAGESIZE * (curr_page->page_table_entries.front().second->ppage)]);
        //((char *)vm_physmem)[VM_PAGESIZE * (curr_page->page_table_entries.front().second->ppage)] = ((char *) vm_physmem)[VM_PAGESIZE * buff_index];
        if(write_flag == true){
            curr_page->privacy_bit = true;
        }
    }
    else if(curr_page->swap_backed == true && curr_page->privacy_bit == true && !curr_page->in_physmem){
        int result = file_read(curr_page->filename, curr_page->block, &((char *)vm_physmem)[VM_PAGESIZE * (curr_page->page_table_entries.front().second->ppage)]);
        if(result == -1){
            //file_read was a failure
            //assert(false);
            return -1;
        }
    }
    else if(!curr_page->in_physmem){
        //File backed
        if(write_flag == true){
            curr_page->privacy_bit = true;
        }
        int result = file_read(curr_page->filename, curr_page->block, &((char *)vm_physmem)[VM_PAGESIZE * (curr_page->page_table_entries.front().second->ppage)]);
        if(result == -1){
            //file_read was a failure
            //assert(false);
            return -1;
        } 
    }

    curr_page->in_physmem = true;
    if(write_flag){
        curr_page->page_table_entries.front().second->read_enable = true;
        curr_page->page_table_entries.front().second->write_enable = true;
        curr_page->dirty_bit = true;
    }
    else{
        if(curr_page->page_table_entries.front().second->write_enable == true || curr_page->in_physmem == true){
            if(((curr_page->swap_backed == true && !(curr_page->page_table_entries.size() > 1)) || curr_page->swap_backed == false) && curr_page->dirty_bit == true){
                curr_page->page_table_entries.front().second->write_enable = true;
                curr_page->dirty_bit = true;
            }
        }
        else{
            curr_page->page_table_entries.front().second->write_enable = false;
            curr_page->dirty_bit = false;
        }
        curr_page->page_table_entries.front().second->read_enable = true;
    }

    for(pair<int, page_table_entry_t*> entry : curr_page->page_table_entries){
        entry.second->ppage = curr_page->page_table_entries.front().second->ppage;
        entry.second->read_enable = curr_page->page_table_entries.front().second->read_enable;
        entry.second->write_enable = curr_page->page_table_entries.front().second->write_enable;
    }

    return 0;
}