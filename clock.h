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
        vector<page_table_entry_t*> tlb;
};

#endif /* _CLOCK_H_ */