#include "clock.h"

Clock::Clock() {
    tlb.resize(physmem_size, nullptr);
    looked_at.resize(physmem_size, 0);
    curr_index = 0;
}