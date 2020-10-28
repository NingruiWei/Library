#include "clock.h"
#include <cassert>
#include <list>

Clock::Clock() {
    tlb.resize(physmem_size, nullptr);
    curr_index = 0;
    tlb_size = 0;
}


void Clock::insert(pager_page_t* page){
    if (tlb_size < physmem_size){
        tlb[curr_index] = page;
        curr_index++;
    }
    else{
        



    }

}
void Clock::evict(){

}

int Clock::spin_clock(){ // goes around and looks at all the pages and returns the first unseen page. 
    for(size_t i = 0; i < physmem_size; ++i){
        assert()
        if (tlb[curr_index]->reference_bit == 0){
            return curr_index;
        }
        else{
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
