 
#include "ost_common.h"
#include "debug.h"
#include "filesystem.h"

int main(void)
{
    system_initialize();
    debug_printf("[OST] Starting OpenStoryTeller: V%d.%d\n", 1, 0);
    filesystem_mount();

    for (;;)
    {
    }
    
    return 0;
}
