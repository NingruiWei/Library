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
    ~pager_page_t(){ 
        delete filename;
    }
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
    bool in_clock = false;
    bool is_zero = false;
    unsigned int physical_page;

    void echo_to_ptes(){
        for(pair<int, page_table_entry_t*> entry : page_table_entries){
            entry.second->ppage = page_table_entries.front().second->ppage;
            entry.second->read_enable = page_table_entries.front().second->read_enable;
            entry.second->write_enable = page_table_entries.front().second->write_enable;
        }
    }

    void initalize_new_entry(){
        if(resident_bit){
            page_table_entries.front().second->ppage = physical_page;
        }
        else{
            page_table_entries.front().second->ppage = 0;
        }

        page_table_entries.front().second->read_enable = reference_bit && resident_bit;
        page_table_entries.front().second->write_enable = reference_bit && dirty_bit;

        if(swap_backed == true && page_table_entries.size() > 1){
            page_table_entries.front().second->write_enable = false;
            echo_to_ptes();
        }
    }
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
            page_table->entries[i] = nullptr;
        }
    }
    ~process(){
        delete page_table; //Not sure why, but this line doesn't work. We're deleting everything else that's dynamically allocated, but this line causes issues
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
char *copy_on_page_buffer;

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
    zero->is_zero = true;
    page_table_entry_t * zero_entry = new page_table_entry_t;
    zero_entry->ppage = 0;
    zero->page_table_entries.push_back(make_pair(curr_pid, zero_entry));
    clocker.push_back(zero);
}

bool copy_parent_process(process* parent_process, process* child_process){
    //Eager swap reservation algorithm
    size_t eager = 0;
    for (size_t i = 0; i < (parent_process->arena_valid_end - parent_process->arena_start) / VM_PAGESIZE; i++) {
        if (parent_process->page_table->entries[i]->swap_backed == true) {
            eager++;
        }
    }
    if (swap_counter + eager > swap_size) {
        return false;
    }

    for(size_t i = 0; i < eager; i++){
        swap_counter++;
        reserved_swap_index.push_back(swap_index.front());
        swap_index.pop_front();
    }


    for (size_t i = 0; i < (parent_process->arena_valid_end - parent_process->arena_start) / VM_PAGESIZE; i++) {
        pager_page_t* curr_parent_entry = parent_process->page_table->entries[i];
        page_table_entry_t* curr_child_entry = &child_process->infrastructure_page_table->ptes[i];

        if(!curr_parent_entry->page_table_entries.empty()){
            curr_child_entry->ppage = curr_parent_entry->page_table_entries.front().second->ppage;
            curr_child_entry->read_enable = curr_parent_entry->page_table_entries.front().second->read_enable;
            curr_child_entry->write_enable = curr_parent_entry->page_table_entries.front().second->write_enable;
        }

        if(curr_parent_entry->pinned){
            curr_child_entry->ppage = 0;
            curr_child_entry->read_enable = true;
            curr_child_entry->write_enable = false;
        }

        curr_parent_entry->page_table_entries.push_back(make_pair(child_process->process_id, curr_child_entry));
        curr_parent_entry->initalize_new_entry();
        // if(curr_parent_entry->resident_bit){
        //     curr_child_entry.ppage = curr_parent_entry->physical_page;
        // }
        // else{
        //     curr_child_entry.ppage = 0;
        // }

        // curr_child_entry.read_enable = curr_parent_entry->reference_bit && curr_parent_entry->resident_bit;
        // curr_child_entry.write_enable = curr_parent_entry->reference_bit && curr_parent_entry->dirty_bit;

        // if(curr_parent_entry->swap_backed == true && curr_parent_entry->page_table_entries.size() > 1){
        //     curr_parent_entry->page_table_entries.front().second->write_enable = false;
        //     curr_parent_entry->echo_to_ptes();
        // }

        child_process->page_table->entries[i] = curr_parent_entry;
    }

    child_process->arena_valid_end = parent_process->arena_valid_end;
    return true;
}


