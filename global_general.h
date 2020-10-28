#ifndef _GLOBAL_GENERAL_H_
#define _GLOBAL_GENERAL_H_
#include "vm_pager.h"

extern unsigned int physmem_size;

struct pager_page_t{
    page_table_entry_t base;
    bool reference_bit = 0;
    bool swap_back = 0;
};


#endif /* _GLOBAL_GENERAL_ */