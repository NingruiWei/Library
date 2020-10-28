#include "clock.h"
#include <cassert>

Clock::Clock() {
    tlb.resize(physmem_size, nullptr);
    curr_index = 0;
    tlb_size = 0;
}


void Clock::insert(page_table_entry_t* page){
   for(size_t i = 0; i < physmem_size; ++i){
        if (tlb[curr_index] == nullptr){ // if we're at a nullptr, it's the first time through
            pager_page_t*temp;
            temp->base = page;
            tlb[curr_index] = temp;
            return;
        }
        if (tlb[curr_index] == nullptr || tlb[curr_index]->reference_bit == 0){
            
            return curr_index;
        }
        else{
            assert(tlb[curr_index] != nullptr);
            tlb[curr_index]->reference_bit = 0;
        }
        if (curr_index == physmem_size - 1){
        
            curr_index = 0;
        }
        else{
            ++curr_index;
        }

}
void Clock::evict(){

}

int Clock::spin_clock(){ // goes around and looks at all the pages and returns the first unseen page. 
    for(size_t i = 0; i < physmem_size; ++i){
        if (tlb[curr_index] == nullptr){ // if we're at a nullptr, it's the first time through
            pager_page_t * temp;
            temp->base = 
            return;
        }
        if (tlb[curr_index] == nullptr || tlb[curr_index]->reference_bit == 0){
            
            return curr_index;
        }
        else{
            assert(tlb[curr_index] != nullptr);
            tlb[curr_index]->reference_bit = 0;
        }
        if (curr_index == physmem_size - 1){
        
            curr_index = 0;
        }
        else{
            ++curr_index;
        }
    }

}