int vm_create(pid_t parent_pid, pid_t child_pid){
    if(processes.find(parent_pid) != processes.end()){ // 498 part
        process* parent_process = processes[parent_pid];

        process* child_process = new process;
        child_process->process_id = child_pid;

        if(parent_process->arena_start != parent_process->arena_valid_end && !copy_parent_process(parent_process, child_process)){
            //Unable to copy all of parent's pages due to eager swap reservation
            delete child_process;
            return -1;
        }
        else{
            processes[child_pid] = child_process;
            return 0;
        }
    }
    else{
        process* temp_process = new process;
        temp_process->process_id = child_pid;
        processes[child_pid] = temp_process;
        return 0;
    }
}

void vm_switch(pid_t pid){
    page_table_base_register = processes[pid]->infrastructure_page_table;
    curr_pid = pid;
}

void destroy_clock_page(pager_page_t* destroy_page){
    clocker.erase(remove(clocker.begin(), clocker.end(), destroy_page));
}


void vm_destroy(){
    page_table_base_register = nullptr; //No current page table since we're deleting the one we're currently running

    for (size_t i = 0; i < (processes[curr_pid]->arena_valid_end - processes[curr_pid]->arena_start) / VM_PAGESIZE; i++) {
        pager_page_t* curr_page = processes[curr_pid]->page_table->entries[i];
        bool curr_page_was_shared = curr_page->page_table_entries.size() > 1;

        curr_page->page_table_entries.erase(remove_if(curr_page->page_table_entries.begin(), curr_page->page_table_entries.end(), [curr_page](pair <int, page_table_entry_t*> entry){
            return entry.first == curr_pid;
        }));

        if(curr_page_was_shared && !(curr_page->page_table_entries.size() > 1)){
            //for(pair<int, page_table_entry_t*> entry : curr_page->page_table_entries){
                curr_page->initalize_new_entry();
                // if(curr_page->resident_bit){
                //     entry.second->ppage = curr_page->physical_page;
                // }
                // else{
                //     entry.second->ppage = 0;
                // }

                // entry.second->ppage = curr_page->reference_bit && curr_page->resident_bit;
                // entry.second->ppage = curr_page->reference_bit && curr_page->dirty_bit;

                // if(curr_page->swap_backed == true && curr_page->page_table_entries.size() > 1){
                //     curr_page->page_table_entries.front().second->write_enable = false;
                //     curr_page->echo_to_ptes();
                // }
            //}
        }

        if(curr_page->swap_backed == true){
            if(curr_page->pinned == false && curr_page->page_table_entries.size() == 0){

                swap_index.push_back(curr_page->block);
                if(curr_page->resident_bit == true){
                    phys_index.push_back(curr_page->physical_page);
                    phys_counter++;
                    destroy_clock_page(curr_page);
                }

            }

            if (curr_page_was_shared && curr_page->page_table_entries.size() > 0){
                swap_index.push_back(reserved_swap_index.front());
                reserved_swap_index.pop_front();
            }
            swap_counter--; 

            if(curr_page->page_table_entries.size() == 0){
                delete curr_page; //Currently only calling page destructor on EMPTY swap back pages. If a swap back page has other things in page_table_entries, might still be accessed in the future.
            }
        }
    }

    delete processes[curr_pid];
    processes.erase(curr_pid); //Remove process from ma
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

bool invalid_filename_address(const char *filename) {
    return processes[curr_pid]->arena_start > (uintptr_t)filename || (uintptr_t)filename >= processes[curr_pid]->arena_valid_end;
}

pager_page_t* new_pager_page(const char* filename, unsigned int block){
    pager_page_t* return_pager_page = new pager_page_t;

    return_pager_page->filename = filename;
    return_pager_page->block = block;

    return_pager_page->reference_bit = false;
    return_pager_page->resident_bit = false;
    return_pager_page->dirty_bit = false;
    return_pager_page->privacy_bit = false;
    return_pager_page->pinned = false;
    return_pager_page->is_zero = false;
    return_pager_page->in_physmem = false;
    return_pager_page->in_clock = false;
    return_pager_page->physical_page = 0;

    return return_pager_page;
}

void *vm_map(const char *filename, unsigned int block){

    if(!(processes[curr_pid]->arena_valid_end < ((uintptr_t)VM_ARENA_BASEADDR + (uintptr_t)VM_ARENA_SIZE))){
        //Arena is full, cannot allocate another page
        return nullptr;
    }

    if(filename != nullptr){
        unsigned int filename_offset = 0;
        string filename_str;
        while (1) {
            const char * id = filename + filename_offset;
            if (invalid_filename_address(id)) { //Checks to make sure filename is within our address space, if not, fail to create page
                return nullptr;
            }

            if(vm_fault(id, false) == -1){ //If unable to read-in filename, fail to create page
                return nullptr;
            }

            unsigned int ppage = page_table_base_register->ptes[((uintptr_t)id - processes[curr_pid]->arena_start)/VM_PAGESIZE].ppage;
            unsigned int ppage_offset = ((uintptr_t)id - processes[curr_pid]->arena_start) % VM_PAGESIZE;
            filename_str += ((char*) vm_physmem)[((VM_PAGESIZE * ppage) + ppage_offset)];
            filename_offset++;

            if(((char*) vm_physmem)[((VM_PAGESIZE * ppage) + ppage_offset)] == '\0'){ //If we reach a nullptr, we know we've read our whole filename
                break;
            }
        }

        char * copy_str = new char[filename_offset]; //We need to maintain our own copy of filename, so that we always know what it was.
        filename_str.copy(copy_str, filename_offset);

        string filename_for_map(copy_str);
        filename_for_map += '-' + to_string(block);

        if(filebacked_map.find(filename_for_map) == filebacked_map.end()){//Filename+block not found in map, insert new pager_page_t for that filename+block
            //create new page to add to fileback map

            pager_page_t* temp_page = new_pager_page(copy_str, block);

            filebacked_map[filename_for_map] = temp_page;
        }

        if(filebacked_map[filename_for_map] == nullptr){ //No virtual page for the filename+block found in the map.
            return nullptr;
        }

        int first_invalid_page = arena_valid_page_size();
        uintptr_t return_arena_end = processes[curr_pid]->arena_valid_end;
        processes[curr_pid]->arena_valid_end += VM_PAGESIZE; 
        page_table_entry_t add_to_page = processes[curr_pid]->infrastructure_page_table->ptes[(return_arena_end - (uintptr_t)VM_ARENA_BASEADDR)/VM_PAGESIZE];
        enable_page_protection(&add_to_page);
        filebacked_map[filename_for_map]->page_table_entries.push_back(make_pair(curr_pid, &processes[curr_pid]->infrastructure_page_table->ptes[(return_arena_end - (uintptr_t)VM_ARENA_BASEADDR)/VM_PAGESIZE]));
        if (filebacked_map[filename_for_map]->page_table_entries.size() > 1) {
            filebacked_map[filename_for_map]->echo_to_ptes();
        }
        // processes[curr_pid]->infrastructure_page_table->ptes[(return_arena_end - (uintptr_t)VM_ARENA_BASEADDR)/VM_PAGESIZE] = add_to_page;

        filebacked_map[filename_for_map]->initalize_new_entry();
        // if(filebacked_map[filename_for_map]->resident_bit){
        //     filebacked_map[filename_for_map]->page_table_entries.front().second->ppage = filebacked_map[filename_for_map]->physical_page;
        // }
        // else{
        //     filebacked_map[filename_for_map]->page_table_entries.front().second->ppage = 0;
        // }

        // filebacked_map[filename_for_map]->page_table_entries.front().second->ppage = filebacked_map[filename_for_map]->reference_bit && curr_page->resident_bit;
        // filebacked_map[filename_for_map]->page_table_entries.front().second->ppage = filebacked_map[filename_for_map]->reference_bit && curr_page->dirty_bit;

        // if(filebacked_map[filename_for_map]->swap_backed == true && curr_page->page_table_entries.size() > 1){
        //     filebacked_map[filename_for_map]->page_table_entries.front().second->write_enable = false;
        //     filebacked_map[filename_for_map]->echo_to_ptes();
        // }
        processes[curr_pid]->page_table->entries[first_invalid_page] = filebacked_map[filename_for_map];
        return (void *)(return_arena_end);
    }
    else{
        if(swap_counter < swap_size){
            int first_invalid_page = arena_valid_page_size();
            pager_page_t* temp_page = new_pager_page(filename, swap_index.front());
            swap_index.pop_front();
            swap_counter++;
            //Make new page a "zero" page
            temp_page->swap_backed = true;
            temp_page->reference_bit = true;
            temp_page->resident_bit = true;
            temp_page->pinned = true;
            temp_page->physical_page = 0;

            if(temp_page == nullptr){
                return nullptr;
            }

            uintptr_t return_arena_end = processes[curr_pid]->arena_valid_end;
            processes[curr_pid]->arena_valid_end += VM_PAGESIZE;

            page_table_entry_t add_to_page = processes[curr_pid]->infrastructure_page_table->ptes[(return_arena_end - (uintptr_t)VM_ARENA_BASEADDR)/VM_PAGESIZE];
            enable_page_protection(&add_to_page);
            add_to_page.read_enable = true;
            processes[curr_pid]->infrastructure_page_table->ptes[(return_arena_end - (uintptr_t)VM_ARENA_BASEADDR)/VM_PAGESIZE] = add_to_page;
            temp_page->page_table_entries.push_back(make_pair(curr_pid, &processes[curr_pid]->infrastructure_page_table->ptes[(return_arena_end - (uintptr_t)VM_ARENA_BASEADDR)/VM_PAGESIZE]));

            temp_page->initalize_new_entry();
            processes[curr_pid]->page_table->entries[first_invalid_page] = temp_page;
            return (void *)(return_arena_end);
        }
        else{
            return nullptr;
        }
    }
}

pager_page_t* copy_on_write(const void* addr, pager_page_t* original_page){
    if(original_page->swap_backed == true && original_page->page_table_entries.size() > 1){

        for(size_t i = 0; i < original_page->page_table_entries.size(); i++){
            if(original_page->page_table_entries[i].first == curr_pid && original_page->page_table_entries[i].second == &page_table_base_register->ptes[((uintptr_t) addr - processes[curr_pid]->arena_start) / VM_PAGESIZE]){
                //copy_on_page = original_page->page_table_entries[i].second;
                original_page->page_table_entries.erase(original_page->page_table_entries.begin() + i);
                break;
            }
        }

        vm_fault(addr, false); //Hint: Writing to a virtual page that is being shared via copy-on-write should have the same effect on the system as reading it, then writing it.
        pager_page_t* copy_on_written_page = new_pager_page(nullptr, reserved_swap_index.front());
        reserved_swap_index.pop_front();

        processes[curr_pid]->page_table->entries[((uintptr_t)addr - (uintptr_t)VM_ARENA_BASEADDR)/VM_PAGESIZE] = copy_on_written_page;

        copy_on_page_buffer = new char[VM_PAGESIZE];
        memcpy(copy_on_page_buffer, &((char*)vm_physmem)[VM_PAGESIZE * original_page->page_table_entries.front().second->ppage], VM_PAGESIZE);

        copy_on_written_page->page_table_entries.push_back(make_pair(curr_pid, &page_table_base_register->ptes[((uintptr_t) addr - processes[curr_pid]->arena_start) / VM_PAGESIZE]));

        return copy_on_written_page;
    }
    return nullptr;
}

void evict(){
    while(1){ 
        if(clocker.front()->reference_bit == true || clocker.front()->pinned == true){ //Clock hand pointing at page with reference of "1"
            if(clocker.front()->pinned == false){
                clocker.front()->reference_bit = false;
                if(!clocker.front()->page_table_entries.empty()){
                    clocker.front()->page_table_entries.front().second->read_enable = false; //Lose privelages when you lose your reference
                    clocker.front()->page_table_entries.front().second->write_enable = false;
                }

                if(clocker.front()->page_table_entries.size() > 1){
                    clocker.front()->echo_to_ptes();
                }
            }

            clocker.push_back(clocker.front());
            clocker.pop_front();
        }
        else{  //Clock hand pointing at page with reference of "0"

            clocker.front()->page_table_entries.front().second->ppage = 0;
            clocker.front()->page_table_entries.front().second->read_enable = false;
            clocker.front()->page_table_entries.front().second->write_enable = false;

            if(clocker.front()->page_table_entries.size() > 1){
                clocker.front()->echo_to_ptes();
            }


            if(clocker.front()->dirty_bit == true && clocker.front()->physical_page != 0 && (clocker.front()->filename != nullptr || clocker.front()->page_table_entries.size() != 0)){
                file_write(clocker.front()->filename, clocker.front()->block, &((char *)vm_physmem)[VM_PAGESIZE * clocker.front()->physical_page]);
            }

            phys_index.push_back(clocker.front()->physical_page);
            clocker.front()->physical_page = 0;
            phys_counter--;
            
            clocker.front()->resident_bit = false;
            clocker.front()->reference_bit = false;
            clocker.front()->dirty_bit = false;
            
            clocker.pop_front();
            return;
        }
    }
}

int clock_insert(pager_page_t* insert_page){
    if(clocker.size() == physmem_size){
        evict();
    }

    insert_page->page_table_entries.front().second->ppage = phys_index.front();
    insert_page->physical_page = phys_index.front();
    phys_index.pop_front();
    phys_counter++;

    if(insert_page->page_table_entries.size() > 1){
        insert_page->echo_to_ptes();
    }

    if(copy_on_page_buffer != nullptr){ //If copy_on_page_buffer has some relevant information
        memcpy(&((char *)vm_physmem)[VM_PAGESIZE * (insert_page->page_table_entries.front().second->ppage)], copy_on_page_buffer, VM_PAGESIZE);
        delete copy_on_page_buffer;
        copy_on_page_buffer = nullptr; //Reset copy_on_page_buffer to be nothing, so that in case something didn't copy on write, it's the same as the zero page.
    }
    else if(insert_page->pinned == true){//Zero page, so we just copy out data from the zero page
        memcpy(&((char *)vm_physmem)[VM_PAGESIZE * (insert_page->page_table_entries.front().second->ppage)], &((char *)vm_physmem)[VM_PAGESIZE * 0], VM_PAGESIZE);
        insert_page->pinned = false;
    }
    else{
        int result = file_read(insert_page->filename, insert_page->block, &((char *)vm_physmem)[VM_PAGESIZE * (insert_page->page_table_entries.front().second->ppage)]);
        if(result == -1){
            //Failed to read
            phys_index.push_back(insert_page->physical_page); //return physical block because read was a failure
            return -1;
        }
    }

    insert_page->resident_bit = true;

    clocker.push_back(insert_page); //Push back is inserting the element and then moving the hand one place forward
    return 0;
}

int vm_fault(const void* addr, bool write_flag){
    
    if(invalid_filename_address((char *)addr)){ //If addr falls outside of bounds of arena, cannot read/write to it
        return -1;
    }

    int result = 0; //Assume that everything will be a success
    uintptr_t addr_index = ((uintptr_t)addr - (uintptr_t)VM_ARENA_BASEADDR)/VM_PAGESIZE;
    pager_page_t* addr_page = processes[curr_pid]->page_table->entries[addr_index];
    pager_page_t* written_to_copy = nullptr;

    if(write_flag == true){ //Only need to copy on write if the write flag is high
        written_to_copy = copy_on_write(addr, addr_page);
        if(written_to_copy != nullptr){ //Only change addr_page if the page being written to was actually changed (i.e. it was copied during copy_on_write)
            addr_page = written_to_copy;
        }
    }

    if(addr_page->resident_bit == false || (addr_page->swap_backed == true && addr_page->pinned == true)){
        result = clock_insert(addr_page); //Changes to result to be determined by if inserting into clock (and physmem) was successful
        if(result == -1){
            return -1; //vm_fault was a failure
        }
    }

    addr_page->page_table_entries.front().second->read_enable = true;

    bool swap_backed_not_shared = !((addr_page->swap_backed == true) && (addr_page->page_table_entries.size() > 1)); //A write enabled page cannot be swap back and shared (need to fault for copy on write)
    bool written_to_or_dirty = ((write_flag == true) || (addr_page->dirty_bit == true)); //Write flag must either need to become true now (we wrote to it) or was previously true (dirty bit is true)
    bool already_write_enabled = addr_page->page_table_entries.front().second->write_enable; //Shouldn't matter, but if a page was already write enabled, it gets to stay write enabled

    addr_page->page_table_entries.front().second->write_enable = (swap_backed_not_shared && written_to_or_dirty) || already_write_enabled;

    if(addr_page->page_table_entries.size() > 1){
        addr_page->echo_to_ptes();
    }

    addr_page->reference_bit = true;
    addr_page->dirty_bit = write_flag || addr_page->dirty_bit; //Dirty bit should stay high if it's already high or if it's being written to on this fault

    return result;
}