#ifndef _CLOCK_H_
#define _CLOCK_H_
#include "vm_pager.h"
#include "global_general.h"
#include <vector>
using namespace std;
class Clock {
    public:
        vector<pager_page_t*> tlb;
        int curr_index;
        int tlb_size;
        Clock();
        ~Clock();
        void insert(page_table_entry_t* page);
        void evict();
        int spin_clock();
        
};

#endif /* _CLOCK_H_ */