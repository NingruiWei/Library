#ifndef _CLOCK_H_
#define _CLOCK_H_
#include "vm_pager.h"
#include "global_general.h"
#include <vector>
using namespace std;
class Clock {
    public:
        Clock();
        ~Clock();
        vector<pager_page_t*> tlb;
        unsigned int curr_index;
        int tlb_size;
        void insert(page_table_entry_t* page, const char *filename, unsigned int block);
        void evict_from_clock();
        void move_clock_hand();
        void copy_page(page_table_entry_t* page, const char *filename, unsigned int block);
};

#endif /* _CLOCK_H_ */