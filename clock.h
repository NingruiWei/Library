#ifndef _CLOCK_H_
#define _CLOCK_H_
#include "vm_pager.h"
#include "global_general.h"
#include <vector>
#include <list>
using namespace std;

class Clock {
    public:
        list<pager_page_t*> tlb;
        int clock_hand;
        int tlb_size;
        Clock();
        ~Clock();
        void insert(pager_page_t* page);
        void evict();
        int spin_clock();
        
};

#endif /* _CLOCK_H_ */