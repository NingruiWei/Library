#include "clock.h"
#include <cassert>

Clock::Clock() {
    tlb.resize(physmem_size, nullptr);
    curr_index = 0;
    tlb_size = 0;
}

void Clock::copy_page(page_table_entry_t* page, const char *filename, unsigned int block) {
    tlb[curr_index] = new pager_page_t;
    tlb[curr_index]->base = page;
    tlb[curr_index]->filename = filename;
    tlb[curr_index]->block = block;
}

void Clock::move_clock_hand() {
    if (curr_index == physmem_size - 1){
        curr_index = 0;
    }
    else{
        ++curr_index;
    }
}

void Clock::insert(page_table_entry_t* page, const char *filename, unsigned int block){
   for(size_t i = 0; i < physmem_size; ++i){
        if (tlb[curr_index] == nullptr){ // if we're at a nullptr, there is space to fill the clock
            copy_page(page, filename, block);
            tlb[curr_index]->reference_bit = 1;
            return;
        }
        else if (tlb[curr_index]->reference_bit == 0){
            evict_from_clock();
            copy_page(page, filename, block);
            tlb[curr_index]->reference_bit = 1;
            return;
        }
        else{
            assert(tlb[curr_index] != nullptr);
            tlb[curr_index]->reference_bit = 0;
        }
        move_clock_hand();
    }
}
void Clock::evict_from_clock() {
    if (tlb[curr_index]->base->write_enable == 1) {
        file_write(tlb[curr_index]->filename, tlb[curr_index]->block, tlb[curr_index]->base);
    }
    delete tlb[curr_index];
}

Clock::~Clock() {

}
