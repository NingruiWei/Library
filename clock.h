#ifndef _CLOCK_H_
#define _CLOCK_H_
#include "vm_pager.h"
#include "global_general.h"
#include <vector>

using namespace std;

class Clock {
    public:
        vector<page_table_entry_t*> tlb;
        int curr_index;
        vector<bool> looked_at;
        Clock();
        ~Clock();
        
};

#endif /* _CLOCK_H_ */